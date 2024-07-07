#define ICYLIB_IMPLEMENTATION

#include "regular_image.h"
#include "color.h"

void main() {
    icylib_RegularImage* image = icylib_regular_create_from_size(800, 800, 3);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            double a = (double)x / image->width * 4 - 2;
            double b = (double)y / image->height * 4 - 2;
            double ca = -0.8;
            double cb = 0.156;
            int n = 0;
            while (n < 100) {
                double aa = a * a - b * b;
                double bb = 2 * a * b;
                a = aa + ca;
                b = bb + cb;
                if (a * a + b * b > 16) {
                    break;
                }
                n++;
            }
            icylib_Color color = icylib_color_from_rgb(0, 0, 0);
            if (n < 100) {
                color = icylib_color_from_rgb(255 * n / 100, 255 * n / 100, 255 * n / 100);
            }
            icylib_regular_set_pixel(image, x, y, color);
        }
    }

    icylib_regular_save_to_file(image, "julia_set.png");
}