#define NETUTILS_TLSE_IMPLEMENTATION
#define NETUTILS_MALLOC ASPL_MALLOC
#define NETUTILS_REALLOC ASPL_REALLOC
#define NETUTILS_FREE ASPL_FREE
#include "thirdparty/netutils/tcp_connection.c"

#define ECHTTP_IMPLEMENTATION
#define ECHTTP_MALLOC ASPL_MALLOC
#define ECHTTP_REALLOC ASPL_REALLOC
#define ECHTTP_FREE ASPL_FREE
#include "thirdparty/echttp/echttp.h"

#define CWSC_IMPLEMENTATION
#define CWSC_MALLOC ASPL_MALLOC
#define CWSC_REALLOC ASPL_REALLOC
#define CWSC_FREE ASPL_FREE
#include "thirdparty/cwsc/cwsc.h"

// Note: Those #undefs are required because CipherSuite.h (which is included by sokol) also uses these identifiers
// TODO: Check if there are more #undefs than required
#undef TLS_RSA_WITH_AES_128_GCM_SHA256
#undef TLS_RSA_WITH_AES_256_GCM_SHA384
#undef TLS_DHE_RSA_WITH_AES_128_GCM_SHA256
#undef TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
#undef TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
#undef TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
#undef TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
#undef TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
#undef TLS_AES_128_GCM_SHA256
#undef TLS_AES_256_GCM_SHA384
#undef TLS_CHACHA20_POLY1305_SHA256
#undef TLS_AES_128_CCM_SHA256
#undef TLS_AES_128_CCM_8_SHA256
#undef TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256
#undef TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256
#undef TLS_RSA_WITH_AES_128_CBC_SHA
#undef TLS_DHE_RSA_WITH_AES_128_CBC_SHA
#undef TLS_RSA_WITH_AES_256_CBC_SHA
#undef TLS_DHE_RSA_WITH_AES_256_CBC_SHA
#undef TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
#undef TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
#undef TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
#undef TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
#undef TLS_RSA_WITH_AES_128_CBC_SHA256
#undef TLS_RSA_WITH_AES_256_CBC_SHA256
#undef TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
#undef TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
#undef TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
#undef TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
#undef TLS_DHE_RSA_WITH_AES_128_CBC_SHA256

void aspl_callback_invoke(ASPL_OBJECT_TYPE closure);
void aspl_callback_string__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* message);
void aspl_callback_integer_string__invoke(ASPL_OBJECT_TYPE closure, ASPL_OBJECT_TYPE* code, ASPL_OBJECT_TYPE* message);

typedef struct ASPL_handle_internet$http$ResponseData {
    unsigned char* data;
    size_t length;
} ASPL_handle_internet$http$ResponseData;

