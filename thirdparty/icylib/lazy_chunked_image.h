#ifndef ICYLIB_LAZY_CHUNKED_IMAGE_H
#define ICYLIB_LAZY_CHUNKED_IMAGE_H

#include <math.h>

#include "thirdparty/stb_image_write.h"

#include "icylib.h"
#include "regular_image.h"
#include "color.h"
#include "thick_xiaolin_wu.h"
#include "text.h"
#include "math_utils.h"
#include "file_utils.h"

const unsigned int ICYLIB_LAZY_CHUNK_WIDTH = 256;
const unsigned int ICYLIB_LAZY_CHUNK_HEIGHT = 256;

typedef struct icylib_LazyChunkRow {
    icylib_RegularImage** chunk_columns;
} icylib_LazyChunkRow;

typedef struct icylib_LazyChunkedImage {
    int width;
    int height;
    int channels;
    icylib_LazyChunkRow* chunk_rows;
    int rows_length;
    int columns_length;
    const char* directory;
} icylib_LazyChunkedImage;

icylib_LazyChunkedImage* icylib_lazy_chunked_create_from_size(int width, int height, int channels, const char* directory);

icylib_LazyChunkedImage* icylib_lazy_chunked_create_from_memory(unsigned char* data, int width, int height, int channels);

icylib_LazyChunkedImage* icylib_lazy_chunked_load_from_file(const char* filename, const char* directory);

int icylib_lazy_chunked_save_to_file(icylib_LazyChunkedImage* image, const char* filename, int* length);

void icylib_lazy_chunked_free(icylib_LazyChunkedImage* image);

char icylib_lazy_chunked_is_chunk_loaded(icylib_LazyChunkedImage* image, int x, int y);

void icylib_lazy_chunked_load_chunk(icylib_LazyChunkedImage* image, int x, int y);

void icylib_lazy_chunked_unload_chunk(icylib_LazyChunkedImage* image, int x, int y);

void icylib_lazy_chunked_require_area(icylib_LazyChunkedImage* image, int x, int y, int width, int height);

icylib_Color icylib_lazy_chunked_get_pixel(icylib_LazyChunkedImage* image, int x, int y);

void icylib_lazy_chunked_set_pixel(icylib_LazyChunkedImage* image, int x, int y, icylib_Color color);

void icylib_lazy_chunked_set_pixel_blend(icylib_LazyChunkedImage* image, int x, int y, icylib_Color color);

void icylib_lazy_chunked_fill(icylib_LazyChunkedImage* image, icylib_Color color, unsigned char blend);

void icylib_lazy_chunked_draw_rectangle(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_lazy_chunked_fill_rectangle(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend);

void icylib_lazy_chunked_draw_image(icylib_LazyChunkedImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend);

void icylib_lazy_chunked_draw_circle(icylib_LazyChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_lazy_chunked_fill_circle(icylib_LazyChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend);

void icylib_lazy_chunked_draw_line_with_thickness(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_lazy_chunked_draw_line(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias);

void icylib_lazy_chunked_draw_text(icylib_LazyChunkedImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend);

void icylib_lazy_chunked_replace_color(icylib_LazyChunkedImage* image, icylib_Color old_color, icylib_Color new_color, unsigned char blend);

void icylib_lazy_chunked_replace_color_ignore_alpha(icylib_LazyChunkedImage* image, icylib_Color old_color, icylib_Color new_color);

void icylib_lazy_chunked_blur(icylib_LazyChunkedImage* image, int radius);

icylib_RegularImage* icylib_lazy_chunked_get_sub_image(icylib_LazyChunkedImage* image, int x, int y, int width, int height);

icylib_LazyChunkedImage* icylib_lazy_chunked_copy(icylib_LazyChunkedImage* image);

void icylib_lazy_chunked_extend_to(icylib_LazyChunkedImage* image, int new_width, int new_height);

void icylib_lazy_chunked_resize(icylib_LazyChunkedImage* image, int width, int height);

void icylib_lazy_chunked_resize_scale(icylib_LazyChunkedImage* image, float scale);

#ifdef ICYLIB_IMPLEMENTATION

icylib_LazyChunkedImage* icylib_lazy_chunked_create_from_size(int width, int height, int channels, const char* directory) {
    icylib_LazyChunkRow* chunk_rows = ICYLIB_MALLOC(sizeof(icylib_LazyChunkRow) * icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT));
    for (int j = 0; j < icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        icylib_LazyChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH));
        for (int i = 0; i < icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            chunk_columns[i] = icylib_regular_create_from_size(ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT, channels);
        }
        row.chunk_columns = chunk_columns;
        chunk_rows[j] = row;
    }
    icylib_LazyChunkedImage* image = ICYLIB_MALLOC(sizeof(icylib_LazyChunkedImage));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->chunk_rows = chunk_rows;
    image->rows_length = icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT);
    image->columns_length = icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH);
    image->directory = directory;
    return image;
}

