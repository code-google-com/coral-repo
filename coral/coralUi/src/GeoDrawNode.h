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

#ifndef CORALGEODRAWNODE_H
#define CORALGEODRAWNODE_H

#include <cstdio>
#include <string.h>

#include <coral/src/Node.h>
#include <coral/src/Geo.h>
#include <coral/src/GeoAttribute.h>
#include <coral/src/BoolAttribute.h>
#include "DrawNode.h"
#include "coralUiDefinitions.h"

namespace coralUi{

class CORALUI_EXPORT GeoDrawNode : public DrawNode{
public:
	GeoDrawNode(const std::string &name, coral::Node *parent);
	void draw();

private:
	coral::GeoAttribute *_geo;
	coral::BoolAttribute *_smooth;
	coral::BoolAttribute *_flat;
	coral::BoolAttribute *_wireframe;
	coral::BoolAttribute *_points;
	coral::BoolAttribute *_normals;
	// coral::BoolAttribute *_ids;
	
	void drawNormals(coral::Geo *geo, bool shouldDrawFlat);
	void drawWireframe(coral::Geo *geo);
	void drawFlat(coral::Geo *geo);
	void drawSmooth(coral::Geo *geo);
	void drawPoints(coral::Geo *geo);
	// void drawPointIds(coral::Geo *geo);
};

}

#endif
