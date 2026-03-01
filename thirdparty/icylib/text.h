#ifndef ICYLIB_TEXT_H
#define ICYLIB_TEXT_H

#include <stdint.h>

#include "thirdparty/stb_truetype.h"

#include "color.h"

typedef struct icylib_Font {
    stbtt_fontinfo* handle;
} icylib_Font;

typedef enum icylib_HorizontalAlignment {
    ICYLIB_HORIZONTAL_ALIGNMENT_LEFT,
    ICYLIB_HORIZONTAL_ALIGNMENT_CENTER,
    ICYLIB_HORIZONTAL_ALIGNMENT_RIGHT
} icylib_HorizontalAlignment;

typedef enum icylib_VerticalAlignment {
    ICYLIB_VERTICAL_ALIGNMENT_TOP,
    ICYLIB_VERTICAL_ALIGNMENT_CENTER,
    ICYLIB_VERTICAL_ALIGNMENT_BOTTOM
} icylib_VerticalAlignment;

typedef enum icylib_HorizontalTextFitting {
    ICYLIB_HORIZONTAL_TEXT_FITTING_FULL,
    ICYLIB_HORIZONTAL_TEXT_FITTING_TIGHT
} icylib_HorizontalTextFitting;

typedef enum icylib_VerticalTextFitting {
    ICYLIB_VERTICAL_TEXT_FITTING_FULL,
    ICYLIB_VERTICAL_TEXT_FITTING_TIGHT
} icylib_VerticalTextFitting;

uint32_t* icylib_utf8_decode(const char* utf8_str, size_t* out_len);

const char* icylib_get_default_font_path();

char* icylib_find_font_variant_path(const char* font_path, const char* variant);

icylib_Font icylib_create_font_from_memory(unsigned char* data);

icylib_Font icylib_load_font_from_file(const char* path);

void icylib_destroy_font(icylib_Font font);

void icylib_measure_text_size(const char* text, icylib_Font font, int pixel_size, double* result_x, double* result_y, icylib_HorizontalTextFitting horizontal_fitting, icylib_VerticalTextFitting vertical_fitting, double* offset_x, double* offset_y);

void icylib_draw_text(unsigned char* image, char* text, int x, int y, icylib_Color color, icylib_Font font, int pixel_size, icylib_HorizontalAlignment horizontal_alignment, icylib_VerticalAlignment vertical_alignment, icylib_HorizontalTextFitting horizontal_fitting, icylib_VerticalTextFitting vertical_fitting, void (*set_pixel)(unsigned char* image, int, int, icylib_Color));

#ifdef ICYLIB_IMPLEMENTATION

#include <string.h>
#include <ctype.h>

#include "icylib.h"
#include "file_utils.h"
#include "math_utils.h"

