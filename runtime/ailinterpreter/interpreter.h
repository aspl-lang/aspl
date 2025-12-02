#ifndef ASPL_AILI_INTERPRETER_H_INCLUDED
#define ASPL_AILI_INTERPRETER_H_INCLUDED

#define ASPL_INTERPRETER_MODE

#define ASPL_AILI_AIL_VERSION_MAJOR 1
#define ASPL_AILI_AIL_VERSION_MINOR 1

#include "byte_list.h"
#include "interpreter.h"

#ifndef ASPL_TEMPLATE_INCLUDED
#include "../../stdlib/aspl/compiler/backend/stringcode/c/template.c"
#define ASPL_TEMPLATE_INCLUDED
#endif

typedef struct {
    hashmap_str_to_voidptr_HashMap* functions;
    hashmap_str_to_voidptr_HashMap* builtin_functions;
    hashmap_str_to_voidptr_HashMap* custom_functions;
    hashmap_str_to_voidptr_HashMap* builtin_methods;
    hashmap_str_to_voidptr_HashMap* custom_methods;
    hashmap_str_to_voidptr_HashMap* classes;
    hashmap_str_to_voidptr_HashMap* enums;
} ASPL_AILI_EnvironmentContext;

typedef struct ASPL_AILI_StackFrame{
    ASPL_OBJECT_TYPE* data;
    int top;
} ASPL_AILI_StackFrame;

typedef struct ASPL_AILI_Stack {
    ASPL_AILI_StackFrame* frames;
    int frames_size;
    int current_frame_index;
} ASPL_AILI_Stack;

typedef enum ASPL_AILI_CallableType {
    ASPL_AILINTERPRETER_CALLABLE_TYPE_FUNCTION,
    ASPL_AILINTERPRETER_CALLABLE_TYPE_METHOD,
    ASPL_AILINTERPRETER_CALLABLE_TYPE_CALLBACK,
    ASPL_AILINTERPRETER_CALLABLE_TYPE_REACTIVE_PROPERTY,
    ASPL_AILINTERPRETER_CALLABLE_TYPE_CATCH_BLOCK
} ASPL_AILI_CallableType;

typedef struct ASPL_AILI_LoopMeta {
    long long body_end_address;
    long long full_end_address;
} ASPL_AILI_LoopMeta;

typedef struct ASPL_AILI_LoopMetaStack {
    ASPL_AILI_LoopMeta* data;
    int top;
} ASPL_AILI_LoopMetaStack;

typedef struct ASPL_AILI_ClassStack {
    char** data;
    int top;
} ASPL_AILI_ClassStack;

typedef struct ASPL_AILI_CallableInvocationMeta {
    ASPL_AILI_CallableType type;
    long long previous_address;
    hashmap_str_to_voidptr_HashMap** previous_scopes;
    int previous_scopes_top;
    ASPL_OBJECT_TYPE* previous_instance;
    ASPL_AILI_LoopMetaStack* previous_loop_meta_stack;
    char is_constructor;
    char is_reactive_property_setter;
} ASPL_AILI_CallableInvocationMeta;

typedef struct ASPL_AILI_CallableInvocationMetaStack {
    ASPL_AILI_CallableInvocationMeta* data;
    int top;
} ASPL_AILI_CallableInvocationMetaStack;

typedef struct ASPL_AILI_TypeList {
    char** types;
    int size;
} ASPL_AILI_TypeList;

typedef struct ASPL_AILI_Parameter {
    char* name;
    ASPL_AILI_TypeList expected_types;
    int optional;
    ASPL_OBJECT_TYPE default_value;
} ASPL_AILI_Parameter;

typedef struct ASPL_AILI_ParameterList {
    ASPL_AILI_Parameter* parameters;
    int size;
} ASPL_AILI_ParameterList;

typedef struct ASPL_AILI_BuiltinFunction {
    ASPL_AILI_ParameterList parameters;
} ASPL_AILI_BuiltinFunction;

typedef struct ASPL_AILI_CustomFunction {
    int address;
    ASPL_AILI_ParameterList parameters;
} ASPL_AILI_CustomFunction;

