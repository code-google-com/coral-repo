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

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#if defined(WIN64) || defined(_WIN64) || defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#endif


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
}

void GeoDrawNode::drawPoints(Geo *geo){

	// Get attribute using this way at each draw?
	Numeric *numeric = _colors->value();
	Numeric::Type numType = numeric->type();
	const std::vector<Imath::Color4f> &col4Values = numeric->col4Values();

	if(numType == Numeric::numericTypeCol4){
		glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);
	}
	else{
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);	// default green
	}

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	
	const std::vector<Imath::V3f> &points = geo->points();
	
	glPointSize(5.f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)&points[0].x);

	GLfloat colArray[points.size()*4];	// this need to be outside of the if{} do exists during glDrawArrays()

	if(numType == Numeric::numericTypeCol4Array){

		int index = 0;
		// first step, put existing color in the array (color array can't be bigger than points.size() to avoid segfault)
		for(int i=0; i<col4Values.size() && i<points.size(); i++){
			colArray[index] = col4Values[i].r;
			colArray[index+1] = col4Values[i].g;
			colArray[index+2] = col4Values[i].b;
			colArray[index+3] = col4Values[i].a;
			index+=4;
		}

		// If more points than colors, use default green
		int diff = points.size() - col4Values.size();

		if(diff > 0){
			for(int i=0; i<diff; i++){
				colArray[index] = 0.0f;
				colArray[index+1] = 1.0f;
				colArray[index+2] = 0.0f;
				colArray[index+3] = 1.0f;
				index+=4;
			}
		}

		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, (GLvoid*)&colArray);
	}

	glDrawArrays(GL_POINTS, 0, points.size());

	if(numType == Numeric::numericTypeCol4Array){
		glColorPointer(4, GL_FLOAT, 0, NULL);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	glDisableClientState(GL_VERTEX_ARRAY);



}

void GeoDrawNode::drawSmooth(Geo *geo){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();
	const std::vector<Imath::V3f> &verticesNormals = geo->verticesNormals();
	
	Numeric *numeric = _colors->value();
	Numeric::Type numType = numeric->type();
	const std::vector<Imath::Color4f> &col4Values = numeric->col4Values();

	if(numType == Numeric::numericTypeCol4){
		glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);
	}else{
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);	// default green
	}

	if(verticesNormals.empty() == false){
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);

		if(numType == Numeric::numericTypeCol4 || numType == Numeric::numericTypeCol4Array){
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
			glEnable(GL_COLOR_MATERIAL);
		}

		int facesCount = (int)faces.size();

		for(int faceID = 0; faceID < facesCount; ++faceID){
			const std::vector<int> &face = faces[faceID];

			glBegin(GL_POLYGON);
			for(unsigned int index = 0; index < face.size(); ++index){
				int vertexID = face[index];
				if(numType == Numeric::numericTypeCol4Array){
					glColor4f(col4Values[vertexID].r, col4Values[vertexID].g, col4Values[vertexID].b, col4Values[vertexID].a);
				}
				glNormal3f(verticesNormals[vertexID].x, verticesNormals[vertexID].y, verticesNormals[vertexID].z);
				glVertex3f(points[vertexID].x, points[vertexID].y, points[vertexID].z);
			}
			glEnd();
		}
	}
}

void GeoDrawNode::drawFlat(Geo *geo){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();
	const std::vector<Imath::V3f> &faceNormals = geo->faceNormals();
	
	if(faceNormals.empty() == false){
		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);

		int facesCount = (int)faces.size();

		for(int faceID = 0; faceID < facesCount; ++faceID){
			const std::vector<int> &face = faces[faceID];

			const Imath::V3f &faceNormal = faceNormals[faceID];

			glBegin(GL_POLYGON);
			for(unsigned int index = 0; index < face.size(); ++index){

				glNormal3fv(&faceNormal.x);

				const Imath::V3f &point = points[face[index]];
				glVertex3fv(&point.x);
			}
			glEnd();
		}
	}
}

void GeoDrawNode::drawWireframe(Geo *geo){
	const std::vector<Imath::V3f> &points = geo->points();
	const std::vector<std::vector<int> > &faces = geo->rawFaces();
	int facesCount = (int)faces.size();
	
	glLineWidth(1.f);							// GL_LINE_BIT
	glColor3f(1.f, 1.f, 1.f);					// GL_CURRENT_BIT

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// GL_POLYGON_BIT

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)&points[0].x);

	for(int faceID = 0; faceID < facesCount; ++faceID){
		const std::vector<int> &face = faces[faceID];
		glDrawElements(GL_POLYGON, face.size(), GL_UNSIGNED_INT, (GLvoid*)&face[0]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
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

	bool shouldDrawSmooth = _smooth->value()->boolValueAt(0);
	bool shouldDrawFlat = _flat->value()->boolValueAt(0);
	bool shouldDrawWireframe = _wireframe->value()->boolValueAt(0);
	bool shouldDrawPoints = _points->value()->boolValueAt(0);
	bool shouldDrawNormals = _normals->value()->boolValueAt(0);
	// bool shouldDrawIds = _ids->value()->boolValueAt(0);
	
	Geo *geo = _geo->value();
	
	if(geo->pointsCount() == 0)
		return;
	
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
