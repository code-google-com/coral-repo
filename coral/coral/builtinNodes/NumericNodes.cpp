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

#include <map>
#include <cmath>

#include "NumericNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/mathUtils.h"
#include "../src/stringUtils.h"
#include "../src/Command.h"

#include <ImathEuler.h>
#include <ImathRandom.h>

using namespace coral;

namespace {

int findMinorNumericSize(NumericAttribute *attrs[], int numAttrs){
	int minorSize = -1;
	for(int i = 0; i < numAttrs; ++i){
		if(attrs[i]->input()){
			Numeric *num = attrs[i]->value();
			int size = num->size();
			if(size < minorSize || minorSize == -1){
				minorSize = size;
			}
		}
	}
	if(minorSize == -1){
		minorSize = 0;
	}
	
	return minorSize;
}

std::map<int, Numeric> _globalNumericStorage;

}


IntNode::IntNode(const std::string &name, Node* parent): Node(name, parent){
	_out = new NumericAttribute("out", this);
	addOutputAttribute(_out);
	
	setAttributeAllowedSpecialization(_out, "Int");
	
	_out->outValue()->resize(1);
	_out->outValue()->setFloatValueAt(0, 0.0);
}

FloatNode::FloatNode(const std::string &name, Node* parent): Node(name, parent){
	_out = new NumericAttribute("out", this);
	addOutputAttribute(_out);
	
	setAttributeAllowedSpecialization(_out, "Float");
	
	_out->outValue()->resize(1);
	_out->outValue()->setIntValueAt(0, 0.0);
}

Vec3Node::Vec3Node(const std::string &name, Node* parent): Node(name, parent){
	_x = new NumericAttribute("x", this);
	_y = new NumericAttribute("y", this);
	_z = new NumericAttribute("z", this);
	_vector = new NumericAttribute("vector", this);
	
	addInputAttribute(_x);
	addInputAttribute(_y);
	addInputAttribute(_z);
	addOutputAttribute(_vector);
	
	setAttributeAffect(_x, _vector);
	setAttributeAffect(_y, _vector);
	setAttributeAffect(_z, _vector);
	
	std::vector<std::string> xyzSpecializations;
	xyzSpecializations.push_back("Float");
	xyzSpecializations.push_back("FloatArray");
	
	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Vec3");
	vectorSpecializations.push_back("Vec3Array");
	
	setAttributeAllowedSpecializations(_x, xyzSpecializations);
	setAttributeAllowedSpecializations(_y, xyzSpecializations);
	setAttributeAllowedSpecializations(_z, xyzSpecializations);
	setAttributeAllowedSpecializations(_vector, vectorSpecializations);
	
	addAttributeSpecializationLink(_x, _vector);
	addAttributeSpecializationLink(_x, _y);
	addAttributeSpecializationLink(_y, _z);
	
	setSpecializationPreset("single", _x, "Float");
	setSpecializationPreset("single", _y, "Float");
	setSpecializationPreset("single", _z, "Float");
	setSpecializationPreset("single", _vector, "Vec3");
	
	setSpecializationPreset("array", _x, "FloatArray");
	setSpecializationPreset("array", _y, "FloatArray");
	setSpecializationPreset("array", _z, "FloatArray");
	setSpecializationPreset("array", _vector, "Vec3Array");
	enableSpecializationPreset("single");
}

void Vec3Node::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _x && attributeB == _vector){
		std::vector<std::string> vectorSpecializations = _vector->allowedSpecialization();
		std::vector<std::string> xSpecializations = _x->allowedSpecialization();
	
		if(specializationA.size() == 1){
			specializationB.clear();
			
			if(specializationA[0] == "Float"){
				specializationB.push_back(vectorSpecializations[0]);
			}
			else if(specializationA[0] == "FloatArray"){
				specializationB.push_back(vectorSpecializations[1]);
			}
		}
		else if(specializationB.size() == 1){
			specializationA.clear();
			
			if(specializationB[0] == "Vec3"){
				specializationA.push_back(xSpecializations[0]);
			}
			else if(specializationB[0] == "Vec3Array"){
				specializationA.push_back(xSpecializations[1]);
			}
		}
	}
	else{
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
}

void Vec3Node::update(Attribute *attribute){
	Numeric *x = _x->value();
	Numeric *y = _y->value();
	Numeric *z = _z->value();
	
	NumericAttribute *attrs[] = {_x, _y, _z};
	int minorSize = findMinorNumericSize(attrs, 3);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<Imath::V3f> outArray(minorSize);

	for(int i = 0; i < minorSize; ++i){
		Imath::V3f *vec = &outArray[i];
		vec->x = x->floatValueAt(i);
		vec->y = y->floatValueAt(i);
		vec->z = z->floatValueAt(i);
	}
	
	_vector->outValue()->setVec3Values(outArray);
}

Vec3ToFloats::Vec3ToFloats(const std::string &name, Node* parent): Node(name, parent){	
	_vector = new NumericAttribute("vector", this);
	_x = new NumericAttribute("x", this);
	_y = new NumericAttribute("y", this);
	_z = new NumericAttribute("z", this);
	
	addInputAttribute(_vector);
	addOutputAttribute(_x);
	addOutputAttribute(_y);
	addOutputAttribute(_z);
	
	setAttributeAffect(_vector, _x);
	setAttributeAffect(_vector, _y);
	setAttributeAffect(_vector, _z);
	
	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Vec3");
	vectorSpecializations.push_back("Vec3Array");
	
	std::vector<std::string> xyzSpecializations;
	xyzSpecializations.push_back("Float");
	xyzSpecializations.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_vector, vectorSpecializations);
	setAttributeAllowedSpecializations(_x, xyzSpecializations);
	setAttributeAllowedSpecializations(_y, xyzSpecializations);
	setAttributeAllowedSpecializations(_z, xyzSpecializations);
	
	addAttributeSpecializationLink(_vector, _x);
	addAttributeSpecializationLink(_x, _y);
	addAttributeSpecializationLink(_y, _z);
}

