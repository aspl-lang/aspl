#ifndef ICYLIB_PRIMITIVE_CHUNKED_IMAGE_H
#define ICYLIB_PRIMITIVE_CHUNKED_IMAGE_H

#include <math.h>

#include "thirdparty/stb_image_write.h"

#include "icylib.h"
#include "regular_image.h"
#include "color.h"
#include "thick_xiaolin_wu.h"
#include "text.h"
#include "math_utils.h"

const unsigned int ICYLIB_PRIMITIVE_CHUNK_WIDTH = 256;
const unsigned int ICYLIB_PRIMITIVE_CHUNK_HEIGHT = 256;

typedef struct icylib_PrimitiveChunkRow {
    icylib_RegularImage** chunk_columns;
} icylib_PrimitiveChunkRow;

typedef struct icylib_PrimitiveChunkedImage {
    int width;
    int height;
    int channels;
    icylib_PrimitiveChunkRow* chunk_rows;
    int rows_length;
    int columns_length;
} icylib_PrimitiveChunkedImage;

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_create_from_size(int width, int height, int channels);

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_create_from_memory(unsigned char* data, int width, int height, int channels);

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_load_from_file_data(unsigned char* data, int length);

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_load_from_file(const char* filename);

unsigned char* icylib_primitive_chunked_save_to_file_data(icylib_PrimitiveChunkedImage* image, icylib_RegularImageFormat format, int* length);

int icylib_primitive_chunked_save_to_file(icylib_PrimitiveChunkedImage* image, const char* filename);

void icylib_primitive_chunked_free(icylib_PrimitiveChunkedImage* image);

icylib_Color icylib_primitive_chunked_get_pixel(icylib_PrimitiveChunkedImage* image, int x, int y);

void icylib_primitive_chunked_set_pixel(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_Color color);

void icylib_primitive_chunked_set_pixel_blend(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_Color color);

void icylib_primitive_chunked_fill(icylib_PrimitiveChunkedImage* image, icylib_Color color, unsigned char blend);

void icylib_primitive_chunked_draw_rectangle(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_primitive_chunked_fill_rectangle(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_primitive_chunked_draw_image(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend);

void icylib_primitive_chunked_draw_circle(icylib_PrimitiveChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_primitive_chunked_fill_circle(icylib_PrimitiveChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_primitive_chunked_draw_line_with_thickness(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_primitive_chunked_draw_line(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_primitive_chunked_draw_text(icylib_PrimitiveChunkedImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend);

void icylib_primitive_chunked_replace_color(icylib_PrimitiveChunkedImage* image, icylib_Color old_color, icylib_Color new_color, unsigned char blend);

void icylib_primitive_chunked_replace_color_ignore_alpha(icylib_PrimitiveChunkedImage* image, icylib_Color old_color, icylib_Color new_color);

void icylib_primitive_chunked_blur(icylib_PrimitiveChunkedImage* image, int radius);

icylib_RegularImage* icylib_primitive_chunked_get_sub_image(icylib_PrimitiveChunkedImage* image, int x, int y, int width, int height);

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_copy(icylib_PrimitiveChunkedImage* image);

void icylib_primitive_chunked_extend_to(icylib_PrimitiveChunkedImage* image, int new_width, int new_height);

void icylib_primitive_chunked_resize(icylib_PrimitiveChunkedImage* image, int width, int height);

void icylib_primitive_chunked_resize_scale(icylib_PrimitiveChunkedImage* image, float scale);

#ifdef ICYLIB_IMPLEMENTATION

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_create_from_size(int width, int height, int channels) {
    icylib_PrimitiveChunkRow* chunk_rows = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkRow) * icylib_ceil((float)height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT));
    for (int j = 0; j < icylib_ceil((float)height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        icylib_PrimitiveChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * icylib_ceil((float)width / ICYLIB_PRIMITIVE_CHUNK_WIDTH));
        for (int i = 0; i < icylib_ceil((float)width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            chunk_columns[i] = icylib_regular_create_from_size(ICYLIB_PRIMITIVE_CHUNK_WIDTH, ICYLIB_PRIMITIVE_CHUNK_HEIGHT, channels);
        }
        row.chunk_columns = chunk_columns;
        chunk_rows[j] = row;
    }
    icylib_PrimitiveChunkedImage* image = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkedImage));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->chunk_rows = chunk_rows;
    image->rows_length = height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
    image->columns_length = width / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
    return image;
}

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_load_from_file_data(unsigned char* data, int length) {
    int width = (int)data[0] | (int)data[1] << 8 | (int)data[2] << 16 | (int)data[3] << 24;
    int height = (int)data[4] | (int)data[5] << 8 | (int)data[6] << 16 | (int)data[7] << 24;
    int rows = (int)data[8] | (int)data[9] << 8 | (int)data[10] << 16 | (int)data[11] << 24;
    int columns = (int)data[12] | (int)data[13] << 8 | (int)data[14] << 16 | (int)data[15] << 24;
    unsigned int data_pointer = 16;
    icylib_PrimitiveChunkRow* chunk_rows = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkRow) * rows);
    for (int j = 0; j < rows; j++) {
        icylib_PrimitiveChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * columns);
        for (int i = 0; i < columns; i++) {
            int chunk_data_length = (int)data[data_pointer] | (int)data[data_pointer + 1] << 8 | (int)data[data_pointer + 2] << 16 | (int)data[data_pointer + 3] << 24;
            data_pointer += 4;
            unsigned char* chunk_data = ICYLIB_MALLOC_ATOMIC(chunk_data_length);
            memcpy(chunk_data, data + data_pointer, chunk_data_length);
            data_pointer += chunk_data_length;
            chunk_columns[i] = icylib_regular_load_from_file_data(chunk_data, chunk_data_length);
        }
        row.chunk_columns = chunk_columns;
        chunk_rows[j] = row;
    }
    icylib_PrimitiveChunkedImage* image = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkedImage));
    image->width = width;
    image->height = height;
    image->channels = 4;
    image->chunk_rows = chunk_rows;
    image->rows_length = rows;
    image->columns_length = columns;
    return image;
}

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_load_from_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        ICYLIB_ERROR("Failed to open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char* data = ICYLIB_MALLOC_ATOMIC(length);
    fread(data, 1, length, file);
    fclose(file);

    icylib_PrimitiveChunkedImage* image = icylib_primitive_chunked_load_from_file_data(data, length);
    ICYLIB_FREE(data);
    return image;
}

