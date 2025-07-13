#ifndef ICYLIB_REGULAR_IMAGE_H
#define ICYLIB_REGULAR_IMAGE_H

#include <math.h>

#include "thirdparty/stb_image_write.h"

#include "icylib.h"
#include "color.h"
#include "bresenham.h"
#include "thick_xiaolin_wu.h"
#include "text.h"
#include "math_utils.h"

typedef struct icylib_RegularImage {
    int width;
    int height;
    int channels;
    unsigned char* data;
} icylib_RegularImage;

typedef enum icylib_RegularImageFormat {
    ICYLIB_REGULAR_IMAGE_FORMAT_PNG,
    ICYLIB_REGULAR_IMAGE_FORMAT_BMP,
    ICYLIB_REGULAR_IMAGE_FORMAT_TGA,
    ICYLIB_REGULAR_IMAGE_FORMAT_JPG,
    ICYLIB_REGULAR_IMAGE_FORMAT_HDR
} icylib_RegularImageFormat;

int icylib_regular_get_width_from_file(const char* filename);

int icylib_regular_get_height_from_file(const char* filename);

icylib_RegularImage* icylib_regular_create_from_size(int width, int height, int channels);

icylib_RegularImage* icylib_regular_create_from_memory(unsigned char* data, int width, int height, int channels);

icylib_RegularImage* icylib_regular_load_from_file(const char* filename);

icylib_RegularImage* icylib_regular_load_from_file_data(unsigned char* data, int length);

int icylib_regular_save_to_file(icylib_RegularImage* image, const char* filename);

unsigned char* icylib_regular_save_to_file_data(icylib_RegularImage* image, icylib_RegularImageFormat format, int* length);

void icylib_regular_free(icylib_RegularImage* image);

icylib_Color icylib_regular_get_pixel(icylib_RegularImage* image, int x, int y);

void icylib_regular_set_pixel(icylib_RegularImage* image, int x, int y, icylib_Color color);

void icylib_regular_set_pixel_blend(icylib_RegularImage* image, int x, int y, icylib_Color color);

void icylib_regular_fill(icylib_RegularImage* image, icylib_Color color, unsigned char blend);

