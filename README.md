# RayTracer
RayTracer is a basic ray tracer for planes and spheres.
I only implemented the ray_tracer.cpp file. All other files were provided. 
Flat shading, phong shading, shadows, and reflections are implemented.
Each Test case tests different parts. 
###Test
 1. Flat Shading
 2. Phong Shading 
 3. Shadows
 4. Reflections

## Installation 

>$git clone https://github.com/UrielNaranjo/RayTracer.git

>$cd RayTracer

>$ make

>$ ./ray_tracer <1-4>

##Dependencies
The OpenGL library is required to compile this code. 

To install OpenGL on linux run: 
>$ sudo apt-get install freeglut3-dev

By installing glut, you will be prompted to install it's dependencies which include OpenGL.

To install OpenGL on Mac you must install the developer tools that come with Xcode. Xcode can be found on the Appstore.
You must then change the LDFLAGS in the Makefile to: 
```
LDFLAGS = -g -O2 -Wall -framework -OpenGL -framework -GLUT   
```
##Bugs/Limitations/Issues
* Shadow artifacts on side of spheres
