#include "server.h"
int main()
{
	ServerConfig config = {.port = 6999};
	initialize_server(&config);
}


