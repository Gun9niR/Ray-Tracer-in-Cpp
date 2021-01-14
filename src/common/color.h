#pragma once
#include "vec3.h"
#include "../InOneWeekend/rtweekend.h"
void write_color(std::ostream& out, color pixel_color,int samples_per_pixel) {
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();
	//Divide the color total by the number of samples. Now the color values before tansaltion are no longer among [0,1]
	//for a gamma value of 2.0
	auto scale = 1.0 / samples_per_pixel;
	//r,g,b < 1
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	// Write the translated [0,255] value of each color component.
	out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
		<< static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
		<< static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}