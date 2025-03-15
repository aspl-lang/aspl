#ifndef CWSC_H
#define CWSC_H

#include <inttypes.h>

#include "thirdparty/tlse.h"

typedef struct cwsc_Url {
    char protocol[10];
    char host[256];
    char path[1024];
} cwsc_Url;

typedef enum cwsc_Opcode {
    CWSC_OPCODE_CONTINUATION = 0x0,
    CWSC_OPCODE_TEXT = 0x1,
    CWSC_OPCODE_BINARY = 0x2,
    CWSC_OPCODE_CLOSE = 0x8,
    CWSC_OPCODE_PING = 0x9,
    CWSC_OPCODE_PONG = 0xa
} cwsc_Opcode;

typedef struct cwsc_Message {
    cwsc_Opcode opcode;
    unsigned char* message;
    int length;
} cwsc_Message;

typedef struct cwsc_WebSocket {
    void* user_data;
    int sockfd;
    struct TLSContext* tls_context;
    cwsc_Url url;
    void (*on_open)(struct cwsc_WebSocket*);
    void (*on_message)(struct cwsc_WebSocket*, cwsc_Opcode, const unsigned char*, int);
    void (*on_error)(struct cwsc_WebSocket*, const char*, int);
    void (*on_close)(struct cwsc_WebSocket*, int, const char*, int);
} cwsc_WebSocket;

cwsc_Url cwsc_parse_url(const char* input);
int cwsc_init_socket(const char* address, int port);
int cwsc_init_tls(int sockfd, struct TLSContext* tls_context);
int cwsc_connect(cwsc_WebSocket* ws, const char* address, int port);
cwsc_Message cwsc_internal_read_message(cwsc_WebSocket* ws);
unsigned char* cwsc_internal_create_masking_key();
void cwsc_send_control_frame(cwsc_WebSocket* ws, cwsc_Opcode opcode, const char* message, int length);
void cwsc_send_message(cwsc_WebSocket* ws, const char* message);
void cwsc_listen(cwsc_WebSocket* ws);

#endif

#ifdef CWSC_IMPLEMENTATION

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifndef CWSC_MALLOC
#include <stdlib.h>
#define CWSC_MALLOC malloc
#define CWSC_REALLOC realloc
#define CWSC_FREE free
#endif

#include "thirdparty/tlse_adapter.c"

cwsc_Url cwsc_parse_url(const char* input) {
    cwsc_Url toReturn;
    memset(&toReturn, 0, sizeof(cwsc_Url));
    const char* protocolEnd = strstr(input, "://");
    if (protocolEnd == NULL) {
        return toReturn; // Invalid URL
    }
    strncpy(toReturn.protocol, input, protocolEnd - input);
    protocolEnd += 3;

    const char* hostEnd = strchr(protocolEnd, '/');
    if (hostEnd == NULL) {
        strcpy(toReturn.host, protocolEnd);
    }
    else {
        strncpy(toReturn.host, protocolEnd, hostEnd - protocolEnd);
        strcpy(toReturn.path, hostEnd);
    }
    return toReturn;
}

int cwsc_init_socket(const char* address, int port) {
    return cwsc_tlse_wrapper_connect_socket(address, port);
}

int cwsc_init_tls(int sockfd, struct TLSContext* tls_context) {
    cwsc_tlse_wrapper_connect_tls(sockfd, tls_context);
    return 0; // TODO: Don't return 0 if an error occurred
}

int cwsc_connect(cwsc_WebSocket* ws, const char* address, int port) {
    ws->url = cwsc_parse_url(address);
    int sockfd;
    if ((sockfd = cwsc_init_socket(ws->url.host, port)) == -1) {
        return 1;
    }
    ws->sockfd = sockfd;
    if (cwsc_init_tls(sockfd, ws->tls_context) != 0) {
        return 2;
    }

    char request[2048];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Origin: http://localhost\r\n"
        "Sec-WebSocket-Protocol: chat, superchat\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n",
        address, ws->url.host);
    cwsc_tlse_wrapper_write_tls(sockfd, ws->tls_context, (const unsigned char*)request, strlen(request));

    char handshakeBuffer[1024];
    int totalBytesRead = 0;
    char buffer[1];
    for (totalBytesRead = 0; totalBytesRead < 1024; totalBytesRead++) {
        int bytesRead = cwsc_tlse_wrapper_read_tls(sockfd, ws->tls_context, buffer, 1);
        if (bytesRead == 0) {
            return 3;
        }
        handshakeBuffer[totalBytesRead] = buffer[0];
        if (totalBytesRead > 4 && handshakeBuffer[totalBytesRead] == '\n' && handshakeBuffer[totalBytesRead - 1] == '\r' && handshakeBuffer[totalBytesRead - 2] == '\n' && handshakeBuffer[totalBytesRead - 3] == '\r') {
            break;
        }
    }
    handshakeBuffer[totalBytesRead + 1] = '\0';
    // TODO: Validate handshake

    if (ws->on_open) {
        ws->on_open(ws);
    }

    return 0;
}

