// TODO: Clean up & refactor this whole file sometime in the (near) future

#ifdef __linux__
#define _GNU_SOURCE // TODO: I think this shouldn't be necessary
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef __linux__
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>
#endif

#ifndef ASPL_NO_MULTITHREADING
#define THREAD_IMPLEMENTATION
#define thread_id_t libthread_id_t
#ifdef __APPLE__
#define thread_create libthread_create // thread_create is already used by macOS
#endif
#include "thirdparty/thread.h/thread.h"
#ifdef __APPLE__
#undef thread_create
#endif
#undef thread_id_t
#undef thread_h
#define GC_THREADS
#endif

#ifdef _WIN32
#define GC_NOT_DLL = 1
#else
#define GC_NO_DLOPEN
#endif
#define GC_BUILTIN_ATOMIC = 1

#ifdef ASPL_DEBUG
#define GC_DEBUG
#endif

#include "thirdparty/libgc/gc.c"
#undef PROC

#ifdef __APPLE__
#define thread_create libthread_create
#endif

#define ASPL_MALLOC GC_MALLOC
#define ASPL_MALLOC_ATOMIC GC_MALLOC_ATOMIC
#define ASPL_REALLOC GC_REALLOC
#define ASPL_FREE GC_FREE

#ifdef ASPL_HEAP_BASED
#define ASPL_OBJECT_TYPE ASPL_Object_internal*
#define ASPL_ALLOC_OBJECT() ASPL_MALLOC(sizeof(ASPL_Object_internal))
#define ASPL_ACCESS(obj) (*obj)
#define ASPL_UNINITIALIZED NULL
#define ASPL_IS_UNINITIALIZED(obj) (obj == NULL)
#define ASPL_HASHMAP_WRAP(obj) (obj)
#define ASPL_HASHMAP_UNWRAP(obj) (obj)
#else
#define ASPL_OBJECT_TYPE ASPL_Object_internal
#define ASPL_ALLOC_OBJECT() (ASPL_OBJECT_TYPE){}
#define ASPL_ACCESS(obj) (obj)
#define ASPL_UNINITIALIZED (ASPL_OBJECT_TYPE){.kind = ASPL_OBJECT_KIND_UNINITIALIZED}
#define ASPL_IS_UNINITIALIZED(obj) (obj.kind == ASPL_OBJECT_KIND_UNINITIALIZED)
#define ASPL_HASHMAP_WRAP(obj) C_REFERENCE(obj)
#define ASPL_HASHMAP_UNWRAP(obj) (*obj)
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define aspl_util_print(...)  __android_log_print(ANDROID_LOG_INFO, ASPL_APP_NAME_STRING, __VA_ARGS__)
#define aspl_util_vprint(...)  __android_log_print(ANDROID_LOG_INFO, ASPL_APP_NAME_STRING, __VA_ARGS__)
#else
#define aspl_util_print(...) printf(__VA_ARGS__);
#define aspl_util_vprint(...) vprintf(__VA_ARGS__);
#endif

typedef enum ASPL_ObjectKind
{
    ASPL_OBJECT_KIND_UNINITIALIZED,
    ASPL_OBJECT_KIND_NULL,
    ASPL_OBJECT_KIND_BOOLEAN,
    ASPL_OBJECT_KIND_BYTE,
    ASPL_OBJECT_KIND_INTEGER,
    ASPL_OBJECT_KIND_LONG,
    ASPL_OBJECT_KIND_FLOAT,
    ASPL_OBJECT_KIND_DOUBLE,
    ASPL_OBJECT_KIND_STRING,
    ASPL_OBJECT_KIND_LIST,
    ASPL_OBJECT_KIND_MAP,
    ASPL_OBJECT_KIND_CALLBACK,
    ASPL_OBJECT_KIND_POINTER,
    ASPL_OBJECT_KIND_CLASS_INSTANCE,
    ASPL_OBJECT_KIND_ENUM_FIELD,
    ASPL_OBJECT_KIND_HANDLE
} ASPL_ObjectKind;

typedef struct ASPL_Object_internal
{
    ASPL_ObjectKind kind;
    union
    {
        char boolean;
        unsigned char integer8;
        long integer32;
        long long integer64;
        float float32;
        double float64;
        struct ASPL_String* string;
        struct ASPL_List* list;
        struct ASPL_Map* map;
        struct ASPL_Pointer* pointer;
        struct ASPL_Callback* callback;
        struct ASPL_ClassInstance* classInstance;
        struct ASPL_EnumField* enumField;
        void* handle;
    } value;
} ASPL_Object_internal;

typedef struct ASPL_String
{
    char* str;
    size_t length;
} ASPL_String;

typedef struct ASPL_List
{
    char* typePtr;
    int typeLen;
    ASPL_OBJECT_TYPE* value;
    size_t length;
    size_t capacity;
} ASPL_List;

ASPL_OBJECT_TYPE* C_REFERENCE(ASPL_OBJECT_TYPE x)
{
    ASPL_OBJECT_TYPE* ptr = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
    *ptr = x;
    return ptr;
}

#define HASHMAP_PREFIX hashmap_aspl_object_to_aspl_object
#define HASHMAP_KEY_TYPE ASPL_Object_internal *
#define HASHMAP_KEY_NULL_VALUE NULL
#define HASHMAP_VALUE_TYPE ASPL_Object_internal *
#define HASHMAP_VALUE_NULL_VALUE NULL

unsigned int aspl_hash(ASPL_OBJECT_TYPE obj);

unsigned int hashmap_aspl_object_to_aspl_object_hash_key(HASHMAP_KEY_TYPE key)
{
    return aspl_hash(ASPL_HASHMAP_UNWRAP(key));
}

int ASPL_IS_EQUAL(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b);

int hashmap_aspl_object_to_aspl_object_equals_key(HASHMAP_KEY_TYPE a, HASHMAP_KEY_TYPE b)
{
    return ASPL_IS_EQUAL(ASPL_HASHMAP_UNWRAP(a), ASPL_HASHMAP_UNWRAP(b));
}

unsigned int hashmap_aspl_object_to_aspl_object_hash_value(HASHMAP_VALUE_TYPE value)
{
    return aspl_hash(ASPL_HASHMAP_UNWRAP(value));
}

int hashmap_aspl_object_to_aspl_object_equals_value(HASHMAP_VALUE_TYPE a, HASHMAP_VALUE_TYPE b)
{
    return ASPL_IS_EQUAL(ASPL_HASHMAP_UNWRAP(a), ASPL_HASHMAP_UNWRAP(b));
}

#define HASHMAP_MALLOC ASPL_MALLOC
#define HASHMAP_REALLOC ASPL_REALLOC
#define HASHMAP_FREE ASPL_FREE
#include "thirdparty/hashmap/hashmap.h"

typedef struct ASPL_Map
{
    char* typePtr;
    int typeLen;
    hashmap_aspl_object_to_aspl_object_HashMap* hashmap;
} ASPL_Map;

typedef struct ASPL_Pointer
{
    char* typePtr;
    int typeLen;
    ASPL_OBJECT_TYPE* object;
} ASPL_Pointer;

typedef struct ASPL_ClosureMapPair
{
    char* key;
    ASPL_OBJECT_TYPE* value;
} ASPL_ClosureMapPair;

typedef struct ASPL_ClosureMap
{
    ASPL_ClosureMapPair* pairs;
    int length;
} ASPL_ClosureMap;

typedef struct ASPL_Callback
{
    char* typePtr;
    int typeLen;
    void* function;
    ASPL_ClosureMap* closure_map;
} ASPL_Callback;

#undef HASHMAP_PREFIX
#undef HASHMAP_KEY_TYPE
#undef HASHMAP_KEY_NULL_VALUE
#undef HASHMAP_VALUE_TYPE
#undef HASHMAP_VALUE_NULL_VALUE
#define HASHMAP_PREFIX hashmap_str_to_voidptr
#define HASHMAP_KEY_TYPE char *
#define HASHMAP_KEY_NULL_VALUE NULL
#define HASHMAP_VALUE_TYPE void *
#define HASHMAP_VALUE_NULL_VALUE NULL

unsigned int hashmap_str_to_voidptr_hash_key(char* key)
{
    unsigned int hash = 5381;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash = ((hash << 5) + hash) + key[i];
    }
    return hash;
}

int hashmap_str_to_voidptr_equals_key(char* key1, char* key2)
{
    return strcmp(key1, key2) == 0;
}

unsigned int hashmap_str_to_voidptr_hash_value(void* value)
{
    return (unsigned int)(size_t)value;
}

int hashmap_str_to_voidptr_equals_value(void* value1, void* value2)
{
    return value1 == value2;
}

#include "thirdparty/hashmap/hashmap.h"

typedef struct ASPL_ClassInstance
{
    char* typePtr;
    int typeLen;
    hashmap_str_to_voidptr_HashMap* properties;
    char isError;
} ASPL_ClassInstance;

ASPL_OBJECT_TYPE* ASPL_CLASS_INSTANCE_GET_PROPERTY_ADDRESS(ASPL_ClassInstance* instance, char* property)
{
    return ASPL_HASHMAP_UNWRAP((ASPL_Object_internal**)hashmap_str_to_voidptr_hashmap_get_value(instance->properties, property));
}

ASPL_OBJECT_TYPE ASPL_CLASS_INSTANCE_GET_PROPERTY(ASPL_ClassInstance* instance, char* property)
{
    return *ASPL_HASHMAP_UNWRAP((ASPL_Object_internal**)hashmap_str_to_voidptr_hashmap_get_value(instance->properties, property));
}

ASPL_OBJECT_TYPE ASPL_CLASS_INSTANCE_SET_PROPERTY(ASPL_ClassInstance* instance, char* property, ASPL_OBJECT_TYPE value)
{
    if (hashmap_str_to_voidptr_hashmap_get_value(instance->properties, property) == NULL)
    {
        ASPL_Object_internal** address = ASPL_MALLOC(sizeof(ASPL_Object_internal*));
        *address = ASPL_HASHMAP_WRAP(value);
        hashmap_str_to_voidptr_hashmap_set(instance->properties, property, address);
    }
    else {
        *ASPL_HASHMAP_UNWRAP((ASPL_Object_internal**)hashmap_str_to_voidptr_hashmap_get_value(instance->properties, property)) = value;
    }
    return value;
}

typedef struct ASPL_ReactiveProperty
{
    void* get_callback;
    void* set_callback;
} ASPL_ReactiveProperty;

hashmap_str_to_voidptr_HashMap reactive_properties_map;

void ASPL_CLASS_INIT_REACTIVE_PROPERTY(char* class, char* property, void* get_callback, void* set_callback)
{
    ASPL_ReactiveProperty* reactive_property = ASPL_MALLOC(sizeof(ASPL_ReactiveProperty));
    reactive_property->get_callback = get_callback;
    reactive_property->set_callback = set_callback;
    hashmap_str_to_voidptr_HashMap* reactive_properties = hashmap_str_to_voidptr_hashmap_get_value(&reactive_properties_map, class);
    if (reactive_properties == NULL)
    {
        reactive_properties = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 1 });
        hashmap_str_to_voidptr_hashmap_set(&reactive_properties_map, class, reactive_properties);
    }
    hashmap_str_to_voidptr_hashmap_set(reactive_properties, property, reactive_property);
}

ASPL_OBJECT_TYPE ASPL_REACTIVE_PROPERTY_GET(ASPL_OBJECT_TYPE* this, char* property)
{
    ASPL_ReactiveProperty* reactive_property = hashmap_str_to_voidptr_hashmap_get_value(hashmap_str_to_voidptr_hashmap_get_value(&reactive_properties_map, ASPL_ACCESS(*this).value.classInstance->typePtr), property);
    // TODO: Maybe add a debug check here to see if the property exists
    return ((ASPL_OBJECT_TYPE(*)(ASPL_OBJECT_TYPE*))reactive_property->get_callback)(this);
}

ASPL_OBJECT_TYPE ASPL_REACTIVE_PROPERTY_SET(ASPL_OBJECT_TYPE* this, char* property, ASPL_OBJECT_TYPE* value)
{
    ASPL_ReactiveProperty* reactive_property = hashmap_str_to_voidptr_hashmap_get_value(hashmap_str_to_voidptr_hashmap_get_value(&reactive_properties_map, ASPL_ACCESS(*this).value.classInstance->typePtr), property);
    return ((ASPL_OBJECT_TYPE(*)(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE*))reactive_property->set_callback)(this, value);
}

#undef HASHMAP_PREFIX
#undef HASHMAP_KEY_TYPE
#undef HASHMAP_KEY_NULL_VALUE
#undef HASHMAP_VALUE_TYPE
#undef HASHMAP_VALUE_NULL_VALUE
#define HASHMAP_PREFIX hashmap_int_to_str
#define HASHMAP_KEY_TYPE int
#define HASHMAP_KEY_NULL_VALUE 0
#define HASHMAP_VALUE_TYPE char *
#define HASHMAP_VALUE_NULL_VALUE NULL