unsigned char* icylib_primitive_chunked_save_to_file_data(icylib_PrimitiveChunkedImage* image, icylib_RegularImageFormat format, int* length) {
    unsigned char* data = ICYLIB_MALLOC_ATOMIC(16);
    unsigned int data_length = 16;

    data[0] = (image->width >> 0) & 0xFF;
    data[1] = (image->width >> 8) & 0xFF;
    data[2] = (image->width >> 16) & 0xFF;
    data[3] = (image->width >> 24) & 0xFF;

    data[4] = (image->height >> 0) & 0xFF;
    data[5] = (image->height >> 8) & 0xFF;
    data[6] = (image->height >> 16) & 0xFF;
    data[7] = (image->height >> 24) & 0xFF;

    data[8] = (image->rows_length >> 0) & 0xFF;
    data[9] = (image->rows_length >> 8) & 0xFF;
    data[10] = (image->rows_length >> 16) & 0xFF;
    data[11] = (image->rows_length >> 24) & 0xFF;

    data[12] = (image->columns_length >> 0) & 0xFF;
    data[13] = (image->columns_length >> 8) & 0xFF;
    data[14] = (image->columns_length >> 16) & 0xFF;
    data[15] = (image->columns_length >> 24) & 0xFF;

    for (int j = 0; j < image->rows_length; j++) {
        for (int i = 0; i < image->columns_length; i++) {
            int chunk_data_length = 0;
            unsigned char* chunk_data = icylib_regular_save_to_file_data(image->chunk_rows[j].chunk_columns[i], format, &chunk_data_length);
            data = ICYLIB_REALLOC(data, data_length + chunk_data_length + 4);
            data[data_length] = (chunk_data_length >> 0) & 0xFF;
            data[data_length + 1] = (chunk_data_length >> 8) & 0xFF;
            data[data_length + 2] = (chunk_data_length >> 16) & 0xFF;
            data[data_length + 3] = (chunk_data_length >> 24) & 0xFF;
            memcpy(data + data_length + 4, chunk_data, chunk_data_length);
            data_length += chunk_data_length + 4;
            ICYLIB_FREE(chunk_data);
        }
    }

    *length = data_length;
    return data;
}

