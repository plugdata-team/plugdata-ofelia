/*
 * Cross-platform compatibility superclass for sockets
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#pragma once

#include "TcpSocket.h"

// Windows
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <winsock2.h>
#include <ws2tcpip.h>

// Linux
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
static const int INVALID_SOCKET = -1;
static const int SOCKET_ERROR   = -1;
#endif


#ifndef _WIN32
static void closesocket(int socket) { close(socket); }
#endif

// TcpSocket

TcpSocket::TcpSocket(const short port)
{
    sprintf_s(_port, "%d", port);

    // No connection yet
    _sock = INVALID_SOCKET;
    _connected = false;
    *_message = 0;

    // Initialize Winsock, returning on failure
    if (!initWinsock()) return;

    // Set up client address info
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the server address and port, returning on failure
    _addressInfo = NULL;
    int iResult = getaddrinfo("127.0.0.1", _port, &hints, &_addressInfo);
    if ( iResult != 0 ) {
        sprintf_s(_message, "getaddrinfo() failed with error: %d", iResult);
        cleanup();
        return;
    }

    // Create a SOCKET for connecting to server, returning on failure
    _sock = socket(_addressInfo->ai_family, _addressInfo->ai_socktype, _addressInfo->ai_protocol);
    if (_sock == INVALID_SOCKET) {
        sprintf_s(_message, "socket() failed");
        cleanup();
        return;
    }
}

bool TcpSocket::sendData(const void *buf, size_t len)
{
    return (size_t)send(_conn, (const char *)buf, len, 0) == len;
}

size_t TcpSocket::receiveData(void *buf, size_t len)
{
    return (size_t)recv(_conn, (char *)buf, len, 0);
}
bool TcpSocket::isConnected()
{
    return _connected;
}


// TcpClientSocket

TcpClientSocket::TcpClientSocket(const short port)
    : TcpSocket(port)
{
}

void TcpClientSocket::openConnection(void)
{
    // Connect to server, returning on failure
    if (connect(_sock, _addressInfo->ai_addr, (int)_addressInfo->ai_addrlen) == SOCKET_ERROR) {
        closesocket(_sock);
        _sock = INVALID_SOCKET;
        sprintf_s(_message, "connect() failed; please make sure server is running");
        return;
    }

    // For a client, the connection is the same as the main socket
    _conn = _sock;

    // Success!
    _connected = true;
}

TcpServerSocket::TcpServerSocket(short port)
    : TcpSocket(port)
{
    // Bind socket to address
    if (bind(_sock, _addressInfo->ai_addr, (int)_addressInfo->ai_addrlen) == SOCKET_ERROR) {
        closesocket(_sock);
        _sock = INVALID_SOCKET;
        sprintf_s(_message, "bind() failed");
        return;
    }
}

void TcpServerSocket::acceptConnection(void)
{
    // Listen for a connection, exiting on failure
    if (listen(_sock, 1)  == -1) {
        sprintf_s(_message, "listen() failed");
        return;
    }

    // Accept connection, exiting on failure
    printf("Waiting for client to connect on %s:%s ... ", "127.0.0.1", _port);
    fflush(stdout);
    _conn = accept(_sock, (struct sockaddr *)NULL, NULL);
    if (_conn == SOCKET_ERROR) {
        sprintf_s(_message, "accept() failed");
        return;
    }
    printf("connected\n");
    fflush(stdout);
}
