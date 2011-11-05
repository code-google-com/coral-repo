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

#include "MathNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/mathUtils.h"

#include <ImathVec.h>
#include <ImathMatrix.h>

using namespace coral;

Vec3Length::Vec3Length(const std::string &name, Node *parent): Node(name, parent){
	_vector = new NumericAttribute("vector", this);
	_length = new NumericAttribute("length", this);
	
	addInputAttribute(_vector);
	addOutputAttribute(_length);
	
	setAttributeAffect(_vector, _length);
	
	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Vec3");
	vectorSpecializations.push_back("Vec3Array");
	
	setAttributeAllowedSpecializations(_vector, vectorSpecializations);
	
	std::vector<std::string> lengthSpecializations;
	lengthSpecializations.push_back("Float");
	lengthSpecializations.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_length, lengthSpecializations);
	
	addAttributeSpecializationLink(_vector, _length);
}

void Vec3Length::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _vector && attributeB == _length){
		std::vector<std::string> vectorSpecializations = _vector->allowedSpecialization();
		std::vector<std::string> lengthSpecializations = _length->allowedSpecialization();
	
		if(specializationA.size() == 1){
			specializationB.clear();
			
			if(specializationA[0] == "Vec3"){
				specializationB.push_back(lengthSpecializations[0]);
			}
			else if(specializationA[0] == "Vec3Array"){
				specializationB.push_back(lengthSpecializations[1]);
			}
		}
		else if(specializationB.size() == 1){
			specializationA.clear();
			
			if(specializationB[0] == "Float"){
				specializationA.push_back(vectorSpecializations[0]);
			}
			else if(specializationB[0] == "FloatArray"){
				specializationA.push_back(vectorSpecializations[1]);
			}
		}
	}
}

void Vec3Length::update(Attribute *attribute){
	std::vector<Imath::V3f> vectorValues = _vector->value()->vec3Values();
	unsigned int vectorValuesSize = vectorValues.size();
	
	std::vector<float> lengthValues(vectorValuesSize);
	
	for(int i = 0; i < vectorValuesSize; ++i){
		lengthValues[i] = vectorValues[i].length();
	}
	
	_length->outValue()->setFloatValues(lengthValues);
}

Matrix44Inverse::Matrix44Inverse(const std::string &name, Node *parent): Node(name, parent){	
	_inMatrix = new NumericAttribute("inMatrix", this);
	_outMatrix = new NumericAttribute("outMatrix", this);
	
	addInputAttribute(_inMatrix);
	addOutputAttribute(_outMatrix);
	
	setAttributeAffect(_inMatrix, _outMatrix);
	
	std::vector<std::string> specialization;
	specialization.push_back("Matrix44");
	specialization.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_inMatrix, specialization);
	setAttributeAllowedSpecializations(_outMatrix, specialization);
	
	addAttributeSpecializationLink(_inMatrix, _outMatrix);
}

void Matrix44Inverse::update(Attribute *attribute){
	std::vector<Imath::M44f> inMatrixValues = _inMatrix->value()->matrix44Values();
	
	std::vector<Imath::M44f> outMatrixValues(inMatrixValues.size());
	for(int i = 0; i < inMatrixValues.size() ;++i){
		outMatrixValues[i] = inMatrixValues[i].inverse();
	}
	
	_outMatrix->outValue()->setMatrix44Values(outMatrixValues);
}

Abs::Abs(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0){
	
	_inNumber = new NumericAttribute("inNumber", this);
	_outNumber = new NumericAttribute("ouNumber", this);
	
	addInputAttribute(_inNumber);
	addOutputAttribute(_outNumber);
	
	setAttributeAffect(_inNumber, _outNumber);
	
	std::vector<std::string> specialization;
	specialization.push_back("Int");
	specialization.push_back("IntArray");
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_inNumber, specialization);
	setAttributeAllowedSpecializations(_outNumber, specialization);
	
	addAttributeSpecializationLink(_inNumber, _outNumber);
}

void Abs::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;

	Numeric::Type type = _inNumber->outValue()->type();
	
	if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
		_selectedOperation = &Abs::abs_int;
	}
	else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
		_selectedOperation = &Abs::abs_float;
	}
}

