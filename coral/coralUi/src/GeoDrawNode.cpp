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
#include "GeoDrawNode.h"
#include <coral/src/Attribute.h>
#include <coral/src/Numeric.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


using namespace coral;
using namespace coralUi;

GeoDrawNode::GeoDrawNode(const std::string &name, Node *parent): DrawNode(name, parent){	
	_geo = new GeoAttribute("geo", this);
	_smooth = new BoolAttribute("smooth", this);
	_flat = new BoolAttribute("flat", this);
	_wireframe = new BoolAttribute("wireframe", this);
	_points = new BoolAttribute("points", this);
	_normals = new BoolAttribute("normals", this);
	// _ids = new BoolAttribute("ids", this);
	_colors = new NumericAttribute("colors", this);
	
	addInputAttribute(_geo);
	addInputAttribute(_smooth);
	addInputAttribute(_flat);
	addInputAttribute(_wireframe);
	addInputAttribute(_points);
	addInputAttribute(_normals);
	// addInputAttribute(_ids);
	addInputAttribute(_colors);
	
	setAttributeAffect(_geo, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_smooth, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_flat, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_wireframe, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_points, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_normals, (Attribute*)viewportOutputAttribute());
	// setAttributeAffect(_ids, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_colors, (Attribute*)viewportOutputAttribute());
	
	std::vector<std::string> colorSpecializations;
	colorSpecializations.push_back("Col4");
	colorSpecializations.push_back("Col4Array");
	setAttributeAllowedSpecializations(_colors, colorSpecializations);

	catchAttributeDirtied(_geo);
	catchAttributeDirtied(_colors);

	_smooth->outValue()->setBoolValueAt(0, true);

	// generate OpenGL buffers
	glGenBuffers(1, &_vtxBuffer);
	glGenBuffers(1, &_nrmBuffer);
	glGenBuffers(1, &_colBuffer);
	glGenBuffers(1, &_idxBuffer);
}

GeoDrawNode::~GeoDrawNode(){
	glDeleteBuffers(1, &_vtxBuffer);
	glDeleteBuffers(1, &_nrmBuffer);
	glDeleteBuffers(1, &_colBuffer);
	glDeleteBuffers(1, &_idxBuffer);
}

void GeoDrawNode::attributeDirtied(Attribute *attribute){
	if(attribute == _geo){
		updateGeoVBO();
	}
	else if(attribute == _colors){
		updateColorVBO();
	}
}

void GeoDrawNode::updateGeoVBO(){

	Geo *geo = _geo->value();

	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<Imath::V3f> &vtxNormals = geo->verticesNormals();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*points.size(), (GLvoid*)&points[0].x, GL_STATIC_DRAW);

	// normal buffer
	if(vtxNormals.empty() == false){
		glBindBuffer(GL_ARRAY_BUFFER, _nrmBuffer);
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*vtxNormals.size(), (GLvoid*)&vtxNormals[0].x, GL_STATIC_DRAW);
	}

	// index buffer generation
	const std::vector<int> &indices = geo->rawIndices();
	// send to GPU!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indices.size(), &indices[0], GL_STATIC_DRAW);


	// clean OpenGL states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEndList();
}

void GeoDrawNode::updateColorVBO(){

	Numeric *col4Numeric = _colors->value();
	const std::vector<Imath::Color4f> &col4Values = col4Numeric->col4Values();

	// color buffer
	if((col4Numeric->isArray()) && (col4Values.empty() == false)){
		glBindBuffer(GL_ARRAY_BUFFER, _colBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4*sizeof(GLfloat)*col4Values.size(), (GLvoid*)&col4Values[0].r, GL_STATIC_DRAW);
	}
}

