#ifndef ICYLIB_MATH_UTILS_H
#define ICYLIB_MATH_UTILS_H

#define ICYLIB_PI 3.142

int icylib_imin(int a, int b);

int icylib_imax(int a, int b);

int icylib_floor(double x);

int icylib_ceil(double x);

unsigned char icylib_is_point_inside_circular_arc(double x, double y, double phi1, double phi2);

#ifdef ICYLIB_IMPLEMENTATION

#include <math.h>

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
    return x > (int)x ? (int)x + 1 : (int)x;
}

int icylib_round(double x) {
    return (int)(x + 0.5);
}

double icylib_wrap_angle(double phi) {
    phi = fmod(phi, 2 * ICYLIB_PI);
    if (phi < 0) phi += 2 * ICYLIB_PI;
    return phi;
}

unsigned char icylib_is_point_inside_circular_arc(double x, double y, double phi1, double phi2) {
    double phi = atan2(y, x);
    if (phi < 0) phi += 2 * ICYLIB_PI;
    return phi >= phi1 && phi <= phi2;
}

#endif

#endif