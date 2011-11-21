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

#include "SplineNodes.h"
#include "../src/Numeric.h"
#include "../src/stringUtils.h"

using namespace coral;

SplinePoint::SplinePoint(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0),
	_degree(4){	
	_param = new NumericAttribute("param", this);
	_controlPoints = new NumericAttribute("controlPoints", this);
	_pointOnCurve = new NumericAttribute("pointsOnCurve", this);
	
	addInputAttribute(_param);
	addInputAttribute(_controlPoints);
	addOutputAttribute(_pointOnCurve);
	
	std::vector<std::string> paramSpecialization;
	paramSpecialization.push_back("Float");
	paramSpecialization.push_back("FloatArray");
	
	std::vector<std::string> pointsOnCurveSpecialization;
	pointsOnCurveSpecialization.push_back("Vec3");
	pointsOnCurveSpecialization.push_back("Vec3Array");
	
	setAttributeAllowedSpecializations(_param, paramSpecialization);
	setAttributeAllowedSpecialization(_controlPoints, "Vec3Array");
	setAttributeAllowedSpecializations(_pointOnCurve, pointsOnCurveSpecialization);
	
	addAttributeSpecializationLink(_param, _pointOnCurve);
	
	setAttributeAffect(_param, _pointOnCurve);
	setAttributeAffect(_controlPoints, _pointOnCurve);
}

void SplinePoint::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _param && attributeB == _pointOnCurve){
		if(specializationA.size() == 1){
			specializationB.resize(1);
			if(specializationA[0] == "Float"){
				specializationB[0] = "Vec3";
			}
			else if(specializationA[0] == "FloatArray"){
				specializationB[0] = "Vec3Array";
			}
		}
		else if(specializationB.size() == 1){
			specializationA.resize(1);
			if(specializationB[0] == "Vec3"){
				specializationA[0] = "Float";
			}
			else if(specializationB[0] == "Vec3Array"){
				specializationA[0] = "FloatArray";
			}
		}
	}
}

void SplinePoint::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;
	
	std::vector<std::string> specialization = attribute->specialization();
	if(specialization.size() == 1){
		if(stringUtils::endswith(specialization[0], "Array")){
			_selectedOperation = &SplinePoint::updateArray;
		}
		else{
			_selectedOperation = &SplinePoint::updateSingle;
		}
	}
}

float SplinePoint::basis(int i, int degree, float param, const std::vector<float> &knots){
	float ret = 0.0;
	
	if(degree > 0){
		float n1 = (param - knots[i]) * basis(i, degree - 1, param, knots);
		
		float d1 = knots[i + degree] - knots[i];
		float n2 = (knots[i + degree + 1] - param) * basis(i + 1, degree - 1, param, knots);
		float d2 = knots[i + degree + 1] - knots[i + 1];
		
		float a = 0.0;
		if(d1 > 0.0001 || d1 < - 0.0001){
			a = n1 / d1;
		}
		
		float b = 0.0;
		if(d2 > 0.0001 || d2 < - 0.0001){
			b = n2 / d2;
		}
		
		ret = a + b;
	}
	else{
		if(knots[0] <= param && param <= knots[i + 1]){
			ret = 1.0;
		}
	}
	
	return ret;
}

Imath::V3f SplinePoint::evaluatePointOnCurve(float param, int degree, const std::vector<Imath::V3f> &cvs, const std::vector<float> &knots){
	if(param < 0.0){
		param = 0.0;
	}
	else if(param > 1.0){
		param = 1.0;
	}
	
	float normalizedParam = param * abs(knots[0] - knots[knots.size() - 1]);
	
	Imath::V3f pos(0.0, 0.0, 0.0);
	float rational = 0.0;
	
	for(int i = 0; i < cvs.size(); ++i){
		float b = basis(i, degree, normalizedParam, knots);
		Imath::V3f p = cvs[i] * b;
		pos += p;
		rational += b;
	}
	
	pos *= (1.0 / (rational + 0.00001));
	
	return pos;
}

void SplinePoint::updateArray(){
	const std::vector<float> &params = _param->value()->floatValues();
	const std::vector<Imath::V3f> &cvs = _controlPoints->value()->vec3Values();
	
	int paramsSize = params.size();
	int cvsSize = cvs.size();
	
	std::vector<Imath::V3f> pointsOnCurve(paramsSize);
	
	for(int i = 0; i < paramsSize; ++i){
		pointsOnCurve[i] = evaluatePointOnCurve(params[i], _degree, cvs, _knots);
	}
	
	_pointOnCurve->outValue()->setVec3Values(pointsOnCurve);
}

void SplinePoint::updateSingle(){
	float param = _param->value()->floatValueAt(0);
	const std::vector<Imath::V3f> &cvs = _controlPoints->value()->vec3Values();
	
	Imath::V3f pointOnCurve = evaluatePointOnCurve(param, _degree, cvs, _knots);
	
	_pointOnCurve->outValue()->setVec3ValueAt(0, pointOnCurve);
}

void SplinePoint::updateKnots(){
	int ncvs = _controlPoints->value()->size();
	_degree = ncvs - 1;
	int nknots = ncvs + (_degree * 2) -2;
	_knots.clear();
	
	for(int i = 0; i < _degree; ++i){
		_knots.push_back(0.0);
	}
	
	float n = 0.0;
	int nspans = ncvs - _degree;
	for(int i = 0; i < nspans; ++i){
		_knots.push_back(n);
		n += 1.0;
	}
	
	for(int i = 0; i < (_degree + 1); ++i){
		_knots.push_back(n);
	}
}

void SplinePoint::update(Attribute *attribute){
	if(_selectedOperation){
		updateKnots();
		
		(this->*_selectedOperation)();
	}
}


