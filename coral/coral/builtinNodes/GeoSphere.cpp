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
#include "GeoSphere.h"
#include "../src/Geo.h"

using namespace coral;

GeoSphere::GeoSphere(const std::string &name, Node *parent): Node(name, parent){	
	_radius = new NumericAttribute("radius", this);
	_rings = new NumericAttribute("rings", this);
	_sectors = new NumericAttribute("sectors", this);
	_out = new GeoAttribute("out", this);
	
	addInputAttribute(_radius);
	addInputAttribute(_rings);
	addInputAttribute(_sectors);
	addOutputAttribute(_out);
	
	setAttributeAffect(_radius, _out);
	setAttributeAffect(_rings, _out);
	setAttributeAffect(_sectors, _out);
	
	setAttributeAllowedSpecialization(_radius, "Float");
	setAttributeAllowedSpecialization(_rings, "Int");
	setAttributeAllowedSpecialization(_sectors, "Int");
	
	_radius->outValue()->setFloatValueAt(0, 10.0);
	_rings->outValue()->setIntValueAt(0, 10);
	_sectors->outValue()->setIntValueAt(0, 10);
}

void GeoSphere::updateSlice(Attribute *attribute, unsigned int slice){
	float radius = _radius->value()->floatValueAt(0);
	int rings = _rings->value()->intValueAt(0);
	int sectors = _sectors->value()->intValueAt(0);
	
	if(rings < 2){
		rings = 2;
		_rings->outValue()->setIntValueAt(0, 2);
	}
	if(sectors < 3){
		sectors = 3;
		_sectors->outValue()->setIntValueAt(0, 3);
	}
	
	int totalFaces = rings * sectors;
	int totalPoints = totalFaces + rings + sectors + 1;
	
	float ringStep = 1.0 / rings;
	float sectorStep = 1.0 / sectors;
	
	int i = 0;
	std::vector<Imath::V3f> points(totalPoints);
	
	for(int ring = 0; ring <= rings; ring++){
		float closingOffset = - sin(-M_PI_2 + M_PI * ring * ringStep) * 0.001; // prevents intersection artifacts at both ends of the sphere
		
		for(int sector = 0; sector <= sectors; sector++){
			float y = sin( -M_PI_2 + M_PI * (float(ring) + closingOffset) * ringStep) * radius;
			float x = cos(2*M_PI * (float(sector) + closingOffset) * sectorStep) * sin( M_PI * (float(ring) + closingOffset) * ringStep ) * radius;
			float z = sin(2*M_PI * (float(sector) + closingOffset) * sectorStep) * sin( M_PI * (float(ring) + closingOffset) * ringStep ) * radius;
			
			points[i] = Imath::V3f(x, y, z);
			i++;
		}
	}
	
	std::vector<std::vector<int> > faces(totalFaces);
	std::vector<int> faceVertices(4);
	
	for(int faceId = 0; faceId < totalFaces; ++faceId){
		int row = (faceId / sectors);
		
		faceVertices[3] = (faceId + row);
		faceVertices[2] = (faceId + row + 1);
		faceVertices[1] = (faceId + row + sectors + 2);
		faceVertices[0] = (faceId + row + sectors + 1);
		
		// welding end vertices
		if(faceVertices[2] == ((sectors * (row + 1)) + row)){
			faceVertices[2] = (sectors * row) + row;
		}
		
		if(faceVertices[1] == (sectors * (row + 2)) + row + 1){
			faceVertices[1] = (sectors * (row + 1)) + row + 1;
		}
		
		faces[faceId] = faceVertices;
	}

	_out->outValue()->build(points, faces);
}
