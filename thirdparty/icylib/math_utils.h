#ifndef ICYLIB_MATH_UTILS_H
#define ICYLIB_MATH_UTILS_H

int icylib_imin(int a, int b);

int icylib_imax(int a, int b);

int icylib_floor(double x);

int icylib_ceil(double x);

#ifdef ICYLIB_IMPLEMENTATION

#include "icylib.h"

int icylib_imin(int a, int b) {
    return a < b ? a : b;
}

int icylib_imax(int a, int b) {
    return a > b ? a : b;
}

int icylib_floor(double x) {
    return (int)x;
}

int icylib_ceil(double x) {
    return (int)x + 1;
}

#endif

#endif