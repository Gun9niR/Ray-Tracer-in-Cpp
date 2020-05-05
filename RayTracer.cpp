#include "rtweekend.h" //include utility functions and basic data types

#include "hittable_list.h" //include the tool to store all the objects to be hitted
#include "sphere.h" //a kind of hittable object
#include "camera.h"
#include <iostream>

vec3 ray_color(const ray& r,const hittable& world, int depth) {
	//determine which color to use for the ray based on its coordinates
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return vec3(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		vec3 target = rec.p + random_in_hemisphere(rec.normal);
		return 0.5 * ray_color(ray(rec.p, target - rec.p), world,depth-1);  //反射次数越多就越暗
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1);
	return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

int main()
{
	const int image_width = 200;
	const int image_height = 100;
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	hittable_list world;
	world.add(make_shared<sphere>(vec3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(vec3(0, -100.5, -1), 100));

	camera cam;

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; i++) {
			vec3 color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = (i + random_double()) / image_width;
				auto v = (j + random_double()) / image_height;
				ray r = cam.get_ray(u, v);
				color += ray_color(r, world, max_depth);
			}
			color.write_color(std::cout, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";
}