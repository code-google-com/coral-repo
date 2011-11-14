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

#include "DagNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/mathUtils.h"

#include <ImathVec.h>
#include <ImathMatrix.h>
#include <ImathQuat.h>

using namespace coral;

Transform::Transform(const std::string &name, Node *parent): Node(name, parent){
	_parent = new NumericAttribute("parentMatrix", this);
	_translate = new NumericAttribute("translate", this);
	_rotate = new NumericAttribute("rotate", this);
	_scale = new NumericAttribute("scale", this);

	_local = new NumericAttribute("matrix", this);
	_outWorld = new NumericAttribute("worldMatrix", this);

	addInputAttribute(_parent);
	addInputAttribute(_translate);
	addInputAttribute(_rotate);
	addInputAttribute(_scale);
	addOutputAttribute(_local);
	addOutputAttribute(_outWorld);

	setAttributeAffect(_parent, _outWorld);
	setAttributeAffect(_translate, _outWorld);
	setAttributeAffect(_rotate, _outWorld);
	setAttributeAffect(_scale, _outWorld);

	setAttributeAffect(_translate, _local);
	setAttributeAffect(_rotate, _local);
	setAttributeAffect(_scale, _local);

	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Vec3");

	setAttributeAllowedSpecializations(_translate, vectorSpecializations);
	setAttributeAllowedSpecializations(_rotate, vectorSpecializations);
	setAttributeAllowedSpecializations(_scale, vectorSpecializations);

	std::vector<std::string> mtxSpecializations;
	mtxSpecializations.push_back("Matrix44");

	setAttributeAllowedSpecializations(_parent, mtxSpecializations);
	setAttributeAllowedSpecializations(_local, mtxSpecializations);
	setAttributeAllowedSpecializations(_outWorld, mtxSpecializations);

	addAttributeSpecializationLink(_parent, _local);
}


void Transform::update(Attribute *attribute){
	std::vector<Imath::M44f> parentValues = _parent->value()->matrix44Values();

	std::vector<Imath::M44f> localValues(1);


	_local->outValue()->setMatrix44Values(localValues);
	_outWorld->outValue()->setMatrix44Values(localValues);
}