icylib_LazyChunkedImage* icylib_lazy_chunked_load_from_file(const char* filename, const char* directory) {
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

    int width = (int)data[0] | (int)data[1] << 8 | (int)data[2] << 16 | (int)data[3] << 24;
    int height = (int)data[4] | (int)data[5] << 8 | (int)data[6] << 16 | (int)data[7] << 24;
    int columns = (int)data[8] | (int)data[9] << 8 | (int)data[10] << 16 | (int)data[11] << 24;
    int rows = (int)data[12] | (int)data[13] << 8 | (int)data[14] << 16 | (int)data[15] << 24;
    icylib_LazyChunkRow* chunk_rows = ICYLIB_MALLOC(sizeof(icylib_LazyChunkRow) * rows);
    for (int j = 0; j < rows; j++) {
        icylib_LazyChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * columns);
        for (int i = 0; i < columns; i++) {
            chunk_columns[i] = NULL;
        }
        row.chunk_columns = chunk_columns;
        chunk_rows[j] = row;
    }
    icylib_LazyChunkedImage* image = ICYLIB_MALLOC(sizeof(icylib_LazyChunkedImage));
    image->width = width;
    image->height = height;
    image->channels = 4;
    image->chunk_rows = chunk_rows;
    image->rows_length = rows;
    image->columns_length = columns;
    image->directory = directory;

    ICYLIB_FREE(data);
    return image;
}

int icylib_lazy_chunked_save_to_file(icylib_LazyChunkedImage* image, const char* filename, int* length) {
    unsigned int data_length = 16;
    unsigned char* data = ICYLIB_MALLOC_ATOMIC(data_length);

    data[0] = (image->width >> 0) & 0xFF;
    data[1] = (image->width >> 8) & 0xFF;
    data[2] = (image->width >> 16) & 0xFF;
    data[3] = (image->width >> 24) & 0xFF;

    data[4] = (image->height >> 0) & 0xFF;
    data[5] = (image->height >> 8) & 0xFF;
    data[6] = (image->height >> 16) & 0xFF;
    data[7] = (image->height >> 24) & 0xFF;

    data[8] = (image->columns_length >> 0) & 0xFF;
    data[9] = (image->columns_length >> 8) & 0xFF;
    data[10] = (image->columns_length >> 16) & 0xFF;
    data[11] = (image->columns_length >> 24) & 0xFF;

    data[12] = (image->rows_length >> 0) & 0xFF;
    data[13] = (image->rows_length >> 8) & 0xFF;
    data[14] = (image->rows_length >> 16) & 0xFF;
    data[15] = (image->rows_length >> 24) & 0xFF;

    for (int j = 0; j < image->rows_length; j++) {
        for (int i = 0; i < image->columns_length; i++) {
            if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                char* path = ICYLIB_MALLOC_ATOMIC(strlen(image->directory)
                    + 1 // '/'
                    + 1 // '\0'
                    + 1 // '_'
                    + 10 // max int length
                    + 1 // '_'
                    + 10 // max int length
                    + 4 // ".png"
                );
                sprintf(path, "%s/%d_%d.png", image->directory, i, j);
                while (1) {
                    if (icylib_regular_save_to_file(image->chunk_rows[j].chunk_columns[i], path)) break;
                    // TODO: Add a limit to the number of retries
                }
                ICYLIB_FREE(path);
            }
        }
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        ICYLIB_ERROR("Failed to open file");
        return 0;
    }
    fwrite(data, 1, data_length, file);
    fclose(file);
    ICYLIB_FREE(data);

    *length = data_length;
    return 1;
}

void icylib_lazy_chunked_free(icylib_LazyChunkedImage* image) {
    for (int j = 0; j < (image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                icylib_regular_free(image->chunk_rows[j].chunk_columns[i]);
            }
        }
    }
    ICYLIB_FREE(image->chunk_rows);
    ICYLIB_FREE(image);
}

