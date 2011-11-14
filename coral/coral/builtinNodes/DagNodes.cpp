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

	_rotateOrder = new EnumAttribute("rotateOrder", this);
	_share = new NumericAttribute("share", this);
	_rotatePivot = new NumericAttribute("rotatePivot", this);
	_rotatePivotTranslate = new NumericAttribute("rotatePivotTranslate", this);
	_scalePivot = new NumericAttribute("scalePivot", this);
	_scalePivotTranslate = new NumericAttribute("scalePivotTranslate", this);
	_rotateAxis = new NumericAttribute("rotateAxis", this);

	_local = new NumericAttribute("matrix", this);
	_outWorld = new NumericAttribute("worldMatrix", this);

	_ingeo = new GeoAttribute("inGeo", this);
	_outgeo = new GeoAttribute("outGeo", this);

	addInputAttribute(_parent);
	addInputAttribute(_translate);
	addInputAttribute(_rotate);
	addInputAttribute(_scale);
	addInputAttribute(_rotateOrder);
	addInputAttribute(_share);
	addInputAttribute(_rotatePivot);
	addInputAttribute(_rotatePivotTranslate);
	addInputAttribute(_scalePivot);
	addInputAttribute(_scalePivotTranslate);
	addInputAttribute(_rotateAxis);
	addInputAttribute(_ingeo);

	addOutputAttribute(_local);
	addOutputAttribute(_outWorld);
	addOutputAttribute(_outgeo);

	setAttributeAffect(_ingeo, _outgeo);

	setAttributeAffect(_parent, _outWorld);
	setAttributeAffect(_translate, _outWorld);
	setAttributeAffect(_rotate, _outWorld);
	setAttributeAffect(_scale, _outWorld);
	setAttributeAffect(_rotateOrder, _outWorld);
	setAttributeAffect(_share, _outWorld);
	setAttributeAffect(_rotatePivot, _outWorld);
	setAttributeAffect(_rotatePivotTranslate, _outWorld);
	setAttributeAffect(_scalePivot, _outWorld);
	setAttributeAffect(_scalePivotTranslate, _outWorld);
	setAttributeAffect(_rotateAxis, _outWorld);

	setAttributeAffect(_parent, _outgeo);
	setAttributeAffect(_translate, _outgeo);
	setAttributeAffect(_rotate, _outgeo);
	setAttributeAffect(_scale, _outgeo);
	setAttributeAffect(_rotateOrder, _outgeo);
	setAttributeAffect(_share, _outgeo);
	setAttributeAffect(_rotatePivot, _outgeo);
	setAttributeAffect(_rotatePivotTranslate, _outgeo);
	setAttributeAffect(_scalePivot, _outgeo);
	setAttributeAffect(_scalePivotTranslate, _outgeo);
	setAttributeAffect(_rotateAxis, _outgeo);

	setAttributeAffect(_translate, _local);
	setAttributeAffect(_rotate, _local);
	setAttributeAffect(_scale, _local);
	setAttributeAffect(_rotateOrder, _local);
	setAttributeAffect(_share, _local);
	setAttributeAffect(_rotatePivot, _local);
	setAttributeAffect(_rotatePivotTranslate, _local);
	setAttributeAffect(_scalePivot, _local);
	setAttributeAffect(_scalePivotTranslate, _local);
	setAttributeAffect(_rotateAxis, _local);

	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Vec3");

	setAttributeAllowedSpecializations(_translate, vectorSpecializations);
	setAttributeAllowedSpecializations(_rotate, vectorSpecializations);
	setAttributeAllowedSpecializations(_scale, vectorSpecializations);

	setAttributeAllowedSpecializations(_share, vectorSpecializations);
	setAttributeAllowedSpecializations(_rotatePivot, vectorSpecializations);
	setAttributeAllowedSpecializations(_rotatePivotTranslate, vectorSpecializations);
	setAttributeAllowedSpecializations(_scalePivot, vectorSpecializations);
	setAttributeAllowedSpecializations(_scalePivotTranslate, vectorSpecializations);
	setAttributeAllowedSpecializations(_rotateAxis, vectorSpecializations);

	std::vector<std::string> mtxSpecializations;
	mtxSpecializations.push_back("Matrix44");

	setAttributeAllowedSpecializations(_parent, mtxSpecializations);
	setAttributeAllowedSpecializations(_local, mtxSpecializations);
	setAttributeAllowedSpecializations(_outWorld, mtxSpecializations);

	_scale->outValue()->setVec3ValueAt(0,Imath::V3f(1.0,1.0,1.0));

	Enum *rOrder = _rotateOrder->outValue();
	rOrder->addEntry(0, "xyz");
	rOrder->addEntry(1, "yzx");
	rOrder->addEntry(2, "zxy");
	rOrder->addEntry(3, "xzy");
	rOrder->addEntry(4, "yxz");
	rOrder->addEntry(5, "zyx");

}


