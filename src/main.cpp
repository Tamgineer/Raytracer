#include <iostream>
#include <fstream>
#include <sstream>

#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"
#include "bvh.h"
#include "texture.h"

#include "camera.h"

enum scene {
        bouncing_spheres,
        checkered_spheres,
        textured_sphere,
        untextures_planes,
        transparency_test,
        mix_mat_test,
        image_transparency_test,
        simple_light,
        cornell_box,
        debug_cornell_box,
        simple_shadows,
    };

void bouncingSpheres() {
   
    hittable_list world;
    
    auto checker = std::make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    world.add(std::make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    world = hittable_list(std::make_shared<bvh_node>(world));

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}

void checkeredSpheres() {
    hittable_list world;

    auto checker = std::make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(std::make_shared<sphere>(point3(0,-10, 0), 10, std::make_shared<lambertian>(checker)));
    world.add(std::make_shared<sphere>(point3(0, 10, 0), 10, std::make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void texturedSphere() {
    auto sphere_texture = std::make_shared<image_texture>("images/ball3.png");
    auto sphere_surface = std::make_shared<lambertian>(sphere_texture);
    auto globe = std::make_shared<sphere>(point3(0,0,0), 2, sphere_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(hittable_list(globe));
}

void planes() {
    hittable_list world;

    // Materials
    auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal   = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(std::make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(std::make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(std::make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(std::make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(std::make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void transparency() {
    hittable_list world;

    auto checkerTex = std::make_shared<checker_texture>(0.32, color(.1), color(0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(checkerTex)));
    world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<transparent>()));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void mixMaterial(){
    hittable_list world;

    auto matA = std::make_shared<lambertian>(color(0, 1, 1));
    auto matB = std::make_shared<lambertian>(color(1, 0, 1));

    auto mixMat = std::make_shared<mix>(matA, matB, 0.5);
    
    auto checkerTex = std::make_shared<checker_texture>(0.32, color(.1), color(0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(checkerTex)));
    world.add(std::make_shared<sphere>(point3(0,2,0), 2, mixMat));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void imageTransparencyMapping(){
    hittable_list world;

    auto imageTex   = std::make_shared<image_texture>("images/brush.png");

    auto checkerTex = std::make_shared<checker_texture>(0.32, color(.1), color(0.5));
    auto mixedTex   = std::make_shared<textureMix>(std::make_shared<transparent>(), std::make_shared<lambertian>(color(0.8, 0.7, 1)), imageTex);

    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(checkerTex)));
    world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(color(0, 1, 1))));
    world.add(std::make_shared<quad>(point3( 3,0, 1), vec3(0, 0, 2), vec3(0, 2, 0), mixedTex));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void simpleLight() {
    hittable_list world;

    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(color(1))));
    world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(color(1))));

    auto difflight = std::make_shared<diffuse_light>(color(4,4,4));
    world.add(std::make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.lookfrom = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornellBox() {
    hittable_list world;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    auto mirror = std::make_shared<metal>(color(1), 0);

    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(std::make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    world.add(std::make_shared<sphere>(point3(130, 50, 65), 50, white));

    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), mirror);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.width             = 300;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    //cam.render(world);
    //cam.render_buffer(world);

    cam.render_outline_buffer(world);
}

void debug_cornellBox() {
    hittable_list world;

    auto normals = std::make_shared<normalMat>();

    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), normals));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), normals));
    world.add(std::make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), normals));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), normals));
    world.add(std::make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), normals));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), normals));

    world.add(std::make_shared<sphere>(point3(130, 50, 65), 50, normals));
 
    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), normals);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), normals);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.width             = 300;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0.1,0.1,0.1);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render_buffer(world);
    //cam.render(world);
}

void simple_shadow_example() {
    hittable_list world;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    //auto cel   = std::make_shared<fakeShadows>(color(0, .5, .5), point3(343, 554, 332));

    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(std::make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    world.add(std::make_shared<sphere>(point3(278, 278, 400), 70, white)); 

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.width             = 300;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
    //cam.render_buffer(world);

    //cam.render_outline_buffer(world);
}

int main() { 

    switch (simple_shadows) {
        case bouncing_spheres        : bouncingSpheres();          break;
        case checkered_spheres       : checkeredSpheres();         break;
        case textured_sphere         : texturedSphere();           break;
        case untextures_planes       : planes();                   break;
        case transparency_test       : transparency();             break;
        case mix_mat_test            : mixMaterial();              break;
        case image_transparency_test : imageTransparencyMapping(); break;
        case simple_light            : simpleLight();              break;
        case cornell_box             : cornellBox();               break;
        case debug_cornell_box       : debug_cornellBox();         break;
        case simple_shadows          : simple_shadow_example();    break;
    }
    
    return 0;

}