#ifndef APPBUNDLE_MALLOC
#define APPBUNDLE_MALLOC malloc
#define STDLIB_INCLUDED
#endif
#ifndef APPBUNDLE_REALLOC
#define APPBUNDLE_REALLOC realloc
#define STDLIB_INCLUDED
#endif
#ifndef APPBUNDLE_FREE
#define APPBUNDLE_FREE free
#define STDLIB_INCLUDED
#endif
#ifdef STDLIB_INCLUDED
#include <stdlib.h>
#endif
#ifndef APPBUNDLE_ERROR
#include <stdio.h>
#define APPBUNDLE_ERROR(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#endif

typedef struct appbundle_Resource {
    char* name;
    void* data;
    unsigned int length;
} appbundle_Resource;

typedef struct appbundle_Bundle {
    appbundle_Resource* resources;
    unsigned int resources_length;
} appbundle_Bundle;

char appbundle_Bundle_contains_resource(appbundle_Bundle* bundle, char* name);
appbundle_Resource appbundle_Bundle_get_resource(appbundle_Bundle* bundle, char* name);
appbundle_Bundle* appbundle_Bundle_load();