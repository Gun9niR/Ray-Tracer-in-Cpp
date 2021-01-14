# A ray tracer in C++

This is an implementation of the ray tracer introduced in [Peter Shirley's ray tracing book series](https://raytracing.github.io/).

It's implemented in a brute-force way, so rendering even a 480P image can possibly take hours. However, it does contain a few interesting techniques in computer graphics.

The images generated are in `.ppm` format, so you might want a [ppm viewer](http://www.cs.rhodes.edu/welshc/COMP141_F16/ppmReader.html) to view them.

## Usage

To build the project, run `make` in root directory, where the executable will be located.

To render the image, run `./raytracer > <file-name-of-your-choosing>`. Note that the image must be in `.ppm` format.

## Supported Features

### Objects

Objects are geometries that can be added to a scene and get rendered. They include

- Spheres
- Moving shperes
- Planar rectangles
- Cuboids

### Material

The material of an object decides the way rays scatter on their surface (or inside the object). They include

- Diffuse material. Rays have equal possibility to scatter in every direcition.
- Matal. Rays will be reflected, but not strictly adhereing to Snell's Law due to fuzziness.
- Dielectric. Rays can either be reflected or refracted.
- Light. Light sources don't scatter rays. They change the color of rays hitting the light source object.
- Constant medium. Rays don't necessarily get scattered on the surface. They can enter the inside of the object, but may be scattered at any moment until they're out again.

### Texture

The texture of an object decides the change of color for rays hitting the surface. They include

- Solid color. Just one color, plain and simple.
- Checker. Self-explanatory.
- Perlin noise. It utilizes a few randomization techniques to implement a repeatable black-and-white pattern that resembles noises on a television.
- Image. It maps an image to the surface of the object.

### Visual effects

Visual effects make the image more realistic. They include

- Antialiasing
- Defocus blur (Depth of field)
- Motion blur

### Performace

The ray tracer adopts bounding box hierarchy to accelerate rendering, especially for scenes containing dense, small objects.

## Image preview

- Checker texture

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/checker.png?raw=true)

- Perlin texture

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/perlin.png?raw=true)

- Cornell box

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/cornell.png?raw=true)

- Cornell box with smoke

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/cornell_smoke.png?raw=true)

- Earth

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/earth.png?raw=true)

- Light source

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/light.png?raw=true)

- Final scene of the first book

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/onewk.png?raw=true)

- Final scene of the second book

![image](https://github.com/Gun9niR/Ray-Tracer-in-Cpp/blob/master/png_images/final.png?raw=true)
