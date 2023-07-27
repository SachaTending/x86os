#include <stdint.h>
#include <graphics.hpp>

#include <libcpp.hpp>

putpixel_t putpixel_func;
fbinfo_t *fb_info;

void Graphics::Init(putpixel_t func_p, fbinfo_t *fbinfo) {
    putpixel_func = func_p;
    fb_info = fbinfo;
}

void Graphics::Square_Filled(int x, int y, int x2, int y2, int color) {
    for (int x3=x;x3<x2;x3++) {
        for (int y3=y;y3<y2;y3++) {
            putpixel_func(x3, y3, color);
        }
    }
}