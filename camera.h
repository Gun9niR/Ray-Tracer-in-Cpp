#pragma once

#include "rtweekend.h"

class camera {
public:
	
	//aspect宽高比
	//vfov垂直视野，单位为角度
	camera(point3 lookfrom, point3 lookat, vec3 vup,
		   double vfov, 
		   double aspect, double aperture, double focus_dist, 
		   double t0 = 0, double t1 = 0
	) {
		origin = lookfrom;
		lens_radius = aperture / 2;   //光圈孔径

		time0 = t0;
		time1 = t1;

		auto theta = degrees_to_radians(vfov);
		auto half_height = tan(theta / 2);
		auto half_width = aspect * half_height;
		w = unit_vector(lookfrom - lookat); //指向原点到平面的反向
		u = unit_vector(cross(vup, w));     //平行于平面，垂直于vup
		v = cross(w, u);                    //平行于平面，和vup, w在同一个平面内

		lower_left_corner = origin - half_width * focus_dist * u-half_height * focus_dist * v - focus_dist * w;

		horizontal = 2 * half_width * focus_dist * u;   //对焦平面的宽度向量
		vertical = 2 * half_height * focus_dist * v;    //对焦平面的高度向量
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
	double lens_radius;  //光圈半径
	double time0, time1;  //shutter open/close times
};