void Vec3ToFloats::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _vector && attributeB == _x){
		std::vector<std::string> vectorSpecializations = _vector->allowedSpecialization();
		std::vector<std::string> xSpecializations = _x->allowedSpecialization();
	
		if(specializationA.size() == 1){
			specializationB.clear();

			if(specializationA[0] == "Vec3"){
				specializationB.push_back(xSpecializations[0]);
			}
			else if(specializationA[0] == "Vec3Array"){
				specializationB.push_back(xSpecializations[1]);
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
	else{
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
}

void Vec3ToFloats::update(Attribute *attribute){
	const std::vector<Imath::V3f> &vec3Values = _vector->value()->vec3Values();
	int size = vec3Values.size();
	
	std::vector<float> xValues(size);
	std::vector<float> yValues(size);
	std::vector<float> zValues(size);
	
	for(int i = 0; i < size; ++i){
		const Imath::V3f &vec = vec3Values[i];
		xValues[i] = vec.x;
		yValues[i] = vec.y;
		zValues[i] = vec.z;
	}
	
	_x->outValue()->setFloatValues(xValues);
	_y->outValue()->setFloatValues(yValues);
	_z->outValue()->setFloatValues(zValues);
	
	setAttributeIsClean(_x, true);
	setAttributeIsClean(_y, true);
	setAttributeIsClean(_z, true);
}

QuatNode::QuatNode(const std::string &name, Node* parent): Node(name, parent)
{
	_r = new NumericAttribute("r", this);
	_x = new NumericAttribute("x", this);
	_y = new NumericAttribute("y", this);
	_z = new NumericAttribute("z", this);
	_quat = new NumericAttribute("quaternion", this);

	addInputAttribute(_r);
	addInputAttribute(_x);
	addInputAttribute(_y);
	addInputAttribute(_z);
	addOutputAttribute(_quat);

	setAttributeAffect(_r, _quat);
	setAttributeAffect(_x, _quat);
	setAttributeAffect(_y, _quat);
	setAttributeAffect(_z, _quat);

	std::vector<std::string> rxyzSpecializations;
	rxyzSpecializations.push_back("Float");
	rxyzSpecializations.push_back("FloatArray");

	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Quat");
	vectorSpecializations.push_back("QuatArray");


	setAttributeAllowedSpecializations(_r, rxyzSpecializations);
	setAttributeAllowedSpecializations(_x, rxyzSpecializations);
	setAttributeAllowedSpecializations(_y, rxyzSpecializations);
	setAttributeAllowedSpecializations(_z, rxyzSpecializations);
	setAttributeAllowedSpecializations(_quat, vectorSpecializations);

	addAttributeSpecializationLink(_r ,_quat);
	addAttributeSpecializationLink(_r, _x);
	addAttributeSpecializationLink(_x, _y);
	addAttributeSpecializationLink(_y, _z);
}

void QuatNode::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _r && attributeB == _quat){
		std::vector<std::string> vectorSpecializations = _quat->allowedSpecialization();
		std::vector<std::string> xSpecializations = _r->allowedSpecialization();

		if(specializationA.size() == 1){
			specializationB.clear();

			if(specializationA[0] == "Float"){
				specializationB.push_back(vectorSpecializations[0]);
			}
			else if(specializationA[0] == "FloatArray"){
				specializationB.push_back(vectorSpecializations[1]);
			}
		}
		else if(specializationB.size() == 1){
			specializationA.clear();

			if(specializationB[0] == "Quat"){
				specializationA.push_back(xSpecializations[0]);
			}
			else if(specializationB[0] == "QuatArray"){
				specializationA.push_back(xSpecializations[1]);
			}
		}
	}
	else{
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
}

void QuatNode::update(Attribute *attribute){
	Numeric *r = _r->value();
	Numeric *x = _x->value();
	Numeric *y = _y->value();
	Numeric *z = _z->value();

	NumericAttribute *attrs[] = {_r,_x, _y, _z};
	int minorSize = findMinorNumericSize(attrs, 4);
	if(minorSize < 1)
		minorSize = 1;

	std::vector<Imath::Quatf> outArray(minorSize);

	for(int i = 0; i < minorSize; ++i){
		Imath::Quatf *vec = &outArray[i];
		vec->r = r->floatValueAt(i);
		vec->v.x = x->floatValueAt(i);
		vec->v.y = y->floatValueAt(i);
		vec->v.z = z->floatValueAt(i);
	}

	_quat->outValue()->setQuatValues(outArray);
}

QuatToFloats::QuatToFloats(const std::string &name, Node* parent): Node(name, parent){
	_quat = new NumericAttribute("quaternion", this);
	_r = new NumericAttribute("r", this);
	_x = new NumericAttribute("x", this);
	_y = new NumericAttribute("y", this);
	_z = new NumericAttribute("z", this);

	addInputAttribute(_quat);
	addOutputAttribute(_r);
	addOutputAttribute(_x);
	addOutputAttribute(_y);
	addOutputAttribute(_z);

	setAttributeAffect(_quat, _r);
	setAttributeAffect(_quat, _x);
	setAttributeAffect(_quat, _y);
	setAttributeAffect(_quat, _z);

	std::vector<std::string> vectorSpecializations;
	vectorSpecializations.push_back("Quat");
	vectorSpecializations.push_back("QuatArray");

	std::vector<std::string> xyzSpecializations;
	xyzSpecializations.push_back("Float");
	xyzSpecializations.push_back("FloatArray");

	setAttributeAllowedSpecializations(_quat, vectorSpecializations);

	setAttributeAllowedSpecializations(_r, xyzSpecializations);
	setAttributeAllowedSpecializations(_x, xyzSpecializations);
	setAttributeAllowedSpecializations(_y, xyzSpecializations);
	setAttributeAllowedSpecializations(_z, xyzSpecializations);

	addAttributeSpecializationLink(_quat, _r);
	addAttributeSpecializationLink(_r, _x);
	addAttributeSpecializationLink(_x, _y);
	addAttributeSpecializationLink(_y, _z);

}

void QuatToFloats::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _quat && attributeB == _r){
		std::vector<std::string> vectorSpecializations = _quat->allowedSpecialization();
		std::vector<std::string> xSpecializations = _r->allowedSpecialization();

		if(specializationA.size() == 1){
			specializationB.clear();

			if(specializationA[0] == "Quat"){
				specializationB.push_back(xSpecializations[0]);
			}
			else if(specializationA[0] == "QuatArray"){
				specializationB.push_back(xSpecializations[1]);
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
	else{
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
}

void QuatToFloats::update(Attribute *attribute){
	const std::vector<Imath::Quatf> &quatValues = _quat->value()->quatValues();
	int size = quatValues.size();

	std::vector<float> rValues(size);
	std::vector<float> xValues(size);
	std::vector<float> yValues(size);
	std::vector<float> zValues(size);

	for(int i = 0; i < size; ++i){
		const Imath::Quatf &vec = quatValues[i];
		rValues[i] = vec.r;
		xValues[i] = vec.v.x;
		yValues[i] = vec.v.y;
		zValues[i] = vec.v.z;
	}

	_r->outValue()->setFloatValues(rValues);
	_x->outValue()->setFloatValues(xValues);
	_y->outValue()->setFloatValues(yValues);
	_z->outValue()->setFloatValues(zValues);

	setAttributeIsClean(_r, true);
	setAttributeIsClean(_x, true);
	setAttributeIsClean(_y, true);
	setAttributeIsClean(_z, true);
}


Matrix44Node::Matrix44Node(const std::string &name, Node* parent): Node(name, parent){	
	_translateX = new NumericAttribute("translateX", this);
	_translateY = new NumericAttribute("translateY", this);
	_translateZ = new NumericAttribute("translateZ", this);
	_eulerX = new NumericAttribute("eulerX", this);
	_eulerY = new NumericAttribute("eulerY", this);
	_eulerZ = new NumericAttribute("eulerZ", this);
	_scaleX = new NumericAttribute("scaleX", this);
	_scaleY = new NumericAttribute("scaleY", this);
	_scaleZ = new NumericAttribute("scaleZ", this);
	_matrix = new NumericAttribute("matrix", this);
	
	addInputAttribute(_translateX);
	addInputAttribute(_translateY);
	addInputAttribute(_translateZ);
	addInputAttribute(_eulerX);
	addInputAttribute(_eulerY);
	addInputAttribute(_eulerZ);
	addInputAttribute(_scaleX);
	addInputAttribute(_scaleY);
	addInputAttribute(_scaleZ);
	addOutputAttribute(_matrix);
	
	setAttributeAffect(_translateX, _matrix);
	setAttributeAffect(_translateY, _matrix);
	setAttributeAffect(_translateZ, _matrix);
	setAttributeAffect(_eulerX, _matrix);
	setAttributeAffect(_eulerY, _matrix);
	setAttributeAffect(_eulerZ, _matrix);
	setAttributeAffect(_scaleX, _matrix);
	setAttributeAffect(_scaleY, _matrix);
	setAttributeAffect(_scaleZ, _matrix);
	
	std::vector<std::string> floatSpecialization;
	floatSpecialization.push_back("Float");
	floatSpecialization.push_back("FloatArray");
	
	std::vector<std::string> matrixSpecialization;
	matrixSpecialization.push_back("Matrix44");
	matrixSpecialization.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_translateX, floatSpecialization);
	setAttributeAllowedSpecializations(_translateY, floatSpecialization);
	setAttributeAllowedSpecializations(_translateZ, floatSpecialization);
	setAttributeAllowedSpecializations(_eulerX, floatSpecialization);
	setAttributeAllowedSpecializations(_eulerY, floatSpecialization);
	setAttributeAllowedSpecializations(_eulerZ, floatSpecialization);
	setAttributeAllowedSpecializations(_scaleX, floatSpecialization);
	setAttributeAllowedSpecializations(_scaleY, floatSpecialization);
	setAttributeAllowedSpecializations(_scaleZ, floatSpecialization);
	setAttributeAllowedSpecializations(_matrix, matrixSpecialization);
	
	addAttributeSpecializationLink(_translateX, _matrix);
	addAttributeSpecializationLink(_translateY, _matrix);
	addAttributeSpecializationLink(_translateZ, _matrix);
	addAttributeSpecializationLink(_eulerX, _matrix);
	addAttributeSpecializationLink(_eulerY, _matrix);
	addAttributeSpecializationLink(_eulerZ, _matrix);
	addAttributeSpecializationLink(_scaleX, _matrix);
	addAttributeSpecializationLink(_scaleY, _matrix);
	addAttributeSpecializationLink(_scaleZ, _matrix);
	
	_scaleX->outValue()->setFloatValueAt(0, 1.0);
	_scaleY->outValue()->setFloatValueAt(0, 1.0);
	_scaleZ->outValue()->setFloatValueAt(0, 1.0);
}

void Matrix44Node::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() == 1){
		specializationB.clear();
		
		if(specializationA[0] == "Float"){
			specializationB.push_back("Matrix44");
		}
		else if(specializationA[0] == "FloatArray"){
			specializationB.push_back("Matrix44Array");
		}
	}
	else if(specializationB.size() == 1){
		specializationA.clear();

		if(specializationB[0] == "Matrix44"){
			specializationA.push_back("Float");
		}
		else if(specializationB[0] == "Matrix44Array"){
			specializationA.push_back("FloatArray");
		}
	}
}

void Matrix44Node::update(Attribute *attribute){
	Numeric *translateX = _translateX->value();
	Numeric *translateY = _translateY->value();
	Numeric *translateZ = _translateZ->value();
	Numeric *eulerX = _eulerX->value();
	Numeric *eulerY = _eulerY->value();
	Numeric *eulerZ = _eulerZ->value();
	Numeric *scaleX = _scaleX->value();
	Numeric *scaleY = _scaleY->value();
	Numeric *scaleZ = _scaleZ->value();
	
	NumericAttribute *attrs[] = {
			_translateX,
			_translateY,
			_translateZ,
			_eulerX,
			_eulerY,
			_eulerZ,
			_scaleX,
			_scaleY,
			_scaleZ};
			
	int minorSize = findMinorNumericSize(attrs, 9);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<Imath::M44f> newMatrixValues(minorSize);
	float radian = (M_PI / 180);
	for(int i = 0; i < minorSize; ++i){
		Imath::M44f matrix;
		Imath::V3f vec(translateX->floatValueAt(i), translateY->floatValueAt(i), translateZ->floatValueAt(i));
		matrix.translate(vec);
		
		vec.setValue(radian * eulerX->floatValueAt(i), radian * eulerY->floatValueAt(i), radian * eulerZ->floatValueAt(i));
		matrix.rotate(vec);
		
		vec.setValue(scaleX->floatValueAt(i), scaleY->floatValueAt(i), scaleZ->floatValueAt(i));
		matrix.scale(vec);
		
		newMatrixValues[i] = matrix;
	}
	
	_matrix->outValue()->setMatrix44Values(newMatrixValues);
}

ConstantArray::ConstantArray(const std::string &name, Node *parent): Node(name, parent){	
	_size = new NumericAttribute("size", this);
	_constant = new NumericAttribute("constant", this);
	_array = new NumericAttribute("array", this);
	
	addInputAttribute(_size);
	addInputAttribute(_constant);
	addOutputAttribute(_array);
	
	setAttributeAffect(_size, _array);
	setAttributeAffect(_constant, _array);
	
	std::vector<std::string> constantSpecializations;
	constantSpecializations.push_back("Int");
	constantSpecializations.push_back("Float");
	constantSpecializations.push_back("Vec3");
	constantSpecializations.push_back("Matrix44");
	
	std::vector<std::string> arraySpecializations;
	arraySpecializations.push_back("IntArray");
	arraySpecializations.push_back("FloatArray");
	arraySpecializations.push_back("Vec3Array");
	arraySpecializations.push_back("Matrix44Array");
	
	setAttributeAllowedSpecialization(_size, "Int");
	setAttributeAllowedSpecializations(_constant, constantSpecializations);
	setAttributeAllowedSpecializations(_array, arraySpecializations);
	
	addAttributeSpecializationLink(_constant, _array);
}

void ConstantArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		std::vector<std::string> newSpecialization;
		
		for(int i = 0; i < specializationA.size(); ++i){
			newSpecialization.push_back(specializationA[i] + "Array");
		}
		
		specializationB = newSpecialization;
	}
	else{
		std::vector<std::string> newSpecialization;
		
		for(int i = 0; i < specializationB.size(); ++i){
			newSpecialization.push_back(stringUtils::replace(specializationB[i], "Array", ""));
		}
		
		specializationA = newSpecialization;
	}
}

