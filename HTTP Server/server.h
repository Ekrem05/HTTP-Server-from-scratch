#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>
#include "hashmap.h"

#define HTTP_STATUS(code, message) (HttpStatus){code, message}
#define OK HTTP_STATUS(200, "OK")
#define CREATED HTTP_STATUS(201, "Created")
#define BAD_REQUEST HTTP_STATUS(400, "Bad Request")
#define NOT_FOUND HTTP_STATUS(404, "Not Found")
#define METHOD_NOT_ALLOWED HTTP_STATUS(405, "Method Not Allowed")


#define MAX_KEY_LENGTH 256 
#define MAX_VALUE_LENGTH 1024 
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
#define MAXIMUM_ROUTE_COUNT_REACHED -200
#define ROUTE_ALREADY_EXISTS -201



#ifdef __cplusplus
extern "C" {
#endif
    typedef struct
    {
        int code;
        const char* message;
    } HttpStatus;

    typedef enum
    {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        OPTIONS
    } HttpMethod;

    typedef struct
    {
        HttpMethod method;
        ht* headers;
        char body[MAX_BODY_LENGTH];
    } Request;

    typedef struct
    {
        HttpStatus status;
        ht* headers;
        char body[MAX_BODY_LENGTH];
    } Response;

    typedef struct
    {
        Request* request;
        Response* response;
    } HttpContext;

    typedef void (*RouteHandler)(HttpContext* context);

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

    const char* http_method_to_string(HttpMethod method);
    int add(HttpMethod method, const char* route, RouteHandler handler, ServerConfig* config);
    void router(Request* request, ServerConfig* config);
    int run(ServerConfig* config);
    void buildHttpResponse(char* buffer, int buffer_size, int status_code, const char* status_message, const char* content_type, const char* body);

#ifdef __cplusplus
}
#endif

#endif // SERVER_H
