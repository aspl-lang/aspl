// TODO: Test if this works

const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

        encoded_data[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
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
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : base64_table[(unsigned char)input[i++]];
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : base64_table[(unsigned char)input[i++]];
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : base64_table[(unsigned char)input[i++]];
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : base64_table[(unsigned char)input[i++]];

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