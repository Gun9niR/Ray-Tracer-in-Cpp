#pragma once

#include "../InOneWeekend/rtweekend.h"

class camera {
public:
	
	//aspect���߱�
	//vfov��ֱ��Ұ����λΪ�Ƕ�
	camera(point3 lookfrom, point3 lookat, vec3 vup,
		   double vfov, 
		   double aspect, double aperture, double focus_dist, 
		   double t0 = 0, double t1 = 0
	) {
		origin = lookfrom;
		lens_radius = aperture / 2;   //��Ȧ�׾�

		time0 = t0;
		time1 = t1;

		auto theta = degrees_to_radians(vfov);
		auto half_height = tan(theta / 2);
		auto half_width = aspect * half_height;
		w = unit_vector(lookfrom - lookat); //ָ��ԭ�㵽ƽ��ķ���
		u = unit_vector(cross(vup, w));     //ƽ����ƽ�棬��ֱ��vup
		v = cross(w, u);                    //ƽ����ƽ�棬��vup, w��ͬһ��ƽ����

		lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;

		horizontal = 2 * half_width * focus_dist * u;   //�Խ�ƽ��Ŀ�������
		vertical = 2 * half_height * focus_dist * v;    //�Խ�ƽ��ĸ߶�����
	}

	ray get_ray(double s, double t) {  //s,t�ֱ�����������½�ռx�ᣬy��ı���
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();  //��Ƭ����ָ����߷���������
		return ray(origin+offset, lower_left_corner + s * horizontal + t * vertical- origin-offset, random_double(time0,time1));
	}
	
private:
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;  //��Ȧ�뾶
	double time0, time1;  //shutter open/close times
};