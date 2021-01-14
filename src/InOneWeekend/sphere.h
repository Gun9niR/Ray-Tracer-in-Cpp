#pragma once

#include "hittable.h"
#include "../common/vec3.h"
class sphere : public hittable
{
public:
	sphere() {}
	sphere(vec3 cen, double r, shared_ptr<material> m) :center(cen), radius(r) , mat_ptr(m) {};
	
	virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) const;

public:
	vec3 center;
	double radius;
	shared_ptr<material> mat_ptr;
};

void get_sphere_uv(const vec3& p, double& u, double& v)
{
	//view note to see the geometric meaning of phi and theta
	auto phi = atan2(p.z(), p.x());
	auto theta = asin(p.y());
	//map phi and theta to u, v, ranging from 0 to 1
	u = 1 - (phi + pi) / (2 * pi);
	v = (theta + pi / 2) / pi;
}

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 oc = r.origin() - center;
	auto a = dot(r.direction(), r.direction());
	auto half_b = dot(oc, r.direction());
	auto c = dot(oc, oc) - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant > 0) {
		auto root = sqrt(discriminant);
		auto temp = (-half_b - root) / a;
		if (temp<t_max && temp>t_min) {  //t_min makes sure you don't see things behind you
			rec.t = temp;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);  //this makes sure normal points against the ray hitting the surface
			rec.mat_ptr = mat_ptr;
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			return true;
		}
		temp = (-half_b + root) / a;
		if (temp<t_max && temp>t_min) {
			rec.t = temp;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);  //this makes sure normal points against the ray hitting the surface
			rec.mat_ptr = mat_ptr;
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			return true;
		}
	}
	return false;
}

bool sphere::bounding_box(double t0, double t1, aabb& output_box) const {
	output_box = aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
	return true;
}