void Abs::abs_int(Numeric *inNumber, Numeric *outNumber){
	std::vector<int> inValues = inNumber->intValues();
	std::vector<int> outValues(inValues.size());
	
	for(int i = 0; i < inValues.size(); ++i){
		outValues[i] = abs(inValues[i]);
	}
	
	outNumber->setIntValues(outValues);
}

void Abs::abs_float(Numeric *inNumber, Numeric *outNumber){
	std::vector<float> inValues = inNumber->floatValues();
	std::vector<float> outValues(inValues.size());
	
	for(int i = 0; i < inValues.size(); ++i){
		outValues[i] = fabs(inValues[i]);
	}
	
	outNumber->setFloatValues(outValues);
}

void Abs::update(Attribute *attribute){
	if(_selectedOperation){
		(this->*_selectedOperation)(_inNumber->value(), _outNumber->outValue());
	}
}

Vec3Cross::Vec3Cross(const std::string &name, Node *parent): Node(name, parent){	
	_vector0 = new NumericAttribute("vector0", this);
	_vector1 = new NumericAttribute("vector1", this);
	_crossProduct = new NumericAttribute("crossProduct", this);
	
	addInputAttribute(_vector0);
	addInputAttribute(_vector1);
	addOutputAttribute(_crossProduct);
	
	setAttributeAffect(_vector0, _crossProduct);
	setAttributeAffect(_vector1, _crossProduct);
	
	std::vector<std::string> specialization;
	specialization.push_back("Vec3");
	specialization.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_vector0, specialization);
	setAttributeAllowedSpecializations(_vector1, specialization);
	setAttributeAllowedSpecializations(_crossProduct, specialization);
	
	addAttributeSpecializationLink(_vector0, _crossProduct);
	addAttributeSpecializationLink(_vector1, _crossProduct);
}

void Vec3Cross::update(Attribute *attribute){
	const std::vector<Imath::V3f> &vector0 = _vector0->value()->vec3Values();
	const std::vector<Imath::V3f> &vector1 = _vector1->value()->vec3Values();
	
	int size0 = vector0.size();
	int size1 = vector1.size();
	
	int minSize = size0;
	if(size0 < size1){
		minSize = size1;
	}
	
	std::vector<Imath::V3f> crossedValues(minSize);
	
	for(int i = 0; i < minSize; ++i){
		crossedValues[i] = vector0[i].cross(vector1[i]);
	}
	
	_crossProduct->outValue()->setVec3Values(crossedValues);
}

Vec3Normalize::Vec3Normalize(const std::string &name, Node *parent): Node(name, parent){
	_vector = new NumericAttribute("vector", this);
	_normalized = new NumericAttribute("normalized", this);
	
	addInputAttribute(_vector);
	addOutputAttribute(_normalized);
	
	setAttributeAffect(_vector, _normalized);
	
	std::vector<std::string> specialization;
	specialization.push_back("Vec3");
	specialization.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_vector, specialization);
	setAttributeAllowedSpecializations(_normalized, specialization);
	
	addAttributeSpecializationLink(_vector, _normalized);
}

void Vec3Normalize::update(Attribute *attribute){
	const std::vector<Imath::V3f> &vector = _vector->value()->vec3Values();
	int size = vector.size();
	
	std::vector<Imath::V3f> normalizedValues(size);
	for(int i = 0; i < size; ++i){
		normalizedValues[i] = vector[i].normalized();
	}
	
	_normalized->outValue()->setVec3Values(normalizedValues);
}

Acos::Acos(const std::string &name, Node *parent): Node(name, parent){
	_inNumber = new NumericAttribute("inNumber", this);
	_outNumber = new NumericAttribute("outNumber", this);
	
	addInputAttribute(_inNumber);
	addOutputAttribute(_outNumber);
	
	setAttributeAffect(_inNumber, _outNumber);
	
	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_inNumber, specialization);
	setAttributeAllowedSpecializations(_outNumber, specialization);
	
	addAttributeSpecializationLink(_inNumber, _outNumber);
}

