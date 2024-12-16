#pragma once

#include "hittable.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

//#define STBI_MSC_SECURE_CRT //use only for visual studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image_write.h"


class camera {
  public:
    /* Public Camera Parameters Here */

    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    width  = 400;  // Rendered image width in pixel count
    
    #define CHANNEL_NUM 3

    void render(const hittable& world) {
        initialize();
        /*** NOTICE!! You have to use uint8_t array to pass in stb function  ***/
        // Because the size of color is normally 255, 8bit.
        // If you don't use this one, you will get a weird imge.
        uint8_t* pixels = new uint8_t[width * height * CHANNEL_NUM];

        //render
        int index = 0;
        for (int j = 0; j < height; j++) {
            std::clog << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
            for (int i = 0; i < width; ++i) {
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                auto ray_direction = pixel_center - center;
                ray r(center, ray_direction);

                color pixel_color = ray_color(r, world);
                write_color(pixels, index, pixel_color);
            }
        }
    
        std::clog << "\rDone.                                                \n";

        // if CHANNEL_NUM is 4, you can use alpha channel in png
        stbi_write_png("image.png", width, height, CHANNEL_NUM, pixels, width * CHANNEL_NUM);

        delete[] pixels;

    }

  private:
    /* Private Camera Variables Here */
    int    height;   // Rendered image height
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below

    void initialize() {
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        center = point3(0, 0, 0);

        // Determine viewport dimensions.
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(width)/height);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left =
            center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    color ray_color(const ray& r, const hittable& world) const {
        hit_record rec;
        if (world.hit(r, interval(0, infinity), rec)) {
            return 0.5 * (rec.normal + color(1,1,1));
        } 

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};