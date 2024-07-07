ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$utf8$encode(ASPL_OBJECT_TYPE* string) {
    ASPL_OBJECT_TYPE stringObj = (ASPL_OBJECT_TYPE)*string;
    ASPL_OBJECT_TYPE* bytes = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * ASPL_ACCESS(stringObj).value.string->length);
    for (int i = 0; i < ASPL_ACCESS(stringObj).value.string->length; i++)
    {
        bytes[i] = ASPL_BYTE_LITERAL(ASPL_ACCESS(stringObj).value.string->str[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytes, ASPL_ACCESS(stringObj).value.string->length);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$utf8$encode_char_to_int(ASPL_OBJECT_TYPE* c) {
    ASPL_OBJECT_TYPE charObj = (ASPL_OBJECT_TYPE)*c;
    int length = 0;
    int value = 0;
    for (int i = 0; i < ASPL_ACCESS(charObj).value.string->length; i++)
    {
        if (length == 0)
        {
            if ((ASPL_ACCESS(charObj).value.string->str[i] & 0b10000000) == 0b00000000)
            {
                value = ASPL_ACCESS(charObj).value.string->str[i];
                length = 0;
            }
            else if ((ASPL_ACCESS(charObj).value.string->str[i] & 0b11100000) == 0b11000000)
            {
                value = ASPL_ACCESS(charObj).value.string->str[i] & 0b00011111;
                length = 1;
            }
            else if ((ASPL_ACCESS(charObj).value.string->str[i] & 0b11110000) == 0b11100000)
            {
                value = ASPL_ACCESS(charObj).value.string->str[i] & 0b00001111;
                length = 2;
            }
            else if ((ASPL_ACCESS(charObj).value.string->str[i] & 0b11111000) == 0b11110000)
            {
                value = ASPL_ACCESS(charObj).value.string->str[i] & 0b00000111;
                length = 3;
            }
        }
        else
        {
            value = (value << 6) | (ASPL_ACCESS(charObj).value.string->str[i] & 0b00111111);
            length--;
        }
    }
    return ASPL_INT_LITERAL(value);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$utf8$decode(ASPL_OBJECT_TYPE* bytes) {
    ASPL_OBJECT_TYPE bytesObj = (ASPL_OBJECT_TYPE)*bytes;
    char* string = ASPL_MALLOC(ASPL_ACCESS(bytesObj).value.list->length + 1);
    for (int i = 0; i < ASPL_ACCESS(bytesObj).value.list->length; i++)
    {
        string[i] = ASPL_ACCESS(ASPL_ACCESS(bytesObj).value.list->value[i]).value.integer8;
    }
    string[ASPL_ACCESS(bytesObj).value.list->length] = '\0';
    return ASPL_STRING_LITERAL(string);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$utf8$decode_int_to_char(ASPL_OBJECT_TYPE* i) {
    ASPL_OBJECT_TYPE intObj = (ASPL_OBJECT_TYPE)*i;
    char* string = ASPL_MALLOC(5);
    if (ASPL_ACCESS(intObj).value.integer32 < 0x80)
    {
        string[0] = ASPL_ACCESS(intObj).value.integer32;
        string[1] = '\0';
        return ASPL_STRING_LITERAL(string);
    }
    else if (ASPL_ACCESS(intObj).value.integer32 < 0x800)
    {
        string[0] = 0b11000000 | (ASPL_ACCESS(intObj).value.integer32 >> 6);
        string[1] = 0b10000000 | (ASPL_ACCESS(intObj).value.integer32 & 0b00111111);
        string[2] = '\0';
        return ASPL_STRING_LITERAL(string);
    }
    else if (ASPL_ACCESS(intObj).value.integer32 < 0x10000)
    {
        string[0] = 0b11100000 | (ASPL_ACCESS(intObj).value.integer32 >> 12);
        string[1] = 0b10000000 | ((ASPL_ACCESS(intObj).value.integer32 >> 6) & 0b00111111);
        string[2] = 0b10000000 | (ASPL_ACCESS(intObj).value.integer32 & 0b00111111);
        string[3] = '\0';
        return ASPL_STRING_LITERAL(string);
    }
    else if (ASPL_ACCESS(intObj).value.integer32 < 0x110000)
    {
        string[0] = 0b11110000 | (ASPL_ACCESS(intObj).value.integer32 >> 18);
        string[1] = 0b10000000 | ((ASPL_ACCESS(intObj).value.integer32 >> 12) & 0b00111111);
        string[2] = 0b10000000 | ((ASPL_ACCESS(intObj).value.integer32 >> 6) & 0b00111111);
        string[3] = 0b10000000 | (ASPL_ACCESS(intObj).value.integer32 & 0b00111111);
        string[4] = '\0';
        return ASPL_STRING_LITERAL(string);
    }
    else
    {
        ASPL_PANIC("Invalid UTF-8 code point %d", ASPL_ACCESS(intObj).value.integer32);
    }
}