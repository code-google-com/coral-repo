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

#include "ConditionalNodes.h"
#include "../src/stringUtils.h"

using namespace coral;

namespace {

void greaterOrLessUpdateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	bool array = false;
	bool single = false;
		
	if(specializationA.size() < specializationB.size()){	
		for(int i = 0; i < specializationA.size(); ++i){
			if(stringUtils::endswith(specializationA[i], "Array")){
				array = true;
			}
			else{
				single = true;
			}
		}
	}
	else if(specializationB.size() < specializationA.size()){
		for(int i = 0; i < specializationB.size(); ++i){
			if(stringUtils::endswith(specializationB[i], "Array")){
				array = true;
			}
			else{
				single = true;
			}
		}
	}
	
	if(array != single){
		std::vector<std::string> newSpecializationA;
		std::vector<std::string> newSpecializationB;
		
		for(int i = 0; i < specializationA.size(); ++i){
			bool arraySpec = stringUtils::endswith(specializationA[i], "Array");
			if(arraySpec && array){
				newSpecializationA.push_back(specializationA[i]);
			}
			else if(!arraySpec && single){
				newSpecializationA.push_back(specializationA[i]);
			}
		}
		
		for(int i = 0; i < specializationB.size(); ++i){
			bool arraySpec = stringUtils::endswith(specializationB[i], "Array");
			if(arraySpec && array){
				newSpecializationB.push_back(specializationB[i]);
			}
			else if(!arraySpec && single){
				newSpecializationB.push_back(specializationB[i]);
			}
		}
		
		specializationA = newSpecializationA;
		specializationB = newSpecializationB;
	}
}

}

IfGreaterThan::IfGreaterThan(const std::string &name, Node *parent): Node(name, parent){	
	_selectedOperation = 0;
	
	_in0 = new NumericAttribute("in0", this);
	_in1 = new NumericAttribute("in1", this);
	_out = new BoolAttribute("out", this);
	
	addInputAttribute(_in0);
	addInputAttribute(_in1);
	addOutputAttribute(_out);
	
	setAttributeAffect(_in0, _out);
	setAttributeAffect(_in1, _out);
	
	std::vector<std::string> numericSpecialization;
	numericSpecialization.push_back("Int");
	numericSpecialization.push_back("IntArray");
	numericSpecialization.push_back("Float");
	numericSpecialization.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_in0, numericSpecialization);
	setAttributeAllowedSpecializations(_in1, numericSpecialization);
	
	std::vector<std::string> boolSpecialization;
	boolSpecialization.push_back("Bool");
	boolSpecialization.push_back("BoolArray");
	
	setAttributeAllowedSpecializations(_out, boolSpecialization);
	
	addAttributeSpecializationLink(_in0, _out);
}

void IfGreaterThan::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	greaterOrLessUpdateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
}

void IfGreaterThan::attributeSpecializationChanged(Attribute *attribute){
	Numeric *in0 = _in0->outValue();
	Numeric *in1 = _in1->outValue();
	Numeric::Type typeA = in0->type();
	Numeric::Type typeB = in1->type();
	_selectedOperation = 0;
	
	if(typeA != Numeric::numericTypeAny && typeB != Numeric::numericTypeAny){
		if(in0->isArray() == in1->isArray()){
			if((typeA == Numeric::numericTypeInt || typeA == Numeric::numericTypeIntArray) && (typeB == Numeric::numericTypeInt || typeB == Numeric::numericTypeIntArray)){
				_selectedOperation = &IfGreaterThan::intGreaterThanInt_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeInt || typeA == Numeric::numericTypeIntArray) && (typeB == Numeric::numericTypeFloat || typeB == Numeric::numericTypeFloatArray)){
				_selectedOperation = &IfGreaterThan::intGreaterThanFloat_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeFloat || typeA == Numeric::numericTypeFloatArray) && (typeB == Numeric::numericTypeFloat || typeB == Numeric::numericTypeFloatArray)){
				_selectedOperation = &IfGreaterThan::floatGreaterThanFloat_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeFloat || typeA == Numeric::numericTypeFloatArray) && (typeB == Numeric::numericTypeInt || typeB == Numeric::numericTypeIntArray)){
				_selectedOperation = &IfGreaterThan::floatGreaterThanInt_arrayToArray;
			}
		}
		else if(in0->isArray() && in1->isArray() == false){
			if(typeA == Numeric::numericTypeIntArray && typeB == Numeric::numericTypeInt){
				_selectedOperation = &IfGreaterThan::intGreaterThanInt_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeIntArray && typeB == Numeric::numericTypeFloat){
				_selectedOperation = &IfGreaterThan::intGreaterThanFloat_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeFloatArray && typeB == Numeric::numericTypeFloat){
				_selectedOperation = &IfGreaterThan::floatGreaterThanFloat_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeFloatArray && typeB == Numeric::numericTypeInt){
				_selectedOperation = &IfGreaterThan::floatGreaterThanInt_arrayToSingle;
			}
		}
		else if(in0->isArray() == false && in1->isArray()){
			if(typeA == Numeric::numericTypeInt && typeB == Numeric::numericTypeIntArray){
				_selectedOperation = &IfGreaterThan::intGreaterThanInt_singleToArray;
			}
			else if(typeA == Numeric::numericTypeInt && typeB == Numeric::numericTypeFloatArray){
				_selectedOperation = &IfGreaterThan::intGreaterThanFloat_singleToArray;
			}
			else if(typeA == Numeric::numericTypeFloat && typeB == Numeric::numericTypeFloatArray){
				_selectedOperation = &IfGreaterThan::floatGreaterThanFloat_singleToArray;
			}
			else if(typeA == Numeric::numericTypeFloat && typeB == Numeric::numericTypeIntArray){
				_selectedOperation = &IfGreaterThan::floatGreaterThanInt_singleToArray;
			}
		}
	}
}

