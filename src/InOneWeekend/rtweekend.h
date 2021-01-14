#pragma once

#include<cmath>
#include<cstdio>
#include<limits>
#include<memory>
#include<cstdlib>

//Usings
using std::shared_ptr;
using std::make_shared;

//Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi= 3.1415926535897932385;

//Utility Functions
inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180;
}

inline double random_double() {
	return (double)rand() / RAND_MAX;
}

inline double random_double(double min, double max) {
	return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max) 
{ 
	if (x < min) return min; 
	if (x > max) return max;
	return x; 
}

inline int random_int(int min, int max) {
	return static_cast<int>(random_double(min, max + 1));
}

//Common Headers
#include "../common/ray.h"
#include "../common/vec3.h"