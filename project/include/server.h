#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>

#define MAX_KEY_LENGTH 256 //key of header
#define MAX_VALUE_LENGTH 1024 //value of header
#define MAX_NUMBER_OF_HEADERS 30
#define MAX_BODY_LENGTH 8120
#define MAX_ADDITIONAL_HEADERS_LENGTH 8120
#define MAX_ROUTES 100
#define MAX_URL_LENGTH 2000
#define MAX_ROUTE_NAME_LENGTH 64

// Error Codes
#define ERROR_SUCCESS 0
#define ERROR_UNKNOWN -1
#define ERROR_INVALID_ARGUMENT -2
#define ERROR_OUT_OF_MEMORY -3
#define ERROR_NETWORK_TIMEOUT -100
#define ERROR_CONNECTION_FAILED -101
#define MAXIMUM_ROUTE_COUNT_REACHED 0
#define ROUTE_ALREADY_EXISTS 0

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        GET,
        POST,
        PUT,
        DELETE,
    } HttpMethod;

    typedef void (*RouteHandler)(const char* request, char* response, size_t* response_len);

    typedef struct
    {
        HttpMethod method;
        char route[MAX_ROUTE_NAME_LENGTH];
        RouteHandler handler;
    } Route;

    typedef struct
    {
        uint16_t port;
        Route routes[MAX_ROUTES];
        uint16_t routeCount;
    } ServerConfig;

    typedef struct
    {
        char key[MAX_KEY_LENGTH];
        char value[MAX_VALUE_LENGTH];
    } Header;

    typedef struct
    {
        HttpMethod method;
        Header headers[MAX_NUMBER_OF_HEADERS];
        char body[MAX_BODY_LENGTH];
    } Request;

    typedef struct
    {
        HttpMethod method;
        Header* headers;
        char* body;
    } Response;

    int add(HttpMethod method, const char* route, RouteHandler handler, ServerConfig* config);
    void router(const char* request, ServerConfig* config);
    int run(ServerConfig* config);


#ifdef __cplusplus
}
#endif

#endif // SERVER_H