char icylib_lazy_chunked_is_chunk_loaded(icylib_LazyChunkedImage* image, int x, int y) {
    return image->chunk_rows[y].chunk_columns[x] != NULL;
}

void icylib_lazy_chunked_load_chunk(icylib_LazyChunkedImage* image, int x, int y) {
    if (image->chunk_rows[y].chunk_columns[x] == NULL) {
        char* path = ICYLIB_MALLOC_ATOMIC(strlen(image->directory)
            + 1 // '/'
            + 1 // '\0'
            + 1 // '_'
            + 10 // max int length
            + 1 // '_'
            + 10 // max int length
            + 4 // ".png"
        );
        sprintf(path, "%s/%d_%d.png", image->directory, x, y);
        if (icylib_file_exists(path) && icylib_file_size(path) == 0) { // safety measure in case chunks got corrupted somehow
            icylib_file_delete(path);
            icylib_RegularImage* chunk = icylib_regular_create_from_size(ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT, image->channels);
            image->chunk_rows[y].chunk_columns[x] = chunk;
        }
        else {
            icylib_RegularImage* f;
            while (1) {
                f = icylib_regular_load_from_file(path);
                if (f != NULL) break;
                // TODO: Add a limit to the number of retries
            }
            image->chunk_rows[y].chunk_columns[x] = f;
        }
        ICYLIB_FREE(path);
    }
}

void icylib_lazy_chunked_unload_chunk(icylib_LazyChunkedImage* image, int x, int y) {
    if (image->chunk_rows[y].chunk_columns[x] != NULL) {
        char* path = ICYLIB_MALLOC_ATOMIC(strlen(image->directory)
            + 1 // '/'
            + 1 // '\0'
            + 1 // '_'
            + 10 // max int length
            + 1 // '_'
            + 10 // max int length
            + 4 // ".png"
        );
        sprintf(path, "%s/%d_%d.png", image->directory, x, y);
        while (1) {
            if (icylib_regular_save_to_file(image->chunk_rows[y].chunk_columns[x], path)) break;
            // TODO: Add a limit to the number of retries
        }
        ICYLIB_FREE(path);
        icylib_regular_free(image->chunk_rows[y].chunk_columns[x]);
        image->chunk_rows[y].chunk_columns[x] = NULL;
    }
}

void icylib_lazy_chunked_require_area(icylib_LazyChunkedImage* image, int x, int y, int width, int height) {
    for (int j = 0; j < image->rows_length; j++) {
        for (int i = 0; i < image->columns_length; i++) {
            if (i * ICYLIB_LAZY_CHUNK_WIDTH <= x + width && i * ICYLIB_LAZY_CHUNK_WIDTH + ICYLIB_LAZY_CHUNK_WIDTH >= x
                && j * ICYLIB_LAZY_CHUNK_HEIGHT <= y + height && j * ICYLIB_LAZY_CHUNK_HEIGHT + ICYLIB_LAZY_CHUNK_HEIGHT >= y) {
                if (image->chunk_rows[j].chunk_columns[i] == NULL) {
                    char* path = ICYLIB_MALLOC_ATOMIC(strlen(image->directory)
                        + 1 // '/'
                        + 1 // '\0'
                        + 1 // '_'
                        + 10 // max int length
                        + 1 // '_'
                        + 10 // max int length
                        + 4 // ".png"
                    );
                    sprintf(path, "%s/%d_%d.png", image->directory, i, j);
                    if (icylib_file_exists(path)) {
                        icylib_lazy_chunked_load_chunk(image, i, j);
                    }
                    else {
                        image->chunk_rows[j].chunk_columns[i] = icylib_regular_create_from_size(ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT, image->channels);
                    }
                    ICYLIB_FREE(path);
                }
            }
            else {
                if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                    icylib_lazy_chunked_unload_chunk(image, i, j);
                }
            }
        }
    }
}

icylib_Color icylib_lazy_chunked_get_pixel(icylib_LazyChunkedImage* image, int x, int y) {
    return icylib_regular_get_pixel(image->chunk_rows[y / ICYLIB_LAZY_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_LAZY_CHUNK_WIDTH], x % ICYLIB_LAZY_CHUNK_WIDTH, y % ICYLIB_LAZY_CHUNK_HEIGHT);
}

