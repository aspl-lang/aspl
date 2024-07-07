#ifndef ICYLIB_COLOR_H
#define ICYLIB_COLOR_H

#include "icylib.h"

typedef struct icylib_Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} icylib_Color;

icylib_Color icylib_color_from_rgb(unsigned char r, unsigned char g, unsigned char b);
icylib_Color icylib_color_from_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#ifdef ICYLIB_IMPLEMENTATION

icylib_Color icylib_color_from_rgb(unsigned char r, unsigned char g, unsigned char b) {
    icylib_Color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;
    return color;
}

icylib_Color icylib_color_from_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    icylib_Color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

#endif

#endif