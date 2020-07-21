#pragma once

#include"hittable.h"
#include<memory>
#include<vector>
#include<cstdlib>

using std::shared_ptr;
using std::make_shared;

class hittable_list :public hittable {
public:
	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;

	bool bounding_box(double t0, double t1, aabb& output_box) const;
	
public:
	std::vector<shared_ptr<hittable>> objects;
};

//determine if a ray hits any hittable thing in the hittable_list and stores the closest hit in rec
bool hittable_list::hit(const const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record temp_rec;
	bool hit_anything = false;
	auto closest_so_far = t_max;
	for (const auto& object : objects) {
		if (object->hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

aabb surrounding_box(const aabb& box0, const aabb& box1) {
	point3 small(fmin(box0.min().x(), box1.min().x()),
				 fmin(box0.min().y(), box1.min().y()),
				 fmin(box0.min().z(), box1.min().z()));
	point3 big(fmax(box0.max().x(), box1.max().x()),
				 fmax(box0.max().y(), box1.max().y()),
				 fmax(box0.max().z(), box1.max().z()));
	return aabb(small, big);
}

//每次调用
bool hittable_list::bounding_box(double t0,double t1,aabb& output_box) const{
	if (objects.empty()) return false;   

	aabb temp_box;
	bool first_box = true;

	for (const auto& object : objects) {  //对所有hittable建box
		if (!object->bounding_box(t0, t1, temp_box)) return false;
		output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
		first_box = false;
	}
	return true;
}