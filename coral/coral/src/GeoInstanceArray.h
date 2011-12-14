#ifndef CORAL_GEOINSTANCEARRAY_H
#define CORAL_GEOINSTANCEARRAY_H

#include <vector>
#include <ImathMatrix.h>
#include "Value.h"
#include "Geo.h"

namespace coral{

class CORAL_EXPORT GeoInstanceArray: public Value{
public:
	GeoInstanceArray();

	void setSelector(const std::vector<int> &selector);
	void setLocations(const std::vector<Imath::M44f> &locations);
	void setSourceGeos(const std::vector<Geo*> &sourceGeos);

private:
	std::vector<Geo*> _sourceGeos;
	std::vector<Imath::M44f> _locations;
	std::vector<int> _selector;
};

}

#endif