typedef struct ASPL_AILI_BuiltinMethod {
    // static builtin methods are not supported yet in ASPL
    ASPL_AILI_ParameterList parameters;
} ASPL_AILI_BuiltinMethod;

typedef struct ASPL_AILI_CustomMethod {
    int address;
    char is_static;
    ASPL_AILI_ParameterList parameters;
} ASPL_AILI_CustomMethod;

typedef struct ASPL_AILI_ReactiveProperty {
    int get_address;
    int set_address;
} ASPL_AILI_ReactiveProperty;

typedef struct ASPL_AILI_Class {
    char is_static;
    char** parents;
    int parents_size;
    char is_error;
    hashmap_str_to_voidptr_HashMap* default_threadlocal_static_property_values;
    hashmap_str_to_voidptr_HashMap* static_property_values;
    hashmap_str_to_voidptr_HashMap* reactive_properties;
} ASPL_AILI_Class;

typedef struct ASPL_AILI_MemoryAddress {
    ASPL_OBJECT_TYPE object;
} ASPL_AILI_MemoryAddress;

typedef struct ASPL_AILI_StackTrace {
    char** frames;
    int size;
    int top;
} ASPL_AILI_StackTrace;

typedef struct ASPL_AILI_ThreadContext {
    ASPL_AILI_EnvironmentContext* environment_context;
    ASPL_AILI_Stack* stack;
    ASPL_AILI_CallableInvocationMetaStack* callable_invocation_meta_stack;
    ASPL_AILI_LoopMetaStack* loop_meta_stack;
    ASPL_AILI_ClassStack* class_stack;
    hashmap_str_to_voidptr_HashMap** scopes;
    int scopes_size;
    int scopes_top;
    char* current_class; // only used for declaring properties and methods
    ASPL_OBJECT_TYPE current_instance;
    hashmap_str_to_voidptr_HashMap* local_static_property_values;
    ASPL_AILI_StackTrace* stack_trace;
} ASPL_AILI_ThreadContext;

typedef struct ASPL_AILI_ArgumentList {
    ASPL_OBJECT_TYPE* arguments;
    int size;
} ASPL_AILI_ArgumentList;

typedef void(*ASPL_AILI_FunctionPtr)(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments);

typedef struct ASPL_AILI_ThreadFunctionWrapperData {
    ASPL_AILI_FunctionPtr function;
    ASPL_AILI_ThreadContext* context;
    ASPL_AILI_ByteList* bytes;
    char* identifier;
    ASPL_AILI_ArgumentList arguments;
} ASPL_AILI_ThreadFunctionWrapperData;

typedef struct ASPL_AILI_ThreadMethodWrapperData {
    ASPL_AILI_CustomMethod* method;
    ASPL_AILI_ThreadContext* context;
    ASPL_AILI_ByteList* bytes;
    char* identifier;
    ASPL_AILI_ArgumentList arguments;
} ASPL_AILI_ThreadMethodWrapperData;

typedef struct ASPL_AILI_ThreadCallbackWrapperData {
    ASPL_Callback* callback;
    ASPL_AILI_ThreadContext* context;
    ASPL_AILI_ByteList* bytes;
    char* identifier;
    ASPL_AILI_ArgumentList arguments;
} ASPL_AILI_ThreadCallbackWrapperData;

typedef struct ASPL_AILI_CallbackData {
    long long address;
    hashmap_str_to_voidptr_HashMap** creation_scopes;
    int creation_scopes_top;
    ASPL_AILI_ParameterList parameters;
} ASPL_AILI_CallbackData;

ASPL_AILI_EnvironmentContext* aspl_ailinterpreter_new_environment_context();

ASPL_AILI_ThreadContext* aspl_ailinterpreter_new_thread_context(ASPL_AILI_EnvironmentContext* environment_context);

void aspl_ailinterpreter_stack_push_frame(ASPL_AILI_Stack* stack);

void aspl_ailinterpreter_stack_push(ASPL_AILI_Stack* stack, ASPL_OBJECT_TYPE object);

