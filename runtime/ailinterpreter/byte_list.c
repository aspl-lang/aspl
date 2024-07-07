#include <stdint.h>

#include "byte_list.h"

ASPL_AILI_ByteList* aspl_ailinterpreter_bytelist_clone(ASPL_AILI_ByteList* old) {
    ASPL_AILI_ByteList* new = ASPL_MALLOC(sizeof(ASPL_AILI_ByteList));
    *new = *old;
    return new;
}

unsigned char aspl_ailinterpreter_read_byte(ASPL_AILI_ByteList* bytes) {
    return bytes->data[bytes->position++];
}

unsigned char aspl_ailinterpreter_peek_byte(ASPL_AILI_ByteList* bytes) {
    return bytes->data[bytes->position];
}

ASPL_AILI_Instruction aspl_ailinterpreter_fetch_instruction(ASPL_AILI_ByteList* bytes) {
    return aspl_ailinterpreter_read_byte(bytes);
}

ASPL_AILI_Instruction aspl_ailinterpreter_peek_instruction(ASPL_AILI_ByteList* bytes) {
    return aspl_ailinterpreter_peek_byte(bytes);
}

unsigned char aspl_ailinterpreter_read_bool(ASPL_AILI_ByteList* bytes) {
    return bytes->data[bytes->position++];
}

short aspl_ailinterpreter_read_short(ASPL_AILI_ByteList* bytes) {
    short result = 0;
    for (int i = 0; i < 2; i++) {
        result |= (unsigned short)aspl_ailinterpreter_read_byte(bytes) << (8 * i);
    }
    return result;
}

int aspl_ailinterpreter_read_int(ASPL_AILI_ByteList* bytes) {
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result |= (unsigned int)aspl_ailinterpreter_read_byte(bytes) << (8 * i);
    }
    return result;
}

long long aspl_ailinterpreter_read_long(ASPL_AILI_ByteList* bytes) {
    long long result = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char byte = aspl_ailinterpreter_read_byte(bytes);
        result |= ((unsigned long long)byte) << (8 * i);
    }
    return result;
}

float aspl_ailinterpreter_read_float(ASPL_AILI_ByteList* bytes) {
    uint32_t integerResult = 0;
    for (int i = 0; i < 4; i++) {
        integerResult |= ((uint32_t)aspl_ailinterpreter_read_byte(bytes)) << (8 * i);
    }
    float result;
    memcpy(&result, &integerResult, sizeof(float)); // use memcpy to avoid endianess issues
    return result;
}

double aspl_ailinterpreter_read_double(ASPL_AILI_ByteList* bytes) {
    uint64_t integerResult = 0;
    for (int i = 0; i < 8; i++) {
        integerResult |= ((uint64_t)aspl_ailinterpreter_read_byte(bytes)) << (8 * i);
    }
    double result;
    memcpy(&result, &integerResult, sizeof(double)); // use memcpy to avoid endianess issues
    return result;
}

char* aspl_ailinterpreter_read_short_string(ASPL_AILI_ByteList* bytes) {
    short length = aspl_ailinterpreter_read_short(bytes);
    char* result = ASPL_MALLOC(sizeof(char) * (length + 1));
    for (int i = 0; i < length; i++) {
        result[i] = aspl_ailinterpreter_read_byte(bytes);
    }
    result[length] = '\0';
    return result;
}

char* aspl_ailinterpreter_read_long_string(ASPL_AILI_ByteList* bytes) {
    long long length = aspl_ailinterpreter_read_long(bytes);
    char* result = ASPL_MALLOC(sizeof(char) * (length + 1));
    for (int i = 0; i < length; i++) {
        result[i] = aspl_ailinterpreter_read_byte(bytes);
    }
    result[length] = '\0';
    return result;
}