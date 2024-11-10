#define ICYLIB_IMPLEMENTATION
#define ICYLIB_MALLOC ASPL_MALLOC
#define ICYLIB_MALLOC_ATOMIC ASPL_MALLOC // TODO: Actually use ASPL_MALLOC_ATOMIC when it doesn't cause random crashes
#define ICYLIB_REALLOC ASPL_REALLOC
#define ICYLIB_FREE ASPL_FREE
#define ICYLIB_ERROR ASPL_PANIC
#define ICYLIB_NO_SIMD
#define STBI_NO_SIMD
#define STBIR_NO_SIMD

#include "thirdparty/icylib/regular_image.h"
#include "thirdparty/icylib/primitive_chunked_image.h"
#include "thirdparty/icylib/lazy_chunked_image.h"

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$new_from_size(ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    return ASPL_HANDLE_LITERAL(icylib_regular_create_from_size(ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, 4));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    icylib_Color color = icylib_regular_get_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32);
    ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 4);
    array[0] = ASPL_BYTE_LITERAL(color.a);
    array[1] = ASPL_BYTE_LITERAL(color.r);
    array[2] = ASPL_BYTE_LITERAL(color.g);
    array[3] = ASPL_BYTE_LITERAL(color.b);
    return ASPL_LIST_LITERAL("list<integer>", 13, array, 4);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$set_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    if (ASPL_ACCESS(*blend).value.boolean) {
        icylib_regular_set_pixel_blend(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    else {
        icylib_regular_set_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$fill(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_fill(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* image, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_draw_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*image).value.handle, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_line(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* thickness, ASPL_OBJECT_TYPE* blend, ASPL_OBJECT_TYPE* antialias) {
    icylib_regular_draw_line_with_thickness(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, ASPL_ACCESS(*thickness).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean, ASPL_ACCESS(*antialias).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_draw_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$fill_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_fill_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$fill_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_draw_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$fill_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_fill_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$draw_text(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* text, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* font_path, ASPL_OBJECT_TYPE* font_size, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* horizontal_alignment, ASPL_OBJECT_TYPE* vertical_alignment, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_draw_text(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*text).value.string->str, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*font_path).value.string->str, ASPL_ACCESS(*font_size).value.integer32, ASPL_ACCESS(*horizontal_alignment).value.integer32, ASPL_ACCESS(*vertical_alignment).value.integer32, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$replace_color(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_a, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_a, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b, ASPL_OBJECT_TYPE* blend) {
    icylib_regular_replace_color(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8, ASPL_ACCESS(*old_a).value.integer8), icylib_color_from_rgba(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8, ASPL_ACCESS(*new_a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$replace_color_ignore_alpha(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b) {
    icylib_regular_replace_color_ignore_alpha(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgb(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8), icylib_color_from_rgb(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8));
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$blur(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* radius) {
    icylib_regular_blur(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*radius).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$resize_with_size(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_regular_resize(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$resize_with_scale(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* scale) {
    icylib_regular_resize_scale(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*scale).value.float32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$extend_to(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_regular_extend_to(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_sub_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    return ASPL_HANDLE_LITERAL(icylib_regular_get_sub_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$copy(ASPL_OBJECT_TYPE* handle) {
    return ASPL_HANDLE_LITERAL(icylib_regular_copy(ASPL_ACCESS(*handle).value.handle));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$save_to_file(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* path) {
    icylib_regular_save_to_file(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*path).value.string->str);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$load_from_file(ASPL_OBJECT_TYPE* path) {
    return ASPL_HANDLE_LITERAL(icylib_regular_load_from_file(ASPL_ACCESS(*path).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$load_from_file_data(ASPL_OBJECT_TYPE* bytes) {
    unsigned char* array = ASPL_MALLOC(ASPL_ACCESS(*bytes).value.list->length);
    for (int i = 0; i < ASPL_ACCESS(*bytes).value.list->length; i++) {
        array[i] = ASPL_ACCESS(ASPL_ACCESS(*bytes).value.list->value[i]).value.integer8;
    }
    return ASPL_HANDLE_LITERAL(icylib_regular_load_from_file_data(array, ASPL_ACCESS(*bytes).value.list->length));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$load_from_data(ASPL_OBJECT_TYPE* bytes, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* channels) {
    unsigned char* array = ASPL_MALLOC(ASPL_ACCESS(*bytes).value.list->length);
    for (int i = 0; i < ASPL_ACCESS(*bytes).value.list->length; i++) {
        array[i] = ASPL_ACCESS(ASPL_ACCESS(*bytes).value.list->value[i]).value.integer8;
    }
    return ASPL_HANDLE_LITERAL(icylib_regular_create_from_memory(array, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, ASPL_ACCESS(*channels).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_width(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_RegularImage*)ASPL_ACCESS(*handle).value.handle)->width);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_height(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_RegularImage*)ASPL_ACCESS(*handle).value.handle)->height);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_width_from_file(ASPL_OBJECT_TYPE* path) {
    return ASPL_INT_LITERAL(icylib_regular_get_width_from_file(ASPL_ACCESS(*path).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$get_height_from_file(ASPL_OBJECT_TYPE* path) {
    return ASPL_INT_LITERAL(icylib_regular_get_height_from_file(ASPL_ACCESS(*path).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$new_from_size(ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* channels) {
    return ASPL_HANDLE_LITERAL(icylib_primitive_chunked_create_from_size(ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, ASPL_ACCESS(*channels).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$get_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    icylib_Color color = icylib_primitive_chunked_get_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32);
    ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 4);
    array[0] = ASPL_BYTE_LITERAL(color.a);
    array[1] = ASPL_BYTE_LITERAL(color.r);
    array[2] = ASPL_BYTE_LITERAL(color.g);
    array[3] = ASPL_BYTE_LITERAL(color.b);
    return ASPL_LIST_LITERAL("list<integer>", 13, array, 4);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$set_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    if (ASPL_ACCESS(*blend).value.boolean) {
        icylib_primitive_chunked_set_pixel_blend(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    else {
        icylib_primitive_chunked_set_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$fill(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_fill(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* image, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_draw_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*image).value.handle, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_line(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* thickness, ASPL_OBJECT_TYPE* blend, ASPL_OBJECT_TYPE* antialias) {
    icylib_primitive_chunked_draw_line_with_thickness(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, ASPL_ACCESS(*thickness).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean, ASPL_ACCESS(*antialias).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_draw_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$fill_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_fill_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$fill_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_draw_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$fill_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_fill_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$draw_text(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* text, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* font_path, ASPL_OBJECT_TYPE* font_size, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* horizontal_alignment, ASPL_OBJECT_TYPE* vertical_alignment, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_draw_text(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*text).value.string->str, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*font_path).value.string->str, ASPL_ACCESS(*font_size).value.integer32, ASPL_ACCESS(*horizontal_alignment).value.integer32, ASPL_ACCESS(*vertical_alignment).value.integer32, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$replace_color(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_a, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_a, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b, ASPL_OBJECT_TYPE* blend) {
    icylib_primitive_chunked_replace_color(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8, ASPL_ACCESS(*old_a).value.integer8), icylib_color_from_rgba(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8, ASPL_ACCESS(*new_a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$replace_color_ignore_alpha(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b) {
    icylib_primitive_chunked_replace_color_ignore_alpha(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgb(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8), icylib_color_from_rgb(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8));
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$blur(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* radius) {
    icylib_primitive_chunked_blur(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*radius).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$resize_with_size(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_primitive_chunked_resize(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$resize_with_scale(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* scale) {
    icylib_primitive_chunked_resize_scale(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*scale).value.float32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$extend_to(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_primitive_chunked_extend_to(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$get_sub_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    return ASPL_HANDLE_LITERAL(icylib_primitive_chunked_get_sub_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$copy(ASPL_OBJECT_TYPE* handle) {
    return ASPL_HANDLE_LITERAL(icylib_primitive_chunked_copy(ASPL_ACCESS(*handle).value.handle));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$save_to_file(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* path) {
    icylib_primitive_chunked_save_to_file(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*path).value.string->str);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$convert_to_lazy(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* file, ASPL_OBJECT_TYPE* directory) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$load_from_file(ASPL_OBJECT_TYPE* file) {
    return ASPL_HANDLE_LITERAL(icylib_primitive_chunked_load_from_file(ASPL_ACCESS(*file).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$load_from_file_data(ASPL_OBJECT_TYPE* bytes) {
    unsigned char* array = ASPL_MALLOC(ASPL_ACCESS(*bytes).value.list->length);
    for (int i = 0; i < ASPL_ACCESS(*bytes).value.list->length; i++) {
        array[i] = ASPL_ACCESS(ASPL_ACCESS(*bytes).value.list->value[i]).value.integer8;
    }
    return ASPL_HANDLE_LITERAL(icylib_primitive_chunked_load_from_file_data(array, ASPL_ACCESS(*bytes).value.list->length));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$get_width(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_LazyChunkedImage*)ASPL_ACCESS(*handle).value.handle)->width);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$primitive_chunked$get_height(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_LazyChunkedImage*)ASPL_ACCESS(*handle).value.handle)->height);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$new_from_size(ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* channels, ASPL_OBJECT_TYPE* directory) {
    return ASPL_HANDLE_LITERAL(icylib_lazy_chunked_create_from_size(ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, ASPL_ACCESS(*channels).value.integer32, ASPL_ACCESS(*directory).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$get_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    icylib_Color color = icylib_lazy_chunked_get_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32);
    ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 4);
    array[0] = ASPL_BYTE_LITERAL(color.a);
    array[1] = ASPL_BYTE_LITERAL(color.r);
    array[2] = ASPL_BYTE_LITERAL(color.g);
    array[3] = ASPL_BYTE_LITERAL(color.b);
    return ASPL_LIST_LITERAL("list<integer>", 13, array, 4);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$set_pixel(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    if (ASPL_ACCESS(*blend).value.boolean) {
        icylib_lazy_chunked_set_pixel_blend(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    else {
        icylib_lazy_chunked_set_pixel(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8));
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$fill(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_fill(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* image, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_draw_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*image).value.handle, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_line(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* thickness, ASPL_OBJECT_TYPE* blend, ASPL_OBJECT_TYPE* antialias) {
    icylib_lazy_chunked_draw_line_with_thickness(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, ASPL_ACCESS(*thickness).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean, ASPL_ACCESS(*antialias).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_draw_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$fill_rectangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_fill_rectangle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x1).value.integer32, ASPL_ACCESS(*y1).value.integer32, ASPL_ACCESS(*x2).value.integer32, ASPL_ACCESS(*y2).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$fill_triangle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x1, ASPL_OBJECT_TYPE* y1, ASPL_OBJECT_TYPE* x2, ASPL_OBJECT_TYPE* y2, ASPL_OBJECT_TYPE* x3, ASPL_OBJECT_TYPE* y3, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_draw_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$fill_circle(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* radius, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_fill_circle(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*radius).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$draw_text(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* text, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* font_path, ASPL_OBJECT_TYPE* font_size, ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* r, ASPL_OBJECT_TYPE* g, ASPL_OBJECT_TYPE* b, ASPL_OBJECT_TYPE* horizontal_alignment, ASPL_OBJECT_TYPE* vertical_alignment, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_draw_text(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*text).value.string->str, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, icylib_color_from_rgba(ASPL_ACCESS(*r).value.integer8, ASPL_ACCESS(*g).value.integer8, ASPL_ACCESS(*b).value.integer8, ASPL_ACCESS(*a).value.integer8), ASPL_ACCESS(*font_path).value.string->str, ASPL_ACCESS(*font_size).value.integer32, ASPL_ACCESS(*horizontal_alignment).value.integer32, ASPL_ACCESS(*vertical_alignment).value.integer32, ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$replace_color(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_a, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_a, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b, ASPL_OBJECT_TYPE* blend) {
    icylib_lazy_chunked_replace_color(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgba(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8, ASPL_ACCESS(*old_a).value.integer8), icylib_color_from_rgba(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8, ASPL_ACCESS(*new_a).value.integer8), ASPL_ACCESS(*blend).value.boolean);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$replace_color_ignore_alpha(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* old_r, ASPL_OBJECT_TYPE* old_g, ASPL_OBJECT_TYPE* old_b, ASPL_OBJECT_TYPE* new_r, ASPL_OBJECT_TYPE* new_g, ASPL_OBJECT_TYPE* new_b) {
    icylib_lazy_chunked_replace_color_ignore_alpha(ASPL_ACCESS(*handle).value.handle, icylib_color_from_rgb(ASPL_ACCESS(*old_r).value.integer8, ASPL_ACCESS(*old_g).value.integer8, ASPL_ACCESS(*old_b).value.integer8), icylib_color_from_rgb(ASPL_ACCESS(*new_r).value.integer8, ASPL_ACCESS(*new_g).value.integer8, ASPL_ACCESS(*new_b).value.integer8));
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$blur(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* radius) {
    icylib_lazy_chunked_blur(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*radius).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$resize_with_size(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_lazy_chunked_resize(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$resize_with_scale(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* scale) {
    icylib_lazy_chunked_resize_scale(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*scale).value.float32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$extend_to(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_lazy_chunked_extend_to(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$get_sub_image(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    return ASPL_HANDLE_LITERAL(icylib_lazy_chunked_get_sub_image(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$copy(ASPL_OBJECT_TYPE* handle) {
    return ASPL_HANDLE_LITERAL(icylib_lazy_chunked_copy(ASPL_ACCESS(*handle).value.handle));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$save_to_file(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* path) {
    int length;
    icylib_lazy_chunked_save_to_file(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*path).value.string->str, &length);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$load_from_file(ASPL_OBJECT_TYPE* file, ASPL_OBJECT_TYPE* directory) {
    return ASPL_HANDLE_LITERAL(icylib_lazy_chunked_load_from_file(ASPL_ACCESS(*file).value.string->str, ASPL_ACCESS(*directory).value.string->str));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$get_width(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_LazyChunkedImage*)ASPL_ACCESS(*handle).value.handle)->width);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$get_height(ASPL_OBJECT_TYPE* handle) {
    return ASPL_INT_LITERAL(((icylib_LazyChunkedImage*)ASPL_ACCESS(*handle).value.handle)->height);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$require_area(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    icylib_lazy_chunked_require_area(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32, ASPL_ACCESS(*width).value.integer32, ASPL_ACCESS(*height).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$is_chunk_loaded(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    return ASPL_BOOL_LITERAL(icylib_lazy_chunked_is_chunk_loaded(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$load_chunk(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    icylib_lazy_chunked_load_chunk(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$lazy_chunked$unload_chunk(ASPL_OBJECT_TYPE* handle, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y) {
    icylib_lazy_chunked_unload_chunk(ASPL_ACCESS(*handle).value.handle, ASPL_ACCESS(*x).value.integer32, ASPL_ACCESS(*y).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$font$get_default_font_path() {
    return ASPL_STRING_LITERAL_NO_COPY(icylib_get_default_font_path());
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$font$get_font_variant_path(ASPL_OBJECT_TYPE* font_path, ASPL_OBJECT_TYPE* bold, ASPL_OBJECT_TYPE* italic, ASPL_OBJECT_TYPE* underline, ASPL_OBJECT_TYPE* strikeout) {
    char* variant = "regular";
    if (ASPL_ACCESS(*bold).value.boolean) {
        variant = "bold";
    }
    else if (ASPL_ACCESS(*italic).value.boolean) {
        variant = "italic";
    }
    else if (ASPL_ACCESS(*underline).value.boolean) {
        variant = "underline";
    }
    else if (ASPL_ACCESS(*strikeout).value.boolean) {
        variant = "strikeout";
    }
    // TODO: Allow combining variants
    return ASPL_STRING_LITERAL_NO_COPY(icylib_get_font_variant_path(ASPL_ACCESS(*font_path).value.string->str, variant));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$canvas$measure_text_size(ASPL_OBJECT_TYPE* text, ASPL_OBJECT_TYPE* font_path, ASPL_OBJECT_TYPE* font_size) {
    double width, height;
    icylib_measure_text_size(ASPL_ACCESS(*text).value.string->str, ASPL_ACCESS(*font_path).value.string->str, ASPL_ACCESS(*font_size).value.integer32, &width, &height);
    ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 2);
    array[0] = ASPL_FLOAT_LITERAL((float)width);
    array[1] = ASPL_FLOAT_LITERAL((float)height);
    return ASPL_LIST_LITERAL("list<float>", 11, array, 2);
}

#ifndef ASPL_USE_SSL
#define SOKOL_IMPL
#ifndef __ANDROID__
#define SOKOL_GLCORE
#define SOKOL_NO_ENTRY
#endif
#endif
#include "thirdparty/sokol/sokol_app.h"
#include "thirdparty/sokol/sokol_log.h"
#include "thirdparty/sokol/sokol_gfx.h"
#include "thirdparty/sokol/sokol_glue.h"
#include "thirdparty/sokol/util/sokol_gl.h"

void* sokol_malloc(size_t size, void* user_data) {
    return ASPL_MALLOC(size);
}

void sokol_free(void* ptr, void* user_data) {
    ASPL_FREE(ptr);
}

typedef struct StreamingImage {
    sg_image image;
    sg_sampler sampler;
} StreamingImage;

typedef struct ASPL_handle_graphics$Window {
    sapp_desc* desc;
    StreamingImage* canvas_image;
    char keys_down[512];
    char mouse_buttons_down[32];

    ASPL_OBJECT_TYPE load_callback;
    ASPL_OBJECT_TYPE paint_callback;
    ASPL_OBJECT_TYPE resize_callback;
    ASPL_OBJECT_TYPE key_press_callback;
    ASPL_OBJECT_TYPE key_down_callback;
    ASPL_OBJECT_TYPE key_up_callback;
    ASPL_OBJECT_TYPE mouse_click_callback;
    ASPL_OBJECT_TYPE mouse_down_callback;
    ASPL_OBJECT_TYPE mouse_up_callback;
    ASPL_OBJECT_TYPE mouse_move_callback;
    ASPL_OBJECT_TYPE mouse_wheel_callback;
    ASPL_OBJECT_TYPE touch_down_callback;
    ASPL_OBJECT_TYPE touch_move_callback;
    ASPL_OBJECT_TYPE touch_up_callback;
} ASPL_handle_graphics$Window;

#ifndef ASPL_INTERPRETER_MODE
void aspl_callback_invoke(ASPL_OBJECT_TYPE closure);
void aspl_callback_any__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* canvas);
void aspl_callback_integer_integer__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height);
void aspl_callback_integer__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* key);
void aspl_callback_float_float_integer__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* x, ASPL_OBJECT_TYPE* y, ASPL_OBJECT_TYPE* button);
void aspl_callback_float_float_float_float__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* fromX, ASPL_OBJECT_TYPE* fromY, ASPL_OBJECT_TYPE* deltaX, ASPL_OBJECT_TYPE* deltaY);
void aspl_callback_list_list_long_OR_float_OR_integer_OR_boolean____invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* touches);
#endif

void aspl_util_graphics$Window_load_callback(void* userdata) {
    struct GC_stack_base sb; // TODO: Is this only required for Android?
    GC_get_stack_base(&sb);
    GC_register_my_thread(&sb);

    sg_setup(&(sg_desc) {
        // .context = sapp_sgcontext(), TODO: Is this actually required?
        .logger.func = slog_func,
    });
    sgl_setup(&(sgl_desc_t) {
        .logger.func = slog_func,
    });

    ASPL_handle_graphics$Window* handle = (ASPL_handle_graphics$Window*)userdata;
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 0, .arguments = NULL };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->load_callback).value.callback, arguments);
#else
    aspl_callback_invoke(handle->load_callback);
#endif
}

StreamingImage new_streaming_image(int width, int height, int channels) {
    sg_image_desc desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .num_slices = 1,
        .num_mipmaps = 1,
        .usage = SG_USAGE_STREAM,
        .label = "Default Canvas"
    };
    sg_sampler_desc sampler_desc = {
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    return (StreamingImage) { .image = sg_make_image(&desc), .sampler = sg_make_sampler(&sampler_desc) };
}

void aspl_util_graphics$Window_paint_callback(void* userdata) {
    ASPL_handle_graphics$Window* handle = (ASPL_handle_graphics$Window*)userdata;
    ASPL_OBJECT_TYPE canvas = ASPL_IMPLEMENT_graphics$canvas$new_from_size(C_REFERENCE(ASPL_INT_LITERAL(sapp_width())), C_REFERENCE(ASPL_INT_LITERAL(sapp_height())));
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { canvas } };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->paint_callback).value.callback, arguments);
#else
    aspl_callback_any__invoke(handle->paint_callback, C_REFERENCE(canvas));
#endif
    if (handle->canvas_image == 0) {
        handle->canvas_image = ASPL_MALLOC(sizeof(StreamingImage));
        *handle->canvas_image = new_streaming_image(((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->width, ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->height, 4);
    }
    else if (sg_query_image_desc(handle->canvas_image->image).width != ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->width || sg_query_image_desc(handle->canvas_image->image).height != ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->height) {
        sg_destroy_image(handle->canvas_image->image);
        sg_destroy_sampler(handle->canvas_image->sampler);
        *handle->canvas_image = new_streaming_image(((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->width, ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->height, 4);
    }
    StreamingImage image = *handle->canvas_image;
    sg_image_data data = {
        .subimage[0][0] = {
            .ptr = ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->data,
            .size = ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->width * ((icylib_RegularImage*)ASPL_ACCESS(canvas).value.handle)->height * 4
        }
    };
    sg_update_image(image.image, &data);

    sgl_defaults();
    sgl_ortho(0.0f, sapp_width(), sapp_height(), 0.0f, -1.0f, 1.0f);

    float scale = sapp_dpi_scale();
    if (scale < 1.0f) scale = 1.0f;

    sgl_enable_texture();
    sgl_texture(image.image, image.sampler);
    sgl_begin_quads();
    sgl_c4b(255, 255, 255, 255);
    sgl_v3f_t2f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    sgl_v3f_t2f(sapp_width() * scale, 0.0f, 0.0f, 1.0f, 0.0f);
    sgl_v3f_t2f(sapp_width() * scale, sapp_height() * scale, 0.0f, 1.0f, 1.0f);
    sgl_v3f_t2f(0.0f, sapp_height() * scale, 0.0f, 0.0f, 1.0f);
    sgl_end();
    sgl_disable_texture();

    sg_begin_pass(&(sg_pass) { .action = (sg_pass_action){ .colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } } }, .swapchain = sglue_swapchain() });
    sgl_draw();
    sg_end_pass();
    sg_commit();
}

void aspl_util_graphics$Window_event_callback(const sapp_event* event, void* userdata) {
    ASPL_handle_graphics$Window* handle = (ASPL_handle_graphics$Window*)userdata;
    switch (event->type) {
    case SAPP_EVENTTYPE_RESIZED:
    case SAPP_EVENTTYPE_RESTORED:
    case SAPP_EVENTTYPE_RESUMED: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 2, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_INT_LITERAL(event->framebuffer_width), ASPL_INT_LITERAL(event->framebuffer_height) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->resize_callback).value.callback, arguments);
#else
        aspl_callback_integer_integer__invoke(handle->resize_callback, C_REFERENCE(ASPL_INT_LITERAL(event->framebuffer_width)), C_REFERENCE(ASPL_INT_LITERAL(event->framebuffer_height)));
#endif
        break;
    }
    case SAPP_EVENTTYPE_KEY_DOWN: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_INT_LITERAL(event->key_code) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->key_down_callback).value.callback, arguments);
#else
        aspl_callback_integer__invoke(handle->key_down_callback, C_REFERENCE(ASPL_INT_LITERAL(event->key_code)));
#endif
        handle->keys_down[event->key_code] = 1;
        break;
    }
    case SAPP_EVENTTYPE_KEY_UP: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_INT_LITERAL(event->key_code) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->key_up_callback).value.callback, arguments);
#else
        aspl_callback_integer__invoke(handle->key_up_callback, C_REFERENCE(ASPL_INT_LITERAL(event->key_code)));
#endif
        if (handle->keys_down[event->key_code] == 1) {
#ifdef ASPL_INTERPRETER_MODE
            ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_INT_LITERAL(event->key_code) } };
            aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->key_press_callback).value.callback, arguments);
#else
            aspl_callback_integer__invoke(handle->key_press_callback, C_REFERENCE(ASPL_INT_LITERAL(event->key_code)));
#endif
        }
        handle->keys_down[event->key_code] = 0;
        break;
    }
    case SAPP_EVENTTYPE_MOUSE_DOWN: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 3, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_FLOAT_LITERAL(event->mouse_x), ASPL_FLOAT_LITERAL(event->mouse_y), ASPL_INT_LITERAL(event->mouse_button) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->mouse_down_callback).value.callback, arguments);
#else
        aspl_callback_float_float_integer__invoke(handle->mouse_down_callback, C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_y)), C_REFERENCE(ASPL_INT_LITERAL(event->mouse_button)));
#endif
        handle->mouse_buttons_down[event->mouse_button] = 1;
        break;
    }
    case SAPP_EVENTTYPE_MOUSE_UP: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 3, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_FLOAT_LITERAL(event->mouse_x), ASPL_FLOAT_LITERAL(event->mouse_y), ASPL_INT_LITERAL(event->mouse_button) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->mouse_up_callback).value.callback, arguments);
#else
        aspl_callback_float_float_integer__invoke(handle->mouse_up_callback, C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_y)), C_REFERENCE(ASPL_INT_LITERAL(event->mouse_button)));
#endif
        if (handle->mouse_buttons_down[event->mouse_button] == 1) {
#ifdef ASPL_INTERPRETER_MODE
            ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 3, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_FLOAT_LITERAL(event->mouse_x), ASPL_FLOAT_LITERAL(event->mouse_y), ASPL_INT_LITERAL(event->mouse_button) } };
            aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->mouse_click_callback).value.callback, arguments);
#else
            aspl_callback_float_float_integer__invoke(handle->mouse_click_callback, C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_y)), C_REFERENCE(ASPL_INT_LITERAL(event->mouse_button)));
#endif
        }
        handle->mouse_buttons_down[event->mouse_button] = 0;
        break;
    }
    case SAPP_EVENTTYPE_MOUSE_MOVE: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 4, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_FLOAT_LITERAL(event->mouse_x), ASPL_FLOAT_LITERAL(event->mouse_y), ASPL_FLOAT_LITERAL(event->mouse_dx), ASPL_FLOAT_LITERAL(event->mouse_dy) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->mouse_move_callback).value.callback, arguments);
#else
        aspl_callback_float_float_float_float__invoke(handle->mouse_move_callback, C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_y)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_dx)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_dy)));
#endif
        break;
    }
    case SAPP_EVENTTYPE_MOUSE_SCROLL: {
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 4, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_FLOAT_LITERAL(event->mouse_x), ASPL_FLOAT_LITERAL(event->mouse_y), ASPL_FLOAT_LITERAL(event->scroll_x), ASPL_FLOAT_LITERAL(event->scroll_y) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->mouse_wheel_callback).value.callback, arguments);
#else
        aspl_callback_float_float_float_float__invoke(handle->mouse_wheel_callback, C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->mouse_y)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->scroll_x)), C_REFERENCE(ASPL_FLOAT_LITERAL(event->scroll_y)));
#endif
        break;
    }
    case SAPP_EVENTTYPE_TOUCHES_BEGAN: {
        ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * event->num_touches);
        for (int i = 0; i < event->num_touches; i++) {
            sapp_touchpoint touchpoint = event->touches[i];
            ASPL_OBJECT_TYPE* array2 = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 5);
            array2[0] = ASPL_LONG_LITERAL(touchpoint.identifier);
            array2[1] = ASPL_FLOAT_LITERAL(touchpoint.pos_x);
            array2[2] = ASPL_FLOAT_LITERAL(touchpoint.pos_y);
            array2[3] = ASPL_INT_LITERAL(touchpoint.android_tooltype);
            array2[4] = ASPL_BOOL_LITERAL(touchpoint.changed);
            array[i] = ASPL_LIST_LITERAL("list<long|float|int|bool>", 25, array2, 5);
        }
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->touch_down_callback).value.callback, arguments);
#else
        aspl_callback_list_list_long_OR_float_OR_integer_OR_boolean____invoke(handle->touch_down_callback, C_REFERENCE(ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches)));
#endif
        break;
    }
    case SAPP_EVENTTYPE_TOUCHES_MOVED: {
        ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * event->num_touches);
        for (int i = 0; i < event->num_touches; i++) {
            sapp_touchpoint touchpoint = event->touches[i];
            ASPL_OBJECT_TYPE* array2 = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 5);
            array2[0] = ASPL_LONG_LITERAL(touchpoint.identifier);
            array2[1] = ASPL_FLOAT_LITERAL(touchpoint.pos_x);
            array2[2] = ASPL_FLOAT_LITERAL(touchpoint.pos_y);
            array2[3] = ASPL_INT_LITERAL(touchpoint.android_tooltype);
            array2[4] = ASPL_BOOL_LITERAL(touchpoint.changed);
            array[i] = ASPL_LIST_LITERAL("list<long|float|int|bool>", 25, array2, 5);
        }
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->touch_move_callback).value.callback, arguments);
#else
        aspl_callback_list_list_long_OR_float_OR_integer_OR_boolean____invoke(handle->touch_move_callback, C_REFERENCE(ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches)));
#endif
        break;
    }
    case SAPP_EVENTTYPE_TOUCHES_ENDED: {
        ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * event->num_touches);
        for (int i = 0; i < event->num_touches; i++) {
            sapp_touchpoint touchpoint = event->touches[i];
            ASPL_OBJECT_TYPE* array2 = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 5);
            array2[0] = ASPL_LONG_LITERAL(touchpoint.identifier);
            array2[1] = ASPL_FLOAT_LITERAL(touchpoint.pos_x);
            array2[2] = ASPL_FLOAT_LITERAL(touchpoint.pos_y);
            array2[3] = ASPL_INT_LITERAL(touchpoint.android_tooltype);
            array2[4] = ASPL_BOOL_LITERAL(touchpoint.changed);
            array[i] = ASPL_LIST_LITERAL("list<long|float|int|bool>", 25, array2, 5);
        }
#ifdef ASPL_INTERPRETER_MODE
        ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = (ASPL_OBJECT_TYPE[]) { ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches) } };
        aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->touch_up_callback).value.callback, arguments);
#else
        aspl_callback_list_list_long_OR_float_OR_integer_OR_boolean____invoke(handle->touch_up_callback, C_REFERENCE(ASPL_LIST_LITERAL("list<list<long|float|int|bool>>", 31, array, event->num_touches)));
#endif
        break;
    }
    default:
    {
        // TODO: Implement all remaining events
    }
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$new() {
    ASPL_handle_graphics$Window* handle = ASPL_MALLOC(sizeof(ASPL_handle_graphics$Window));
    sapp_desc* desc = ASPL_MALLOC(sizeof(sapp_desc));
    *desc = (sapp_desc){
        .logger.func = slog_func,
        .width = 640,
        .height = 480,
        .high_dpi = 1,
        .window_title = "ASPL Graphics Window",
        .allocator = {
            .alloc_fn = sokol_malloc,
            .free_fn = sokol_free
        },
        .user_data = handle,
        .init_userdata_cb = aspl_util_graphics$Window_load_callback,
        .frame_userdata_cb = aspl_util_graphics$Window_paint_callback,
        .event_userdata_cb = aspl_util_graphics$Window_event_callback
    };
    *handle = (ASPL_handle_graphics$Window){
        .desc = desc
    };
    return ASPL_HANDLE_LITERAL(handle);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_title(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* title) {
    sapp_desc* desc = ((ASPL_handle_graphics$Window*)ASPL_ACCESS(*window).value.handle)->desc;
    desc->window_title = ASPL_ACCESS(*title).value.string->str;
    if (sapp_isvalid()) {
        sapp_set_window_title(ASPL_ACCESS(*title).value.string->str);
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_load(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->load_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$get_fps(ASPL_OBJECT_TYPE* window) {
    return ASPL_INT_LITERAL((int)(0.5 + 1.0 / sapp_frame_duration()));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_size(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* width, ASPL_OBJECT_TYPE* height) {
    sapp_desc* desc = ((ASPL_handle_graphics$Window*)ASPL_ACCESS(*window).value.handle)->desc;
    desc->width = ASPL_ACCESS(*width).value.integer32;
    desc->height = ASPL_ACCESS(*height).value.integer32;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_resize(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->resize_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_paint(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->paint_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_key_press(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->key_press_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_key_down(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->key_down_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_key_up(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->key_up_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_mouse_click(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->mouse_click_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_mouse_down(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->mouse_down_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_mouse_up(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->mouse_up_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_mouse_move(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->mouse_move_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_mouse_wheel(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->mouse_wheel_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_touch_down(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->touch_down_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_touch_move(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->touch_move_callback = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$set_on_touch_up(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* callback) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    handle->touch_up_callback = *callback;
    return ASPL_UNINITIALIZED;
}

sapp_desc aspl_global_sapp_desc;

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$show(ASPL_OBJECT_TYPE* window) {
    aspl_global_sapp_desc = *((ASPL_handle_graphics$Window*)ASPL_ACCESS(*window).value.handle)->desc;
    sapp_run(((ASPL_handle_graphics$Window*)ASPL_ACCESS(*window).value.handle)->desc);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$is_fullscreen() {
    return ASPL_BOOL_LITERAL(sapp_is_fullscreen());
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$toggle_fullscreen(ASPL_OBJECT_TYPE* window) {
    sapp_toggle_fullscreen();
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$is_mouse_button_down(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* button) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    return ASPL_BOOL_LITERAL(handle->mouse_buttons_down[ASPL_ACCESS(*button).value.integer32]);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_graphics$window$is_key_down(ASPL_OBJECT_TYPE* window, ASPL_OBJECT_TYPE* key) {
    ASPL_handle_graphics$Window* handle = ASPL_ACCESS(*window).value.handle;
    return ASPL_BOOL_LITERAL(handle->keys_down[ASPL_ACCESS(*key).value.integer32]);
}