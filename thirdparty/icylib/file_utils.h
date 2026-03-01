#ifndef ICYLIB_FILE_UTILS_H
#define ICYLIB_FILE_UTILS_H

#include <stdint.h>

char icylib_file_exists(const char* path);
size_t icylib_file_size(const char* path);
void icylib_file_read_bytes(const char* path, unsigned char** buffer);
char icylib_file_delete(const char* path);

#ifdef ICYLIB_IMPLEMENTATION

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

char icylib_file_exists(const char* path) {
#ifdef _WIN32
    return _access(path, 0) != -1;
#else
    return access(path, F_OK) != -1;
#endif
}

size_t icylib_file_size(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);
    return size;
}

void icylib_file_read_bytes(const char* path, unsigned char** buffer) {
    FILE* file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = ICYLIB_MALLOC_ATOMIC(length);
    fread(*buffer, 1, length, file);
    fclose(file);
}

char icylib_file_delete(const char* path) {
    return remove(path) == 0;
}

#endif

#endif