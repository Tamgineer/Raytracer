#pragma once

#include "hittable.h"
#include "material.h"

#ifdef _MSC_VER
    #define STBI_MSC_SECURE_CRT //use only for visual studio
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION

//#include "../ThirdParty/stb/stb_image.h"

#include "../ThirdParty/stb/stb_image_write.h"


class camera {
  public:
    /* Public Camera Parameters Here */

    double aspect_ratio = 1.0;       // Ratio of image width over height
    int    width  = 400;             // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene
    color  background;               //scene background colour

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,0);   // Point camera is looking from
    point3 lookat   = point3(0,0,-1);  // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    const int CHANNEL_NUM = 3;

    uint8_t* normal_buffer;
    /*** NOTICE!! You have to use uint8_t array to pass in stb function  ***/
        // Because the size of color is normally 255, 8bit.
        // If you don't use this one, you will get a weird imge.
    uint8_t* pixels;

    ~camera(){
        delete[] pixels;
        delete[] normal_buffer;
    }

    void render(const hittable& world) {
        initialize();
        
        pixels = new uint8_t[width * height * CHANNEL_NUM];

        //render
        int index = 0;
 
        std::cout << "rendering image\n";
        for (int j = 0; j < height; j++) {
            std::clog << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
            for (int i = 0; i < width; i++) {
                color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }

                write_color(pixels, index, pixel_samples_scale * pixel_color);
            }
        }
    
        std::clog << "\rImage done...                                             \n";

        // if CHANNEL_NUM is 4, you can use alpha channel in png
        stbi_write_png("image.png", width, height, CHANNEL_NUM, pixels, width * CHANNEL_NUM);

        //note: make a way to delete pixels when deleting camera
        delete[] pixels;

    }

    void render_buffer(const hittable& world) {
        initialize();
        /*** NOTICE!! You have to use uint8_t array to pass in stb function  ***/
        // Because the size of color is normally 255, 8bit.
        // If you don't use this one, you will get a weird imge.
        normal_buffer = new uint8_t[width * height * CHANNEL_NUM];

        //render
        int index = 0;
        
        std::cout << "rendering normal_buffer\n";
        for (int j = 0; j < height; j++) {
            std::clog << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
            for (int i = 0; i < width; i++) {
                color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_normal(r, max_depth, world);
                    //since the normals are going to have negatives, we'll just get the
                    //absolute value instead
                }

                pixel_color = color(std::fabs(pixel_color.x()), std::fabs(pixel_color.y()), std::fabs(pixel_color.z()));

                write_color(normal_buffer, index, pixel_samples_scale * pixel_color);
            }
        }
    
        std::clog << "\rDone.                                                \n";

        // if CHANNEL_NUM is 4, you can use alpha channel in png
        stbi_write_png("normal_buffer.png", width, height, 3, normal_buffer, width * 3);

        //note: make a way to delete pixels when deleting camera
        delete[] normal_buffer;

    }

  private:
    /* Private Camera Variables Here */
    int    height;   // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius

    void initialize() {
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        //center = point3(0, 0, 0);

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(width)/height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;
        //if the world hits nothing, return background
        if (!world.hit(r, interval(0.001, infinity), rec, lookfrom))
            return background;

        ray scattered;
        color attenuation;
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

        if(rec.mat->rgb(r, rec, attenuation)){
            //depth = 0;
            return attenuation;
        }

        if (!rec.mat->scatter(r, rec, attenuation, scattered))
            return color_from_emission;

        color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

        return color_from_emission + color_from_scatter; 
    }

    color ray_normal(const ray& r, int depth, const hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;
        //if the world hits nothing, return background
        if (!world.hit(r, interval(0.001, infinity), rec, lookfrom))
            return rec.normal;

        color normal;
        ray scattered;
        //color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p); 

        if (!rec.mat->scatter_normal(r, rec, normal, scattered))
            return normal;

        color color_from_scatter = ray_normal(scattered, depth-1, world);

        return normal + color_from_scatter;
    }

    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }
};