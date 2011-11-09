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

#include "GeoNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"

using namespace coral;

void GetGeoElements::contextChanged(Node *parentNode, Enum *enum_){
	GetGeoElements* self = (GetGeoElements*)parentNode;
	int id = enum_->currentIndex();
	if(id == 0){
		self->_contextualUpdate = &GetGeoElements::updateVertices;
	}
	else if(id == 1){
		self->_contextualUpdate = &GetGeoElements::updateEdges;
	}
	else if(id == 2){
		self->_contextualUpdate = &GetGeoElements::updateFaces;
	}
	else if(id == 3){
		self->_contextualUpdate = &GetGeoElements::updateNormalsPerVertex;
	}
	else if(id == 4){
		self->_contextualUpdate = &GetGeoElements::updateNormalsPerFace;
	}
}

GetGeoElements::GetGeoElements(const std::string &name, Node *parent): 
Node(name, parent),
_contextualUpdate(0){
	_context = new EnumAttribute("context", this);
	_geo = new GeoAttribute("geo", this);
	_points = new NumericAttribute("points", this);
	_indices = new NumericAttribute("indices", this);
	
	addInputAttribute(_context);
	addInputAttribute(_geo);
	addOutputAttribute(_points);
	addOutputAttribute(_indices);
	
	setAttributeAffect(_context, _points);
	setAttributeAffect(_context, _indices);
	setAttributeAffect(_geo, _points);
	setAttributeAffect(_geo, _indices);
	
	setAttributeAllowedSpecialization(_indices, "IntArray");
	setAttributeAllowedSpecialization(_points, "Vec3Array");
	
	Enum *context = _context->outValue();
	context->addEntry(0, "vertices");
	context->addEntry(1, "edges");
	context->addEntry(2, "faces");
	context->addEntry(3, "normals per vertex");
	context->addEntry(4, "normals per face");
	
	context->setCurrentIndexChangedCallback(this, GetGeoElements::contextChanged);
	context->setCurrentIndex(0);
}

void GetGeoElements::updateVertices(Geo *geo, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	points = geo->points();
	int size = points.size();
	indices.resize(size);
	for(int i = 0; i < size; ++i){
		indices[i] = i;
	}
}

void GetGeoElements::updateEdges(Geo *geo, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	const std::vector<Edge*> &edges = geo->edges();
	int size = edges.size();
	indices.resize(size);
	points.resize(size * 2);
	
	for(int i = 0; i < size; ++i){
		indices[i] = i;
		
		Edge *edge = edges[i];
		const std::vector<Vertex*> &vertices = edge->vertices();
		
		int pointId = i * 2;
		points[pointId] = vertices[0]->point();
		points[pointId + 1] = vertices[1]->point();
	}
}

void GetGeoElements::updateFaces(Geo *geo, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	const std::vector<Face*> &faces = geo->faces();
	int size = faces.size();
	indices.resize(size);
	
	for(int i = 0; i < size; ++i){
		indices[i] = i;
		
		Face *face = faces[i];
		std::vector<Imath::V3f> facePoints = face->points();
		for(int j = 0; j < facePoints.size(); ++j){
			points.push_back(facePoints[j]);
		}
	}
}

void GetGeoElements::updateNormalsPerVertex(Geo *geo, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	points = geo->verticesNormals();
	int size = points.size();
	indices.resize(size);
	for(int i = 0; i < size; ++i){
		indices[i] = i;
	}
}

void GetGeoElements::updateNormalsPerFace(Geo *geo, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	points = geo->faceNormals();
	int size = points.size();
	indices.resize(size);
	for(int i = 0; i < size; ++i){
		indices[i] = i;
	}
}

void GetGeoElements::update(Attribute *attribute){
	if(_contextualUpdate){
		Geo *geo = _geo->value();
		std::vector<Imath::V3f> points;
		std::vector<int> indices;
		(this->*_contextualUpdate)(geo, points, indices);
		
		_points->outValue()->setVec3Values(points);
		_indices->outValue()->setIntValues(indices);
		
		setAttributeIsClean(_points, true);
		setAttributeIsClean(_indices, true);
	}
}