void icylib_regular_draw_rectangle(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_regular_fill_rectangle(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_regular_draw_image(icylib_RegularImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend);

void icylib_regular_draw_circle(icylib_RegularImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_regular_fill_circle(icylib_RegularImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_regular_draw_line_with_thickness(icylib_RegularImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_regular_draw_line(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_regular_draw_text(icylib_RegularImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend);

void icylib_regular_replace_color(icylib_RegularImage* image, icylib_Color old, icylib_Color new, unsigned char blend);

void icylib_regular_replace_color_ignore_alpha(icylib_RegularImage* image, icylib_Color old, icylib_Color new);

void icylib_regular_blur(icylib_RegularImage* image, int radius);

icylib_RegularImage* icylib_regular_get_sub_image(icylib_RegularImage* image, int x, int y, int width, int height);

icylib_RegularImage* icylib_regular_copy(icylib_RegularImage* image);

void icylib_regular_extend_to(icylib_RegularImage* image, int new_width, int new_height);

void icylib_regular_resize(icylib_RegularImage* image, int width, int height);

void icylib_regular_resize_scale(icylib_RegularImage* image, float scale);

#ifdef ICYLIB_IMPLEMENTATION

int icylib_regular_get_width_from_file(const char* filename) {
    int width, height, channels;
    stbi_info(filename, &width, &height, &channels);
    return width;
}

int icylib_regular_get_height_from_file(const char* filename) {
    int width, height, channels;
    stbi_info(filename, &width, &height, &channels);
    return height;
}

icylib_RegularImage* icylib_regular_create_from_size(int width, int height, int channels) {
    icylib_RegularImage* image = ICYLIB_MALLOC(sizeof(icylib_RegularImage));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->data = ICYLIB_MALLOC_ATOMIC(width * height * channels);
    return image;
}

icylib_RegularImage* icylib_regular_create_from_memory(unsigned char* data, int width, int height, int channels) {
    icylib_RegularImage* image = ICYLIB_MALLOC(sizeof(icylib_RegularImage));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->data = data;
    return image;
}

icylib_RegularImage* icylib_regular_load_from_file(const char* filename) {
    icylib_RegularImage* image = ICYLIB_MALLOC(sizeof(icylib_RegularImage));
    image->data = stbi_load(filename, &image->width, &image->height, &image->channels, 0);
    if (image->data == NULL) {
        ICYLIB_FREE(image);
        return NULL;
    }
    return image;
}

icylib_RegularImage* icylib_regular_load_from_file_data(unsigned char* data, int length) {
    icylib_RegularImage* image = ICYLIB_MALLOC(sizeof(icylib_RegularImage));
    image->data = stbi_load_from_memory(data, length, &image->width, &image->height, &image->channels, 0);
    return image;
}

int icylib_regular_save_to_file(icylib_RegularImage* image, const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (strcmp(extension, ".png") == 0) {
        return stbi_write_png(filename, image->width, image->height, image->channels, image->data, image->width * image->channels);
    }
    else if (strcmp(extension, ".bmp") == 0) {
        return stbi_write_bmp(filename, image->width, image->height, image->channels, image->data);
    }
    else if (strcmp(extension, ".tga") == 0) {
        return stbi_write_tga(filename, image->width, image->height, image->channels, image->data);
    }
    else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
        return stbi_write_jpg(filename, image->width, image->height, image->channels, image->data, 100);
    }
    else if (strcmp(extension, ".hdr") == 0) {
        return stbi_write_hdr(filename, image->width, image->height, image->channels, (float*)image->data);
    }
    else {
        ICYLIB_ERROR("Unsupported image format");
        return 0;
    }
}

unsigned char* icylib_regular_save_to_file_data(icylib_RegularImage* image, icylib_RegularImageFormat format, int* length) {
    unsigned char* data;
    switch (format) {
    case ICYLIB_REGULAR_IMAGE_FORMAT_PNG:
        return stbi_write_png_to_mem(image->data, image->width * image->channels, image->width, image->height, image->channels, length);
    case ICYLIB_REGULAR_IMAGE_FORMAT_BMP:
    case ICYLIB_REGULAR_IMAGE_FORMAT_TGA:
    case ICYLIB_REGULAR_IMAGE_FORMAT_JPG:
    case ICYLIB_REGULAR_IMAGE_FORMAT_HDR:
        // TODO: Implement
    default:
        ICYLIB_ERROR("Unsupported image format");
        return NULL;
    }
    return data;
}

void icylib_regular_free(icylib_RegularImage* image) {
    stbi_image_free(image->data);
    ICYLIB_FREE(image);
}

icylib_Color icylib_regular_get_pixel(icylib_RegularImage* image, int x, int y) {
    if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
        return (icylib_Color) { 0, 0, 0, 0 };
    }

    int index = (y * image->width + x) * image->channels;
    return (icylib_Color) { image->data[index], image->data[index + 1], image->data[index + 2], image->channels == 4 ? image->data[index + 3] : 255 };
}

void icylib_regular_set_pixel(icylib_RegularImage* image, int x, int y, icylib_Color color) {
    if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
        return;
    }

    int index = (y * image->width + x) * image->channels;
    image->data[index] = color.r;
    image->data[index + 1] = color.g;
    image->data[index + 2] = color.b;
    if (image->channels == 4) {
        image->data[index + 3] = color.a;
    }
}

void icylib_regular_set_pixel_blend(icylib_RegularImage* image, int x, int y, icylib_Color color) {
    if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
        return;
    }

    int index = (y * image->width + x) * image->channels;
    float aa = (float)color.a / 255;
    float ba = (float)image->data[index + 3] / 255;
    float ca = aa + (1 - aa) * ba;
    image->data[index] = (unsigned char)(1 / ca * (aa * ((float)color.r / 255) + (1 - aa) * ba * ((float)image->data[index] / 255)) * 255);
    image->data[index + 1] = (unsigned char)(1 / ca * (aa * ((float)color.g / 255) + (1 - aa) * ba * ((float)image->data[index + 1] / 255)) * 255);
    image->data[index + 2] = (unsigned char)(1 / ca * (aa * ((float)color.b / 255) + (1 - aa) * ba * ((float)image->data[index + 2] / 255)) * 255);
    if (image->channels == 4) {
        image->data[index + 3] = (unsigned char)(ca * 255);
    }
}

void icylib_regular_fill(icylib_RegularImage* image, icylib_Color color, unsigned char blend) {
    if (blend) {
        icylib_regular_fill_rectangle(image, 0, 0, image->width, image->height, color, blend);
    }
    else {
        for (int i = 0; i < image->width * image->height; i++) {
            memcpy(image->data + i * image->channels, (unsigned char*)&color, image->channels);
        }
    }
}

void icylib_regular_draw_rectangle(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    icylib_regular_draw_line(image, x1, y1, x2, y1, color, blend, 0);
    icylib_regular_draw_line(image, x1, y2, x2, y2, color, blend, 0);
    icylib_regular_draw_line(image, x1, y1, x1, y2, color, blend, 0);
    icylib_regular_draw_line(image, x2, y1, x2, y2, color, blend, 0);
}

void icylib_regular_fill_rectangle(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    if (blend) {
        for (int j = icylib_imax(0, y1); j < icylib_imin(image->height, y2); j++) {
            for (int i = icylib_imax(0, x1); i < icylib_imin(image->width, x2); i++) {
                icylib_regular_set_pixel_blend(image, i, j, color);
            }
        }
    }
    else {
        for (int j = icylib_imax(0, y1); j < icylib_imin(image->height, y2); j++) {
            for (int i = icylib_imax(0, x1); i < icylib_imin(image->width, x2); i++) {
                memcpy(image->data + (j * image->width + i) * image->channels, (unsigned char*)&color, image->channels);
            }
        }
    }
}

#ifndef ICYLIB_NO_SIMD
#include <immintrin.h>
#endif

void icylib_regular_draw_image(icylib_RegularImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend) {
    int minY = icylib_imax(-y, 0);
    int minX = icylib_imax(-x, 0);
    int maxY = icylib_imin(other->height, image->height - icylib_imax(y, 0));
    int maxX = icylib_imin(other->width, image->width - icylib_imax(x, 0));
    if (blend) {
        for (int j = minY; j < maxY; j++) {
            for (int i = minX; i < maxX; i++) {
                icylib_regular_set_pixel_blend(image, x + i, y + j, icylib_color_from_rgba(other->data[(j * other->width + i) * other->channels], other->data[(j * other->width + i) * other->channels + 1], other->data[(j * other->width + i) * other->channels + 2], other->channels == 4 ? other->data[(j * other->width + i) * other->channels + 3] : 255));
            }
        }
    }
    else {
#ifndef ICYLIB_NO_SIMD
        if (image->channels == 4) {
            for (int j = minY; j < maxY; j++) {
                for (int i = minX; i < maxX / 4; i++) {
                    unsigned char* source_address = other->data + (i * 4 + other->width * j) * other->channels;
                    unsigned char* destination_address = image->data + ((x + i * 4) + image->width * (y + j)) * image->channels;
                    __m128i source = _mm_loadu_si128((__m128i*)source_address);
                    _mm_storeu_si128((__m128i*)destination_address, source);
                }
            }
            // Handle the remaining pixels separately
            for (int j = minY; j < maxY; j++) {
                for (int i = (minX / 4) * 4; i < maxX; i++) {
                    unsigned char* source_address = other->data + (i + other->width * j) * other->channels;
                    unsigned char* destination_address = image->data + ((x + i) + image->width * (y + j)) * image->channels;
                    memcpy(destination_address, source_address, other->channels);
                }
            }
        }
        else {
#endif
            for (int j = minY; j < maxY; j++) {
                for (int i = minX; i < maxX; i++) {
                    memcpy(image->data + ((x + i) + image->width * (y + j)) * image->channels, other->data + (i + other->width * j) * other->channels, other->channels);
                }
            }
#ifndef ICYLIB_NO_SIMD
        }
#endif
    }
}

void icylib_regular_draw_circle(icylib_RegularImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    icylib_draw_bresenham_circle((unsigned char*)image, x, y, radius, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_regular_set_pixel_blend : icylib_regular_set_pixel));
}

void icylib_regular_fill_circle(icylib_RegularImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    int rsquared = radius * radius;
    for (int j = -radius; j < radius; j++) {
        for (int i = -radius; i < radius; i++) {
            if ((i * i + j * j) < rsquared) {
                if (blend) {
                    icylib_regular_set_pixel_blend(image, x + i, y + j, color);
                }
                else {
                    icylib_regular_set_pixel(image, x + i, y + j, color);
                }
            }
        }
    }
}

void icylib_regular_draw_line_with_thickness(icylib_RegularImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias) {
    if (antialias) {
        icylib_draw_thick_xiaolin_wu_aa_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_regular_set_pixel_blend : icylib_regular_set_pixel));
    }
    else {
        icylib_draw_bresenham_thick_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_regular_set_pixel_blend : icylib_regular_set_pixel));
    }
}

