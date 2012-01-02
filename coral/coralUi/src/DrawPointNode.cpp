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
#include "DrawPointNode.h"
#include <coral/src/Attribute.h>
#include <coral/src/Numeric.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


using namespace coral;
using namespace coralUi;

DrawPointNode::DrawPointNode(const std::string &name, Node *parent):
DrawNode(name, parent),
_shouldUpdatePointValues(true),
_shouldUpdateSizeValues(true),
_shouldUpdateColorValues(true),
_pointIndexAttr(0),
_colorIndexAttr(1),
_sizeIndexAttr(2){
	_points = new NumericAttribute("points", this);
	_sizes = new NumericAttribute("sizes", this);
	_colors = new NumericAttribute("colors", this);
	
	addInputAttribute(_points);
	addInputAttribute(_sizes);
	addInputAttribute(_colors);
	
	setAttributeAffect(_points, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_sizes, (Attribute*)viewportOutputAttribute());
	setAttributeAffect(_colors, (Attribute*)viewportOutputAttribute());
	
	std::vector<std::string> pointSpecializations;
	pointSpecializations.push_back("Vec3");
	pointSpecializations.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_points, pointSpecializations);

	std::vector<std::string> sizeSpecializations;
	sizeSpecializations.push_back("Float");
	sizeSpecializations.push_back("FloatArray");
	setAttributeAllowedSpecializations(_sizes, sizeSpecializations);

	std::vector<std::string> colorSpecializations;
	colorSpecializations.push_back("Col4");
	colorSpecializations.push_back("Col4Array");
	setAttributeAllowedSpecializations(_colors, colorSpecializations);

	if(glContextExists()){
		initGL();
	}
}

void DrawPointNode::initGL(){
	catchAttributeDirtied(_points);
	catchAttributeDirtied(_sizes);
	catchAttributeDirtied(_colors);

	// generate OpenGL buffers
	glGenBuffers(1, &_pointBuffer);
	glGenBuffers(1, &_sizeBuffer);
	glGenBuffers(1, &_colorBuffer);

	initShader();
}

DrawPointNode::~DrawPointNode(){
	if(glContextExists()){
		glDeleteBuffers(1, &_pointBuffer);
		glDeleteBuffers(1, &_sizeBuffer);
		glDeleteBuffers(1, &_colorBuffer);
	}
}

// TODO remove duplicate code
void DrawPointNode::attributeConnectionChanged(Attribute *attribute){
	if(attribute == _points){
		_shouldUpdatePointValues = true;
	}
	else if(attribute == _sizes){
		_shouldUpdateSizeValues = true;
	}
	else if(attribute == _colors){
		_shouldUpdateColorValues = true;
	}
}

void DrawPointNode::attributeDirtied(Attribute *attribute){
	if(attribute == _points){
		_shouldUpdatePointValues = true;
	}
	else if(attribute == _sizes){
		_shouldUpdateSizeValues = true;
	}
	else if(attribute == _colors){
		_shouldUpdateColorValues = true;
	}
}

void DrawPointNode::initShader(){

	// this permit to use per vertex point size in GLSL (gl_PointSize)
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// TODO gl_ModelViewProjectionMatrix is deprecated. We should send our own matrix: http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=283405
	// and http://stackoverflow.com/questions/4202456/how-do-you-get-the-modelview-and-projection-matrices-in-opengl
	std::string vertexShaderSource =
		"\
		uniform bool un_useSingleColor;\n\
		uniform bool un_useSingleSize;\n\
		uniform vec4 un_singleColor;\n\
		uniform float un_singleSize;\n\
		in vec3 in_Position;\n\
		in float in_Size;\n\
		in vec4 in_Color;\n\
		void main() {\n\
			if(un_useSingleColor){\n\
				gl_FrontColor = un_singleColor;\n\
			} else {\n\
				gl_FrontColor = in_Color;\n\
			}\n\
			if(un_useSingleSize){\n\
				gl_PointSize = un_singleSize;\n\
			} else {\n\
				gl_PointSize = in_Size;\n\
			}\n\
			gl_Position = gl_ModelViewProjectionMatrix * vec4(in_Position,1.0);\n\
		}";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	GLchar *glVertexShaderSource = (GLchar*)vertexShaderSource.data();
	glShaderSource(vertexShader, 1, (const GLchar**) &glVertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check compilation status
	int shaderStatus;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderStatus);
	if(!shaderStatus){
		std::cout << "error while compiling vertex shader" << std::endl;
	}

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);

	// prepare varing for future binding to VBOs
	glBindAttribLocation(_shaderProgram, _pointIndexAttr, "in_Position");
	glBindAttribLocation(_shaderProgram, _sizeIndexAttr, "in_Size");
	glBindAttribLocation(_shaderProgram, _colorIndexAttr, "in_Color");

	glLinkProgram(_shaderProgram);

	// check link status
	GLint linkStatus;
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkStatus);
	if(linkStatus == GL_FALSE){
		std::cout << "error while linking shader program" << std::endl;
	}
}

