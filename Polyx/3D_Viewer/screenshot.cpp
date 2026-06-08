#include "screenshot.h"

#include <vector>
#include <iostream>

#include <GL/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void save_screenshot(
    const char* filename,
    int width,
    int height)
{
    std::vector<unsigned char> pixels(
        width * height * 3);

    glPixelStorei(
        GL_PACK_ALIGNMENT,
        1);

    glReadBuffer(
        GL_FRONT);

    glReadPixels(
        0,
        0,
        width,
        height,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pixels.data());

    std::vector<unsigned char> flipped(
        width * height * 3);

    for (int y = 0; y < height; y++)
    {
        memcpy(
            &flipped[y * width * 3],
            &pixels[(height - 1 - y) * width * 3],
            width * 3);
    }

    if (stbi_write_jpg(
            filename,
            width,
            height,
            3,
            flipped.data(),
            95))
    {
        std::cout
            << "Screenshot saved: "
            << filename
            << std::endl;
    }
    else
    {
        std::cout
            << "Failed to save screenshot: "
            << filename
            << std::endl;
    }
}