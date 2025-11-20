#include "interpreter.h"

#include "byte_list.c"
#include "builtins.c"
#include "implementations.c"

ASPL_AILI_EnvironmentContext* aspl_ailinterpreter_new_environment_context() {
    ASPL_AILI_EnvironmentContext* environment_context = ASPL_MALLOC(sizeof(ASPL_AILI_EnvironmentContext));

    environment_context->functions = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });
    environment_context->builtin_functions = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 8 });
    aspl_ailinterpreter_initialize_builtin_functions(environment_context);
    environment_context->custom_functions = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });

    environment_context->builtin_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 8 });
    aspl_ailinterpreter_initialize_builtin_methods(environment_context);
    environment_context->custom_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });

    environment_context->classes = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });
    environment_context->enums = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });

    return environment_context;
}

ASPL_AILI_ThreadContext* aspl_ailinterpreter_new_thread_context(ASPL_AILI_EnvironmentContext* environment_context) {
    ASPL_AILI_ThreadContext* thread_context = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadContext));
    thread_context->environment_context = environment_context;

    thread_context->stack = ASPL_MALLOC(sizeof(ASPL_AILI_Stack));
    thread_context->stack->frames = ASPL_MALLOC(sizeof(ASPL_AILI_StackFrame) * 1024);
    thread_context->stack->frames_size = 1024;
    thread_context->stack->current_frame_index = -1; // push_frame will increment this to 0
    aspl_ailinterpreter_stack_push_frame(thread_context->stack);

    thread_context->callable_invocation_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_CallableInvocationMetaStack));
    thread_context->callable_invocation_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_CallableInvocationMeta) * 1024);
    thread_context->callable_invocation_meta_stack->top = 0;

    thread_context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
    thread_context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
    thread_context->loop_meta_stack->top = 0;

    thread_context->class_stack = ASPL_MALLOC(sizeof(ASPL_AILI_ClassStack));
    thread_context->class_stack->data = ASPL_MALLOC(sizeof(char*) * 8);
    thread_context->class_stack->top = 0;

    thread_context->scopes_top = -1; // push_scope will increment this to 0
    thread_context->scopes_size = 128;
    thread_context->scopes = ASPL_MALLOC(sizeof(hashmap_str_to_voidptr_HashMap*) * thread_context->scopes_size);
    aspl_ailinterpreter_push_scope(thread_context);

    thread_context->current_instance = ASPL_NULL();

    thread_context->local_static_property_values = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });
    for (int i = 0; i < environment_context->classes->len; i++) {
        char* class_name = environment_context->classes->pairs[i]->key;
        hashmap_str_to_voidptr_hashmap_set(thread_context->local_static_property_values, class_name, hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 }));
        ASPL_AILI_Class* class = environment_context->classes->pairs[i]->value;
        for (int j = 0; j < class->default_threadlocal_static_property_values->len; j++) {
            char* property_name = class->default_threadlocal_static_property_values->pairs[j]->key;
            ASPL_OBJECT_TYPE* property_value = class->default_threadlocal_static_property_values->pairs[j]->value;
            hashmap_str_to_voidptr_hashmap_set(hashmap_str_to_voidptr_hashmap_get_value(thread_context->local_static_property_values, class_name), property_name, property_value);
        }
    }

    thread_context->stack_trace = ASPL_MALLOC(sizeof(ASPL_AILI_StackTrace));
    thread_context->stack_trace->frames = ASPL_MALLOC(sizeof(char*) * 1024);
    thread_context->stack_trace->size = 1024;
    thread_context->stack_trace->top = 0;

    return thread_context;
}

void aspl_ailinterpreter_stack_push_frame(ASPL_AILI_Stack* stack) {
    if (stack->current_frame_index >= (stack->frames_size - 1)) {
        stack->frames_size *= 2;
        stack->frames = ASPL_REALLOC(stack->frames, sizeof(ASPL_AILI_StackFrame*) * stack->frames_size);
    }
    stack->frames[++stack->current_frame_index] = (ASPL_AILI_StackFrame){ .data = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 64), .top = 0 };
}

void aspl_ailinterpreter_stack_push(ASPL_AILI_Stack* stack, ASPL_OBJECT_TYPE object) {
    stack->frames[stack->current_frame_index].data[stack->frames[stack->current_frame_index].top++] = object;
}

ASPL_AILI_StackFrame aspl_ailinterpreter_stack_pop_frame(ASPL_AILI_Stack* stack) {
    if (stack->current_frame_index <= 0) ASPL_PANIC("Cannot pop frame from empty stack");
    return stack->frames[--stack->current_frame_index];
}

ASPL_OBJECT_TYPE aspl_ailinterpreter_stack_pop(ASPL_AILI_Stack* stack) {
    if (stack->frames[stack->current_frame_index].top <= 0) ASPL_PANIC("Cannot pop from empty stack frame");
    return stack->frames[stack->current_frame_index].data[--stack->frames[stack->current_frame_index].top];
}

ASPL_OBJECT_TYPE aspl_ailinterpreter_stack_peek(ASPL_AILI_Stack* stack) {
    if (stack->frames[stack->current_frame_index].top <= 0) ASPL_PANIC("Cannot peek from empty stack frame");
    return stack->frames[stack->current_frame_index].data[stack->frames[stack->current_frame_index].top - 1];
}

void aspl_ailinterpreter_cims_push(ASPL_AILI_CallableInvocationMetaStack* stack, ASPL_AILI_CallableInvocationMeta value) {
    stack->data[stack->top++] = value;
}

ASPL_AILI_CallableInvocationMeta aspl_ailinterpreter_cims_pop(ASPL_AILI_CallableInvocationMetaStack* stack) {
    if (stack->top <= 0) ASPL_PANIC("Cannot pop from empty callable invocation meta stack");
    return stack->data[--stack->top];
}

ASPL_AILI_CallableInvocationMeta aspl_ailinterpreter_cims_peek(ASPL_AILI_CallableInvocationMetaStack* stack) {
    if (stack->top <= 0) ASPL_PANIC("Cannot peek from empty callable invocation meta stack");
    return stack->data[stack->top - 1];
}

void aspl_ailinterpreter_lms_push(ASPL_AILI_LoopMetaStack* stack, ASPL_AILI_LoopMeta value) {
    stack->data[stack->top++] = value;
}

ASPL_AILI_LoopMeta aspl_ailinterpreter_lms_pop(ASPL_AILI_LoopMetaStack* stack) {
    if (stack->top <= 0) ASPL_PANIC("Cannot pop from empty loop meta stack");
    return stack->data[--stack->top];
}

ASPL_AILI_LoopMeta aspl_ailinterpreter_lms_peek(ASPL_AILI_LoopMetaStack* stack) {
    if (stack->top <= 0) ASPL_PANIC("Cannot peek from empty loop meta stack");
    return stack->data[stack->top - 1];
}

void aspl_ailinterpreter_class_stack_push(ASPL_AILI_ClassStack* stack, char* object) {
    stack->data[stack->top++] = object;
}

char* aspl_ailinterpreter_class_stack_pop(ASPL_AILI_ClassStack* stack) {
    if (stack->top <= 0) ASPL_PANIC("Cannot pop from empty class stack");
    return stack->data[--stack->top];
}

void aspl_ailinterpreter_push_scope(ASPL_AILI_ThreadContext* context) {
    if (context->scopes_top >= (context->scopes_size - 1)) {
        context->scopes_size *= 2;
        context->scopes = ASPL_REALLOC(context->scopes, sizeof(hashmap_str_to_voidptr_HashMap*) * context->scopes_size);
    }
    context->scopes[++context->scopes_top] = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 64 });
}

void aspl_ailinterpreter_pop_scope(ASPL_AILI_ThreadContext* context) {
    context->scopes_top--;
}

void aspl_ailinterpreter_register_variable(ASPL_AILI_ThreadContext* context, char* identifier, ASPL_OBJECT_TYPE value) {
    hashmap_str_to_voidptr_hashmap_set(context->scopes[context->scopes_top], identifier, C_REFERENCE(value));
}

ASPL_OBJECT_TYPE* aspl_ailinterpreter_access_variable_address(ASPL_AILI_ThreadContext* context, char* identifier) {
    for (int i = context->scopes_top; i >= 0; i--) {
        if (hashmap_str_to_voidptr_hashmap_contains_key(context->scopes[i], identifier)) {
            return hashmap_str_to_voidptr_hashmap_get_value(context->scopes[i], identifier);
        }
    }
    ASPL_PANIC("Cannot access unregistered variable '%s'", identifier);
}

ASPL_OBJECT_TYPE aspl_ailinterpreter_access_variable(ASPL_AILI_ThreadContext* context, char* identifier) {
    return *aspl_ailinterpreter_access_variable_address(context, identifier);
}

void aspl_ailinterpreter_register_class(ASPL_AILI_EnvironmentContext* context, char* identifier, ASPL_AILI_Class* class) {
    hashmap_str_to_voidptr_hashmap_set(context->classes, identifier, class);
}

ASPL_AILI_Class* aspl_ailinterpreter_access_class(ASPL_AILI_EnvironmentContext* context, char* identifier) {
    return hashmap_str_to_voidptr_hashmap_get_value(context->classes, identifier);
}

void aspl_ailinterpreter_stack_trace_push(ASPL_AILI_ThreadContext* context, char* identifier) {
    if (context->stack_trace->top >= (context->stack_trace->size - 1)) {
        context->stack_trace->size *= 2;
        context->stack_trace->frames = ASPL_REALLOC(context->stack_trace->frames, sizeof(char*) * context->stack_trace->size);
    }
    context->stack_trace->frames[context->stack_trace->top++] = identifier;
}

char* aspl_ailinterpreter_stack_trace_pop(ASPL_AILI_ThreadContext* context) {
    if (context->stack_trace->top <= 0) ASPL_PANIC("Cannot pop from empty stack trace");
    return context->stack_trace->frames[--context->stack_trace->top];
}

