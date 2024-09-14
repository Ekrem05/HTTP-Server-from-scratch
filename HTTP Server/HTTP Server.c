#include "server.h"
void myRouteHandler(HttpContext* context)
{
	strcpy(context->response->body, "This is the response from myRouteHandler.");
	HttpStatus status = OK;
	context->response->status = status;
}
void index(HttpContext* context)
{
	strcpy(context->response->body, "Hello from index route");
	HttpStatus status = OK;
	context->response->status = status;
}

int main()
{
	ServerConfig config = {.port = 6999};
	

	add(POST,"/event", myRouteHandler, &config);

	add(GET, "/index", index, &config);

	run(&config);
}


