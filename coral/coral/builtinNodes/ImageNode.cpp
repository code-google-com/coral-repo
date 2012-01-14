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
#include "ImageNode.h"
#include "../src/Image.h"
#include "../src/NetworkManager.h"

using namespace coral;

ImageNode::ImageNode(const std::string &name, Node *parent): Node(name, parent){
	_fileName = new StringAttribute("filename", this);
	_image = new ImageAttribute("image", this);

	addInputAttribute(_fileName);
	addOutputAttribute(_image);

	setAttributeAffect(_fileName, _image);

	setAttributeAllowedSpecialization(_fileName, "String");

	_fileName->outValue()->setStringValue("checker.jpeg");
}
ImageNode::~ImageNode(){

}

void ImageNode::update(Attribute *attribute){

	// read
	std::string filename = _fileName->value()->stringValue();
	std::cout<<"first filepath "<<filename<<std::endl;
	//filename = NetworkManager::resolveFilename(filename);

	std::cout<<"whole filepath "<<filename<<std::endl;
	_image->outValue()->load(filename.c_str());

	/*float width = _width->value()->floatValueAt(0);
	float height = _height->value()->floatValueAt(0);
	int widthSubdivisions = _widthSubdivisions->value()->intValueAt(0);
	int heightSubdivisions = _heightSubdivisions->value()->intValueAt(0);

	if(widthSubdivisions < 1){
		_widthSubdivisions->outValue()->setIntValueAt(0, 1);
		widthSubdivisions = 1;
	}
	if(heightSubdivisions < 1){
		_heightSubdivisions->outValue()->setIntValueAt(0, 1);
		heightSubdivisions = 1;
	}

	int totalFaces = widthSubdivisions * heightSubdivisions;
	int totalPoints = totalFaces + widthSubdivisions + heightSubdivisions + 1;

	std::vector<Imath::V3f> points(totalPoints);
	std::vector<std::vector<int> > faces(totalFaces);
	std::vector<int> faceVertices(4);

	float widthStep = width / widthSubdivisions;
	float heightStep = - (height / heightSubdivisions);
	float startWidth = - (width / 2.0);
	float startHeight = height / 2.0;

	int i = 0;
	for(int row = 0; row <= heightSubdivisions; ++row){
		for(int col = 0; col <= widthSubdivisions; ++col){
			float currentWidth = startWidth + (widthStep * col);
			float currentHeight = startHeight + (heightStep * row);

			points[i] = Imath::V3f(currentWidth, 0.0, currentHeight);
			i++;
		}
	}

	for(int faceId = 0; faceId < totalFaces; ++faceId){
		int row = faceId / widthSubdivisions;

		faceVertices[0] = faceId + row;
		faceVertices[1] = faceId + row + 1;
		faceVertices[2] = faceId + row + widthSubdivisions + 2;
		faceVertices[3] = faceId + row + widthSubdivisions + 1;

		faces[faceId] = faceVertices;
	}

	_out->outValue()->build(points, faces);*/
}

