// <license>
// Copyright (C) 2011 Andrea Interguglielmi, All rights reserved.
// This file is part of the coral repository downloaded from http://code.google.com/p/coral-repo.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
// 
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// </license>



#include "Geo.h"
#include <assert.h>
#include "containerUtils.h"

using namespace coral;
using namespace containerUtils;

Geo::Geo():
_faceNormalsDirty(true),
_verticesNormalsDirty(true),
_topologyStructuresDirty(true){
}

void Geo::copy(const Geo *other){
	clear();
	
	_points = other->_points;
	_rawFaces = other->_rawFaces;
}

const std::vector<Imath::V3f> &Geo::points(){
	return _points;
}

int Geo::pointsCount() const{
	return (int)_points.size();
}

const std::vector<std::vector<int> > &Geo::faces(){
	return _rawFaces;
}

int Geo::facesCount() const{
	return (int)_rawFaces.size();
}

bool Geo::hasSameTopology(const std::vector<std::vector<int> > &faces) const{
	return faces == _rawFaces;
}

// assign new vertices coordinates IF arrays match
void Geo::setPoints(const std::vector<Imath::V3f> &points){
	if(_points.size() == points.size()){

		_points = points;

		_faceNormalsDirty = true;
		_verticesNormalsDirty = true;
	}
}

// Will displace the points of this geo without modifying the size of the array.
void Geo::displacePoints(const std::vector<Imath::V3f> &displacedPoints){
	int displacedPointsSize = displacedPoints.size();
	int pointsSize = _points.size();
	int minSize;
	
	if(displacedPointsSize >= pointsSize){
		minSize = pointsSize;
	}
	else{
		minSize = displacedPointsSize;
	}
	
	for(int i = 0; i < minSize; ++i){
		_points[i] = displacedPoints[i];
	}

	_faceNormalsDirty = true;
	_verticesNormalsDirty = true;
}

void Geo::clear(){
	_points.clear();
	_rawFaces.clear();
	_faces.clear();
	_vertices.clear();
	
	_vertexIdOffset.clear();
	_faceNormals.clear();
	_vertexFaces.clear();

	_faceNormalsDirty = true;
	_verticesNormalsDirty = true;
	_topologyStructuresDirty = true;
}

void Geo::build(const std::vector<Imath::V3f> &points, const std::vector<std::vector<int> > &faces){
	clear();
	
	_points = points;
	_rawFaces = faces;
}

void Geo::computeVertexPerFaceNormals(std::vector<Imath::V3f> &vertexPerFaceNormals){
	int lastFaceID = _rawFaces.size()-1;
	std::vector<int> &lastFace = _rawFaces[lastFaceID];

	int size = _vertexIdOffset[lastFaceID] + lastFace.size();

	vertexPerFaceNormals.resize(size);

	int counter = 0;
	for(unsigned int faceID = 0; faceID < _rawFaces.size(); ++faceID){
		std::vector<int> &face = _rawFaces[faceID];

		int faceVerticesCount = (int)face.size();

		for(int i = 0; i < faceVerticesCount; i++){
			// for each triplet of points in this polygon, cross the 2 adjacent points of each point
			// es: {last,0,1}, {0,1,2}, {1,2,3}, {2,3,last}, {3,last,0}

			const Imath::V3f& v0 = _points[face[i == 0 ? faceVerticesCount-1 : i-1]];
			const Imath::V3f& v1 = _points[face[i]];
			const Imath::V3f& v2 = _points[face[i == faceVerticesCount-1 ? 0 : i+1]];

			vertexPerFaceNormals[counter].setValue((v1-v0).cross(v2-v0).normalized());
			++counter;
		}
	}
}

void Geo::cacheFaceNormals(){
	if(_topologyStructuresDirty){
		cacheTopologyStructures();
	}
	
	std::vector<Imath::V3f> vertexPerFaceNormals;
	computeVertexPerFaceNormals(vertexPerFaceNormals);
	
	int facesCount = (int)_rawFaces.size();
	_faceNormals.resize(facesCount);

	int counter = 0;
	for(int f = 0; f < facesCount; ++f){
		std::vector<int> &face = _rawFaces[f];

		Imath::V3f faceNormal(0.f, 0.f, 0.f);

		for(unsigned int v = 0; v < face.size(); ++v){
			faceNormal += vertexPerFaceNormals[counter];
			++counter;
		}

		faceNormal /= face.size();
		faceNormal.normalize();

		_faceNormals[f].setValue(faceNormal);
	}

	_faceNormalsDirty = false;
}

const std::vector<Imath::V3f> &Geo::faceNormals(){
	#ifdef CORAL_PARALLEL_TBB
		tbb::mutex::scoped_lock lock(_localMutex);
	#endif
	
	if(_faceNormalsDirty){
		cacheFaceNormals();
	}
	
	return _faceNormals;
}

