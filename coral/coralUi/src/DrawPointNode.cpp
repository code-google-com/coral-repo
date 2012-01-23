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
_pointIndexAttr(-1),
_colorIndexAttr(-1),
_sizeIndexAttr(-1),
_pointCount(0){
	_points = new NumericAttribute("points", this);
	_sizes = new NumericAttribute("sizes", this);
	_colors = new NumericAttribute("colors", this);
	
	addInputAttribute(_points);
	addInputAttribute(_sizes);
	addInputAttribute(_colors);
	
	// setAttributeAffect(_points, (Attribute*)viewportOutputAttribute());
	// setAttributeAffect(_sizes, (Attribute*)viewportOutputAttribute());
	// setAttributeAffect(_colors, (Attribute*)viewportOutputAttribute());
	
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

		_shouldUpdateSizeValues = true;	// if the number of point as changed, we need to update the number of elements in buffer too
		_shouldUpdateColorValues = true;
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

		_shouldUpdateSizeValues = true;	// if the number of point as changed, we need to update the number of elements in buffer too
		_shouldUpdateColorValues = true;
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
		"#version 120\n\
		attribute vec3 in_Position;\n\
		attribute float in_Size;\n\
		attribute vec4 in_Color;\n\
		void main() {\n\
			gl_FrontColor = in_Color;\n\
			gl_PointSize = in_Size;\n\
			gl_Position = gl_ModelViewProjectionMatrix * vec4(in_Position,1.0);\n\
		}";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	GLchar *glVertexShaderSource = (GLchar*)vertexShaderSource.data();
	glShaderSource(vertexShader, 1, (const GLchar**) &glVertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check compilation status
	int shaderStatus;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderStatus);

	// output compilation error if one
	if(shaderStatus == GL_FALSE){
		std::cout << "error while compiling vertex shader" << std::endl;

		GLsizei logSize = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logSize);

		char *log = new char[logSize];
		if(log == NULL)
		{
			std::cout << "impossible to allocate memory for shader compilation log" << std::endl;
		}

		memset(log, '\0', logSize + 1);

		glGetShaderInfoLog(vertexShader, logSize, &logSize, log);
		std::cout << "shader compilation log:" << std::endl << log << std::endl;
	}

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);

	glLinkProgram(_shaderProgram);

	// check link status
	GLint linkStatus;
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkStatus);
	if(linkStatus == GL_FALSE){
		std::cout << "error while linking shader program" << std::endl;
	}

	// get attribute location
	_pointIndexAttr = glGetAttribLocation(_shaderProgram, "in_Position");
	_sizeIndexAttr = glGetAttribLocation(_shaderProgram, "in_Size");
	_colorIndexAttr = glGetAttribLocation(_shaderProgram, "in_Color");
}

void DrawPointNode::updatePointValues(){

	Numeric *vec3Numeric = _points->value();
	const std::vector<Imath::V3f> &vec3Values = vec3Numeric->vec3Values();

	// search if a new allocation for points is needed (if the number of point have changed)
	_newPointCount = true;
	if(_pointCount == vec3Values.size()){
		_newPointCount = false;
	}

	_pointCount = vec3Values.size();

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, _pointBuffer);
	if(_newPointCount){
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*_pointCount, (GLvoid*)&vec3Values[0].x, GL_STATIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(GLfloat)*_pointCount, (GLvoid*)&vec3Values[0].x);
	}


	// clean OpenGL states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawPointNode::updateSizeValues(){

	Numeric *sizeNumeric = _sizes->value();
	const std::vector<float> &sizeValues = sizeNumeric->floatValues();

	// create the default size value.
	GLfloat defaultSize = 5.0;
	if(sizeNumeric->size() == 1){
		// if only one float is connected, we use it as default size
		defaultSize = sizeValues[0];
	}

	int sizeCount = (int)sizeNumeric->size();

	glBindBuffer(GL_ARRAY_BUFFER, _sizeBuffer);
	if(_newPointCount){
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*_pointCount, (GLvoid*)&sizeValues[0], GL_STATIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*_pointCount, (GLvoid*)&sizeValues[0]);
	}

	if(sizeCount < _pointCount){
		int emptySizeCount = _pointCount - sizeCount;	// get the number of empty color to create in the buffer to match the number of vertex

		// create an array to feed
		std::vector<GLfloat> emptySizeArray;
		emptySizeArray.resize(emptySizeCount, defaultSize);

		GLintptr offset = sizeof(GLfloat)*sizeCount;
		GLsizeiptr size = sizeof(GLfloat)*emptySizeCount;
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, (GLvoid*)&emptySizeArray[0]);
	}

	// clean OpenGL state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawPointNode::updateColorValues(){
	Numeric *col4Numeric = _colors->value();
	const std::vector<Imath::Color4f> &col4Values = col4Numeric->col4Values();

	// create the default color.
	Imath::Color4f defaultColor = Imath::Color4f(0.0, 1.0, 0.0, 1.0);
	if(col4Numeric->size() == 1){
		// if only one col4 is connected, we use it as default color
		defaultColor = col4Values[0];
	}

	int colorCount = (int)col4Numeric->size();

	glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
	if(_newPointCount){
		glBufferData(GL_ARRAY_BUFFER, 4*sizeof(GLfloat)*_pointCount, (GLvoid*)&col4Values[0].r, GL_STATIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(GLfloat)*_pointCount, (GLvoid*)&col4Values[0].r);
	}

	if(colorCount < _pointCount){
		int emptyColCount = _pointCount - colorCount;	// get the number of empty color to create in the buffer to match the number of vertex

		// create an array and feed it with default color
		std::vector<Imath::Color4f> emptyColArray;
		emptyColArray.resize(emptyColCount, defaultColor);

		GLintptr offset = 4*sizeof(GLfloat)*colorCount;
		GLsizeiptr size = 4*sizeof(GLfloat)*emptyColCount;
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, (GLvoid*)&emptyColArray[0].r);

	}

	// clean OpenGL state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawPointNode::drawPoints(){

	// prepare OpenGL rendering
	/*glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	glPointSize(5.f);
	glColor3f(1.f, 1.f, 0.f);*/

	glUseProgram(_shaderProgram);

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
	
	// no input connection for points, no need to render
	Numeric *vec3Numeric = _points->value();
	if(vec3Numeric->size() == 0)
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
	
	// Once every update is supposed to be done, we consider the number of point is not new
	_newPointCount = false;

	drawPoints();
}
