#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>

#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 256

// Error Codes
#define ERROR_SUCCESS 0
#define ERROR_UNKNOWN -1
#define ERROR_INVALID_ARGUMENT -2
#define ERROR_OUT_OF_MEMORY -3
#define ERROR_NETWORK_TIMEOUT -100
#define ERROR_CONNECTION_FAILED -101
#define MAXIMUM_ROUTE_COUNT_REACHED 0
#define ROUTE_ALREADY_EXISTS 0
#define MAX_ROUTES 100

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        METHOD_GET,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
    } HttpMethod;

    typedef void (*RouteHandler)(const char* request, char* response, size_t* response_len);

    typedef struct
    {
        HttpMethod method;
        const char* route;
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
        Header* headers;
        char* body;
    } Request;

    typedef struct
    {
        Header* headers;
        char* body;
    } Response;

    int register_route(HttpMethod method, const char* route, RouteHandler handler, ServerConfig* config);
    void router(const char* request, ServerConfig* config);
    int initialize_server(ServerConfig* config);

#ifdef __cplusplus
}
#endif

#endif // SERVER_H
