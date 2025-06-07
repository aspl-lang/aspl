#include <stddef.h>
#include <limits.h>
#include <stdint.h>

const char base64_encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64_decode_table[CHAR_MAX] = {
    ['A'] =  0, ['B'] =  1, ['C'] =  2, ['D'] =  3,
    ['E'] =  4, ['F'] =  5, ['G'] =  6, ['H'] =  7,
    ['I'] =  8, ['J'] =  9, ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15,
    ['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19,
    ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25,
    ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33,
    ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37,
    ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
    ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45,
    ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
    ['y'] = 50, ['z'] = 51,
    ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55,
    ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
    ['8'] = 60, ['9'] = 61,
    ['+'] = 62, ['/'] = 63
};

char* aspl_util_encoding$base64$encode(const char* input, size_t length) {
    char* encoded_data = NULL;
    size_t encoded_length = 0;

    // Calculate the size of the encoded data
    encoded_length = 4 * ((length + 2) / 3);

    // Allocate memory for the encoded data
    encoded_data = (char*)ASPL_MALLOC(encoded_length + 1);
    if (encoded_data == NULL) {
        return NULL; // Memory allocation error
    }

    // Encoding loop
    size_t i = 0, j = 0;
    while (i < length) {
        uint32_t octet_a = i < length ? input[i++] : 0;
        uint32_t octet_b = i < length ? input[i++] : 0;
        uint32_t octet_c = i < length ? input[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_encode_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_encode_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_encode_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_encode_table[(triple >> 0 * 6) & 0x3F];
    }

    // Add padding if necessary
    for (size_t padding = 0; padding < (3 - (length % 3)) % 3; padding++) {
        encoded_data[encoded_length - 1 - padding] = '=';
    }

    // Null-terminate the encoded data
    encoded_data[encoded_length] = '\0';

    return encoded_data;
}

char* aspl_util_encoding$base64$decode(const char* input, size_t length) {
    char* decoded_data = NULL;
    size_t decoded_length = 0;

    // Calculate the size of the decoded data
    decoded_length = length / 4 * 3;
    if (input[length - 1] == '=') {
        decoded_length--;
    }
    if (input[length - 2] == '=') {
        decoded_length--;
    }

    // Allocate memory for the decoded data
    decoded_data = (char*)ASPL_MALLOC(decoded_length + 1);
    if (decoded_data == NULL) {
        return NULL; // Memory allocation error
    }

    // Decoding loop
    size_t i = 0, j = 0;
    while (i < length) {
        // Accumulate 4 valid characters (ignore everything else)
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : base64_decode_table[(unsigned char)input[i++]];
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : base64_decode_table[(unsigned char)input[i++]];
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : base64_decode_table[(unsigned char)input[i++]];
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : base64_decode_table[(unsigned char)input[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
            + (sextet_b << 2 * 6)
            + (sextet_c << 1 * 6)
            + (sextet_d << 0 * 6);

        if (j < decoded_length) {
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        }
        if (j < decoded_length) {
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        }
        if (j < decoded_length) {
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
        }
    }

    // Null-terminate the decoded data
    decoded_data[decoded_length] = '\0';

    return decoded_data;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$base64$encode(ASPL_OBJECT_TYPE* string) {
    ASPL_OBJECT_TYPE stringObj = (ASPL_OBJECT_TYPE)*string;
    char* encoded = aspl_util_encoding$base64$encode(ASPL_ACCESS(stringObj).value.string->str, ASPL_ACCESS(stringObj).value.string->length);
    return ASPL_STRING_LITERAL(encoded);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_encoding$base64$decode(ASPL_OBJECT_TYPE* string) {
    ASPL_OBJECT_TYPE stringObj = (ASPL_OBJECT_TYPE)*string;
    char* decoded = aspl_util_encoding$base64$decode(ASPL_ACCESS(stringObj).value.string->str, ASPL_ACCESS(stringObj).value.string->length);
    return ASPL_STRING_LITERAL(decoded);
}