const std::vector<Imath::V3f> &Geo::verticesNormals(){
	#ifdef CORAL_PARALLEL_TBB
		tbb::mutex::scoped_lock lock(_localMutex);
	#endif
	
	if(_verticesNormalsDirty){
		if(_faceNormalsDirty){
			cacheFaceNormals();
		}
		
		int verticesCount = (int)_points.size();
		_verticesNormals.resize(verticesCount);

		for(int vertexID = 0; vertexID < verticesCount; ++vertexID){
			std::vector<int> &faces = _vertexFaces[vertexID];

			Imath::V3f vertexNormal(0.f, 0.f, 0.f);

			for(unsigned int index = 0; index < faces.size(); ++index){
				int faceID = faces[index];

				vertexNormal += _faceNormals[faceID];
			}

			vertexNormal /= faces.size();
			vertexNormal.normalize();

			_verticesNormals[vertexID].setValue(vertexNormal);
		}

		_verticesNormalsDirty = false;
	}
	
	return _verticesNormals;
}

void Geo::cacheTopologyStructures(){
	int faceCount = _rawFaces.size();
	_faces.resize(faceCount);
	_vertexIdOffset.resize(faceCount);
	
	int vertexCount = _points.size();
	_vertices.resize(vertexCount);
	_vertexFaces.resize(vertexCount);
	
	int vertexIdOffset = 0;
	
	for(int i = 0; i < faceCount; ++i){
		std::vector<int> &rawVerticesPerFace = _rawFaces[i];
		
		Face &face = _faces[i];
		face._id = i;
		face._geo = this;
		
		int verticesPerFaceCount = rawVerticesPerFace.size();
		
		_vertexIdOffset[i] = vertexIdOffset;
		
		face._vertices.resize(verticesPerFaceCount);
		face._edges.resize(verticesPerFaceCount);
		face._points.resize(verticesPerFaceCount);
		
		for(int j = 0; j < verticesPerFaceCount; ++j){
			int vertexId = rawVerticesPerFace[j];
			
			_vertexFaces[vertexId].push_back(i);
			
			Imath::V3f &point = _points[vertexId];
			
			face._points[j] = &point;
			
			// vertex
			Vertex &vertex = _vertices[vertexId];
			vertex._id = vertexId;
			vertex._geo = this;
			vertex._point = &point;
			
			face._vertices[j] = &vertex;
			if(!containerUtils::elementInContainer<Face*>(&face, vertex._neighbourFaces)){
				vertex._neighbourFaces.push_back(&face);
			}
			
			// edge
			Edge &edge = face._edges[j];
			edge._faces.push_back(&face);
			edge._geo = this;
			
			if(j > 0){
				int edgeVertexId1;
				int edgeVertexId2;
			
				if(j == verticesPerFaceCount - 1){
					edgeVertexId1 = rawVerticesPerFace[j];
					edgeVertexId2 = rawVerticesPerFace[0];
				}
				else{
					edgeVertexId1 = rawVerticesPerFace[j - 1];
					edgeVertexId2 = rawVerticesPerFace[j];
				}
			
				Vertex &vertex1 = _vertices[edgeVertexId1];
				Vertex &vertex2 = _vertices[edgeVertexId2];
			
				Imath::V3f &point1 = _points[edgeVertexId1];
				Imath::V3f &point2 = _points[edgeVertexId2];
			
				edge._vertices[0] = &vertex1;
				edge._vertices[1] = &vertex2;
				edge._points[0] = &point1;
				edge._points[1] = &point2;
				
				// vertex neighbours
				if(!containerUtils::elementInContainer<Vertex*>(&vertex1, vertex2._neighbourVertices)){
					vertex2._neighbourVertices.push_back(&vertex1);
					vertex2._neighbourPoints.push_back(&point1);
				}
				if(!containerUtils::elementInContainer<Vertex*>(&vertex2, vertex1._neighbourVertices)){
					vertex1._neighbourVertices.push_back(&vertex2);
					vertex1._neighbourPoints.push_back(&point2);
				}
				if(!containerUtils::elementInContainer<Edge*>(&edge, vertex1._neighbourEdges)){
					vertex1._neighbourEdges.push_back(&edge);
				}
				if(!containerUtils::elementInContainer<Edge*>(&edge, vertex2._neighbourEdges)){
					vertex2._neighbourEdges.push_back(&edge);
				}
			}
			
			vertexIdOffset += verticesPerFaceCount;
		}
	}
	
	_topologyStructuresDirty = false;
}

const std::vector<Vertex> &Geo::vertices(){
	#ifdef CORAL_PARALLEL_TBB
		tbb::mutex::scoped_lock lock(_localMutex);
	#endif
	
	if(_topologyStructuresDirty){
		cacheTopologyStructures();
	}

	return _vertices;
}