int icylib_primitive_chunked_save_to_file(icylib_PrimitiveChunkedImage* image, const char* filename) {
    const char* extension = strrchr(filename, '.');
    icylib_RegularImageFormat format;
    if (extension == NULL) {
        format = ICYLIB_REGULAR_IMAGE_FORMAT_PNG;
    }
    else {
        if (strcmp(extension, ".png") == 0) {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_PNG;
        }
        else if (strcmp(extension, ".jpg") == 0) {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_JPG;
        }
        else if (strcmp(extension, ".bmp") == 0) {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_BMP;
        }
        else if (strcmp(extension, ".tga") == 0) {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_TGA;
        }
        else if (strcmp(extension, ".hdr") == 0) {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_HDR;
        }
        else {
            format = ICYLIB_REGULAR_IMAGE_FORMAT_PNG;
        }
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        ICYLIB_ERROR("Failed to open file");
        return 0;
    }
    int length;
    unsigned char* data = icylib_primitive_chunked_save_to_file_data(image, format, &length);
    fwrite(data, 1, length, file);
    fclose(file);
    ICYLIB_FREE(data);
    return 1;
}

void icylib_primitive_chunked_free(icylib_PrimitiveChunkedImage* image) {
    for (int j = 0; j < (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_free(image->chunk_rows[j].chunk_columns[i]);
        }
    }
    ICYLIB_FREE(image->chunk_rows);
    ICYLIB_FREE(image);
}

icylib_Color icylib_primitive_chunked_get_pixel(icylib_PrimitiveChunkedImage* image, int x, int y) {
    return icylib_regular_get_pixel(image->chunk_rows[y / ICYLIB_PRIMITIVE_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_PRIMITIVE_CHUNK_WIDTH], x % ICYLIB_PRIMITIVE_CHUNK_WIDTH, y % ICYLIB_PRIMITIVE_CHUNK_HEIGHT);
}

void icylib_primitive_chunked_set_pixel(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_Color color) {
    icylib_regular_set_pixel(image->chunk_rows[y / ICYLIB_PRIMITIVE_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_PRIMITIVE_CHUNK_WIDTH], x % ICYLIB_PRIMITIVE_CHUNK_WIDTH, y % ICYLIB_PRIMITIVE_CHUNK_HEIGHT, color);
}

void icylib_primitive_chunked_set_pixel_blend(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_Color color) {
    icylib_regular_set_pixel_blend(image->chunk_rows[y / ICYLIB_PRIMITIVE_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_PRIMITIVE_CHUNK_WIDTH], x % ICYLIB_PRIMITIVE_CHUNK_WIDTH, y % ICYLIB_PRIMITIVE_CHUNK_HEIGHT, color);
}