ASPL_AILI_StackFrame aspl_ailinterpreter_stack_pop_frame(ASPL_AILI_Stack* stack);

ASPL_OBJECT_TYPE aspl_ailinterpreter_stack_pop(ASPL_AILI_Stack* stack);

ASPL_OBJECT_TYPE aspl_ailinterpreter_stack_peek(ASPL_AILI_Stack* stack);

void aspl_ailinterpreter_cims_push(ASPL_AILI_CallableInvocationMetaStack* stack, ASPL_AILI_CallableInvocationMeta value);

ASPL_AILI_CallableInvocationMeta aspl_ailinterpreter_cims_pop(ASPL_AILI_CallableInvocationMetaStack* stack);

ASPL_AILI_CallableInvocationMeta aspl_ailinterpreter_cims_peek(ASPL_AILI_CallableInvocationMetaStack* stack);

void aspl_ailinterpreter_lms_push(ASPL_AILI_LoopMetaStack* stack, ASPL_AILI_LoopMeta value);

ASPL_AILI_LoopMeta aspl_ailinterpreter_lms_pop(ASPL_AILI_LoopMetaStack* stack);

ASPL_AILI_LoopMeta aspl_ailinterpreter_lms_peek(ASPL_AILI_LoopMetaStack* stack);

void aspl_ailinterpreter_class_stack_push(ASPL_AILI_ClassStack* stack, char* object);

char* aspl_ailinterpreter_class_stack_pop(ASPL_AILI_ClassStack* stack);

void aspl_ailinterpreter_push_scope(ASPL_AILI_ThreadContext* context);

void aspl_ailinterpreter_pop_scope(ASPL_AILI_ThreadContext* context);

void aspl_ailinterpreter_register_variable(ASPL_AILI_ThreadContext* context, char* identifier, ASPL_OBJECT_TYPE value);

ASPL_OBJECT_TYPE* aspl_ailinterpreter_access_variable_address(ASPL_AILI_ThreadContext* context, char* identifier);

ASPL_OBJECT_TYPE aspl_ailinterpreter_access_variable(ASPL_AILI_ThreadContext* context, char* identifier);

void aspl_ailinterpreter_register_class(ASPL_AILI_EnvironmentContext* context, char* identifier, ASPL_AILI_Class* class);

ASPL_AILI_Class* aspl_ailinterpreter_access_class(ASPL_AILI_EnvironmentContext* context, char* identifier);

ASPL_AILI_TypeList aspl_ailinterpreter_parse_types(ASPL_AILI_ByteList* bytes);

ASPL_AILI_ParameterList aspl_ailinterpreter_parse_parameters(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes);

char* aspl_ailinterpreter_construct_list_from_type_list(ASPL_AILI_TypeList type_list);

char* aspl_ailinterpreter_construct_map_from_type_lists(ASPL_AILI_TypeList key_type_list, ASPL_AILI_TypeList value_type_list);

void aspl_ailinterpreter_return(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, ASPL_OBJECT_TYPE value);

ASPL_AILI_FunctionPtr aspl_ailinterpreter_get_function(ASPL_AILI_ThreadContext* context, char* identifier);

void aspl_ailinterpreter_custom_function_callback(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments);

ASPL_AILI_CustomMethod* aspl_ailinterpreter_util_find_custom_method(ASPL_AILI_ThreadContext* context, char* type, char* name);

ASPL_AILI_ReactiveProperty* aspl_ailinterpreter_util_find_reactive_property(ASPL_AILI_ThreadContext* context, char* type, char* name);

void aspl_ailinterpreter_display_stack_trace(ASPL_AILI_ThreadContext* context);

void aspl_ailinterpreter_invoke_callback_from_outside_of_loop_internal(ASPL_Callback* callback, ASPL_AILI_ArgumentList arguments, ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes);

void aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_Callback* callback, ASPL_AILI_ArgumentList arguments);

int aspl_ailinterpreter_new_thread_function_invocation_callback(void* arguments);

void aspl_ailinterpreter_loop(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes);

#endif