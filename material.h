#pragma once
#include "ray.h"
#include "hittable.h"
class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

class lambertian :public material {
public:
	lambertian(const vec3& a) :albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		//compute the reflecting ray.
		vec3 scatter_direction = rec.normal + random_unit_vector();
		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}
public:
	vec3 albedo; //Âþ·´ÉäÏµÊý
};

class metal : public material {
public:
	metal(const vec3& a) :albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

public:
	vec3 albedo;
};