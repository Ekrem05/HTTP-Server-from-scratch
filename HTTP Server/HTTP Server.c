#include "server.h"
void index(HttpContext* context)
{
	
	HttpStatus status = OK;
	Response response = {
		.body="This is the Index endpoint",
		.contentType=CONTENT_TYPE_TEXT_PLAIN,
		.status=OK
	};
	context->response=&response;
}

void user(HttpContext* context)
{
	
	HttpStatus status = OK;
	context->response->status = status;
	context->response->contentType = CONTENT_TYPE_APPLICATION_JSON;
	strcpy(context->response->body, "This is the response from myRouteHandler.");
}

int main()
{
	ServerConfig config = {.port = 6999};
	
	add(DELETE, "/user", user, &config);

	add(GET, "/index", index, &config);

	run(&config);
}