void icylib_lazy_chunked_set_pixel(icylib_LazyChunkedImage* image, int x, int y, icylib_Color color) {
    icylib_regular_set_pixel(image->chunk_rows[y / ICYLIB_LAZY_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_LAZY_CHUNK_WIDTH], x % ICYLIB_LAZY_CHUNK_WIDTH, y % ICYLIB_LAZY_CHUNK_HEIGHT, color);
}

void icylib_lazy_chunked_set_pixel_blend(icylib_LazyChunkedImage* image, int x, int y, icylib_Color color) {
    icylib_regular_set_pixel_blend(image->chunk_rows[y / ICYLIB_LAZY_CHUNK_HEIGHT].chunk_columns[x / ICYLIB_LAZY_CHUNK_WIDTH], x % ICYLIB_LAZY_CHUNK_WIDTH, y % ICYLIB_LAZY_CHUNK_HEIGHT, color);
}

void icylib_lazy_chunked_fill(icylib_LazyChunkedImage* image, icylib_Color color, unsigned char blend) {
    for (int j = 0; j < icylib_floor((float)image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < icylib_floor((float)image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            icylib_regular_fill(image->chunk_rows[j].chunk_columns[i], color, blend);
        }
    }
    if (image->width % ICYLIB_LAZY_CHUNK_WIDTH != 0) {
        for (int j = 0; j < icylib_floor((float)image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
            icylib_regular_fill_rectangle(image->chunk_rows[j].chunk_columns[icylib_floor((float)image->width / ICYLIB_LAZY_CHUNK_WIDTH)], 0, 0, image->width % ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT, color, blend);
        }
    }
    if (image->height % ICYLIB_LAZY_CHUNK_HEIGHT != 0) {
        for (int i = 0; i < icylib_floor((float)image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            icylib_regular_fill_rectangle(image->chunk_rows[icylib_floor((float)image->height / ICYLIB_LAZY_CHUNK_HEIGHT)].chunk_columns[i], 0, 0, ICYLIB_LAZY_CHUNK_WIDTH, image->height % ICYLIB_LAZY_CHUNK_HEIGHT, color, blend);
        }
    }
}

void icylib_lazy_chunked_draw_rectangle(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    icylib_lazy_chunked_draw_line(image, x1, y1, x2, y1, color, blend, 0);
    icylib_lazy_chunked_draw_line(image, x2, y1, x2, y2, color, blend, 0);
    icylib_lazy_chunked_draw_line(image, x2, y2, x1, y2, color, blend, 0);
    icylib_lazy_chunked_draw_line(image, x1, y2, x1, y1, color, blend, 0);
}

void icylib_lazy_chunked_fill_rectangle(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend) {
    for (int j = icylib_imax(0, y1); j < icylib_imin(image->height, y2); j++) {
        for (int i = icylib_imax(0, x1); i < icylib_imin(image->width, x2); i++) {
            if (blend) {
                icylib_lazy_chunked_set_pixel_blend(image, i, j, color);
            }
            else {
                icylib_lazy_chunked_set_pixel(image, i, j, color);
            }
        }
    }
}

void icylib_lazy_chunked_draw_image(icylib_LazyChunkedImage* image, int x, int y, icylib_RegularImage* other, unsigned char blend) {
    if (blend) {
        for (int j = 0; j < other->height; j++) {
            for (int i = 0; i < other->width; i++) {
                icylib_lazy_chunked_set_pixel_blend(image, x + i, y + j, icylib_color_from_rgba(other->data[(j * other->width + i) * other->channels], other->data[(j * other->width + i) * other->channels + 1], other->data[(j * other->width + i) * other->channels + 2], other->channels == 4 ? other->data[(j * other->width + i) * other->channels + 3] : 255));
            }
        }
    }
    else {
        for (int j = 0; j < other->height; j++) {
            for (int i = 0; i < other->width; i++) {
                icylib_lazy_chunked_set_pixel(image, x + i, y + j, icylib_color_from_rgba(other->data[(j * other->width + i) * other->channels], other->data[(j * other->width + i) * other->channels + 1], other->data[(j * other->width + i) * other->channels + 2], other->channels == 4 ? other->data[(j * other->width + i) * other->channels + 3] : 255));
            }
        }
    }
}

void icylib_lazy_chunked_draw_circle(icylib_LazyChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    icylib_draw_bresenham_circle((unsigned char*)image, x, y, radius, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_lazy_chunked_set_pixel_blend : icylib_lazy_chunked_set_pixel));
}

void icylib_lazy_chunked_fill_circle(icylib_LazyChunkedImage* image, int x, int y, int radius, icylib_Color color, unsigned char blend) {
    int rsquared = radius * radius;
    for (int j = -radius; j < radius; j++) {
        for (int i = -radius; i < radius; i++) {
            if ((i * i + j * j) < rsquared) {
                if (blend) {
                    icylib_lazy_chunked_set_pixel_blend(image, x + i, y + j, color);
                }
                else {
                    icylib_lazy_chunked_set_pixel(image, x + i, y + j, color);
                }
            }
        }
    }
}

void icylib_lazy_chunked_draw_line_with_thickness(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, int thickness, icylib_Color color, unsigned char blend, unsigned char antialias) {
    if (antialias) {
        icylib_draw_thick_xiaolin_wu_aa_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_lazy_chunked_set_pixel_blend : icylib_lazy_chunked_set_pixel));
    }
    else {
        icylib_draw_bresenham_thick_line((unsigned char*)image, x1, y1, x2, y2, thickness, color, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_lazy_chunked_set_pixel_blend : icylib_lazy_chunked_set_pixel));
    }
}

