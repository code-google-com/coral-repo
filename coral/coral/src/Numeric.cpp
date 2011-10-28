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
#include "Numeric.h"
#include <assert.h>
#include "stringUtils.h"

using namespace coral;

Numeric::Numeric():
	_type(numericTypeAny),
	_isArray(false),
	_size(1){
	
	// giving first initial size of 1 to allow for default values to be there before the type is set.
	_intValues.resize(1);
	_intValues[0] = 0;
	
	_floatValues.resize(1);
	_floatValues[0] = 0.0;
	
	_vec3Values.resize(1);
	_vec3Values[0] = Imath::V3f(0.0, 0.0, 0.0);
	
	_quatValues.resize(1);
	_quatValues[0] = Imath::Quatf();

	_matrix44Values.resize(1);
	_matrix44Values[0] = Imath::M44f();
}

void Numeric::copy(const Value *other){
	const Numeric *otherNum = dynamic_cast<const Numeric*>(other);
	
	if(otherNum){
		_type = otherNum->_type;
		_isArray = otherNum->_isArray;
		_size = otherNum->_size;
		_intValues = otherNum->_intValues;
		_floatValues = otherNum->_floatValues;
		_vec3Values = otherNum->_vec3Values;
		_quatValues = otherNum->_quatValues;
		_matrix44Values = otherNum->_matrix44Values;
	}
}

bool Numeric::isArray(){
	return _isArray;
}

unsigned int Numeric::size(){
	return _size;
}

void Numeric::copyFromOther(Numeric *other){
	_type = other->_type;
	_isArray = other->_isArray;
	_intValues = other->_intValues;
	_floatValues = other->_floatValues;
	_vec3Values = other->_vec3Values;
	_quatValues = other->_quatValues;
}

Numeric::Type Numeric::type(){
	return _type;
}

void Numeric::setType(Numeric::Type type){
	_type = type;
	_size = 0;
	_isArray = false;
	
	if(type == numericTypeInt){
		_intValues.resize(1);
		_size = 1;
		_isArray = false;
	}
	else if(type == numericTypeIntArray){
		_size = _intValues.size();
		_isArray = true;
	}
	else if(type == numericTypeFloat){
		_floatValues.resize(1);
		_size = 1;
		_isArray = false;
	}
	else if(type == numericTypeFloatArray){
		_size = _floatValues.size();
		_isArray = true;
	}
	else if(type == numericTypeVec3){
		_vec3Values.resize(1);
		_size = 1;
		_isArray = false;
	}
	else if(type == numericTypeVec3Array){
		_size = _vec3Values.size();
		_isArray = true;
	}
	else if(type == numericTypeQuat){
			_quatValues.resize(1);
			_size = 1;
			_isArray = false;
		}
		else if(type == numericTypeQuatArray){
			_size = _quatValues.size();
			_isArray = true;
		}
	else if(type == numericTypeMatrix44){
		_matrix44Values.resize(1);
		_size = 1;
		_isArray = false;
	}
	else if(type == numericTypeMatrix44Array){
		_size = _matrix44Values.size();
		_isArray = true;
	}
}

void Numeric::resize(unsigned int newSize){
	if(_type != numericTypeAny){
		if(_type == numericTypeInt || _type == numericTypeIntArray){
			_intValues.resize(newSize);
		}
		else if(_type == numericTypeFloat || _type == numericTypeFloatArray){
			_floatValues.resize(newSize);
		}
		else if(_type == numericTypeVec3 || _type == numericTypeVec3Array){
			_vec3Values.resize(newSize);
		}
		else if(_type == numericTypeQuat || _type == numericTypeQuatArray){
			_quatValues.resize(newSize);
		}
		else if(_type == numericTypeMatrix44 || _type == numericTypeMatrix44Array){
			_matrix44Values.resize(newSize);
		}
		_size = newSize;
	}
}

bool Numeric::isArrayType(Numeric::Type type){
	bool arrayType = false;
	if(
		type == numericTypeIntArray ||
		type == numericTypeFloatArray ||
		type == numericTypeVec3Array ||
		type == numericTypeQuatArray ||
		type == numericTypeMatrix44Array){
		
		arrayType = true;
	}
	
	return arrayType;
}

void Numeric::setIntValueAt(unsigned int id, int value){
	if(id < _intValues.size())
		_intValues[id] = value;
}

void Numeric::setFloatValueAt(unsigned int id, float value){
	if(id < _floatValues.size())
		_floatValues[id] = value;
}

void Numeric::setVec3ValueAt(unsigned int id, const Imath::V3f &value){
	if(id < _vec3Values.size())
		_vec3Values[id] = value;
}

void Numeric::setQuatValueAt(unsigned int id, const Imath::Quatf &value){
	if(id < _quatValues.size())
		_quatValues[id] = value;
}

void Numeric::setMatrix44ValueAt(unsigned int id, const Imath::M44f &value){
	if(id < _matrix44Values.size())
		_matrix44Values[id] = value;
}