void icylib_primitive_chunked_fill(icylib_PrimitiveChunkedImage* image, icylib_Color color, unsigned char blend) {
    for (int j = 0; j < icylib_floor((float)image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < icylib_floor((float)image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_fill(image->chunk_rows[j].chunk_columns[i], color, blend);
        }
    }
    if (image->width % ICYLIB_PRIMITIVE_CHUNK_WIDTH != 0) {
        for (int j = 0; j < icylib_floor((float)image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
            icylib_regular_fill_rectangle(image->chunk_rows[j].chunk_columns[icylib_floor((float)image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH)], 0, 0, image->width % ICYLIB_PRIMITIVE_CHUNK_WIDTH, ICYLIB_PRIMITIVE_CHUNK_HEIGHT, color, blend);
        }
    }
    if (image->height % ICYLIB_PRIMITIVE_CHUNK_HEIGHT != 0) {
        for (int i = 0; i < icylib_floor((float)image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_fill_rectangle(image->chunk_rows[icylib_floor((float)image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT)].chunk_columns[i], 0, 0, ICYLIB_PRIMITIVE_CHUNK_WIDTH, image->height % ICYLIB_PRIMITIVE_CHUNK_HEIGHT, color, blend);
        }
    }
}

void icylib_primitive_chunked_draw_rectangle(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    icylib_primitive_chunked_draw_line(image, x1, y1, x2, y1, color, blend, 0);
    icylib_primitive_chunked_draw_line(image, x1, y1, x1, y2, color, blend, 0);
    icylib_primitive_chunked_draw_line(image, x2, y1, x2, y2, color, blend, 0);
    icylib_primitive_chunked_draw_line(image, x1, y2, x2, y2, color, blend, 0);
}

void icylib_primitive_chunked_fill_rectangle(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    for (int j = icylib_imax(0, y1); j < icylib_imin(image->height, y2); j++) {
        for (int i = icylib_imax(0, x1); i < icylib_imin(image->width, x2); i++) {
            if (blend) {
                icylib_primitive_chunked_set_pixel_blend(image, i, j, color);
            }
            else {
                icylib_primitive_chunked_set_pixel(image, i, j, color);
            }
        }
    }
}

void icylib_primitive_chunked_draw_image(icylib_PrimitiveChunkedImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend) {
    if (blend) {
        for (int j = 0; j < other->height; j++) {
            for (int i = 0; i < other->width; i++) {
                icylib_primitive_chunked_set_pixel_blend(image, x + i, y + j, icylib_color_from_rgba(other->data[(j * other->width + i) * other->channels], other->data[(j * other->width + i) * other->channels + 1], other->data[(j * other->width + i) * other->channels + 2], other->channels == 4 ? other->data[(j * other->width + i) * other->channels + 3] : 255));
            }
        }
    }
    else {
        for (int j = 0; j < other->height; j++) {
            for (int i = 0; i < other->width; i++) {
                icylib_primitive_chunked_set_pixel(image, x + i, y + j, icylib_color_from_rgba(other->data[(j * other->width + i) * other->channels], other->data[(j * other->width + i) * other->channels + 1], other->data[(j * other->width + i) * other->channels + 2], other->channels == 4 ? other->data[(j * other->width + i) * other->channels + 3] : 255));
            }
        }
    }
}

void icylib_primitive_chunked_draw_circle(icylib_PrimitiveChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    icylib_draw_bresenham_circle((unsigned char*)image, x, y, radius, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_primitive_chunked_set_pixel_blend : icylib_primitive_chunked_set_pixel));
}

void icylib_primitive_chunked_fill_circle(icylib_PrimitiveChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    int rsquared = radius * radius;
    for (int j = -radius; j < radius; j++) {
        for (int i = -radius; i < radius; i++) {
            if ((i * i + j * j) < rsquared) {
                if (blend) {
                    icylib_primitive_chunked_set_pixel_blend(image, x + i, y + j, color);
                }
                else {
                    icylib_primitive_chunked_set_pixel(image, x + i, y + j, color);
                }
            }
        }
    }
}

void icylib_primitive_chunked_draw_line_with_thickness(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias) {
    if (antialias) {
        icylib_draw_thick_xiaolin_wu_aa_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_primitive_chunked_set_pixel_blend : icylib_primitive_chunked_set_pixel));
    }
    else {
        icylib_draw_bresenham_thick_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_primitive_chunked_set_pixel_blend : icylib_primitive_chunked_set_pixel));
    }
}

void icylib_primitive_chunked_draw_line(icylib_PrimitiveChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias) {
    icylib_primitive_chunked_draw_line_with_thickness(image, x1, y1, x2, y2, 1, color, blend, antialias);
}

