#ifndef ICYLIB_TEXT_H
#define ICYLIB_TEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thirdparty/stb_truetype.h"

#include "icylib.h"

typedef struct icylib_FontCacheEntry {
    const char* name;
    stbtt_fontinfo* font;
} icylib_FontCacheEntry;

typedef struct icylib_FontCache {
    icylib_FontCacheEntry* data;
    int length;
} icylib_FontCache;

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

icylib_FontCache* icylib_get_font_cache();

void icylib_set_font_cache(icylib_FontCache* fonts);

char* icylib_get_default_font_path();

char* icylib_get_font_variant_path(const char* font_path, const char* variant);

void icylib_read_bytes(const char* path, unsigned char** font_file);

void icylib_measure_text_size(const char* text, const char* font_path, int pixel_size, double* result_x, double* result_y);

#ifdef ICYLIB_IMPLEMENTATION

icylib_FontCache* icylib_get_font_cache() {
    static icylib_FontCache* fonts = NULL;
    if (!fonts) {
        fonts = ICYLIB_MALLOC(sizeof(icylib_FontCache));
        fonts->data = ICYLIB_MALLOC(1);
        fonts->length = 0;
    }
    return fonts;
}

void icylib_set_font_cache(icylib_FontCache* fonts) {
    icylib_FontCache* old_fonts = icylib_get_font_cache();
    if (old_fonts->data) {
        ICYLIB_FREE(old_fonts->data);
    }
    old_fonts->data = fonts->data;
}

char* icylib_get_default_font_path() {
    // TODO: Improve this
#ifdef _WIN32
    return "C:/Windows/Fonts/arial.ttf";
#elif __APPLE__
    return "/Library/Fonts/Arial.ttf";
#elif __ANDROID__
    // taken from the V standard library
    char* xml_files[] = { "/system/etc/system_fonts.xml", "/system/etc/fonts.xml", "/etc/system_fonts.xml", "/etc/fonts.xml", "/data/fonts/fonts.xml", "/etc/fallback_fonts.xml" };
    char* font_locations[] = { "/system/fonts", "/data/fonts" };
    for (int i = 0; i < 6; ++i) {
        FILE* file = fopen(xml_files[i], "r");
        if (file) {
            char* xml = NULL;
            size_t n = 0;
            ssize_t read;
            while ((read = getline(&xml, &n, file)) != -1) {
                char* candidate_font = NULL;
                if (strstr(xml, "<font")) {
                    candidate_font = strstr(xml, ">") + 1;
                    candidate_font = strtok(candidate_font, "<");
                    while (*candidate_font == ' ' || *candidate_font == '\t' || *candidate_font == '\n' || *candidate_font == '\r') {
                        candidate_font++;
                    }
                    char* end = candidate_font + strlen(candidate_font) - 1;
                    while (end > candidate_font && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
                        *end = '\0';
                        end--;
                    }
                    if (strstr(candidate_font, ".ttf")) {
                        for (int j = 0; j < 2; ++j) {
                            char* candidate_path = ICYLIB_MALLOC_ATOMIC(strlen(font_locations[j]) + strlen(candidate_font) + 2);
                            strcpy(candidate_path, font_locations[j]);
                            strcat(candidate_path, "/");
                            strcat(candidate_path, candidate_font);
                            if (access(candidate_path, F_OK) == 0) {
                                return candidate_path;
                            }
                            ICYLIB_FREE(candidate_path);
                        }
                    }
                }
            }
            fclose(file);
        }
    }
    ICYLIB_ERROR("No default font found");
#else
    return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
}

