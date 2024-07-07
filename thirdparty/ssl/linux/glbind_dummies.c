#define GLBIND_IMPLEMENTATION
#include "../glbind.h"

#include <stdio.h>
#include <stdlib.h>

#define SSL_ERROR_CB(...) { printf("SSL Error: "); printf(__VA_ARGS__); printf("\n"); exit(1); }

void ssl_init_glbind(){
    GLenum result = glbInit(NULL, NULL);
    if (result != GL_NO_ERROR) {
        SSL_ERROR_CB("Failed to initialize glbind: %d\n", result);
    }
}