void ConstantArray::update(Attribute *attribute){
	Numeric *constant = _constant->value();
	if(constant->type() != Numeric::numericTypeAny){
		int size = _size->value()->intValueAt(0);
		if(size < 0){
			size = 0;
			_size->outValue()->setIntValueAt(0, 0);
		}
		
		Numeric *array = _array->outValue();
		if(constant->type() == Numeric::numericTypeInt){
			int constantValue = constant->intValueAt(0);
			std::vector<int> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setIntValues(values);
		}
		else if(constant->type() == Numeric::numericTypeFloat){
			float constantValue = constant->floatValueAt(0);
			std::vector<float> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setFloatValues(values);
		}
		else if(constant->type() == Numeric::numericTypeVec3){
			Imath::V3f constantValue = constant->vec3ValueAt(0);
			std::vector<Imath::V3f> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setVec3Values(values);
		}
		else if(constant->type() == Numeric::numericTypeMatrix44){
			Imath::M44f constantValue = constant->matrix44ValueAt(0);
			std::vector<Imath::M44f> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setMatrix44Values(values);
		}
	}
	else{
		setAttributeIsClean(_array, false);
	}
}

ArraySize::ArraySize(const std::string &name, Node *parent): Node(name, parent){	
	_array = new NumericAttribute("array", this);
	_size = new NumericAttribute("size", this);
	
	addInputAttribute(_array);
	addOutputAttribute(_size);
	
	setAttributeAffect(_array, _size);
	
	std::vector<std::string> arraySpecializations;
	arraySpecializations.push_back("IntArray");
	arraySpecializations.push_back("FloatArray");
	arraySpecializations.push_back("Vec3Array");
	arraySpecializations.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_array, arraySpecializations);
	setAttributeAllowedSpecialization(_size, "Int");
}

