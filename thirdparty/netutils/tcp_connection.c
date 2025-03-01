#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#define SOCKET int
#endif

#ifndef NETUTILS_MALLOC
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#define NETUTILS_MALLOC malloc
#define NETUTILS_REALLOC realloc
#define NETUTILS_FREE free
#endif

#include "tlse_adapter.c"

typedef struct netutils_TcpConnection
{
    SOCKET handle;
    int use_tls;
    struct TLSContext* tls_context;
} netutils_TcpConnection;

netutils_TcpConnection* netutils_tcp_connection_new(char* host, int port, int use_tls, void (*error_callback)(char* a, void* b), void* user_data)
{
    netutils_TcpConnection* connection = NETUTILS_MALLOC(sizeof(netutils_TcpConnection));

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
        error_callback("Failed to get address info", user_data); // TODO: Include reason
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        connection->handle = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (connection->handle == -1)
            continue;

        if (connect(connection->handle, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(connection->handle);
    }

    if (p == NULL)
    {
        error_callback("Failed to connect to server", user_data);
        freeaddrinfo(servinfo);
        return NULL;
    }

    freeaddrinfo(servinfo);

    connection->use_tls = use_tls;
    if (use_tls)
    {
        connection->tls_context = tls_create_context(0, TLS_V12);
        tls_sni_set(connection->tls_context, host); // TODO: Should we use this here? It looks like it is required for "www.google.com" for example
        netutils_tlse_adapter_connect_tls(connection->handle, connection->tls_context);
    }

    return connection;
}

int netutils_tcp_connection_read(netutils_TcpConnection* connection, char* buffer, int length)
{
    int bytes_received;
    if (connection->use_tls)
    {
        bytes_received = netutils_tlse_adapter_read_tls(connection->handle, connection->tls_context, (unsigned char*)buffer, length);
    }
    else
    {
        bytes_received = recv(connection->handle, buffer, length, 0);
    }
    return bytes_received;
}

int netutils_tcp_connection_send(netutils_TcpConnection* connection, char* buffer, int length)
{
    int bytes_sent;
    if (connection->use_tls)
    {
        bytes_sent = netutils_tlse_adapter_write_tls(connection->handle, connection->tls_context, (unsigned char*)buffer, length);
    }
    else
    {
        bytes_sent = send(connection->handle, buffer, length, 0);
    }
    return bytes_sent;
}

void netutils_tcp_connection_close(netutils_TcpConnection* connection)
{
    if (connection->use_tls)
    {
        tls_destroy_context(connection->tls_context);
    }
    close(connection->handle);
    NETUTILS_FREE(connection);
}