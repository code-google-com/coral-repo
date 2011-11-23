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
#include <coral/src/Numeric.h>

#include <GL/glew.h>
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
	
	_smooth->outValue()->setBoolValueAt(0, true);

	std::vector<std::string> colorSpecializations;
	colorSpecializations.push_back("Col4");
	colorSpecializations.push_back("Col4Array");
	setAttributeAllowedSpecializations(_colors, colorSpecializations);

	// generate OpenGL buffers
	glGenBuffers(1, &_vtxBuffer);
	glGenBuffers(1, &_nrmBuffer);
	glGenBuffers(1, &_idxBuffer);
}

GeoDrawNode::~GeoDrawNode(){
	glDeleteBuffers(1, &_vtxBuffer);
	glDeleteBuffers(1, &_nrmBuffer);
	glDeleteBuffers(1, &_idxBuffer);
}

void GeoDrawNode::updateVBO(Geo *geo){

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
	int facesCount = (int)faces.size();

	// count the total number of index element
	int idxCount = 0;
	for(int faceID = 0; faceID < facesCount; ++faceID){
		const std::vector<int> &face = faces[faceID];
		idxCount += face.size();
	}

	// create and feed the array of index
	GLuint quadIndices[idxCount];

	int idxOffset = 0;
	for(int faceID = 0; faceID < facesCount; ++faceID){

		const std::vector<int> &face = faces[faceID];

		for(unsigned int index = 0; index < face.size(); ++index){

			quadIndices[idxOffset] = face[index];
			idxOffset++;
		}
	}

	// send to GPU!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);


	// clean OpenGL states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEndList();
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

void GeoDrawNode::drawSmooth(Geo *geo){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();
	const std::vector<Imath::V3f> &vtxNormals = geo->verticesNormals();

	Numeric *numeric = _colors->value();
	Numeric::Type numType = numeric->type();
	const std::vector<Imath::Color4f> &col4Values = numeric->col4Values();

	if(numType == Numeric::numericTypeCol4){
		glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);
	}else{
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);	// default green
	}

	if(numType == Numeric::numericTypeCol4){
		glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);
	}else{
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);	// default green
	}

	if(vtxNormals.empty() == false){
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);

		// prepare VBOs
		glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
		glVertexPointer(3, GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, _nrmBuffer);
		glNormalPointer(GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idxBuffer);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		// render
		// draw polygon sending poly size (3 or 4+) and an offset in the GPU memory to the indice to use
		int facesCount = (int)faces.size();
		int idOffset = 0;
		for(int faceID = 0; faceID < facesCount; ++faceID){

			const std::vector<int> &face = faces[faceID];
			glDrawElements(GL_POLYGON, face.size(), GL_UNSIGNED_INT, (GLuint*)NULL+idOffset);
			idOffset += face.size();
		}

		// clean OpenGL states
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void GeoDrawNode::drawFlat(Geo *geo){
	
		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);

		// prepare VBOs
		glBindBuffer(GL_ARRAY_BUFFER, _vtxBuffer);
		glVertexPointer(3, GL_FLOAT, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, _nrmBuffer);
		glNormalPointer(GL_FLOAT, 0, NULL);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		// render tri
		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _triIdxBuffer);

		glDrawElements(GL_TRIANGLES, _triNumber*3, GL_UNSIGNED_INT, NULL);

		// render quads
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadIdxBuffer);

		glDrawElements(GL_QUADS, _quadNumber*4, GL_UNSIGNED_INT, NULL);*/

		// clean OpenGL states
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

	bool shouldUpdateVBO = false;
	// check something is connected in _geo and check it is resolved
	if(_geo->input()){
		if(_geo->input()->isClean() == false){
			shouldUpdateVBO = true;
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

	if(shouldUpdateVBO){
		std::cout << "update VBO" << std::endl;
		updateVBO(geo);
	}

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
			drawSmooth(geo);
		}
		
		if(shouldDrawFlat){
			drawFlat(geo);
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