void ArraySize::update(Attribute *attribute){
	Numeric *array = _array->value();
	if(array->type() != Numeric::numericTypeAny){
		_size->outValue()->setIntValueAt(0, array->size());
	}
	else{
		setAttributeIsClean(_size, false);
	}
}


BuildArray::BuildArray(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0){
	setAllowDynamicAttributes(true);
	
	_array = new NumericAttribute("array", this);
	addOutputAttribute(_array);
}

void BuildArray::addNumericAttribute(){
	std::string numStr = stringUtils::intToString(inputAttributes().size());
	NumericAttribute *attr = new NumericAttribute("in" + numStr, this);
	addInputAttribute(attr);
	setAttributeAffect(attr, _array);
	
	std::vector<std::string> specialization;
	specialization.push_back("Int");
	specialization.push_back("Float");
	specialization.push_back("Vec3");
	specialization.push_back("Matrix44");
	
	setAttributeAllowedSpecializations(attr, specialization);
	
	addAttributeSpecializationLink(attr, _array);
	
	addDynamicAttribute(attr);
	updateAttributeSpecialization(attr);
}

void BuildArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() <= specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else{
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void BuildArray::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &BuildArray::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &BuildArray::updateFloat;
			}
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &BuildArray::updateVec3;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &BuildArray::updateMatrix44;
			}
		}
	}
}

void BuildArray::updateInt(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array){
	std::vector<int> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->intValueAt(0);
	}
	
	array->setIntValues(arrayValues);
}

void BuildArray::updateFloat(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array){
	std::vector<float> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->floatValueAt(0);
	}
	
	array->setFloatValues(arrayValues);
}

void BuildArray::updateVec3(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array){
	std::vector<Imath::V3f> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->vec3ValueAt(0);
	}
	
	array->setVec3Values(arrayValues);
}

void BuildArray::updateMatrix44(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array){
	std::vector<Imath::M44f> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->matrix44ValueAt(0);
	}
	
	array->setMatrix44Values(arrayValues);
}

void BuildArray::update(Attribute *attribute){
	if(_selectedOperation){
		std::vector<Attribute*> inAttrs = inputAttributes();
		int arraySize = inAttrs.size();
		Numeric *array = _array->outValue();
		
		(this->*_selectedOperation)(inAttrs, arraySize, array);
	}
}


RangeArray::RangeArray(const std::string &name, Node* parent):
Node(name, parent),
_selectedOperation(0){	
	_start = new NumericAttribute("start", this);
	_end = new NumericAttribute("end", this);
	_steps = new NumericAttribute("steps", this);
	_array = new NumericAttribute("array", this);
	
	addInputAttribute(_start);
	addInputAttribute(_end);
	addInputAttribute(_steps);
	addOutputAttribute(_array);
	
	setAttributeAffect(_start, _array);
	setAttributeAffect(_end, _array);
	setAttributeAffect(_steps, _array);
	
	std::vector<std::string> inSpecializations;
	inSpecializations.push_back("Int");
	inSpecializations.push_back("Float");
	
	std::vector<std::string> outSpecializations;
	outSpecializations.push_back("IntArray");
	outSpecializations.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_start, inSpecializations);
	setAttributeAllowedSpecializations(_end, inSpecializations);
	setAttributeAllowedSpecialization(_steps, "Int");
	setAttributeAllowedSpecializations(_array, outSpecializations);
	
	addAttributeSpecializationLink(_start, _array);
	addAttributeSpecializationLink(_end, _array);
}

void RangeArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() <= specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else{
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void RangeArray::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &RangeArray::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &RangeArray::updateFloat;
			}
		}
	}
}

void RangeArray::updateInt(Numeric *start, Numeric *end, int steps, Numeric *array){
	int startValue = start->intValueAt(0);
	int endValue = end->intValueAt(0);
	
	float totalRange = endValue - startValue;
	int incrStep = totalRange / steps;
	int currentStep = startValue;
	std::vector<int> arrayValues(steps + 1);
	
	for(int i = 0; i < steps + 1; ++i){
		arrayValues[i] = currentStep;
		currentStep += incrStep;
	}
	
	array->setIntValues(arrayValues);
}