unsigned int hashmap_int_to_str_hash_key(int key)
{
    return key;
}

int hashmap_int_to_str_equals_key(int key1, int key2)
{
    return key1 == key2;
}

unsigned int hashmap_int_to_str_hash_value(char* value)
{
    unsigned long hash = 5381;
    int c;
    char* str = value;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

int hashmap_int_to_str_equals_value(char* value1, char* value2)
{
    return strcmp(value1, value2) == 0;
}

#include "thirdparty/hashmap/hashmap.h"

typedef struct ASPL_Enum
{
    char* typePtr;
    int typeLen;
    char isFlagEnum;
    hashmap_int_to_str_HashMap* stringValues;
} ASPL_Enum;

typedef struct ASPL_EnumField
{
    ASPL_Enum* e;
    int intValue;
} ASPL_EnumField;

hashmap_str_to_voidptr_HashMap enums_map;

unsigned int aspl_hash(ASPL_OBJECT_TYPE obj)
{
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        return 0;
    case ASPL_OBJECT_KIND_BOOLEAN:
        return ASPL_ACCESS(obj).value.boolean;
    case ASPL_OBJECT_KIND_BYTE:
        return ASPL_ACCESS(obj).value.integer8;
    case ASPL_OBJECT_KIND_INTEGER:
        return ASPL_ACCESS(obj).value.integer32;
    case ASPL_OBJECT_KIND_LONG:
        return ASPL_ACCESS(obj).value.integer64;
    case ASPL_OBJECT_KIND_FLOAT:
        return ASPL_ACCESS(obj).value.float32;
    case ASPL_OBJECT_KIND_DOUBLE:
        return ASPL_ACCESS(obj).value.float64;
    case ASPL_OBJECT_KIND_STRING:
    {
        // hash function from https://stackoverflow.com/a/7666577
        unsigned long hash = 5381;
        int c;
        char* str = ASPL_ACCESS(obj).value.string->str;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
    }
    case ASPL_OBJECT_KIND_LIST:
    {
        unsigned long hash = 5381;
        for (int i = 0; i < ASPL_ACCESS(obj).value.list->length; i++)
        {
            ASPL_OBJECT_TYPE item = ASPL_ACCESS(obj).value.list->value[i];
            hash = ((hash << 5) + hash) + aspl_hash(item);
        }
        return hash;
    }
    case ASPL_OBJECT_KIND_MAP:
    {
        unsigned long hash = 5381;
        hashmap_aspl_object_to_aspl_object_Pair* pair = NULL;
        while ((pair = hashmap_aspl_object_to_aspl_object_hashmap_next(ASPL_ACCESS(obj).value.map->hashmap)) != NULL)
        {
            hash = ((hash << 5) + hash) + aspl_hash(ASPL_HASHMAP_UNWRAP(pair->key));
            hash = ((hash << 5) + hash) + aspl_hash(ASPL_HASHMAP_UNWRAP(pair->value));
        }
        return hash;
    }
    case ASPL_OBJECT_KIND_CALLBACK:
        return (int)(size_t)ASPL_ACCESS(obj).value.callback;
    case ASPL_OBJECT_KIND_POINTER:
        return aspl_hash(*ASPL_ACCESS(obj).value.pointer->object);
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
        return (int)(size_t)ASPL_ACCESS(obj).value.classInstance;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        return ASPL_ACCESS(obj).value.enumField->intValue;
    default:
        return 0;
    }
}

ASPL_OBJECT_TYPE aspl_object_clone_shallow(ASPL_OBJECT_TYPE obj)
{
    ASPL_OBJECT_TYPE clone = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(clone).kind = ASPL_ACCESS(obj).kind;
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_LIST:
        ASPL_ACCESS(clone).value.list = ASPL_MALLOC(sizeof(ASPL_List));
        ASPL_ACCESS(clone).value.list->typePtr = ASPL_ACCESS(obj).value.list->typePtr;
        ASPL_ACCESS(clone).value.list->typeLen = ASPL_ACCESS(obj).value.list->typeLen;
        ASPL_ACCESS(clone).value.list->length = ASPL_ACCESS(obj).value.list->length;
        ASPL_ACCESS(clone).value.list->capacity = ASPL_ACCESS(obj).value.list->capacity;
        ASPL_ACCESS(clone).value.list->value = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * ASPL_ACCESS(clone).value.list->capacity);
        memcpy(ASPL_ACCESS(clone).value.list->value, ASPL_ACCESS(obj).value.list->value, sizeof(ASPL_OBJECT_TYPE) * ASPL_ACCESS(obj).value.list->length);
        break;
    case ASPL_OBJECT_KIND_MAP:
    {
        ASPL_ACCESS(clone).value.map = ASPL_MALLOC(sizeof(ASPL_Map));
        ASPL_ACCESS(clone).value.map->typePtr = ASPL_ACCESS(obj).value.map->typePtr;
        ASPL_ACCESS(clone).value.map->typeLen = ASPL_ACCESS(obj).value.map->typeLen;
        int initial_capacity = ASPL_ACCESS(obj).value.map->hashmap->len;
        if (initial_capacity < 1) initial_capacity = 1; // hashmap_new can't handle 0 as initial capacity
        ASPL_ACCESS(clone).value.map->hashmap = hashmap_aspl_object_to_aspl_object_new_hashmap((hashmap_aspl_object_to_aspl_object_HashMapConfig) { .initial_capacity = initial_capacity });
        hashmap_aspl_object_to_aspl_object_Pair* pair = NULL;
        while ((pair = hashmap_aspl_object_to_aspl_object_hashmap_next(ASPL_ACCESS(obj).value.map->hashmap)) != NULL)
        {
            hashmap_aspl_object_to_aspl_object_hashmap_set(ASPL_ACCESS(clone).value.map->hashmap, pair->key, pair->value);
        }
        break;
    }
    case ASPL_OBJECT_KIND_POINTER:
        ASPL_ACCESS(clone).value.pointer = ASPL_MALLOC(sizeof(ASPL_Pointer));
        ASPL_ACCESS(clone).value.pointer->typePtr = ASPL_ACCESS(obj).value.pointer->typePtr;
        ASPL_ACCESS(clone).value.pointer->typeLen = ASPL_ACCESS(obj).value.pointer->typeLen;
        ASPL_ACCESS(clone).value.pointer->object = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
        *ASPL_ACCESS(clone).value.pointer->object = *ASPL_ACCESS(obj).value.pointer->object;
        break;
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
    {
        ASPL_ACCESS(clone).value.classInstance = ASPL_MALLOC(sizeof(ASPL_ClassInstance));
        ASPL_ACCESS(clone).value.classInstance->typePtr = ASPL_ACCESS(obj).value.classInstance->typePtr;
        ASPL_ACCESS(clone).value.classInstance->typeLen = ASPL_ACCESS(obj).value.classInstance->typeLen;
        int initial_capacity = ASPL_ACCESS(obj).value.classInstance->properties->len;
        if (initial_capacity < 1) initial_capacity = 1; // hashmap_new can't handle 0 as initial capacity
        ASPL_ACCESS(clone).value.classInstance->properties = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = initial_capacity });
        char* key = NULL;
        for (int i = 0; i < ASPL_ACCESS(obj).value.classInstance->properties->len; i++)
        {
            key = ASPL_ACCESS(obj).value.classInstance->properties->pairs[i]->key;
            if (key != NULL)
            {
                hashmap_str_to_voidptr_hashmap_set(ASPL_ACCESS(clone).value.classInstance->properties, key, hashmap_str_to_voidptr_hashmap_get_value(ASPL_ACCESS(obj).value.classInstance->properties, key));
            }
        }
        ASPL_ACCESS(clone).value.classInstance->isError = ASPL_ACCESS(obj).value.classInstance->isError;
        break;
    }
    default:
        ASPL_ACCESS(clone).value = ASPL_ACCESS(obj).value;
    }
    return clone;
}

ASPL_OBJECT_TYPE aspl_object_clone_deep(ASPL_OBJECT_TYPE obj)
{
    ASPL_OBJECT_TYPE clone = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(clone).kind = ASPL_ACCESS(obj).kind;
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_STRING:
        ASPL_ACCESS(clone).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(clone).value.string->str = ASPL_MALLOC(ASPL_ACCESS(obj).value.string->length + 1);
        memcpy(ASPL_ACCESS(clone).value.string->str, ASPL_ACCESS(obj).value.string->str, ASPL_ACCESS(obj).value.string->length);
        ASPL_ACCESS(clone).value.string->str[ASPL_ACCESS(obj).value.string->length] = '\0';
        ASPL_ACCESS(clone).value.string->length = ASPL_ACCESS(obj).value.string->length;
        break;
    case ASPL_OBJECT_KIND_LIST:
        ASPL_ACCESS(clone).value.list = ASPL_MALLOC(sizeof(ASPL_List));
        ASPL_ACCESS(clone).value.list->typePtr = ASPL_ACCESS(obj).value.list->typePtr;
        ASPL_ACCESS(clone).value.list->typeLen = ASPL_ACCESS(obj).value.list->typeLen;
        ASPL_ACCESS(clone).value.list->length = ASPL_ACCESS(obj).value.list->length;
        ASPL_ACCESS(clone).value.list->capacity = ASPL_ACCESS(obj).value.list->capacity;
        ASPL_ACCESS(clone).value.list->value = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * ASPL_ACCESS(clone).value.list->capacity);
        for (int i = 0; i < ASPL_ACCESS(obj).value.list->length; i++)
        {
            ASPL_ACCESS(clone).value.list->value[i] = aspl_object_clone_deep(ASPL_ACCESS(obj).value.list->value[i]);
        }
        break;
    case ASPL_OBJECT_KIND_MAP:
    {
        ASPL_ACCESS(clone).value.map = ASPL_MALLOC(sizeof(ASPL_Map));
        ASPL_ACCESS(clone).value.map->typePtr = ASPL_ACCESS(obj).value.map->typePtr;
        ASPL_ACCESS(clone).value.map->typeLen = ASPL_ACCESS(obj).value.map->typeLen;
        int initial_capacity = ASPL_ACCESS(obj).value.map->hashmap->len;
        if (initial_capacity < 1) initial_capacity = 1; // hashmap_new can't handle 0 as initial capacity
        ASPL_ACCESS(clone).value.map->hashmap = hashmap_aspl_object_to_aspl_object_new_hashmap((hashmap_aspl_object_to_aspl_object_HashMapConfig) { .initial_capacity = initial_capacity });
        hashmap_aspl_object_to_aspl_object_Pair* pair = NULL;
        while ((pair = hashmap_aspl_object_to_aspl_object_hashmap_next(ASPL_ACCESS(obj).value.map->hashmap)) != NULL)
        {
            hashmap_aspl_object_to_aspl_object_hashmap_set(ASPL_ACCESS(clone).value.map->hashmap, ASPL_HASHMAP_WRAP(aspl_object_clone_deep(ASPL_HASHMAP_UNWRAP(pair->key))), ASPL_HASHMAP_WRAP(aspl_object_clone_deep(ASPL_HASHMAP_UNWRAP(pair->value))));
        }
        break;
    }
    case ASPL_OBJECT_KIND_POINTER:
        ASPL_ACCESS(clone).value.pointer = ASPL_MALLOC(sizeof(ASPL_Pointer));
        ASPL_ACCESS(clone).value.pointer->typePtr = ASPL_ACCESS(obj).value.pointer->typePtr;
        ASPL_ACCESS(clone).value.pointer->typeLen = ASPL_ACCESS(obj).value.pointer->typeLen;
        ASPL_ACCESS(clone).value.pointer->object = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
        *ASPL_ACCESS(clone).value.pointer->object = aspl_object_clone_deep(*ASPL_ACCESS(obj).value.pointer->object);
        break;
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
    {
        ASPL_ACCESS(clone).value.classInstance = ASPL_MALLOC(sizeof(ASPL_ClassInstance));
        ASPL_ACCESS(clone).value.classInstance->typePtr = ASPL_ACCESS(obj).value.classInstance->typePtr;
        ASPL_ACCESS(clone).value.classInstance->typeLen = ASPL_ACCESS(obj).value.classInstance->typeLen;
        int initial_capacity = ASPL_ACCESS(obj).value.classInstance->properties->len;
        if (initial_capacity < 1) initial_capacity = 1; // hashmap_new can't handle 0 as initial capacity
        ASPL_ACCESS(clone).value.classInstance->properties = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = initial_capacity });
        char* key = NULL;
        for (int i = 0; i < ASPL_ACCESS(obj).value.classInstance->properties->len; i++)
        {
            key = ASPL_ACCESS(obj).value.classInstance->properties->pairs[i]->key;
            if (key != NULL)
            {
                hashmap_str_to_voidptr_hashmap_set(ASPL_ACCESS(clone).value.classInstance->properties, key, ASPL_HASHMAP_WRAP(aspl_object_clone_deep(ASPL_HASHMAP_UNWRAP((ASPL_Object_internal*)hashmap_str_to_voidptr_hashmap_get_value(ASPL_ACCESS(obj).value.classInstance->properties, key)))));
            }
        }
        ASPL_ACCESS(clone).value.classInstance->isError = ASPL_ACCESS(obj).value.classInstance->isError;
        break;
    }
    default:
        ASPL_ACCESS(clone).value = ASPL_ACCESS(obj).value;
        break;
    }
    return clone;
}

