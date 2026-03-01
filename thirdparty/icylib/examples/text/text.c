#define ICYLIB_IMPLEMENTATION

#include "regular_image.h"
#include "color.h"

void main() {
    icylib_RegularImage* image = icylib_regular_create_from_size(500, 500, 4);

    icylib_Font font = icylib_load_font_from_file(icylib_get_default_font_path());

    icylib_regular_draw_text(image, "Left aligned", 250, 125, icylib_color_from_rgb(255, 0, 0), font, 30, ICYLIB_HORIZONTAL_ALIGNMENT_LEFT, ICYLIB_VERTICAL_ALIGNMENT_CENTER, ICYLIB_HORIZONTAL_TEXT_FITTING_FULL, ICYLIB_VERTICAL_TEXT_FITTING_FULL, 1);
    icylib_regular_draw_text(image, "Middle aligned", 250, 250, icylib_color_from_rgb(0, 255, 0), font, 40, ICYLIB_HORIZONTAL_ALIGNMENT_CENTER, ICYLIB_VERTICAL_ALIGNMENT_CENTER, ICYLIB_HORIZONTAL_TEXT_FITTING_FULL, ICYLIB_VERTICAL_TEXT_FITTING_FULL, 1);
    icylib_regular_draw_text(image, "Right aligned", 250, 375, icylib_color_from_rgb(0, 0, 255), font, 30, ICYLIB_HORIZONTAL_ALIGNMENT_RIGHT, ICYLIB_VERTICAL_ALIGNMENT_CENTER, ICYLIB_HORIZONTAL_TEXT_FITTING_FULL, ICYLIB_VERTICAL_TEXT_FITTING_FULL, 1);

    icylib_regular_save_to_file(image, "text.png");
}