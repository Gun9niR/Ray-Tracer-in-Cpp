#pragma once
#include"vec3.h"
#include"ray.h"
class aabb 
{
public:
	aabb() {}
	aabb(const point3& a, const point3& b):_min(a),_max(b) {}

	point3 min() const { return _min; }
	point3 max() const { return _max; }

	bool hit(const ray& r, double tmin, double tmax) const{
		for (int i = 0; i < 3; i++) { //three dimensions
			auto invD = 1.0f / r.direction()[i];
			auto t0 = (_min[i] - r.origin()[i]) * invD;
			auto t1 = (_min[i] - r.origin()[i]) * invD;
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

public:
	point3 _min;
	point3 _max;
};