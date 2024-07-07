#define ICYLIB_IMPLEMENTATION

#include "regular_image.h"
#include "color.h"

void main() {
    icylib_RegularImage* image = icylib_regular_create_from_size(500, 500, 4);

    icylib_regular_fill_rectangle(image, 100, 100, 400, 400, icylib_color_from_rgb(128, 64, 32), 0);
    icylib_regular_draw_rectangle(image, 150, 150, 350, 350, icylib_color_from_rgb(32, 64, 128), 0);
    icylib_regular_fill_circle(image, 250, 250, 100, icylib_color_from_rgb(128, 64, 128), 0);
    icylib_regular_draw_circle(image, 250, 250, 200, icylib_color_from_rgb(32, 64, 32), 0);

    icylib_regular_save_to_file(image, "shapes.png");
}