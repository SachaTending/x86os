#pragma once

typedef void (*putpixel_t)(int x, int y, int color);

typedef struct
{
    int width;
    int height;
    int pitch;
} fbinfo_t;

namespace Graphics
{
    void Init(putpixel_t func, fbinfo_t *fbinfo);
    void Square_Filled(int x, int y, int x2, int y2, int color);
} // namespace 
