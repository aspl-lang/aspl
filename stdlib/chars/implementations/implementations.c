ASPL_OBJECT_TYPE ASPL_IMPLEMENT_chars$int_to_char(ASPL_OBJECT_TYPE* integer){
    ASPL_OBJECT_TYPE integerObj = (ASPL_OBJECT_TYPE)*integer;
    char *string = ASPL_MALLOC(2);
    string[0] = ASPL_ACCESS(integerObj).value.integer8;
    string[1] = '\0';
    return ASPL_STRING_LITERAL(string);
}