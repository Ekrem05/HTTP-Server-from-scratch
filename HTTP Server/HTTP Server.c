#include "server.h"
void myRouteHandler(const char* request, char* response, size_t* response_len)
{
	// Handle the request and prepare the response
	strcpy(response, "This is the response from myRouteHandler.");
	*response_len = strlen(response);
}

int main()
{
	ServerConfig config = {.port = 6999};
	

	add(POST,"/event", myRouteHandler, &config);

	run(&config);
}


