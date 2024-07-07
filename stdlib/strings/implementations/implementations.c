typedef struct ASPL_StringBuilder
{
    char* buffer;
    int length;
    int capacity;
} ASPL_StringBuilder;

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$new()
{
    ASPL_StringBuilder* builder = ASPL_MALLOC(sizeof(struct ASPL_StringBuilder));
    builder->buffer = ASPL_MALLOC(sizeof(char) * 16);
    builder->length = 0;
    builder->capacity = 16;
    return ASPL_HANDLE_LITERAL(builder);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$get_length(ASPL_OBJECT_TYPE* builder)
{
    return ASPL_INT_LITERAL(((ASPL_StringBuilder*)ASPL_ACCESS(*builder).value.handle)->length);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$append_string(ASPL_OBJECT_TYPE* builder, ASPL_OBJECT_TYPE* string)
{
    ASPL_StringBuilder* builder_value = (ASPL_StringBuilder*)ASPL_ACCESS(*builder).value.handle;
    int string_length = ASPL_ACCESS(*string).value.string->length;
    int new_length = builder_value->length + string_length;
    if (new_length > builder_value->capacity)
    {
        int new_capacity = builder_value->capacity * 2;
        while (new_capacity < new_length)
        {
            new_capacity *= 2;
        }
        char* new_buffer = ASPL_MALLOC(sizeof(char) * new_capacity);
        memcpy(new_buffer, builder_value->buffer, builder_value->length);
        ASPL_FREE(builder_value->buffer); // TODO: Figure out if such calls are related to these random & weird memory corruptions that can occur in production mode
        builder_value->buffer = new_buffer;
        builder_value->capacity = new_capacity;
    }
    memcpy(builder_value->buffer + builder_value->length, ASPL_ACCESS(*string).value.string->str, string_length);
    builder_value->length = new_length;
    return *builder;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$append_string_builder(ASPL_OBJECT_TYPE* a, ASPL_OBJECT_TYPE* b)
{
    ASPL_StringBuilder* a_value = (ASPL_StringBuilder*)ASPL_ACCESS(*a).value.handle;
    ASPL_StringBuilder* b_value = (ASPL_StringBuilder*)ASPL_ACCESS(*b).value.handle;
    int new_length = a_value->length + b_value->length;
    if (new_length > a_value->capacity)
    {
        int new_capacity = a_value->capacity * 2;
        while (new_capacity < new_length)
        {
            new_capacity *= 2;
        }
        char* new_buffer = ASPL_MALLOC(sizeof(char) * new_capacity);
        memcpy(new_buffer, a_value->buffer, a_value->length);
        //ASPL_FREE(a_value->buffer);
        a_value->buffer = new_buffer;
        a_value->capacity = new_capacity;
    }
    memcpy(a_value->buffer + a_value->length, b_value->buffer, b_value->length);
    a_value->length = new_length;
    return *a;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$flush(ASPL_OBJECT_TYPE* builder)
{
    ASPL_StringBuilder* builder_value = (ASPL_StringBuilder*)ASPL_ACCESS(*builder).value.handle;
    //ASPL_FREE(builder_value->buffer);
    builder_value->buffer = ASPL_MALLOC(sizeof(char) * 16);
    builder_value->length = 0;
    builder_value->capacity = 16;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_strings$string_builder$to_string_flush(ASPL_OBJECT_TYPE* builder)
{
    ASPL_StringBuilder* builder_value = (ASPL_StringBuilder*)ASPL_ACCESS(*builder).value.handle;
    char* buffer = ASPL_MALLOC(sizeof(char) * (builder_value->length + 1));
    memcpy(buffer, builder_value->buffer, builder_value->length);
    buffer[builder_value->length] = '\0';
    //ASPL_FREE(builder_value->buffer);
    builder_value->buffer = ASPL_MALLOC(sizeof(char) * 16);
    builder_value->length = 0;
    builder_value->capacity = 16;
    return ASPL_STRING_LITERAL_NO_COPY(buffer);
}