uint32_t* icylib_utf8_decode(const char* utf8_str, size_t* out_length) {
    size_t length = 0;
    size_t capacity = 8; // estimated average number of codepoints per string
    uint32_t* codepoints = ICYLIB_MALLOC(capacity * sizeof(uint32_t));

    for (size_t i = 0; utf8_str[i] != '\0';) {
        uint32_t codepoint;
        if ((utf8_str[i] & 0x80) == 0) {
            codepoint = utf8_str[i];
            i += 1;
        }
        else if ((utf8_str[i] & 0xE0) == 0xC0) {
            codepoint = ((utf8_str[i] & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
            i += 2;
        }
        else if ((utf8_str[i] & 0xF0) == 0xE0) {
            codepoint = ((utf8_str[i] & 0x0F) << 12) | ((utf8_str[i + 1] & 0x3F) << 6) | (utf8_str[i + 2] & 0x3F);
            i += 3;
        }
        else if ((utf8_str[i] & 0xF8) == 0xF0) {
            codepoint = ((utf8_str[i] & 0x07) << 18) | ((utf8_str[i + 1] & 0x3F) << 12) | ((utf8_str[i + 2] & 0x3F) << 6) | (utf8_str[i + 3] & 0x3F);
            i += 4;
        }
        else {
            // invalid UTF-8, just ignore
            i++;
            continue;
        }
        if (i >= capacity) {
            capacity *= 2;
            codepoints = ICYLIB_REALLOC(codepoints, capacity * sizeof(uint32_t));
        }
        codepoints[length++] = codepoint;
    }

    *out_length = length;
    return codepoints;
}

const char* icylib_get_default_font_path() {
#ifdef _WIN32
    return "C:/Windows/Fonts/arial.ttf";
#elif __APPLE__
    return "/System/Library/Fonts/SFNS.ttf";
#elif __ANDROID__
    return "/system/fonts/Roboto-Regular.ttf";
#else
    return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
}

char* icylib_build_font_variant_path(const char* font_path, const char* variant) {
    if (!font_path || !variant) return NULL;

    const char* dash = strrchr(font_path, '-');
    if (!dash) dash = strrchr(font_path, '.');
    if (!dash) return NULL;

    size_t base_len = dash - font_path;
    size_t variant_len = strlen(variant);
    size_t total_len = base_len + variant_len + strlen(".ttf");

    char* variant_path = ICYLIB_MALLOC_ATOMIC(total_len + 1);
    if (!variant_path) return NULL;

    memcpy(variant_path, font_path, base_len);
    strcpy(variant_path + base_len, variant);
    strcpy(variant_path + base_len + variant_len, ".ttf");

    return variant_path;
}

char* icylib_try_font_variant_path(const char* font_path, const char* variant) {
    char* path = icylib_build_font_variant_path(font_path, variant);
    if (path) {
        if (icylib_file_exists(path)) {
            return path;
        }
        else {
            ICYLIB_FREE(path);
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

char* icylib_find_font_variant_path(const char* font_path, const char* variant) {
    char* path = NULL;
    if (strcmp(variant, "regular") == 0) {
        if ((path = icylib_try_font_variant_path(font_path, "-Regular")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "Regular")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "")) != NULL) return path;
    }
    else if (strcmp(variant, "bold") == 0) {
        if ((path = icylib_try_font_variant_path(font_path, "-Bold")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "Bold")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "bd")) != NULL) return path;
    }
    else if (strcmp(variant, "italic") == 0) {
        if ((path = icylib_try_font_variant_path(font_path, "-Italic")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "Italic")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "i")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "-Oblique")) != NULL) return path;
    }
    else if (strcmp(variant, "mono") == 0) {
        if ((path = icylib_try_font_variant_path(font_path, "-Mono")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "Mono")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "-MonoRegular")) != NULL) return path;
        if ((path = icylib_try_font_variant_path(font_path, "MonoRegular")) != NULL) return path;
    }
    else {
        if ((path = icylib_try_font_variant_path(font_path, variant)) != NULL) return path;
        char* variant_with_dash = ICYLIB_MALLOC(1 + strlen(variant) + 1);
        if (!variant_with_dash) return NULL;
        strcpy(variant_with_dash, "-");
        strcat(variant_with_dash, variant);
        if ((path = icylib_try_font_variant_path(font_path, variant_with_dash)) != NULL) {
            ICYLIB_FREE(variant_with_dash);
            return path;
        }
        if (variant[0] != '\0') {
            variant_with_dash[1] = toupper(variant_with_dash[1]);
            if ((path = icylib_try_font_variant_path(font_path, variant_with_dash)) != NULL) {
                ICYLIB_FREE(variant_with_dash);
                return path;
            }
        }
        ICYLIB_FREE(variant_with_dash);
        return NULL;
    }
}

icylib_Font icylib_create_font_from_memory(unsigned char* data) {
    stbtt_fontinfo* handle = ICYLIB_MALLOC(sizeof(stbtt_fontinfo));
    int offset = stbtt_GetFontOffsetForIndex(data, 0);
    stbtt_InitFont(handle, data, offset);
    return (icylib_Font) { .handle = handle };
}

icylib_Font icylib_load_font_from_file(const char* path) {
    unsigned char* data;
    icylib_file_read_bytes(path, &data);
    return icylib_create_font_from_memory(data);
}

void icylib_destroy_font(icylib_Font font) {
    ICYLIB_FREE(font.handle);
}