cwsc_Message cwsc_internal_read_message(cwsc_WebSocket* ws) {
    int sockfd = ws->sockfd;
    struct TLSContext* tls_context = ws->tls_context;
    char socketBuffer[2];
    int bytesReceived1 = cwsc_tlse_wrapper_read_tls(sockfd, tls_context, socketBuffer, sizeof(socketBuffer));
    cwsc_Opcode opcode = (cwsc_Opcode)(socketBuffer[0] & 0b00001111); // get the four least significant bits
    unsigned char hasMask = (socketBuffer[1] & 0b10000000) == 0b10000000;
    uint8_t payloadLengthSimple = socketBuffer[1] & 0b01111111; // get the seven least significant bits
    uint64_t payloadLength;
    if (payloadLengthSimple <= 125) {
        payloadLength = payloadLengthSimple;
    }
    else if (payloadLengthSimple == 126) {
        uint8_t payloadLengthBuffer[2];
        cwsc_tlse_wrapper_read_tls(sockfd, tls_context, payloadLengthBuffer, sizeof(payloadLengthBuffer));
        payloadLength = (uint64_t)payloadLengthBuffer[0] << 8;
        payloadLength += (uint64_t)payloadLengthBuffer[1];
    }
    else if (payloadLengthSimple == 127) {
        uint8_t payloadLengthBuffer[8];
        cwsc_tlse_wrapper_read_tls(sockfd, tls_context, payloadLengthBuffer, sizeof(payloadLengthBuffer));
        payloadLength = (uint64_t)payloadLengthBuffer[0] << 56;
        payloadLength += (uint64_t)payloadLengthBuffer[1] << 48;
        payloadLength += (uint64_t)payloadLengthBuffer[2] << 40;
        payloadLength += (uint64_t)payloadLengthBuffer[3] << 32;
        payloadLength += (uint64_t)payloadLengthBuffer[4] << 24;
        payloadLength += (uint64_t)payloadLengthBuffer[5] << 16;
        payloadLength += (uint64_t)payloadLengthBuffer[6] << 8;
        payloadLength += (uint64_t)payloadLengthBuffer[7];
    }
    char maskingKey[4];
    if (hasMask) {
        cwsc_tlse_wrapper_read_tls(sockfd, tls_context, maskingKey, sizeof(maskingKey)); // TODO: Check if this is correct
    }
    unsigned char* textBuffer = (unsigned char*)CWSC_MALLOC(payloadLength + 1);
    uint32_t bytesReceived = 0;
    while (bytesReceived < payloadLength) {
        if (ws->sockfd == -1) {
            cwsc_Message message;
            message.opcode = -1;
            return message;
        }
        bytesReceived += cwsc_tlse_wrapper_read_tls(sockfd, tls_context, textBuffer + bytesReceived, payloadLength - bytesReceived);
    }
    if (hasMask) {
        for (uint32_t i = 0; i < payloadLength; i++) {
            textBuffer[i] ^= maskingKey[i % 4] & 0xff;
        }
    }
    textBuffer[payloadLength] = '\0';
    cwsc_Message message;
    message.opcode = opcode;
    message.message = textBuffer;
    message.length = payloadLength;
    return message;
}

unsigned char* cwsc_internal_create_masking_key() {
    unsigned char* key = (unsigned char*)CWSC_MALLOC(4);
    key[0] = (unsigned char)rand();
    key[1] = (unsigned char)rand();
    key[2] = (unsigned char)rand();
    key[3] = (unsigned char)rand();
    return key;
}

void cwsc_send_control_frame(cwsc_WebSocket* ws, cwsc_Opcode opcode, const char* message, int length) {
    int headerLength = 6;
    int frameLength = headerLength + length;
    unsigned char* frame = (unsigned char*)CWSC_MALLOC(frameLength);
    unsigned char* maskingKey = cwsc_internal_create_masking_key();
    frame[0] = 0b10000000 + opcode;
    frame[1] = (uint8_t)length;
    if (opcode == CWSC_OPCODE_CLOSE) {
        if (length >= 2) {
            for (int i = 0; i < length; i++) {
                frame[6 + i] = (message[i] ^ maskingKey[i % 4]) & 0xff;
            }
        }
    }
    else {
        for (int i = 0; i < length; i++) {
            frame[headerLength + i] = (message[i] ^ maskingKey[i % 4]) & 0xff;
        }
    }
    cwsc_tlse_wrapper_write_tls(ws->sockfd, ws->tls_context, frame, frameLength);
    CWSC_FREE(maskingKey);
    CWSC_FREE(frame);
}