void GeoDrawNode::drawPoints(Geo *geo){

	const std::vector<Imath::V3f> &points = geo->points();

	// prepare OpenGL rendering
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	glPointSize(5.f);
	glColor3f(0.f, 1.f, 0.f);

	// set VBOs
	glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glEnableClientState(GL_VERTEX_ARRAY);

	// render
	glDrawArrays(GL_POINTS, 0, points.size());

	// clean OpenGL state
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GeoDrawNode::drawSurface(Geo *geo, bool smooth){
	const std::vector<int> &indexCounts = geo->rawIndexCounts();
	const std::vector<Imath::V3f> &vtxNormals = geo->verticesNormals();

	Numeric *col4Numeric = _colors->value();
	const std::vector<Imath::Color4f> &col4Values = col4Numeric->col4Values();

	bool useColVbo = false;
	if(col4Numeric->type() == Numeric::numericTypeCol4Array){
		useColVbo = true;
	}
	else if(col4Numeric->type() == Numeric::numericTypeCol4){
		// simple color? use it for all the surface
		glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);
	}else{
		// nothing connected? Use default. TODO: should be removed. Default color should be in the color attribute of this node.
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);	// default middle gray
	}

	if(vtxNormals.empty() == false){
		glEnable(GL_LIGHTING);

		if(smooth){
			glShadeModel(GL_SMOOTH);
		}else{
			glShadeModel(GL_FLAT);
		}

		// prepare VBOs
		glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
		glVertexPointer(3, GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, _nrmBuffer);
		glNormalPointer(GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idxBuffer);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		if(useColVbo){
			glBindBuffer(GL_ARRAY_BUFFER, _colBuffer);
			glColorPointer(4, GL_FLOAT, 0, NULL);
			glEnableClientState(GL_COLOR_ARRAY);
		}

		// material stuff
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		// render
		// draw polygon sending poly size (3 or 4+ via indexCounts[i]) and an offset pointer in the GPU memory to the indices to use
		int idOffset = 0;
		for(int i = 0; i < indexCounts.size(); ++i){
			glDrawElements(GL_POLYGON, indexCounts[i], GL_UNSIGNED_INT, (GLuint*)NULL+idOffset);
			idOffset += indexCounts[i];	// 4+3+4+4+4+4+etc...
		}

		// clean OpenGL states
		glDisable(GL_COLOR_MATERIAL);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		if(useColVbo){
			glDisableClientState(GL_COLOR_ARRAY);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void GeoDrawNode::drawWireframe(Geo *geo){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();
	int facesCount = (int)faces.size();
	
	glLineWidth(1.f);							// GL_LINE_BIT
	glColor3f(1.f, 1.f, 1.f);					// GL_CURRENT_BIT

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// GL_POLYGON_BIT

	// set vertices VBOs
	glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glEnableClientState(GL_VERTEX_ARRAY);

	// render
	for(int faceID = 0; faceID < facesCount; ++faceID){
		const std::vector<int> &face = faces[faceID];

		glDrawElements(GL_POLYGON, face.size(), GL_UNSIGNED_INT, (GLvoid*)&face[0]);
	}

	// clean OpenGL states
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GeoDrawNode::drawNormals(Geo *geo, bool shouldDrawFlat){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();

	int facesCount = (int)faces.size();

	glLineWidth(1.f);
	glColor3f(0.f, 0.f, 0.5f);

	if(shouldDrawFlat){
		const std::vector<Imath::V3f> &faceNormals = geo->faceNormals();

		for(int faceID = 0; faceID < facesCount; ++faceID){
			const std::vector<int> &face = faces[faceID];

			const Imath::V3f &normal = faceNormals[faceID];

			glBegin(GL_LINES);
			for(unsigned int index = 0; index < face.size(); ++index){
				int vertexID = face[index];

				const Imath::V3f &point = points[vertexID];

				glVertex3fv(point.getValue());
				glVertex3fv((point+normal).getValue());
			}
			glEnd();
		}
	}
	else{
		const std::vector<Imath::V3f> &verticesNormals = geo->verticesNormals();

		for(int faceID = 0; faceID < facesCount; ++faceID){
			const std::vector<int> &face = faces[faceID];

			glBegin(GL_LINES);
			for(unsigned int index = 0; index < face.size(); ++index){
				int vertexID = face[index];

				const Imath::V3f &normal = verticesNormals[vertexID];
				const Imath::V3f &point = points[vertexID];

				glVertex3fv(point.getValue());
				glVertex3fv((point+normal).getValue());
			}
			glEnd();
		}
	}


	// faceNormals
	const std::vector<Imath::V3f> &faceNormals = geo->faceNormals();

	glLineWidth(2.f);
	glColor3f(0.f, 1.f, 0.f);

	for(int f = 0; f < facesCount; ++f){
		const Imath::V3f &faceNormal = faceNormals[f];

		const std::vector<int> &face = faces[f];

		Imath::V3f faceMidPosition(0.f, 0.f, 0.f);

		glBegin(GL_LINES);
		for(unsigned int p = 0; p < face.size(); ++p){
			faceMidPosition += points[face[p]];
		}
		faceMidPosition /= face.size();

		glVertex3fv(faceMidPosition.getValue());
		glVertex3fv((faceMidPosition + faceNormal).getValue());

		glEnd();
	}
}

// void GeoDrawNode::drawPointIds(Geo *geo){
// 	glDisable(GL_LIGHTING);
// 	glDisable(GL_DEPTH_TEST);
// 	glColor3f(200, 200, 0);
// 	
// 	const std::vector<Imath::V3f> &points = geo->points();
// 	
// 	for(int i = 0; i < points.size(); ++i){
// 		char idStr[sizeof(int)];
// 		sprintf(idStr, "%i", i);
// 		glPushMatrix();
// 		glTranslatef(points[i].x, points[i].y, points[i].z);
// 		glRasterPos2f(0.0, 0.0);
// 		int len = (int) strlen(idStr);
// 		for(int k = 0; k < len; ++k){
// 			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, idStr[k]);
// 		}
// 		glPopMatrix();
// 	}
// 	
// 	glEnable(GL_DEPTH_TEST);
// 	glEnable(GL_LIGHTING);
// 
// }


void GeoDrawNode::draw(){
	DrawNode::draw();

	bool shouldUpdateGeoVBO = false;
	bool shouldUpdateColorVBO = false;

	// check something is connected in _geo and check it is resolved
	if(_geo->input()){
		if(_geo->input()->isClean() == false){
			shouldUpdateGeoVBO = true;
		}
	}

	// check something is connected in _colors and check it is resolved
	if(_colors->input()){
		if(_colors->input()->isClean() == false){
			shouldUpdateColorVBO = true;
		}
	}

	bool shouldDrawSmooth = _smooth->value()->boolValueAt(0);
	bool shouldDrawFlat = _flat->value()->boolValueAt(0);
	bool shouldDrawWireframe = _wireframe->value()->boolValueAt(0);
	bool shouldDrawPoints = _points->value()->boolValueAt(0);
	bool shouldDrawNormals = _normals->value()->boolValueAt(0);
	// bool shouldDrawIds = _ids->value()->boolValueAt(0);
	
	Geo *geo = _geo->value();
	
	if(geo->pointsCount() == 0)
		return;

	// temporal fix to avoid segfault on Mac. (But reload every VBO at each frame)
	/*if(shouldUpdateGeoVBO){
		updateGeoVBO();
	}
	if(shouldUpdateColorVBO){
		updateColorVBO();
	}*/
	updateGeoVBO();
	updateColorVBO();

	glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_LINE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
	
	if(shouldDrawPoints){
		drawPoints(geo);
	}
	
	if(geo->facesCount()){
		if(shouldDrawSmooth && shouldDrawWireframe){
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.f, 1.f);
		}
		
		if(shouldDrawSmooth){
			drawSurface(geo, true);
		}
		
		if(shouldDrawFlat){
			drawSurface(geo, false);
		}
	
		if(shouldDrawWireframe || shouldDrawNormals){
			glDisable(GL_LIGHTING);
			glShadeModel(GL_FLAT);
	
			if(shouldDrawWireframe){
				drawWireframe(geo);
			}
			
			if(shouldDrawNormals){
				drawNormals(geo, shouldDrawFlat);
			}
		}
	}
	
	// if(shouldDrawIds){
	// 	drawPointIds(geo);
	// }
	
	glPopAttrib();
}
