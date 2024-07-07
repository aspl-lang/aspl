#ifndef ICYLIB_THICK_XIAOLIN_WU_H
#define ICYLIB_THICK_XIAOLIN_WU_H

#include "color.h"

void icylib_draw_thick_xiaolin_wu_aa_line(unsigned char* image, int x0, int y0, int x1, int y1, int w, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color));

#ifdef ICYLIB_IMPLEMENTATION

#include <math.h>

#include "icylib.h"

// The following function was originally written by John Bolton in CoffeScript and translated to C for this project.
/*
MIT License Copyright (c) 2022 John Bolton

Permission is hereby granted, free of
charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice
(including the next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// last argument is a pointer to a function that takes an x, y, and a color
void icylib_draw_thick_xiaolin_wu_aa_line(unsigned char* image, int x0, int y0, int x1, int y1, int w, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color)) {
    // Ensure positive integer values for width
    if (w < 1)
    {
        w = 1;
    }

    // steep means that m > 1
    int steep = abs(y1 - y0) > abs(x1 - x0);

    // If steep, then x and y must be swapped because the width is fixed in the y direction and that won't work if
    // dx < dy. Note that they are swapped again when plotting.
    if (steep)
    {
        int temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }

    // Swap endpoints to ensure that dx > 0
    if (x0 > x1)
    {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    double gradient = (dx > 0) ? (double)dy / (double)dx : 1.0;

    // Rotate w
    w = w * sqrt(1 + (gradient * gradient));

    // Handle first endpoint
    int xend = round(x0);
    double yend = y0 - (double)(w - 1) * 0.5 + gradient * (xend - x0);
    double xgap = 1 - ((double)x0 + 0.5 - xend);
    int xpxl1 = xend; // this will be used in the main loop
    int ypxl1 = (int)floor(yend);
    double fpart = yend - floor(yend);
    double rfpart = 1 - fpart;

    if (steep)
    {
        set_pixel(image, ypxl1, xpxl1, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (rfpart * xgap)));
        for (int i = 1; i < w; i++)
        {
            set_pixel(image, ypxl1 + i, xpxl1, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
        }
        set_pixel(image, ypxl1 + w, xpxl1, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (fpart * xgap)));
    }
    else
    {
        set_pixel(image, xpxl1, ypxl1, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (rfpart * xgap)));
        for (int i = 1; i < w; i++)
        {
            set_pixel(image, xpxl1, ypxl1 + i, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
        }
        set_pixel(image, xpxl1, ypxl1 + w, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (fpart * xgap)));
    }

    double intery = yend + gradient; // first y-intersection for the main loop

    // Handle second endpoint
    xend = round(x1);
    yend = y1 - (w - 1) * 0.5 + gradient * (xend - x1);
    xgap = 1 - (x1 + 0.5 - xend);
    int xpxl2 = xend; // this will be used in the main loop
    int ypxl2 = (int)floor(yend);
    fpart = yend - floor(yend);
    rfpart = 1 - fpart;

    if (steep) {
        set_pixel(image, ypxl2, xpxl2, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (rfpart * xgap)));
        for (int i = 1; i < w; i++) {
            set_pixel(image, ypxl2 + i, xpxl2, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
        }
        set_pixel(image, ypxl2 + w, xpxl2, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (fpart * xgap)));
    }
    else {
        set_pixel(image, xpxl2, ypxl2, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (rfpart * xgap)));
        for (int i = 1; i < w; i++) {
            set_pixel(image, xpxl2, ypxl2 + i, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
        }
        set_pixel(image, xpxl2, ypxl2 + w, icylib_color_from_rgba(color.r, color.g, color.b, color.a * (fpart * xgap)));
    }

    // Main loop
    if (steep)
    {
        for (int x = xpxl1 + 1; x < xpxl2; x++)
        {
            fpart = intery - floor(intery);
            rfpart = 1 - fpart;
            int y = (int)floor(intery);
            set_pixel(image, y, x, icylib_color_from_rgba(color.r, color.g, color.b, color.a * rfpart));
            for (int i = 1; i < w; i++)
            {
                set_pixel(image, y + i, x, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
            }
            set_pixel(image, y + w, x, icylib_color_from_rgba(color.r, color.g, color.b, color.a * fpart));
            intery = intery + gradient;
        }
    }
    else
    {
        for (int x = xpxl1 + 1; x < xpxl2; x++)
        {
            fpart = intery - floor(intery);
            rfpart = 1 - fpart;
            int y = (int)floor(intery);
            set_pixel(image, x, y, icylib_color_from_rgba(color.r, color.g, color.b, color.a * rfpart));
            for (int i = 1; i < w; i++)
            {
                set_pixel(image, x, y + i, icylib_color_from_rgba(color.r, color.g, color.b, color.a));
            }
            set_pixel(image, x, y + w, icylib_color_from_rgba(color.r, color.g, color.b, color.a * fpart));
            intery = intery + gradient;
        }
    }
}

#endif

#endif