ASPL_AILI_TypeList aspl_ailinterpreter_parse_types(ASPL_AILI_ByteList* bytes) {
    char** type_list = ASPL_MALLOC(sizeof(char*) * 8);
    int type_list_size = 8;
    int type_list_top = 0;
    int type_list_length = aspl_ailinterpreter_read_short(bytes);
    while (type_list_top < type_list_length) {
        while (type_list_top >= type_list_size) {
            type_list_size *= 2;
            type_list = ASPL_REALLOC(type_list, sizeof(char*) * type_list_size);
        }
        type_list[type_list_top++] = aspl_ailinterpreter_read_short_string(bytes);
    }
    if (type_list_top == 0) {
        type_list[type_list_top++] = "any";
    }
    return (ASPL_AILI_TypeList) { .types = type_list, .size = type_list_top };
}

ASPL_AILI_ParameterList aspl_ailinterpreter_parse_parameters(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes) {
    ASPL_AILI_Parameter* parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 8);
    int parameters_size = 8;
    int parameters_top = 0;
    int parameters_length = aspl_ailinterpreter_read_short(bytes);
    ASPL_OBJECT_TYPE* default_values = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 8);
    int default_values_size = 8;
    int default_values_top = 0;
    while (parameters_top < parameters_length) {
        while (parameters_top >= parameters_size) {
            parameters_size *= 2;
            parameters = ASPL_REALLOC(parameters, sizeof(ASPL_AILI_Parameter) * parameters_size);
        }
        char* name = aspl_ailinterpreter_read_short_string(bytes);
        ASPL_AILI_TypeList expected_types = aspl_ailinterpreter_parse_types(bytes);
        int optional = aspl_ailinterpreter_read_bool(bytes);
        if (optional) {
            ASPL_OBJECT_TYPE default_value = aspl_ailinterpreter_stack_pop(context->stack);
            while (default_values_top >= default_values_size) {
                default_values_size *= 2;
                default_values = ASPL_REALLOC(default_values, sizeof(ASPL_OBJECT_TYPE) * default_values_size);
            }
            default_values[default_values_top++] = default_value;
            parameters[parameters_top++] = (ASPL_AILI_Parameter){ .name = name, .expected_types = expected_types, .optional = optional };
        }
        else {
            parameters[parameters_top++] = (ASPL_AILI_Parameter){ .name = name, .expected_types = expected_types, .optional = optional };
        }
    }
    for (int i = 0; i < parameters_top; i++) {
        if (parameters[i].optional) {
            parameters[i].default_value = default_values[--default_values_top];
        }
    }
    return (ASPL_AILI_ParameterList) { .parameters = parameters, .size = parameters_top };
}

char* aspl_ailinterpreter_construct_list_from_type_list(ASPL_AILI_TypeList type_list) {
    int length = 5; // "list<"
    for (int i = 0; i < type_list.size; i++) {
        length += strlen(type_list.types[i]);
        if (i < type_list.size - 1) {
            length += 1; // "|"
        }
    }
    length += 1; // ">"
    char* result = ASPL_MALLOC(sizeof(char) * (length + 1));
    result[0] = 'l';
    result[1] = 'i';
    result[2] = 's';
    result[3] = 't';
    result[4] = '<';
    int pos = 5;
    for (int i = 0; i < type_list.size; i++) {
        for (int j = 0; j < strlen(type_list.types[i]); j++) {
            result[pos++] = type_list.types[i][j];
        }
        if (i < type_list.size - 1) {
            result[pos++] = '|';
        }
    }
    result[pos++] = '>';
    result[pos] = '\0';
    return result;
}

char* aspl_ailinterpreter_construct_map_from_type_lists(ASPL_AILI_TypeList key_type_list, ASPL_AILI_TypeList value_type_list) {
    int length = 4; // "map<"
    for (int i = 0; i < key_type_list.size; i++) {
        length += strlen(key_type_list.types[i]);
        if (i < key_type_list.size - 1) {
            length += 1; // "|"
        }
    }
    length += 2; // ", "
    for (int i = 0; i < value_type_list.size; i++) {
        length += strlen(value_type_list.types[i]);
        if (i < value_type_list.size - 1) {
            length += 1; // "|"
        }
    }
    length += 1; // ">"
    char* result = ASPL_MALLOC(sizeof(char) * (length + 1));
    result[0] = 'm';
    result[1] = 'a';
    result[2] = 'p';
    result[3] = '<';
    int pos = 4;
    for (int i = 0; i < key_type_list.size; i++) {
        for (int j = 0; j < strlen(key_type_list.types[i]); j++) {
            result[pos++] = key_type_list.types[i][j];
        }
        if (i < key_type_list.size - 1) {
            result[pos++] = '|';
        }
    }
    result[pos++] = ',';
    result[pos++] = ' ';
    for (int i = 0; i < value_type_list.size; i++) {
        for (int j = 0; j < strlen(value_type_list.types[i]); j++) {
            result[pos++] = value_type_list.types[i][j];
        }
        if (i < value_type_list.size - 1) {
            result[pos++] = '|';
        }
    }
    result[pos++] = '>';
    result[pos] = '\0';
    return result;
}

void aspl_ailinterpreter_return(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, ASPL_OBJECT_TYPE value) {
#ifdef ASPL_INTERPRETER_FIND_STACK_LEAKS
    if (context->stack->frames[context->stack->current_frame_index].top != 0) {
        ASPL_PANIC("Leaked stack object(s) found!\n");
    }
#endif
    aspl_ailinterpreter_stack_pop_frame(context->stack);
    ASPL_AILI_CallableInvocationMeta meta = aspl_ailinterpreter_cims_pop(context->callable_invocation_meta_stack);
    if (meta.is_reactive_property_setter) {
        aspl_ailinterpreter_stack_push(context->stack, aspl_ailinterpreter_access_variable(context, "value"));
    }
    else if (!meta.is_constructor) {
        aspl_ailinterpreter_stack_push(context->stack, value);
    }
    if (meta.type == ASPL_AILINTERPRETER_CALLABLE_TYPE_CALLBACK) {
        context->scopes = meta.previous_scopes;
        context->scopes_top = meta.previous_scopes_top;
    }
    else {
        aspl_ailinterpreter_pop_scope(context);
    }
    if (meta.previous_instance != NULL) {
        context->current_instance = *meta.previous_instance;
    }
    context->loop_meta_stack = meta.previous_loop_meta_stack;
    aspl_ailinterpreter_stack_trace_pop(context);
    bytes->position = meta.previous_address;
}

ASPL_AILI_FunctionPtr aspl_ailinterpreter_get_function(ASPL_AILI_ThreadContext* context, char* identifier) {
    return hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->functions, identifier);
}

void aspl_ailinterpreter_custom_function_callback(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_AILI_CustomFunction* custom_function = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_functions, identifier);
    bytes->position = custom_function->address;
    for (int i = 0; i < custom_function->parameters.size; i++) {
        if (i < arguments.size) {
            aspl_ailinterpreter_register_variable(context, custom_function->parameters.parameters[i].name, arguments.arguments[i]);
        }
        else if (custom_function->parameters.parameters[i].optional) {
            aspl_ailinterpreter_register_variable(context, custom_function->parameters.parameters[i].name, aspl_object_clone_shallow(custom_function->parameters.parameters[i].default_value));
        }
        else {
            ASPL_PANIC("Missing argument for parameter '%s'", custom_function->parameters.parameters[i].name);
        }
    }
}

ASPL_AILI_CustomMethod* aspl_ailinterpreter_util_find_custom_method(ASPL_AILI_ThreadContext* context, char* type, char* name) {
    if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->custom_methods, type)) {
        hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, type);
        if (hashmap_str_to_voidptr_hashmap_contains_key(methods, name)) {
            return hashmap_str_to_voidptr_hashmap_get_value(methods, name);
        }
    }
    ASPL_AILI_Class* class = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->classes, type);
    for (int i = 0; i < class->parents_size; i++) {
        ASPL_AILI_CustomMethod* method = aspl_ailinterpreter_util_find_custom_method(context, class->parents[i], name);
        if (method != NULL) {
            return method;
        }
    }
    return NULL;
}

ASPL_AILI_ReactiveProperty* aspl_ailinterpreter_util_find_reactive_property(ASPL_AILI_ThreadContext* context, char* type, char* name) {
    if (hashmap_str_to_voidptr_hashmap_contains_key(aspl_ailinterpreter_access_class(context->environment_context, type)->reactive_properties, name)) {
        return hashmap_str_to_voidptr_hashmap_get_value(aspl_ailinterpreter_access_class(context->environment_context, type)->reactive_properties, name);
    }
    ASPL_AILI_Class* class = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->classes, type);
    for (int i = 0; i < class->parents_size; i++) {
        ASPL_AILI_ReactiveProperty* property = aspl_ailinterpreter_util_find_reactive_property(context, class->parents[i], name);
        if (property != NULL) {
            return property;
        }
    }
    return NULL;
}

char aspl_ailinterpreter_util_is_class_child_of(ASPL_AILI_ThreadContext* context, char* child, char* parent) {
    if (strcmp(child, parent) == 0) {
        return 1;
    }
    ASPL_AILI_Class* class = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->classes, child);
    for (int i = 0; i < class->parents_size; i++) {
        if (aspl_ailinterpreter_util_is_class_child_of(context, class->parents[i], parent)) {
            return 1;
        }
    }
    return 0;
}

void aspl_ailinterpreter_display_stack_trace(ASPL_AILI_ThreadContext* context) {
    printf("Stack trace (most recent call first):\n");
    for (int i = context->stack_trace->top - 1; i >= 0; i--) {
        if (i == context->stack_trace->top - 1) {
            printf("   > %s\n", context->stack_trace->frames[i]);
        }
        else {
            printf("     %s\n", context->stack_trace->frames[i]);
        }
    }
}

_Thread_local ASPL_AILI_ThreadContext* aspl_ailinterpreter_current_thread_context;
_Thread_local ASPL_AILI_ByteList* aspl_ailinterpreter_current_byte_list;

