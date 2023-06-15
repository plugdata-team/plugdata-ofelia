/*
 * Cross-platform compatibility superclass for sockets
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#pragma once

#include "SocketCompat.h"

class TcpSocket : public Socket {

    protected:
        char _port[10];
        SOCKET _conn;
        struct addrinfo * _addressInfo;
        bool _connected;

        TcpSocket(const short port);

    public:

        bool sendData(const void *buf, size_t len);
        size_t receiveData(void *buf, size_t len);
        bool isConnected();
};

class TcpClientSocket : public TcpSocket {

    public:

    TcpClientSocket(const short port);

    void openConnection(void);
};

class TcpServerSocket : public TcpSocket {

    public:

    TcpServerSocket(short port);

    void acceptConnection(void);
};
