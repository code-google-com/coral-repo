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
#include "BoolAttribute.h"
#include "stringUtils.h"

using namespace coral;

Bool::Bool():
	_isArray(false){
	_boolValues.resize(1);
	_boolValues[0] = false;
}

unsigned int Bool::size(){
	return _boolValues.size();
}

void Bool::setBoolValueAt(unsigned int id, bool value){
	if(id < _boolValues.size())
		_boolValues[id] = value;
}

bool Bool::boolValueAt(unsigned int id){
	bool value = false;

	if(id < _boolValues.size()){
		value = _boolValues[id];
	}
		
	return value;
}

const std::vector<bool> &Bool::boolValues(){
	return _boolValues;
}

void Bool::setBoolValues(const std::vector<bool> &values){
	_boolValues = values;
}

void Bool::setIsArray(bool value){
	_isArray = value;
}

bool Bool::isArray(){
	return _isArray;
}

void Bool::resize(unsigned int size){
	_boolValues.resize(size);
}

std::string Bool::asString(){
	std::string script;
	if(_isArray){
		script = "[";
		for(int i = 0; i < _boolValues.size(); ++i){
			std::string value = "False";
			if(_boolValues[i]){
				value = "True";
			}
		
			script += value + ",";
		
			if(i % 20 == 19)
				value += "\n";
		}
	
		script += "]";
	}
	else{
		bool value = boolValueAt(0);
		if(value){
			script = "True";
		}
		else{
			script = "False";
		}
	}
	
	return script;
}

void Bool::setFromString(const std::string &value){
	if(stringUtils::startswith(value, "[")){
		std::string tmp = stringUtils::replace(value, "\n", "");
		std::string valuesStr = stringUtils::strip(value, "[]");
		std::vector<std::string> values;
		stringUtils::split(valuesStr, values, ",");
	
		_boolValues.clear();
		_isArray = true;
		for(int i = 0; i < values.size(); ++i){
			std::string valueStr = values[i];
			if(valueStr == "True"){
				_boolValues.push_back(true);
			}
			else if(valueStr == "False"){
				_boolValues.push_back(false);
			}
		}
	}
	else{
		_isArray = false;
		_boolValues.resize(1);
		if(value == "True"){
			_boolValues[0] = true;
		}
		else if(value == "False"){
			_boolValues[1] = false;
		}
	}
}

BoolAttribute::BoolAttribute(const std::string &name, Node *parent) : Attribute(name, parent){
	setClassName("BoolAttribute");

	setValuePtr(new Bool());
	
	std::vector<std::string> allowedSpecialization;
	allowedSpecialization.push_back("Bool");
	setAllowedSpecialization(allowedSpecialization);
}

Bool *BoolAttribute::value(){
	return (Bool*)Attribute::value();
}
Bool *BoolAttribute::outValue(){
	return (Bool*)Attribute::outValue();
}

void BoolAttribute::onSettingSpecialization(const std::vector<std::string> &specialization){
	Bool *boolVal = outValue();

	boolVal->setIsArray(false);
	if(boolVal->size() == 0){
		boolVal->resize(1);
		boolVal->setBoolValueAt(0, false);
	}

	if(specialization.size() == 1){
		if(specialization[0] == "BoolArray"){
			boolVal->setIsArray(true);
		}
	}
}