const std::vector<int> &Numeric::intValues(){
	return _intValues;
}

const std::vector<float> &Numeric::floatValues(){
	return _floatValues;
}

const std::vector<Imath::V3f> &Numeric::vec3Values(){
	return _vec3Values;
}

const std::vector<Imath::Quatf> &Numeric::quatValues(){
	return _quatValues;
}

const std::vector<Imath::M44f> &Numeric::matrix44Values(){
	return _matrix44Values;
}

int Numeric::intValueAt(unsigned int id){
	int size = _intValues.size();
	if(id < size){
		return _intValues[id];
	}
	else if(size > 0){
		return _intValues[size - 1];
	}
		
	return 0;
}

float Numeric::floatValueAt(unsigned int id){
	int size = _floatValues.size();
	if(id < size){
		return _floatValues[id];
	}
	else if(size > 0){
		return _floatValues[size - 1];
	}
		
	return 0.0;
}

Imath::V3f Numeric::vec3ValueAt(unsigned int id){
	int size = _vec3Values.size();
	if(id < size){
		return _vec3Values[id];
	}
	else if(size > 0){
		return _vec3Values[size - 1];
	}
	
	return Imath::V3f(0.0, 0.0, 0.0);
}

Imath::Quatf Numeric::quatValueAt(unsigned int id){
	int size = _quatValues.size();
	if(id < size){
		return _quatValues[id];
	}
	else if(size > 0){
		return _quatValues[size - 1];
	}

	return Imath::Quatf();
}

Imath::M44f Numeric::matrix44ValueAt(unsigned int id){
	int size = _matrix44Values.size();
	if(id < size){
		return _matrix44Values[id];
	}
	else if(size > 0){
		return _matrix44Values[size - 1];
	}
	
	return Imath::M44f();
}

void Numeric::setIntValues(const std::vector<int> &values){
	_intValues = values;
	_size = _intValues.size();
}

void Numeric::setFloatValues(const std::vector<float> &values){
	_floatValues = values;
	_size = _floatValues.size();
}

void Numeric::setVec3Values(const std::vector<Imath::V3f> &values){
	_vec3Values = values;
	_size = _vec3Values.size();
}

void Numeric::setQuatValues(const std::vector<Imath::Quatf> &values){
	_quatValues = values;
	_size = _quatValues.size();
}

void Numeric::setMatrix44Values(const std::vector<Imath::M44f> &values){
	_matrix44Values = values;
	_size = _matrix44Values.size();
}

std::string Numeric::asString(){
	std::string script;
	
	if(_type != numericTypeAny){
		std::string value;
		std::string type;
		
		if(_type == numericTypeInt || _type == numericTypeIntArray){
			char buffer[sizeof(int)];
			for(int i = 0; i < _intValues.size(); ++i){
				sprintf(buffer, "%i", _intValues[i]);
				value += std::string(buffer);
				
				if(i < _intValues.size() - 1){
					value += ",";
				}
				
				if(i % 20 == 19)
					value += "\n";
			}
		}
		else if(_type == numericTypeFloat || _type == numericTypeFloatArray){
			char buffer[sizeof(float)];
			for(int i = 0; i < _floatValues.size(); ++i){
				sprintf(buffer, "%f", _floatValues[i]);
				value += std::string(buffer);
				
				if(i < _floatValues.size() - 1){
					value += ",";
				}
				
				if(i % 20 == 19)
					value += "\n";
			}
		}
		else if(_type == numericTypeVec3 || _type == numericTypeVec3Array){
			char buffer[sizeof(float)];
			for(int i = 0; i < _vec3Values.size(); ++i){
				value += "(";
				Imath::V3f *vec = &_vec3Values[i];
			
				sprintf(buffer, "%f", vec->x);
				value += std::string(buffer) + ",";
			
				sprintf(buffer, "%f", vec->y);
				value += std::string(buffer) + ",";
			
				sprintf(buffer, "%f", vec->z);
				value += std::string(buffer) + ")";
				
				if(i < _vec3Values.size() - 1){
					value += ",";
				}
				
				if(i % 20 == 19)
					value += "\n";
			}
		}
		else if(_type == numericTypeQuat || _type == numericTypeQuatArray){
			char buffer[sizeof(float)];
			for(int i = 0; i < _quatValues.size(); ++i){
				value += "(";
				Imath::Quatf *quat = &_quatValues[i];

				sprintf(buffer, "%f", quat->r);
				value += std::string(buffer) + ",";

				sprintf(buffer, "%f", quat->v.x);
				value += std::string(buffer) + ",";

				sprintf(buffer, "%f", quat->v.y);
				value += std::string(buffer) + ",";

				sprintf(buffer, "%f", quat->v.z);
				value += std::string(buffer) + ")";

				if(i < _quatValues.size() - 1){
					value += ",";
				}

				if(i % 20 == 19)
					value += "\n";
			}
		}
		else if(_type == numericTypeMatrix44 || _type == numericTypeMatrix44Array){
			char buffer[sizeof(float)];
			for(int i = 0; i < _matrix44Values.size(); ++i){
				value += "(";
				Imath::M44f *mat = &_matrix44Values[i];
				
				sprintf(buffer, "%f", mat->x[0][0]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[0][1]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[0][2]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[0][3]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[1][0]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[1][1]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[1][2]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[1][3]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[2][0]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[2][1]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[2][2]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[2][3]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[3][0]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[3][1]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[3][2]);
				value += std::string(buffer) + ",";
				
				sprintf(buffer, "%f", mat->x[3][3]);
				value += std::string(buffer) + ")";
				
				if(i < _vec3Values.size() - 1){
					value += ",";
				}
				
				if(i % 20 == 19)
					value += "\n";
			}
		}
		
		char buffer[sizeof(int)];
		sprintf(buffer, "%i", int(_type));
		type = std::string(buffer);
		
		script = "[" + value + "] " + type;
	}
	
	return script;
}

