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
#include "NumericDrawNode.h"
#include <coral/src/Numeric.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#if defined(WIN64) || defined(_WIN64) || defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#endif
#include <GL/gl.h>
#endif


using namespace coral;
using namespace coralUi;

NumericDrawNode::NumericDrawNode(const std::string &name, Node *parent): DrawNode(name, parent){	
	_numeric = new NumericAttribute("numeric", this);
	
	addInputAttribute(_numeric);
	
	setAttributeAffect(_numeric, (Attribute*)viewportOutputAttribute());
}

void NumericDrawNode::drawMatrix44(Numeric *numeric){
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	
	std::vector<Imath::M44f> matrixValues = numeric->matrix44Values();

	float displayScale = 5.0;
	
	for(int i = 0; i < matrixValues.size(); ++i){
		Imath::M44f *currentMatrix = &matrixValues[i];
		
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(currentMatrix->x[3][0], currentMatrix->x[3][1], currentMatrix->x[3][2]);
		glVertex3f(
				currentMatrix->x[3][0] + currentMatrix->x[0][0] * displayScale, 
				currentMatrix->x[3][1] + currentMatrix->x[0][1] * displayScale,  
				currentMatrix->x[3][2] + currentMatrix->x[0][2] * displayScale);
		glEnd();
		
		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
		glVertex3f(currentMatrix->x[3][0], currentMatrix->x[3][1], currentMatrix->x[3][2]);
		glVertex3f(
				currentMatrix->x[3][0] + currentMatrix->x[1][0] * displayScale, 
				currentMatrix->x[3][1] + currentMatrix->x[1][1] * displayScale,  
				currentMatrix->x[3][2] + currentMatrix->x[1][2] * displayScale);
		glEnd();
		
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(currentMatrix->x[3][0], currentMatrix->x[3][1], currentMatrix->x[3][2]);
		glVertex3f(
				currentMatrix->x[3][0] + currentMatrix->x[2][0] * displayScale, 
				currentMatrix->x[3][1] + currentMatrix->x[2][1] * displayScale,  
				currentMatrix->x[3][2] + currentMatrix->x[2][2] * displayScale);
		glEnd();
	}
	
	glEnable(GL_LIGHTING);
}

void NumericDrawNode::drawVec3(Numeric *numeric){
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	
	std::vector<Imath::V3f> vec3Values = numeric->vec3Values();
	
	glPointSize(5.f);
	glColor3f(0.f, 1.f, 0.f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)&vec3Values[0].x);
	glDrawArrays(GL_POINTS, 0, vec3Values.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glEnable(GL_LIGHTING);
}

void NumericDrawNode::drawCol4(Numeric *numeric){
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	std::vector<Imath::Color4f> col4Values = numeric->col4Values();

	glPointSize(50.f);
	//glColor4f(col4Values[0].r, col4Values[0].g, col4Values[0].b, col4Values[0].a);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*4, (GLvoid*)&col4Values[0].r);
	glColorPointer(4, GL_FLOAT, 0, (GLvoid*)&col4Values[0].r);
	glDrawArrays(GL_POINTS, 0, col4Values.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glEnable(GL_LIGHTING);
}

void NumericDrawNode::draw(){
	DrawNode::draw();
	
	Numeric *numeric = _numeric->value();
	Numeric::Type type = numeric->type();
	if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
		drawVec3(numeric);
	}
	else if(type == Numeric::numericTypeCol4 || type == Numeric::numericTypeCol4Array){
		drawCol4(numeric);
	}
	else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
		drawMatrix44(numeric);
	}
}
