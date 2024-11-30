#pragma once

#include <cstdint>

#include "vec3.h"

using color = vec3;

void write_color(uint8_t* pixels, int& index, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Translate the [0,1] component values to the byte range [0,255].
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    // Write out the pixel color components.

    pixels[index++] = rbyte; 
    pixels[index++] = gbyte; 
    pixels[index++] = bbyte; 
}