#ifndef ASPL_NO_MULTITHREADING
void ASPL_LAUNCH_THREAD(int (*callback)(void*), void* arguments, int arg_size)
{
    void* args = ASPL_MALLOC(arg_size);
    memcpy(args, arguments, arg_size);
    thread_create(callback, args, THREAD_STACK_SIZE_DEFAULT);
}
typedef struct ASPL_ThreadMethodWrapperData
{
    ASPL_OBJECT_TYPE (*callback)(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE* []);
    ASPL_OBJECT_TYPE* this;
    ASPL_OBJECT_TYPE** args;
} ASPL_ThreadMethodWrapperData;

int aspl_method_thread_wrapper(void* arguments)
{
    struct GC_stack_base sb;
    GC_get_stack_base(&sb);
    GC_register_my_thread(&sb);

    ASPL_ThreadMethodWrapperData* data = arguments;
    ASPL_OBJECT_TYPE (*callback)(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE* []) = data->callback;
    ASPL_OBJECT_TYPE* this = data->this;
    ASPL_OBJECT_TYPE** args = data->args;
    callback(this, args);

    GC_unregister_my_thread();
    return 0;
}

void ASPL_LAUNCH_THREAD_METHOD(ASPL_OBJECT_TYPE (*callback)(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE* []), ASPL_OBJECT_TYPE* this, void* arguments, int arg_size)
{
    ASPL_ThreadMethodWrapperData* args = ASPL_MALLOC(sizeof(ASPL_ThreadMethodWrapperData));
    args->callback = callback;
    args->this = this;
    args->args = arguments;
    thread_create(aspl_method_thread_wrapper, args, THREAD_STACK_SIZE_DEFAULT);
}
#endif

__attribute__((noreturn)) void ASPL_PANIC(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aspl_util_print("ASPL panic: ");
    aspl_util_vprint(format, args);
    aspl_util_print("\n");
    va_end(args);

#ifdef ASPL_DEBUG
    abort();
#else
    exit(1);
#endif
}

ASPL_OBJECT_TYPE ASPL_NULL()
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_NULL;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_TRUE()
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_BOOLEAN;
    ASPL_ACCESS(obj).value.boolean = 1;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_FALSE()
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_BOOLEAN;
    ASPL_ACCESS(obj).value.boolean = 0;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_BOOL_LITERAL(char b)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_BOOLEAN;
    ASPL_ACCESS(obj).value.boolean = b;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_EQUALS(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    return ASPL_BOOL_LITERAL(ASPL_IS_EQUAL(a, b));
}

int ASPL_IS_TRUE(ASPL_OBJECT_TYPE x)
{
    ASPL_OBJECT_TYPE obj = (ASPL_OBJECT_TYPE)x;
    if (ASPL_ACCESS(obj).kind == ASPL_OBJECT_KIND_BOOLEAN)
    {
        return ASPL_ACCESS(obj).value.boolean;
    }
    return 0;
}

int ASPL_IS_EQUAL(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    if (ASPL_ACCESS(a).kind != ASPL_ACCESS(b).kind)
        return 0;
    switch (ASPL_ACCESS(a).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        return 1;
    case ASPL_OBJECT_KIND_BOOLEAN:
        return ASPL_ACCESS(a).value.boolean == ASPL_ACCESS(b).value.boolean;
    case ASPL_OBJECT_KIND_BYTE:
        return ASPL_ACCESS(a).value.integer8 == ASPL_ACCESS(b).value.integer8;
    case ASPL_OBJECT_KIND_INTEGER:
        return ASPL_ACCESS(a).value.integer32 == ASPL_ACCESS(b).value.integer32;
    case ASPL_OBJECT_KIND_LONG:
        return ASPL_ACCESS(a).value.integer64 == ASPL_ACCESS(b).value.integer64;
    case ASPL_OBJECT_KIND_FLOAT:
        return ASPL_ACCESS(a).value.float32 == ASPL_ACCESS(b).value.float32;
    case ASPL_OBJECT_KIND_DOUBLE:
        return ASPL_ACCESS(a).value.float64 == ASPL_ACCESS(b).value.float64;
    case ASPL_OBJECT_KIND_STRING:
        return ASPL_ACCESS(a).value.string->length == ASPL_ACCESS(b).value.string->length && strcmp(ASPL_ACCESS(a).value.string->str, ASPL_ACCESS(b).value.string->str) == 0;
    case ASPL_OBJECT_KIND_LIST:
        return ASPL_ACCESS(a).value.list == ASPL_ACCESS(b).value.list; // TODO: Shouldn't this compare the elements and check if they are equal?
    case ASPL_OBJECT_KIND_MAP:
        // return ASPL_ACCESS(a).value.map == ASPL_ACCESS(b).value.map;
        return hashmap_aspl_object_to_aspl_object_hashmap_equals(ASPL_ACCESS(a).value.map->hashmap, ASPL_ACCESS(b).value.map->hashmap);
    case ASPL_OBJECT_KIND_CALLBACK:
        return ASPL_ACCESS(a).value.callback == ASPL_ACCESS(b).value.callback;
    case ASPL_OBJECT_KIND_POINTER:
        return ASPL_ACCESS(a).value.pointer == ASPL_ACCESS(b).value.pointer;
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
        return ASPL_ACCESS(a).value.classInstance == ASPL_ACCESS(b).value.classInstance;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        return ASPL_ACCESS(a).value.enumField->intValue == ASPL_ACCESS(b).value.enumField->intValue;
    default:
        return 0;
    }
}

ASPL_OBJECT_TYPE ASPL_NEGATE(ASPL_OBJECT_TYPE a)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    if (ASPL_ACCESS(objA).kind == ASPL_OBJECT_KIND_BOOLEAN)
    {
        ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_BOOLEAN;
        ASPL_ACCESS(obj).value.boolean = !ASPL_ACCESS(objA).value.boolean;
        return obj;
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_STRING_LITERAL(const char* str)
{
    size_t len = strlen(str);
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(obj).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(obj).value.string->str = ASPL_MALLOC_ATOMIC(len + 1);
    memcpy(ASPL_ACCESS(obj).value.string->str, str, len);
    ASPL_ACCESS(obj).value.string->str[len] = '\0';
    ASPL_ACCESS(obj).value.string->length = len;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_STRING_LITERAL_NO_COPY(char* str)
{
    size_t len = strlen(str);
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(obj).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(obj).value.string->str = str;
    ASPL_ACCESS(obj).value.string->length = len;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_BYTE_LITERAL(unsigned char b)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_BYTE;
    ASPL_ACCESS(obj).value.integer8 = b;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_INT_LITERAL(long i)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_INTEGER;
    ASPL_ACCESS(obj).value.integer32 = i;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_LONG_LITERAL(long long l)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_LONG;
    ASPL_ACCESS(obj).value.integer64 = l;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_FLOAT_LITERAL(float f)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_FLOAT;
    ASPL_ACCESS(obj).value.float32 = f;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_DOUBLE_LITERAL(double d)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_DOUBLE;
    ASPL_ACCESS(obj).value.float64 = d;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_LIST_LITERAL(char* typePtr, int typeLen, ASPL_OBJECT_TYPE list[], size_t size)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_LIST;
    ASPL_List* l = ASPL_MALLOC(sizeof(ASPL_List));
    l->typePtr = typePtr;
    l->typeLen = typeLen;
    l->length = size;
    l->capacity = size;
    if (l->capacity < 1) l->capacity = 1; // malloc can return a null pointer if size is 0, but e.g. memcpy can't handle null pointers (even if the size is 0)
    l->value = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * l->capacity);
    memcpy(l->value, list, sizeof(ASPL_OBJECT_TYPE) * size);
    ASPL_ACCESS(obj).value.list = l;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_MAP_LITERAL(char* typePtr, int typeLen, ASPL_OBJECT_TYPE list[], size_t size)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_MAP;
    ASPL_Map* m = ASPL_MALLOC(sizeof(ASPL_Map));
    m->typePtr = typePtr;
    m->typeLen = typeLen;
    hashmap_aspl_object_to_aspl_object_HashMap* hashmap = hashmap_aspl_object_to_aspl_object_new_hashmap((hashmap_aspl_object_to_aspl_object_HashMapConfig) { .initial_capacity = size < 1 ? 1 : size });
    for (int i = 0; i < size * 2; i += 2)
    {
        hashmap_aspl_object_to_aspl_object_hashmap_set(hashmap, ASPL_HASHMAP_WRAP(list[i]), ASPL_HASHMAP_WRAP(list[i + 1]));
    }
    m->hashmap = hashmap;
    ASPL_ACCESS(obj).value.map = m;
    return obj;
}

ASPL_ClosureMap* ASPL_NEW_CLOSURE_MAP(int count, ...)
{
    va_list args;
    va_start(args, count);

    ASPL_ClosureMapPair* pairs = ASPL_MALLOC(count * sizeof(ASPL_ClosureMapPair));
    for (int i = 0; i < count; i++)
    {
        pairs[i].key = va_arg(args, char*);
        pairs[i].value = va_arg(args, ASPL_OBJECT_TYPE*);
    }

    va_end(args);
    ASPL_ClosureMap* map = ASPL_MALLOC(sizeof(ASPL_ClosureMap));
    map->pairs = pairs;
    map->length = count;
    return map;
}

ASPL_OBJECT_TYPE* ASPL_CLOSURE_MAP_GET(ASPL_ClosureMap* map, char* key)
{
    for (int i = 0; i < map->length; i++)
    {
        if (strcmp(map->pairs[i].key, key) == 0)
        {
            return map->pairs[i].value;
        }
    }
    return NULL;
}

ASPL_OBJECT_TYPE ASPL_CALLBACK_LITERAL(char* typePtr, int typeLen, void* ptr, ASPL_ClosureMap* closure_map)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_CALLBACK;
    ASPL_Callback* cb = ASPL_MALLOC(sizeof(ASPL_Callback));
    cb->typePtr = typePtr;
    cb->typeLen = typeLen;
    cb->function = ptr;
    cb->closure_map = closure_map;
    ASPL_ACCESS(obj).value.callback = cb;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_ENUM_FIELD_LITERAL(ASPL_Enum* e, int value)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_ENUM_FIELD;
    ASPL_EnumField* field = ASPL_MALLOC(sizeof(ASPL_EnumField));
    field->e = e;
    field->intValue = value;
    ASPL_ACCESS(obj).value.enumField = field;
    return obj;
}

char* aspl_object_get_type_pointer(ASPL_OBJECT_TYPE obj)
{
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        return "null";
    case ASPL_OBJECT_KIND_BOOLEAN:
        return "boolean";
    case ASPL_OBJECT_KIND_INTEGER:
        return "integer";
    case ASPL_OBJECT_KIND_LONG:
        return "integer64";
    case ASPL_OBJECT_KIND_FLOAT:
        return "float";
    case ASPL_OBJECT_KIND_DOUBLE:
        return "double";
    case ASPL_OBJECT_KIND_STRING:
        return "string";
    case ASPL_OBJECT_KIND_LIST:
        return ASPL_ACCESS(obj).value.list->typePtr;
    case ASPL_OBJECT_KIND_MAP:
        return ASPL_ACCESS(obj).value.map->typePtr;
    case ASPL_OBJECT_KIND_CALLBACK:
        return ASPL_ACCESS(obj).value.callback->typePtr;
    case ASPL_OBJECT_KIND_POINTER:
        return ASPL_ACCESS(obj).value.pointer->typePtr;
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
        return ASPL_ACCESS(obj).value.classInstance->typePtr;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        return ASPL_ACCESS(obj).value.enumField->e->typePtr;
    case ASPL_OBJECT_KIND_HANDLE:
        return "handle";
    default:
        return "unknown";
    }
}

char* aspl_object_get_short_type_pointer(ASPL_OBJECT_TYPE obj)
{
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_LIST:
        return "list";
    case ASPL_OBJECT_KIND_MAP:
        return "map";
    default:
        return aspl_object_get_type_pointer(obj);
    }
}

char aspl_object_is_error(ASPL_OBJECT_TYPE obj) {
    if (ASPL_ACCESS(obj).kind == ASPL_OBJECT_KIND_CLASS_INSTANCE) {
        return ASPL_ACCESS(obj).value.classInstance->isError;
    }
    return 0;
}

typedef struct ASPL_ParentsList
{
    int amount;
    char** parents;
} ASPL_ParentsList;

