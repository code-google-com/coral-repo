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

#ifndef GEO_H
#define GEO_H

#include <vector>
#include <ImathVec.h>

#include "Value.h"

namespace coral{
class Geo;
class Face;
class Edge;
class Vertex;

class Face{
public:
	Face(): _id(0), _geo(0){
	}
	
	Geo *geo(){
		return _geo;
	}
	
	const std::vector<Edge> &edges(){
		return _edges;
	}
	
	std::vector<Vertex*> vertices(){
		return _vertices;
	}
	
	std::vector<Imath::V3f> points(){
		int size = _points.size();
		std::vector<Imath::V3f> points(size);
		for(int i = 0; i < size; ++i){
			points[i] = *_points[i];
		}
		
		return points;
	}
	
private:
	friend class Geo;
	
	int _id;
	Geo *_geo;
	std::vector<Edge> _edges;
	std::vector<Vertex*> _vertices;
	std::vector<Imath::V3f*> _points;
};

class Edge{
public:
	Edge(): _geo(0), _vertices(2), _points(2){
	}
	
	std::vector<Face*> faces(){
		return _faces;
	}
	
	std::vector<Vertex*> vertices(){
		return _vertices;
	}
	
	std::vector<Imath::V3f> points(){
		int size = _points.size();
		std::vector<Imath::V3f> points(size);
		for(int i = 0; i < size; ++i){
			points[i] = *_points[i];
		}
		
		return points;
	}

private:
	friend class Geo;
	
	Geo *_geo;
	std::vector<Face*> _faces;
	std::vector<Vertex*> _vertices;
	std::vector<Imath::V3f*> _points;
};

class Vertex{
public:
	Vertex(): _id(0), _geo(0), _point(0){
	}
	
	Imath::V3f point(){
		return *_point;
	}
	
	std::vector<Face*> neighbourFaces(){
		return _neighbourFaces;
	}
	
	std::vector<Edge*> neighbourEdges(){
		return _neighbourEdges;
	}
	
	std::vector<Vertex*> neighbourVertices(){
		return _neighbourVertices;
	}
	
	std::vector<Imath::V3f> neighbourPoints() const{
		int size = _neighbourPoints.size();
		std::vector<Imath::V3f> points(size);
		for(int i = 0; i < size; ++i){
			points[i] = *_neighbourPoints[i];
		}
		
		return points;
	}

private:
	friend class Geo;

	int _id;
	Geo *_geo;
	Imath::V3f* _point;
	std::vector<Face*> _neighbourFaces;
	std::vector<Edge*> _neighbourEdges;
	std::vector<Vertex*> _neighbourVertices;
	std::vector<Imath::V3f*> _neighbourPoints;
};

//! A class to handle Geometry, used by GeoAttribute. 
class CORAL_EXPORT Geo: public Value{ 
public:
	Geo();
	
	void copy(const Geo *other);
	void build(const std::vector<Imath::V3f> &points, const std::vector<std::vector<int> > &faces);
	const std::vector<Imath::V3f> &points();
	int pointsCount() const;
	const std::vector<std::vector<int> > &faces();
	int facesCount() const;
	const std::vector<Imath::V3f> &faceNormals();
	const std::vector<Imath::V3f> &verticesNormals();
	void setPoints(const std::vector<Imath::V3f> &points);
	void displacePoints(const std::vector<Imath::V3f> &displacedPoints);
	bool hasSameTopology(const std::vector<std::vector<int> > &faces) const;
	void clear();
	const std::vector<Vertex> &vertices();

private:
	void computeVertexPerFaceNormals(std::vector<Imath::V3f> &vertexPerFaceNormals);
	void cacheTopologyStructures();
	
	// topology
	std::vector<std::vector<int> > _rawFaces;
	std::vector<Face> _faces;
	std::vector<Vertex> _vertices;
	
	// external access
	std::vector<Imath::V3f> _points;
	std::vector<Imath::V3f> _faceNormals;
	std::vector<Imath::V3f> _verticesNormals;
	
	// internal
	std::vector<int> _vertexIdOffset;
	std::vector<std::vector<int> > _vertexFaces;

	bool _topologyStructuresDirty;
	bool _faceNormalsDirty;
	bool _verticesNormalsDirty;
};

}

#endif
