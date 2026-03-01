#ifndef ICYLIB_BRESENHAM_H
#define ICYLIB_BRESENHAM_H

#include "color.h"

void icylib_draw_bresenham_circle(unsigned char* image, int xm, int ym, int r, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color));

void icylib_draw_bresenham_circular_arc(unsigned char* image, int xm, int ym, int r, float phi1, float phi2, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color));

void icylib_draw_bresenham_thick_line(unsigned char* image, int x0, int y0, int x1, int y1, int thickness, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color));

#ifdef ICYLIB_IMPLEMENTATION

#include <math.h>

#include "math_utils.h"

// The following functions were originally written by Alois Zingl and slightly modified to fit this project.
/*
MIT License

Copyright (c) 2020 zingl

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

void icylib_draw_bresenham_circle(unsigned char* image, int xm, int ym, int r, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color))
{
   int x = -r, y = 0, err = 2 - 2 * r;                /* bottom left to top right */
   do {
      set_pixel(image, xm - x, ym + y, color);                            /*   I. Quadrant +x +y */
      set_pixel(image, xm - y, ym - x, color);                            /*  II. Quadrant -x +y */
      set_pixel(image, xm + x, ym - y, color);                            /* III. Quadrant -x -y */
      set_pixel(image, xm + y, ym + x, color);                            /*  IV. Quadrant +x -y */
      r = err;
      if (r <= y) err += ++y * 2 + 1;                             /* e_xy+e_y < 0 */
      if (r > x || err > y)                  /* e_xy+e_x > 0 or no 2nd y-step */
         err += ++x * 2 + 1;                                     /* -> x-step now */
   } while (x < 0);
}

void icylib_draw_bresenham_circular_arc(unsigned char* image, int xm, int ym, int r, float phi1, float phi2, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color))
{
   int x = -r, y = 0, err = 2 - 2 * r;                /* bottom left to top right */
   do {
      if (icylib_is_point_inside_circular_arc(-x, y, phi1, phi2))
         set_pixel(image, xm - x, ym + y, color);                          /*   I. Quadrant +x +y */
      if (icylib_is_point_inside_circular_arc(-y, -x, phi1, phi2))
         set_pixel(image, xm - y, ym - x, color);                          /*  II. Quadrant -x +y */
      if (icylib_is_point_inside_circular_arc(x, -y, phi1, phi2))
         set_pixel(image, xm + x, ym - y, color);                          /* III. Quadrant -x -y */
      if (icylib_is_point_inside_circular_arc(y, x, phi1, phi2))
         set_pixel(image, xm + y, ym + x, color);                          /*  IV. Quadrant +x -y */
      r = err;
      if (r <= y) err += ++y * 2 + 1;                             /* e_xy+e_y < 0 */
      if (r > x || err > y)                  /* e_xy+e_x > 0 or no 2nd y-step */
         err += ++x * 2 + 1;                                     /* -> x-step now */
   } while (x < 0);
}

void icylib_draw_bresenham_thick_line(unsigned char* image, int x0, int y0, int x1, int y1, int thickness, icylib_Color color, void (*set_pixel)(unsigned char* image, int, int, icylib_Color))
{
   int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
   int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
   int err = dx + dy, e2; /* error value e_xy */

   for (;;) {  /* loop */
      for (int i = 0; i < thickness; i++) {
         set_pixel(image, x0, y0 + i, color);
         set_pixel(image, x0, y0 - i, color);
         set_pixel(image, x0 + i, y0, color);
         set_pixel(image, x0 - i, y0, color);
      }
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
   }
}

#endif

#endif