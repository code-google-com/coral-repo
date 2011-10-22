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
#include "../src/Node.h"
#include "../src/Attribute.h"
#include "../src/NumericAttribute.h"

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

class Vec3Normalize: public Node{
public:
	Vec3Normalize(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_vector;
	NumericAttribute *_normalized;
};

class Acos: public Node{
public:
	Acos(const std::string &name, Node *parent);
	void update(Attribute *attribute);

private:
	NumericAttribute *_inNumber;
	NumericAttribute *_outNumber;
};

}

#endif
