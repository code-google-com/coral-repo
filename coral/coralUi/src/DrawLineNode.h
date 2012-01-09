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

#ifndef CORALDRAWLINENODE_H
#define CORALDRAWLINENODE_H

#include <cstdio>
#include <string.h>

#include <coral/src/Node.h>
#include <coral/src/NumericAttribute.h>
#include "DrawNode.h"
#include "coralUiDefinitions.h"

#include <GL/glew.h>

namespace coralUi{

class CORALUI_EXPORT DrawLineNode : public DrawNode{
public:
	DrawLineNode(const std::string &name, coral::Node *parent);
	~DrawLineNode();
	void attributeConnectionChanged(coral::Attribute *attribute);
	void attributeDirtied(coral::Attribute *attribute);
	void draw();
	void initGL();
	void initShader();

private:
	coral::NumericAttribute *_points;
	coral::NumericAttribute *_thickness;
	coral::NumericAttribute *_colors;

	bool _shouldUpdatePointValues;
	bool _shouldUpdateColorValues;

	void updatePointValues();
	void updateColorValues();
	void drawLines();

	bool _useSingleColor;	// define if we use the color array store in VBO or the single color for everyone
	Imath::Color4f _singleColor;	// will only be used if _useSingleColor is true

	// OpenGL
	GLuint _pointBuffer;		// buffer of vertices: {0.35, 0.76, 0.48, 0.56, 0.37, etc...}
	GLuint _colorBuffer;		// buffer of color4: {0.5, 0.5, 0.5, 1.0, 0.5, 0.5, 0.5, 1.0, etc...}
	GLsizei _pointCount;

	GLuint _shaderProgram;	// the main shader program (only a vertex shader actually
	GLuint _pointIndexAttr;	// uniform indices
	GLuint _colorIndexAttr;
};

}

#endif
