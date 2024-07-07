#include "thirdparty/appbundle/loader.h"

#include <string.h>

char appbundle_Bundle_contains_resource(appbundle_Bundle* bundle, char* name) {
    for (int i = 0; i < bundle->resources_length; i++) {
        if (strcmp(bundle->resources[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

appbundle_Resource appbundle_Bundle_get_resource(appbundle_Bundle* bundle, char* name) {
    for (int i = 0; i < bundle->resources_length; i++) {
        if (strcmp(bundle->resources[i].name, name) == 0) {
            return bundle->resources[i];
        }
    }
    return (appbundle_Resource) { 0 };
}

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <libproc.h>
#else
#include <unistd.h>
#endif

char* appbundle_util_get_name_of_executable() {
#ifdef _WIN32
    char* name = APPBUNDLE_MALLOC(1024);
    GetModuleFileName(NULL, name, 1024);
    return name;
#elif __APPLE__
    char* name = APPBUNDLE_MALLOC(1024);
    pid_t pid = getpid();
    int ret = proc_pidpath(pid, name, 1024);
    if (ret <= 0) {
        APPBUNDLE_ERROR("proc_pidpath failed with pid %d: %d\n", pid, ret);
    }
    return name;
#else
    char* name = APPBUNDLE_MALLOC(1024);
    ssize_t count = readlink("/proc/self/exe", name, 1024);
    if (count == -1) {
        APPBUNDLE_ERROR("readlink failed with: %s", strerror(errno));
    }
    name[count] = '\0';
    return name;
#endif
}

appbundle_Bundle* appbundle_Bundle_load() {
    appbundle_Bundle* bundle = APPBUNDLE_MALLOC(sizeof(appbundle_Bundle));
    bundle->resources_length = 0;
    bundle->resources = APPBUNDLE_MALLOC(sizeof(appbundle_Resource) * bundle->resources_length);

    FILE* file = fopen(appbundle_util_get_name_of_executable(), "rb");
    fseek(file, -8, SEEK_END);
    unsigned long long bytes_length = 0;
    fread(&bytes_length, 8, 1, file);
    fseek(file, 0, SEEK_END);
    unsigned long long start_position = ftell(file) - bytes_length - 8; // TODO: Why is this -8 required?
    fseek(file, start_position, SEEK_SET);
    unsigned char* bytes = APPBUNDLE_MALLOC(bytes_length);
    fread(bytes, 1, bytes_length, file);
    fclose(file);

    unsigned long long position = 0;
    while (position < bytes_length) {
        unsigned char resource_type = bytes[position++]; // 0 = binary, 2 = text; ignored in this implementation
        unsigned long long resource_name_length;
        memcpy(&resource_name_length, bytes + position, sizeof(unsigned long long));
        position += 8;
        char* resource_name = APPBUNDLE_MALLOC(resource_name_length + 1);
        for (int i = 0; i < resource_name_length; i++) {
            resource_name[i] = bytes[position + i];
        }
        resource_name[resource_name_length] = '\0';
        position += resource_name_length;
        unsigned long long resource_data_length;
        memcpy(&resource_data_length, bytes + position, sizeof(unsigned long long));
        position += 8;
        unsigned char* resource_data = APPBUNDLE_MALLOC(resource_data_length);
        for (int i = 0; i < resource_data_length; i++) {
            resource_data[i] = bytes[position + i];
        }
        position += resource_data_length;
        bundle->resources_length++;
        bundle->resources = APPBUNDLE_REALLOC(bundle->resources, sizeof(appbundle_Resource) * bundle->resources_length);
        bundle->resources[bundle->resources_length - 1].name = resource_name;
        bundle->resources[bundle->resources_length - 1].data = resource_data;
        bundle->resources[bundle->resources_length - 1].length = resource_data_length;
    }

    return bundle;
}