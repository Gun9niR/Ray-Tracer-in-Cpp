#include "rtweekend.h" //include utility functions and basic data types

#include "material.h"
#include "hittable_list.h" //include the tool to store all the objects to be hitted
#include "sphere.h" //a kind of hittable object
#include "camera.h"
#include "color.h"
#include "moving_sphere.h"
#include <iostream>

vec3 ray_color(const ray& r,const hittable& world, int depth) {
	//determine which color to use for the ray based on its coordinates
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return vec3(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		vec3 attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return vec3(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1);
	return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

hittable_list random_scene1() {
	hittable_list world;

	world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(vec3(0.5, 0.5, 0.5))));

	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 9)).length() > 0.9) {
				if (choose_mat < 0.8) {
					//diffuse
					auto albedo = vec3::random() * vec3::random();
					world.add(make_shared<sphere>(center, 0.2, make_shared<lambertian>(albedo)));
				}
				else if (choose_mat < 0.95) {
					//metal
					auto albedo = vec3::random(0.5, 1);
					auto fuzz = random_double(0, .5);
					world.add(make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
				}
				else {
					//glass
					world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
				}
			}
		}
	}
	world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));
	world.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(vec3(0.4, 0.2, 0.1))));
	world.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

	return world;
}

hittable_list random_scene2() {
	hittable_list world;

	world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(vec3(0.5, 0.5, 0.5))));

	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			auto choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {
					//diffuse
					auto albedo = vec3::random() * vec3::random();
					world.add(make_shared<moving_sphere>(center, center+vec3(0,random_double(0,.5),0) ,0.0, 1.0, 0.2, make_shared<lambertian>(albedo)));
				}
				else if (choose_mat < 0.95) {
					//metal
					auto albedo = vec3::random(0.5, 1);
					auto fuzz = random_double(0, .5);
					world.add(make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
				}
				else {
					//glass
					world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
				}
			}
		}
	}
	world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));
	world.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(vec3(0.4, 0.2, 0.1))));
	world.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

	return world;
}

int main()
{
	const int image_width = 200;
	const int image_height = 100;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	
	auto world = random_scene2();

	const auto aspect_ratio = double(image_width) / image_height;
	point3 lookfrom(13, 2, 3); 
	point3 lookat(0, 0, 0); 
	vec3 vup(0, 1, 0);  	//vup决定了视角在camera平面上旋转多少度，0，1，0就是人直立，1，0，0就是往右歪头
	auto dist_to_focus = 10; 
	auto aperture = 0.0; 
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0, 1);

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; i++) {
			color color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = (i + random_double()) / image_width;
				auto v = (j + random_double()) / image_height;
				ray r = cam.get_ray(u, v);
				color += ray_color(r, world, max_depth);
			}
			write_color(std::cout, color, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";
}