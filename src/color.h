#pragma once

#include <cstdint>

#include "interval.h"
#include "vec3.h"

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
        return std::sqrt(linear_component);

    return 0;
}

void write_color(uint8_t* pixels, int& index, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    // Write out the pixel color components.

    pixels[index++] = rbyte; 
    pixels[index++] = gbyte; 
    pixels[index++] = bbyte; 
}

color hsv2rgb(int h, double s, double v){

    double c = s * v;

    double x = c * (1 - std::abs(h / 60) % 2 - 1);

    double m = v - c;

    color rgbP;

    if(h >= 0   && h < 60 ){ rgbP = color(c, x, 0); }
    if(h >= 60  && h < 120){ rgbP = color(x, c, 0); }
    if(h >= 120 && h < 180){ rgbP = color(0, c, x); }
    if(h >= 180 && h < 240){ rgbP = color(0, x, c); }
    if(h >= 240 && h < 300){ rgbP = color(x, 0, c); }
    if(h >= 300           ){ rgbP = color(c, 0, x); }

    return color(rgbP.x() + m, rgbP.y() + m, rgbP.z() + m);
}