void aspl_ailinterpreter_invoke_callback_from_outside_of_loop_internal(ASPL_Callback* callback, ASPL_AILI_ArgumentList arguments, ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes) {
    char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(callback->typePtr) + strlen("invoke") + 2));
    strcpy(identifier, callback->typePtr);
    strcat(identifier, ".");
    strcat(identifier, "invoke");
    aspl_ailinterpreter_stack_trace_push(context, identifier);
    aspl_ailinterpreter_stack_push_frame(context->stack);
    ASPL_AILI_CallbackData* callback_data = (ASPL_AILI_CallbackData*)callback->function;
    ASPL_AILI_CallableInvocationMetaStack* original_callable_invocation_meta_stack = context->callable_invocation_meta_stack;
    ASPL_AILI_CallableInvocationMetaStack* callable_invocation_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_CallableInvocationMetaStack));
    callable_invocation_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_CallableInvocationMeta) * 1024);
    callable_invocation_meta_stack->top = 0;
    context->callable_invocation_meta_stack = callable_invocation_meta_stack;
    hashmap_str_to_voidptr_HashMap** original_scopes = context->scopes;
    int original_scopes_top = context->scopes_top;
    context->scopes = callback_data->creation_scopes;
    context->scopes_top = callback_data->creation_scopes_top;
    int original_position = bytes->position;
    bytes->position = callback_data->address;
    for (int i = 0; i < callback_data->parameters.size; i++) {
        if (i < arguments.size) {
            aspl_ailinterpreter_register_variable(context, callback_data->parameters.parameters[i].name, arguments.arguments[i]);
        }
        else if (callback_data->parameters.parameters[i].optional) {
            aspl_ailinterpreter_register_variable(context, callback_data->parameters.parameters[i].name, aspl_object_clone_shallow(callback_data->parameters.parameters[i].default_value));
        }
        else {
            ASPL_PANIC("Missing argument for parameter '%s'", callback_data->parameters.parameters[i].name);
        }
    }
    aspl_ailinterpreter_loop(context, bytes);
    bytes->position = original_position;
    context->scopes = original_scopes;
    context->scopes_top = original_scopes_top;
    context->callable_invocation_meta_stack = original_callable_invocation_meta_stack;
    aspl_ailinterpreter_stack_trace_pop(context);
    aspl_ailinterpreter_stack_pop_frame(context->stack);
}

void aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_Callback* callback, ASPL_AILI_ArgumentList arguments) {
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop_internal(callback, arguments, aspl_ailinterpreter_current_thread_context, aspl_ailinterpreter_current_byte_list);
}

int aspl_ailinterpreter_new_thread_function_invocation_callback(void* arguments) {
    struct GC_stack_base sb;
    GC_get_stack_base(&sb);
    GC_register_my_thread(&sb);

    ASPL_AILI_ThreadFunctionWrapperData* data = (ASPL_AILI_ThreadFunctionWrapperData*)arguments;

    aspl_ailinterpreter_current_thread_context = data->context;
    aspl_ailinterpreter_current_byte_list = data->bytes;

    aspl_ailinterpreter_stack_trace_push(data->context, data->identifier);
    aspl_ailinterpreter_stack_push_frame(data->context->stack);
    data->function(data->context, data->bytes, data->identifier, data->arguments);
    aspl_ailinterpreter_loop(data->context, data->bytes);

    GC_unregister_my_thread();
    return 0;
}

int aspl_ailinterpreter_new_thread_method_invocation_callback(void* arguments) {
    struct GC_stack_base sb;
    GC_get_stack_base(&sb);
    GC_register_my_thread(&sb);

    ASPL_AILI_ThreadMethodWrapperData* data = (ASPL_AILI_ThreadMethodWrapperData*)arguments;

    aspl_ailinterpreter_current_thread_context = data->context;
    aspl_ailinterpreter_current_byte_list = data->bytes;

    aspl_ailinterpreter_stack_trace_push(data->context, data->identifier);
    aspl_ailinterpreter_stack_push_frame(data->context->stack);
    ASPL_AILI_CustomMethod* custom_method = data->method;
    data->bytes->position = custom_method->address;
    for (int i = 0; i < custom_method->parameters.size; i++) {
        if (i < data->arguments.size) {
            aspl_ailinterpreter_register_variable(data->context, custom_method->parameters.parameters[i].name, data->arguments.arguments[i]);
        }
        else if (custom_method->parameters.parameters[i].optional) {
            aspl_ailinterpreter_register_variable(data->context, custom_method->parameters.parameters[i].name, aspl_object_clone_shallow(custom_method->parameters.parameters[i].default_value));
        }
        else {
            ASPL_PANIC("Missing argument for parameter '%s'", custom_method->parameters.parameters[i].name);
        }
    }
    aspl_ailinterpreter_loop(data->context, data->bytes);

    GC_unregister_my_thread();
    return 0;
}

int aspl_ailinterpreter_new_thread_callback_invocation_callback(void* arguments) {
    struct GC_stack_base sb;
    GC_get_stack_base(&sb);
    GC_register_my_thread(&sb);

    ASPL_AILI_ThreadCallbackWrapperData* data = (ASPL_AILI_ThreadCallbackWrapperData*)arguments;

    aspl_ailinterpreter_current_thread_context = data->context;
    aspl_ailinterpreter_current_byte_list = data->bytes;

    aspl_ailinterpreter_invoke_callback_from_outside_of_loop_internal(data->callback, data->arguments, data->context, data->bytes);

    GC_unregister_my_thread();
    return 0;
}