void icylib_lazy_chunked_draw_line(icylib_LazyChunkedImage* image, int x1, int y1, int x2, int y2, icylib_Color color, unsigned char blend, unsigned char antialias) {
    icylib_lazy_chunked_draw_line_with_thickness(image, x1, y1, x2, y2, 1, color, blend, antialias);
}

void icylib_lazy_chunked_draw_text(icylib_LazyChunkedImage* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, unsigned char blend) {
    icylib_draw_text((unsigned char*)image, text, x, y, color, fontPath, pixelSize, horizontalAlignment, verticalAlignment, (void (*)(unsigned char*, int, int, icylib_Color))(blend ? icylib_lazy_chunked_set_pixel_blend : icylib_lazy_chunked_set_pixel));
}

void icylib_lazy_chunked_replace_color(icylib_LazyChunkedImage* image, icylib_Color old_color, icylib_Color new_color, unsigned char blend) {
    for (int j = 0; j < (image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                icylib_regular_replace_color(image->chunk_rows[j].chunk_columns[i], old_color, new_color, blend);
            }
        }
    }
}

void icylib_lazy_chunked_replace_color_ignore_alpha(icylib_LazyChunkedImage* image, icylib_Color old_color, icylib_Color new_color) {
    for (int j = 0; j < (image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                icylib_regular_replace_color_ignore_alpha(image->chunk_rows[j].chunk_columns[i], old_color, new_color);
            }
        }
    }
}