void icylib_primitive_chunked_draw_text(icylib_PrimitiveChunkedImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend) {
    double width, height;
    icylib_measure_text_size(text, fontPath, pixelSize, &width, &height);

    stbtt_fontinfo font;
    icylib_FontCache* fonts = icylib_get_font_cache();

    char found = 0;
    for (int i = 0; i < fonts->length; ++i) {
        if (strcmp(fonts->data[i].name, fontPath) == 0) {
            font = *fonts->data[i].font;
            found = 1;
            break;
        }
    }
    if (!found) {
        unsigned char* font_file;
        icylib_read_bytes(fontPath, &font_file);

        int offset = stbtt_GetFontOffsetForIndex(font_file, 0);
        stbtt_InitFont(&font, font_file, offset);

        fonts->data = ICYLIB_REALLOC(fonts->data, sizeof(icylib_FontCacheEntry) * (fonts->length + 1));
        fonts->data[fonts->length].name = fontPath;
        fonts->data[fonts->length].font = ICYLIB_MALLOC(sizeof(stbtt_fontinfo));
        memcpy(fonts->data[fonts->length].font, &font, sizeof(stbtt_fontinfo));
        fonts->length++;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, pixelSize);

    int ascent, zero;
    stbtt_GetFontVMetrics(&font, &ascent, &zero, &zero);

    int advance, lsb, x0, y0, x1, y1;

    float x_cursor = x;
    if (horizontalAlignment == ICYLIB_HORIZONTAL_ALIGNMENT_CENTER) {
        x_cursor -= width / 2;
    }
    else if (horizontalAlignment == ICYLIB_HORIZONTAL_ALIGNMENT_RIGHT) {
        x_cursor -= width;
    }
    float y_cursor = y;
    if (verticalAlignment == ICYLIB_VERTICAL_ALIGNMENT_CENTER) {
        y_cursor -= height / 2;
    }
    else if (verticalAlignment == ICYLIB_VERTICAL_ALIGNMENT_TOP) {
        y_cursor -= height;
    }
    for (size_t i = 0; i < strlen(text); ++i) {
        if (text[i] == '\n') {
            y_cursor += ascent * scale;
            x_cursor = x;
            continue;
        }

        int c = (int)text[i];

        stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x0, &y0, &x1, &y1);

        int w = 0;
        int h = 0;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, 0, 0);
        for (int b = 0; b < h; ++b) {
            for (int a = 0; a < w; ++a) {
                if (bitmap[b * w + a] != 0) {
                    int pixel_x = (int)x_cursor + x0 + a;
                    int pixel_y = (int)y_cursor + y0 + b;
                    if (pixel_x < 0 || pixel_x >= image->width || pixel_y < 0 || pixel_y >= image->height) {
                        continue;
                    }
                    if (blend) {
                        icylib_primitive_chunked_set_pixel_blend(image, pixel_x, pixel_y, icylib_color_from_rgba(color.r, color.g, color.b, (unsigned char)(bitmap[b * w + a])));
                    }
                    else {
                        icylib_primitive_chunked_set_pixel(image, pixel_x, pixel_y, icylib_color_from_rgba(color.r, color.g, color.b, (unsigned char)(bitmap[b * w + a])));
                    }
                }
            }
        }

        if (i < strlen(text) - 1) {
            x_cursor += advance * scale;
            x_cursor += scale * stbtt_GetCodepointKernAdvance(&font, c, (int)text[i + 1]);
        }
    }
}

void icylib_primitive_chunked_replace_color(icylib_PrimitiveChunkedImage* image, icylib_Color old_color, icylib_Color new_color, unsigned char blend) {
    for (int j = 0; j < (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_replace_color(image->chunk_rows[j].chunk_columns[i], old_color, new_color, blend);
        }
    }
}

void icylib_primitive_chunked_replace_color_ignore_alpha(icylib_PrimitiveChunkedImage* image, icylib_Color old_color, icylib_Color new_color) {
    for (int j = 0; j < (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_replace_color_ignore_alpha(image->chunk_rows[j].chunk_columns[i], old_color, new_color);
        }
    }
}

void icylib_primitive_chunked_blur(icylib_PrimitiveChunkedImage* image, int radius) {
    for (int j = 0; j < (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_regular_blur(image->chunk_rows[j].chunk_columns[i], radius);
        }
    }
}

icylib_RegularImage* icylib_primitive_chunked_get_sub_image(icylib_PrimitiveChunkedImage* image, int x, int y, int width, int height) {
    unsigned int start_chunk_x = x / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
    unsigned int start_chunk_y = y / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
    unsigned int end_chunk_x = (x + width - 1) / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
    unsigned int end_chunk_y = (y + height - 1) / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
    icylib_RegularImage* img = icylib_regular_create_from_size(icylib_ceil(((float)(width + x % ICYLIB_PRIMITIVE_CHUNK_WIDTH)) / ICYLIB_PRIMITIVE_CHUNK_WIDTH) * ICYLIB_PRIMITIVE_CHUNK_WIDTH, icylib_ceil(((float)(height + y % ICYLIB_PRIMITIVE_CHUNK_HEIGHT)) / ICYLIB_PRIMITIVE_CHUNK_HEIGHT) * ICYLIB_PRIMITIVE_CHUNK_HEIGHT, image->channels);
    int j2 = 0;
    for (int j = start_chunk_y; j <= end_chunk_y; j++) {
        int i2 = 0;
        for (int i = start_chunk_x; i <= end_chunk_x; i++) {
            icylib_regular_draw_image(img, i2 * ICYLIB_PRIMITIVE_CHUNK_WIDTH, j2 * ICYLIB_PRIMITIVE_CHUNK_HEIGHT, image->chunk_rows[i].chunk_columns[j], 0);
            i2++;
        }
        j2++;
    }
    return icylib_regular_get_sub_image(img, x % ICYLIB_PRIMITIVE_CHUNK_WIDTH, y % ICYLIB_PRIMITIVE_CHUNK_HEIGHT, width, height);
}