void aspl_ailinterpreter_loop(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes) {
    while (1) {
        ASPL_AILI_Instruction instruction = aspl_ailinterpreter_fetch_instruction(bytes);
        switch (instruction) {
        case ASPL_AILI_INSTRUCTION_MANIFEST: {
            int length = aspl_ailinterpreter_read_int(bytes);
            // Ignore the manifest for now...
            bytes->position += length;
            break;
        }
        case ASPL_AILI_INSTRUCTION_CREATE_OBJECT: {
            char* type = aspl_ailinterpreter_read_short_string(bytes);
            if (strcmp(type, "null") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
            }
            else if (strcmp(type, "bool") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BOOL_LITERAL(aspl_ailinterpreter_read_byte(bytes)));
            }
            else if (strcmp(type, "byte") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(aspl_ailinterpreter_read_byte(bytes)));
            }
            else if (strcmp(type, "int") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(aspl_ailinterpreter_read_int(bytes)));
            }
            else if (strcmp(type, "long") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(aspl_ailinterpreter_read_long(bytes)));
            }
            else if (strcmp(type, "float") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_FLOAT_LITERAL(aspl_ailinterpreter_read_float(bytes)));
            }
            else if (strcmp(type, "double") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_DOUBLE_LITERAL(aspl_ailinterpreter_read_double(bytes)));
            }
            else if (strcmp(type, "string") == 0) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_STRING_LITERAL(aspl_ailinterpreter_read_long_string(bytes)));
            }
            else if (strcmp(type, "list") == 0) {
                ASPL_AILI_TypeList generic_types = aspl_ailinterpreter_parse_types(bytes);
                char* type = aspl_ailinterpreter_construct_list_from_type_list(generic_types);
                int length = aspl_ailinterpreter_read_int(bytes);
                ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * length);
                for (int i = length - 1; i >= 0; i--) {
                    array[i] = aspl_ailinterpreter_stack_pop(context->stack);
                }
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_LITERAL(type, strlen(type), array, length));
            }
            else if (strcmp(type, "map") == 0) {
                ASPL_AILI_TypeList generic_key_types = aspl_ailinterpreter_parse_types(bytes);
                ASPL_AILI_TypeList generic_value_types = aspl_ailinterpreter_parse_types(bytes);
                char* type = aspl_ailinterpreter_construct_map_from_type_lists(generic_key_types, generic_value_types);
                int length = aspl_ailinterpreter_read_int(bytes);
                int initial_capacity = length;
                if (initial_capacity < 1) initial_capacity = 1; // hashmap_new can't handle 0 as initial capacity
                hashmap_aspl_object_to_aspl_object_HashMap* hashmap = hashmap_aspl_object_to_aspl_object_new_hashmap((hashmap_aspl_object_to_aspl_object_HashMapConfig) { .initial_capacity = initial_capacity });
                for (int i = 0; i < length; i++) {
                    ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
                    ASPL_OBJECT_TYPE key = aspl_ailinterpreter_stack_pop(context->stack);
                    hashmap_aspl_object_to_aspl_object_hashmap_set(hashmap, ASPL_HASHMAP_WRAP(key), ASPL_HASHMAP_WRAP(value));
                }
                hashmap = hashmap_aspl_object_to_aspl_object_hashmap_reverse(hashmap);
                ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
                ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_MAP;
                ASPL_Map* m = ASPL_MALLOC(sizeof(ASPL_Map));
                m->typePtr = type;
                m->typeLen = strlen(type);
                m->hashmap = hashmap;
                ASPL_ACCESS(obj).value.map = m;
                aspl_ailinterpreter_stack_push(context->stack, obj);
            }
            else {
                ASPL_PANIC("Unknown type '%s'", type);
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BYTE_ARRAY_LITERAL: {
            long long length = aspl_ailinterpreter_read_long(bytes);
            ASPL_OBJECT_TYPE* array = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * length);
            for (int i = 0; i < length; i++) {
                array[i] = ASPL_BYTE_LITERAL(aspl_ailinterpreter_read_byte(bytes));
            }
            aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_LITERAL("list<byte>", 10, array, length));
            break;
        }
        case ASPL_AILI_INSTRUCTION_IMPLEMENT: {
            char* call = aspl_ailinterpreter_read_long_string(bytes);
            int argc = aspl_ailinterpreter_read_int(bytes);
            ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * argc), .size = argc };
            for (int i = argc - 1; i >= 0; i--) {
                arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
            }
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_implement(context, call, arguments);
            if (!ASPL_IS_UNINITIALIZED(value)) {
                aspl_ailinterpreter_stack_push(context->stack, value);
            }
            else {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_VARIABLE: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_AILI_TypeList types = aspl_ailinterpreter_parse_types(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_register_variable(context, identifier, value);
            aspl_ailinterpreter_stack_push(context->stack, value);
            break;
        }
        case ASPL_AILI_INSTRUCTION_UPDATE_VARIABLE: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_peek(context->stack);
            ASPL_OBJECT_TYPE* address = aspl_ailinterpreter_access_variable_address(context, identifier);
            *address = value;
            break;
        }
        case ASPL_AILI_INSTRUCTION_ACCESS_VARIABLE: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_access_variable(context, identifier);
            aspl_ailinterpreter_stack_push(context->stack, value);
            break;
        }
        case ASPL_AILI_INSTRUCTION_JUMP_RELATIVE: {
            long long offset = aspl_ailinterpreter_read_long(bytes);
            bytes->position += offset;
            break;
        }
        case ASPL_AILI_INSTRUCTION_JUMP_RELATIVE_IF_FALSE: {
            long long offset = aspl_ailinterpreter_read_long(bytes);
            if (!ASPL_IS_TRUE(aspl_ailinterpreter_stack_pop(context->stack))) {
                bytes->position += offset;
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_CALL_FUNCTION: {
            char* identifier = aspl_ailinterpreter_read_long_string(bytes);
            int new_thread = aspl_ailinterpreter_read_bool(bytes);
            int argc = aspl_ailinterpreter_read_int(bytes);
            ASPL_AILI_ParameterList* function_parameters = NULL;
            if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->builtin_functions, identifier)) {
                ASPL_AILI_BuiltinFunction* builtin_function = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->builtin_functions, identifier);
                function_parameters = &builtin_function->parameters;
            }
            else if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->custom_functions, identifier)) {
                ASPL_AILI_CustomFunction* custom_function = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_functions, identifier);
                function_parameters = &custom_function->parameters;
            }
            int size = argc > function_parameters->size ? argc : function_parameters->size;
            ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * size), .size = size };
            for (int i = argc - 1; i >= 0; i--) {
                arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
            }
            for (int i = argc; i < function_parameters->size; i++) {
                if (function_parameters->parameters[i].optional) {
                    arguments.arguments[i] = aspl_object_clone_shallow(function_parameters->parameters[i].default_value);
                }
                else {
                    ASPL_PANIC("Missing argument for parameter '%s'", function_parameters->parameters[i].name);
                }
            }
            ASPL_AILI_FunctionPtr function = aspl_ailinterpreter_get_function(context, identifier);
            if (new_thread) {
                ASPL_AILI_ThreadFunctionWrapperData* data = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadFunctionWrapperData));
                data->function = function;
                data->context = aspl_ailinterpreter_new_thread_context(context->environment_context);
                data->bytes = aspl_ailinterpreter_bytelist_clone(bytes);
                data->identifier = identifier;
                data->arguments = arguments;
                thread_create(aspl_ailinterpreter_new_thread_function_invocation_callback, data, THREAD_STACK_SIZE_DEFAULT);
                aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
            }
            else {
                aspl_ailinterpreter_stack_trace_push(context, identifier);
                aspl_ailinterpreter_stack_push_frame(context->stack);
                aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_FUNCTION, .previous_address = bytes->position, .previous_loop_meta_stack = context->loop_meta_stack });
                context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                context->loop_meta_stack->top = 0;
                aspl_ailinterpreter_push_scope(context);
                function(context, bytes, identifier, arguments);
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_FUNCTION: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_AILI_ParameterList parameters = aspl_ailinterpreter_parse_parameters(context, bytes);
            ASPL_AILI_TypeList return_types = aspl_ailinterpreter_parse_types(bytes);
            ASPL_AILI_CustomFunction* custom_function = ASPL_MALLOC(sizeof(ASPL_AILI_CustomFunction));
            custom_function->parameters = parameters;
            long long code_length = aspl_ailinterpreter_read_long(bytes);
            custom_function->address = bytes->position;
            bytes->position += code_length;
            hashmap_str_to_voidptr_hashmap_set(context->environment_context->custom_functions, identifier, custom_function);
            hashmap_str_to_voidptr_hashmap_set(context->environment_context->functions, identifier, aspl_ailinterpreter_custom_function_callback);
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_METHOD: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            char* name = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_AILI_ParameterList parameters = aspl_ailinterpreter_parse_parameters(context, bytes);
            ASPL_AILI_TypeList return_types = aspl_ailinterpreter_parse_types(bytes);
            ASPL_AILI_CustomMethod* custom_method = ASPL_MALLOC(sizeof(ASPL_AILI_CustomMethod));
            custom_method->is_static = is_static;
            custom_method->parameters = parameters;
            long long code_length = aspl_ailinterpreter_read_long(bytes);
            custom_method->address = bytes->position;
            bytes->position += code_length;
            hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, context->current_class);
            hashmap_str_to_voidptr_hashmap_set(methods, name, custom_method);
            break;
        }
        case ASPL_AILI_INSTRUCTION_CALL_METHOD: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            if (is_static) {
                char* type = aspl_ailinterpreter_read_short_string(bytes);
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                char new_thread = aspl_ailinterpreter_read_bool(bytes);
                int argc = aspl_ailinterpreter_read_int(bytes);
                ASPL_AILI_ParameterList* method_parameters = NULL;
                // static builtin methods are not supported in ASPL yet
                if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->custom_methods, type)) {
                    hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, type);
                    if (hashmap_str_to_voidptr_hashmap_contains_key(methods, name)) {
                        ASPL_AILI_CustomMethod* method = hashmap_str_to_voidptr_hashmap_get_value(methods, name);
                        method_parameters = &method->parameters;
                    }
                }
                int size = argc > method_parameters->size ? argc : method_parameters->size;
                ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * size), .size = size };
                for (int i = argc - 1; i >= 0; i--) {
                    arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
                }
                for (int i = argc; i < method_parameters->size; i++) {
                    if (method_parameters->parameters[i].optional) {
                        arguments.arguments[i] = aspl_object_clone_shallow(method_parameters->parameters[i].default_value);
                    }
                    else {
                        ASPL_PANIC("Missing argument for parameter '%s'", method_parameters->parameters[i].name);
                    }
                }
                ASPL_AILI_CustomMethod* method = hashmap_str_to_voidptr_hashmap_get_value(hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, type), name);
                char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(type) + strlen(name) + 2));
                strcpy(identifier, type);
                strcat(identifier, ".");
                strcat(identifier, name);
                if (new_thread) {
                    ASPL_AILI_ThreadMethodWrapperData* data = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadMethodWrapperData));
                    data->method = method;
                    data->context = aspl_ailinterpreter_new_thread_context(context->environment_context);
                    data->bytes = aspl_ailinterpreter_bytelist_clone(bytes);
                    data->identifier = identifier;
                    data->arguments = arguments;
                    thread_create(aspl_ailinterpreter_new_thread_method_invocation_callback, data, THREAD_STACK_SIZE_DEFAULT);
                    aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
                }
                else {
                    aspl_ailinterpreter_stack_trace_push(context, identifier);
                    aspl_ailinterpreter_stack_push_frame(context->stack);
                    ASPL_AILI_CallableInvocationMeta meta = { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_METHOD, .previous_address = bytes->position, .previous_loop_meta_stack = context->loop_meta_stack };
                    aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, meta);
                    context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                    context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                    context->loop_meta_stack->top = 0;
                    aspl_ailinterpreter_push_scope(context);
                    bytes->position = method->address;
                    for (int i = 0; i < method->parameters.size; i++) {
                        if (i < arguments.size) {
                            aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, arguments.arguments[i]);
                        }
                        else if (method->parameters.parameters[i].optional) {
                            aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, aspl_object_clone_shallow(method->parameters.parameters[i].default_value));
                        }
                        else {
                            ASPL_PANIC("Missing argument for parameter '%s'", method->parameters.parameters[i].name);
                        }
                    }
                }
            }
            else {
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                char new_thread = aspl_ailinterpreter_read_bool(bytes);
                int argc = aspl_ailinterpreter_read_int(bytes);
                ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * argc), .size = argc };
                for (int i = argc - 1; i >= 0; i--) {
                    arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
                }
                ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
                char isInvokeMethod = ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_CALLBACK && strcmp(name, "invoke") == 0;
                if (!isInvokeMethod) {
                    ASPL_AILI_ParameterList* method_parameters = NULL;
                    if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->builtin_methods, aspl_object_get_short_type_pointer(object))) {
                        hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->builtin_methods, aspl_object_get_short_type_pointer(object));
                        if (hashmap_str_to_voidptr_hashmap_contains_key(methods, name)) {
                            ASPL_AILI_BuiltinMethod* method = hashmap_str_to_voidptr_hashmap_get_value(methods, name);
                            method_parameters = &method->parameters;
                        }
                    }
                    if (method_parameters == NULL) {
                        if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->builtin_methods, "any")) {
                            hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->builtin_methods, "any");
                            if (hashmap_str_to_voidptr_hashmap_contains_key(methods, name)) {
                                ASPL_AILI_BuiltinMethod* method = hashmap_str_to_voidptr_hashmap_get_value(methods, name);
                                method_parameters = &method->parameters;
                            }
                        }
                    }
                    if (method_parameters == NULL) {
                        method_parameters = &aspl_ailinterpreter_util_find_custom_method(context, aspl_object_get_short_type_pointer(object), name)->parameters;
                    }
                    if (method_parameters->size > argc) {
                        arguments.arguments = ASPL_REALLOC(arguments.arguments, sizeof(ASPL_OBJECT_TYPE) * method_parameters->size);
                        arguments.size = method_parameters->size;
                    }
                    for (int i = argc; i < method_parameters->size; i++) {
                        if (method_parameters->parameters[i].optional) {
                            arguments.arguments[i] = aspl_object_clone_shallow(method_parameters->parameters[i].default_value);
                        }
                        else {
                            ASPL_PANIC("Missing argument for parameter '%s'", method_parameters->parameters[i].name);
                        }
                    }
                    if (ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_NULL || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_BOOLEAN || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_BYTE || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_INTEGER || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_LONG || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_FLOAT || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_DOUBLE || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_STRING || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_LIST || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_MAP) {
                        ASPL_OBJECT_TYPE** args = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * argc);
                        for (int i = 0; i < arguments.size; i++) {
                            args[i] = C_REFERENCE(arguments.arguments[i]);
                        }
                        aspl_ailinterpreter_stack_push(context->stack, aspl_object_method_invoke(object, name, args));
                    }
                    else {
                        ASPL_AILI_CustomMethod* method = aspl_ailinterpreter_util_find_custom_method(context, aspl_object_get_short_type_pointer(object), name);
                        char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(aspl_object_get_short_type_pointer(object)) + strlen(name) + 2));
                        strcpy(identifier, aspl_object_get_short_type_pointer(object));
                        strcat(identifier, ".");
                        strcat(identifier, name);
                        if (new_thread) {
                            ASPL_AILI_ThreadMethodWrapperData* data = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadMethodWrapperData));
                            data->method = method;
                            data->context = aspl_ailinterpreter_new_thread_context(context->environment_context);
                            data->bytes = aspl_ailinterpreter_bytelist_clone(bytes);
                            data->identifier = identifier;
                            data->arguments = arguments;
                            data->context->current_instance = object;
                            thread_create(aspl_ailinterpreter_new_thread_method_invocation_callback, data, THREAD_STACK_SIZE_DEFAULT);
                            aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
                        }
                        else {
                            aspl_ailinterpreter_stack_trace_push(context, identifier);
                            aspl_ailinterpreter_stack_push_frame(context->stack);
                            ASPL_AILI_CallableInvocationMeta meta = { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_METHOD, .previous_address = bytes->position, .previous_instance = C_REFERENCE(context->current_instance), .previous_loop_meta_stack = context->loop_meta_stack };
                            aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, meta);
                            context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                            context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                            context->loop_meta_stack->top = 0;
                            aspl_ailinterpreter_push_scope(context);
                            bytes->position = method->address;
                            if (ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_CLASS_INSTANCE) { // optimization
                                meta.previous_instance = C_REFERENCE(context->current_instance);
                                context->current_instance = object;
                                for (int i = 0; i < method->parameters.size; i++) {
                                    if (i < arguments.size) {
                                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, arguments.arguments[i]);
                                    }
                                    else if (method->parameters.parameters[i].optional) {
                                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, aspl_object_clone_shallow(method->parameters.parameters[i].default_value));
                                    }
                                    else {
                                        ASPL_PANIC("Missing argument for parameter '%s'", method->parameters.parameters[i].name);
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    ASPL_Callback* callback = ASPL_ACCESS(object).value.callback;
                    char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(callback->typePtr) + strlen(name) + 2));
                    strcpy(identifier, callback->typePtr);
                    strcat(identifier, ".");
                    strcat(identifier, name);
                    if (new_thread) {
                        ASPL_AILI_ThreadCallbackWrapperData* data = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadCallbackWrapperData));
                        data->callback = callback;
                        data->context = aspl_ailinterpreter_new_thread_context(context->environment_context);
                        data->bytes = aspl_ailinterpreter_bytelist_clone(bytes);
                        data->identifier = identifier;
                        data->arguments = arguments;
                        thread_create(aspl_ailinterpreter_new_thread_callback_invocation_callback, data, THREAD_STACK_SIZE_DEFAULT);
                        aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
                    }
                    else {
                        aspl_ailinterpreter_stack_trace_push(context, identifier);
                        aspl_ailinterpreter_stack_push_frame(context->stack);
                        ASPL_AILI_CallbackData* callback_data = (ASPL_AILI_CallbackData*)callback->function;
                        aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_CALLBACK, .previous_address = bytes->position, .previous_scopes = context->scopes, .previous_scopes_top = context->scopes_top, .previous_loop_meta_stack = context->loop_meta_stack });
                        context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                        context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                        context->loop_meta_stack->top = 0;
                        context->scopes = callback_data->creation_scopes;
                        context->scopes_top = callback_data->creation_scopes_top;
                        bytes->position = callback_data->address;
                        for (int i = 0; i < callback_data->parameters.size; i++) {
                            if (i < arguments.size) {
                                aspl_ailinterpreter_register_variable(context, callback_data->parameters.parameters[i].name, arguments.arguments[i]);
                            }
                            else if (callback_data->parameters.parameters[i].optional) {
                                aspl_ailinterpreter_register_variable(context, callback_data->parameters.parameters[i].name, aspl_object_clone_shallow(callback_data->parameters.parameters[i].default_value));
                            }
                            else {
                                ASPL_PANIC("Missing argument for parameter '%s'", callback_data->parameters.parameters[i].name);
                            }
                        }
                    }
                }
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_CALL_EXACT_METHOD: {
            char* type = aspl_ailinterpreter_read_short_string(bytes);
            char* name = aspl_ailinterpreter_read_short_string(bytes);
            char new_thread = aspl_ailinterpreter_read_bool(bytes);
            int argc = aspl_ailinterpreter_read_int(bytes);
            hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, type);
            ASPL_AILI_CustomMethod* method = hashmap_str_to_voidptr_hashmap_get_value(methods, name);
            ASPL_AILI_ParameterList* method_parameters = &method->parameters;
            int size = argc > method_parameters->size ? argc : method_parameters->size;
            ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * size), .size = size };
            for (int i = argc - 1; i >= 0; i--) {
                arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
            }
            for (int i = argc; i < method_parameters->size; i++) {
                if (method_parameters->parameters[i].optional) {
                    arguments.arguments[i] = aspl_object_clone_shallow(method_parameters->parameters[i].default_value);
                }
                else {
                    ASPL_PANIC("Missing argument for parameter '%s'", method_parameters->parameters[i].name);
                }
            }
            char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(type) + strlen(name) + 2));
            strcpy(identifier, type);
            strcat(identifier, ".");
            strcat(identifier, name);
            ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
            if (new_thread) {
                ASPL_AILI_ThreadMethodWrapperData* data = ASPL_MALLOC(sizeof(ASPL_AILI_ThreadMethodWrapperData));
                data->method = method;
                data->context = aspl_ailinterpreter_new_thread_context(context->environment_context);
                data->bytes = aspl_ailinterpreter_bytelist_clone(bytes);
                data->identifier = identifier;
                data->arguments = arguments;
                data->context->current_instance = object;
                thread_create(aspl_ailinterpreter_new_thread_method_invocation_callback, data, THREAD_STACK_SIZE_DEFAULT);
                aspl_ailinterpreter_stack_push(context->stack, ASPL_NULL());
            }
            else {
                aspl_ailinterpreter_stack_trace_push(context, identifier);
                aspl_ailinterpreter_stack_push_frame(context->stack);
                ASPL_AILI_CallableInvocationMeta meta = { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_METHOD, .previous_address = bytes->position, .previous_loop_meta_stack = context->loop_meta_stack };
                aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, meta);
                context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                context->loop_meta_stack->top = 0;
                aspl_ailinterpreter_push_scope(context);
                bytes->position = method->address;
                meta.previous_instance = C_REFERENCE(context->current_instance);
                context->current_instance = object;
                for (int i = 0; i < method->parameters.size; i++) {
                    if (i < arguments.size) {
                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, arguments.arguments[i]);
                    }
                    else if (method->parameters.parameters[i].optional) {
                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, aspl_object_clone_shallow(method->parameters.parameters[i].default_value));
                    }
                    else {
                        ASPL_PANIC("Missing argument for parameter '%s'", method->parameters.parameters[i].name);
                    }
                }
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BREAK: {
            char legacy = aspl_ailinterpreter_read_bool(bytes); // TODO: Remove this
            int level = aspl_ailinterpreter_read_int(bytes); // TODO: Take this into account
            bytes->position = aspl_ailinterpreter_lms_peek(context->loop_meta_stack).full_end_address;
            break;
        }
        case ASPL_AILI_INSTRUCTION_CONTINUE: {
            char legacy = aspl_ailinterpreter_read_bool(bytes); // TODO: Remove this
            int level = aspl_ailinterpreter_read_int(bytes); // TODO: Take this into account
            bytes->position = aspl_ailinterpreter_lms_peek(context->loop_meta_stack).body_end_address;
            break;
        }
        case ASPL_AILI_INSTRUCTION_RETURN: {
            aspl_ailinterpreter_return(context, bytes, aspl_ailinterpreter_stack_pop(context->stack));
            break;
        }
        case ASPL_AILI_INSTRUCTION_FALLBACK: {
            aspl_ailinterpreter_return(context, bytes, aspl_ailinterpreter_stack_pop(context->stack));
            break;
        }
        case ASPL_AILI_INSTRUCTION_ESCAPE: {
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_return(context, bytes, ASPL_UNINITIALIZED);
            aspl_ailinterpreter_return(context, bytes, value);
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_ENUM: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            int field_count = aspl_ailinterpreter_read_int(bytes);
            ASPL_Enum* e = ASPL_MALLOC(sizeof(ASPL_Enum));
            e->typePtr = identifier;
            e->typeLen = strlen(identifier);
            e->isFlagEnum = aspl_ailinterpreter_read_bool(bytes);
            e->stringValues = hashmap_int_to_str_new_hashmap((hashmap_int_to_str_HashMapConfig) { .initial_capacity = field_count });
            for (int i = 0; i < field_count; i++) {
                char* field = aspl_ailinterpreter_read_short_string(bytes);
                int value = aspl_ailinterpreter_read_int(bytes);
                hashmap_int_to_str_hashmap_set(e->stringValues, value, field);
            }
            hashmap_str_to_voidptr_hashmap_set(context->environment_context->enums, identifier, e);
            break;
        }
        case ASPL_AILI_INSTRUCTION_ENUM_FIELD: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            char* string_field = aspl_ailinterpreter_read_short_string(bytes);
            (void)string_field; // ignored in this implementation
            int int_field = aspl_ailinterpreter_read_int(bytes);
            ASPL_Enum* e = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->enums, identifier);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_ENUM_FIELD_LITERAL(e, int_field));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_POINTER: {
            ASPL_OBJECT_TYPE* address;
            switch (aspl_ailinterpreter_fetch_instruction(bytes)) {
            case ASPL_AILI_INSTRUCTION_ACCESS_VARIABLE: {
                address = aspl_ailinterpreter_access_variable_address(context, aspl_ailinterpreter_read_short_string(bytes));
                break;
            }
            default:
                ASPL_PANIC("Cannot declare a pointer to an invalid address");
            }
            char* to_type = aspl_object_get_type_pointer(*address);
            char* type = ASPL_MALLOC(sizeof(char) * (strlen(to_type) + 1));
            strcpy(type, to_type);
            type[strlen(to_type)] = '*';
            aspl_ailinterpreter_stack_push(context->stack, ASPL_REFERENCE(type, strlen(type), address));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DEREFERENCE_POINTER: {
            ASPL_OBJECT_TYPE pointer = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_DEREFERENCE(pointer));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_CALLBACK: {
            char* type = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_AILI_ParameterList parameters = aspl_ailinterpreter_parse_parameters(context, bytes);
            ASPL_AILI_TypeList return_types = aspl_ailinterpreter_parse_types(bytes);
            long long code_length = aspl_ailinterpreter_read_long(bytes);
            ASPL_Callback* callback = ASPL_MALLOC(sizeof(ASPL_Callback));
            callback->typeLen = strlen(type);
            callback->typePtr = type;
            callback->function = 0;
            callback->function = ASPL_MALLOC(sizeof(ASPL_AILI_CallbackData)); // TODO: This is a nasty hack to avoid having to add a new field to the ASPL_Callback struct
            ((ASPL_AILI_CallbackData*)callback->function)->address = bytes->position;
            ((ASPL_AILI_CallbackData*)callback->function)->creation_scopes = memcpy(ASPL_MALLOC(context->scopes_size * sizeof(hashmap_str_to_voidptr_HashMap*)), context->scopes, context->scopes_size * sizeof(hashmap_str_to_voidptr_HashMap*));
            ((ASPL_AILI_CallbackData*)callback->function)->creation_scopes_top = context->scopes_top;
            ((ASPL_AILI_CallbackData*)callback->function)->parameters = parameters;
            bytes->position += code_length;
            ASPL_OBJECT_TYPE callback_object = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(callback_object).kind = ASPL_OBJECT_KIND_CALLBACK;
            ASPL_ACCESS(callback_object).value.callback = callback;
            aspl_ailinterpreter_stack_push(context->stack, callback_object);
            break;
        }
        case ASPL_AILI_INSTRUCTION_CAST: {
            char* target_type = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
            char pushed = 0;
            if (ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_INTEGER || ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_ENUM_FIELD) {
                if (hashmap_str_to_voidptr_hashmap_contains_key(context->environment_context->enums, target_type)) {
                    pushed = 1;
                    switch (ASPL_ACCESS(object).kind)
                    {
                    case ASPL_OBJECT_KIND_INTEGER:
                        aspl_ailinterpreter_stack_push(context->stack, ASPL_ENUM_FIELD_LITERAL(hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->enums, target_type), ASPL_ACCESS(object).value.integer32));
                        break;
                    case ASPL_OBJECT_KIND_ENUM_FIELD:
                        aspl_ailinterpreter_stack_push(context->stack, object);
                        break;
                    default:
                        // impossible
                        break;
                    }
                }
            }
            if (!pushed) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_CAST(object, target_type));
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_COUNT_COLLECTION: {
            ASPL_OBJECT_TYPE collection = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(collection).kind) {
            case ASPL_OBJECT_KIND_LIST:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_LENGTH(collection));
                break;
            case ASPL_OBJECT_KIND_MAP:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_LENGTH(collection));
                break;
            case ASPL_OBJECT_KIND_STRING:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_STRING_LENGTH(collection));
                break;
            default:
                ASPL_PANIC("Cannot count a non-collection object");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_ACCESS_COLLECTION_KEY_WITH_INDEX: {
            ASPL_OBJECT_TYPE index = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE collection = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(collection).kind) {
            case ASPL_OBJECT_KIND_MAP:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_GET_KEY_FROM_INDEX(collection, index));
                break;
            default:
                ASPL_PANIC("Cannot access a key from something that is not a map");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_ACCESS_COLLECTION_VALUE_WITH_INDEX: {
            ASPL_OBJECT_TYPE index = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE collection = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(collection).kind) {
            case ASPL_OBJECT_KIND_LIST:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_GET(collection, index));
                break;
            case ASPL_OBJECT_KIND_MAP:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_GET_VALUE_FROM_INDEX(collection, index));
                break;
            case ASPL_OBJECT_KIND_STRING:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_STRING_INDEX(collection, index));
                break;
            default:
                ASPL_PANIC("Cannot access a value from something that is not a collection");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_INDEX_COLLECTION: {
            ASPL_OBJECT_TYPE key = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE collection = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(collection).kind) {
            case ASPL_OBJECT_KIND_LIST:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_GET(collection, key));
                break;
            case ASPL_OBJECT_KIND_MAP:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_GET(collection, key));
                break;
            case ASPL_OBJECT_KIND_STRING:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_STRING_INDEX(collection, key));
                break;
            default:
                ASPL_PANIC("Cannot index a non-collection object");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_UPDATE_COLLECTION: {
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE key = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE collection = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(collection).kind) {
            case ASPL_OBJECT_KIND_LIST:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_SET(collection, key, value));
                break;
            case ASPL_OBJECT_KIND_MAP:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_SET(collection, key, value));
                break;
            default:
                ASPL_PANIC("Cannot update a non-collection object by an index");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_NEGATE: {
            aspl_ailinterpreter_stack_push(context->stack, ASPL_NEGATE(aspl_ailinterpreter_stack_pop(context->stack)));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_CLASS: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            short parents_size = aspl_ailinterpreter_read_short(bytes);
            char** parents = ASPL_MALLOC(sizeof(char*) * parents_size);
            for (int i = 0; i < parents_size; i++) {
                parents[i] = aspl_ailinterpreter_read_short_string(bytes);
            }
            char is_error = aspl_ailinterpreter_read_bool(bytes);
            ASPL_AILI_Class* class = ASPL_MALLOC(sizeof(ASPL_AILI_Class));
            class->is_static = is_static;
            class->parents = parents;
            class->parents_size = parents_size;
            class->is_error = is_error;
            aspl_ailinterpreter_register_class(context->environment_context, identifier, class);
            aspl_ailinterpreter_class_stack_push(context->class_stack, context->current_class);
            context->current_class = identifier;
            hashmap_str_to_voidptr_hashmap_set(context->environment_context->custom_methods, context->current_class, hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 4 }));
            aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->default_threadlocal_static_property_values = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 4 });
            aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->static_property_values = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 4 });
            aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->reactive_properties = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 4 });
            break;
        }
        case ASPL_AILI_INSTRUCTION_NEW: {
            char* identifier = aspl_ailinterpreter_read_short_string(bytes);
            int argc = aspl_ailinterpreter_read_int(bytes);
            ASPL_AILI_ArgumentList arguments = { .arguments = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * argc), .size = argc };
            for (int i = argc - 1; i >= 0; i--) {
                arguments.arguments[i] = aspl_ailinterpreter_stack_pop(context->stack);
            }
            ASPL_OBJECT_TYPE obj = ASPL_ALLOC_OBJECT();
            ASPL_ACCESS(obj).kind = ASPL_OBJECT_KIND_CLASS_INSTANCE;
            ASPL_ClassInstance* instance = ASPL_MALLOC(sizeof(ASPL_ClassInstance));
            instance->typePtr = identifier;
            instance->typeLen = strlen(identifier);
            instance->properties = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 1 });
            instance->isError = aspl_ailinterpreter_access_class(context->environment_context, identifier)->is_error;
            ASPL_ACCESS(obj).value.classInstance = instance;

            aspl_ailinterpreter_stack_push(context->stack, obj);

            hashmap_str_to_voidptr_HashMap* methods = hashmap_str_to_voidptr_hashmap_get_value(context->environment_context->custom_methods, identifier);
            if (hashmap_str_to_voidptr_hashmap_contains_key(methods, "construct")) {
                ASPL_AILI_CustomMethod* method = hashmap_str_to_voidptr_hashmap_get_value(methods, "construct");
                char* method_identifier = ASPL_MALLOC(sizeof(char) * (strlen(identifier) + strlen("construct") + 2));
                strcpy(method_identifier, identifier);
                strcat(method_identifier, ".");
                strcat(method_identifier, "construct");
                aspl_ailinterpreter_stack_trace_push(context, method_identifier);
                aspl_ailinterpreter_stack_push_frame(context->stack);
                aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_METHOD, .previous_instance = C_REFERENCE(context->current_instance), .previous_address = bytes->position, .is_constructor = 1, .previous_loop_meta_stack = context->loop_meta_stack });
                context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                context->loop_meta_stack->top = 0;
                aspl_ailinterpreter_push_scope(context);
                context->current_instance = obj;
                bytes->position = method->address;
                for (int i = 0; i < method->parameters.size; i++) {
                    if (i < arguments.size) {
                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, arguments.arguments[i]);
                    }
                    else if (method->parameters.parameters[i].optional) {
                        aspl_ailinterpreter_register_variable(context, method->parameters.parameters[i].name, aspl_object_clone_shallow(method->parameters.parameters[i].default_value));
                    }
                    else {
                        ASPL_PANIC("Missing argument for parameter '%s'", method->parameters.parameters[i].name);
                    }
                }
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_THIS: {
            aspl_ailinterpreter_stack_push(context->stack, context->current_instance);
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECLARE_PROPERTY: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            char is_thread_local = aspl_ailinterpreter_read_bool(bytes);
            char* name = aspl_ailinterpreter_read_short_string(bytes);
            ASPL_AILI_TypeList types = aspl_ailinterpreter_parse_types(bytes);
            if (aspl_ailinterpreter_read_bool(bytes)) {
                long long get_code_length = aspl_ailinterpreter_read_long(bytes);
                ASPL_AILI_ReactiveProperty* reactive_property = ASPL_MALLOC(sizeof(ASPL_AILI_ReactiveProperty));
                if (get_code_length > -1) {
                    reactive_property->get_address = bytes->position;
                    bytes->position += get_code_length;
                }
                long long set_code_length = aspl_ailinterpreter_read_long(bytes);
                if (set_code_length > -1) {
                    reactive_property->set_address = bytes->position;
                    bytes->position += set_code_length;
                }
                hashmap_str_to_voidptr_hashmap_set(aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->reactive_properties, name, reactive_property);
            }
            else {
                if (is_static) {
                    ASPL_OBJECT_TYPE default_value = aspl_ailinterpreter_stack_pop(context->stack);
                    if (is_thread_local) {
                        hashmap_str_to_voidptr_HashMap* default_threadlocal_static_property_values = aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->default_threadlocal_static_property_values;
                        hashmap_str_to_voidptr_hashmap_set(default_threadlocal_static_property_values, name, C_REFERENCE(default_value));
                        if (!hashmap_str_to_voidptr_hashmap_contains_key(context->local_static_property_values, context->current_class)) {
                            hashmap_str_to_voidptr_hashmap_set(context->local_static_property_values, context->current_class, hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 4 }));
                        }
                        hashmap_str_to_voidptr_hashmap_set(hashmap_str_to_voidptr_hashmap_get_value(context->local_static_property_values, context->current_class), name, C_REFERENCE(default_value));
                    }
                    else {
                        hashmap_str_to_voidptr_HashMap* static_property_values = aspl_ailinterpreter_access_class(context->environment_context, context->current_class)->static_property_values;
                        hashmap_str_to_voidptr_hashmap_set(static_property_values, name, C_REFERENCE(default_value));
                    }
                }
                else {
                    // TOOD: Do we need anything here?
                }
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_ACCESS_PROPERTY: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            if (is_static) {
                char* type = aspl_ailinterpreter_read_short_string(bytes);
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                if (hashmap_str_to_voidptr_hashmap_contains_key(aspl_ailinterpreter_access_class(context->environment_context, type)->reactive_properties, name)) {
                    ASPL_AILI_ReactiveProperty* reactive_property = hashmap_str_to_voidptr_hashmap_get_value(aspl_ailinterpreter_access_class(context->environment_context, type)->reactive_properties, name);
                    char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(type) + strlen(name) + 2));
                    strcpy(identifier, type);
                    strcat(identifier, ".");
                    strcat(identifier, name);
                    aspl_ailinterpreter_stack_trace_push(context, identifier);
                    aspl_ailinterpreter_stack_push_frame(context->stack);
                    aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_REACTIVE_PROPERTY, .previous_address = bytes->position, .previous_loop_meta_stack = context->loop_meta_stack });
                    context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                    context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                    context->loop_meta_stack->top = 0;
                    aspl_ailinterpreter_push_scope(context);
                    bytes->position = reactive_property->get_address;
                }
                else if (hashmap_str_to_voidptr_hashmap_contains_key(context->local_static_property_values, type) && hashmap_str_to_voidptr_hashmap_contains_key(hashmap_str_to_voidptr_hashmap_get_value(context->local_static_property_values, type), name)) {
                    aspl_ailinterpreter_stack_push(context->stack, *(ASPL_OBJECT_TYPE*)hashmap_str_to_voidptr_hashmap_get_value(hashmap_str_to_voidptr_hashmap_get_value(context->local_static_property_values, type), name));
                }
                else {
                    hashmap_str_to_voidptr_HashMap* static_property_values = aspl_ailinterpreter_access_class(context->environment_context, type)->static_property_values;
                    aspl_ailinterpreter_stack_push(context->stack, *(ASPL_OBJECT_TYPE*)hashmap_str_to_voidptr_hashmap_get_value(static_property_values, name));
                }
            }
            else {
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
                if (ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_CLASS_INSTANCE) {
                    if (hashmap_str_to_voidptr_hashmap_contains_key(aspl_ailinterpreter_access_class(context->environment_context, aspl_object_get_short_type_pointer(object))->reactive_properties, name)) { // TODO: Support reactive properties in parents
                        ASPL_AILI_ReactiveProperty* reactive_property = hashmap_str_to_voidptr_hashmap_get_value(aspl_ailinterpreter_access_class(context->environment_context, aspl_object_get_short_type_pointer(object))->reactive_properties, name);
                        char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(aspl_object_get_short_type_pointer(object)) + strlen(name) + 2));
                        strcpy(identifier, aspl_object_get_short_type_pointer(object));
                        strcat(identifier, ".");
                        strcat(identifier, name);
                        aspl_ailinterpreter_stack_trace_push(context, identifier);
                        aspl_ailinterpreter_stack_push_frame(context->stack);
                        aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_REACTIVE_PROPERTY, .previous_address = bytes->position, .previous_instance = C_REFERENCE(context->current_instance), .previous_loop_meta_stack = context->loop_meta_stack });
                        context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                        context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                        context->loop_meta_stack->top = 0;
                        aspl_ailinterpreter_push_scope(context);
                        context->current_instance = object;
                        bytes->position = reactive_property->get_address;
                    }
                    else {
                        aspl_ailinterpreter_stack_push(context->stack, ASPL_CLASS_INSTANCE_GET_PROPERTY(ASPL_ACCESS(object).value.classInstance, name));
                    }
                }
                else {
                    switch (ASPL_ACCESS(object).kind) {
                    case ASPL_OBJECT_KIND_STRING:
                        if (strcmp(name, "length") == 0) {
                            aspl_ailinterpreter_stack_push(context->stack, ASPL_STRING_LENGTH(object));
                        }
                        else {
                            ASPL_PANIC("Cannot access the unknown property string.%s", name);
                        }
                        break;
                    case ASPL_OBJECT_KIND_LIST:
                        if (strcmp(name, "length") == 0) {
                            aspl_ailinterpreter_stack_push(context->stack, ASPL_LIST_LENGTH(object));
                        }
                        else {
                            ASPL_PANIC("Cannot access the unknown property list.%s", name);
                        }
                        break;
                    case ASPL_OBJECT_KIND_MAP:
                        if (strcmp(name, "length") == 0) {
                            aspl_ailinterpreter_stack_push(context->stack, ASPL_MAP_LENGTH(object));
                        }
                        else {
                            ASPL_PANIC("Cannot access the unknown property map.%s", name);
                        }
                        break;
                    default:
                        ASPL_PANIC("Cannot access the unknown property %s.%s", aspl_object_get_type_pointer(object), name);
                    }
                }
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_UPDATE_PROPERTY: {
            char is_static = aspl_ailinterpreter_read_bool(bytes);
            char is_reactive = aspl_ailinterpreter_read_bool(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            if (is_static) {
                char* type = aspl_ailinterpreter_read_short_string(bytes);
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                if (is_reactive) {
                    ASPL_AILI_ReactiveProperty* reactive_property = hashmap_str_to_voidptr_hashmap_get_value(aspl_ailinterpreter_access_class(context->environment_context, type)->reactive_properties, name);
                    char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(type) + strlen(name) + 2));
                    strcpy(identifier, type);
                    strcat(identifier, ".");
                    strcat(identifier, name);
                    aspl_ailinterpreter_stack_trace_push(context, identifier);
                    aspl_ailinterpreter_stack_push_frame(context->stack);
                    aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_REACTIVE_PROPERTY, .previous_address = bytes->position, .previous_loop_meta_stack = context->loop_meta_stack, .is_reactive_property_setter = 1 });
                    context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                    context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                    context->loop_meta_stack->top = 0;
                    aspl_ailinterpreter_push_scope(context);
                    bytes->position = reactive_property->set_address;
                    aspl_ailinterpreter_register_variable(context, "value", value);
                }
                else if (hashmap_str_to_voidptr_hashmap_contains_key(context->local_static_property_values, type) && hashmap_str_to_voidptr_hashmap_contains_key(hashmap_str_to_voidptr_hashmap_get_value(context->local_static_property_values, type), name)) {
                    *(ASPL_OBJECT_TYPE*)hashmap_str_to_voidptr_hashmap_get_value(hashmap_str_to_voidptr_hashmap_get_value(context->local_static_property_values, type), name) = value;
                }
                else {
                    hashmap_str_to_voidptr_HashMap* static_property_values = aspl_ailinterpreter_access_class(context->environment_context, type)->static_property_values;
                    hashmap_str_to_voidptr_hashmap_set(static_property_values, name, C_REFERENCE(value));
                }
            }
            else {
                char* name = aspl_ailinterpreter_read_short_string(bytes);
                ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
                if (is_reactive) {
                    ASPL_AILI_ReactiveProperty* reactive_property = aspl_ailinterpreter_util_find_reactive_property(context, aspl_object_get_short_type_pointer(object), name);
                    char* identifier = ASPL_MALLOC(sizeof(char) * (strlen(aspl_object_get_short_type_pointer(object)) + strlen(name) + 2));
                    strcpy(identifier, aspl_object_get_short_type_pointer(object));
                    strcat(identifier, ".");
                    strcat(identifier, name);
                    aspl_ailinterpreter_stack_trace_push(context, identifier);
                    aspl_ailinterpreter_stack_push_frame(context->stack);
                    aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_REACTIVE_PROPERTY, .previous_address = bytes->position, .previous_instance = C_REFERENCE(context->current_instance), .previous_loop_meta_stack = context->loop_meta_stack, .is_reactive_property_setter = 1 });
                    context->loop_meta_stack = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMetaStack));
                    context->loop_meta_stack->data = ASPL_MALLOC(sizeof(ASPL_AILI_LoopMeta) * 1024);
                    context->loop_meta_stack->top = 0;
                    aspl_ailinterpreter_push_scope(context);
                    context->current_instance = object;
                    bytes->position = reactive_property->set_address;
                    aspl_ailinterpreter_register_variable(context, "value", value);
                }
                else {
                    ASPL_CLASS_INSTANCE_SET_PROPERTY(ASPL_ACCESS(object).value.classInstance, name, value);
                }
            }
            if (!is_reactive) {
                aspl_ailinterpreter_stack_push(context->stack, value);
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_ADD: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_PLUS(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_INCREMENT: {
            ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(object).kind) {
            case ASPL_OBJECT_KIND_BYTE:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(ASPL_ACCESS(object).value.integer8 + 1));
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(ASPL_ACCESS(object).value.integer32 + 1));
                break;
            case ASPL_OBJECT_KIND_LONG:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(ASPL_ACCESS(object).value.integer64 + 1));
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_FLOAT_LITERAL(ASPL_ACCESS(object).value.float32 + 1));
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_DOUBLE_LITERAL(ASPL_ACCESS(object).value.float64 + 1));
                break;
            default:
                ASPL_PANIC("Unknown or invalid object type for the ++ operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_SUBTRACT: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_MINUS(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DECREMENT: {
            ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
            switch (ASPL_ACCESS(object).kind) {
            case ASPL_OBJECT_KIND_BYTE:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(ASPL_ACCESS(object).value.integer8 - 1));
                break;
            case ASPL_OBJECT_KIND_INTEGER:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(ASPL_ACCESS(object).value.integer32 - 1));
                break;
            case ASPL_OBJECT_KIND_LONG:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(ASPL_ACCESS(object).value.integer64 - 1));
                break;
            case ASPL_OBJECT_KIND_FLOAT:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_FLOAT_LITERAL(ASPL_ACCESS(object).value.float32 - 1));
                break;
            case ASPL_OBJECT_KIND_DOUBLE:
                aspl_ailinterpreter_stack_push(context->stack, ASPL_DOUBLE_LITERAL(ASPL_ACCESS(object).value.float64 - 1));
                break;
            default:
                ASPL_PANIC("Unknown or invalid object type for the -- operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_MULTIPLY: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_MULTIPLY(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_DIVIDE: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_DIVIDE(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_MODULO: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_MODULO(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_SMALLER_THAN: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_LESS_THAN(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_SMALLER_EQUAL: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_LESS_THAN_OR_EQUAL(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_GREATER_THAN: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_GREATER_THAN(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_GREATER_EQUAL: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_GREATER_THAN_OR_EQUAL(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_EQUALS: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_EQUALS(a, b));
            break;
        }
        case ASPL_AILI_INSTRUCTION_NOT_EQUALS: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_stack_push(context->stack, ASPL_NEGATE(ASPL_EQUALS(a, b)));
            break;
        }
        case ASPL_AILI_INSTRUCTION_BOOLEAN_AND: {
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            int length = aspl_ailinterpreter_read_int(bytes);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BOOLEAN) {
                if (!ASPL_ACCESS(a).value.boolean) {
                    bytes->position += length;
                    aspl_ailinterpreter_stack_push(context->stack, ASPL_FALSE());
                }
                // b is now just evaluated normally
            }
            else {
                ASPL_PANIC("Cannot use the boolean AND (&&) operator on a non-boolean value");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BITWISE_AND: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BYTE && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_BYTE) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(ASPL_ACCESS(a).value.integer8 & ASPL_ACCESS(b).value.integer8));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_INTEGER && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_INTEGER) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(ASPL_ACCESS(a).value.integer32 & ASPL_ACCESS(b).value.integer32));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_LONG && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_LONG) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(ASPL_ACCESS(a).value.integer64 & ASPL_ACCESS(b).value.integer64));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_ENUM_FIELD && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_ENUM_FIELD) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_ENUM_AND(a, b));
            }
            else {
                ASPL_PANIC("Invalid types for the & operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BOOLEAN_OR: {
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            int length = aspl_ailinterpreter_read_int(bytes);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BOOLEAN) {
                if (ASPL_ACCESS(a).value.boolean) {
                    bytes->position += length;
                    aspl_ailinterpreter_stack_push(context->stack, ASPL_TRUE());
                }
                // b is now just evaluated normally
            }
            else {
                ASPL_PANIC("Cannot use the boolean OR (||) operator on a non-boolean value");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BITWISE_OR: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BYTE && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_BYTE) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(ASPL_ACCESS(a).value.integer8 | ASPL_ACCESS(b).value.integer8));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_INTEGER && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_INTEGER) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(ASPL_ACCESS(a).value.integer32 | ASPL_ACCESS(b).value.integer32));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_LONG && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_LONG) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(ASPL_ACCESS(a).value.integer64 | ASPL_ACCESS(b).value.integer64));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_ENUM_FIELD && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_ENUM_FIELD) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_ENUM_OR(a, b));
            }
            else {
                ASPL_PANIC("Invalid types for the | operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BOOLEAN_XOR: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BOOLEAN && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_BOOLEAN) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BOOL_LITERAL(ASPL_ACCESS(a).value.boolean ^ ASPL_ACCESS(b).value.boolean));
            }
            else {
                ASPL_PANIC("Invalid types for the ^ operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_BITWISE_XOR: {
            ASPL_OBJECT_TYPE b = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_OBJECT_TYPE a = aspl_ailinterpreter_stack_pop(context->stack);
            if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_BYTE && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_BYTE) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BYTE_LITERAL(ASPL_ACCESS(a).value.integer8 ^ ASPL_ACCESS(b).value.integer8));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_INTEGER && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_INTEGER) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_INT_LITERAL(ASPL_ACCESS(a).value.integer32 ^ ASPL_ACCESS(b).value.integer32));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_LONG && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_LONG) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_LONG_LITERAL(ASPL_ACCESS(a).value.integer64 ^ ASPL_ACCESS(b).value.integer64));
            }
            else if (ASPL_ACCESS(a).kind == ASPL_OBJECT_KIND_ENUM_FIELD && ASPL_ACCESS(b).kind == ASPL_OBJECT_KIND_ENUM_FIELD) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_ENUM_XOR(a, b));
            }
            else {
                ASPL_PANIC("Invalid types for the ^ operator");
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_ASSERT: {
            char* file = aspl_ailinterpreter_read_short_string(bytes);
            int line = aspl_ailinterpreter_read_int(bytes);
            int column = aspl_ailinterpreter_read_int(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            ASPL_ASSERT(value, file, line, column);
            break;
        }
        case ASPL_AILI_INSTRUCTION_OFTYPE: {
            ASPL_OBJECT_TYPE object = aspl_ailinterpreter_stack_pop(context->stack);
            char* type = aspl_ailinterpreter_read_short_string(bytes);
            if (ASPL_ACCESS(object).kind == ASPL_OBJECT_KIND_CLASS_INSTANCE) {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_BOOL_LITERAL(aspl_ailinterpreter_util_is_class_child_of(context, ASPL_ACCESS(object).value.classInstance->typePtr, type)));
            }
            else {
                aspl_ailinterpreter_stack_push(context->stack, ASPL_OFTYPE(object, type));
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_SPECIFY_LOOP: {
            long long body_length = aspl_ailinterpreter_read_long(bytes);
            long long full_length = aspl_ailinterpreter_read_long(bytes);
            aspl_ailinterpreter_lms_push(context->loop_meta_stack, (ASPL_AILI_LoopMeta) { .body_end_address = bytes->position + body_length, .full_end_address = bytes->position + full_length });
            break;
        }
        case ASPL_AILI_INSTRUCTION_THROW: {
            ASPL_OBJECT_TYPE error_instance = aspl_ailinterpreter_stack_pop(context->stack);
            aspl_ailinterpreter_return(context, bytes, error_instance);
            break;
        }
        case ASPL_AILI_INSTRUCTION_CATCH: {
            char* variable = aspl_ailinterpreter_read_short_string(bytes);
            long long body_length = aspl_ailinterpreter_read_long(bytes);
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            if (aspl_object_is_error(value)) {
                aspl_ailinterpreter_stack_trace_push(context, "catch_block"); // TODO
                aspl_ailinterpreter_stack_push_frame(context->stack);
                aspl_ailinterpreter_cims_push(context->callable_invocation_meta_stack, (ASPL_AILI_CallableInvocationMeta) { .type = ASPL_AILINTERPRETER_CALLABLE_TYPE_CATCH_BLOCK, .previous_address = bytes->position + body_length, .previous_loop_meta_stack = context->loop_meta_stack });
                if (strlen(variable) > 0) {
                    aspl_ailinterpreter_register_variable(context, variable, value);
                }
            }
            else {
                aspl_ailinterpreter_stack_push(context->stack, value);
                bytes->position += body_length;
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_PROPAGATE_ERROR: {
            ASPL_OBJECT_TYPE value = aspl_ailinterpreter_stack_pop(context->stack);
            if (aspl_object_is_error(value)) {
                int n = context->stack->frames[context->stack->current_frame_index].top;
                for (int i = 0; i < n; i++) {
                    aspl_ailinterpreter_stack_pop(context->stack);
                }
                if (context->callable_invocation_meta_stack->top > 0) {
                    aspl_ailinterpreter_return(context, bytes, value);
                }
                else {
                    ASPL_PANIC("Uncaught %s error", ASPL_ACCESS(value).value.classInstance->typePtr);
                }
            }
            else {
                aspl_ailinterpreter_stack_push(context->stack, value);
            }
            break;
        }
        case ASPL_AILI_INSTRUCTION_POP_LOOP: {
            aspl_ailinterpreter_lms_pop(context->loop_meta_stack);
            break;
        }
        case ASPL_AILI_INSTRUCTION_POP: {
            aspl_ailinterpreter_stack_pop(context->stack);
            break;
        }
        case ASPL_AILI_INSTRUCTION_END:
            if (context->callable_invocation_meta_stack->top > 0) {
                aspl_ailinterpreter_return(context, bytes, ASPL_NULL());
                break;
            }
            else if (context->class_stack->top > 0) {
                context->current_class = aspl_ailinterpreter_class_stack_pop(context->class_stack);
                break;
            }
            else {
                return;
            }
        default:
            ASPL_PANIC("Unknown instruction %d", instruction);
        }
    }
}