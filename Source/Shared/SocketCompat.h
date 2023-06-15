/*
 * Cross-platform compatibility superclass for sockets
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#pragma once

#ifndef _WIN32
#define sprintf_s sprintf
typedef int SOCKET;
#endif

#include <stdio.h>
#include <cstdint>

class Socket {

    protected:

        SOCKET _sock;

        char _message[200];

        bool initWinsock(void);

        void cleanup(void);

        void inetPton(const char * host, struct sockaddr_in & saddr_in);

        void setUdpTimeout(uint32_t msec);

    public:

        void closeConnection(void);

        char * getMessage(void);
};
