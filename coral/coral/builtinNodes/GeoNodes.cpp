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

using namespace coral;

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

	const std::vector<Vertex> &vertices = geo->vertices();
	
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
			_neighbourPoints->outValue()->setVec3Values(vertices[vertexId].neighbourPoints());
		}
		else{
			const std::vector<Vertex*> &neighbourVertices = vertices[vertexId].neighbourVertices();
			int neighboursSize = neighbourVertices.size();

			std::vector<int> neighbourIds(neighboursSize);
			for(int i = 0; i < neighboursSize; ++i){
				neighbourIds[i] = neighbourVertices[i]->id();
			}
			_neighbourVertices->outValue()->setIntValues(neighbourIds);
		}
	}	
}