void icylib_measure_text_size(const char* text, icylib_Font font, int pixel_size, double* result_x, double* result_y, icylib_HorizontalTextFitting horizontal_fitting, icylib_VerticalTextFitting vertical_fitting, double* offset_x, double* offset_y) {
    float scale = stbtt_ScaleForPixelHeight(font.handle, pixel_size);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font.handle, &ascent, &descent, &lineGap);

    int advance, lsb, x0, y0, x1, y1;
    double x_cursor = 0.0;
    double min_x = 1 << 16; // ~ infinity
    double max_x_full = -(1 << 16);
    double max_x_tight = -(1 << 16);
    double y_cursor = ascent * scale;
    double min_y = 1 << 16;
    double max_y = -(1 << 16);

    size_t codepoints_length;
    uint32_t* codepoints = icylib_utf8_decode(text, &codepoints_length);

    for (size_t i = 0; i < codepoints_length; ++i) {
        if (codepoints[i] == '\n') {
            if (x_cursor > max_x_full) {
                max_x_full = x_cursor;
            }
            y_cursor += (ascent - descent + lineGap) * scale;
            x_cursor = 0;
            continue;
        }

        uint32_t codepoint = codepoints[i];

        stbtt_GetCodepointHMetrics(font.handle, codepoint, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(font.handle, codepoint, scale, scale, &x0, &y0, &x1, &y1);

        if (x_cursor + x0 < min_x) {
            min_x = x_cursor + x0;
        }
        if (x_cursor + x1 > max_x_tight) {
            max_x_tight = x_cursor + x1;
        }
        if (y_cursor - ascent * scale + y0 < min_y) {
            min_y = y_cursor - ascent * scale + y0;
        }
        if (y_cursor - ascent * scale + y1 > max_y) {
            max_y = y_cursor - ascent * scale + y1;
        }

        if (i < codepoints_length - 1) {
            x_cursor += advance * scale;
            x_cursor += stbtt_GetCodepointKernAdvance(font.handle, codepoint, codepoints[i + 1]) * scale;
        }
        else {
            x_cursor += x1;
        }
    }

    ICYLIB_FREE(codepoints);

    if (x_cursor > max_x_full) {
        max_x_full = x_cursor;
    }

    if (horizontal_fitting == ICYLIB_HORIZONTAL_TEXT_FITTING_TIGHT) {
        *result_x = max_x_tight - min_x;
    }
    else {
        *result_x = max_x_full;
    }
    if (vertical_fitting == ICYLIB_VERTICAL_TEXT_FITTING_TIGHT) {
        *result_y = max_y - min_y;
    }
    else {
        *result_y = y_cursor - descent * scale;
    }
    if (offset_x != 0) {
        *offset_x = min_x;
    }
    if (offset_y != 0) {
        *offset_y = min_y + ascent * scale;
    }
}

void icylib_draw_text(unsigned char* image, char* text, int x, int y, icylib_Color color, icylib_Font font, int pixel_size, icylib_HorizontalAlignment horizontal_alignment, icylib_VerticalAlignment vertical_alignment, icylib_HorizontalTextFitting horizontal_fitting, icylib_VerticalTextFitting vertical_fitting, void (*set_pixel)(unsigned char* image, int, int, icylib_Color)) {
    double width, height;
    double offset_x, offset_y;
    icylib_measure_text_size(text, font, pixel_size, &width, &height, horizontal_fitting, vertical_fitting, &offset_x, &offset_y);
    if (horizontal_fitting == ICYLIB_HORIZONTAL_TEXT_FITTING_FULL) {
        offset_x = 0;
    }
    if (vertical_fitting == ICYLIB_VERTICAL_TEXT_FITTING_FULL) {
        offset_y = 0;
    }

    float scale = stbtt_ScaleForPixelHeight(font.handle, pixel_size);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font.handle, &ascent, &descent, &lineGap);

    int advance, lsb, x0, y0, x1, y1;

    float x_cursor = x - offset_x;
    if (horizontal_alignment == ICYLIB_HORIZONTAL_ALIGNMENT_CENTER) {
        x_cursor -= width / 2;
    }
    else if (horizontal_alignment == ICYLIB_HORIZONTAL_ALIGNMENT_RIGHT) {
        x_cursor -= width;
    }
    float y_cursor = y + ascent * scale - offset_y;
    if (vertical_alignment == ICYLIB_VERTICAL_ALIGNMENT_CENTER) {
        y_cursor -= height / 2;
    }
    else if (vertical_alignment == ICYLIB_VERTICAL_ALIGNMENT_BOTTOM) {
        y_cursor -= height;
    }

    size_t codepoints_length;
    uint32_t* codepoints = icylib_utf8_decode(text, &codepoints_length);

    for (size_t i = 0; i < codepoints_length; ++i) {
        if (codepoints[i] == '\n') {
            y_cursor += (ascent - descent + lineGap) * scale;
            x_cursor = x;
            continue;
        }

        uint32_t codepoint = codepoints[i];

        stbtt_GetCodepointHMetrics(font.handle, codepoint, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(font.handle, codepoint, scale, scale, &x0, &y0, &x1, &y1);

        int w = 0;
        int h = 0;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(font.handle, 0, scale, codepoint, &w, &h, 0, 0);
        for (int b = 0; b < h; ++b) {
            for (int a = 0; a < w; ++a) {
                if (bitmap[b * w + a] != 0) {
                    int pixel_x = icylib_round(x_cursor) + x0 + a;
                    int pixel_y = icylib_round(y_cursor) + y0 + b;
                    set_pixel(image, pixel_x, pixel_y, icylib_color_from_rgba(color.r, color.g, color.b, (unsigned char)(bitmap[b * w + a])));
                }
            }
        }

        if (i < codepoints_length - 1) {
            x_cursor += advance * scale;
            x_cursor += stbtt_GetCodepointKernAdvance(font.handle, codepoint, codepoints[i + 1]) * scale;
        }
    }

    ICYLIB_FREE(codepoints);
}

#endif

#endif