void IfGreaterThan::intGreaterThanInt_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_greaterThan_arrayToArray<int, int>(in0->intValues(), in1->intValues(), minorSize, out);
}

void IfGreaterThan::intGreaterThanFloat_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_greaterThan_arrayToArray<int, float>(in0->intValues(), in1->floatValues(), minorSize, out);
}

void IfGreaterThan::floatGreaterThanFloat_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_greaterThan_arrayToArray<float, float>(in0->floatValues(), in1->floatValues(), minorSize, out);
}

void IfGreaterThan::floatGreaterThanInt_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_greaterThan_arrayToArray<float, int>(in0->floatValues(), in1->intValues(), minorSize, out);
}

void IfGreaterThan::intGreaterThanInt_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_greaterThan_arrayToSingle<int, int>(in0->intValues(), in1->intValueAt(0), size, out);
}

void IfGreaterThan::intGreaterThanFloat_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_greaterThan_arrayToSingle<int, float>(in0->intValues(), in1->floatValueAt(0), size, out);
}

void IfGreaterThan::floatGreaterThanFloat_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_greaterThan_arrayToSingle<float, float>(in0->floatValues(), in1->floatValueAt(0), size, out);
}

void IfGreaterThan::floatGreaterThanInt_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_greaterThan_arrayToSingle<float, int>(in0->floatValues(), in1->intValueAt(0), size, out);
}

void IfGreaterThan::intGreaterThanInt_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_greaterThan_singleToArray<int, int>(in0->intValueAt(0), in1->intValues(), out);
}

void IfGreaterThan::intGreaterThanFloat_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_greaterThan_singleToArray<int, float>(in0->intValueAt(0), in1->floatValues(), out);
}

void IfGreaterThan::floatGreaterThanFloat_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_greaterThan_singleToArray<float, float>(in0->floatValueAt(0), in1->floatValues(), out);
}

void IfGreaterThan::floatGreaterThanInt_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_greaterThan_singleToArray<float, int>(in0->floatValueAt(0), in1->intValues(), out);
}

void IfGreaterThan::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *in0 = _in0->value();
		Numeric *in1 = _in1->value();
	
		std::vector<bool> outValues;
		
		(this->*_selectedOperation)(in0, in1, outValues);
		
		_out->outValue()->setBoolValues(outValues);
	}
	else{
		setAttributeIsClean(_out, false);
	}
}

