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

#include "Image.h"

#include <OpenImageIO/imageio.h>

using namespace coral;
OIIO_NAMESPACE_USING

Image::Image():
	_xres(0),
	_yres(0),
	_channelCount(0),
	_pixels(0){

	/*_col4Values.resize(1);
	_col4Values[0] = Imath::Color4f(0.0, 0.0, 0.0, 1.0);*/
}

Image::~Image(){
	delete _pixels;
}

void Image::load(const char *filePath){
	std::cout<<"opening image "<<filePath<<std::endl;

	ImageInput *imgIn = ImageInput::create(filePath);

	if(!imgIn)
		return;

	// get image specs
	ImageSpec imgSpec;
	imgIn->open(filePath, imgSpec);

	_xres = imgSpec.width;
	_yres = imgSpec.height;
	_channelCount = imgSpec.nchannels;

	std::cout<<"spec.tile_width->"<<imgSpec.tile_width<<std::endl;
	std::cout<<"spec.tile_height->"<<imgSpec.tile_height<<std::endl;
	std::cout<<"spec.format->"<<imgSpec.format<<std::endl;

	_pixels = new float [_xres*_yres*_channelCount];
	imgIn->read_image(TypeDesc::FLOAT, _pixels);
	imgIn->close();

	delete imgIn;
	std::cout<<"image x->"<<_xres<<" y->"<<_yres<<" nchannels->"<<_channelCount<<std::endl;
}

const float* Image::pixels(){
	return _pixels;
}

int Image::width(){
	return _xres;
}

int Image::height(){
	return _yres;
}

int Image::channelCount(){
	return _channelCount;
}
