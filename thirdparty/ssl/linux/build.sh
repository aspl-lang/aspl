#!/bin/bash
# TODO: Find a more robust way to prevent sokol from including the OpenGL headers
SOKOL_PATH="../../../thirdparty/sokol/"
if ! grep -q '//#include <GL/gl.h>' "${SOKOL_PATH}sokol_app.h"; then
    sed -i 's|#include <GL/gl.h>|//&|' "${SOKOL_PATH}sokol_app.h"
    sed -i 's|typedef struct __GLXFBConfig\* GLXFBConfig;|/*&|' "${SOKOL_PATH}sokol_app.h"
    sed -i 's|typedef GLXContext (\*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display\*,GLXFBConfig,GLXContext,Bool,const int\*);|&*/|' "${SOKOL_PATH}sokol_app.h"
fi
if ! grep -q '//#include <GL/gl.h>' "${SOKOL_PATH}sokol_gfx.h"; then
    sed -i 's|#include <GL/gl.h>|//&|' "${SOKOL_PATH}sokol_gfx.h"
fi
zig cc -o bin/ssl.o -c ssl.c -DSOKOL_NO_ENTRY -O3
zig ar rcs bin/libssl.a bin/ssl.o
rm bin/ssl.o