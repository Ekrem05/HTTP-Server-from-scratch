#include "server.h"
#include "errors.h"

int register_route(HttpMethod method, const char* route, RouteHandler handler,ServerConfig* config)
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
	config->routes->route  = route;
	config->routes->handler = handler;
	config->routeCount++;

	return 1;
}
void router(const char* request, ServerConfig* config)
{

}