hashmap_str_to_voidptr_HashMap class_parents_map;

void aspl_class_parent_init(char* class, char* parent)
{
    unsigned int key = hashmap_str_to_voidptr_hash_key(class);
    ASPL_ParentsList* parents = hashmap_str_to_voidptr_hashmap_get_value(&class_parents_map, class);
    if (parents == NULL)
    {
        parents = ASPL_MALLOC(sizeof(ASPL_ParentsList));
        parents->amount = 0;
        parents->parents = ASPL_MALLOC(sizeof(char*));
        hashmap_str_to_voidptr_hashmap_set(&class_parents_map, class, parents);
    }
    parents->amount++;
    parents->parents = ASPL_REALLOC(parents->parents, sizeof(char*) * parents->amount);
    parents->parents[parents->amount - 1] = parent;
}

int aspl_is_class_child_of(char* class, char* parent)
{
    ASPL_ParentsList* parents = hashmap_str_to_voidptr_hashmap_get_value(&class_parents_map, class);
    if (parents == NULL)
    {
        return 0;
    }
    for (int i = 0; i < parents->amount; i++)
    {
        if (strcmp(parents->parents[i], parent) == 0)
        {
            return 1;
        }
    }
    return 0;
}

ASPL_OBJECT_TYPE ASPL_OFTYPE(ASPL_OBJECT_TYPE a, char* type)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    int typeLen = strlen(type);
    switch (ASPL_ACCESS(a).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        return ASPL_BOOL_LITERAL(strcmp(type, "null") == 0);
    case ASPL_OBJECT_KIND_BOOLEAN:
        return ASPL_BOOL_LITERAL(strcmp(type, "bool") == 0);
    case ASPL_OBJECT_KIND_BYTE:
        return ASPL_BOOL_LITERAL(strcmp(type, "byte") == 0);
    case ASPL_OBJECT_KIND_INTEGER:
        return ASPL_BOOL_LITERAL(strcmp(type, "int") == 0);
    case ASPL_OBJECT_KIND_LONG:
        return ASPL_BOOL_LITERAL(strcmp(type, "long") == 0);
    case ASPL_OBJECT_KIND_FLOAT:
        return ASPL_BOOL_LITERAL(strcmp(type, "float") == 0);
    case ASPL_OBJECT_KIND_DOUBLE:
        return ASPL_BOOL_LITERAL(strcmp(type, "double") == 0);
    case ASPL_OBJECT_KIND_STRING:
        return ASPL_BOOL_LITERAL(strcmp(type, "string") == 0);
    case ASPL_OBJECT_KIND_LIST:
        return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.list->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.list->typePtr, type, typeLen) == 0);
    case ASPL_OBJECT_KIND_MAP:
        return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.map->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.map->typePtr, type, typeLen) == 0);
    case ASPL_OBJECT_KIND_CALLBACK:
        return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.callback->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.callback->typePtr, type, typeLen) == 0);
    case ASPL_OBJECT_KIND_POINTER:
        return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.pointer->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.pointer->typePtr, type, typeLen) == 0);
    case ASPL_OBJECT_KIND_CLASS_INSTANCE:
        if (ASPL_ACCESS(objA).value.classInstance->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.classInstance->typePtr, type, typeLen) == 0)
        {
            return ASPL_TRUE();
        }
        else
        {
            return ASPL_BOOL_LITERAL(aspl_is_class_child_of(ASPL_ACCESS(objA).value.classInstance->typePtr, type));
        }
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.enumField->e->typeLen == typeLen && memcmp(ASPL_ACCESS(objA).value.enumField->e->typePtr, type, typeLen) == 0);
    default:
        return ASPL_FALSE();
    }
}

ASPL_OBJECT_TYPE ASPL_REFERENCE(char* typePtr, int typeLen, ASPL_OBJECT_TYPE* address)
{
    ASPL_OBJECT_TYPE pointerObject = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(pointerObject).kind = ASPL_OBJECT_KIND_POINTER;
    ASPL_Pointer* ptr = ASPL_MALLOC(sizeof(ASPL_Pointer));
    ptr->typePtr = typePtr;
    ptr->typeLen = typeLen;
    ptr->object = address;
    ASPL_ACCESS(pointerObject).value.pointer = ptr;
    return pointerObject;
}

#define ASPL_DEREFERENCE(obj) (*ASPL_ACCESS(obj).value.pointer->object)

ASPL_OBJECT_TYPE ASPL_HANDLE_LITERAL(void* ptr)
{
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_HANDLE;
    ASPL_ACCESS(obj).value.handle = ptr;
    return obj;
}

void ASPL_ASSERT(ASPL_OBJECT_TYPE obj, char* file, int line, int column)
{
    if (ASPL_IS_TRUE(obj))
    {
        return;
    }
    aspl_util_print("%s:%d:%d: Assertion failed\n", file, line, column);
    exit(1);
}

char* aspl_stringify(ASPL_OBJECT_TYPE obj)
{
    char* str = NULL;
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        return "null";
        break;
    case ASPL_OBJECT_KIND_BOOLEAN:
        return ASPL_ACCESS(obj).value.boolean ? "true" : "false";
        break;
    case ASPL_OBJECT_KIND_BYTE:
        str = ASPL_MALLOC(sizeof(char) * 4);
        sprintf(str, "%u", ASPL_ACCESS(obj).value.integer8);
        return str;
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        str = ASPL_MALLOC(sizeof(char) * 12);
        sprintf(str, "%ld", ASPL_ACCESS(obj).value.integer32);
        return str;
        break;
    case ASPL_OBJECT_KIND_LONG:
        str = ASPL_MALLOC(sizeof(char) * 24);
        sprintf(str, "%lld", ASPL_ACCESS(obj).value.integer64);
        return str;
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        str = ASPL_MALLOC(sizeof(char) * 12);
        sprintf(str, "%f", ASPL_ACCESS(obj).value.float32);
        return str;
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        str = ASPL_MALLOC(sizeof(char) * 24);
        sprintf(str, "%f", ASPL_ACCESS(obj).value.float64);
        return str;
        break;
    case ASPL_OBJECT_KIND_STRING:
        return ASPL_ACCESS(obj).value.string->str;
    case ASPL_OBJECT_KIND_LIST:
        str = ASPL_MALLOC(sizeof(char) * 3);
        strcat(str, "[");
        for (int i = 0; i < ASPL_ACCESS(obj).value.list->length; i++)
        {
            char* str2 = aspl_stringify(ASPL_ACCESS(obj).value.list->value[i]);
            if (i != ASPL_ACCESS(obj).value.list->length - 1)
            {
                str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + 3));
                strcat(str, str2);
                strcat(str, ", ");
            }
            else
            {
                str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + 2));
                strcat(str, str2);
            }
        }
        strcat(str, "]");
        return str;
        break;
    case ASPL_OBJECT_KIND_MAP:
        str = ASPL_MALLOC(sizeof(char) * 2);
        strcat(str, "{");
        for (int i = 0; i < ASPL_ACCESS(obj).value.map->hashmap->len; i++)
        {
            char* str2 = aspl_stringify(ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(obj).value.map->hashmap->pairs[i]->key));
            char* str3 = aspl_stringify(ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(obj).value.map->hashmap->pairs[i]->value));
            if (i != ASPL_ACCESS(obj).value.map->hashmap->len - 1)
            {
                str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + strlen(str3) + 5));
                strcat(str, str2);
                strcat(str, ": ");
                strcat(str, str3);
                strcat(str, ", ");
            }
            else
            {
                str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + strlen(str3) + 4));
                strcat(str, str2);
                strcat(str, ": ");
                strcat(str, str3);
            }
        }
        strcat(str, "}");
        return str;
        break;
    case ASPL_OBJECT_KIND_CALLBACK:
        return ASPL_ACCESS(obj).value.callback->typePtr;
        break;
    case ASPL_OBJECT_KIND_POINTER:
    {
        char* s = aspl_stringify(*ASPL_ACCESS(obj).value.pointer->object);
        str = ASPL_MALLOC(sizeof(char) * (strlen(s) + 2));
        strcat(str, s);
        strcat(str, "*");
        return str;
    }
    break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        if (ASPL_ACCESS(obj).value.enumField->e->isFlagEnum)
        {
            for (int i = 0; i < ASPL_ACCESS(obj).value.enumField->e->stringValues->len; i++)
            {
                if (ASPL_ACCESS(obj).value.enumField->intValue & (1 << i))
                {
                    char* str2 = hashmap_int_to_str_hashmap_get_value(ASPL_ACCESS(obj).value.enumField->e->stringValues, 1 << i);
                    if (i != ASPL_ACCESS(obj).value.enumField->e->stringValues->len - 1 && (ASPL_ACCESS(obj).value.enumField->intValue & ~((1 << (i + 1)) - 1)))
                    {
                        if (str == NULL)
                        {
                            str = ASPL_MALLOC(sizeof(char) * (strlen(str2) + 2));
                        }
                        else
                        {
                            str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + 2));
                        }
                        strcat(str, str2);
                        strcat(str, "|");
                    }
                    else
                    {
                        if (str == NULL)
                        {
                            str = ASPL_MALLOC(sizeof(char) * (strlen(str2) + 1));
                        }
                        else
                        {
                            str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + 1));
                        }
                        strcat(str, str2);
                    }
                }
            }
            return str;
        }
        else
        {
            return hashmap_int_to_str_hashmap_get_value(ASPL_ACCESS(obj).value.enumField->e->stringValues, ASPL_ACCESS(obj).value.enumField->intValue);
        }
    default:
        return "ASPL: Unknown type";
        break;
    }
}

