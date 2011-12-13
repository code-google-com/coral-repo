#include "GeoInstanceArray.h"

using namespace coral;

GeoInstanceArray::GeoInstanceArray(): Value(){

}

void GeoInstanceArray::setSelector(const std::vector<int> &selector){
	_selector = selector;
}

void GeoInstanceArray::setLocations(const std::vector<Imath::M44f> &locations){
	_locations = locations;
}

void GeoInstanceArray::setSourceGeos(const std::vector<Geo*> &sourceGeos){
	_sourceGeos = sourceGeos;
}
