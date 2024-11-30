#include <iostream>
#include <fstream>
#include <sstream>

#include "vec3.h"
#include "color.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

//#define STBI_MSC_SECURE_CRT //use only for visual studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image_write.h"

int main() {
    //define image
    const int width = 256;
    const int height = 256;
    #define CHANNEL_NUM 3

    /*** NOTICE!! You have to use uint8_t array to pass in stb function  ***/
    // Because the size of color is normally 255, 8bit.
    // If you don't use this one, you will get a weird imge.
    uint8_t* pixels = new uint8_t[width * height * CHANNEL_NUM];

    //render
    int index = 0;
    for (int j = height - 1; j >= 0; --j) {
        std::clog << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
        for (int i = 0; i < width; ++i) {
            color pixel_color = color(double(i)/(width-1), double(j)/(height-1), 0);
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