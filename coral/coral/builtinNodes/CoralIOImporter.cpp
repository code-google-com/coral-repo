#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "CoralIOImporter.h"
#include "../src/stringUtils.h"
#include "../src/Numeric.h"

using namespace coral;

namespace {
void parseFile(const std::string &filename, std::string &version, std::string &type, int &elementsPerFrame, int &frames, std::vector<Imath::M44f> &matrixValues, std::vector<Imath::V3f> &vec3Values){
	std::ifstream t;
	t.open(filename.data());
	std::string buffer;
	std::string line;
	
	int lineNumber = 0;
	while(t){
		std::getline(t, line);
		
		if(lineNumber == 0){
			std::vector<std::string> result;
			stringUtils::split(line, result, ":");
			if(result.size() == 2){
				if(result[0] == "coralIO"){
					version = result[1];
				}
			}
		}
		else if(lineNumber == 1){
			std::vector<std::string> result;
			stringUtils::split(line, result, ":");
			if(result.size() == 2){
				if(result[0] == "type"){
					type = result[1];
				}
			}
		}
		else if(lineNumber == 2){
			std::vector<std::string> result;
			stringUtils::split(line, result, ":");
			if(result.size() == 2){
				if(result[0] == "elementsPerFrame"){
					elementsPerFrame = stringUtils::parseInt(result[1]);
				}
			}
		}
		else if(lineNumber == 3){
			std::vector<std::string> result;
			stringUtils::split(line, result, ":");
			if(result.size() == 2){
				if(result[0] == "frames"){
					frames = stringUtils::parseInt(result[1]);
				}
			}
		}
		else{
			if(type == "matrix"){
				std::vector<std::string> result;
				stringUtils::split(line, result, ",");
				
				Imath::M44f matrix;
				for(int i = 0; i < result.size(); ++i){
					float val = stringUtils::parseFloat(result[i]);
					int row = i / 4;
					int col= i % 4;
					matrix.x[row][col] = val;
				}
				matrixValues.push_back(matrix);
			}
		}
		
		lineNumber ++;
	}
	t.close();
}

void cacheMatrixValues(int frames, int elementsPerFrame, const std::vector<Imath::M44f> &matrixValues, std::map<int, std::vector<Imath::M44f> > &cachedMatrixValues){
	int matrixCounter = 0;
	for(int t = 0; t < frames; ++t){
		cachedMatrixValues[t].resize(elementsPerFrame);
		for(int i = 0; i < elementsPerFrame; ++i){
			cachedMatrixValues[t][i] = matrixValues[matrixCounter];
			
			matrixCounter += 1;
		}
	}
}

void interpolateMatrix(const Imath::M44f &matrixA, const Imath::M44f &matrixB, float blend, Imath::M44f &outMatrix){
	Imath::V3f matrixAX(matrixA[0][0], matrixA[0][1], matrixA[0][2]);
	Imath::V3f matrixAY(matrixA[1][0], matrixA[1][1], matrixA[1][2]);
	Imath::V3f matrixAZ(matrixA[2][0], matrixA[2][1], matrixA[2][2]);
	
	Imath::V3f matrixBX(matrixB[0][0], matrixB[0][1], matrixB[0][2]);
	Imath::V3f matrixBY(matrixB[1][0], matrixB[1][1], matrixB[1][2]);
	Imath::V3f matrixBZ(matrixB[2][0], matrixB[2][1], matrixB[2][2]);
	
	Imath::V3f interpX = matrixAX + ((matrixBX - matrixAX) * blend);
	Imath::V3f interpY = matrixAY + ((matrixBY - matrixAY) * blend);
	Imath::V3f interpZ = matrixAZ + ((matrixBZ - matrixAZ) * blend);
	
	Imath::V3f matrixApos(matrixA[3][0], matrixA[3][1], matrixA[3][2]);
    Imath::V3f matrixBpos(matrixB[3][0], matrixB[3][1], matrixB[3][2]);
	Imath::V3f interpPos = matrixApos + ((matrixBpos - matrixApos) * blend);
	
	interpX.normalize();
	interpY.normalize();
	interpZ.normalize();
	
	outMatrix.x[0][0] = interpX.x;
	outMatrix.x[0][1] = interpX.y;
	outMatrix.x[0][2] = interpX.z;
	outMatrix.x[0][3] = 0.0;
	
	outMatrix.x[1][0] = interpY.x;
	outMatrix.x[1][1] = interpY.y;
	outMatrix.x[1][2] = interpY.z;
	outMatrix.x[1][3] = 0.0;
	
	outMatrix.x[2][0] = interpZ.x;
	outMatrix.x[2][1] = interpZ.y;
	outMatrix.x[2][2] = interpZ.z;
	outMatrix.x[3][3] = 0.0;
	
	outMatrix.x[3][0] = interpPos.x;
	outMatrix.x[3][1] = interpPos.y;
	outMatrix.x[3][2] = interpPos.z;
	outMatrix.x[3][3] = 1.0;
}

void interpolateMatrixValues(const std::vector<Imath::M44f> &matrixValuesA, const std::vector<Imath::M44f> &matrixValuesB, float blend, std::vector<Imath::M44f> &outMatrixValues){
	int size = matrixValuesA.size();
	if(size != matrixValuesB.size())
		return;
	
	outMatrixValues.resize(size);
	for(int i = 0; i < size; ++i){
		interpolateMatrix(matrixValuesA[i], matrixValuesB[i], blend, outMatrixValues[i]);
	}
}

}

