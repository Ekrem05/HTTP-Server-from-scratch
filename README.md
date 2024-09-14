
<h1 align="center">🌐🌐 HTTP Server in C 🌐🌐</h1>

## Overview

This project is a work-in-progress HTTP server implemented in C using Winsock. I aim to provide a flexible and customizable framework for handling HTTP requests and responses. Users will be able to add their own routes and handle functions to extend the server's capabilities.

## Features

- **Modular Design:** Allows users to add custom routes and request handlers.
- **Configurable Settings:** Users can set their desired port number and other parameters.

## Current Limitations 🔒🔒

  - **Maximum characters for HTTP header keys:** 256
  - **Maximum characters for HTTP header values:** 1024
  - **Maximum number of HTTP headers per request:** 100
  - **Maximum number of routes the server can handle:** 100
  - **Maximum length of the URL:** 2000

## Getting Started

### Prerequisites

- Windows operating system
- Visual Studio or another C compiler that supports Winsock

### Usage

- Configure
```c
// Choose a port for the server to run on

#include "server.h"

int main() {
   ServerConfig config = {.port = 6999};
}
```
- Endpoints
```c
// Add your handle functions
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
	strcpy(context->response->body, "User deleted");
}
```
- Add your endpoints to the config and run
```c
// Add and run
int main()
{
	ServerConfig config = {.port = 6999};
	
	add(DELETE, "/user", user, &config);

	add(GET, "/index", index, &config);

	run(&config);
}