ASPL_String aspl_stringify_rso(ASPL_OBJECT_TYPE obj)
{
    ASPL_String str = { 0 };
    switch (ASPL_ACCESS(obj).kind)
    {
    case ASPL_OBJECT_KIND_NULL:
        str.str = "null";
        str.length = 4;
        return str;
        break;
    case ASPL_OBJECT_KIND_BOOLEAN:
        str.str = ASPL_ACCESS(obj).value.boolean ? "true" : "false";
        str.length = ASPL_ACCESS(obj).value.boolean ? 4 : 5;
        return str;
        break;
    case ASPL_OBJECT_KIND_BYTE:
        str.str = ASPL_MALLOC(sizeof(char) * 4);
        sprintf(str.str, "%u", ASPL_ACCESS(obj).value.integer8);
        str.length = strlen(str.str);
        return str;
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        str.str = ASPL_MALLOC(sizeof(char) * 12);
        sprintf(str.str, "%ld", ASPL_ACCESS(obj).value.integer32);
        str.length = strlen(str.str);
        return str;
        break;
    case ASPL_OBJECT_KIND_LONG:
        str.str = ASPL_MALLOC(sizeof(char) * 24);
        sprintf(str.str, "%lld", ASPL_ACCESS(obj).value.integer64);
        str.length = strlen(str.str);
        return str;
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        str.str = ASPL_MALLOC(sizeof(char) * 12);
        sprintf(str.str, "%f", ASPL_ACCESS(obj).value.float32);
        str.length = strlen(str.str);
        return str;
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        str.str = ASPL_MALLOC(sizeof(char) * 24);
        sprintf(str.str, "%f", ASPL_ACCESS(obj).value.float64);
        str.length = strlen(str.str);
        return str;
        break;
    case ASPL_OBJECT_KIND_STRING:
        return *ASPL_ACCESS(obj).value.string;
    case ASPL_OBJECT_KIND_LIST:
        str.str = ASPL_MALLOC(sizeof(char) * 3);
        strcat(str.str, "[");
        for (int i = 0; i < ASPL_ACCESS(obj).value.list->length; i++)
        {
            ASPL_String str2 = aspl_stringify_rso(ASPL_ACCESS(obj).value.list->value[i]);
            if (i != ASPL_ACCESS(obj).value.list->length - 1)
            {
                str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + 3));
                strcat(str.str, str2.str);
                strcat(str.str, ", ");
                str.length += str2.length + 2;
            }
            else
            {
                str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + 2));
                strcat(str.str, str2.str);
                str.length += str2.length + 1;
            }
        }
        strcat(str.str, "]");
        str.length += 1;
        return str;
        break;
    case ASPL_OBJECT_KIND_MAP:
        str.str = ASPL_MALLOC(sizeof(char) * 2);
        strcat(str.str, "{");
        for (int i = 0; i < ASPL_ACCESS(obj).value.map->hashmap->len; i++)
        {
            ASPL_String str2 = aspl_stringify_rso(ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(obj).value.map->hashmap->pairs[i]->key));
            ASPL_String str3 = aspl_stringify_rso(ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(obj).value.map->hashmap->pairs[i]->value));
            if (i != ASPL_ACCESS(obj).value.map->hashmap->len - 1)
            {
                str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + str3.length + 5));
                strcat(str.str, str2.str);
                strcat(str.str, ": ");
                strcat(str.str, str3.str);
                strcat(str.str, ", ");
                str.length += str2.length + str3.length + 4;
            }
            else
            {
                str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + str3.length + 4));
                strcat(str.str, str2.str);
                strcat(str.str, ": ");
                strcat(str.str, str3.str);
                str.length += str2.length + str3.length + 3;
            }
        }
        strcat(str.str, "}");
        str.length += 1;
        return str;
        break;
    case ASPL_OBJECT_KIND_CALLBACK:
        str.str = ASPL_ACCESS(obj).value.callback->typePtr;
        str.length = ASPL_ACCESS(obj).value.callback->typeLen;
        return str;
        break;
    case ASPL_OBJECT_KIND_POINTER:
    {
        ASPL_String s = aspl_stringify_rso(*ASPL_ACCESS(obj).value.pointer->object);
        str.str = ASPL_MALLOC(sizeof(char) * (s.length + 2));
        strcat(str.str, s.str);
        strcat(str.str, "*");
        str.length = s.length + 1;
        return str;
    }
    break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        /*if (((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->e->isFlagEnum)
        {
            for (int i = 0; i < ((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->e->stringValues->len; i++)
            {
                if (((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->intValue & (1 << i))
                {
                    printf("%s", hashmap_int_to_str_hashmap_get_value(((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->e->stringValues, 1 << i));
                    if (i != ((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->e->stringValues->len - 1 && (((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->intValue & ~((1 << (i + 1)) - 1)))
                    {
                        printf("|");
                    }
                }
            }
        }
        else
        {
            printf("%s", hashmap_int_to_str_hashmap_get_value(((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->e->stringValues, ((ASPL_EnumField *)ASPL_ACCESS(objA).value.custom)->intValue));
        }
        break;*/
        if (ASPL_ACCESS(obj).value.enumField->e->isFlagEnum)
        {
            for (int i = 0; i < ASPL_ACCESS(obj).value.enumField->e->stringValues->len; i++)
            {
                if (ASPL_ACCESS(obj).value.enumField->intValue & (1 << i))
                {
                    ASPL_String str2;
                    str2.str = hashmap_int_to_str_hashmap_get_value(ASPL_ACCESS(obj).value.enumField->e->stringValues, 1 << i);
                    str2.length = strlen(str2.str);
                    if (i != ASPL_ACCESS(obj).value.enumField->e->stringValues->len - 1 && (ASPL_ACCESS(obj).value.enumField->intValue & ~((1 << (i + 1)) - 1)))
                    {
                        if (str.str == NULL)
                        {
                            str.str = ASPL_MALLOC(sizeof(char) * (str2.length + 2));
                        }
                        else
                        {
                            str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + 2));
                        }
                        strcat(str.str, str2.str);
                        strcat(str.str, "|");
                        str.length += str2.length + 1;
                    }
                    else
                    {
                        if (str.str == NULL)
                        {
                            str.str = ASPL_MALLOC(sizeof(char) * (str2.length + 1));
                        }
                        else
                        {
                            str.str = ASPL_REALLOC(str.str, sizeof(char) * (str.length + str2.length + 1));
                        }
                        strcat(str.str, str2.str);
                        str.length += str2.length;
                    }
                }
            }
            return str;
        }
        else
        {
            ASPL_String str2;
            str2.str = hashmap_int_to_str_hashmap_get_value(ASPL_ACCESS(obj).value.enumField->e->stringValues, ASPL_ACCESS(obj).value.enumField->intValue);
            str2.length = strlen(str2.str);
            return str2;
        }
    default:
        str.str = "ASPL: Unknown type";
        str.length = 18;
        return str;
        break;
    }
}

#define ASPL_AND(a, b) (ASPL_IS_TRUE(a) ? b : ASPL_FALSE())
#define ASPL_OR(a, b) (ASPL_IS_TRUE(a) ? ASPL_TRUE() : b)
#define ASPL_XOR(a, b) (ASPL_IS_TRUE(a) ? (ASPL_IS_TRUE(b) ? ASPL_FALSE() : ASPL_TRUE()) : b)
#define ASPL_NEGATE(a) (ASPL_IS_TRUE(a) ? ASPL_FALSE() : ASPL_TRUE())

ASPL_OBJECT_TYPE ASPL_ENUM_AND(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_ENUM_FIELD;
    ASPL_ACCESS(obj).value.enumField = ASPL_MALLOC(sizeof(ASPL_EnumField));
    ASPL_ACCESS(obj).value.enumField->e = (ASPL_ACCESS(objA).value.enumField)->e;
    ASPL_ACCESS(obj).value.enumField->intValue = (ASPL_ACCESS(objA).value.enumField)->intValue & ASPL_ACCESS(objB).value.enumField->intValue;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_ENUM_OR(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_ENUM_FIELD;
    ASPL_ACCESS(obj).value.enumField = ASPL_MALLOC(sizeof(ASPL_EnumField));
    ASPL_ACCESS(obj).value.enumField->e = (ASPL_ACCESS(objA).value.enumField)->e;
    ASPL_ACCESS(obj).value.enumField->intValue = (ASPL_ACCESS(objA).value.enumField)->intValue | ASPL_ACCESS(objB).value.enumField->intValue;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_ENUM_XOR(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_ENUM_FIELD;
    ASPL_ACCESS(obj).value.enumField = ASPL_MALLOC(sizeof(ASPL_EnumField));
    ASPL_ACCESS(obj).value.enumField->e = (ASPL_ACCESS(objA).value.enumField)->e;
    ASPL_ACCESS(obj).value.enumField->intValue = (ASPL_ACCESS(objA).value.enumField)->intValue ^ ASPL_ACCESS(objB).value.enumField->intValue;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_PLUS(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    if (ASPL_ACCESS(objA).kind == ASPL_OBJECT_KIND_STRING)
    {
        ASPL_String s = aspl_stringify_rso(objB);
        int sLen = s.length;
        char* str = ASPL_MALLOC(ASPL_ACCESS(objA).value.string->length + sLen + 1);
        memcpy(str, ASPL_ACCESS(objA).value.string->str, ASPL_ACCESS(objA).value.string->length);
        memcpy(str + ASPL_ACCESS(objA).value.string->length, s.str, sLen);
        str[ASPL_ACCESS(objA).value.string->length + sLen] = '\0';
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(result).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(result).value.string->str = str;
        ASPL_ACCESS(result).value.string->length = ASPL_ACCESS(objA).value.string->length + sLen;
    }
    else if (ASPL_ACCESS(objB).kind == ASPL_OBJECT_KIND_STRING)
    {
        ASPL_String s = aspl_stringify_rso(objA);
        int sLen = s.length;
        char* str = ASPL_MALLOC(ASPL_ACCESS(objB).value.string->length + sLen + 1);
        memcpy(str, s.str, sLen);
        memcpy(str + sLen, ASPL_ACCESS(objB).value.string->str, ASPL_ACCESS(objB).value.string->length);
        str[ASPL_ACCESS(objB).value.string->length + sLen] = '\0';
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(result).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(result).value.string->str = str;
        ASPL_ACCESS(result).value.string->length = ASPL_ACCESS(objB).value.string->length + sLen;
    }
    else
    {
        switch (ASPL_ACCESS(objA).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            switch (ASPL_ACCESS(objB).kind)
            {
            case ASPL_OBJECT_KIND_BYTE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
                ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 + ASPL_ACCESS(objB).value.integer8;
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
                ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer8 + ASPL_ACCESS(objB).value.integer32;
                break;
            case ASPL_OBJECT_KIND_LONG:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
                ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer8 + ASPL_ACCESS(objB).value.integer64;
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer8 + ASPL_ACCESS(objB).value.float32;
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer8 + ASPL_ACCESS(objB).value.float64;
                break;
            default:
                ASPL_PANIC("Invalid type combination for + operator");
            }
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            switch (ASPL_ACCESS(objB).kind)
            {
            case ASPL_OBJECT_KIND_BYTE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
                ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 + ASPL_ACCESS(objB).value.integer8;
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
                ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 + ASPL_ACCESS(objB).value.integer32;
                break;
            case ASPL_OBJECT_KIND_LONG:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
                ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer32 + ASPL_ACCESS(objB).value.integer64;
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer32 + ASPL_ACCESS(objB).value.float32;
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer32 + ASPL_ACCESS(objB).value.float64;
                break;
            default:
                ASPL_PANIC("Invalid type combination for + operator");
            }
            break;
        case ASPL_OBJECT_KIND_LONG:
            switch (ASPL_ACCESS(objB).kind)
            {
            case ASPL_OBJECT_KIND_BYTE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
                ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 + ASPL_ACCESS(objB).value.integer8;
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
                ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 + ASPL_ACCESS(objB).value.integer32;
                break;
            case ASPL_OBJECT_KIND_LONG:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
                ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 + ASPL_ACCESS(objB).value.integer64;
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer64 + ASPL_ACCESS(objB).value.float32;
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer64 + ASPL_ACCESS(objB).value.float64;
                break;
            default:
                ASPL_PANIC("Invalid type combination for + operator");
            }
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            switch (ASPL_ACCESS(objB).kind)
            {
            case ASPL_OBJECT_KIND_BYTE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 + ASPL_ACCESS(objB).value.integer8;
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 + ASPL_ACCESS(objB).value.integer32;
                break;
            case ASPL_OBJECT_KIND_LONG:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 + ASPL_ACCESS(objB).value.integer64;
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
                ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 + ASPL_ACCESS(objB).value.float32;
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float32 + ASPL_ACCESS(objB).value.float64;
                break;
            default:
                ASPL_PANIC("Invalid type combination for + operator");
            }
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            switch (ASPL_ACCESS(objB).kind)
            {
            case ASPL_OBJECT_KIND_BYTE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + ASPL_ACCESS(objB).value.integer8;
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + ASPL_ACCESS(objB).value.integer32;
                break;
            case ASPL_OBJECT_KIND_LONG:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + ASPL_ACCESS(objB).value.integer64;
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + ASPL_ACCESS(objB).value.float32;
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                result = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
                ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + ASPL_ACCESS(objB).value.float64;
                break;
            default:
                ASPL_PANIC("Invalid type combination for + operator");
            }
            break;
        default:
            ASPL_PANIC("Invalid type combination for + operator");
        }
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_PLUS_PLUS(ASPL_OBJECT_TYPE a)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
        ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 + 1;
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
        ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 + 1;
        break;
    case ASPL_OBJECT_KIND_LONG:
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
        ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 + 1;
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
        ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 + 1;
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        result = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
        ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 + 1;
        break;
    default:
        ASPL_PANIC("Invalid type combination for + operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_MINUS(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 - ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer8 - ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer8 - ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer8 - ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer8 - ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for - operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 - ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 - ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer32 - ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer32 - ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer32 - ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for - operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 - ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 - ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 - ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer64 - ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer64 - ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for - operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 - ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 - ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 - ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 - ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float32 - ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for - operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 - ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 - ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 - ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 - ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 - ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for - operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for - operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_MULTIPLY(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 * ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer8 * ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer8 * ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer8 * ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer8 * ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for * operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 * ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 * ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer32 * ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer32 * ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer32 * ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for * operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 * ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 * ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 * ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer64 * ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer64 * ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for * operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 * ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 * ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 * ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 * ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float32 * ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for * operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 * ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 * ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 * ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 * ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 * ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for * operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for * operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_DIVIDE(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 / ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer8 / ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer8 / ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer8 / ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer8 / ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for / operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 / ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 / ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer32 / ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.integer32 / ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer32 / ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for / operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 / ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 / ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 / ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer64 / ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.integer64 / ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for / operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 / (float)ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 / (float)ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float32 / (double)ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = ASPL_ACCESS(objA).value.float32 / ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float32 / ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for / operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 / (double)ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 / (double)ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 / (double)ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 / (double)ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = ASPL_ACCESS(objA).value.float64 / ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for / operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for / operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_MODULO(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(result).value.integer8 = ASPL_ACCESS(objA).value.integer8 % ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer8 % ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer8 % ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.integer8, ASPL_ACCESS(objB).value.float32);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.integer8, ASPL_ACCESS(objB).value.float64);
            break;
        default:
            ASPL_PANIC("Invalid type combination for \% operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 % ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(result).value.integer32 = ASPL_ACCESS(objA).value.integer32 % ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer32 % ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.integer32, ASPL_ACCESS(objB).value.float32);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.integer32, ASPL_ACCESS(objB).value.float64);
            break;
        default:
            ASPL_PANIC("Invalid type combination for \% operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 % ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 % ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(result).value.integer64 = ASPL_ACCESS(objA).value.integer64 % ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.integer64, ASPL_ACCESS(objB).value.float32);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.integer64, ASPL_ACCESS(objB).value.float64);
            break;
        default:
            ASPL_PANIC("Invalid type combination for \% operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.float32, (float)ASPL_ACCESS(objB).value.integer8);
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.float32, (float)ASPL_ACCESS(objB).value.integer32);
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.float32, (float)ASPL_ACCESS(objB).value.integer64);
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(result).value.float32 = fmodf(ASPL_ACCESS(objA).value.float32, ASPL_ACCESS(objB).value.float32);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float32, ASPL_ACCESS(objB).value.float64);
            break;
        default:
            ASPL_PANIC("Invalid type combination for \% operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float64, (double)ASPL_ACCESS(objB).value.integer8);
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float64, (double)ASPL_ACCESS(objB).value.integer32);
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float64, (double)ASPL_ACCESS(objB).value.integer64);
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float64, (double)ASPL_ACCESS(objB).value.float32);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(result).value.float64 = fmod(ASPL_ACCESS(objA).value.float64, ASPL_ACCESS(objB).value.float64);
            break;
        default:
            ASPL_PANIC("Invalid type combination for \% operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for \% operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_LESS_THAN(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 < ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 < ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 < ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 < ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 < ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 < ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 < ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 < ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 < ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 < ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 < ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 < ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 < ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 < ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 < ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 < ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 < ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 < ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 < ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 < ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 < ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 < ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 < ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 < ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 < ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.enumField->intValue < ASPL_ACCESS(objB).value.enumField->intValue;
            break;
        default:
            ASPL_PANIC("Invalid type combination for < operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for < operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_LESS_THAN_OR_EQUAL(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 <= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 <= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 <= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 <= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 <= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 <= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 <= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 <= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 <= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 <= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 <= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 <= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 <= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 <= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 <= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 <= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 <= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 <= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 <= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 <= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 <= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 <= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 <= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 <= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 <= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.enumField->intValue <= ASPL_ACCESS(objB).value.enumField->intValue;
            break;
        default:
            ASPL_PANIC("Invalid type combination for <= operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for <= operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_GREATER_THAN(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 > ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 > ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 > ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 > ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 > ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 > ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 > ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 > ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 > ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 > ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 > ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 > ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 > ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 > ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 > ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 > ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 > ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 > ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 > ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 > ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 > ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 > ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 > ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 > ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 > ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.enumField->intValue > ASPL_ACCESS(objB).value.enumField->intValue;
            break;
        default:
            ASPL_PANIC("Invalid type combination for > operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for > operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_GREATER_THAN_OR_EQUAL(ASPL_OBJECT_TYPE a, ASPL_OBJECT_TYPE b)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)a;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)b;
    ASPL_OBJECT_TYPE result = ASPL_UNINITIALIZED;

    switch (ASPL_ACCESS(objA).kind)
    {
    case ASPL_OBJECT_KIND_BYTE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 >= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 >= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 >= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 >= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer8 >= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    case ASPL_OBJECT_KIND_INTEGER:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 >= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 >= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 >= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 >= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer32 >= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    case ASPL_OBJECT_KIND_LONG:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 >= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 >= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 >= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 >= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.integer64 >= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    case ASPL_OBJECT_KIND_FLOAT:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 >= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 >= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 >= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 >= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float32 >= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    case ASPL_OBJECT_KIND_DOUBLE:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 >= ASPL_ACCESS(objB).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 >= ASPL_ACCESS(objB).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 >= ASPL_ACCESS(objB).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 >= ASPL_ACCESS(objB).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.float64 >= ASPL_ACCESS(objB).value.float64;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    case ASPL_OBJECT_KIND_ENUM_FIELD:
        switch (ASPL_ACCESS(objB).kind)
        {
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            result = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(result).kind = ASPL_OBJECT_KIND_BOOLEAN;
            ASPL_ACCESS(result).value.boolean = ASPL_ACCESS(objA).value.enumField->intValue >= ASPL_ACCESS(objB).value.enumField->intValue;
            break;
        default:
            ASPL_PANIC("Invalid type combination for >= operator");
        }
        break;
    default:
        ASPL_PANIC("Invalid type combination for >= operator");
    }

    return result;
}

ASPL_OBJECT_TYPE ASPL_LIST_LENGTH(ASPL_OBJECT_TYPE list)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)list;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_INTEGER;
    ASPL_ACCESS(obj).value.integer32 = ASPL_ACCESS(objA).value.list->length;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_LIST_GET(ASPL_OBJECT_TYPE list, ASPL_OBJECT_TYPE index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)list;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)index;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < l->length)
    {
        return l->value[ASPL_ACCESS(objB).value.integer32];
    }
    else {
        ASPL_PANIC("Indexing list out of range (index: %d, length: %d)", ASPL_ACCESS(objB).value.integer32, l->length);
    }
}

ASPL_OBJECT_TYPE ASPL_LIST_SET(ASPL_OBJECT_TYPE list, ASPL_OBJECT_TYPE index, ASPL_OBJECT_TYPE value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)list;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)index;
    ASPL_OBJECT_TYPE objC = (ASPL_OBJECT_TYPE)value;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < l->length)
    {
        l->value[ASPL_ACCESS(objB).value.integer32] = objC;
    }
    return list;
}

