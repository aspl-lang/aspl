#include "interpreter.c"

#define APPBUNDLE_MALLOC ASPL_MALLOC
#define APPBUNDLE_REALLOC ASPL_REALLOC
#define APPBUNDLE_FREE ASPL_FREE
#include "thirdparty/appbundle/loader.c"

ASPL_AILI_ByteList* aspl_ailinterpreter_read_file_into_byte_list(char* filename) {
    FILE* file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char* buffer = ASPL_MALLOC(sizeof(char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    ASPL_AILI_ByteList* result = ASPL_MALLOC(sizeof(ASPL_AILI_ByteList));
    result->size = length;
    result->position = 0;
    result->data = buffer;
    return result;
}

int main(int argc, char** argv) {
    aspl_argc = argc;
    aspl_argv = argv;
    aspl_setup_gc();

#ifdef _WIN32
    aspl_setup_windows_console();
#endif

    aspl_setup_builtin_method_pointers();
    class_parents_map = *hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 }); // TODO: Is this required?

    ASPL_AILI_EnvironmentContext* environment_context = aspl_ailinterpreter_new_environment_context();
    ASPL_AILI_ThreadContext* thread_context = aspl_ailinterpreter_new_thread_context(environment_context);
    aspl_ailinterpreter_current_thread_context = thread_context;


#ifdef ASPL_AILI_BUNDLED
    appbundle_Bundle* bundle = appbundle_Bundle_load();
    appbundle_Resource resource = appbundle_Bundle_get_resource(bundle, "AIL Code");
    ASPL_AILI_ByteList* bytes = ASPL_MALLOC(sizeof(ASPL_AILI_ByteList));
    bytes->size = resource.length;
    bytes->position = 0;
    bytes->data = resource.data;
#else
    if (argc < 2) {
        printf("ailbyteinterpreter: no file specified\n");
        return 1;
    }
    char* file;
    file = argv[1];
    if (access(file, F_OK) == -1) {
        printf("ailbyteinterpreter: file %s not found\n", file);
        return 1;
    }
    ASPL_AILI_ByteList* bytes = aspl_ailinterpreter_read_file_into_byte_list(file);
#endif
    aspl_ailinterpreter_current_byte_list = bytes;

    aspl_ailinterpreter_loop(thread_context, bytes);
    return 0;
}