void icylib_lazy_chunked_blur(icylib_LazyChunkedImage* image, int radius) {
    for (int j = 0; j < (image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        for (int i = 0; i < (image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            if (image->chunk_rows[j].chunk_columns[i] != NULL) {
                icylib_regular_blur(image->chunk_rows[j].chunk_columns[i], radius);
            }
        }
    }
}

icylib_RegularImage* icylib_lazy_chunked_get_sub_image(icylib_LazyChunkedImage* image, int x, int y, int width, int height) {
    unsigned int start_chunk_x = x / ICYLIB_LAZY_CHUNK_WIDTH;
    unsigned int start_chunk_y = y / ICYLIB_LAZY_CHUNK_HEIGHT;
    unsigned int end_chunk_x = (x + width - 1) / ICYLIB_LAZY_CHUNK_WIDTH;
    unsigned int end_chunk_y = (y + height - 1) / ICYLIB_LAZY_CHUNK_HEIGHT;
    icylib_RegularImage* img = icylib_regular_create_from_size(icylib_ceil(((float)(width + x % ICYLIB_LAZY_CHUNK_WIDTH)) / ICYLIB_LAZY_CHUNK_WIDTH) * ICYLIB_LAZY_CHUNK_WIDTH, icylib_ceil(((float)(height + y % ICYLIB_LAZY_CHUNK_HEIGHT)) / ICYLIB_LAZY_CHUNK_HEIGHT) * ICYLIB_LAZY_CHUNK_HEIGHT, image->channels);
    int j2 = 0;
    for (int j = start_chunk_y; j <= end_chunk_y; j++) {
        int i2 = 0;
        for (int i = start_chunk_x; i <= end_chunk_x; i++) {
            icylib_regular_draw_image(img, i2 * ICYLIB_LAZY_CHUNK_WIDTH, j2 * ICYLIB_LAZY_CHUNK_HEIGHT, image->chunk_rows[j].chunk_columns[i], 0);
            i2++;
        }
        j2++;
    }
    return icylib_regular_get_sub_image(img, x % ICYLIB_LAZY_CHUNK_WIDTH, y % ICYLIB_LAZY_CHUNK_HEIGHT, width, height);
}

icylib_LazyChunkedImage* icylib_lazy_chunked_copy(icylib_LazyChunkedImage* image) {
    icylib_LazyChunkedImage* new_image = ICYLIB_MALLOC(sizeof(icylib_LazyChunkedImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->channels = image->channels;
    new_image->chunk_rows = ICYLIB_MALLOC(sizeof(icylib_LazyChunkRow*) * (image->height / ICYLIB_LAZY_CHUNK_HEIGHT));
    new_image->rows_length = image->rows_length;
    new_image->columns_length = image->columns_length;
    for (int j = 0; j < icylib_ceil((float)image->height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        icylib_LazyChunkRow row;
        row.chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * (image->width / ICYLIB_LAZY_CHUNK_WIDTH));
        for (int i = 0; i < (image->width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            row.chunk_columns[i] = icylib_regular_copy(image->chunk_rows[j].chunk_columns[i]);
        }
        new_image->chunk_rows[j] = row;
    }
    return new_image;
}

void icylib_lazy_chunked_extend_to(icylib_LazyChunkedImage* image, int new_width, int new_height) {
    if (new_width > image->rows_length * ICYLIB_LAZY_CHUNK_WIDTH || new_height > image->columns_length * ICYLIB_LAZY_CHUNK_HEIGHT) {
        int new_rows_length = icylib_ceil((float)new_height / ICYLIB_LAZY_CHUNK_HEIGHT);
        int new_columns_length = icylib_ceil((float)new_width / ICYLIB_LAZY_CHUNK_WIDTH);
        icylib_LazyChunkRow* new_chunk_rows = ICYLIB_MALLOC(sizeof(icylib_LazyChunkRow) * new_rows_length);
        for (int j = 0; j < new_rows_length; j++) {
            icylib_LazyChunkRow row;
            icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * new_columns_length);
            for (int i = 0; i < new_columns_length; i++) {
                if (j < image->rows_length && i < image->columns_length) {
                    chunk_columns[i] = image->chunk_rows[j].chunk_columns[i];
                }
                else {
                    chunk_columns[i] = icylib_regular_create_from_size(ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT, image->channels);
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

void icylib_lazy_chunked_resize(icylib_LazyChunkedImage* image, int width, int height) {
    icylib_lazy_chunked_extend_to(image, width, height);
    // scale the invididual chunks and split them up into multiple chunks
    icylib_LazyChunkRow* new_chunk_rows = ICYLIB_MALLOC(sizeof(icylib_LazyChunkRow) * icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT));
    for (int j = 0; j < icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT); j++) {
        icylib_LazyChunkRow row;
        icylib_RegularImage** chunk_columns = ICYLIB_MALLOC(sizeof(icylib_RegularImage*) * icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH));
        for (int i = 0; i < icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH); i++) {
            icylib_RegularImage* sub = icylib_lazy_chunked_get_sub_image(image, i * ICYLIB_LAZY_CHUNK_WIDTH, j * ICYLIB_LAZY_CHUNK_HEIGHT, ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT);
            icylib_regular_resize(sub, ICYLIB_LAZY_CHUNK_WIDTH, ICYLIB_LAZY_CHUNK_HEIGHT);
            chunk_columns[i] = sub;
        }
        row.chunk_columns = chunk_columns;
        new_chunk_rows[j] = row;
    }
    ICYLIB_FREE(image->chunk_rows);
    image->chunk_rows = new_chunk_rows;
    image->rows_length = icylib_ceil((float)height / ICYLIB_LAZY_CHUNK_HEIGHT);
    image->columns_length = icylib_ceil((float)width / ICYLIB_LAZY_CHUNK_WIDTH);
    image->width = width;
    image->height = height;
}

void icylib_lazy_chunked_resize_scale(icylib_LazyChunkedImage* image, float scale) {
    icylib_lazy_chunked_resize(image, image->width * scale, image->height * scale);
}

#endif

#endif