void icylib_regular_draw_line(icylib_RegularImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias) {
    icylib_regular_draw_line_with_thickness(image, x1, y1, x2, y2, 1, color, blend, antialias);
}

void icylib_regular_draw_text(icylib_RegularImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend) {
    icylib_draw_text((unsigned char*)image, text, x, y, color, fontPath, pixelSize, horizontalAlignment, verticalAlignment, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_regular_set_pixel_blend : icylib_regular_set_pixel));
}

void icylib_regular_replace_color(icylib_RegularImage* image, icylib_Color old, icylib_Color new, unsigned char blend) {
    for (int j = 0; j < image->height; j++) {
        for (int i = 0; i < image->width; i++) {
            icylib_Color pixel = icylib_regular_get_pixel(image, i, j);
            if (pixel.r == old.r && pixel.g == old.g && pixel.b == old.b && pixel.a == old.a) {
                if (blend) {
                    icylib_regular_set_pixel_blend(image, i, j, new);
                }
                else {
                    icylib_regular_set_pixel(image, i, j, new);
                }
            }
        }
    }
}

void icylib_regular_replace_color_ignore_alpha(icylib_RegularImage* image, icylib_Color old, icylib_Color new) {
    for (int j = 0; j < image->height; j++) {
        for (int i = 0; i < image->width; i++) {
            icylib_Color pixel = icylib_regular_get_pixel(image, i, j);
            if (pixel.r == old.r && pixel.g == old.g && pixel.b == old.b) {
                icylib_regular_set_pixel(image, i, j, icylib_color_from_rgba(new.r, new.g, new.b, pixel.a));
            }
        }
    }
}

