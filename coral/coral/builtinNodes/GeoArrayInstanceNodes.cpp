
#include "GeoArrayInstanceNodes.h"
#include "../src/Numeric.h"
#include <ImathMatrix.h>

using namespace coral;

GeoInstanceGenerator::GeoInstanceGenerator(const std::string &name, Node *parent):
Node(name, parent){
	_geo = new GeoAttribute("geo", this);
	_locations = new NumericAttribute("locations", this);
	_selector = new NumericAttribute("selector", this);
	_geoInstance = new GeoInstanceArrayAttribute("geoInstance", this);

	addInputAttribute(_geo);
	addInputAttribute(_locations);
	addInputAttribute(_selector);
	addOutputAttribute(_geoInstance);

	setAttributeAffect(_geo, _geoInstance);
	setAttributeAffect(_locations, _geoInstance);
	setAttributeAffect(_selector, _geoInstance);

	setAttributeAllowedSpecialization(_locations, "Matrix44Array");
	setAttributeAllowedSpecialization(_selector, "IntArray");

	_inputGeos.push_back(_geo);
}

void GeoInstanceGenerator::update(Attribute *attribute){
	const std::vector<Imath::M44f> &locations = _locations->value()->matrix44Values();
	const std::vector<int> &selector = _selector->value()->intValues();
	
	GeoInstanceArray *geoInstance = _geoInstance->outValue();

	int locationsSize = locations.size();
	int selectorSize = selector.size();

	if(selectorSize == locationsSize){
		geoInstance->setSelector(selector);
	}
	else{
		if(selectorSize > locationsSize){
			selectorSize = locationsSize;
		}

		std::vector<int> selec(locationsSize);
		int lastSelector = 0;
		for(int i = 0; i < selectorSize; ++i){
			lastSelector = selector[i];
			selec[i] = lastSelector;
		}

		for(int i = selectorSize; i < locationsSize; ++i){
			selec[i] = lastSelector;
		}

		geoInstance->setSelector(selector);
	}

	geoInstance->setLocations(locations);

	int inputGeosSize = _inputGeos.size();
	std::vector<Geo*> sourceGeos(inputGeosSize);
	for(int i = 0; i < inputGeosSize; ++i){
		Geo *geo = _inputGeos[i]->value();
	}
	
	geoInstance->setSourceGeos(sourceGeos);
}