void DrawPointNode::updatePointValues(){

	Numeric *vec3Numeric = _points->value();
	const std::vector<Imath::V3f> &vec3Values = vec3Numeric->vec3Values();

	_pointCount = vec3Values.size();

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _pointBuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*_pointCount, (GLvoid*)&vec3Values[0].x, GL_STATIC_DRAW);

	// clean OpenGL states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawPointNode::updateSizeValues(){

	Numeric *sizeNumeric = _sizes->value();
	const std::vector<float> &sizeValues = sizeNumeric->floatValues();

	if((sizeNumeric->isArray()) && (sizeNumeric->size() > 0)){

		// we will not use default size
		_useSingleSize = false;

		// vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, _sizeBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*sizeValues.size(), (GLvoid*)&sizeValues[0], GL_STATIC_DRAW);

		// clean OpenGL states
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else {
		// size is not an array so we use a single one color
		_useSingleSize = true;

		// one size value? use it! No size? use default size
		if(sizeNumeric->size() == 1){
			_singleSize = sizeValues[0];
		}
		else {
			_singleSize = 5.0;
		}
	}
}

void DrawPointNode::updateColorValues(){
	Numeric *col4Numeric = _colors->value();
	const std::vector<Imath::Color4f> &col4Values = col4Numeric->col4Values();

	// color buffer
	if((col4Numeric->isArray()) && (col4Numeric->size() > 0)){

		// we will not use default color
		_useSingleColor = false;

		// avoid empty color (and maybe crashs)
		Numeric *vec3Numeric = _points->value();
		const std::vector<Imath::V3f> &vec3Values = vec3Numeric->vec3Values();
		int pointCount = (int)vec3Values.size();
		int colorCount = (int)col4Values.size();

		glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4*sizeof(GLfloat)*pointCount, (GLvoid*)&col4Values[0].r, GL_STATIC_DRAW);

		if(colorCount < pointCount){
			int emptyColCount = pointCount - colorCount;	// get the number of empty color to create in the buffer to match the number of vertex
			
			// create an array to feed
			std::vector<Imath::Color4f> emptyColArray;
			emptyColArray.reserve(emptyColCount);

			for(int i = 0; i<emptyColCount; ++i){
				emptyColArray.push_back(Imath::Color4f(0.0, 1.0, 0.0, 1.0));
			}

			GLintptr offset = 4*sizeof(GLfloat)*colorCount;
			GLsizeiptr size = 4*sizeof(GLfloat)*emptyColCount;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, (GLvoid*)&emptyColArray[0].r);

		}

		// clean OpenGL state
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else {
		// color is not an array so we use a single one color
		_useSingleColor = true;

		// one col4? use it! No color? use default green
		if(col4Numeric->size() == 1){
			_singleColor = col4Values[0];
		}
		else {
			_singleColor = Imath::Color4f(0.0, 1.0, 0.0, 1.0);
		}
	}
}

void DrawPointNode::drawPoints(){

	// prepare OpenGL rendering
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	glPointSize(5.f);
	glColor3f(1.f, 1.f, 0.f);

	glUseProgram(_shaderProgram);

	// set uniform values (can only be done after glUseProgram)
	GLint locUseSingleColor = glGetUniformLocation(_shaderProgram, "un_useSingleColor");
	glUniform1i(locUseSingleColor, _useSingleColor);

	GLint locSingleColor = glGetUniformLocation(_shaderProgram, "un_singleColor");
	glUniform4f(locSingleColor, _singleColor.r, _singleColor.g, _singleColor.b, _singleColor.a);

	GLint locUseSingleSize = glGetUniformLocation(_shaderProgram, "un_useSingleSize");
	glUniform1i(locUseSingleSize, _useSingleSize);

	GLint locSingleSize = glGetUniformLocation(_shaderProgram, "un_singleSize");
	glUniform1f(locSingleSize, _singleSize);

	// set VBOs
	glBindBuffer(GL_ARRAY_BUFFER, _pointBuffer);
	glVertexAttribPointer(_pointIndexAttr, 3, GL_FLOAT, 0, 0, NULL);
	glEnableVertexAttribArray(_pointIndexAttr);

	glBindBuffer(GL_ARRAY_BUFFER, _sizeBuffer);
	glVertexAttribPointer(_sizeIndexAttr, 1, GL_FLOAT, 0, 0, NULL);
	glEnableVertexAttribArray(_sizeIndexAttr);

	glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
	glVertexAttribPointer(_colorIndexAttr, 4, GL_FLOAT, 0, 0, NULL);
	glEnableVertexAttribArray(_colorIndexAttr);

	// render
	glDrawArrays(GL_POINTS, 0, _pointCount);

	// clean OpenGL statement
	glDisableVertexAttribArray(_pointIndexAttr);
	glDisableVertexAttribArray(_sizeIndexAttr);
	glDisableVertexAttribArray(_colorIndexAttr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
}

void DrawPointNode::draw(){
	DrawNode::draw();
	
	Numeric *vec3Numeric = _points->value();
	const std::vector<Imath::V3f> &vec3Values = vec3Numeric->vec3Values();
	
	if(vec3Values.size() == 0)
		return;

	if(_shouldUpdatePointValues){
		updatePointValues();
		_shouldUpdatePointValues = false;
	}
	
	if(_shouldUpdateSizeValues){
		updateSizeValues();
		_shouldUpdateSizeValues = false;
	}

	if(_shouldUpdateColorValues){
		updateColorValues();
		_shouldUpdateColorValues = false;
	}
	
	drawPoints();
}
