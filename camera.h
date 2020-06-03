#pragma once

#include "rtweekend.h"

class camera {
public:
	
	// aspect长宽比
	camera(point3 lookfrom, point3 lookat, vec3 vup,
		   double vfov, // vertical field of view, in degrees
		   double aspect, double aperture, double focus_dist, 
		   double t0 = 0, double t1 = 0
	) {
		origin = lookfrom;
		lens_radius = aperture / 2;

		time0 = t0;
		time1 = t1;

		auto theta = degrees_to_radians(vfov);
		auto half_height = tan(theta / 2);
		auto half_width = aspect * half_height;
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		lower_left_corner = origin - half_width * focus_dist * u-half_height * focus_dist * v - focus_dist * w;

		horizontal = 2 * half_width * focus_dist * u;
		vertical = 2 * half_height * focus_dist * v;
	}

	ray get_ray(double s, double t) {  //s,t分别是坐标从左下角占x轴，y轴的比例
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();  //镜片中心指向光线发射点的向量
		return ray(origin+offset, lower_left_corner + s * horizontal + t * vertical- origin-offset, random_double(time0,time1));
	}
	
private:
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;
	double time0, time1;  //shutter open/close times
};