void Numeric::setFromString(const std::string &value){
	std::string tmp = stringUtils::replace(value, "\n", "");
	std::vector<std::string> fields;
	stringUtils::split(tmp, fields, " ");
	
	if(fields.size() == 2){
		std::string valuesStr = stringUtils::strip(fields[0], "[]");
		Numeric::Type type = Numeric::Type(stringUtils::parseInt(fields[1]));
		
		if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
			std::vector<std::string> values;
			stringUtils::split(valuesStr, values, ",");
			_intValues.clear();
			for(int i = 0; i < values.size(); ++i){
				int value = stringUtils::parseInt(values[i]);
				_intValues.push_back(value);
			}
		
			_size = _intValues.size();
		}
		else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
			std::vector<std::string> values;
			stringUtils::split(valuesStr, values, ",");
			_floatValues.clear();
			for(int i = 0; i < values.size(); ++i){
				float value = stringUtils::parseFloat(values[i]);
				_floatValues.push_back(value);
			}
			
			_size = _floatValues.size();
		}
		else if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
			_vec3Values.clear();
			
			std::vector<std::string> values;
			stringUtils::split(valuesStr, values, "),(");

			for(int i = 0; i < values.size(); ++i){
				std::string numericValuesStr = stringUtils::strip(values[i], "()");
				std::vector<std::string> numericValues;
				stringUtils::split(numericValuesStr, numericValues, ",");
				
				if(numericValues.size() == 3){
					float x = stringUtils::parseFloat(numericValues[0]);
					float y = stringUtils::parseFloat(numericValues[1]);
					float z = stringUtils::parseFloat(numericValues[2]);
					
					Imath::V3f vec(x, y, z);
					_vec3Values.push_back(vec);
				}
			}
		
			_size = _vec3Values.size();
		}
		else if(type == Numeric::numericTypeQuat || type == Numeric::numericTypeQuatArray){
			_quatValues.clear();

			std::vector<std::string> values;
			stringUtils::split(valuesStr, values, "),(");

			for(int i = 0; i < values.size(); ++i){
				std::string numericValuesStr = stringUtils::strip(values[i], "()");
				std::vector<std::string> numericValues;
				stringUtils::split(numericValuesStr, numericValues, ",");

				if(numericValues.size() == 4){
					float r = stringUtils::parseFloat(numericValues[0]);
					float x = stringUtils::parseFloat(numericValues[1]);
					float y = stringUtils::parseFloat(numericValues[2]);
					float z = stringUtils::parseFloat(numericValues[3]);

					Imath::Quatf vec(r, x, y, z);
					_quatValues.push_back(vec);
				}
			}

			_size = _quatValues.size();
		}
		else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
			_matrix44Values.clear();
			
			std::vector<std::string> values;
			stringUtils::split(valuesStr, values, "),(");
			
			for(int i = 0; i < values.size(); ++i){
				std::string numericValuesStr = stringUtils::strip(values[i], "()");
				std::vector<std::string> numericValues;
				stringUtils::split(numericValuesStr, numericValues, ",");
				
				if(numericValues.size() == 16){
					Imath::M44f matrix(
						stringUtils::parseFloat(numericValues[0]), stringUtils::parseFloat(numericValues[1]), stringUtils::parseFloat(numericValues[2]), stringUtils::parseFloat(numericValues[3]), 
						stringUtils::parseFloat(numericValues[4]), stringUtils::parseFloat(numericValues[5]), stringUtils::parseFloat(numericValues[6]), stringUtils::parseFloat(numericValues[7]), 
						stringUtils::parseFloat(numericValues[8]), stringUtils::parseFloat(numericValues[9]), stringUtils::parseFloat(numericValues[10]), stringUtils::parseFloat(numericValues[11]), 
						stringUtils::parseFloat(numericValues[12]), stringUtils::parseFloat(numericValues[13]), stringUtils::parseFloat(numericValues[14]), stringUtils::parseFloat(numericValues[15]));
					
					_matrix44Values.push_back(matrix);
				}
			}
			
			_size = _matrix44Values.size();
		}
	}
}