ASPL_OBJECT_TYPE ASPL_MAP_LENGTH(ASPL_OBJECT_TYPE map)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)map;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_INTEGER;
    ASPL_ACCESS(obj).value.integer32 = ASPL_ACCESS(objA).value.map->hashmap->len;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_MAP_GET(ASPL_OBJECT_TYPE map, ASPL_OBJECT_TYPE key)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)map;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)key;
    ASPL_Object_internal* value = hashmap_aspl_object_to_aspl_object_hashmap_get_value(ASPL_ACCESS(objA).value.map->hashmap, ASPL_HASHMAP_WRAP(objB));
    if (value == NULL)
    {
        return ASPL_NULL();
    }
    return ASPL_HASHMAP_UNWRAP(value);
}

ASPL_OBJECT_TYPE ASPL_MAP_GET_KEY_FROM_INDEX(ASPL_OBJECT_TYPE map, ASPL_OBJECT_TYPE index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)map;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)index;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < m->hashmap->len)
    {
        return ASPL_HASHMAP_UNWRAP(m->hashmap->pairs[ASPL_ACCESS(objB).value.integer32]->key);
    }
    else {
        ASPL_PANIC("Indexing map out of range (index: %d, length: %d)", ASPL_ACCESS(objB).value.integer32, m->hashmap->len);
    }
}

ASPL_OBJECT_TYPE ASPL_MAP_GET_VALUE_FROM_INDEX(ASPL_OBJECT_TYPE map, ASPL_OBJECT_TYPE index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)map;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)index;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < m->hashmap->len)
    {
        return ASPL_HASHMAP_UNWRAP(m->hashmap->pairs[ASPL_ACCESS(objB).value.integer32]->value);
    }
    else {
        ASPL_PANIC("Indexing map out of range (index: %d, length: %d)", ASPL_ACCESS(objB).value.integer32, m->hashmap->len);
    }
}

ASPL_OBJECT_TYPE ASPL_MAP_SET(ASPL_OBJECT_TYPE map, ASPL_OBJECT_TYPE key, ASPL_OBJECT_TYPE value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)map;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)key;
    ASPL_OBJECT_TYPE objC = (ASPL_OBJECT_TYPE)value;
    hashmap_aspl_object_to_aspl_object_hashmap_set(ASPL_ACCESS(objA).value.map->hashmap, ASPL_HASHMAP_WRAP(objB), ASPL_HASHMAP_WRAP(objC));
    return map;
}

ASPL_OBJECT_TYPE ASPL_STRING_LENGTH(ASPL_OBJECT_TYPE string)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)string;
    ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_INTEGER;
    ASPL_ACCESS(obj).value.integer32 = ASPL_ACCESS(objA).value.string->length;
    return obj;
}

ASPL_OBJECT_TYPE ASPL_STRING_INDEX(ASPL_OBJECT_TYPE string, ASPL_OBJECT_TYPE index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)string;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)index;
    char* str = ASPL_ACCESS(objA).value.string->str;
    int len = ASPL_ACCESS(objA).value.string->length;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < len)
    {
        ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(obj).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(obj).value.string->str = ASPL_MALLOC(sizeof(char) * 2);
        ASPL_ACCESS(obj).value.string->str[0] = str[ASPL_ACCESS(objB).value.integer32];
        ASPL_ACCESS(obj).value.string->str[1] = '\0';
        ASPL_ACCESS(obj).value.string->length = 1;
        return obj;
    }
    else
    {
        ASPL_PANIC("Indexing string out of range (index: %d, length: %d)", ASPL_ACCESS(objB).value.integer32, len);
    }
}

ASPL_OBJECT_TYPE ASPL_CAST(ASPL_OBJECT_TYPE obj, char* type)
{
    ASPL_OBJECT_TYPE newObj = ASPL_UNINITIALIZED;
    if (strcmp(type, "null") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_NULL:
            return obj;
        default:
            ASPL_PANIC("Cannot an object of type %s to type null", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "bool") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BOOLEAN:
            return obj;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type bool", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "byte") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            return obj;
        case ASPL_OBJECT_KIND_INTEGER:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(newObj).value.integer8 = ASPL_ACCESS(obj).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(newObj).value.integer8 = ASPL_ACCESS(obj).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(newObj).value.integer8 = ASPL_ACCESS(obj).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(newObj).value.integer8 = ASPL_ACCESS(obj).value.float64;
            break;
        case ASPL_OBJECT_KIND_STRING:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_BYTE;
            ASPL_ACCESS(newObj).value.integer8 = atoi(ASPL_ACCESS(obj).value.string->str);
            break;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type byte", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "int") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = ASPL_ACCESS(obj).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            return obj;
        case ASPL_OBJECT_KIND_LONG:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = ASPL_ACCESS(obj).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = ASPL_ACCESS(obj).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = ASPL_ACCESS(obj).value.float64;
            break;
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = ASPL_ACCESS(obj).value.enumField->intValue;
            break;
        case ASPL_OBJECT_KIND_STRING:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_INTEGER;
            ASPL_ACCESS(newObj).value.integer32 = atoi(ASPL_ACCESS(obj).value.string->str);
            break;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type int", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "long") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(newObj).value.integer64 = ASPL_ACCESS(obj).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(newObj).value.integer64 = ASPL_ACCESS(obj).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            return obj;
        case ASPL_OBJECT_KIND_FLOAT:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(newObj).value.integer64 = ASPL_ACCESS(obj).value.float32;
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(newObj).value.integer64 = ASPL_ACCESS(obj).value.float64;
            break;
        case ASPL_OBJECT_KIND_STRING:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_LONG;
            ASPL_ACCESS(newObj).value.integer64 = atoi(ASPL_ACCESS(obj).value.string->str);
            break;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type long", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "float") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(newObj).value.float32 = ASPL_ACCESS(obj).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(newObj).value.float32 = ASPL_ACCESS(obj).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(newObj).value.float32 = ASPL_ACCESS(obj).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            return obj;
        case ASPL_OBJECT_KIND_DOUBLE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(newObj).value.float32 = ASPL_ACCESS(obj).value.float64;
            break;
        case ASPL_OBJECT_KIND_STRING:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_FLOAT;
            ASPL_ACCESS(newObj).value.float32 = atof(ASPL_ACCESS(obj).value.string->str);
            break;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type float", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "double") == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_BYTE:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(newObj).value.float64 = ASPL_ACCESS(obj).value.integer8;
            break;
        case ASPL_OBJECT_KIND_INTEGER:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(newObj).value.float64 = ASPL_ACCESS(obj).value.integer32;
            break;
        case ASPL_OBJECT_KIND_LONG:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(newObj).value.float64 = ASPL_ACCESS(obj).value.integer64;
            break;
        case ASPL_OBJECT_KIND_FLOAT:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(newObj).value.float64 = ASPL_ACCESS(obj).value.float32;
            break;
        case ASPL_OBJECT_KIND_STRING:
            newObj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(newObj).kind = ASPL_OBJECT_KIND_DOUBLE;
            ASPL_ACCESS(newObj).value.float64 = atof(ASPL_ACCESS(obj).value.string->str);
            break;
        case ASPL_OBJECT_KIND_DOUBLE:
            return obj;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type double", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strcmp(type, "string") == 0)
    {
        return ASPL_STRING_LITERAL(aspl_stringify(obj));
    }
    else if (strncmp(type, "list<", 5) == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_LIST:
            return obj;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type list", aspl_object_get_type_pointer(obj));
        }
    }
    else if (strncmp(type, "map<", 4) == 0)
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_MAP:
            return obj;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to type map", aspl_object_get_type_pointer(obj));
        }
    }
    else if (hashmap_str_to_voidptr_hashmap_contains_key(&enums_map, type))
    {
        switch (ASPL_ACCESS(obj).kind)
        {
        case ASPL_OBJECT_KIND_INTEGER:
            return ASPL_ENUM_FIELD_LITERAL(hashmap_str_to_voidptr_hashmap_get_value(&enums_map, type), ASPL_ACCESS(obj).value.integer32);
        case ASPL_OBJECT_KIND_ENUM_FIELD:
            return obj;
        default:
            ASPL_PANIC("Cannot cast an object of type %s to an enum", aspl_object_get_type_pointer(obj));
        }
    }
    else
    {
        return obj;
    }
    return newObj;
}

