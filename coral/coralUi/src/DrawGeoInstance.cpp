
#include "DrawGeoInstance.h"

using namespace coral;
using namespace coralUi;

DrawGeoInstance::DrawGeoInstance(const std::string &name, Node *parent):
DrawNode(name, parent){

	_geoInstance = new GeoInstanceArrayAttribute(name, parent);

	addInputAttribute(_geoInstance);
	setAttributeAffect(_geoInstance, (Attribute*)viewportOutputAttribute());
	
	if(glContextExists()){
		initGL();
	}
}

void DrawGeoInstance::initGL(){
}

void DrawGeoInstance::draw(){
}