void GetGeoSubElements::contextChanged(Node *parentNode, Enum *enum_){
	GetGeoSubElements* self = (GetGeoSubElements*)parentNode;
	int id = enum_->currentIndex();
	if(id == 0){
		self->_contextualUpdate = &GetGeoSubElements::updateVertexNeighbours;
	}
	else if(id == 1){
		self->_contextualUpdate = &GetGeoSubElements::updateEdgeVertices;
	}
	else if(id == 2){
		self->_contextualUpdate = &GetGeoSubElements::updateFaceVertices;
	}
}

GetGeoSubElements::GetGeoSubElements(const std::string &name, Node *parent): 
Node(name, parent),
_contextualUpdate(0){
	_context = new EnumAttribute("context", this);
	_geo = new GeoAttribute("geo", this);
	_index = new NumericAttribute("index", this);
	_points = new NumericAttribute("points", this);
	_indices = new NumericAttribute("indices", this);

	addInputAttribute(_context);
	addInputAttribute(_geo);
	addInputAttribute(_index);
	addOutputAttribute(_points);
	addOutputAttribute(_indices);

	setAttributeAffect(_context, _points);
	setAttributeAffect(_context, _indices);
	setAttributeAffect(_geo, _points);
	setAttributeAffect(_geo, _indices);
	setAttributeAffect(_index, _points);
	setAttributeAffect(_index, _indices);
	
	setAttributeAllowedSpecialization(_index, "Int");
	setAttributeAllowedSpecialization(_points, "Vec3Array");
	setAttributeAllowedSpecialization(_indices, "IntArray");
	
	Enum *context = _context->outValue();
	context->addEntry(0, "vertex neighbours");
	context->addEntry(1, "edge vertices");
	context->addEntry(2, "face vertices");

	context->setCurrentIndexChangedCallback(this, GetGeoSubElements::contextChanged);
	context->setCurrentIndex(0);
}

void GetGeoSubElements::updateVertexNeighbours(Geo *geo, int index, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	const std::vector<Vertex*> &vertices = geo->vertices();
	if(index >= 0 && index < vertices.size()){
		Vertex *vertex = vertices[index];
		const std::vector<Vertex*> &neighbours = vertex->neighbourVertices();
		int size = neighbours.size();

		points.resize(size);
		indices.resize(size);

		for(int i = 0; i < size; ++i){
			Vertex *neighbour = neighbours[i];
			points[i] = neighbour->point();
			indices[i] = neighbour->id();
		}
	}
}

void GetGeoSubElements::updateEdgeVertices(Geo *geo, int index, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	const std::vector<Edge*> &edges = geo->edges();
	if(index >= 0 && index < edges.size()){
		Edge *edge = edges[index];
		const std::vector<Vertex*> &vertices = edge->vertices();
		
		int size = 2;
		points.resize(size);
		indices.resize(size);

		for(int i = 0; i < size; ++i){
			Vertex *vertex = vertices[i];
			points[i] = vertex->point();
			indices[i] = vertex->id();
		}
	}
}

void GetGeoSubElements::updateFaceVertices(Geo *geo, int index, std::vector<Imath::V3f> &points, std::vector<int> &indices){
	const std::vector<Face*> &faces = geo->faces();
	if(index >= 0 && index < faces.size()){
		Face *face = faces[index];
		const std::vector<Vertex*> &vertices = face->vertices();

		int size = vertices.size();
		points.resize(size);
		indices.resize(size);

		for(int i = 0; i < size; ++i){
			Vertex *vertex = vertices[i];
			points[i] = vertex->point();
			indices[i] = vertex->id();
		}
	}
}

void GetGeoSubElements::update(Attribute *attribute){
	if(_contextualUpdate){
		Geo *geo = _geo->value();
		int index = _index->value()->intValueAt(0);

		std::vector<Imath::V3f> points;
		std::vector<int> indices;
		(this->*_contextualUpdate)(geo, index, points, indices);
		
		_points->outValue()->setVec3Values(points);
		_indices->outValue()->setIntValues(indices);
		
		setAttributeIsClean(_points, true);
		setAttributeIsClean(_indices, true);
	}
}


