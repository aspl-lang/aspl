#include "interpreter.h"

void aspl_ailinterpreter_print(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE object = arguments.arguments[0];
    ASPL_OBJECT_TYPE newLine = arguments.arguments[1];
    aspl_function_print(C_REFERENCE(object), C_REFERENCE(newLine));
    aspl_ailinterpreter_return(context, bytes, ASPL_NULL());
}

void aspl_ailinterpreter_key(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE prompt = arguments.arguments[0];
    // TODO: Implement key when it's implemented in the C backend
}

void aspl_ailinterpreter_input(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE prompt = arguments.arguments[0];
    aspl_ailinterpreter_return(context, bytes, aspl_function_input(C_REFERENCE(prompt)));
}

void aspl_ailinterpreter_exit(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE code = arguments.arguments[0];
    aspl_function_exit(C_REFERENCE(code));
}

void aspl_ailinterpreter_range(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE start = arguments.arguments[0];
    ASPL_OBJECT_TYPE end = arguments.arguments[1];
    // TODO: Implement range when it's implemented in the C backend
}

void aspl_ailinterpreter_panic(ASPL_AILI_ThreadContext* context, ASPL_AILI_ByteList* bytes, char* identifier, ASPL_AILI_ArgumentList arguments) {
    ASPL_OBJECT_TYPE object = arguments.arguments[0];
    ASPL_OBJECT_TYPE newLine = arguments.arguments[1];
    aspl_function_panic(C_REFERENCE(object));
    aspl_ailinterpreter_return(context, bytes, ASPL_NULL());
}

void aspl_ailinterpreter_initialize_builtin_functions(ASPL_AILI_EnvironmentContext* context) {
    hashmap_str_to_voidptr_hashmap_set(context->functions, "_print", aspl_ailinterpreter_print);
    ASPL_AILI_BuiltinFunction* print = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    print->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    print->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "object", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "any" }, .size = 1 }, .optional = 0 };
    print->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "newLine", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "bool" }, .size = 1 }, .optional = 1, .default_value = ASPL_TRUE() };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_print", print);

    hashmap_str_to_voidptr_hashmap_set(context->functions, "_key", aspl_ailinterpreter_key);
    ASPL_AILI_BuiltinFunction* key = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    key->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    key->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "prompt", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 1, .default_value = ASPL_STRING_LITERAL("") };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_key", key);

    hashmap_str_to_voidptr_hashmap_set(context->functions, "_input", aspl_ailinterpreter_input);
    ASPL_AILI_BuiltinFunction* input = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    input->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    input->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "prompt", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 1, .default_value = ASPL_STRING_LITERAL("") };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_input", input);

    hashmap_str_to_voidptr_hashmap_set(context->functions, "_exit", aspl_ailinterpreter_exit);
    ASPL_AILI_BuiltinFunction* exit = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    exit->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    exit->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "code", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 1, .default_value = ASPL_INT_LITERAL(0) };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_exit", exit);

    hashmap_str_to_voidptr_hashmap_set(context->functions, "_range", aspl_ailinterpreter_range);
    ASPL_AILI_BuiltinFunction* range = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    range->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    range->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "start", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "byte", "int", "long", "float", "double" }, .size = 5 }, .optional = 0 };
    range->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "end", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "byte", "int", "long", "float", "double" }, .size = 5 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_range", range);

    hashmap_str_to_voidptr_hashmap_set(context->functions, "_panic", aspl_ailinterpreter_panic);
    ASPL_AILI_BuiltinFunction* panic = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinFunction));
    panic->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    panic->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "object", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "any" }, .size = 1 }, .optional = 0 };
    panic->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "newLine", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "bool" }, .size = 1 }, .optional = 1, .default_value = ASPL_TRUE() };
    hashmap_str_to_voidptr_hashmap_set(context->builtin_functions, "_panic", panic);
}

