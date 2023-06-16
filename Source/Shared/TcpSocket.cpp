/*
 * Cross-platform compatibility superclass for sockets
 *
 * Copyright (C) 2019 Simon D. Levy
 * Modified by Timothy Schoen in 2023
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
typedef SSIZE_T ssize_t;
// Unix
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
static const int INVALID_SOCKET = -1;
static const int SOCKET_ERROR   = -1;
using SOCKET = int;
#endif

#include <locale.h>
#include <wchar.h>
#include <iostream>
#include <thread>
#include <chrono>

#ifndef _WIN32
static void closesocket(int socket) { close(socket); }
#endif

// TcpSocket

TcpSocket::TcpSocket(unsigned short port)
{
    snprintf(_port, 10, "%d", port);
    // No connection yet
    _sock = INVALID_SOCKET;
    _connected = false;

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
        std::cerr << "getaddrinfo() failed with error: %d" << std::endl;
        cleanup();
        return;
    }

    // Create a SOCKET for connecting to server, returning on failure
    _sock = socket(_addressInfo->ai_family, _addressInfo->ai_socktype, _addressInfo->ai_protocol);
    if (_sock == INVALID_SOCKET) {
        std::cerr << "socket() failed" << std::endl;
        cleanup();
        return;
    }
}

TcpSocket::~TcpSocket()
{
#ifdef _WIN32
    closesocket(static_cast<SOCKET>(_conn));
#else
    close(static_cast<SOCKET>(_conn));
#endif
}

bool TcpSocket::sendData(const char *buf, size_t len)
{
    uint32_t messageLength = htonl(static_cast<uint32_t>(len));
    ssize_t bytesSent = send(static_cast<SOCKET>(_conn), reinterpret_cast<const char*>(&messageLength), sizeof(messageLength), 0);
    if (bytesSent != sizeof(messageLength)) {
        // Handle error or connection closed
        return false;
    }
    
    // Send the message content
    ssize_t totalBytesSent = 0;
    while (totalBytesSent < len) {
        ssize_t bytesSent = send(static_cast<SOCKET>(_conn), buf + totalBytesSent, len - totalBytesSent, 0);
        if (bytesSent <= 0) {
            // Handle error or connection closed
            return false;
        }
        totalBytesSent += bytesSent;
    }

    return true;
}

size_t TcpSocket::receiveData(char *buf, size_t len)
{
    uint32_t messageLength;
    ssize_t bytesRead = recv(static_cast<SOCKET>(_conn), reinterpret_cast<char*>(&messageLength), sizeof(messageLength), 0);
    if (bytesRead != sizeof(messageLength)) {
        // Handle error or connection closed
        return 0;
    }
    
    messageLength = ntohl(messageLength);
    
    size_t totalBytesReceived = 0;
    while (totalBytesReceived < messageLength) {
        ssize_t bytesReceived = recv(static_cast<SOCKET>(_conn), &buf[totalBytesReceived], messageLength - totalBytesReceived, 0);
        if (bytesReceived <= 0) {
            // Handle error or connection closed
            return 0;
        }
        totalBytesReceived += bytesReceived;
    }

    
    return totalBytesReceived;
}

void TcpSocket::setBlocking(bool blocking) {
    // Get the current socket flags
#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    if (ioctlsocket(static_cast<SOCKET>(_conn), FIONBIO, &mode) != 0) {
        // Failed to set socket mode
        return;
    }
#else
    int flags = fcntl(static_cast<SOCKET>(_conn), F_GETFL, 0);
    if (flags == -1) {
        // Failed to get socket flags
        return;
    }

    if (blocking) {
        flags &= ~O_NONBLOCK;  // Clear non-blocking flag
    } else {
        flags |= O_NONBLOCK;  // Set non-blocking flag
    }

    if (fcntl(static_cast<SOCKET>(_conn), F_SETFL, flags) == -1) {
        // Failed to set socket flags
        return;
    }
#endif

    return;
}

bool TcpSocket::isConnected()
{
    return _connected;
}

bool TcpSocket::initWinsock(void)
{
#ifdef _WIN32
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
        sprintf_s(_message, "WSAStartup() failed with error: %d\n", iResult);
        return false;
        }
#endif
        return true;
}

void TcpSocket::cleanup(void)
{
#ifdef _WIN32
        WSACleanup();
#endif
}

void TcpSocket::inetPton(const char * host, struct sockaddr_in & saddr_in)
{
#ifdef _WIN32
        WCHAR wsz[64];
        swprintf_s(wsz, L"%S", host);
        InetPton(AF_INET, wsz,   &(saddr_in.sin_addr.s_addr));
#else
        inet_pton(AF_INET, host, &(saddr_in.sin_addr));
#endif
}


void TcpSocket::closeConnection(void)
{
#ifdef _WIN32
        closesocket(_sock);
#else
        close(_sock);
#endif
}


// TcpClientSocket

TcpClientSocket::TcpClientSocket(unsigned short port)
    : TcpSocket(port)
{
}

void TcpClientSocket::openConnection(void)
{
    int trials = 0;
    while(!_connected && trials < 10) {
        // Connect to server, returning on failure
        if (connect(static_cast<SOCKET>(_sock), _addressInfo->ai_addr, (int)_addressInfo->ai_addrlen) == SOCKET_ERROR) {

            trials++;
            std::this_thread::sleep_for(std::chrono::milliseconds(trials * 50));
            
            continue;
        }
        
        // For a client, the connection is the same as the main socket
        _conn = _sock;
        
        // Success!
        _connected = true;
        return;
    }
    
    closesocket(static_cast<SOCKET>(_sock));
    _sock = INVALID_SOCKET;
    std::cerr <<  "connect() failed; please make sure server is running" << std::endl;
    
}

TcpServerSocket::TcpServerSocket(unsigned short port)
    : TcpSocket(port)
{
    // Bind socket to address
    if (bind(static_cast<SOCKET>(_sock), _addressInfo->ai_addr, (int)_addressInfo->ai_addrlen) == SOCKET_ERROR) {
        closesocket(static_cast<SOCKET>(_sock));
        _sock = INVALID_SOCKET;
        std::cerr <<  "bind() failed" << std::endl;
        return;
    }
}

void TcpServerSocket::acceptConnection(void)
{
    // Listen for a connection, exiting on failure
    if (listen(static_cast<SOCKET>(_sock), 1)  == -1) {
        std::cerr <<  "listen() failed" << std::endl;
        return;
    }

    fflush(stdout);
    _conn = accept(static_cast<SOCKET>(_sock), (struct sockaddr *)NULL, NULL);
    if (_conn == SOCKET_ERROR) {
        std::cerr <<  "accept() failed" << std::endl;
        return;
    }
    fflush(stdout);
}
