/**
* Name: Uriel Naranjo
* Assignment 2: Ray tracer 
* CS 130
*/


#define SET_RED(P, C)   (P = (((P) & 0x00ffffff) | ((C) << 24)))
#define SET_GREEN(P, C)  (P = (((P) & 0xff00ffff) | ((C) << 16)))
#define SET_BLUE(P, C) (P = (((P) & 0xffff00ff) | ((C) << 8)))

#include "ray_tracer.h"
#include <algorithm> 

using namespace std;

const double Object::small_t=1e-6;
//--------------------------------------------------------------------------------
// utility functions
//--------------------------------------------------------------------------------
double sqr(const double x)
{
    return x*x;
}

Pixel Pixel_Color(const Vector_3D<double>& color)
{
    Pixel pixel=0;
    SET_RED(pixel,(unsigned char)(min(color.x,1.0)*255));
    SET_GREEN(pixel,(unsigned char)(min(color.y,1.0)*255));
    SET_BLUE(pixel,(unsigned char)(min(color.z,1.0)*255));
    return pixel;
}
//--------------------------------------------------------------------------------
// Shader
//--------------------------------------------------------------------------------
Vector_3D<double> Phong_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,
            const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    Vector_3D<double> color, ambient, diffuse, specular, lightPos, lightDir, H;
    double NdotL, NdotH, kd, ks;

    ambient = color_ambient;

    for(size_t i = 0; i < world.lights.size(); i++){

        Light *light = world.lights.at(i);
        lightPos = light->position;
        lightDir = lightPos - intersection_point;  
        lightDir.Normalize();

        ambient = ambient*light->Emitted_Light(ray);

        // diffuse color
        NdotL = Vector_3D<double>::Dot_Product(same_side_normal, lightDir);
        kd = max(NdotL, 0.0);
                
        diffuse += light->Emitted_Light(ray) * color_diffuse * kd;

        // specular color
        H = lightDir - ray.direction;
        H.Normalize();

        NdotH = Vector_3D<double>::Dot_Product(same_side_normal, H);
        NdotH= max(NdotH, 0.0);
        ks = pow(NdotH, specular_power);

        specular += light->Emitted_Light(ray) * color_specular * ks;

        if(world.enable_shadows){
            Ray LRay;
            LRay.direction = light->position - intersection_point;
            LRay.endpoint = intersection_point + ( LRay.direction * Object::small_t);

            bool Shadow = false;

            for(size_t j = 0; j < world.objects.size(); j++){
                if(world.objects.at(j)->Intersection(LRay)){
                    Shadow = true; 
                }
            }

            if(!Shadow){
                color += specular + diffuse;
            }
        }

        else{ // shadows not enabled add all color components
            color += specular + diffuse;
        }

    }
    
    color += ambient;

    return color;
}

Vector_3D<double> Reflective_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,
            const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    Vector_3D<double> color;

    color = Phong_Shader::Shade_Surface(ray,intersection_object,intersection_point, same_side_normal);

    // base case
    if(ray.recursion_depth >= world.recursion_depth_limit){ 
        return color;
    }

    Vector_3D<double> reflected_color;
    Ray reflected_ray;

    reflected_ray.endpoint = intersection_point;
    reflected_ray.direction = ray.direction - same_side_normal*Vector_3D<double>::Dot_Product(ray.direction,same_side_normal) * 2;
    reflected_ray.direction.Normalize();
    reflected_ray.recursion_depth = ray.recursion_depth + 1; 
    reflected_ray.t_max = FLT_MAX; // need it to be largest possible to find intersections

    return color + world.Cast_Ray(reflected_ray,ray)*reflectivity;
}

Vector_3D<double> Flat_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,
            const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    return color;
}

//--------------------------------------------------------------------------------
// Objects
//--------------------------------------------------------------------------------
// determine if the ray intersects with the sphere
// if there is an intersection, set t_max, current_object, and semi_infinite as appropriate and return true
bool Sphere::
Intersection(Ray& ray) const
{
    // used to calculate quadratic formula 


    Vector_3D<double> cs = ray.endpoint - center;

    
    double a = Vector_3D<double>::Dot_Product(ray.direction,ray.direction);
    double b = Vector_3D<double>::Dot_Product(ray.direction, cs); 
    double c = Vector_3D<double>::Dot_Product(cs,cs) - sqr(radius);

    // roots or intersection points 
    double t0, t1, close_t;

    double discriminant = sqr(b) - a*c;
    
    if(discriminant < 0) { // no intersections
        return false;
    }

    else if(discriminant == 0){ // one intersections
        close_t = -b/a;
    }
    
    else{ // two intersections

        t0 = (-b - sqrt(discriminant) ) / a; 
        t1 = (-b + sqrt(discriminant) ) / a;

        //if t is negative then we exclude it 
        if( (t0 > 0) && (t1 >0) ){
            if(t0 > t1){
                close_t = t1;
            }
            else{
                close_t = t0;
            }   
        }
        
        else if( (t0 > 0) && (t1 <0) ){
            close_t = t0;
        }

        else if((t0 < 0) && (t1 > 0)){
            close_t = t1;
        }

        else{ // both negative. dont care
            return false;
        }
    }
    
    if(close_t > small_t){
        ray.current_object = this;

        ray.t_max = close_t;
        
        ray.semi_infinite = false;

        return true;
    }

    return false;
}