void RangeArray::updateFloat(Numeric *start, Numeric *end, int steps, Numeric *array){
	float startValue = start->floatValueAt(0);
	float endValue = end->floatValueAt(0);
	
	float totalRange = endValue - startValue;
	float incrStep = totalRange / float(steps);
	float currentStep = startValue;
	std::vector<float> arrayValues(steps + 1);
	
	for(int i = 0; i < steps + 1; ++i){
		arrayValues[i] = currentStep;
		
		currentStep += incrStep;
	}
	
	array->setFloatValues(arrayValues);
}

void RangeArray::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *start = _start->value();
		Numeric *end = _end->value();
		int steps = _steps->value()->intValueAt(0);
		if(steps < 0){
			_steps->outValue()->setIntValueAt(0, 0);
			steps = 0;
		}
		
		Numeric *array = _array->outValue();
		
		(this->*_selectedOperation)(start, end, steps, array);
	}
}

Matrix44Translation::Matrix44Translation(const std::string &name, Node* parent): Node(name, parent){	
	_matrix = new NumericAttribute("matrix", this);
	_translation = new NumericAttribute("translation", this);
	
	std::vector<std::string> matrixSpec;
	matrixSpec.push_back("Matrix44");
	matrixSpec.push_back("Matrix44Array");
	setAttributeAllowedSpecializations(_matrix, matrixSpec);
	
	std::vector<std::string> translationSpec;
	translationSpec.push_back("Vec3");
	translationSpec.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_translation, translationSpec);
	
	addInputAttribute(_matrix);
	addOutputAttribute(_translation);
	
	setAttributeAffect(_matrix, _translation);
	
	addAttributeSpecializationLink(_matrix, _translation);
}

void Matrix44Translation::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(1);
		std::string suffix = stringUtils::replace(specializationA[0], "Matrix44", "");
		specializationB[0] = "Vec3" + suffix;
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(1);
		std::string suffix = stringUtils::replace(specializationB[0], "Vec3", "");
		specializationA[0] = "Matrix44" + suffix;
	}
}

void Matrix44Translation::update(Attribute *attribute){
	const std::vector<Imath::M44f> &matrix = _matrix->value()->matrix44Values();
	int size = matrix.size();
	std::vector<Imath::V3f> translationValues(size);
	
	for(int i = 0; i < size; ++i){
		Imath::V3f &translationValue = translationValues[i];
		const Imath::M44f &m = matrix[i];
		
		translationValue.x = m[3][0];
		translationValue.y = m[3][1];
		translationValue.z = m[3][2];
	}
	
	_translation->outValue()->setVec3Values(translationValues);
}

Matrix44RotationAxis::Matrix44RotationAxis(const std::string &name, Node* parent): Node(name, parent){	
	_matrix = new NumericAttribute("matrix", this);
	_axisX = new NumericAttribute("axisX", this);
	_axisY = new NumericAttribute("axisY", this);
	_axisZ = new NumericAttribute("axisZ", this);
	
	addInputAttribute(_matrix);
	addOutputAttribute(_axisX);
	addOutputAttribute(_axisY);
	addOutputAttribute(_axisZ);
	
	std::vector<std::string> matrixSpec;
	matrixSpec.push_back("Matrix44");
	matrixSpec.push_back("Matrix44Array");
	setAttributeAllowedSpecializations(_matrix, matrixSpec);
	
	std::vector<std::string> axisSpec;
	axisSpec.push_back("Vec3");
	axisSpec.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_axisX, axisSpec);
	setAttributeAllowedSpecializations(_axisY, axisSpec);
	setAttributeAllowedSpecializations(_axisZ, axisSpec);
	
	setAttributeAffect(_matrix, _axisX);
	setAttributeAffect(_matrix, _axisY);
	setAttributeAffect(_matrix, _axisZ);
	
	addAttributeSpecializationLink(_matrix, _axisX);
	addAttributeSpecializationLink(_matrix, _axisY);
	addAttributeSpecializationLink(_matrix, _axisZ);
}

void Matrix44RotationAxis::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(1);
		std::string suffix = stringUtils::replace(specializationA[0], "Matrix44", "");
		specializationB[0] = "Vec3" + suffix;
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(1);
		std::string suffix = stringUtils::replace(specializationB[0], "Vec3", "");
		specializationA[0] = "Matrix44" + suffix;
	}
}

void Matrix44RotationAxis::update(Attribute *attribute){
	const std::vector<Imath::M44f> &matrix = _matrix->value()->matrix44Values();
	int size = matrix.size();
	std::vector<Imath::V3f> axisXValues(size);
	std::vector<Imath::V3f> axisYValues(size);
	std::vector<Imath::V3f> axisZValues(size);
	
	for(int i = 0; i < size; ++i){
		const Imath::M44f &mat = matrix[i];
		axisXValues[i] = Imath::V3f(mat[0][0], mat[0][1], mat[0][2]);
		axisYValues[i] = Imath::V3f(mat[1][0], mat[1][1], mat[1][2]);
		axisZValues[i] = Imath::V3f(mat[2][0], mat[2][1], mat[2][2]);
	}
	
	_axisX->outValue()->setVec3Values(axisXValues);
	_axisY->outValue()->setVec3Values(axisYValues);
	_axisZ->outValue()->setVec3Values(axisZValues);
	
	setAttributeIsClean(_axisX, true);
	setAttributeIsClean(_axisY, true);
	setAttributeIsClean(_axisZ, true);
}

Matrix44FromVectors::Matrix44FromVectors(const std::string &name, Node* parent): Node(name, parent){	
	_translation = new NumericAttribute("translation", this);
	_axisX = new NumericAttribute("axisX", this);
	_axisY = new NumericAttribute("axisY", this);
	_axisZ = new NumericAttribute("axisZ", this);
	_scale = new NumericAttribute("scale", this);
	_matrix = new NumericAttribute("matrix", this);
	
	addInputAttribute(_translation);
	addInputAttribute(_axisX);
	addInputAttribute(_axisY);
	addInputAttribute(_axisZ);
	addInputAttribute(_scale);
	addOutputAttribute(_matrix);
	
	setAttributeAffect(_translation, _matrix);
	setAttributeAffect(_axisX, _matrix);
	setAttributeAffect(_axisY, _matrix);
	setAttributeAffect(_axisZ, _matrix);
	setAttributeAffect(_scale, _matrix);
	
	std::vector<std::string> vecSpec;
	vecSpec.push_back("Vec3");
	vecSpec.push_back("Vec3Array");
	setAttributeAllowedSpecializations(_translation, vecSpec);
	setAttributeAllowedSpecializations(_axisX, vecSpec);
	setAttributeAllowedSpecializations(_axisY, vecSpec);
	setAttributeAllowedSpecializations(_axisZ, vecSpec);
	setAttributeAllowedSpecializations(_scale, vecSpec);
	
	std::vector<std::string> matrixSpec;
	matrixSpec.push_back("Matrix44");
	matrixSpec.push_back("Matrix44Array");
	setAttributeAllowedSpecializations(_matrix, matrixSpec);
	
	addAttributeSpecializationLink(_translation, _matrix);
	addAttributeSpecializationLink(_axisX, _matrix);
	addAttributeSpecializationLink(_axisY, _matrix);
	addAttributeSpecializationLink(_axisZ, _matrix);
	addAttributeSpecializationLink(_scale, _matrix);
}