void Transform::update(Attribute *attribute){
	std::vector<Imath::M44f> parentValues = _parent->value()->matrix44Values();
	std::vector<Imath::V3f> translate = _translate->value()->vec3Values();
	std::vector<Imath::V3f> rotate = _rotate->value()->vec3Values();
	std::vector<Imath::V3f> scale = _scale->value()->vec3Values();
	std::vector<Imath::V3f> share = _share->value()->vec3Values();
	std::vector<Imath::V3f> rotatePivot = _rotatePivot->value()->vec3Values();
	std::vector<Imath::V3f> rotatePivotTranslate = _rotatePivotTranslate->value()->vec3Values();
	std::vector<Imath::V3f> scalePivot = _scalePivot->value()->vec3Values();
	std::vector<Imath::V3f> scalePivotTranslate = _scalePivotTranslate->value()->vec3Values();
	std::vector<Imath::V3f> rotateAxis = _rotateAxis->value()->vec3Values();
	int rotateOrder = _rotateOrder->value()->currentIndex();
	Geo *geo = _ingeo->value();

	std::vector<Imath::M44f> localValues(1);
	std::vector<Imath::M44f> wolrdValues(1);

	Imath::M44f shareMtx;
	shareMtx.setShear(share[0]);

	Imath::M44f rotatePivotMtx;
	rotatePivotMtx.setTranslation(rotatePivot[0]);

	Imath::M44f rotatePivotTranslateMtx;
	rotatePivotTranslateMtx.setTranslation(rotatePivotTranslate[0]);

	Imath::M44f scalePivotMtx;
	scalePivotMtx.setTranslation(scalePivot[0]);

	Imath::M44f scalePivotTranslateMtx;
	scalePivotTranslateMtx.setTranslation(scalePivotTranslate[0]);

	Imath::M44f rotateAxisMtx;
	Imath::V3f rotAxisVec(rotateAxis[0].x*M_PI/180.0,rotateAxis[0].y*M_PI/180.0,rotateAxis[0].z*M_PI/180.0);
	rotateAxisMtx.setEulerAngles(rotAxisVec);

	Imath::M44f translateMtx;
	translateMtx.setTranslation(translate[0]);

	Imath::M44f rotateX;
	rotateX[1][1] = cos(rotate[0].x*M_PI/180.0);
	rotateX[2][2] = cos(rotate[0].x*M_PI/180.0);
	rotateX[1][2] = sin(rotate[0].x*M_PI/180.0);
	rotateX[2][1] = -sin(rotate[0].x*M_PI/180.0);

	Imath::M44f rotateY;
	rotateX[0][0] = cos(rotate[0].y*M_PI/180.0);
	rotateX[2][2] = cos(rotate[0].y*M_PI/180.0);
	rotateX[0][2] = -sin(rotate[0].y*M_PI/180.0);
	rotateX[2][0] = sin(rotate[0].y*M_PI/180.0);


	Imath::M44f rotateZ;
	rotateX[0][0] = cos(rotate[0].z*M_PI/180.0);
	rotateX[1][1] = cos(rotate[0].z*M_PI/180.0);
	rotateX[0][1] = sin(rotate[0].z*M_PI/180.0);
	rotateX[1][0] = -sin(rotate[0].z*M_PI/180.0);


	Imath::M44f rotateMtx;
	switch(rotateOrder)
	{
		case 0:
			rotateMtx = rotateX*rotateY*rotateZ;
			break;
		case 1:
			rotateMtx = rotateY*rotateZ*rotateX;
			break;
		case 2:
			rotateMtx = rotateZ*rotateX*rotateY;
			break;
		case 3:
			rotateMtx = rotateX*rotateZ*rotateY;
			break;
		case 4:
			rotateMtx = rotateY*rotateX*rotateZ;
			break;
		case 5:
			rotateMtx = rotateZ*rotateY*rotateX;
			break;
		default:
			rotateMtx = rotateX*rotateY*rotateZ;
	}

	Imath::M44f scaleMtx;
	scaleMtx.setScale(scale[0]);

	localValues[0] = scalePivotMtx.inverse()*scaleMtx*shareMtx*scalePivotMtx*scalePivotTranslateMtx*
					rotatePivotMtx.inverse()*rotateAxisMtx*rotateMtx*rotatePivotMtx*rotatePivotTranslateMtx*translateMtx;
	wolrdValues[0] = parentValues[0]*localValues[0];

	_local->outValue()->setMatrix44Values(localValues);
	_outWorld->outValue()->setMatrix44Values(wolrdValues);

	Geo *outGeoValue = _outgeo->outValue();

	outGeoValue->copy(geo);

	const std::vector<Imath::V3f> &pts = geo->points();
	if(pts.size()>0)
	{
		std::vector<Imath::V3f> outPts(pts);

#ifdef CORAL_OPENMP
		#pragma omp parallel for
#endif
		for(int i=0;i<outPts.size();++i)
		{
			outPts[i] = pts[i]*wolrdValues[0];
		}

		outGeoValue->displacePoints(outPts);
	}

}