// less than node
IfLessThan::IfLessThan(const std::string &name, Node *parent): Node(name, parent){	
	_selectedOperation = 0;
	
	_in0 = new NumericAttribute("in0", this);
	_in1 = new NumericAttribute("in1", this);
	_out = new BoolAttribute("out", this);
	
	addInputAttribute(_in0);
	addInputAttribute(_in1);
	addOutputAttribute(_out);
	
	setAttributeAffect(_in0, _out);
	setAttributeAffect(_in1, _out);
	
	std::vector<std::string> numericSpecialization;
	numericSpecialization.push_back("Int");
	numericSpecialization.push_back("IntArray");
	numericSpecialization.push_back("Float");
	numericSpecialization.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_in0, numericSpecialization);
	setAttributeAllowedSpecializations(_in1, numericSpecialization);
	
	std::vector<std::string> boolSpecialization;
	boolSpecialization.push_back("Bool");
	boolSpecialization.push_back("BoolArray");
	
	setAttributeAllowedSpecializations(_out, boolSpecialization);
	
	addAttributeSpecializationLink(_in0, _out);
}

void IfLessThan::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	greaterOrLessUpdateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
}

void IfLessThan::intLessThanInt_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_lessThan_arrayToArray<int, int>(in0->intValues(), in1->intValues(), minorSize, out);
}

void IfLessThan::intLessThanFloat_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_lessThan_arrayToArray<int, float>(in0->intValues(), in1->floatValues(), minorSize, out);
}

void IfLessThan::floatLessThanFloat_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_lessThan_arrayToArray<float, float>(in0->floatValues(), in1->floatValues(), minorSize, out);
}

void IfLessThan::floatLessThanInt_arrayToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int sizes[] = {in0->size(), in1->size()};
	int minorSize = mathUtils::findMinorInt(sizes, 2);
	out.resize(minorSize);
	conditionalOperation_lessThan_arrayToArray<float, int>(in0->floatValues(), in1->intValues(), minorSize, out);
}

void IfLessThan::intLessThanInt_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_lessThan_arrayToSingle<int, int>(in0->intValues(), in1->intValueAt(0), size, out);
}

void IfLessThan::intLessThanFloat_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_lessThan_arrayToSingle<int, float>(in0->intValues(), in1->floatValueAt(0), size, out);
}

void IfLessThan::floatLessThanFloat_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_lessThan_arrayToSingle<float, float>(in0->floatValues(), in1->floatValueAt(0), size, out);
}

void IfLessThan::floatLessThanInt_arrayToSingle(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	int size = in0->size();
	out.resize(size);
	conditionalOperation_lessThan_arrayToSingle<float, int>(in0->floatValues(), in1->intValueAt(0), size, out);
}

void IfLessThan::intLessThanInt_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_lessThan_singleToArray<int, int>(in0->intValueAt(0), in1->intValues(), out);
}

void IfLessThan::intLessThanFloat_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_lessThan_singleToArray<int, float>(in0->intValueAt(0), in1->floatValues(), out);
}

void IfLessThan::floatLessThanFloat_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_lessThan_singleToArray<float, float>(in0->floatValueAt(0), in1->floatValues(), out);
}

void IfLessThan::floatLessThanInt_singleToArray(Numeric *in0, Numeric *in1, std::vector<bool> &out){
	out.resize(1);
	conditionalOperation_lessThan_singleToArray<float, int>(in0->floatValueAt(0), in1->intValues(), out);
}

