
#include "KdNodes.h"




using namespace coral;

FindPointsInRange::FindPointsInRange(const std::string &name, Node *parent): Node(name, parent){
	_point = new NumericAttribute("point", this);
	_range = new NumericAttribute("range", this);
	_points = new NumericAttribute("points", this);
	_pointsInRange = new NumericAttribute("neighboursPerPoint", this);

	addInputAttribute(_point);
	addInputAttribute(_range);
	addInputAttribute(_points);
	addOutputAttribute(_pointsInRange);

	setAttributeAffect(_point, _pointsInRange);
	setAttributeAffect(_range, _pointsInRange);
	setAttributeAffect(_points, _pointsInRange);

	setAttributeAllowedSpecialization(_point, "Vec3");
	setAttributeAllowedSpecialization(_range, "Float");
	setAttributeAllowedSpecialization(_points, "Vec3Array");
	setAttributeAllowedSpecialization(_pointsInRange, "IntArray");
}

void FindPointsInRange::update(Attribute *attribute){
	const Imath::V3f &point = _point->value()->vec3Values()[0];
	float range = _range->value()->floatValues()[0];
	if(range < 0.0){
		range = 0.0;
	}

	const std::vector<Imath::V3f> &points = _points->value()->vec3Values();

	int dimentions = 3;
	kdtree *tree = kd_create(dimentions);

	int pointsSize = points.size();
	std::vector<int> indices(pointsSize);

	for(int i = 0; i < pointsSize; ++i){
		indices[i] = i;
		const Imath::V3f &currentPoint = points[i];
		kd_insert3f(tree, currentPoint.x, currentPoint.y, currentPoint.z, (void*)&indices[i]);
	}
	
	kdres *res = kd_nearest_range3f(tree, point.x, point.y, point.z, range);

	std::vector<int> pointsInRange(kd_res_size(res));

	while(!kd_res_end(res)){
		int pointIndex = *(int*)kd_res_item_data(res);
		pointsInRange.push_back(pointIndex);

		kd_res_next(res);
	}

	kd_res_free(res);
	kd_free(tree);

	_pointsInRange->outValue()->setIntValues(pointsInRange);
}