void Matrix44FromVectors::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(1);
		std::string suffix = stringUtils::replace(specializationA[0], "Vec3", "");
		specializationB[0] = "Matrix44" + suffix;
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(1);
		std::string suffix = stringUtils::replace(specializationB[0], "Matrix44", "");
		specializationA[0] = "Vec3" + suffix;
	}
}

void Matrix44FromVectors::update(Attribute *attribute){
	Numeric *translation = _translation->value();
	Numeric *axisX = _axisX->value();
	Numeric *axisY = _axisY->value();
	Numeric *axisZ = _axisZ->value();
	Numeric *scale = _scale->value();
	
	NumericAttribute *attrs[] = {
			_translation,
			_axisX,
			_axisY,
			_axisZ,
			_scale};
			
	int minorSize = findMinorNumericSize(attrs, 5);
	if(minorSize < 1)
		minorSize = 1;
		
	std::vector<Imath::M44f> matrixValues(minorSize);
	for(int i = 0; i < minorSize; ++i){
		Imath::V3f translationValue = translation->vec3ValueAt(i);
		Imath::V3f scaleValue = scale->vec3ValueAt(i);
		
		Imath::V3f axisXValue = axisX->vec3ValueAt(i).normalized() * scaleValue.x;
		Imath::V3f axisYValue = axisY->vec3ValueAt(i).normalized() * scaleValue.y;
		Imath::V3f axisZValue = axisZ->vec3ValueAt(i).normalized() * scaleValue.z;
		
		matrixValues[i] = Imath::M44f(
			axisXValue.x, axisXValue.y, axisXValue.z, 0.0,
			axisYValue.x, axisYValue.y, axisYValue.z, 0.0,
			axisZValue.x, axisZValue.y, axisZValue.z, 0.0,
			translationValue.x, translationValue.y, translationValue.z, 1.0);
	}
	
	_matrix->outValue()->setMatrix44Values(matrixValues);
}

Matrix44EulerRotation::Matrix44EulerRotation(const std::string &name, Node* parent): Node(name, parent){	
	_matrix = new NumericAttribute("matrix", this);
	_eulerAngles = new NumericAttribute("eulerAngles", this);
	
	addInputAttribute(_matrix);
	addOutputAttribute(_eulerAngles);
	
	setAttributeAffect(_matrix, _eulerAngles);
	
	std::vector<std::string> matrixSpec;
	matrixSpec.push_back("Matrix44");
	matrixSpec.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_matrix, matrixSpec);
	
	std::vector<std::string> eulerSpec;
	eulerSpec.push_back("Vec3");
	eulerSpec.push_back("Vec3Array");
	
	setAttributeAllowedSpecializations(_eulerAngles, eulerSpec);
	
	addAttributeSpecializationLink(_matrix, _eulerAngles);
}

void Matrix44EulerRotation::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(1);
		std::string suffix = stringUtils::replace(specializationA[0], "Matrix44", "");
		specializationB[0] = "Vec3" + suffix;
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(1);
		std::string suffix = stringUtils::replace(specializationB[0], "Vec3", "");
		specializationA[0] = "Matrix44" + suffix;
	}
}

void Matrix44EulerRotation::update(Attribute *attribute){
	const std::vector<Imath::M44f> &matrix = _matrix->value()->matrix44Values();
	int size = matrix.size();
	
	std::vector<Imath::V3f> eulerAngles(size);
	
	Imath::Eulerf euler;
	float degree = (180 / M_PI);
	for(int i = 0; i < size; ++i){
		euler.extract(matrix[i]);
		Imath::V3f &currentValue = eulerAngles[i];
		
		currentValue.x = euler.x * degree;
		currentValue.y = euler.y * degree;
		currentValue.z = euler.z * degree;
	}
	
	_eulerAngles->outValue()->setVec3Values(eulerAngles);
}

RangeLoop::RangeLoop(const std::string &name, Node* parent): 
	Node(name, parent),
	_selectedOperation(0){	
	_start = new NumericAttribute("start", this);
	_end = new NumericAttribute("end", this);
	_step = new NumericAttribute("step", this);
	_valueInRange = new NumericAttribute("valueInRange", this);
	
	addInputAttribute(_start);
	addInputAttribute(_end);
	addInputAttribute(_step);
	addOutputAttribute(_valueInRange);
	
	setAttributeAffect(_start, _valueInRange);
	setAttributeAffect(_end, _valueInRange);
	setAttributeAffect(_step, _valueInRange);
	
	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	specialization.push_back("Int");
	specialization.push_back("IntArray");
	
	setAttributeAllowedSpecializations(_start, specialization);
	setAttributeAllowedSpecializations(_end, specialization);
	setAttributeAllowedSpecializations(_step, specialization);
	setAttributeAllowedSpecializations(_valueInRange, specialization);
	
	addAttributeSpecializationLink(_start, _valueInRange);
	addAttributeSpecializationLink(_end, _valueInRange);
	addAttributeSpecializationLink(_step, _valueInRange);
}

void RangeLoop::updateFloat(Numeric *start, Numeric *end, Numeric *step, Numeric *out){
	float startVal = start->floatValueAt(0);
	float endVal = end->floatValueAt(0);
	float stepVal = step->floatValueAt(0);
	
	float outVal = fmod((stepVal + startVal), endVal);
	out->setFloatValueAt(0, outVal);
}

void RangeLoop::updateFloatArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out){
	NumericAttribute *attrs[] = {_start, _end, _step};
			
	int minorSize = findMinorNumericSize(attrs, 3);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<float> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		float startVal = start->floatValueAt(i);
		float endVal = end->floatValueAt(i);
		float stepVal = step->floatValueAt(i);

		outVals[i] = fmod((stepVal + startVal), endVal);
	}
	
	out->setFloatValues(outVals);
}