void Acos::update(Attribute *attribute){
	std::vector<float> inValues = _inNumber->value()->floatValues();
	std::vector<float> outValues(inValues.size());
	
	for(int i = 0; i < inValues.size(); ++i){
		outValues[i] = acos(inValues[i]);
	}
	
	_outNumber->outValue()->setFloatValues(outValues);
}

TrigonometricFunctions::TrigonometricFunctions(const std::string &name, Node *parent):
	Node(name, parent)
{

	_inNumber = new NumericAttribute("inNumber", this);
	_outNumber = new NumericAttribute("outNumber", this);
	_function = new NumericAttribute("func", this);

	addInputAttribute(_inNumber);
	addInputAttribute(_function);
	addOutputAttribute(_outNumber);

	setAttributeAffect(_inNumber, _outNumber);
	setAttributeAffect(_function, _outNumber);

	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");

	std::vector<std::string> funcSpecialization;
	funcSpecialization.push_back("Int");

	setAttributeAllowedSpecializations(_inNumber, specialization);
	setAttributeAllowedSpecializations(_outNumber, specialization);
	setAttributeAllowedSpecializations(_function, funcSpecialization);

	addAttributeSpecializationLink(_inNumber, _outNumber);
}

void TrigonometricFunctions::update(Attribute *attribute){
	std::vector<float> inValues = _inNumber->value()->floatValues();
	int inFunction = _function->value()->intValues()[0];
	std::vector<float> outValues(inValues.size());

	for(int i = 0; i < inValues.size(); ++i){
		switch(inFunction)
		{
		case 0:
			outValues[i] = cos(inValues[i]);
			break;
		case 1:
			outValues[i] = sin(inValues[i]);
			break;
		case 2:
			outValues[i] = tan(inValues[i]);
			break;
		case 3:
			outValues[i] = acos(inValues[i]);
			break;
		case 4:
			outValues[i] = asin(inValues[i]);
			break;
		case 5:
			outValues[i] = atan(inValues[i]);
			break;
		case 6:
			outValues[i] = cosh(inValues[i]);
			break;
		case 7:
			outValues[i] = sinh(inValues[i]);
			break;
		case 8:
			outValues[i] = tanh(inValues[i]);
			break;
		}
	}

	_outNumber->outValue()->setFloatValues(outValues);
}

Radians::Radians(const std::string &name, Node *parent): Node(name, parent){
	_inNumber = new NumericAttribute("in", this);
	_outNumber = new NumericAttribute("out", this);

	addInputAttribute(_inNumber);
	addOutputAttribute(_outNumber);

	setAttributeAffect(_inNumber, _outNumber);

	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	setAttributeAllowedSpecializations(_inNumber, specialization);
	setAttributeAllowedSpecializations(_outNumber, specialization);

	addAttributeSpecializationLink(_inNumber, _outNumber);
}

void Radians::update(Attribute *attribute){
	const std::vector<float> &in = _inNumber->value()->floatValues();
	int size = in.size();

	std::vector<float> outValues(size);
	for(int i = 0; i < size; ++i){
		outValues[i] = in[i]*M_PI/180.0f;
	}

	_outNumber->outValue()->setFloatValues(outValues);
}

Degrees::Degrees(const std::string &name, Node *parent): Node(name, parent){
	_inNumber = new NumericAttribute("in", this);
	_outNumber = new NumericAttribute("out", this);

	addInputAttribute(_inNumber);
	addOutputAttribute(_outNumber);

	setAttributeAffect(_inNumber, _outNumber);

	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	setAttributeAllowedSpecializations(_inNumber, specialization);
	setAttributeAllowedSpecializations(_outNumber, specialization);

	addAttributeSpecializationLink(_inNumber, _outNumber);
}

void Degrees::update(Attribute *attribute){
	const std::vector<float> &in = _inNumber->value()->floatValues();
	int size = in.size();

	std::vector<float> outValues(size);
	for(int i = 0; i < size; ++i){
		outValues[i] = in[i]*180.0f/float(M_PI);
	}

	_outNumber->outValue()->setFloatValues(outValues);
}

