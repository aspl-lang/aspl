#ifndef ECHTTP_H
#define ECHTTP_H

#include <stddef.h>

typedef struct echttp_Header
{
    char const* name;
    size_t value_count;
    char const** values;
} echttp_Header;

typedef struct echttp_Response
{
    void* _handle; // TODO: Type annotate this
    int status_code;
    char const* http_version;
    char const* reason_phrase;
    size_t header_count;
    echttp_Header* headers;
    size_t response_size;
    char const* data;
    size_t full_response_size; // includes the raw header lines as well as the actual data
    char const* full_response_data; // see above
} echttp_Response;

echttp_Response echttp_request(char const* method, char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_get(char const* url, echttp_Header* headers, size_t header_count);

echttp_Response echttp_post(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_put(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_delete(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_patch(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_head(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_options(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_trace(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

echttp_Response echttp_connect(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count);

void echttp_release(echttp_Response response);

#endif

#ifdef ECHTTP_IMPLEMENTATION

#ifdef _WIN32
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( push )
#pragma warning( disable: 4127 ) // conditional expression is constant
#pragma warning( disable: 4255 ) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning( disable: 4365 ) // 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning( disable: 4574 ) // 'Identifier' is defined to be '0': did you mean to use '#if identifier'?
#pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directive'
#pragma warning( disable: 4706 ) // assignment within conditional expression
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma warning( pop )
#pragma comment (lib, "Ws2_32.lib") 
#include <string.h>
#include <stdio.h>
#include <io.h>
#define HTTP_SOCKET SOCKET
#define HTTP_INVALID_SOCKET INVALID_SOCKET
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#define HTTP_SOCKET int
#define HTTP_INVALID_SOCKET -1
#endif

#ifndef ECHTTP_MALLOC
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#define ECHTTP_MALLOC malloc
#define ECHTTP_REALLOC realloc
#define ECHTTP_FREE free
#endif

#include "thirdparty/tlse_adapter.c"

typedef enum echttp_internal_Status
{
    HTTP_STATUS_PENDING,
    HTTP_STATUS_COMPLETED,
    HTTP_STATUS_FAILED,
} echttp_internal_Status;

typedef struct echttp_internal_Request
{
    HTTP_SOCKET socket;
    struct TLSContext* tls_context;
    echttp_internal_Status status;
    int status_code;
    int connect_pending;
    int request_sent;
    char address[256];
    char request_header[256];
    char* request_header_large;
    void const* request_data;
    size_t request_data_size;
    char response_http_version[9];
    char response_reason_phrase[1024];
    size_t response_header_count;
    echttp_Header* response_headers;
    size_t response_data_size;
    size_t response_data_capacity;
    void* response_data;
    size_t full_response_data_size;
    void* full_response_data;
} echttp_internal_Request;

static int echttp_internal_parse_url(char const* url, char* address, size_t address_capacity, char* port, size_t port_capacity, char const** resource, char* is_https)
{
    // make sure url starts with http:// or https://
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) return 0;
    *is_https = strncmp(url, "https://", 8) == 0;
    url += *is_https ? 8 : 7;

    size_t url_len = strlen(url);

    // find end of address part of url
    char const* address_end = strchr(url, ':');
    if (!address_end) address_end = strchr(url, '/');
    if (!address_end) address_end = url + url_len;

    // extract address
    size_t address_len = (size_t)(address_end - url);
    if (address_len >= address_capacity) return 0;
    memcpy(address, url, address_len);
    address[address_len] = 0;

    // check if there's a port defined
    char const* port_end = address_end;
    if (*address_end == ':')
    {
        ++address_end;
        port_end = strchr(address_end, '/');
        if (!port_end) port_end = address_end + strlen(address_end);
        size_t port_len = (size_t)(port_end - address_end);
        if (port_len >= port_capacity) return 0;
        memcpy(port, address_end, port_len);
        port[port_len] = 0;
    }
    else
    {
        // use default port number 80
        if (port_capacity <= 2) return 0;
        strcpy(port, *is_https ? "443" : "80");
    }

    *resource = port_end;

    if (strlen(*resource) == 0)
    {
        *resource = "/";
    }

    return 1;
}

static echttp_internal_Request* echttp_internal_create_handle(void const* request_data, size_t request_data_size)
{
    echttp_internal_Request* request = (echttp_internal_Request*)ECHTTP_MALLOC(sizeof(echttp_internal_Request));

    request->status = HTTP_STATUS_PENDING;
    request->status_code = 0;

    request->connect_pending = 1;
    request->request_sent = 0;

    request->request_data_size = request_data_size;
    request->request_data = request_data;

    strcpy(request->response_http_version, "");

    strcpy(request->response_reason_phrase, "");

    request->response_header_count = 0;
    request->response_headers = NULL;

    request->response_data_size = 0;
    request->response_data_capacity = 64 * 1024;
    request->response_data = ECHTTP_MALLOC(request->response_data_capacity);

    return request;
}

echttp_internal_Request* echttp_build_request(char const* method, char const* url, void const* data, size_t size, echttp_Header* headers, size_t header_count)
{
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(1, 0), &wsa_data) != 0) return 0;
#endif

    char address[256];
    char port[16];
    char const* resource;
    char is_https;

    if (echttp_internal_parse_url(url, address, sizeof(address), port, sizeof(port), &resource, &is_https) == 0)
        return NULL;

    HTTP_SOCKET socket = echttp_tlse_wrapper_connect_socket(address, atoi(port));
    if (socket == HTTP_INVALID_SOCKET) return NULL;

    struct TLSContext* tls_context = NULL;
    if (is_https) {
        tls_context = tls_create_context(0, TLS_V12);
        tls_sni_set(tls_context, address); // TODO: Should we use this here? It looks like it is required for "www.google.com" for example
        echttp_tlse_wrapper_connect_tls(socket, tls_context);
    }

    echttp_internal_Request* request = echttp_internal_create_handle(data, size);
    request->socket = socket;
    request->tls_context = tls_context;

    char* request_header;
    size_t request_header_len = 64 + strlen(resource) + strlen(address) + strlen(port);
    for (size_t i = 0; i < header_count; i++)
    {
        request_header_len += strlen(headers[i].name) + 2;
        for (size_t j = 0; j < headers[i].value_count; j++)
        {
            request_header_len += strlen(headers[i].values[j]) + 2;
        }
    }
    if (request_header_len < sizeof(request->request_header))
    {
        request->request_header_large = NULL;
        request_header = request->request_header;
    }
    else
    {
        request->request_header_large = (char*)ECHTTP_MALLOC(request_header_len + 1);
        request_header = request->request_header_large;
    }
    int default_http_port = (strcmp(port, "80") == 0);
    char* uppercase_method = (char*)ECHTTP_MALLOC(strlen(method) + 1);
    for (size_t i = 0; i < strlen(method); i++)
    {
        uppercase_method[i] = toupper(method[i]);
    }
    uppercase_method[strlen(method)] = '\0';
    sprintf(request_header, "%s %s HTTP/1.0\r\nHost: %s%s%s\r\nContent-Length: %d\r\n", uppercase_method, resource, address, default_http_port ? "" : ":", default_http_port ? "" : port, (int)size);
    for (size_t i = 0; i < header_count; i++)
    {
        sprintf(request_header + strlen(request_header), "%s: ", headers[i].name);
        for (size_t j = 0; j < headers[i].value_count; j++)
        {
            sprintf(request_header + strlen(request_header), "%s", headers[i].values[j]);
            if (j + 1 < headers[i].value_count)
            {
                sprintf(request_header + strlen(request_header), ", ");
            }
        }
        sprintf(request_header + strlen(request_header), "\r\n");
    }
    sprintf(request_header + strlen(request_header), "\r\n");

    return request;
}

echttp_internal_Status echttp_process_request(echttp_internal_Request* request)
{
    if (request->status == HTTP_STATUS_FAILED) return request->status;

    if (request->connect_pending)
    {
        fd_set sockets_to_check;
        FD_ZERO(&sockets_to_check);
#pragma warning( push )
#pragma warning( disable: 4548 ) // expression before comma has no effect; expected expression with side-effect
        FD_SET(request->socket, &sockets_to_check);
#pragma warning( pop )
        struct timeval timeout; timeout.tv_sec = 0; timeout.tv_usec = 0;
        // check if socket is ready for send
        if (select((int)(request->socket + 1), NULL, &sockets_to_check, NULL, &timeout) == 1)
        {
            int opt = -1;
            socklen_t len = sizeof(opt);
            if (getsockopt(request->socket, SOL_SOCKET, SO_ERROR, (char*)(&opt), &len) >= 0 && opt == 0)
                request->connect_pending = 0; // if it is, we're connected
        }
    }

    if (request->connect_pending) return request->status;

    if (!request->request_sent)
    {
        char* request_header = request->request_header_large ? request->request_header_large : request->request_header;
        if ((request->tls_context == NULL ? send(request->socket, (const char*)request_header, (int)strlen(request_header), 0) : echttp_tlse_wrapper_write_tls(request->socket, request->tls_context, (const unsigned char*)request_header, (int)strlen(request_header))) == -1)
        {
            request->status = HTTP_STATUS_FAILED;
            return request->status;
        }
        if (request->request_data_size)
        {
            int res = request->tls_context == NULL ? send(request->socket, (char const*)request->request_data, (int)request->request_data_size, 0) : echttp_tlse_wrapper_write_tls(request->socket, request->tls_context, (unsigned char*)request->request_data, (int)request->request_data_size);
            if (res == -1)
            {
                request->status = HTTP_STATUS_FAILED;
                return request->status;
            }
        }
        request->request_sent = 1;
        return request->status;
    }

    // check if socket is ready for recv
    fd_set sockets_to_check;
    FD_ZERO(&sockets_to_check);
#pragma warning( push )
#pragma warning( disable: 4548 ) // expression before comma has no effect; expected expression with side-effect
    FD_SET(request->socket, &sockets_to_check);
#pragma warning( pop )
    struct timeval timeout; timeout.tv_sec = 0; timeout.tv_usec = 0;
    while (select((int)(request->socket + 1), &sockets_to_check, NULL, NULL, &timeout) == 1)
    {
        unsigned char buffer[4096];
        int size = request->tls_context == NULL ? recv(request->socket, (char*)buffer, sizeof(buffer), 0) : echttp_tlse_wrapper_read_tls(request->socket, request->tls_context, buffer, sizeof(buffer));
        if (size == -1)
        {
            request->status = HTTP_STATUS_FAILED;
            return request->status;
        }
        else if (size > 0)
        {
            size_t min_size = request->response_data_size + size + 1;
            if (request->response_data_capacity < min_size)
            {
                request->response_data_capacity *= 2;
                if (request->response_data_capacity < min_size) request->response_data_capacity = min_size;
                void* new_data = ECHTTP_MALLOC(request->response_data_capacity);
                memcpy(new_data, request->response_data, request->response_data_size);
                ECHTTP_FREE(request->response_data);
                request->response_data = new_data;
            }
            memcpy((void*)(((uintptr_t)request->response_data) + request->response_data_size), buffer, (size_t)size);
            request->response_data_size += size;
        }
        else if (size == 0)
        {
            char const* status_line = (char const*)request->response_data;

            int header_size = 0;
            char const* header_end = strstr(status_line, "\r\n\r\n");
            if (header_end)
            {
                header_end += 4;
                header_size = (int)(header_end - status_line);
            }
            else
            {
                request->status = HTTP_STATUS_FAILED;
                return request->status;
            }

            memcpy(request->response_http_version, status_line, 8);
            request->response_http_version[8] = 0;
            status_line = strchr(status_line, ' ');
            if (!status_line)
            {
                request->status = HTTP_STATUS_FAILED;
                return request->status;
            }
            ++status_line;

            // extract status code
            char status_code[16];
            char const* status_code_end = strchr(status_line, ' ');
            if (!status_code_end)
            {
                request->status = HTTP_STATUS_FAILED;
                return request->status;
            }
            memcpy(status_code, status_line, (size_t)(status_code_end - status_line));
            status_code[status_code_end - status_line] = 0;
            status_line = status_code_end + 1;
            request->status_code = atoi(status_code);

            // extract reason phrase
            char const* reason_phrase_end = strstr(status_line, "\r\n");
            if (!reason_phrase_end)
            {
                request->status = HTTP_STATUS_FAILED;
                return request->status;
            }
            size_t reason_phrase_len = (size_t)(reason_phrase_end - status_line);
            if (reason_phrase_len >= sizeof(request->response_reason_phrase))
                reason_phrase_len = sizeof(request->response_reason_phrase) - 1;
            memcpy(request->response_reason_phrase, status_line, reason_phrase_len);
            request->response_reason_phrase[reason_phrase_len] = 0;
            status_line = reason_phrase_end + 1;

            if ((status_line)-(char const*)request->response_data < header_size)
            {
                if (status_line[0] == '\n') status_line++;

                // extract headers
                while (status_line[0] != '\r' && status_line[1] != '\n')
                {
                    char const* header_end = strstr(status_line, "\r\n");
                    if (!header_end)
                    {
                        request->status = HTTP_STATUS_FAILED;
                        return request->status;
                    }
                    size_t header_len = (size_t)(header_end - status_line);
                    if (header_len > 0)
                    {
                        if (request->response_header_count == 0)
                        {
                            request->response_headers = (echttp_Header*)ECHTTP_MALLOC(sizeof(echttp_Header));
                        }
                        else
                        {
                            request->response_headers = (echttp_Header*)ECHTTP_REALLOC(request->response_headers, (request->response_header_count + 1) * sizeof(echttp_Header));
                        }
                        echttp_Header header = *(request->response_headers + request->response_header_count);
                        size_t name_len = 0;
                        while (name_len < header_len && status_line[name_len] != ':') ++name_len;
                        header.name = (char const*)ECHTTP_MALLOC(name_len + 1);
                        memcpy((void*)header.name, status_line, name_len);
                        ((char*)header.name)[name_len] = 0;
                        status_line += name_len + 2;
                        size_t value_len = header_len - name_len - 2;

                        header.values = (char const**)ECHTTP_MALLOC(sizeof(char const*));
                        header.value_count = 0;

                        char const* value_start = status_line;
                        long remaining_len = value_len; // long instead of size_t because it may be negative
                        while (remaining_len > 0) {
                            char const* semicolon_pos = strchr(value_start, ';');
                            size_t single_value_len;
                            if (semicolon_pos && semicolon_pos < value_start + remaining_len) {
                                single_value_len = (size_t)(semicolon_pos - value_start);
                            }
                            else {
                                single_value_len = remaining_len;
                            }

                            header.values = (char const**)ECHTTP_REALLOC(header.values, sizeof(char const*) * (header.value_count + 1));
                            header.values[header.value_count] = (char const*)ECHTTP_MALLOC(single_value_len + 1);
                            memcpy((void*)header.values[header.value_count], value_start, single_value_len);
                            ((char*)header.values[header.value_count])[single_value_len] = 0;
                            header.value_count++;

                            value_start += single_value_len + 1;
                            remaining_len -= single_value_len + 1;
                        }

                        status_line += value_len;
                        request->response_headers[request->response_header_count] = header;
                        request->response_header_count++;
                        if ((status_line - (char const*)request->response_data) <= header_size - 2 && status_line[0] == '\r' && status_line[1] == '\n') {
                            status_line += 2;
                        }
                    }
                    else
                    {
                        status_line += 2;
                    }
                }
            }

            request->status = request->status_code < 300 ? HTTP_STATUS_COMPLETED : HTTP_STATUS_FAILED;
            request->full_response_data = request->response_data;
            request->full_response_data_size = request->response_data_size;
            request->response_data = (void*)(((uintptr_t)request->response_data) + header_size);
            request->response_data_size = request->response_data_size - header_size;

            // add an extra zero after the received data, but don't modify the size, so ascii results can be used as
            // a zero terminated string. the size returned will be the string without this extra zero terminator.
            ((char*)request->response_data)[request->response_data_size] = 0;
            return request->status;
        }
    }

    return request->status;
}

echttp_Response echttp_request(char const* method, char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    echttp_Response response = { 0 };

    echttp_internal_Request* request = echttp_build_request(method, url, data, data_size, headers, header_count);
    response._handle = request;
    if (!request)
    {
        response.status_code = -1;
        response.reason_phrase = "Invalid request.";
        return response;
    }

    echttp_internal_Status status = HTTP_STATUS_PENDING;
    while (status == HTTP_STATUS_PENDING)
    {
        status = echttp_process_request(request);
    }

    if (request->status_code == 301 || request->status_code == 302 || request->status_code == 303 || request->status_code == 307 || request->status_code == 308)
    {
        if (request->response_header_count > 0)
        {
            for (size_t i = 0; i < request->response_header_count; i++)
            {
                if (strcmp(request->response_headers[i].name, "Location") == 0)
                {
                    return echttp_request(method, request->response_headers[i].values[0], data, data_size, headers, header_count); // TODO: Limit the number of redirects
                }
            }
        }
        response.status_code = -1;
        response.reason_phrase = "Invalid request.";
        return response;
    }

    response.status_code = request->status_code;
    response.http_version = request->response_http_version;
    response.reason_phrase = request->response_reason_phrase;
    response.header_count = request->response_header_count;
    response.headers = request->response_headers;
    response.response_size = request->response_data_size;
    response.data = (char*)request->response_data;
    response.full_response_size = request->full_response_data_size;
    response.full_response_data = (char*)request->full_response_data;
    return response;
}

echttp_Response echttp_get(char const* url, echttp_Header* headers, size_t header_count)
{
    return echttp_request("GET", url, NULL, 0, headers, header_count);
}

echttp_Response echttp_post(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("POST", url, data, data_size, headers, header_count);
}

echttp_Response echttp_put(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("PUT", url, data, data_size, headers, header_count);
}

echttp_Response echttp_delete(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("DELETE", url, data, data_size, headers, header_count);
}

echttp_Response echttp_patch(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("PATCH", url, data, data_size, headers, header_count);
}

echttp_Response echttp_head(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("HEAD", url, data, data_size, headers, header_count);
}

echttp_Response echttp_options(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("OPTIONS", url, data, data_size, headers, header_count);
}

echttp_Response echttp_trace(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("TRACE", url, data, data_size, headers, header_count);
}

echttp_Response echttp_connect(char const* url, char const* data, size_t data_size, echttp_Header* headers, size_t header_count)
{
    return echttp_request("CONNECT", url, data, data_size, headers, header_count);
}

void echttp_release(echttp_Response response)
{
    if (response.status_code != -1)
    {
        echttp_internal_Request* request = (echttp_internal_Request*)response._handle;
#ifdef _WIN32
        closesocket(request->socket);
#else
        close(request->socket);
#endif

        if (request->request_header_large) ECHTTP_FREE(request->request_header_large);
        ECHTTP_FREE(request->full_response_data);
        ECHTTP_FREE(request);
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

#endif