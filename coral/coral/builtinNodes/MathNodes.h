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

#ifndef CORAL_MATHNODES_H
#define CORAL_MATHNODES_H

#include <vector>
#include <cmath>
#include "../src/Node.h"
#include "../src/Attribute.h"
#include "../src/NumericAttribute.h"
#include "../src/EnumAttribute.h"

namespace coral{

class Vec3Length: public Node{
public:
	Vec3Length(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);
	
private:
	NumericAttribute *_vector;
	NumericAttribute *_length;
};

class Matrix44Inverse: public Node{
public:
	Matrix44Inverse(const std::string &name, Node *parent);
	void update(Attribute *attribute);
	
private:
	NumericAttribute *_inMatrix;
	NumericAttribute *_outMatrix;
};

class Abs: public Node{
public:
	Abs(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
	void(Abs::*_selectedOperation)(Numeric*, Numeric*);
	
	void abs_int(Numeric *inNumber, Numeric *outNumber);
	void abs_float(Numeric *inNumber, Numeric *outNumber);
};

class Vec3Cross: public Node{
public:
	Vec3Cross(const std::string &name, Node *parent);
	void update(Attribute *attribute);
	
private:
	NumericAttribute *_vector0;
	NumericAttribute *_vector1;
	NumericAttribute *_crossProduct;
};

class Vec3Dot: public Node{
public:
	Vec3Dot(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);

private:
	NumericAttribute *_vector0;
	NumericAttribute *_vector1;
	NumericAttribute *_dotProduct;
};

class Vec3Normalize: public Node{
public:
	Vec3Normalize(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_vector;
	NumericAttribute *_normalized;
};

class TrigonometricFunctions: public Node{
public:

	TrigonometricFunctions(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
	EnumAttribute *_function;
};

class Radians: public Node{
public:
	Radians(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Degrees: public Node{
public:
	Degrees(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Ceil: public Node{
public:
	Ceil(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Floor: public Node{
public:
	Floor(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Round: public Node{
public:
	Round(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Exp: public Node{
public:
	Exp(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Log: public Node{
public:
	Log(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Pow: public Node{
public:
	Pow(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_base;
	NumericAttribute *_exponent;
	NumericAttribute *_outNumber;
};

class Log10: public Node{
public:
	Log10(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Sqrt: public Node{
public:
	Sqrt(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

class Atan2: public Node{
public:
	Atan2(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumberX;
	NumericAttribute *_inNumberY;
	NumericAttribute *_outNumber;
};

class Min: public Node{
public:
	Min(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
	void(Min::*_selectedOperation)(Numeric*, Numeric*);

	void min_int(Numeric *inNumber, Numeric *outNumber);
	void min_float(Numeric *inNumber, Numeric *outNumber);
};
class Max: public Node{
public:
	Max(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
	void(Max::*_selectedOperation)(Numeric*, Numeric*);

	void max_int(Numeric *inNumber, Numeric *outNumber);
	void max_float(Numeric *inNumber, Numeric *outNumber);
};

class Average: public Node{
public:
	Average(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
	void(Average::*_selectedOperation)(Numeric*, Numeric*);

	void average_int(Numeric *inNumber, Numeric *outNumber);
	void average_float(Numeric *inNumber, Numeric *outNumber);
	void average_vec3(Numeric *inNumber, Numeric *outNumber);
};

class Slerp: public Node{
public:
	Slerp(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inQuat1;
	NumericAttribute *_inQuat2;
	NumericAttribute *_param;
	NumericAttribute *_outNumber;
};

class QuatMultiply: public Node{
public:
	QuatMultiply(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_quat0;
	NumericAttribute *_quat1;
	NumericAttribute *_outQuat;
};

class QuatNormalize: public Node{
public:
	QuatNormalize(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_quat0;
	NumericAttribute *_normalized;
};



}

#endif
