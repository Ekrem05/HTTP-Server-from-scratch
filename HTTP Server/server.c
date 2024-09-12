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

int initialize_server(const ServerConfig* config)
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
        SOCKET* clientSockPtr = malloc(sizeof(SOCKET));

        *clientSockPtr = accept(servsock, NULL, NULL);

        if (clientSockPtr == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(servsock);
            WSACleanup();
            return 1;
        }
        else
        {
            int threadId;
            HANDLE handler = CreateThread(NULL, 0, handleClient, clientSockPtr, 0, &threadId);
            CloseHandle(handler);
        }
    }

    closesocket(servsock);
    WSACleanup();

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

    //printf("Headers:\n%.*s\n", headerLength, buff);
    Header headers[MAX_NUMBER_OF_HEADERS];
    getHeaders(&headers, headerLength,buff);
    //router(buff, config);
    //Pass to the router

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
getHeaders(Header* headers,int length, char* buff)
{
    char* methodNameEnd = strchr(buff, ' ');
    if (methodNameEnd == NULL) return;
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
    if (routeNameEnd == NULL) return;

    int routeNameSize = routeNameEnd - methodNameEnd;

    char route[MAX_URL_LENGTH];
    strncpy_s(&route, MAX_URL_LENGTH, methodNameEnd+1, routeNameSize);
    
    strcpy_s(headers[headersCount].key, MAX_KEY_LENGTH, "Route");
    strcpy_s(headers[headersCount].value, MAX_URL_LENGTH, route);

    headersCount++;

    char *httpVersionEnd = strstr(routeNameEnd +1,"\r\n");
    if (httpVersionEnd == NULL) return;

    int httpVersionSize = httpVersionEnd - routeNameEnd;

    char version[20];
    strncpy_s(&version, 20, routeNameEnd + 1, httpVersionSize-1);

    strcpy_s(headers[headersCount].key, MAX_KEY_LENGTH, "Version");
    strcpy_s(headers[headersCount].value, MAX_VALUE_LENGTH, version);
    headersCount++;

    char* additionalHeadersBegin = strstr(buff, "\r\n");
    char* additionalHeadersEnd = strstr(buff,"\r\n\r\n");
    

    char* currentLine = additionalHeadersBegin+2;
    while (currentLine)
    {
        if (strcmp("\r\n", currentLine) == 0)
        {
            break;
        }
        char* endOfCurrentLine = strstr(currentLine,"\r\n");
        if (endOfCurrentLine == NULL) return;

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