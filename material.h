#pragma once
#include "ray.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"

class material
{
public:
	virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
		return color(0, 0, 0);
	}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, double& pdf) const {
		return false;
	}

	virtual double scattering_pdf(
		const ray& r_in, const hit_record& rec, const ray& scattered
	) const {
		return 0;
	}
};

class lambertian :public material 
{
public:
	lambertian(const color& a) :albedo(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) :albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& alb, ray& scattered, double& pdf) const override {
		// compute the scattering ray
		onb uvw;
		uvw.build_from_w(rec.normal);
		auto direction = uvw.local(random_cosine_direction());
		
		scattered = ray(rec.p, unit_vector(direction), r_in.time());

		alb = albedo->value(rec.u, rec.v, rec.p);
		pdf = dot(uvw.w(), scattered.direction()) / pi;  // dot is actually cosine
		return true;
	}

	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
		auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
		return cosine < 0 ? 0 : cosine / pi; // ideally, cosine < 0 is not supposed to happen
	}
public:
	shared_ptr<texture> albedo;
};

class metal : public material 
{
public:
	metal(const vec3& a,double f) :albedo(a),fuzz(f<1?f:1) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		scattered = ray(rec.p, reflected+fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0); //this is not simply true due to fuzziness
	}

public:
	vec3 albedo;
	double fuzz;
};

double schlick(double cosine, double ref_idx) 
{ 
	auto r0 = (1 - ref_idx) / (1 + ref_idx); 
	r0 = r0 * r0; 
	return r0 + (1 - r0) * pow((1 - cosine), 5); 
}

class dielectric :public material 
{
public:
	dielectric(double ri) :ref_idx(ri) {}  //etha

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		attenuation = vec3(1, 1, 1);
		double etai_over_etat;
		if (rec.front_face) {
			etai_over_etat = 1.0 / ref_idx;
		}
		else {
			etai_over_etat = ref_idx;
		}

		vec3 unit_direction = unit_vector(r_in.direction());
		double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
		if (etai_over_etat * sin_theta > 1) {
			//there's no solution for the theta prime, meaning reflection
			vec3 reflected = reflect(unit_direction, rec.normal);
			scattered = ray(rec.p, reflected);
			return true;
		}

		double reflect_prob = schlick(cos_theta, etai_over_etat);
		if (random_double() < reflect_prob) {
			vec3 reflected = reflect(unit_direction, rec.normal);
			scattered = ray(rec.p, reflected);
			return true;
		}
		vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
		scattered = ray(rec.p, refracted);
		return true;
	}

public:
	double ref_idx;
};

class diffuse_light:public material 
{
public:
	diffuse_light(shared_ptr<texture> a) :emit(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const { return false; }

	virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
		if (rec.front_face)
			return emit->value(u, v, p);
		else
			return color(0, 0, 0);
	}

public:
	shared_ptr<texture> emit;
};