#include <iostream>
#include <fstream>
#include <sstream>

#include "vec3.h"
#include "ray.h"
#include "color.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

//#define STBI_MSC_SECURE_CRT //use only for visual studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image_write.h"

bool hit_sphere(const point3& center, double radius, const ray& r) {
    vec3 oc = center - r.origin();
    auto a = dot(r.direction(), r.direction());
    auto b = -2.0 * dot(r.direction(), oc);
    auto c = dot(oc, oc) - radius*radius;
    auto discriminant = b*b - 4*a*c;
    return (discriminant >= 0);
}

color ray_color(const ray& r) {

    if (hit_sphere(point3(0,0,-1), 0.5, r)){
        return color(1, 0, 1);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}

int main() {
    //define image
    int width = 400;
    auto aspect_ratio = 16.0 / 9.0;
    int height = int(width / aspect_ratio);
    #define CHANNEL_NUM 3


    // Calculate the image height, and ensure that it's at least 1.
    height = (height < 1) ? 1 : height;

    // Camera

    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(width)/height);
    auto camera_center = point3(0, 0, 0);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    auto pixel_delta_u = viewport_u / width;
    auto pixel_delta_v = viewport_v / height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera_center
                             - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    /*** NOTICE!! You have to use uint8_t array to pass in stb function  ***/
    // Because the size of color is normally 255, 8bit.
    // If you don't use this one, you will get a weird imge.
    uint8_t* pixels = new uint8_t[width * height * CHANNEL_NUM];

    //render
    int index = 0;
    for (int j = height - 1; j >= 0; --j) {
        std::clog << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
        for (int i = 0; i < width; ++i) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);

            color pixel_color = ray_color(r);

            write_color(pixels, index, pixel_color);
        }
    }
    
    std::clog << "\rDone.                                                \n";

    //output

    // if CHANNEL_NUM is 4, you can use alpha channel in png
    stbi_write_png("image.png", width, height, CHANNEL_NUM, pixels, width * CHANNEL_NUM);

    delete[] pixels;

    return 0;
}