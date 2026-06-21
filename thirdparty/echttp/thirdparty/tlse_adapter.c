#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define ECHTTP_TLSE_SOCKT SOCKET
#define ECHTTP_TLSE_close closesocket
#define ECHTTP_TLSE_INVALID_SOCKET INVALID_SOCKET
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#define ECHTTP_TLSE_SOCKT int
#define ECHTTP_TLSE_close close
#define ECHTTP_TLSE_INVALID_SOCKET (-1)
#endif

#ifdef ECHTTP_TLSE_IMPLEMENTATION
#define TLS_AMALGAMATION
#define LTC_NO_ASM
#define NO_SSL_COMPATIBLE_INTERFACE
#define NO_TLS_CURVE25519
#define TLS_MALLOC ECHTTP_MALLOC
#define TLS_REALLOC ECHTTP_REALLOC
#define TLS_FREE ECHTTP_FREE
#include "tlse.c"
#else
#include "tlse.h"
#endif

void echttp_tlse_wrapper_error(char* msg)
{
    perror(msg);
    exit(0);
}

int echttp_tlse_wrapper_send_pending(ECHTTP_TLSE_SOCKT client_sock, struct TLSContext* context)
{
    unsigned int out_buffer_len = 0;
    const unsigned char* out_buffer = tls_get_write_buffer(context, &out_buffer_len);
    unsigned int out_buffer_index = 0;
    int send_res = 0;
    while ((out_buffer) && (out_buffer_len > 0))
    {
        int res = send(client_sock, (char*)&out_buffer[out_buffer_index], out_buffer_len, 0);
        if (res <= 0)
        {
            send_res = res;
            break;
        }
        out_buffer_len -= res;
        out_buffer_index += res;
    }
    tls_buffer_clear(context);
    return send_res;
}

int echttp_tlse_wrapper_validate_certificate(struct TLSContext* context, struct TLSCertificate** certificate_chain, int len)
{
    int i;
    if (certificate_chain)
    {
        for (i = 0; i < len; i++)
        {
            struct TLSCertificate* certificate = certificate_chain[i];
            // TODO: Validate certificate
        }
    }
    // return certificate_expired;
    // return certificate_revoked;
    // return certificate_unknown;
    return no_error;
}

ECHTTP_TLSE_SOCKT echttp_tlse_wrapper_connect_socket(const char* host, int port)
{
    ECHTTP_TLSE_SOCKT sockfd;
    struct addrinfo hints, * servinfo, * p;
    char port_str[6];
    int status;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port_str, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "ERROR, getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == ECHTTP_TLSE_INVALID_SOCKET)
            continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        ECHTTP_TLSE_close(sockfd);
    }

    if (p == NULL)
    {
        fprintf(stderr, "ERROR, failed to connect\n");
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);
    return sockfd;
}

int echttp_tlse_wrapper_connect_tls(ECHTTP_TLSE_SOCKT sockfd, struct TLSContext* context)
{
    int res = tls_client_connect(context);
    echttp_tlse_wrapper_send_pending(sockfd, context);
    unsigned char client_message[0xFFFF];
    for (;;)
    {
        int read_size = recv(sockfd, (char*)client_message, sizeof(client_message), 0);
        tls_consume_stream(context, (const unsigned char*)client_message, read_size, echttp_tlse_wrapper_validate_certificate);
        echttp_tlse_wrapper_send_pending(sockfd, context);
        if (tls_established(context))
        {
            break;
        }
    }
    return res;
}

int echttp_tlse_wrapper_read_tls(ECHTTP_TLSE_SOCKT sockfd, struct TLSContext* context, void* buffer, int len)
{
    unsigned char client_message[0xFFFF];
    int read_res;
    int read_size;
    int read = 0;
    unsigned char* read_buffer = (unsigned char*)ECHTTP_MALLOC(sizeof(unsigned char) * len);
    for (;;)
    {
        if (tls_established(context))
        {
            read_res = tls_read(context, read_buffer, len - read);
            if (read_res > 0)
            {
                memcpy((unsigned char*)buffer + read, read_buffer, read_res);
                read += read_res;
            }
        }
        if (read >= len)
        {
            break;
        }
        read_size = recv(sockfd, (char*)client_message, sizeof(client_message), 0);
        if (read_size <= 0)
        {
            break;
        }
        tls_consume_stream(context, (const unsigned char*)client_message, read_size, echttp_tlse_wrapper_validate_certificate);
        echttp_tlse_wrapper_send_pending(sockfd, context);
    }
    ECHTTP_FREE(read_buffer);
    return read;
}

int echttp_tlse_wrapper_write_tls(ECHTTP_TLSE_SOCKT sockfd, struct TLSContext* context, const unsigned char* data, int len)
{
    int write_res = tls_write(context, data, len);
    if (write_res > 0)
    {
        echttp_tlse_wrapper_send_pending(sockfd, context);
    }
    return write_res;
}