void aspl_function_print(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* newline)
{
    aspl_util_print("%s", aspl_stringify((ASPL_OBJECT_TYPE)*obj));
    ASPL_OBJECT_TYPE newlineObj = (ASPL_OBJECT_TYPE)*newline;
    if (ASPL_IS_TRUE(newlineObj))
    {
        aspl_util_print("\n");
    }
}

ASPL_OBJECT_TYPE aspl_function_input(ASPL_OBJECT_TYPE* prompt)
{
    aspl_util_print("%s", aspl_stringify((ASPL_OBJECT_TYPE)*prompt));
    fflush(stdout);
    size_t buffer_size = 1024;
    char* str = ASPL_MALLOC(buffer_size);
    if (fgets(str, buffer_size, stdin) == NULL) {
        ASPL_FREE(str);
        return ASPL_STRING_LITERAL("");
    }
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    return ASPL_STRING_LITERAL_NO_COPY(str);
}

void aspl_function_exit(ASPL_OBJECT_TYPE* code)
{
    ASPL_OBJECT_TYPE codeObj = (ASPL_OBJECT_TYPE)*code;
    exit(ASPL_ACCESS(codeObj).value.integer32);
}

void aspl_function_panic(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_PANIC(ASPL_ACCESS(objA).value.string->str);
}

ASPL_OBJECT_TYPE aspl_method_string_toLower(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    for (int i = 0; i < ASPL_ACCESS(objA).value.string->length; i++)
    {
        str2[i] = tolower(str[i]);
    }
    str2[ASPL_ACCESS(objA).value.string->length] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = ASPL_ACCESS(objA).value.string->length;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_toLower_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_toLower(obj);
}

ASPL_OBJECT_TYPE aspl_method_string_toUpper(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    for (int i = 0; i < ASPL_ACCESS(objA).value.string->length; i++)
    {
        str2[i] = toupper(str[i]);
    }
    str2[ASPL_ACCESS(objA).value.string->length] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = ASPL_ACCESS(objA).value.string->length;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_toUpper_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_toUpper(obj);
}

ASPL_OBJECT_TYPE aspl_method_string_replace(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* oldValue, ASPL_OBJECT_TYPE* newValue)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*oldValue;
    ASPL_OBJECT_TYPE objC = (ASPL_OBJECT_TYPE)*newValue;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_ACCESS(objB).value.string->str;
    char* str3 = ASPL_ACCESS(objC).value.string->str;

    int lenA = ASPL_ACCESS(objA).value.string->length;
    int lenB = ASPL_ACCESS(objB).value.string->length;
    int lenC = ASPL_ACCESS(objC).value.string->length;

    int newSize = lenA + 1;
    for (int i = 0; i < lenA; i++)
    {
        if (strncmp(str + i, str2, lenB) == 0)
        {
            newSize += lenC - lenB;
            i += lenB - 1;
        }
    }

    char* str4 = ASPL_MALLOC(sizeof(char) * newSize);
    int i = 0;
    int j = 0;

    while (i < lenA)
    {
        if (strncmp(str + i, str2, lenB) == 0)
        {
            for (int k = 0; k < lenC; k++)
            {
                str4[j++] = str3[k];
            }
            i += lenB;
        }
        else
        {
            str4[j++] = str[i++];
        }
    }

    str4[j] = '\0';

    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str4;
    ASPL_ACCESS(s).value.string->length = newSize - 1;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_replace_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_replace(obj, arguments[0], arguments[1]);
}

ASPL_OBJECT_TYPE aspl_method_string_replaceMany(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* dictionary)
{
    // TODO: Use a more efficient implementation
    ASPL_OBJECT_TYPE s = *obj;
    for (int i = 0; i < ASPL_ACCESS(*dictionary).value.map->hashmap->len; i++)
    {
        ASPL_OBJECT_TYPE key = ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(*dictionary).value.map->hashmap->pairs[i]->key);
        ASPL_OBJECT_TYPE value = ASPL_HASHMAP_UNWRAP(ASPL_ACCESS(*dictionary).value.map->hashmap->pairs[i]->value);
        s = aspl_method_string_replace(C_REFERENCE(s), C_REFERENCE(key), C_REFERENCE(value));
    }
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_replaceMany_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_replaceMany(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_startsWith(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* prefix)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*prefix;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_ACCESS(objB).value.string->str;
    return ASPL_BOOL_LITERAL(strncmp(str, str2, ASPL_ACCESS(objB).value.string->length) == 0);
}

ASPL_OBJECT_TYPE aspl_method_string_startsWith_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_startsWith(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_endsWith(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* suffix)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*suffix;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_ACCESS(objB).value.string->str;
    return ASPL_BOOL_LITERAL(ASPL_ACCESS(objA).value.string->length >= ASPL_ACCESS(objB).value.string->length && strncmp(str + ASPL_ACCESS(objA).value.string->length - ASPL_ACCESS(objB).value.string->length, str2, ASPL_ACCESS(objB).value.string->length) == 0);
}

ASPL_OBJECT_TYPE aspl_method_string_endsWith_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_endsWith(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_contains(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* substring)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*substring;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_ACCESS(objB).value.string->str;
    return ASPL_BOOL_LITERAL(strstr(str, str2) != NULL);
}

ASPL_OBJECT_TYPE aspl_method_string_contains_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_contains(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_after(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*index;
    char* str = ASPL_ACCESS(objA).value.string->str;
    int i = ASPL_ACCESS(objB).value.integer32;
    if (i >= -1 && i < (signed int)ASPL_ACCESS(objA).value.string->length)
    {
        char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length - i));
        for (int j = i + 1; j < ASPL_ACCESS(objA).value.string->length; j++)
        {
            str2[j - (i + 1)] = str[j];
        }
        str2[ASPL_ACCESS(objA).value.string->length - (i + 1)] = '\0';
        ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(s).value.string->str = str2;
        ASPL_ACCESS(s).value.string->length = ASPL_ACCESS(objA).value.string->length - (i + 1);
        return s;
    }
    ASPL_PANIC("string.after(): Indexing string out of range (index: %d, length: %d)", i, ASPL_ACCESS(objA).value.string->length);
}

ASPL_OBJECT_TYPE aspl_method_string_after_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_after(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_before(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*index;
    char* str = ASPL_ACCESS(objA).value.string->str;
    int i = ASPL_ACCESS(objB).value.integer32;
    if (i >= 0 && i <= ASPL_ACCESS(objA).value.string->length)
    {
        char* str2 = ASPL_MALLOC(sizeof(char) * (i + 1));
        for (int j = 0; j < i; j++)
        {
            str2[j] = str[j];
        }
        str2[i] = '\0';
        ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(s).value.string->str = str2;
        ASPL_ACCESS(s).value.string->length = i;
        return s;
    }
    ASPL_PANIC("string.before(): Indexing string out of range (index: %d, length: %d)", i, ASPL_ACCESS(objA).value.string->length);
}

ASPL_OBJECT_TYPE aspl_method_string_before_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_before(obj, arguments[0]);
}

char aspl_custom_is_space(char c, ASPL_OBJECT_TYPE* chars)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*chars;
    char* str = ASPL_ACCESS(objA).value.string->str;
    for (int i = 0; i < ASPL_ACCESS(objA).value.string->length; i++)
    {
        if (c == str[i])
        {
            return 1;
        }
    }
    return 0;
}

ASPL_OBJECT_TYPE aspl_method_string_trim(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* chars)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    int i = 0;
    int j = ASPL_ACCESS(objA).value.string->length - 1;
    while (i < ASPL_ACCESS(objA).value.string->length && aspl_custom_is_space(str[i], chars))
    {
        i++;
    }
    while (j >= 0 && aspl_custom_is_space(str[j], chars))
    {
        j--;
    }
    for (int k = i; k <= j; k++)
    {
        str2[k - i] = str[k];
    }
    str2[j - i + 1] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = j - i + 1;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_trim_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_trim(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_trimStart(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* chars)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    int i = 0;
    while (i < ASPL_ACCESS(objA).value.string->length && aspl_custom_is_space(str[i], chars))
    {
        i++;
    }
    for (int k = i; k < ASPL_ACCESS(objA).value.string->length; k++)
    {
        str2[k - i] = str[k];
    }
    str2[ASPL_ACCESS(objA).value.string->length - i] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = ASPL_ACCESS(objA).value.string->length - i;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_trimStart_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_trimStart(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_trimEnd(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* chars)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    int i = ASPL_ACCESS(objA).value.string->length - 1;
    while (i >= 0 && aspl_custom_is_space(str[i], chars))
    {
        i--;
    }
    for (int k = 0; k <= i; k++)
    {
        str2[k] = str[k];
    }
    str2[i + 1] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = i + 1;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_trimEnd_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_trimEnd(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_string_split(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* separator, ASPL_OBJECT_TYPE* limit)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*separator;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_ACCESS(objB).value.string->str;
    ASPL_List* l = ASPL_MALLOC(sizeof(ASPL_List));
    l->length = 0;
    l->capacity = 4; // (arbitrary) expected average of splits
    l->value = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * l->capacity);

    char* str3 = strstr(str, str2);
    int remaining_splits = ASPL_ACCESS(*limit).value.integer32 - 1;

    while (str3 != NULL && remaining_splits != 0)
    {
        l->length++;
        l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->length);

        size_t token_len = str3 - str;
        char* token = (char*)ASPL_MALLOC(token_len + 1);
        strncpy(token, str, token_len);
        token[token_len] = '\0';

        ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(s).value.string->str = token;
        ASPL_ACCESS(s).value.string->length = token_len;
        l->value[l->length - 1] = s;

        str = str3 + ASPL_ACCESS(objB).value.string->length;
        str3 = strstr(str, str2);

        remaining_splits--;
    }

    if (strlen(str) > 0) // Attention: strlen() has to be used here (because the pointer is updated in the loop above)
    {
        l->length++;
        l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->length);

        ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
        ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
        ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
        ASPL_ACCESS(s).value.string->str = ASPL_MALLOC(strlen(str) + 1);
        strcpy(ASPL_ACCESS(s).value.string->str, str);
        ASPL_ACCESS(s).value.string->str[strlen(str)] = '\0';
        ASPL_ACCESS(s).value.string->length = strlen(str);
        l->value[l->length - 1] = s;
    }

    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_LIST;
    ASPL_ACCESS(s).value.list = l;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_split_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_split(obj, arguments[0], arguments[1]);
}

ASPL_OBJECT_TYPE aspl_method_string_reverse(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    char* str = ASPL_ACCESS(objA).value.string->str;
    char* str2 = ASPL_MALLOC(sizeof(char) * (ASPL_ACCESS(objA).value.string->length + 1));
    for (int i = 0; i < ASPL_ACCESS(objA).value.string->length; i++)
    {
        str2[i] = str[ASPL_ACCESS(objA).value.string->length - i - 1];
    }
    str2[ASPL_ACCESS(objA).value.string->length] = '\0';
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str2;
    ASPL_ACCESS(s).value.string->length = ASPL_ACCESS(objA).value.string->length;
    return s;
}

ASPL_OBJECT_TYPE aspl_method_string_reverse_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_string_reverse(obj);
}

ASPL_OBJECT_TYPE aspl_method_list_contains(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*value;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    for (int i = 0; i < l->length; i++)
    {
        if (ASPL_IS_EQUAL(l->value[i], objB))
        {
            return ASPL_TRUE();
        }
    }
    return ASPL_FALSE();
}

ASPL_OBJECT_TYPE aspl_method_list_contains_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_contains(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_list_add(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*value;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    l->length++;
    if (l->length >= l->capacity) {
        l->capacity *= 2;
        l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
    }
    l->value[l->length - 1] = objB;
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_list_add_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_add(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_list_insert(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* index, ASPL_OBJECT_TYPE* value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*index;
    ASPL_OBJECT_TYPE objC = (ASPL_OBJECT_TYPE)*value;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 <= l->length)
    {
        l->length++;
        if (l->length >= l->capacity) {
            l->capacity *= 2;
            l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
        }
        for (int i = l->length - 1; i > ASPL_ACCESS(objB).value.integer32; i--)
        {
            l->value[i] = l->value[i - 1];
        }
        l->value[ASPL_ACCESS(objB).value.integer32] = objC;
    }
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_list_insert_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_insert(obj, arguments[0], arguments[1]);
}

ASPL_OBJECT_TYPE aspl_method_list_insertElements(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* index, ASPL_OBJECT_TYPE* values)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*index;
    ASPL_OBJECT_TYPE objC = (ASPL_OBJECT_TYPE)*values;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 <= l->length)
    {
        l->length += ASPL_ACCESS(objC).value.list->length;
        while (l->length >= l->capacity) {
            l->capacity *= 2;
            l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
        }
        for (int i = l->length - 1; i > ASPL_ACCESS(objB).value.integer32; i--)
        {
            l->value[i] = l->value[i - 1];
        }
        for (int i = 0; i < ASPL_ACCESS(objC).value.list->length; i++)
        {
            l->value[ASPL_ACCESS(objB).value.integer32 + i] = ASPL_ACCESS(objC).value.list->value[i];
        }
    }
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_list_insertElements_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_insertElements(obj, arguments[0], arguments[1]);
}