Vector_3D<double> Sphere::
Normal(const Vector_3D<double>& location) const
{

    Vector_3D<double> normal;

    normal = (location - center) * (1 / radius);

    return normal;
}

// determine if the ray intersects with the sphere

// if there is an intersection, set t_max, current_object, and semi_infinite as appropriate and return true
bool Plane::
Intersection(Ray& ray) const
{
    Vector_3D<double> p_l0= x1 - ray.endpoint;

    // denominator of line plane intersection equation
    double ldotn = Vector_3D<double>::Dot_Product(ray.direction, normal);

    // numerator of line plane intersection equation
    double pldotn = Vector_3D<double>::Dot_Product(p_l0, normal);

    // closest intersectoin
    double close_t; 

    if( ldotn < small_t) { // must check; denom cant be 0

        close_t = pldotn / ldotn; // interesection point 

        if( close_t > small_t ) { 
            ray.t_max = close_t;
            ray.current_object = this;
            ray.semi_infinite = false;
            return true;
        }
    }
   
    return false;
}

Vector_3D<double> Plane::
Normal(const Vector_3D<double>& location) const
{
    return normal;
}

//--------------------------------------------------------------------------------
// Camera
//--------------------------------------------------------------------------------
// Find the world position of the input pixel
Vector_3D<double> Camera::
World_Position(const Vector_2D<int>& pixel_index)
{

    Vector_3D<double> result;

    Vector_2D<double> wp = film.pixel_grid.X(pixel_index);

    result = focal_point + horizontal_vector*wp.x + vertical_vector*wp.y;

    return result;
}
//--------------------------------------------------------------------------------
// Render_World
//--------------------------------------------------------------------------------
// Find the closest object of intersection and return a pointer to it
//   if the ray intersects with an object, then ray.t_max, ray.current_object, and ray.semi_infinite will be set appropriately
//   if there is no intersection do not modify the ray and return 0
const Object* Render_World::
Closest_Intersection(Ray& ray)
{
    Ray temp = ray; // temp ray to pass in Intersection function
    const Object *ret = NULL; // return value

    for(size_t i = 0; i < objects.size(); i++) { // traverse through all objects

        if(objects.at(i)->Intersection(temp)){
            if(temp.t_max < ray.t_max){ // if current object is closer than current closest
                ray.t_max = temp.t_max;
                ray.current_object = temp.current_object;
                ret = ray.current_object; // update return object
            }
        }

    }

    return ret;
}

// set up the initial view ray and call 
void Render_World::
Render_Pixel(const Vector_2D<int>& pixel_index)
{
    Ray ray; 
    Ray dummy_root;

    ray.endpoint = camera.position; // rays always start at camera 
    Vector_3D<double> color;

    ray.direction = camera.World_Position(pixel_index) - camera.position; 
    ray.direction.Normalize();
    ray.t_max = FLT_MAX;

    color = Cast_Ray(ray,dummy_root);

    camera.film.Set_Pixel(pixel_index,Pixel_Color(color));
}

// cast ray and return the color of the closest intersected surface point, 
// or the background color if there is no object intersection
Vector_3D<double> Render_World::
Cast_Ray(Ray& ray,const Ray& parent_ray)
{
    Vector_3D<double> color, intersection, norm, dummy1, dummy2;

    const Object *close = Closest_Intersection(ray);

    // intersection exists. get color of object 
    if( close != NULL ) {
        intersection = ray.endpoint + ray.direction*ray.t_max;
        norm = close->Normal(intersection);
        norm.Normalize();
        color = close->material_shader->Shade_Surface(ray, *close, intersection, norm);
        return color;
    }
    // intersection doesnt exist. color equal to background
    color = background_shader->Shade_Surface(ray, *close, dummy1, dummy2);
    return color;
}