// This is a modified version of os.font.get_path_variant() of the V standard library
char* icylib_get_font_variant_path(const char* font_path, const char* variant) {
    // Find some way to make this shorter and more eye-pleasant
    // NotoSans, LiberationSans, DejaVuSans, Arial, and SFNS should work
    char* file = strrchr(font_path, '/');
    char* fpath = ICYLIB_MALLOC_ATOMIC(file - font_path + 2);
    strncpy(fpath, font_path, file - font_path + 1);
    fpath[file - font_path + 1] = '\0';
    file++;
    file = strrchr(font_path, '/');
    file = file ? file + 1 : (char*)font_path;

    char* file_copy = ICYLIB_MALLOC_ATOMIC(strlen(file) + 1);
    strcpy(file_copy, file);
    char* dot_position = strrchr(file_copy, '.');
    if (dot_position != NULL) {
        *dot_position = '\0';
    }

    if (strcmp(variant, "normal") == 0) {
        // No changes for normal variant
    }
    else if (strcmp(variant, "bold") == 0) {
        if (strstr(fpath, "-Regular") != NULL || strstr(file_copy, "-Regular") != NULL) {
            strcpy(file_copy, strstr(file_copy, "-Regular") ? strstr(file_copy, "-Regular") + 1 : strstr(fpath, "-Regular") + 1);
            strcat(file_copy, "-Bold");
        }
        else if (strncmp(file_copy, "DejaVuSans", 10) == 0 || strncmp(file_copy, "DroidSans", 9) == 0) {
            strcat(file_copy, "-Bold");
        }
        else if (strncasecmp(file_copy, "arial", 5) == 0) {
            strcat(file_copy, "bd");
        }
        else {
            strcat(file_copy, "-bold");
        }
#ifdef __APPLE__
        if (access("SFNS-bold", F_OK) == 0) {
            strcpy(file_copy, "SFNS-bold");
        }
#endif
    }
    else if (strcmp(variant, "italic") == 0) {
        if (strstr(file_copy, "-Regular") != NULL) {
            strcpy(file_copy, strstr(file_copy, "-Regular") + 1);
            strcat(file_copy, "-Italic");
        }
        else if (strncmp(file_copy, "DejaVuSans", 10) == 0) {
            strcat(file_copy, "-Oblique");
        }
        else if (strncasecmp(file_copy, "arial", 5) == 0) {
            strcat(file_copy, "i");
        }
        else {
            strcat(file_copy, "Italic");
        }
    }
    else if (strcmp(variant, "mono") == 0) {
        if (strcmp(file_copy, "Mono-Regular") != 0 && strstr(file_copy, "-Regular") != NULL) {
            strcpy(file_copy, strstr(file_copy, "-Regular") + 1);
            strcat(file_copy, "Mono-Regular");
        }
        else if (strncasecmp(file_copy, "arial", 5) != 0) {
            strcat(file_copy, "Mono");
        }
    }

    char* result = ICYLIB_MALLOC_ATOMIC(strlen(fpath) + strlen(file_copy) + 5);  // 5 for ".ttf" and null terminator
    strcpy(result, fpath);
    strcat(result, file_copy);
    strcat(result, ".ttf");

    ICYLIB_FREE(fpath);
    ICYLIB_FREE(file_copy);

    return result;
}

void icylib_read_bytes(const char* path, unsigned char** font_file) {
    FILE* file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    *font_file = ICYLIB_MALLOC_ATOMIC(length);
    fread(*font_file, 1, length, file);
    fclose(file);
}

void icylib_measure_text_size(const char* text, const char* font_path, int pixel_size, double* result_x, double* result_y) {
    stbtt_fontinfo* font = NULL;
    icylib_FontCache* fonts = icylib_get_font_cache();

    for (int i = 0; i < fonts->length; ++i) {
        if (strcmp(fonts->data[i].name, font_path) == 0) {
            font = fonts->data[i].font;
            break;
        }
    }
    if (font == NULL) {
        unsigned char* font_file;
        icylib_read_bytes(font_path, &font_file);

        font = ICYLIB_MALLOC(sizeof(stbtt_fontinfo));

        int offset = stbtt_GetFontOffsetForIndex(font_file, 0);
        stbtt_InitFont(font, font_file, offset);

        fonts->data = ICYLIB_REALLOC(fonts->data, sizeof(icylib_FontCacheEntry) * (fonts->length + 1));
        fonts->data[fonts->length].name = font_path;
        fonts->data[fonts->length].font = font;
        fonts->length++;
    }

    float scale = stbtt_ScaleForPixelHeight(font, pixel_size);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);

    int advance, lsb, x0, y0, x1, y1;
    double x_cursor = 0.0;
    double max_x = 0.0;
    double y_cursor = ascent * scale;

    size_t text_len = strlen(text);
    for (size_t i = 0; i < text_len; ++i) {
        if (text[i] == '\n') {
            if (x_cursor > max_x) {
                max_x = x_cursor;
            }
            y_cursor += (ascent - descent + lineGap) * scale;
            x_cursor = 0;
            continue;
        }

        int c = (int)text[i];

        stbtt_GetCodepointHMetrics(font, c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(font, c, scale, scale, &x0, &y0, &x1, &y1);

        if (i < text_len - 1) {
            x_cursor += advance * scale;
            x_cursor += stbtt_GetCodepointKernAdvance(font, c, (int)text[i + 1]) * scale;
        }else{
            x_cursor += x1;
        }
    }

    if (x_cursor > max_x) {
        max_x = x_cursor;
    }

    *result_x = max_x;
    *result_y = y_cursor;
}

