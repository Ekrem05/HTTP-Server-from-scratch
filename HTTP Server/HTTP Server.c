#include "server.h"
void myRouteHandler(HttpContext* context)
{

	strcpy(context->response->body, "This is the response from myRouteHandler.");
}

int main()
{
	ServerConfig config = {.port = 6999};
	

	add(POST,"/event", myRouteHandler, &config);

	run(&config);
}


