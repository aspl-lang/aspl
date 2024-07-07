#ifndef ASPL_AILI_BYTE_LIST_H_INCLUDED
#define ASPL_AILI_BYTE_LIST_H_INCLUDED

#include "instructions.h"

typedef struct ASPL_AILI_ByteList {
    long long size;
    long long position;
    unsigned char* data;
} ASPL_AILI_ByteList;

ASPL_AILI_ByteList* aspl_ailinterpreter_bytelist_clone(ASPL_AILI_ByteList* old);

unsigned char aspl_ailinterpreter_read_byte(ASPL_AILI_ByteList* bytes);

unsigned char aspl_ailinterpreter_peek_byte(ASPL_AILI_ByteList* bytes);

ASPL_AILI_Instruction aspl_ailinterpreter_fetch_instruction(ASPL_AILI_ByteList* bytes);

ASPL_AILI_Instruction aspl_ailinterpreter_peek_instruction(ASPL_AILI_ByteList* bytes);

unsigned char aspl_ailinterpreter_read_bool(ASPL_AILI_ByteList* bytes);

short aspl_ailinterpreter_read_short(ASPL_AILI_ByteList* bytes);

int aspl_ailinterpreter_read_int(ASPL_AILI_ByteList* bytes);

long long aspl_ailinterpreter_read_long(ASPL_AILI_ByteList* bytes);

float aspl_ailinterpreter_read_float(ASPL_AILI_ByteList* bytes);

double aspl_ailinterpreter_read_double(ASPL_AILI_ByteList* bytes);

char* aspl_ailinterpreter_read_short_string(ASPL_AILI_ByteList* bytes);

char* aspl_ailinterpreter_read_long_string(ASPL_AILI_ByteList* bytes);

#endif