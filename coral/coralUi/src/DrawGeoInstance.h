
#ifndef CORAL_DRAWGEOINSTANCE_H
#define CORAL_DRAWGEOINSTANCE_H

#include <cstdio>
#include <string.h>

#include <coral/src/Node.h>
#include <coral/src/GeoInstanceArrayAttribute.h>
#include "DrawNode.h"
#include "coralUiDefinitions.h"

#include <GL/glew.h>

namespace coralUi{

class CORALUI_EXPORT DrawGeoInstance : public DrawNode{
public:
	DrawGeoInstance(const std::string &name, coral::Node *parent);
	void draw();
	void initGL();

private:
	coral::GeoInstanceArrayAttribute *_geoInstance;
};

}

#endif
