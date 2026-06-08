// screenshot.cpp
#include "screenshot.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <GL/gl.h>
#include <stdlib.h>

void save_screenshot(const char* filename, int width, int height) {
    unsigned char* pixels = (unsigned char*)malloc(width * height * 3);
    if (!pixels) return; // проверка на случай ошибки выделения
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    // Flip vertically
    for(int y=0; y<height/2; y++) {
        int y2 = height-1-y;
        for(int x=0; x<width*3; x++) {
            unsigned char tmp = pixels[y*width*3 + x];
            pixels[y*width*3 + x] = pixels[y2*width*3 + x];
            pixels[y2*width*3 + x] = tmp;
        }
    }
    stbi_write_jpg(filename, width, height, 3, pixels, 90);
    free(pixels);
}