void aspl_ailinterpreter_initialize_builtin_methods(ASPL_AILI_EnvironmentContext* context) {
    hashmap_str_to_voidptr_HashMap* any_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 2 });

    ASPL_AILI_BuiltinMethod* any_cloneShallow = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    any_cloneShallow->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(any_methods, "cloneShallow", any_cloneShallow);

    ASPL_AILI_BuiltinMethod* any_cloneDeep = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    any_cloneDeep->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(any_methods, "cloneDeep", any_cloneDeep);

    hashmap_str_to_voidptr_hashmap_set(context->builtin_methods, "any", any_methods);

    hashmap_str_to_voidptr_HashMap* string_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 13 });

    ASPL_AILI_BuiltinMethod* string_toLower = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_toLower->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "toLower", string_toLower);

    ASPL_AILI_BuiltinMethod* string_toUpper = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_toUpper->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "toUpper", string_toUpper);

    ASPL_AILI_BuiltinMethod* string_replace = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_replace->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    string_replace->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "search", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    string_replace->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "replace", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "replace", string_replace);

    ASPL_AILI_BuiltinMethod* string_startsWith = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_startsWith->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_startsWith->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "substring", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "startsWith", string_startsWith);

    ASPL_AILI_BuiltinMethod* string_endsWith = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_endsWith->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_endsWith->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "substring", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "endsWith", string_endsWith);

    ASPL_AILI_BuiltinMethod* string_contains = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_contains->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_contains->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "substring", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "contains", string_contains);

    ASPL_AILI_BuiltinMethod* string_after = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_after->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_after->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "position", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "after", string_after);

    ASPL_AILI_BuiltinMethod* string_before = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_before->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_before->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "position", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "before", string_before);

    ASPL_AILI_BuiltinMethod* string_trim = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_trim->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    string_trim->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "chars", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 1, .default_value = ASPL_STRING_LITERAL(" \n\t\v\f\r") };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "trim", string_trim);

    ASPL_AILI_BuiltinMethod* string_trimStart = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_trimStart->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "trimStart", string_trimStart);

    ASPL_AILI_BuiltinMethod* string_trimEnd = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_trimEnd->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "trimEnd", string_trimEnd);

    ASPL_AILI_BuiltinMethod* string_split = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_split->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    string_split->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "delimiter", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    string_split->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "maxParts", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 1, .default_value = ASPL_INT_LITERAL(-1) };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "split", string_split);

    ASPL_AILI_BuiltinMethod* string_reverse = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    string_reverse->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(string_methods, "reverse", string_reverse);
    hashmap_str_to_voidptr_hashmap_set(context->builtin_methods, "string", string_methods);

    hashmap_str_to_voidptr_hashmap_set(context->builtin_methods, "string", string_methods);

    hashmap_str_to_voidptr_HashMap* list_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 8 });

    ASPL_AILI_BuiltinMethod* list_contains = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_contains->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    list_contains->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "element", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "contains", list_contains);

    ASPL_AILI_BuiltinMethod* list_add = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_add->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    list_add->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "element", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "add", list_add);

    ASPL_AILI_BuiltinMethod* list_insert = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_insert->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    list_insert->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "index", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 0 };
    list_insert->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "element", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "insert", list_insert);

    ASPL_AILI_BuiltinMethod* list_insertElements = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_insertElements->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 2), .size = 2 };
    list_insertElements->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "index", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 0 };
    list_insertElements->parameters.parameters[1] = (ASPL_AILI_Parameter){ .name = "elements", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "list<T>" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "insertElements", list_insertElements);

    ASPL_AILI_BuiltinMethod* list_remove = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_remove->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    list_remove->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "element", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "remove", list_remove);

    ASPL_AILI_BuiltinMethod* list_removeAt = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_removeAt->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    list_removeAt->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "index", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "int" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "removeAt", list_removeAt);

    ASPL_AILI_BuiltinMethod* list_clear = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_clear->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "clear", list_clear);

    ASPL_AILI_BuiltinMethod* list_join = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    list_join->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    list_join->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "delimiter", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "string" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(list_methods, "join", list_join);

    hashmap_str_to_voidptr_hashmap_set(context->builtin_methods, "list", list_methods);

    hashmap_str_to_voidptr_HashMap* map_methods = hashmap_str_to_voidptr_new_hashmap((hashmap_str_to_voidptr_HashMapConfig) { .initial_capacity = 3 });

    ASPL_AILI_BuiltinMethod* map_containsKey = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    map_containsKey->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    map_containsKey->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "key", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(map_methods, "containsKey", map_containsKey);

    ASPL_AILI_BuiltinMethod* map_remove = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    map_remove->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 1), .size = 1 };
    map_remove->parameters.parameters[0] = (ASPL_AILI_Parameter){ .name = "key", .expected_types = (ASPL_AILI_TypeList) {.types = (char* []) { "T" }, .size = 1 }, .optional = 0 };
    hashmap_str_to_voidptr_hashmap_set(map_methods, "remove", map_remove);

    ASPL_AILI_BuiltinMethod* map_clear = ASPL_MALLOC(sizeof(ASPL_AILI_BuiltinMethod));
    map_clear->parameters = (ASPL_AILI_ParameterList){ .parameters = ASPL_MALLOC(sizeof(ASPL_AILI_Parameter) * 0), .size = 0 };
    hashmap_str_to_voidptr_hashmap_set(map_methods, "clear", map_clear);

    hashmap_str_to_voidptr_hashmap_set(context->builtin_methods, "map", map_methods);
}