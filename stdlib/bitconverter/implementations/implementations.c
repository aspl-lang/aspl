ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$short_to_bytes(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    char* bytes = (char*)&ASPL_ACCESS(valueObj).value.integer32;
    ASPL_OBJECT_TYPE* bytesObj = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 2);
    for (int i = 0; i < 2; i++)
    {
        bytesObj[i] = ASPL_BYTE_LITERAL(bytes[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytesObj, 2);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$bytes_to_short(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* cBytes = ASPL_MALLOC(sizeof(char) * 2);
    cBytes[0] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[0]).value.integer8;
    cBytes[1] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[1]).value.integer8;
    return ASPL_INT_LITERAL(*(short*)&cBytes);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$int_to_bytes(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    char* bytes = (char*)&ASPL_ACCESS(valueObj).value.integer32;
    ASPL_OBJECT_TYPE* bytesObj = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 4);
    for (int i = 0; i < 4; i++)
    {
        bytesObj[i] = ASPL_BYTE_LITERAL(bytes[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytesObj, 4);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$bytes_to_int(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* cBytes = ASPL_MALLOC(sizeof(char) * 4);
    cBytes[0] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[0]).value.integer8;
    cBytes[1] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[1]).value.integer8;
    cBytes[2] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[2]).value.integer8;
    cBytes[3] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[3]).value.integer8;
    return ASPL_INT_LITERAL(*(int*)&cBytes);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$long_to_bytes(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    char* bytes = (char*)&ASPL_ACCESS(valueObj).value.integer64;
    ASPL_OBJECT_TYPE* bytesObj = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 8);
    for (int i = 0; i < 8; i++)
    {
        bytesObj[i] = ASPL_BYTE_LITERAL(bytes[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytesObj, 8);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$bytes_to_long(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* cBytes = ASPL_MALLOC(sizeof(char) * 8);
    cBytes[0] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[0]).value.integer8;
    cBytes[1] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[1]).value.integer8;
    cBytes[2] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[2]).value.integer8;
    cBytes[3] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[3]).value.integer8;
    cBytes[4] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[4]).value.integer8;
    cBytes[5] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[5]).value.integer8;
    cBytes[6] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[6]).value.integer8;
    cBytes[7] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[7]).value.integer8;
    return ASPL_LONG_LITERAL(*(long*)&cBytes);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$float_to_bytes(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    char* bytes = (char*)&ASPL_ACCESS(valueObj).value.float32;
    ASPL_OBJECT_TYPE* bytesObj = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 4);
    for (int i = 0; i < 4; i++)
    {
        bytesObj[i] = ASPL_BYTE_LITERAL(bytes[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytesObj, 4);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$bytes_to_float(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* cBytes = ASPL_MALLOC(sizeof(char) * 4);
    cBytes[0] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[0]).value.integer8;
    cBytes[1] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[1]).value.integer8;
    cBytes[2] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[2]).value.integer8;
    cBytes[3] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[3]).value.integer8;
    return ASPL_FLOAT_LITERAL(*(float*)&cBytes);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$double_to_bytes(ASPL_OBJECT_TYPE* value) {
    ASPL_OBJECT_TYPE valueObj = (ASPL_OBJECT_TYPE)*value;
    char* bytes = (char*)&ASPL_ACCESS(valueObj).value.float64;
    ASPL_OBJECT_TYPE* bytesObj = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 8);
    for (int i = 0; i < 8; i++)
    {
        bytesObj[i] = ASPL_BYTE_LITERAL(bytes[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytesObj, 8);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_bitconverter$convert$bytes_to_double(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* cBytes = ASPL_MALLOC(sizeof(char) * 8);
    cBytes[0] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[0]).value.integer8;
    cBytes[1] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[1]).value.integer8;
    cBytes[2] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[2]).value.integer8;
    cBytes[3] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[3]).value.integer8;
    cBytes[4] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[4]).value.integer8;
    cBytes[5] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[5]).value.integer8;
    cBytes[6] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[6]).value.integer8;
    cBytes[7] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[7]).value.integer8;
    return ASPL_DOUBLE_LITERAL(*(double*)&cBytes);
}