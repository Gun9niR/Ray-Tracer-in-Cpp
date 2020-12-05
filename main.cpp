#include "rtweekend.h" //include utility functions and basic data types

#include "material.h"
#include "hittable_list.h" //include the tool to store all the objects to be hitted
#include "sphere.h" //a kind of hittable object
#include "camera.h"
#include "color.h"
#include "moving_sphere.h"
#include "rtw_stb_image.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "bvh.h"
#include "pdf.h"
#include <iostream>
#include <ctime>

vec3 ray_color(const ray& r, const color& background, const hittable& world, shared_ptr<hittable>& lights, int depth)
{
	//determine which color to use for the ray based on its coordinates
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return vec3(0, 0, 0);

	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
	double pdf_val;
	color albedo;
	if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val)) {
		return emitted;
	}
	
	auto p0 = make_shared<hittable_pdf>(lights, rec.p);
	auto p1 = make_shared<cosine_pdf>(rec.normal);
	mixture_pdf mixed_pdf(p0, p1);

	scattered = ray(rec.p, mixed_pdf.generate(), r.time());
	pdf_val = mixed_pdf.value(scattered.direction());

	return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;
}

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
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
	double pdf_val;
	color albedo;
	if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val)) {
		return emitted;
	}
	cosine_pdf p(rec.normal);
	scattered = ray(rec.p, p.generate(), r.time());
	pdf_val = p.value(scattered.direction());

	return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, depth - 1) / pdf_val;
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
		double pdf;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered, pdf))
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
	const int image_width = 400;
	const int image_height = 225;
	const int samples_per_pixel = 50;
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
	const int image_width = 400;
	const int image_height = 225;
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
	camera cam(lookfrom, lookat, vup, 40, aspect_ratio, aperture, dist_to_focus, 0, 1);

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

hittable_list cornell_box()
{
	hittable_list objects;

	auto red = make_shared<lambertian>(make_shared<solid_color>(.65, .05, .05));
	auto white = make_shared<lambertian>(make_shared<solid_color>(.73, .73, .73));
	auto green = make_shared<lambertian>(make_shared<solid_color>(.12, .45, .15));
	auto light = make_shared<diffuse_light>(make_shared<solid_color>(15, 15, 15));

	objects.add(make_shared<flip_face>(make_shared<yz_rect>(0, 555, 0, 555, 555, green)));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
	objects.add(make_shared<flip_face>(make_shared<xz_rect>(0, 555, 0, 555, 0, white)));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<flip_face>(make_shared<xy_rect>(0, 555, 0, 555, 555, white)));

	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white); 
	box1 = make_shared<rotate_y>(box1, 15); 
	box1 = make_shared<translate>(box1, vec3(265, 0, 295)); 
	objects.add(box1);

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white); 
	box2 = make_shared<rotate_y>(box2, -18); 
	box2 = make_shared<translate>(box2, vec3(130, 0, 65)); 
	objects.add(box2);

	return objects;
}

void cornell_box_scene()
{
	const int image_width = 100;
	const int image_height = 100;
	const int samples_per_pixel = 20;
	const int max_depth = 50;
	const color background(0, 0, 0);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";


	auto world = cornell_box();
	shared_ptr<hittable> lights = make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());

	const auto aspect_ratio = double(image_width) / image_height;
	point3 lookfrom(278, 278, -800);
	point3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0);  	//vup决	定了视角在camera平面上旋转多少度，0，1，0就是人直立，1，0，0就是往右歪头
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	auto vfov = 40.0;
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; i++) {
			color color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = (i + random_double()) / image_width;    //random_double() is for anti-alias
				auto v = (j + random_double()) / image_height;
				ray r = cam.get_ray(u, v);                       //在get_ray中调整发出点，时间在[time0, time1]之间随机选择
				color += ray_color(r, background, world, lights, max_depth);
			}
			write_color(std::cout, color, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";
}

hittable_list cornell_smoke() {
	hittable_list objects;

	auto red = make_shared<lambertian>(make_shared<solid_color>(.65, .05, .05));
	auto white = make_shared<lambertian>(make_shared<solid_color>(.73, .73, .73));
	auto green = make_shared<lambertian>(make_shared<solid_color>(.12, .45, .15));
	auto light = make_shared<diffuse_light>(make_shared<solid_color>(7, 7, 7));

	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));

	objects.add(make_shared<constant_medium>(box1, 0.01, make_shared<solid_color>(0, 0, 0)));
	objects.add(make_shared<constant_medium>(box2, 0.01, make_shared<solid_color>(1, 1, 1)));

	return objects;
}

void cornell_smoke_scene()
{
	const int image_width = 600;
	const int image_height = 600;
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	const color background(0, 0, 0);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	auto world = cornell_smoke();

	const auto aspect_ratio = double(image_width) / image_height;
	point3 lookfrom(278, 278, -800);
	point3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0);  	//vup决	定了视角在camera平面上旋转多少度，0，1，0就是人直立，1，0，0就是往右歪头
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	auto vfov = 40.0;
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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

hittable_list final()
{
	hittable_list boxes1;
	//material for the ground
	auto ground = make_shared<lambertian>(make_shared <solid_color>(0.48, 0.83, 0.53));

	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) {
		for (int j = 0; j < boxes_per_side; j++) {
			auto w = 100;
			auto x0 = -1000 + i * w;
			auto z0 = -1000 + j * w;
			auto y0 = 0;
			auto x1 = x0 + w;
			auto y1 = random_double(1, 101);
			auto z1 = z0 + w;

			boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}

	hittable_list objects;

	objects.add(make_shared<bvh_node>(boxes1, 0, 1));

	//light material
	auto light = make_shared<diffuse_light>(make_shared<solid_color>(7, 7, 7));
	objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<lambertian>(make_shared<solid_color>(0.7, 0.3, 0.1));
	objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

	objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
	objects.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 10)));

	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	objects.add(boundary);
	//if refract, the ray enters the const medium, which forms a blue glass ball
	objects.add(make_shared<constant_medium>(boundary, 0.2, make_shared<solid_color>(0.2, 0.4, 0.9)));
	
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	objects.add(make_shared<constant_medium>(boundary, .0001, make_shared<solid_color>(1, 1, 1)));

	auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));

	objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(0.1);
	objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

	hittable_list boxes2;
	auto white = make_shared<lambertian>(make_shared<solid_color>(.73, .73, .73));
	int ns = 1000;
	for (int i = 0; i < ns; i++)
		boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));

	objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0, 1), 15), vec3(-100, 270, 395)));
	return objects;
}

void final_scene()
{
	const int image_width = 400;
	const int image_height = 400;
	const int samples_per_pixel = 2000;
	const int max_depth = 50;
	const color background(0, 0, 0);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	auto world = final();

	const auto aspect_ratio = double(image_width) / image_height;
	point3 lookfrom(478, 278, -600);
	point3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0);  	//vup决	定了视角在camera平面上旋转多少度，0，1，0就是人直立，1，0，0就是往右歪头
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	auto vfov = 40.0;
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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
	clock_t time;
	time = clock();
	cornell_box_scene();
	time = clock() - time;
	std::cerr << "Total time: " << (double)(time) / CLOCKS_PER_SEC;
	system("pause");
	return 0;
}