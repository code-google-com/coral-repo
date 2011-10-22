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
#include "NumericAttribute.h"
#include "stringUtils.h"

using namespace coral;

NumericAttribute::NumericAttribute(const std::string &name, Node *parent) : Attribute(name, parent){
	setClassName("NumericAttribute");
	setValuePtr(new Numeric());
	
	std::vector<std::string> allowedSpecialization;
	allowedSpecialization.push_back("Int");
	allowedSpecialization.push_back("IntArray");
	allowedSpecialization.push_back("Float");
	allowedSpecialization.push_back("FloatArray");
	allowedSpecialization.push_back("Vec3");
	allowedSpecialization.push_back("Vec3Array");
	allowedSpecialization.push_back("Quat");
	allowedSpecialization.push_back("QuatArray");
	allowedSpecialization.push_back("Matrix44");
	allowedSpecialization.push_back("Matrix44Array");
	setAllowedSpecialization(allowedSpecialization);
}

Numeric *NumericAttribute::value(){
	return (Numeric*)Attribute::value();
}

Numeric *NumericAttribute::outValue(){
	return (Numeric*)Attribute::outValue();
}

void NumericAttribute::onSettingSpecialization(const std::vector<std::string> &specialization){
	if(specialization.size() == 1){
		std::string typeStr = specialization[0];
		outValue()->setType(numericTypeFromString(typeStr));
	}
	else{
		outValue()->setType(Numeric::numericTypeAny);
	}
}

Numeric::Type NumericAttribute::numericTypeFromString(const std::string &typeStr){
	Numeric::Type type = Numeric::numericTypeAny;
	
	if(typeStr == "Int"){
		type = Numeric::numericTypeInt;
	}
	else if(typeStr == "IntArray"){
		type = Numeric::numericTypeIntArray;
	}
	else if(typeStr == "Float"){
		type = Numeric::numericTypeFloat;
	}
	else if(typeStr == "FloatArray"){
		type = Numeric::numericTypeFloatArray;
	}
	else if(typeStr == "Vec3"){
		type = Numeric::numericTypeVec3;
	}
	else if(typeStr == "Vec3Array"){
		type = Numeric::numericTypeVec3Array;
	}
	else if(typeStr == "Quat"){
		type = Numeric::numericTypeQuat;
	}
	else if(typeStr == "QuatArray"){
		type = Numeric::numericTypeQuatArray;
	}
	else if(typeStr == "Matrix44"){
		type = Numeric::numericTypeMatrix44;
	}
	else if(typeStr == "Matrix44Array"){
		type = Numeric::numericTypeMatrix44Array;
	}
	
	return type;
}

std::string NumericAttribute::debugInfo(){
	std::string info = Attribute::debugInfo() + "\n";
	Numeric *value = outValue();
	if(value->isArray()){
		info += "array size: " + stringUtils::intToString(value->size());
	}
	
	return info;
}
