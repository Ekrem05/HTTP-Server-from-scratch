#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>  // For inet_pton and related functions

#pragma comment(lib, "ws2_32.lib")  // Link Winsock library

int main()
{
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return 1;
    }
    struct addrinfo* addrInfo = NULL;

    //We wanna "hint" the getaddrinfo function what kind of results we want from it
    //So it fills it out in the addrInfo variable
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
    hints.ai_protocol = IPPROTO_TCP;   // TCP protocol
    hints.ai_flags = AI_PASSIVE;

    int res = getaddrinfo(NULL, "8080", &hints, &addrInfo);
    if (res != 0)
    {
        printf("error getting the  addr info.\n");
        return 0;

    }

    // Create socket
    SOCKET servsock = socket(addrInfo->ai_family,addrInfo->ai_socktype,addrInfo->ai_protocol);
    if (servsock == INVALID_SOCKET)
    {
        printf("Error creating a socket.\n");
        return 0;
    }

    
    if (0!=bind(servsock, addrInfo->ai_addr, (int) addrInfo->ai_addrlen))
    {
        printf("Error binding the socket.\n");
        return 0;
    }
    freeaddrinfo(addrInfo);

    if (listen(servsock, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
    }

    //Temporary Socket object for accepting client connections
    SOCKET ClientSocket;
    ClientSocket = accept(servsock, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(servsock);
        WSACleanup();
        return 1;
    }

    //Receive data from the connected client socket

    char buff[512];

    while (recv(ClientSocket, buff, sizeof(buff) - 1, 0))
    {
        
       printf("%s",buff);
        const char* httpResponse =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 31\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "Hello from my first HTTP server!";

        if (send(ClientSocket, httpResponse, (int) strlen(httpResponse), 0) == SOCKET_ERROR)
        {
            printf("Send failed: %d\n", WSAGetLastError());
        }
       
    }
    if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        return 1;
    }
    closesocket(servsock);
    WSACleanup();
   
}


void printString(char* chars, int len)
{

}