icylib_PrimitiveChunkedImage* icylib_primitive_chunked_copy(icylib_PrimitiveChunkedImage* image) {
    icylib_PrimitiveChunkedImage* new_image = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkedImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->chunk_rows = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkRow*) * (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT));
    new_image->rows_length = image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
    new_image->columns_length = image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
    for (int j = 0; j < (image->height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        icylib_PrimitiveChunkRow row;
        row.chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH));
        for (int i = 0; i < (image->width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            row.chunk_columns[i] = icylib_regular_copy(image->chunk_rows[j].chunk_columns[i]);
        }
        new_image->chunk_rows[j] = row;
    }
    return new_image;
}

void icylib_primitive_chunked_extend_to(icylib_PrimitiveChunkedImage* image, int new_width, int new_height) {
    if (new_width > image->rows_length * ICYLIB_PRIMITIVE_CHUNK_WIDTH || new_height > image->columns_length * ICYLIB_PRIMITIVE_CHUNK_HEIGHT) {
        int new_rows_length = new_height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
        int new_columns_length = new_width / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
        icylib_PrimitiveChunkRow* new_chunk_rows = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkRow) * new_rows_length);
        for (int j = 0; j < new_rows_length; j++) {
            icylib_PrimitiveChunkRow row;
            icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * new_columns_length);
            for (int i = 0; i < new_columns_length; i++) {
                if (j < image->rows_length && i < image->columns_length) {
                    chunk_columns[i] = image->chunk_rows[j].chunk_columns[i];
                }
                else {
                    chunk_columns[i] = icylib_regular_create_from_size(ICYLIB_PRIMITIVE_CHUNK_WIDTH, ICYLIB_PRIMITIVE_CHUNK_HEIGHT, image->channels);
                }
            }
            row.chunk_columns = chunk_columns;
            new_chunk_rows[j] = row;
        }
        ICYLIB_FREE(image->chunk_rows);
        image->chunk_rows = new_chunk_rows;
        image->rows_length = new_rows_length;
        image->columns_length = new_columns_length;
    }
    image->width = new_width;
    image->height = new_height;
}

void icylib_primitive_chunked_resize(icylib_PrimitiveChunkedImage* image, int width, int height) {
    icylib_primitive_chunked_extend_to(image, width, height);
    // scale the invididual chunks and split them up into multiple chunks
    icylib_PrimitiveChunkRow* new_chunk_rows = ICYLIB_MALLOC(sizeof(icylib_PrimitiveChunkRow) * icylib_ceil((float)height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT));
    for (int j = 0; j < icylib_ceil((float)height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT); j++) {
        icylib_PrimitiveChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * icylib_ceil((float)width / ICYLIB_PRIMITIVE_CHUNK_WIDTH));
        for (int i = 0; i < icylib_ceil((float)width / ICYLIB_PRIMITIVE_CHUNK_WIDTH); i++) {
            icylib_RegularImage* sub = icylib_primitive_chunked_get_sub_image(image, i * ICYLIB_PRIMITIVE_CHUNK_WIDTH, j * ICYLIB_PRIMITIVE_CHUNK_HEIGHT, ICYLIB_PRIMITIVE_CHUNK_WIDTH, ICYLIB_PRIMITIVE_CHUNK_HEIGHT);
            icylib_regular_resize(sub, ICYLIB_PRIMITIVE_CHUNK_WIDTH, ICYLIB_PRIMITIVE_CHUNK_HEIGHT);
            chunk_columns[i] = sub;
        }
        row.chunk_columns = chunk_columns;
        new_chunk_rows[j] = row;
    }
    ICYLIB_FREE(image->chunk_rows);
    image->chunk_rows = new_chunk_rows;
    image->rows_length = width / ICYLIB_PRIMITIVE_CHUNK_WIDTH;
    image->columns_length = height / ICYLIB_PRIMITIVE_CHUNK_HEIGHT;
    image->width = width;
    image->height = height;
}

void icylib_primitive_chunked_resize_scale(icylib_PrimitiveChunkedImage* image, float scale) {
    icylib_primitive_chunked_resize(image, image->width * scale, image->height * scale);
}

#endif

#endif