void icylib_draw_text(unsigned char* image, char* text, int x, int y, icylib_Color color, char* fontPath, int pixelSize, icylib_HorizontalAlignment horizontalAlignment, icylib_VerticalAlignment verticalAlignment, void (*set_pixel)(unsigned char* image, int, int, icylib_Color)) {
    double width, height;
    icylib_measure_text_size(text, fontPath, pixelSize, &width, &height);

    stbtt_fontinfo* font = NULL;
    icylib_FontCache* fonts = icylib_get_font_cache();

    for (int i = 0; i < fonts->length; ++i) {
        if (strcmp(fonts->data[i].name, fontPath) == 0) {
            font = fonts->data[i].font;
            break;
        }
    }
    if (font == NULL) {
        unsigned char* font_file;
        icylib_read_bytes(fontPath, &font_file);

        font = ICYLIB_MALLOC(sizeof(stbtt_fontinfo));

        int offset = stbtt_GetFontOffsetForIndex(font_file, 0);
        stbtt_InitFont(font, font_file, offset);

        fonts->data = ICYLIB_REALLOC(fonts->data, sizeof(icylib_FontCacheEntry) * (fonts->length + 1));
        fonts->data[fonts->length].name = fontPath;
        fonts->data[fonts->length].font = font;
        fonts->length++;
    }

    float scale = stbtt_ScaleForPixelHeight(font, pixelSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);

    int advance, lsb, x0, y0, x1, y1;

    float x_cursor = x;
    if (horizontalAlignment == ICYLIB_HORIZONTAL_ALIGNMENT_CENTER) {
        x_cursor -= width / 2;
    }
    else if (horizontalAlignment == ICYLIB_HORIZONTAL_ALIGNMENT_RIGHT) {
        x_cursor -= width;
    }
    float y_cursor = y + ascent * scale;
    if (verticalAlignment == ICYLIB_VERTICAL_ALIGNMENT_CENTER) {
        y_cursor -= height / 2;
    }
    else if (verticalAlignment == ICYLIB_VERTICAL_ALIGNMENT_BOTTOM) {
        y_cursor -= height;
    }
    for (size_t i = 0; i < strlen(text); ++i) {
        if (text[i] == '\n') {
            y_cursor += (ascent - descent + lineGap) * scale;
            x_cursor = x;
            continue;
        }

        int c = (int)text[i];

        stbtt_GetCodepointHMetrics(font, c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(font, c, scale, scale, &x0, &y0, &x1, &y1);

        int w = 0;
        int h = 0;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(font, 0, scale, c, &w, &h, 0, 0);
        for (int b = 0; b < h; ++b) {
            for (int a = 0; a < w; ++a) {
                if (bitmap[b * w + a] != 0) {
                    int pixel_x = (int)x_cursor + x0 + a;
                    int pixel_y = (int)y_cursor + y0 + b;
                    set_pixel(image, pixel_x, pixel_y, icylib_color_from_rgba(color.r, color.g, color.b, (unsigned char)(bitmap[b * w + a])));
                }
            }
        }

        if (i < strlen(text) - 1) {
            x_cursor += advance * scale;
            x_cursor += stbtt_GetCodepointKernAdvance(font, c, (int)text[i + 1]) * scale;
        }
    }
}

#endif

#endif