CoralIOImporter::CoralIOImporter(const std::string &name, Node *parent): Node(name, parent){	
	_file = new StringAttribute("file", this);
	_time = new NumericAttribute("time", this);
	_out = new NumericAttribute("out", this);
	
	addInputAttribute(_file);
	addInputAttribute(_time);
	addOutputAttribute(_out);
	
	setAttributeAllowedSpecialization(_time, "Float");
	std::vector<std::string> outSpec;
	//outSpec.push_back("Vec3Array");
	outSpec.push_back("Matrix44Array");
	setAttributeAllowedSpecializations(_out, outSpec);
	
	setAttributeAffect(_file, _time);
	setAttributeAffect(_time, _out);
}

void CoralIOImporter::update(Attribute *attribute){
	std::string filename = _file->value()->stringValue();
	Numeric *time = _time->value();
	Numeric *out = _out->outValue();
	
	if(filename != _cachedFilename){
		_cachedFilename = filename;
		_cachedMatrixValues.clear();
		_cachedVec3Values.clear();
		
		std::string version;
		std::string type;
		int elementsPerFrame = 0;
		int frames = 0;
		std::vector<Imath::M44f> matrixValues;
		std::vector<Imath::V3f> vec3Values;
		
		parseFile(filename, version, type, elementsPerFrame, frames, matrixValues, vec3Values);
		
		if(version == "1.0"){
			if(type == "matrix"){
				cacheMatrixValues(frames, elementsPerFrame, matrixValues, _cachedMatrixValues);
			}
			else if(type == "vec3"){
				// to be implemented
			}
		}
	}
	
	int cachedMatrixFrames = _cachedMatrixValues.size();
	float timeVal = time->floatValueAt(0);
	int timeIndex = int(timeVal);
	if(timeIndex < cachedMatrixFrames && timeIndex >= 0){
		float mod = fmod(timeVal, 1.0f);
		if(mod > 0.0){
			std::vector<Imath::M44f> &prevMatrixValues = _cachedMatrixValues[timeIndex];
			std::vector<Imath::M44f> &nextMatrixValues = _cachedMatrixValues[timeIndex + 1];
			
			std::vector<Imath::M44f> matrixValues;
			interpolateMatrixValues(prevMatrixValues, nextMatrixValues, mod, matrixValues);
			out->setMatrix44Values(matrixValues);
		}
		else{
			out->setMatrix44Values(_cachedMatrixValues[timeIndex]);
		}
	}
}