void IfLessThan::attributeSpecializationChanged(Attribute *attribute){
	Numeric *in0 = _in0->outValue();
	Numeric *in1 = _in1->outValue();
	Numeric::Type typeA = in0->type();
	Numeric::Type typeB = in1->type();
	_selectedOperation = 0;
	
	if(typeA != Numeric::numericTypeAny && typeB != Numeric::numericTypeAny){
		if(in0->isArray() == in1->isArray()){
			if((typeA == Numeric::numericTypeInt || typeA == Numeric::numericTypeIntArray) && (typeB == Numeric::numericTypeInt || typeB == Numeric::numericTypeIntArray)){
				_selectedOperation = &IfLessThan::intLessThanInt_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeInt || typeA == Numeric::numericTypeIntArray) && (typeB == Numeric::numericTypeFloat || typeB == Numeric::numericTypeFloatArray)){
				_selectedOperation = &IfLessThan::intLessThanFloat_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeFloat || typeA == Numeric::numericTypeFloatArray) && (typeB == Numeric::numericTypeFloat || typeB == Numeric::numericTypeFloatArray)){
				_selectedOperation = &IfLessThan::floatLessThanFloat_arrayToArray;
			}
			else if((typeA == Numeric::numericTypeFloat || typeA == Numeric::numericTypeFloatArray) && (typeB == Numeric::numericTypeInt || typeB == Numeric::numericTypeIntArray)){
				_selectedOperation = &IfLessThan::floatLessThanInt_arrayToArray;
			}
		}
		else if(in0->isArray() && in1->isArray() == false){
			if(typeA == Numeric::numericTypeIntArray && typeB == Numeric::numericTypeInt){
				_selectedOperation = &IfLessThan::intLessThanInt_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeIntArray && typeB == Numeric::numericTypeFloat){
				_selectedOperation = &IfLessThan::intLessThanFloat_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeFloatArray && typeB == Numeric::numericTypeFloat){
				_selectedOperation = &IfLessThan::floatLessThanFloat_arrayToSingle;
			}
			else if(typeA == Numeric::numericTypeFloatArray && typeB == Numeric::numericTypeInt){
				_selectedOperation = &IfLessThan::floatLessThanInt_arrayToSingle;
			}
		}
		else if(in0->isArray() == false && in1->isArray()){
			if(typeA == Numeric::numericTypeInt && typeB == Numeric::numericTypeIntArray){
				_selectedOperation = &IfLessThan::intLessThanInt_singleToArray;
			}
			else if(typeA == Numeric::numericTypeInt && typeB == Numeric::numericTypeFloatArray){
				_selectedOperation = &IfLessThan::intLessThanFloat_singleToArray;
			}
			else if(typeA == Numeric::numericTypeFloat && typeB == Numeric::numericTypeFloatArray){
				_selectedOperation = &IfLessThan::floatLessThanFloat_singleToArray;
			}
			else if(typeA == Numeric::numericTypeFloat && typeB == Numeric::numericTypeIntArray){
				_selectedOperation = &IfLessThan::floatLessThanInt_singleToArray;
			}
		}
	}
}

void IfLessThan::update(Attribute *attribute){
	if(_selectedOperation){
		Numeric *in0 = _in0->value();
		Numeric *in1 = _in1->value();
	
		std::vector<bool> outValues;
		
		(this->*_selectedOperation)(in0, in1, outValues);
		
		_out->outValue()->setBoolValues(outValues);
	}
	else{
		setAttributeIsClean(_out, false);
	}
}

// conditional value node

ConditionalValue::ConditionalValue(const std::string &name, Node *parent): Node(name, parent){	
	_selectedOperation = 0;
	
	_condition = new BoolAttribute("condition", this);
	_ifTrue = new NumericAttribute("ifTrue", this);
	_ifFalse = new NumericAttribute("ifFalse", this);
	_out = new NumericAttribute("out", this);
	
	addInputAttribute(_condition);
	addInputAttribute(_ifTrue);
	addInputAttribute(_ifFalse);
	addOutputAttribute(_out);
	
	setAttributeAffect(_condition, _out);
	setAttributeAffect(_ifTrue, _out);
	setAttributeAffect(_ifFalse, _out);
	
	addAttributeSpecializationLink(_condition, _ifTrue);
	addAttributeSpecializationLink(_ifTrue, _out);
	addAttributeSpecializationLink(_ifTrue, _ifFalse);
	
	std::vector<std::string> boolSpecialization;
	boolSpecialization.push_back("Bool");
	boolSpecialization.push_back("BoolArray");
	
	setAttributeAllowedSpecializations(_condition, boolSpecialization);
}

void ConditionalValue::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _condition && attributeB == _ifTrue){
		if(specializationA.size() == 1){
			if(specializationA[0] == "BoolArray"){ // specialize attributeB to array only
				std::vector<std::string> newSpecializationB;
				for(int i = 0; i < specializationB.size(); ++i){
					if(stringUtils::endswith(specializationB[i], "Array")){
						newSpecializationB.push_back(specializationB[i]);
					}
				}
				specializationB = newSpecializationB;
			}
		}
		else if(specializationB.size() == 1){
			specializationA.resize(1);
			if(stringUtils::endswith(specializationB[0], "Array")){
				specializationA[0] = "BoolArray";
			}
			else{
				specializationA[0] = "Bool";
			}
		}
	}
	else{
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
}

void ConditionalValue::transferValuesInt(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<int> outValues;
	conditionalValueTransfer<int>(condition->boolValueAt(0), ifTrue->intValues(), ifFalse->intValues(), outValues);
	out->setIntValues(outValues);
}

void ConditionalValue::transferValuesFloat(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<float> outValues;
	conditionalValueTransfer<float>(condition->boolValueAt(0), ifTrue->floatValues(), ifFalse->floatValues(), outValues);
	out->setFloatValues(outValues);
}

