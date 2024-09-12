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
	config->routes->method = method;
	strcpy_s(config->routes[config->routeCount].route, MAX_ROUTE_NAME_LENGTH,route);
	config->routes->handler = handler;
	config->routeCount++;

	return 1;
}
void router(Request* request, ServerConfig* config)
{
	char method[10];
	char route[240];
	for (size_t i = 0; i < config->routeCount; i++)
	{
		if(strcmp(config->routes[i].route,request->headers.)
	}
}