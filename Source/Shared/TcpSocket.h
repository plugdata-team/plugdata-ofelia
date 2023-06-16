/*
 * Cross-platform compatibility superclass for sockets
 *
 * Copyright (C) 2019 Simon D. Levy
 * Modified by Timothy Schoen in 2023
 *
 * MIT License
 */

#pragma once

#include <cstdint>
#include <stdio.h>

class TcpSocket
{

public:

    TcpSocket(unsigned short port);
    
    ~TcpSocket();

    bool sendData(const char *buf, size_t len);
    size_t receiveData(char *buf, size_t len);
    bool isConnected();
    
    void setBlocking(bool blocking);
    
private:

    void closeConnection(void);
    
    bool initWinsock(void);
    void cleanup(void);
    void inetPton(const char * host, struct sockaddr_in & saddr_in);
    void setUdpTimeout(uint32_t msec);
    
protected:
    char _port[10];
    struct addrinfo * _addressInfo;
    bool _connected;
    int _conn;
    int _sock;
};

class TcpClientSocket : public TcpSocket {

public:

    TcpClientSocket(unsigned short port);

    void openConnection(void);
};

class TcpServerSocket : public TcpSocket {

public:

    TcpServerSocket(unsigned short port);

    void acceptConnection(void);
};