ASPL_OBJECT_TYPE aspl_method_list_remove(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*value;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    for (int i = 0; i < l->length; i++)
    {
        if (ASPL_IS_EQUAL(l->value[i], objB))
        {
            for (int j = i; j < l->length - 1; j++)
            {
                l->value[j] = l->value[j + 1];
            }
            l->length--;
            if (l->length < l->capacity / 2) {
                l->capacity /= 2;
                l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
            }
            return ASPL_TRUE();
        }
    }
    return ASPL_FALSE();
}

ASPL_OBJECT_TYPE aspl_method_list_remove_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_remove(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_list_removeAt(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* index)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*index;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    if (ASPL_ACCESS(objB).value.integer32 >= 0 && ASPL_ACCESS(objB).value.integer32 < l->length)
    {
        for (int i = ASPL_ACCESS(objB).value.integer32; i < l->length - 1; i++)
        {
            l->value[i] = l->value[i + 1];
        }
        l->length--;
        if (l->length < l->capacity / 2) {
            l->capacity /= 2;
            l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
        }
    }
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_list_removeAt_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_removeAt(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_list_clear(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    // don't free the objects as the GC will take care of it; freeing them here manually may free objects that are still in use
    l->length = 0;
    l->capacity = 1; // realloc can return a null pointer if size is 0, but e.g. memcpy can't handle null pointers (even if the size is 0)
    l->value = ASPL_REALLOC(l->value, sizeof(ASPL_OBJECT_TYPE) * l->capacity);
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_list_clear_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_clear(obj);
}

ASPL_OBJECT_TYPE aspl_method_list_join(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* separator)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*separator;
    ASPL_List* l = ASPL_ACCESS(objA).value.list;
    char* str = ASPL_MALLOC(sizeof(char));
    str[0] = '\0';
    for (int i = 0; i < l->length; i++)
    {
        char* str2 = aspl_stringify(l->value[i]);
        str = ASPL_REALLOC(str, sizeof(char) * (strlen(str) + strlen(str2) + ASPL_ACCESS(objB).value.string->length + 1));
        strcat(str, str2);
        if (i != l->length - 1)
        {
            strcat(str, ASPL_ACCESS(objB).value.string->str);
        }
    }
    ASPL_OBJECT_TYPE s = ASPL_ALLOC_OBJECT();
    ASPL_ACCESS(s).kind = ASPL_OBJECT_KIND_STRING;
    ASPL_ACCESS(s).value.string = ASPL_MALLOC(sizeof(ASPL_String));
    ASPL_ACCESS(s).value.string->str = str;
    ASPL_ACCESS(s).value.string->length = strlen(str);
    return s;
}

ASPL_OBJECT_TYPE aspl_method_list_join_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_list_join(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_map_containsKey(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* key)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*key;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    return ASPL_BOOL_LITERAL(hashmap_aspl_object_to_aspl_object_hashmap_contains_key(m->hashmap, ASPL_HASHMAP_WRAP(objB)));
}

ASPL_OBJECT_TYPE aspl_method_map_containsKey_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_map_containsKey(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_map_containsValue(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* value)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*value;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    return ASPL_BOOL_LITERAL(hashmap_aspl_object_to_aspl_object_hashmap_contains_value(m->hashmap, ASPL_HASHMAP_WRAP(objB)));
}

ASPL_OBJECT_TYPE aspl_method_map_containsValue_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_map_containsValue(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_map_remove(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* key)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_OBJECT_TYPE objB = (ASPL_OBJECT_TYPE)*key;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    hashmap_aspl_object_to_aspl_object_hashmap_remove(m->hashmap, ASPL_HASHMAP_WRAP(objB));
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_map_remove_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_map_remove(obj, arguments[0]);
}

ASPL_OBJECT_TYPE aspl_method_map_clear(ASPL_OBJECT_TYPE* obj)
{
    ASPL_OBJECT_TYPE objA = (ASPL_OBJECT_TYPE)*obj;
    ASPL_Map* m = ASPL_ACCESS(objA).value.map;
    hashmap_aspl_object_to_aspl_object_hashmap_clear(m->hashmap);
    return objA;
}

ASPL_OBJECT_TYPE aspl_method_map_clear_wrapper(ASPL_OBJECT_TYPE* obj, ASPL_OBJECT_TYPE* arguments[])
{
    return aspl_method_map_clear(obj);
}

hashmap_str_to_voidptr_HashMap object_methods_map;

void aspl_object_method_init(char* class, char* method, ASPL_OBJECT_TYPE (*func)(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE* []))
{
    unsigned int key = hashmap_str_to_voidptr_hash_key(class);
    hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(&object_methods_map, class);
    if (methods == NULL)
    {
        methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 1 });
        hashmap_str_to_voidptr_hashmap_set(&object_methods_map, class, methods);
    }
    hashmap_str_to_voidptr_hashmap_set(methods, method, func);
}

ASPL_OBJECT_TYPE (*aspl_object_method_get(char* class, char* method))(ASPL_OBJECT_TYPE*, ASPL_OBJECT_TYPE* [])
{
    hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(&object_methods_map, class);
    if (methods == NULL)
    {
        return NULL;
    }
    return hashmap_str_to_voidptr_hashmap_get_value(methods, method);
}

ASPL_OBJECT_TYPE aspl_object_method_invoke(ASPL_OBJECT_TYPE base, char* method, ASPL_OBJECT_TYPE* args[])
{
    if (strcmp(method, "cloneShallow") == 0)
    { // TODO: Improve the performance of these
        return aspl_object_clone_shallow(base);
    }
    else if (strcmp(method, "cloneDeep") == 0)
    {
        return aspl_object_clone_deep(base);
    }

    return aspl_object_method_get(aspl_object_get_short_type_pointer(base), method)(C_REFERENCE(base), args);
}

#ifndef ASPL_NO_MULTITHREADING
void aspl_method_invoke_newthread(ASPL_OBJECT_TYPE base, void* method, ASPL_OBJECT_TYPE* args[], int arg_size)
{
    // no cloneShallow and cloneDeep as it wouldn't make sense to run them on a new thread
    ASPL_LAUNCH_THREAD_METHOD(method, C_REFERENCE(base), args, arg_size);
}

void aspl_object_method_invoke_newthread(ASPL_OBJECT_TYPE base, char* method, ASPL_OBJECT_TYPE* args[], int arg_size)
{
    aspl_method_invoke_newthread(base, aspl_object_method_get(aspl_object_get_short_type_pointer(base), method), args, arg_size);
}
#endif

void aspl_setup_builtin_method_pointers()
{
    object_methods_map = *hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 1 });

    aspl_object_method_init("string", "toLower", aspl_method_string_toLower_wrapper);
    aspl_object_method_init("string", "toUpper", aspl_method_string_toUpper_wrapper);
    aspl_object_method_init("string", "replace", aspl_method_string_replace_wrapper);
    aspl_object_method_init("string", "replaceMany", aspl_method_string_replaceMany_wrapper);
    aspl_object_method_init("string", "startsWith", aspl_method_string_startsWith_wrapper);
    aspl_object_method_init("string", "endsWith", aspl_method_string_endsWith_wrapper);
    aspl_object_method_init("string", "contains", aspl_method_string_contains_wrapper);
    aspl_object_method_init("string", "after", aspl_method_string_after_wrapper);
    aspl_object_method_init("string", "before", aspl_method_string_before_wrapper);
    aspl_object_method_init("string", "trim", aspl_method_string_trim_wrapper);
    aspl_object_method_init("string", "trimStart", aspl_method_string_trimStart_wrapper);
    aspl_object_method_init("string", "trimEnd", aspl_method_string_trimEnd_wrapper);
    aspl_object_method_init("string", "split", aspl_method_string_split_wrapper);
    aspl_object_method_init("string", "reverse", aspl_method_string_reverse_wrapper);

    aspl_object_method_init("list", "contains", aspl_method_list_contains_wrapper);
    aspl_object_method_init("list", "add", aspl_method_list_add_wrapper);
    aspl_object_method_init("list", "insert", aspl_method_list_insert_wrapper);
    aspl_object_method_init("list", "insertElements", aspl_method_list_insertElements_wrapper);
    aspl_object_method_init("list", "remove", aspl_method_list_remove_wrapper);
    aspl_object_method_init("list", "removeAt", aspl_method_list_removeAt_wrapper);
    aspl_object_method_init("list", "clear", aspl_method_list_clear_wrapper);
    aspl_object_method_init("list", "join", aspl_method_list_join_wrapper);

    aspl_object_method_init("map", "containsKey", aspl_method_map_containsKey_wrapper);
    aspl_object_method_init("map", "containsValue", aspl_method_map_containsValue_wrapper);
    aspl_object_method_init("map", "remove", aspl_method_map_remove_wrapper);
    aspl_object_method_init("map", "clear", aspl_method_map_clear_wrapper);
}

#include <setjmp.h>

#define ASPL_MAX_ERROR_HANDLING_DEPTH 1028

_Thread_local jmp_buf aspl_error_handling_stack[ASPL_MAX_ERROR_HANDLING_DEPTH];
_Thread_local int aspl_error_handling_stack_index = -1;
_Thread_local ASPL_OBJECT_TYPE aspl_current_error;

int aspl_validate_error_handling_depth(int depth) {
    if (depth < 0 || depth > ASPL_MAX_ERROR_HANDLING_DEPTH) {
        ASPL_PANIC("Error handling depth out of bounds: %d\n", depth);
    }
    return 1;
}

#define try if (aspl_validate_error_handling_depth(++aspl_error_handling_stack_index) && !setjmp(aspl_error_handling_stack[aspl_error_handling_stack_index]))

#define catch else \
    if (aspl_validate_error_handling_depth(--aspl_error_handling_stack_index))

#define actually_throw(error) do { \
    if (aspl_validate_error_handling_depth(aspl_error_handling_stack_index)) { \
        aspl_current_error = error; \
        longjmp(aspl_error_handling_stack[aspl_error_handling_stack_index], 1); \
    } \
} while (0)

#define return_from_try(value) do { \
    aspl_validate_error_handling_depth(--aspl_error_handling_stack_index); \
    return (value); \
} while (0)

#define return_void_from_try do { \
    aspl_validate_error_handling_depth(--aspl_error_handling_stack_index); \
    return; \
} while (0)

#define return_uninitialized_from_try do { \
    aspl_validate_error_handling_depth(--aspl_error_handling_stack_index); \
    return ASPL_UNINITIALIZED; \
} while (0)

#define throw(value) do { \
    aspl_validate_error_handling_depth(--aspl_error_handling_stack_index); \
    return aspl_current_error = (value); \
} while (0)

#define escape(value) do { \
    aspl_current_error = (value); \
    longjmp(aspl_error_handling_stack[aspl_error_handling_stack_index], 1); \
} while (0)

ASPL_OBJECT_TYPE ASPL_UNWRAP_RESULT(ASPL_OBJECT_TYPE result) {
    if (aspl_object_is_error(result)) {
        actually_throw(result);
    }
    else {
        return result;
    }
}

#ifdef _WIN32
#if !defined(_WIN32_WINNT) || _WIN32_WINNT > 0x0600
// TODO: Is this really how it should be done?
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // ensure Windows Vista functions like CreateSymbolicLink are included
#endif
#include <windows.h>
#include <io.h>

unsigned int aspl_original_codepage = 0;

void aspl_restore_windows_console() {
    SetConsoleOutputCP(aspl_original_codepage);
}

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void aspl_setup_windows_console() {
    aspl_original_codepage = GetConsoleOutputCP();
    atexit(aspl_restore_windows_console);
    SetConsoleOutputCP(CP_UTF8);
    DWORD mode = 0;
    HANDLE osfh = (HANDLE)_get_osfhandle(1);
    GetConsoleMode(osfh, &mode);
    if (mode > 0) {
        SetConsoleMode(osfh, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        setbuf(stdout, NULL);
    }
    // TODO: stderr
}
#endif

void aspl_setup_gc() {
    GC_set_pages_executable(0);
    GC_INIT();
#ifndef ASPL_NO_MULTITHREADING
    GC_allow_register_threads();
#endif
}

#ifdef ASPL_USE_SSL
#ifdef __linux__
#include "thirdparty/ssl/linux/ssl.h"
#elif __APPLE__
#include "thirdparty/ssl/macos/ssl.h"
#else
#error SSL is not supported for this target OS
#endif

void aspl_setup_ssl() {
    ssl_init();
}
#endif

int aspl_argc;
char** aspl_argv;