ASPL_OBJECT_TYPE aspl_util_perform_http_request(const char* method, ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers) {
    size_t headerCount = ASPL_ACCESS(*headers).kind == ASPL_OBJECT_KIND_NULL ? 0 : ASPL_ACCESS(*headers).value.map->hashmap->len;
    echttp_Header* headerList = ASPL_MALLOC(sizeof(echttp_Header) * headerCount);
    for (size_t i = 0; i < headerCount; i++)
    {
        ASPL_OBJECT_TYPE key = ASPL_MAP_GET_KEY_FROM_INDEX(*headers, ASPL_INT_LITERAL(i));
        ASPL_OBJECT_TYPE value = ASPL_MAP_GET_VALUE_FROM_INDEX(*headers, ASPL_INT_LITERAL(i));
        headerList[i].name = ASPL_ACCESS(key).value.string->str;
        headerList[i].value_count = ASPL_ACCESS(value).value.list->length;
        headerList[i].values = ASPL_MALLOC(sizeof(char*) * headerList[i].value_count);
        for (size_t j = 0; j < headerList[i].value_count; j++)
        {
            ASPL_OBJECT_TYPE headerValue = ASPL_LIST_GET(value, ASPL_INT_LITERAL(j));
            headerList[i].values[j] = ASPL_ACCESS(headerValue).value.string->str;
        }
    }
    echttp_Response response = echttp_request(method, ASPL_ACCESS(*url).value.string->str, ASPL_ACCESS(*data).value.string->str, ASPL_ACCESS(*data).value.string->length, headerList, headerCount);
    ASPL_OBJECT_TYPE* responseList = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * 5);
    ASPL_handle_internet$http$ResponseData* responseData = ASPL_MALLOC(sizeof(ASPL_handle_internet$http$ResponseData));
    responseData->data = ASPL_MALLOC_ATOMIC(response.response_size + 1);
    mempcpy(responseData->data, response.data, response.response_size);
    responseData->data[response.response_size] = '\0';
    responseData->length = response.response_size;
    responseList[0] = ASPL_HANDLE_LITERAL(responseData);
    ASPL_OBJECT_TYPE* responseHeaders = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * response.header_count * 2);
    for (size_t i = 0; i < response.header_count; i++)
    {
        responseHeaders[i * 2] = ASPL_STRING_LITERAL(response.headers[i].name);
        ASPL_OBJECT_TYPE* headerValues = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * response.headers[i].value_count);
        for (size_t j = 0; j < response.headers[i].value_count; j++)
        {
            headerValues[j] = ASPL_STRING_LITERAL(response.headers[i].values[j]);
        }
        responseHeaders[i * 2 + 1] = ASPL_LIST_LITERAL("list<string>", 12, headerValues, response.headers[i].value_count);
    }
    responseList[1] = ASPL_MAP_LITERAL("map<string, list<string>>", 26, responseHeaders, response.header_count);
    responseList[2] = ASPL_INT_LITERAL(response.status_code);
    responseList[3] = ASPL_STRING_LITERAL(response.reason_phrase);
    responseList[4] = ASPL_STRING_LITERAL(response.http_version);
    echttp_release(response);
    return ASPL_LIST_LITERAL(
        "list<string|map<string, list<string>>|integer>",
        47,
        responseList,
        5
    );
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$get(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("get", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$post(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("post", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$put(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("put", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$head(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("head", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$delete(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("delete", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$options(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("options", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$trace(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("trace", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$connect(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("connect", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$patch(ASPL_OBJECT_TYPE* url, ASPL_OBJECT_TYPE* data, ASPL_OBJECT_TYPE* headers)
{
    return aspl_util_perform_http_request("patch", url, data, headers);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$response_data$fetch_as_string(ASPL_OBJECT_TYPE* handle)
{
    return ASPL_STRING_LITERAL((char*)((ASPL_handle_internet$http$ResponseData*)ASPL_ACCESS(*handle).value.handle)->data);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$http$response_data$fetch_as_bytes(ASPL_OBJECT_TYPE* handle)
{
    ASPL_handle_internet$http$ResponseData* responseData = (ASPL_handle_internet$http$ResponseData*)ASPL_ACCESS(*handle).value.handle;
    ASPL_OBJECT_TYPE* bytes = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE) * responseData->length);
    for (int i = 0; i < responseData->length; i++)
    {
        bytes[i] = ASPL_BYTE_LITERAL(responseData->data[i]);
    }
    return ASPL_LIST_LITERAL("list<byte>", 10, bytes, responseData->length);
}

typedef struct ASPL_handle_TcpSocketClient
{
    char* host;
    int port;
    char use_tls;
    netutils_TcpConnection* handle;
} ASPL_handle_TcpSocketClient;

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$tcp_socket_client$new(ASPL_OBJECT_TYPE* host, ASPL_OBJECT_TYPE* port, ASPL_OBJECT_TYPE* tls)
{
    ASPL_handle_TcpSocketClient* client = ASPL_MALLOC(sizeof(ASPL_handle_TcpSocketClient));
    client->host = ASPL_ACCESS(*host).value.string->str;
    client->port = ASPL_ACCESS(*port).value.integer32;
    client->use_tls = ASPL_ACCESS(*tls).value.boolean;
    return ASPL_HANDLE_LITERAL(client);
}

void aspl_util_internet$TcpSocketClient$error_callback(char* error_message, void* user_data) {
    ASPL_PANIC(error_message);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$tcp_socket_client$connect(ASPL_OBJECT_TYPE* client)
{
    ASPL_handle_TcpSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    handle->handle = netutils_tcp_connection_new(handle->host, handle->port, handle->use_tls, aspl_util_internet$TcpSocketClient$error_callback, handle);
    if (handle->handle->handle == -1) {
        ASPL_PANIC("Failed to connect to server");
    }
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$tcp_socket_client$read(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* length)
{
    ASPL_handle_TcpSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    char* buffer = ASPL_MALLOC(ASPL_ACCESS(*length).value.integer32 + 1);
    int read = netutils_tcp_connection_read(handle->handle, buffer, ASPL_ACCESS(*length).value.integer32);
    if (read == -1) {
        return ASPL_NULL();
    }
    else {
        buffer[read] = '\0';
        return ASPL_STRING_LITERAL(buffer);
    }
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$tcp_socket_client$send(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* message)
{
    ASPL_handle_TcpSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    return ASPL_INT_LITERAL(netutils_tcp_connection_send(handle->handle, ASPL_ACCESS(*message).value.string->str, ASPL_ACCESS(*message).value.string->length));
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$tcp_socket_client$disconnect(ASPL_OBJECT_TYPE* client)
{
    ASPL_handle_TcpSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    netutils_tcp_connection_close(handle->handle);
    return ASPL_UNINITIALIZED;
}

typedef struct ASPL_handle_WebSocketClient
{
    char* uri;
    cwsc_WebSocket* handle;
    ASPL_OBJECT_TYPE on_connect;
    ASPL_OBJECT_TYPE on_message;
    ASPL_OBJECT_TYPE on_error;
    ASPL_OBJECT_TYPE on_close;
} ASPL_handle_WebSocketClient;

void aspl_util_internet$WebSocketClient$connect_callback(cwsc_WebSocket* ws)
{
    ASPL_handle_WebSocketClient* handle = ws->user_data;
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 0, .arguments = NULL };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->on_connect).value.callback, arguments);
#else
    aspl_callback_invoke(handle->on_connect);
#endif
}

void aspl_util_internet$WebSocketClient$message_callback(cwsc_WebSocket* ws, cwsc_Opcode opcode, const unsigned char* message, int length)
{
    ASPL_handle_WebSocketClient* handle = ws->user_data;
    ASPL_OBJECT_TYPE* messageObject = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
    *messageObject = ASPL_STRING_LITERAL((char*)message);
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = messageObject };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->on_message).value.callback, arguments);
#else
    aspl_callback_string__invoke(handle->on_message, messageObject);
#endif
}

void aspl_util_internet$WebSocketClient$error_callback(cwsc_WebSocket* ws, const char* message, int length)
{
    ASPL_handle_WebSocketClient* handle = ws->user_data;
    ASPL_OBJECT_TYPE* messageObject = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
    *messageObject = ASPL_STRING_LITERAL(message);
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 1, .arguments = messageObject };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->on_error).value.callback, arguments);
#else
    aspl_callback_string__invoke(handle->on_error, messageObject);
#endif
}

void aspl_util_internet$WebSocketClient$close_callback(cwsc_WebSocket* ws, int code, const char* message, int length)
{
    ASPL_handle_WebSocketClient* handle = ws->user_data;
    ASPL_OBJECT_TYPE* codeObject = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
    *codeObject = ASPL_INT_LITERAL(code);
    ASPL_OBJECT_TYPE* messageObject = ASPL_MALLOC(sizeof(ASPL_OBJECT_TYPE));
    *messageObject = ASPL_STRING_LITERAL(message);
#ifdef ASPL_INTERPRETER_MODE
    ASPL_AILI_ArgumentList arguments = (ASPL_AILI_ArgumentList){ .size = 2, .arguments = (ASPL_OBJECT_TYPE[]){ *codeObject, *messageObject } };
    aspl_ailinterpreter_invoke_callback_from_outside_of_loop(ASPL_ACCESS(handle->on_close).value.callback, arguments);
#else
    aspl_callback_integer_string__invoke(handle->on_close, codeObject, messageObject);
#endif
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$new(ASPL_OBJECT_TYPE* uri)
{
    ASPL_handle_WebSocketClient* client = ASPL_MALLOC(sizeof(ASPL_handle_WebSocketClient));
    client->uri = ASPL_ACCESS(*uri).value.string->str;
    client->handle = ASPL_MALLOC(sizeof(cwsc_WebSocket));
    return ASPL_HANDLE_LITERAL(client);
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$connect(ASPL_OBJECT_TYPE* client)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    cwsc_WebSocket* ws = ASPL_MALLOC(sizeof(cwsc_WebSocket));
    handle->handle = ws;
    ws->user_data = handle;
    ws->tls_context = tls_create_context(0, TLS_V12);
    ws->on_open = aspl_util_internet$WebSocketClient$connect_callback;
    ws->on_message = aspl_util_internet$WebSocketClient$message_callback;
    ws->on_error = aspl_util_internet$WebSocketClient$error_callback;
    ws->on_close = aspl_util_internet$WebSocketClient$close_callback;
    tls_sni_set(ws->tls_context, handle->uri); // TODO: Should we use this here? It looks like it is required for HTTP for "www.google.com" for example - but do we also need it for WebSockets?
    cwsc_connect(handle->handle, handle->uri, strncmp(handle->uri, "wss", strlen("wss")) == 0 ? 443 : 80);
    cwsc_listen(handle->handle);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$set_on_connect(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* callback)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    handle->on_connect = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$set_on_message(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* callback)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    handle->on_message = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$set_on_error(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* callback)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    handle->on_error = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$set_on_close(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* callback)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    handle->on_close = *callback;
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$send(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* message)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    cwsc_send_message(handle->handle, ASPL_ACCESS(*message).value.string->str);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_client$disconnect(ASPL_OBJECT_TYPE* client, ASPL_OBJECT_TYPE* code, ASPL_OBJECT_TYPE* message)
{
    ASPL_handle_WebSocketClient* handle = ASPL_ACCESS(*client).value.handle;
    cwsc_disconnect(handle->handle, ASPL_ACCESS(*message).value.string->str, ASPL_ACCESS(*code).value.integer32);
    return ASPL_UNINITIALIZED;
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$new(ASPL_OBJECT_TYPE* address_family, ASPL_OBJECT_TYPE* port)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$listen(ASPL_OBJECT_TYPE* server)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$set_on_connect(ASPL_OBJECT_TYPE* server, ASPL_OBJECT_TYPE* callback)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$set_on_message(ASPL_OBJECT_TYPE* server, ASPL_OBJECT_TYPE* callback)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$set_on_error(ASPL_OBJECT_TYPE* server, ASPL_OBJECT_TYPE* callback)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server$set_on_close(ASPL_OBJECT_TYPE* server, ASPL_OBJECT_TYPE* callback)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server_client$get_id(ASPL_OBJECT_TYPE* server_client)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}

ASPL_OBJECT_TYPE ASPL_IMPLEMENT_internet$web_socket_server_client$send(ASPL_OBJECT_TYPE* server_client, ASPL_OBJECT_TYPE* message)
{
    // TODO
    ASPL_PANIC("Unimplemented");
}