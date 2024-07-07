#if !defined(ICYLIB_MALLOC) || !defined(ICYLIB_REALLOC) || !defined(ICYLIB_FREE)
#include <stdlib.h>
#endif
#ifndef ICYLIB_MALLOC
#define ICYLIB_MALLOC malloc
#endif
#ifndef ICYLIB_MALLOC_ATOMIC
#define ICYLIB_MALLOC_ATOMIC ICYLIB_MALLOC
#endif
#ifndef ICYLIB_REALLOC
#define ICYLIB_REALLOC realloc
#endif
#ifndef ICYLIB_FREE
#define ICYLIB_FREE free
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC ICYLIB_MALLOC
#endif
#ifndef STBI_REALLOC
#define STBI_REALLOC ICYLIB_REALLOC
#endif
#ifndef STBI_FREE
#define STBI_FREE ICYLIB_FREE
#endif

#ifndef STBIR_MALLOC
#define STBIR_MALLOC(size, user_data) ((void)(user_data), ICYLIB_MALLOC(size))
#endif
// realloc is not used by stb_image_resize2
#ifndef STBIR_FREE
#define STBIR_FREE(ptr, user_data)    ((void)(user_data), ICYLIB_FREE(ptr))
#endif

#ifndef STBTT_malloc
#define STBTT_malloc(x, u) ICYLIB_MALLOC(x)
#endif
// realloc is not used by stb_truetype
#ifndef STBTT_free
#define STBTT_free(x, u) ICYLIB_FREE(x)
#endif

#ifndef ICYLIB_H
#define ICYLIB_H
#ifdef ICYLIB_IMPLEMENTATION
// TODO: Maybe add another include guard for the individual stb implementations?
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#include "thirdparty/stb_image.h"
#include "thirdparty/stb_image_write.h"
#include "thirdparty/stb_image_resize2.h"
#include "thirdparty/stb_truetype.h"
#endif

// undefine to prevent double includes
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STB_TRUETYPE_IMPLEMENTATION

#ifndef ICYLIB_ERROR
#include <assert.h>
#define ICYLIB_ERROR(message) { assert(!message); }
#endif