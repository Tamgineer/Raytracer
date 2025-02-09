#include <iostream>
#include <fstream>
#include <sstream>

#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include "camera.h"

int main() {

    hittable_list world;

    auto ground_material = std::make_shared<unlit>(color(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    auto material1 = std::make_shared<normalMat>();
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
 
    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 1980;
    cam.samples_per_pixel = 50;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);

    return 0;
}