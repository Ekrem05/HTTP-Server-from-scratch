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

    printf("%s", buff);
    buff[bytesReceived] = '\0';
    char* headerEnd = strstr(buff, "\r\n\r\n");
    if (headerEnd == NULL)
    {
        printf("Headers incomplete\n");
        return 1;
    }

    int headerLength = headerEnd - buff + 4;  

    printf("Headers:\n%.*s\n", headerLength, buff);
    Header headers;
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
    int methodNameSize = methodNameEnd - buff;
    
    char *routeNameEnd = strchr(methodNameEnd+1,' ');
    int routeNameSize = routeNameEnd - methodNameEnd;

    printf("Method name is %.*s\n", methodNameSize, buff);
    printf("Route is %.*s\n", routeNameSize, methodNameEnd + 1);


    //get headers

}