void icylib_regular_blur(icylib_RegularImage* image, int radius) {
    int width = image->width;
    int height = image->height;
    int channels = image->channels;
    unsigned char* temp_data = ICYLIB_MALLOC_ATOMIC(width * height * channels);
    unsigned char* new_data = ICYLIB_MALLOC_ATOMIC(width * height * channels);

    // Horizontal pass
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;
            int count = 0;

            for (int x = -radius; x <= radius; x++) {
                int nx = i + x;
                if (nx >= 0 && nx < width) {
                    icylib_Color pixel = icylib_regular_get_pixel(image, nx, j);
                    r += pixel.r;
                    g += pixel.g;
                    b += pixel.b;
                    a += pixel.a;
                    count++;
                }
            }

            temp_data[(j * width + i) * channels] = r / count;
            temp_data[(j * width + i) * channels + 1] = g / count;
            temp_data[(j * width + i) * channels + 2] = b / count;
            temp_data[(j * width + i) * channels + 3] = a / count;
        }
    }

    // Vertical pass
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;
            int count = 0;

            for (int y = -radius; y <= radius; y++) {
                int ny = j + y;
                if (ny >= 0 && ny < height) {
                    r += temp_data[(ny * width + i) * channels];
                    g += temp_data[(ny * width + i) * channels + 1];
                    b += temp_data[(ny * width + i) * channels + 2];
                    a += temp_data[(ny * width + i) * channels + 3];
                    count++;
                }
            }

            new_data[(j * width + i) * channels] = r / count;
            new_data[(j * width + i) * channels + 1] = g / count;
            new_data[(j * width + i) * channels + 2] = b / count;
            new_data[(j * width + i) * channels + 3] = a / count;
        }
    }

    ICYLIB_FREE(temp_data);
    ICYLIB_FREE(image->data);
    image->data = new_data;
}

icylib_RegularImage* icylib_regular_get_sub_image(icylib_RegularImage* image, int x, int y, int width, int height) {
    icylib_RegularImage* sub_image = ICYLIB_MALLOC(sizeof(icylib_RegularImage));
    sub_image->width = width;
    sub_image->height = height;
    sub_image->channels = image->channels;
    sub_image->data = ICYLIB_MALLOC_ATOMIC(width * height * image->channels);
    for (int j = 0; j < height; j++) {
        memcpy(sub_image->data + j * width * image->channels, image->data + ((j + y) * image->width + x) * image->channels, width * image->channels);
    }
    return sub_image;
}

icylib_RegularImage* icylib_regular_copy(icylib_RegularImage* image) {
    icylib_RegularImage* copy = icylib_regular_create_from_size(image->width, image->height, image->channels);
    memcpy(copy->data, image->data, image->width * image->height * image->channels);
    return copy;
}

void icylib_regular_extend_to(icylib_RegularImage* image, int new_width, int new_height) {
    if (new_width <= image->width && new_height <= image->height) {
        return;
    }

    int old_width = image->width;
    int old_height = image->height;
    int channels = image->channels;

    unsigned char* new_data = ICYLIB_MALLOC_ATOMIC(new_width * new_height * channels);

    for (int j = 0; j < old_height; j++) {
        for (int i = 0; i < old_width; i++) {
            int old_index = (j * old_width + i) * channels;
            int new_index = (j * new_width + i) * channels;
            for (int c = 0; c < channels; c++) {
                new_data[new_index + c] = image->data[old_index + c];
            }
        }
    }

    ICYLIB_FREE(image->data);

    image->data = new_data;
    image->width = new_width;
    image->height = new_height;
}

void icylib_regular_resize(icylib_RegularImage* image, int width, int height) {
    unsigned char* new_data = ICYLIB_MALLOC_ATOMIC(width * height * image->channels);
    stbir_resize_uint8_linear(image->data, image->width, image->height, 0, new_data, width, height, 0, (stbir_pixel_layout)image->channels);
    ICYLIB_FREE(image->data);
    image->data = new_data;
    image->width = width;
    image->height = height;
}

void icylib_regular_resize_scale(icylib_RegularImage* image, float scale) {
    icylib_regular_resize(image, image->width * scale, image->height * scale);
}

#endif

#endif