void RangeLoop::updateInt(Numeric *start, Numeric *end, Numeric *step, Numeric *out){
	int startVal = start->intValueAt(0);
	int endVal = end->intValueAt(0);
	int stepVal = step->intValueAt(0);
	
	int outVal = (stepVal + startVal) % endVal;
	out->setIntValueAt(0, outVal);
}

void RangeLoop::updateIntArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out){
	NumericAttribute *attrs[] = {_start, _end, _step};
			
	int minorSize = findMinorNumericSize(attrs, 3);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<int> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		int startVal = start->intValueAt(i);
		int endVal = end->intValueAt(i);
		int stepVal = step->intValueAt(i);

		outVals[i] = (stepVal + startVal) % endVal;
	}
	
	out->setIntValues(outVals);
}

void RangeLoop::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;
	
	std::vector<std::string> outSpec = _valueInRange->specialization();
	if(outSpec.size() == 1){
		std::string spec = outSpec[0];
		
		if(spec == "Float"){
			_selectedOperation = &RangeLoop::updateFloat;
		}
		else if(spec == "FloatArray"){
			_selectedOperation = &RangeLoop::updateFloatArray;
		}
		else if(spec == "Int"){
			_selectedOperation = &RangeLoop::updateInt;
		}
		else if(spec == "IntArray"){
			_selectedOperation = &RangeLoop::updateIntArray;
		}
	}
}

void RangeLoop::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *start = _start->value();
		Numeric *end = _end->value();
		Numeric *step = _step->value();
		Numeric *out = _valueInRange->outValue();
		
		(this->*_selectedOperation)(start, end, step, out);
	}
}

RandomNumber::RandomNumber(const std::string &name, Node* parent): 
	Node(name, parent),
	_selectedOperation(0){	
	_min = new NumericAttribute("min", this);
	_max = new NumericAttribute("max", this);
	_out = new NumericAttribute("out", this);
	
	addInputAttribute(_min);
	addInputAttribute(_max);
	addOutputAttribute(_out);
	
	setAttributeAffect(_min, _out);
	setAttributeAffect(_max, _out);
	
	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	specialization.push_back("Int");
	specialization.push_back("IntArray");
	
	setAttributeAllowedSpecializations(_min, specialization);
	setAttributeAllowedSpecializations(_max, specialization);
	setAttributeAllowedSpecializations(_out, specialization);
	
	addAttributeSpecializationLink(_min, _out);
	addAttributeSpecializationLink(_max, _out);
}

void RandomNumber::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;
	
	std::vector<std::string> outSpec = _out->specialization();
	if(outSpec.size() == 1){
		std::string spec = outSpec[0];
		
		if(spec == "Float"){
			_selectedOperation = &RandomNumber::updateFloat;
		}
		else if(spec == "FloatArray"){
			_selectedOperation = &RandomNumber::updateFloatArray;
		}
		else if(spec == "Int"){
			_selectedOperation = &RandomNumber::updateInt;
		}
		else if(spec == "IntArray"){
			_selectedOperation = &RandomNumber::updateIntArray;
		}
	}
	
}

void RandomNumber::updateFloat(Numeric *min, Numeric *max, Numeric *out){
	float minVal = min->floatValueAt(0);
	float maxVal = max->floatValueAt(0);
	
	Imath::Rand32 rand;
	float outVal = rand.nextf(minVal, maxVal);
	out->setFloatValueAt(0, outVal);
}

void RandomNumber::updateFloatArray(Numeric *min, Numeric *max, Numeric *out){
	NumericAttribute *attrs[] = {_min, _max};
			
	int minorSize = findMinorNumericSize(attrs, 2);
	if(minorSize < 1)
		minorSize = 1;
	
	Imath::Rand32 rand;
	std::vector<float> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		float minVal = min->floatValueAt(0);
		float maxVal = max->floatValueAt(0);
		
		outVals[i] =  rand.nextf(minVal, maxVal);
	}
	
	out->setFloatValues(outVals);
}

void RandomNumber::updateInt(Numeric *min, Numeric *max, Numeric *out){
	int minVal = min->intValueAt(0);
	int maxVal = max->intValueAt(0);
	
	Imath::Rand32 rand;
	int outVal = rand.nextf(minVal, maxVal);
	out->setIntValueAt(0, outVal);
}

void RandomNumber::updateIntArray(Numeric *min, Numeric *max, Numeric *out){
	NumericAttribute *attrs[] = {_min, _max};
			
	int minorSize = findMinorNumericSize(attrs, 2);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<int> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		int minVal = min->intValueAt(0);
		int maxVal = max->intValueAt(0);
		
		Imath::Rand32 rand;
		outVals[i] =  rand.nextf(minVal, maxVal);
	}
	
	out->setIntValues(outVals);
}

void RandomNumber::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *min = _min->value();
		Numeric *max = _max->value();
		Numeric *out = _out->outValue();
		
		(this->*_selectedOperation)(min, max, out);
	}
}

NumericIterator::NumericIterator(const std::string &name, Node* parent): 
	LoopIteratorNode(name, parent),
	_selectedOperation(0){
	_element = new NumericAttribute("element", this);
	_array = new NumericAttribute("array", this);
	
	addInputAttribute(_element);
	addOutputAttribute(_array);
	
	setAttributeAffect(_element, _array);
	
	std::vector<std::string> elementSpec;
	elementSpec.push_back("Int");
	elementSpec.push_back("Float");
	elementSpec.push_back("Vec3");
	elementSpec.push_back("Matrix44");
	setAttributeAllowedSpecializations(_element, elementSpec);
	
	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Matrix44Array");
	setAttributeAllowedSpecializations(_array, arraySpec);
	
	addAttributeSpecializationLink(_element, _array);
}

void NumericIterator::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void NumericIterator::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &NumericIterator::stepFloat;
			}
		}
	}
}

void NumericIterator::loopStart(unsigned int loopRangeSize){
	_array->outValue()->resize(loopRangeSize);
}

void NumericIterator::stepFloat(unsigned int index, Numeric *element, Numeric *array){
	array->setFloatValueAt(index, element->floatValueAt(0));
}

void NumericIterator::loopStep(unsigned int index){
	if(_selectedOperation){
		Numeric *element = _element->value();
		Numeric *array = _array->outValue();
		
		(this->*_selectedOperation)(index, element, array);
	}
}

ArrayIndices::ArrayIndices(const std::string &name, Node* parent): Node(name, parent){
	_array = new NumericAttribute("array", this);
	_indices = new NumericAttribute("indices", this);
	
	addInputAttribute(_array);
	addOutputAttribute(_indices);
	
	setAttributeAffect(_array, _indices);
	
	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_array, arraySpec);
	setAttributeAllowedSpecialization(_indices, "IntArray");
}