void ConditionalValue::transferValuesVec3(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::V3f> outValues;
	conditionalValueTransfer<Imath::V3f>(condition->boolValueAt(0), ifTrue->vec3Values(), ifFalse->vec3Values(), outValues);
	out->setVec3Values(outValues);
}

void ConditionalValue::transferValuesCol4(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::Color4f> outValues;
	conditionalValueTransfer<Imath::Color4f>(condition->boolValueAt(0), ifTrue->col4Values(), ifFalse->col4Values(), outValues);
	out->setCol4Values(outValues);
}

void ConditionalValue::transferValuesMatrix44(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::M44f> outValues;
	conditionalValueTransfer<Imath::M44f>(condition->boolValueAt(0), ifTrue->matrix44Values(), ifFalse->matrix44Values(), outValues);
	out->setMatrix44Values(outValues);
}

void ConditionalValue::transferValuesIntBoolArray(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<int> outValues;
	conditionalValueTransferBoolArray<int>(condition->boolValues(), ifTrue->intValues(), ifFalse->intValues(), outValues);
	out->setIntValues(outValues);
}

void ConditionalValue::transferValuesFloatBoolArray(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<float> outValues;
	conditionalValueTransferBoolArray<float>(condition->boolValues(), ifTrue->floatValues(), ifFalse->floatValues(), outValues);
	out->setFloatValues(outValues);
}

void ConditionalValue::transferValuesVec3BoolArray(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::V3f> outValues;
	conditionalValueTransferBoolArray<Imath::V3f>(condition->boolValues(), ifTrue->vec3Values(), ifFalse->vec3Values(), outValues);
	out->setVec3Values(outValues);
}

void ConditionalValue::transferValuesCol4BoolArray(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::Color4f> outValues;
	conditionalValueTransferBoolArray<Imath::Color4f>(condition->boolValues(), ifTrue->col4Values(), ifFalse->col4Values(), outValues);
	out->setCol4Values(outValues);
}

void ConditionalValue::transferValuesMatrix44BoolArray(Bool *condition, Numeric *ifTrue, Numeric *ifFalse, Numeric *out){
	std::vector<Imath::M44f> outValues;
	conditionalValueTransferBoolArray<Imath::M44f>(condition->boolValues(), ifTrue->matrix44Values(), ifFalse->matrix44Values(), outValues);
	out->setMatrix44Values(outValues);
}

void ConditionalValue::attributeSpecializationChanged(Attribute *attribute){
	Numeric::Type type = _ifTrue->outValue()->type();
	_selectedOperation = 0;
	
	if(type != Numeric::numericTypeAny && _ifFalse->outValue()->type() != Numeric::numericTypeAny){
		if(_condition->outValue()->isArray() == false){
			if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
				_selectedOperation = &ConditionalValue::transferValuesInt;
			}
			else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
				_selectedOperation = &ConditionalValue::transferValuesFloat;
			}
			else if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
				_selectedOperation = &ConditionalValue::transferValuesVec3;
			}
			else if(type == Numeric::numericTypeCol4 || type == Numeric::numericTypeCol4Array){
				_selectedOperation = &ConditionalValue::transferValuesCol4;
			}
			else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &ConditionalValue::transferValuesMatrix44;
			}
		}
		else{
			if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
				_selectedOperation = &ConditionalValue::transferValuesIntBoolArray;
			}
			else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
				_selectedOperation = &ConditionalValue::transferValuesFloatBoolArray;
			}
			else if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
				_selectedOperation = &ConditionalValue::transferValuesVec3BoolArray;
			}
			else if(type == Numeric::numericTypeCol4 || type == Numeric::numericTypeCol4Array){
				_selectedOperation = &ConditionalValue::transferValuesCol4BoolArray;
			}
			else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &ConditionalValue::transferValuesMatrix44BoolArray;
			}
		}
	}
}

void ConditionalValue::update(Attribute *attribute){
	if(_selectedOperation){
		Bool *condition = _condition->value();
		Numeric *ifTrue = _ifTrue->value();
		Numeric *ifFalse = _ifFalse->value();
		Numeric *out = _out->outValue();
	
		(this->*_selectedOperation)(condition, ifTrue, ifFalse, out);
	}
	else{
		setAttributeIsClean(_out, false);
	}
}

