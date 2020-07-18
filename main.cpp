#include "rtweekend.h" //include utility functions and basic data types

#include "material.h"
#include "hittable_list.h" //include the tool to store all the objects to be hitted
#include "sphere.h" //a kind of hittable object
#include "camera.h"
#include "color.h"
#include "moving_sphere.h"
#include "rtw_stb_image.h"
#include "aarect.h"
#include <iostream>

vec3 ray_color(const ray& r, const color& background, const hittable& world, int depth) 
{
	//determine which color to use for the ray based on its coordinates
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return vec3(0, 0, 0);

	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;
	
	//颜色是自己发出的光的颜色和折射光颜色的叠加
	return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

vec3 ray_color(const ray& r, const hittable& world, int depth) {
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

hittable_list random() 
{
	hittable_list world;

	world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(make_shared<solid_color>(0.5, 0.5, 0.5))));

	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 9)).length() > 0.9) {
				if (choose_mat < 0.8) {
					//diffuse
					auto albedo = vec3::random() * vec3::random();
					world.add(make_shared<sphere>(center, 0.2, make_shared<lambertian>(make_shared<solid_color>(albedo))));
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
	world.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(make_shared<solid_color>(0.4, 0.2, 0.1))));
	world.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

	return world;
}

void random_scene()
{
	const int image_width = 1920;
	const int image_height = 1080;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = random();

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

hittable_list random_checker() 
{
	hittable_list world;

	auto checker = make_shared<checker_texture>(make_shared<solid_color>(0.2, 0.1, 0.3), make_shared<solid_color>(0.9, 0.9, 0.9));
	world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			auto choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {
					//diffuse
					auto albedo = vec3::random() * vec3::random();
					world.add(make_shared<moving_sphere>(center, center+vec3(0,random_double(0,.5),0) ,0.0, 1.0, 0.2, make_shared<lambertian>(make_shared<solid_color>(albedo))));
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
	world.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(make_shared<solid_color>(0.4,0.2,0.1))));
	world.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

	return world;
}

void random_checker_scene()
{
	const int image_width = 1920;
	const int image_height = 1080;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = random_checker();

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

hittable_list two_perlin_spheres() 
{
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(5);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	return objects;
}

void two_perlin_spheres_scene()
{
	const int image_width = 1920;
	const int image_height = 1080;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = two_perlin_spheres();

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

hittable_list earth()
{
	auto earth_texture = make_shared<image_texture>("earthmap.jpg");
	auto earth_surface = make_shared<lambertian>(earth_texture);
	auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

	return hittable_list(globe);
}

void earth_scene()
{
	const int image_width = 1920;
	const int image_height = 1080;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = earth();

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

hittable_list simple_light() {
	hittable_list objects;
	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	//color is clamped in write_color(), so set color to (4,4,4) just affects rays that scatter to a light source
	auto difflight = make_shared<diffuse_light>(make_shared<solid_color>(4, 4, 4));
	objects.add(make_shared<sphere>(point3(0, 7, 0), 2, difflight));
	objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
	return objects;
}

void simple_light_scene()
{
	const int image_width = 1980;
	const int image_height = 1080;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	const color background(0, 0, 0);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = simple_light();

	const auto aspect_ratio = double(image_width) / image_height;
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 2, 0);
	vec3 vup(0, 1, 0);  	//vup决	定了视角在camera平面上旋转多少度，0，1，0就是人直立，1，0，0就是往右歪头
	auto dist_to_focus = 10;
	auto aperture = 0.0;
	camera cam(lookfrom, lookat, vup, 35, aspect_ratio, aperture, dist_to_focus, 0, 1);

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; i++) {
			color color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = (i + random_double()) / image_width;    //random_double() is for anti-alias
				auto v = (j + random_double()) / image_height;
				ray r = cam.get_ray(u, v);                       //在get_ray中调整发出点，时间在[time0, time1]之间随机选择
				color += ray_color(r, background, world, max_depth);
			}
			write_color(std::cout, color, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";
}

int main()
{
	simple_light_scene();
	return 0;
}