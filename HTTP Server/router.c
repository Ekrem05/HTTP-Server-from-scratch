#include "server.h"
#include "errors.h"
#include <stdio.h>
#include <ws2tcpip.h>

int add(HttpMethod method, const char* route, RouteHandler handler,ServerConfig* config)
{
	if (config->routeCount >= MAX_ROUTES)
	{
		return MAXIMUM_ROUTE_COUNT_REACHED;
	}
	for (int i = 0; i < config->routeCount; i++)
	{
		if (config->routes[i].method == method && config->routes[i].route == route)
		{
			return ROUTE_ALREADY_EXISTS;
		}
	}
    config->routes[config->routeCount].method = method;
    strcpy_s(config->routes[config->routeCount].route, MAX_ROUTE_NAME_LENGTH, route);
    config->routes[config->routeCount].handler = handler;
	config->routeCount++;

	return 1;
}

void trim_whitespace(char* str)
{
    // Trim leading whitespace
    while (isspace((unsigned char) *str))
    {
        str++;
    }

    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end))
    {
        end--;
    }

    // Null-terminate the string
    *(end + 1) = '\0';
}

// Helper function to ensure safe string copy
void safe_copy(char* dest, const char* src, size_t dest_size)
{
    if (src)
    {
        strncpy_s(dest, dest_size, src, _TRUNCATE);
        // Ensure the destination is null-terminated
        dest[dest_size - 1] = '\0';
        // Trim leading and trailing whitespace
        trim_whitespace(dest);
    }
    else
    {
        dest[0] = '\0'; // Ensure destination is empty if src is NULL
    }
}

void router(HttpContext* context, ServerConfig* config)
{
    // Retrieve headers
    const char* method = (const char*) ht_get(context->request->headers, "Method");
    const char* route = (const char*) ht_get(context->request->headers, "Route");

    // Check if headers are retrieved successfully
    if (!method || !route)
    {
        // Handle error: missing method or route
        return;
    }

    // Local buffers to hold headers and routes
    char methodBuffer[MAX_KEY_LENGTH] = { 0 }; // Initialize to zero
    char routeBuffer[MAX_URL_LENGTH] = { 0 };  // Initialize to zero

    // Copy and null-terminate
    safe_copy(methodBuffer, method, sizeof(methodBuffer));
    safe_copy(routeBuffer, route, sizeof(routeBuffer));

    // Debug prints to ensure correct values
    printf("Method: '%s'\n", methodBuffer);
    printf("Route: '%s'\n", routeBuffer);

    int route_found = 0;
    

    for (int i = 0; i < config->routeCount; i++)
    {
        if (strcmp(config->routes[i].route, routeBuffer) == 0)
        {
            const char* methodInString = http_method_to_string(config->routes[i].method);
            if (strcmp(methodInString, methodBuffer) == 0)
            {
                route_found = 1;
                config->routes[i].handler(context);
                break; // Exit loop as we found a matching route
            }
            else
            {
                route_found = 1;
                HttpStatus status = METHOD_NOT_ALLOWED;
                context->response->status = status;
                strcpy_s(context->response->body, MAX_BODY_LENGTH,"");
                break;
            }
        }
    }

    if (!route_found)
    {
        HttpStatus status = NOT_FOUND;
        context->response->status = status;
        strcpy_s(context->response->body, MAX_BODY_LENGTH, "");
    }
}

const char* http_method_to_string(HttpMethod method)
{
    switch (method)
    {
    case GET:    return "GET";
    case POST:   return "POST";
    case PUT:    return "PUT";
    case DELETE: return "DELETE";
    case PATCH:  return "PATCH";
    case HEAD:   return "HEAD";
    case OPTIONS:return "OPTIONS";
    default:     return "UNKNOWN";
    }
}