GetGeoPoints::GetGeoPoints(const std::string &name, Node *parent): Node(name, parent){
	_geo = new GeoAttribute("geo", this);
	_points = new NumericAttribute("points", this);
	
	addInputAttribute(_geo);
	addOutputAttribute(_points);
	
	setAttributeAffect(_geo, _points);
	
	setAttributeAllowedSpecialization(_points, "Vec3Array");
}

void GetGeoPoints::update(Attribute *attribute){
	_points->outValue()->setVec3Values(_geo->value()->points());
}

SetGeoPoints::SetGeoPoints(const std::string &name, Node *parent): Node(name, parent){		
	_inGeo = new GeoAttribute("inGeo", this);
	_points = new NumericAttribute("points", this);
	_outGeo = new GeoAttribute("outGeo", this);
	
	addInputAttribute(_inGeo);
	addInputAttribute(_points);
	addOutputAttribute(_outGeo);
	
	setAttributeAffect(_inGeo, _outGeo);
	setAttributeAffect(_points, _outGeo);
	
	setAttributeAllowedSpecialization(_points, "Vec3Array");
}

void SetGeoPoints::update(Attribute *attribute){
	Geo *outGeoValue = _outGeo->outValue();
	
	outGeoValue->copy(_inGeo->value());
	outGeoValue->displacePoints(_points->value()->vec3Values());
}

GetGeoNormals::GetGeoNormals(const std::string &name, Node *parent): Node(name, parent){
	_geo = new GeoAttribute("geo", this);
	_normals = new NumericAttribute("normals", this);
	
	addInputAttribute(_geo);
	addOutputAttribute(_normals);
	
	setAttributeAffect(_geo, _normals);
	
	setAttributeAllowedSpecialization(_normals, "Vec3Array");
}

void GetGeoNormals::update(Attribute *attribute){
	_normals->outValue()->setVec3Values(_geo->value()->verticesNormals());
}


GeoNeighbourPoints::GeoNeighbourPoints(const std::string &name, Node *parent): Node(name, parent){
	_geo = new GeoAttribute("geo", this);	
	_vertex = new NumericAttribute("vertex", this);
	_neighbourPoints = new NumericAttribute("points", this);
	_neighbourVertices = new NumericAttribute("vertices", this);
	
	addInputAttribute(_geo);
	addInputAttribute(_vertex);
	addOutputAttribute(_neighbourPoints);
	addOutputAttribute(_neighbourVertices);
	
	setAttributeAffect(_geo, _neighbourPoints);
	setAttributeAffect(_vertex, _neighbourPoints);
	setAttributeAffect(_geo, _neighbourVertices);
	setAttributeAffect(_vertex, _neighbourVertices);

	setAttributeAllowedSpecialization(_vertex, "Int");
	setAttributeAllowedSpecialization(_neighbourPoints, "Vec3Array");
	setAttributeAllowedSpecialization(_neighbourVertices, "IntArray");
}

void GeoNeighbourPoints::update(Attribute *attribute){
	Geo *geo = _geo->value();
	int vertexId = _vertex->value()->intValueAt(0);

	const std::vector<Vertex*> &vertices = geo->vertices();
	
	if(vertexId < 0 || vertexId >= vertices.size()){
		if(attribute == _neighbourPoints){
			_neighbourPoints->outValue()->setVec3Values(std::vector<Imath::V3f>());
		}
		else{
			_neighbourVertices->outValue()->setIntValues(std::vector<int>());
		}
	}
	else{
		if(attribute == _neighbourPoints){
			_neighbourPoints->outValue()->setVec3Values(vertices[vertexId]->neighbourPoints());
		}
		else{
			const std::vector<Vertex*> &neighbourVertices = vertices[vertexId]->neighbourVertices();
			int neighboursSize = neighbourVertices.size();
			
			std::vector<int> neighbourIds(neighboursSize);
			for(int i = 0; i < neighboursSize; ++i){
				neighbourIds[i] = neighbourVertices[i]->id();
			}
			_neighbourVertices->outValue()->setIntValues(neighbourIds);
		}
	}	
}