void ArrayIndices::update(Attribute *attribute){
	int arraySize = _array->value()->size();
	std::vector<int> indices(arraySize);
	
	for(int i = 0; i < arraySize; ++i){
		indices[i] = i;
	}
	
	_indices->outValue()->setIntValues(indices);
}

GetArrayElement::GetArrayElement(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0){
	_array = new NumericAttribute("array", this);
	_index = new NumericAttribute("index", this);
	_element = new NumericAttribute("element", this);
	
	addInputAttribute(_array);
	addInputAttribute(_index);
	addOutputAttribute(_element);
	
	setAttributeAffect(_array, _element);
	setAttributeAffect(_index, _element);
	
	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Matrix44Array");
	
	std::vector<std::string> elementSpec;
	elementSpec.push_back("Int");
	elementSpec.push_back("Float");
	elementSpec.push_back("Vec3");
	elementSpec.push_back("Matrix44");
	
	setAttributeAllowedSpecializations(_array, arraySpec);
	setAttributeAllowedSpecialization(_index, "Int");
	setAttributeAllowedSpecializations(_element, elementSpec);
	
	addAttributeSpecializationLink(_array, _element);
}

void GetArrayElement::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = stringUtils::strip(specializationA[i], "Array");
		}
	}
	else if(specializationB.size() < specializationA.size()){
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = specializationB[i] + "Array";
		}
	}
}

void GetArrayElement::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &GetArrayElement::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &GetArrayElement::updateFloat;
			}
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &GetArrayElement::updateVec3;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &GetArrayElement::updateMatrix44;
			}
		}
	}
}

void GetArrayElement::updateInt(Numeric *array, int index, Numeric *element){
	element->setIntValueAt(0, array->intValueAt(index));
}

void GetArrayElement::updateFloat(Numeric *array, int index, Numeric *element){
	element->setFloatValueAt(0, array->floatValueAt(index));
}

void GetArrayElement::updateVec3(Numeric *array, int index, Numeric *element){
	element->setVec3ValueAt(0, array->vec3ValueAt(index));
}

void GetArrayElement::updateMatrix44(Numeric *array, int index, Numeric *element){
	element->setMatrix44ValueAt(0, array->matrix44ValueAt(index));
}

void GetArrayElement::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *array = _array->value();
		int index = _index->value()->intValueAt(0);
		Numeric *element = _element->outValue();
		
		(this->*_selectedOperation)(array, index, element);
	}
}

SetSimulationResult::SetSimulationResult(const std::string &name, Node *parent): Node(name, parent){	
	_source = new NumericAttribute("source", this);
	_data = new NumericAttribute("data", this);
	_process = new PassThroughAttribute("process", this);
	
	addInputAttribute(_source);
	addInputAttribute(_data);
	addOutputAttribute(_process);
	
	setAttributeAffect(_data, _process);
	
	setAttributeAllowedSpecialization(_process, "process");
	
	addAttributeSpecializationLink(_source, _data);
}

void SetSimulationResult::update(Attribute *attribute){
	int sourceId = _source->value()->id();
	
	
	_globalNumericStorage[sourceId].setFromOther(*_data->value());
}

GetSimulationResult::GetSimulationResult(const std::string &name, Node *parent): Node(name, parent){	
	_source = new NumericAttribute("source", this);
	_time = new NumericAttribute("time", this);
	_data = new NumericAttribute("data", this);
	
	addInputAttribute(_source);
	addInputAttribute(_time);
	addOutputAttribute(_data);
	
	setAttributeAffect(_source, _data);
	setAttributeAffect(_time, _data);
	
	setAttributeAllowedSpecialization(_time, "Float");
	
	addAttributeSpecializationLink(_source, _data);
}

void GetSimulationResult::update(Attribute *attribute){
	Numeric *source = _source->value();
	int sourceId = source->id();
	float time = _time->value()->floatValueAt(0);
	
	if(time == 0.0 || _globalNumericStorage.find(sourceId) == _globalNumericStorage.end()){
		_globalNumericStorage[sourceId].setFromOther(*source);
	}
	
	_data->outValue()->setFromOther(_globalNumericStorage[sourceId]);
}

QuatToAxisAngle::QuatToAxisAngle(const std::string &name, Node *parent): Node(name, parent)
{
	_quat = new NumericAttribute("quat",this);
	_axis = new NumericAttribute("axis",this);
	_angle = new NumericAttribute("angle",this);

	addInputAttribute(_quat);
	addOutputAttribute(_axis);
	addOutputAttribute(_angle);

	setAttributeAffect(_quat, _axis);
	setAttributeAffect(_quat, _angle);

	std::vector<std::string> quatSpecializations;
	quatSpecializations.push_back("Quat");
	quatSpecializations.push_back("QuatArray");

	std::vector<std::string> angleSpecializations;
	angleSpecializations.push_back("Float");
	angleSpecializations.push_back("FloatArray");

	std::vector<std::string> axisSpecializations;
	axisSpecializations.push_back("Vec3");
	axisSpecializations.push_back("Vec3Array");

	setAttributeAllowedSpecializations(_quat, quatSpecializations);
	setAttributeAllowedSpecializations(_axis, axisSpecializations);
	setAttributeAllowedSpecializations(_angle, angleSpecializations);

	addAttributeSpecializationLink(_quat, _axis);
	addAttributeSpecializationLink(_axis, _angle);
}

void QuatToAxisAngle::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB)
{
	if(specializationA.size() < specializationB.size()){
		std::string specB = specializationB[0];
		specializationB.resize(1);
		if(stringUtils::endswith(specializationA[0], "Array")){
			specializationB[0] = specB + "Array";
		}
		else{
			specializationB[0] = specB;
		}
	}
	else if(specializationB.size() < specializationA.size()){
		std::string specA = specializationA[0];
		specializationA.resize(1);
		
		if(stringUtils::endswith(specializationB[0], "Array")){
			specializationA[0] = specA + "Array";
		}
		else{
			specializationA[0] = specA;
		}
	}
}

void QuatToAxisAngle::update(Attribute *attribute)
{
	const std::vector<Imath::Quatf> &quatValues = _quat->value()->quatValues();
	int size = quatValues.size();

	std::vector<Imath::V3f> axisValues(size);
	std::vector<float> angleValues(size);

	for(int i = 0; i < size; ++i){
		const Imath::Quatf &quat = quatValues[i];
		axisValues[i] = quat.axis();
		angleValues[i] =quat.angle();
	}

	_axis->outValue()->setVec3Values(axisValues);
	_angle->outValue()->setFloatValues(angleValues);

	setAttributeIsClean(_axis, true);
	setAttributeIsClean(_angle, true);
}




