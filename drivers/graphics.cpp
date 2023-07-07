#include <stdint.h>
#include <graphics.hpp>

static putpixel_t func;
static fbinfo_t *f_info;

void Graphics::Init(putpixel_t func_p, fbinfo_t *fbinfo) {
    func = func_p;
    f_info = fbinfo;
    for (int i=0;i<f_info->width;i++) {
        for (int a=0;a<f_info->height;a++) {
            func(i, a, 10);
        }
    }
    for (int i=0;i<f_info->width;i++) {
        for (int a=0;a<f_info->height;a++) {
            func(i, a, 0);
        }
    }
}

void Graphics::Square_Filled(int x, int y, int x2, int y2, int color) {
    for (int x3=x;x3<x2;x3++) {
        for (int y3=y;y3<y2;y3++) {
            func(x3, y3, color);
        }
    }
}