void cwsc_send_message(cwsc_WebSocket* ws, const char* message) {
    // Six bytes for framing data, four bytes for length data (if necessary), and one character for each byte
    size_t messageLen = strlen(message);
    char toSend[10 + messageLen];
    // Set message to not fragmented, type as text, and no special flags
    toSend[0] = 0b10000001;
    uint8_t offset = 0;
    // Set the mask to true on each of these (first bit set to 1)
    if (messageLen <= 125) {
        toSend[1] = 0b10000000 + messageLen;
    }
    else if (messageLen >= 125 && messageLen <= SHRT_MAX) {
        // Set mask to true and length to 126
        toSend[1] = 0b11111110;
        toSend[2] = (uint8_t)(messageLen >> 8);
        toSend[3] = (uint8_t)(messageLen);
        offset = 2;
    }
    else {
        // Set the length to 127 and the mask to true
        toSend[1] = 0xff;
        toSend[2] = (uint8_t)(messageLen >> 56);
        toSend[3] = (uint8_t)(messageLen >> 48);
        toSend[4] = (uint8_t)(messageLen >> 40);
        toSend[5] = (uint8_t)(messageLen >> 32);
        toSend[6] = (uint8_t)(messageLen >> 24);
        toSend[7] = (uint8_t)(messageLen >> 16);
        toSend[8] = (uint8_t)(messageLen >> 8);
        toSend[9] = (uint8_t)(messageLen);
        offset = 8;
    }
    // Generate the masking key
    unsigned char* masking_key = cwsc_internal_create_masking_key();
    // Copy the masking key to the toSend array
    for (size_t i = 0; i < 4; i++) {
        toSend[2 + offset + i] = masking_key[i];
    }
    CWSC_FREE(masking_key);

    // Add the data to the toSend array
    for (size_t i = 0; i < messageLen; i++) {
        // XOR the byte with the masking key to "mask" the message
        toSend[6 + offset + i] = message[i] ^ toSend[2 + offset + (i % 4)];
    }

    cwsc_tlse_wrapper_write_tls(ws->sockfd, ws->tls_context, (const unsigned char*)toSend, offset + 6 + messageLen);
}

void cwsc_listen(cwsc_WebSocket* ws) {
    while (1) {
        cwsc_Message message = cwsc_internal_read_message(ws);
        switch (message.opcode) {
        case CWSC_OPCODE_CONTINUATION:
            // TODO: Report an error
            break;
        case CWSC_OPCODE_TEXT:
        case CWSC_OPCODE_BINARY:
            if (ws->on_message) {
                ws->on_message(ws, message.opcode, message.message, message.length);
            }
            break;
        case CWSC_OPCODE_CLOSE:
            if (ws->on_close) {
                if (message.length >= 2) {
                    uint16_t code = (uint16_t)message.message[0] << 8 | (uint16_t)message.message[1];
                    ws->on_close(ws, code, (const char*)(message.message + 2), message.length - 2);
                }
                else {
                    ws->on_close(ws, 0, "", 0);
                }
            }
            cwsc_send_control_frame(ws, CWSC_OPCODE_CLOSE, (const char*)message.message, message.length);
            close(ws->sockfd);
            ws->sockfd = -1;
            break;
        case CWSC_OPCODE_PING:
            cwsc_send_control_frame(ws, CWSC_OPCODE_PONG, (const char*)message.message, message.length);
            break;
        case CWSC_OPCODE_PONG:
            // ignore for now
            break;
        }
        CWSC_FREE(message.message);
        if (ws->sockfd == -1) {
            break;
        }
    }
    return;
}

void cwsc_disconnect(cwsc_WebSocket* ws, const char* message, int code) {
    if (ws->sockfd <= 0) {
        if (ws->on_error) {
            ws->on_error(ws, "Socket already closed", -1);
        }
        return;
    }

    int message_len = (message != NULL) ? strlen(message) : 0;
    int close_frame_len = 2 + message_len;
    unsigned char* close_frame = (unsigned char*)CWSC_MALLOC(close_frame_len);

    close_frame[0] = (unsigned char)(code >> 8);
    close_frame[1] = (unsigned char)(code & 0xFF);

    if (message) {
        memcpy(close_frame + 2, message, message_len);
    }

    cwsc_send_control_frame(ws, CWSC_OPCODE_CLOSE, (const char*)close_frame, close_frame_len);

    CWSC_FREE(close_frame);

    close(ws->sockfd);
    ws->sockfd = -1;

    if (ws->on_close) {
        ws->on_close(ws, code, message, message_len);
    }
}

#endif