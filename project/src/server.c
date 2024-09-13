#include "server.h"
#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
struct ThreadParams
{
    SOCKET clientSocket;
    ServerConfig* config;
};
DWORD WINAPI handleClient(LPVOID lpParam);

int run(ServerConfig* config)
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return result;
    }

    struct addrinfo* addrInfo = NULL;
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Convert port to string
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", config->port);

    int res = getaddrinfo(NULL, port_str, &hints, &addrInfo);
    if (res != 0)
    {
        printf("Error getting address info.\n");
        return res;
    }

    // Create socket
    SOCKET servsock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (servsock == INVALID_SOCKET)
    {
        printf("Error creating socket.\n");
        freeaddrinfo(addrInfo);
        return WSAGetLastError();
    }

    if (bind(servsock, addrInfo->ai_addr, (int) addrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        printf("Error binding the socket.\n");
        freeaddrinfo(addrInfo);
        closesocket(servsock);
        return WSAGetLastError();
    }

    freeaddrinfo(addrInfo);

    if (listen(servsock, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(servsock);
        return WSAGetLastError();
    }

    while (1)
    {
        struct ThreadParams* params = (struct ThreadParams*) malloc(sizeof(struct ThreadParams));
        if (params == NULL)
        {
            printf("Memory allocation failed\n");
            continue; // Skip this iteration and try to accept a new connection
        }

        params->clientSocket = accept(servsock, NULL, NULL);
        params->config = config; // Set the config

        if (params->clientSocket == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            free(params);
            continue;
        }
        else
        {
            int threadId;
            HANDLE handler = CreateThread(NULL, 0, handleClient, params, 0, &threadId);
            CloseHandle(handler);
        }
    }

    closesocket(servsock);
    WSACleanup();

    return 0;
}

int addRoute(ServerConfig* config, HttpMethod method, char* route, RouteHandler* handler )
{
    Route newRoute;
    newRoute.method = method;
    strcpy_s(newRoute.route, MAX_ROUTE_NAME_LENGTH,route);
    newRoute.handler = handler;
    return 0;
}

DWORD handleClient(LPVOID lpParam)
{
    struct ThreadParams* params = (struct ThreadParams*) lpParam;
    ServerConfig* config = params->config;
    SOCKET clientSocket = params->clientSocket;

    char buff[8192];
    free(lpParam);

    int bytesReceived = recv(clientSocket, buff, sizeof(buff) - 1, 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        printf("Recv failed: %d\n", WSAGetLastError());
        return 1;
    }

    
    buff[bytesReceived] = '\0';
    char* headerEnd = strstr(buff, "\r\n\r\n");
    if (headerEnd == NULL)
    {
        printf("Headers incomplete\n");
        return 1;
    }

    int headerLength = headerEnd - buff + 4;  

    
    Request request;
    strcpy_s(&request.body,sizeof(request.body), &buff); //pass the whole request as the body for now
    
    getHeaders(&request.headers, headerLength, buff);
    router(&request, &config);

    const char* httpResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 31\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello from my first HTTP server!";

    if (send(clientSocket, httpResponse, (int) strlen(httpResponse), 0) == SOCKET_ERROR)
    {
        printf("Send failed: %d\n", WSAGetLastError());
        return 1;
    }


    if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        return 1;
    }
    closesocket(clientSocket);
}

int getHeaders(Header* headers,int length, char* buff)
{
    char* methodNameEnd = strchr(buff, ' ');
    if (methodNameEnd == NULL) return 0;
    int methodNameSize = methodNameEnd - buff;
    
    char methodName[8];
    strncpy_s(&methodName,8,buff,methodNameSize);
    
    if (strcmp("GET", methodName) != 0
        && strcmp("POST", methodName) != 0
        && strcmp("PUT", methodName) != 0
        && strcmp("PATCH", methodName) != 0
        && strcmp("DELETE", methodName) != 0
        && strcmp("HEAD", methodName) != 0
        && strcmp("OPTIONS", methodName) != 0)
    {
        // Invalid method: Send 405 Method Not Allowed response
    }
    int headersCount = 0;
    
    strcpy_s(headers[headersCount].key, MAX_KEY_LENGTH,"Method");
    strcpy_s(headers[headersCount].value, MAX_VALUE_LENGTH,methodName);
    headersCount++;


    char *routeNameEnd = strchr(methodNameEnd+1,' ');
    if (routeNameEnd == NULL) return 0;

    int routeNameSize = routeNameEnd - methodNameEnd;

    char route[MAX_URL_LENGTH];
    strncpy_s(&route, MAX_URL_LENGTH, methodNameEnd+1, routeNameSize);
    
    strcpy_s(headers[headersCount].key, MAX_KEY_LENGTH, "Route");
    strcpy_s(headers[headersCount].value, MAX_URL_LENGTH, route);

    headersCount++;

    char *httpVersionEnd = strstr(routeNameEnd +1,"\r\n");
    if (httpVersionEnd == NULL) return 0;

    int httpVersionSize = httpVersionEnd - routeNameEnd;

    char version[20];
    strncpy_s(&version, 20, routeNameEnd + 1, httpVersionSize-1);

    strcpy_s(headers[headersCount].key, MAX_KEY_LENGTH, "Version");
    strcpy_s(headers[headersCount].value, MAX_VALUE_LENGTH, version);
    headersCount++;

    char* additionalHeadersBegin = strstr(buff, "\r\n");
    char* additionalHeadersEnd = strstr(buff,"\r\n\r\n");
    
    int headersSize = additionalHeadersEnd - additionalHeadersBegin;
    char currentLine[MAX_ADDITIONAL_HEADERS_LENGTH];
    strncpy_s(&currentLine, MAX_ADDITIONAL_HEADERS_LENGTH, additionalHeadersBegin+2, headersSize+2);
    while (currentLine)
    {
        if (strcmp("\r\n", currentLine) == 0)
        {
            break;
        }
        char* endOfCurrentLine = strstr(currentLine,"\r\n");
        if (endOfCurrentLine == NULL) return 0;

        char* endOfKey = strchr(currentLine,':');
        int keySize = endOfKey-currentLine;
        char headerKey[MAX_KEY_LENGTH];
        strncpy_s(&headerKey,MAX_KEY_LENGTH,currentLine,keySize);
        
        int valueSize = endOfCurrentLine - endOfKey;
        char headerValue[MAX_VALUE_LENGTH];
        strncpy_s(&headerValue, MAX_VALUE_LENGTH, endOfKey+2, valueSize-2);
        strcpy_s(headers[headersCount].key, sizeof(headerKey)/sizeof(char), headerKey);
        strcpy_s(headers[headersCount].value, sizeof(headerValue) / sizeof(char), headerValue);
        headersCount++;
        char* nextLine = strstr(currentLine, "\r\n");

        strcpy_s(currentLine, MAX_KEY_LENGTH+MAX_VALUE_LENGTH, nextLine+2);
    }

    for (size_t i = 0; i < headersCount; i++)
    {
        printf("Key: %s\n",headers[i].key);
        printf("Value: %s\n",headers[i].value);

    }

}