#include "ofxOfeliaStream.h"

#include <cstring>
#include <cassert>
#include <cstdint>
#include <vector>
#include <tuple>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef PD
#include <m_pd.h>
#else
#include "ofxPdInterface.h"
#endif

ofxOfeliaStream::ofxOfeliaStream() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
    }
#endif
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        std::cerr << "Failed to create socket." << std::endl;
    }
}

ofxOfeliaStream::~ofxOfeliaStream() {
#ifdef _WIN32
    closesocket(socket_);
    WSACleanup();
#else
    close(socket_);
#endif
}


bool ofxOfeliaStream::bind(int receive_port, int send_port) {
    send_port_ = send_port;
    
    struct sockaddr_in address_;
    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = htonl(INADDR_ANY);
    address_.sin_port = htons(receive_port);

    if (::bind(socket_, (struct sockaddr*)&address_, sizeof(address_)) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;
        return false;
    }

    return true;
}

std::string ofxOfeliaStream::receive(bool blocking)
{
    std::string rcvd;
    if(blocking)
    {
        rcvd = receiveBlocking();
    }
    else
    {
        rcvd = receiveNonBlocking();
    }
    
    return rcvd;
}

bool ofxOfeliaStream::convertAddress(struct sockaddr_in* destAddr) {
#ifdef _WIN32
    int result = InetPton(AF_INET, L"127.0.0.1", &(destAddr->sin_addr));
    return (result == 1);
#else
    int result = inet_pton(AF_INET, "127.0.0.1", &(destAddr->sin_addr));
    return (result == 1);
#endif
}

std::string ofxOfeliaStream::receiveBlocking() {
    char buffer[buffer_size];
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(srcAddr);

    int bytesRead = ::recvfrom(socket_, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr*)&srcAddr, &addrLen);
    if (bytesRead > 0) {
        return std::string(buffer, bytesRead);
    }

    return {};
}

std::string ofxOfeliaStream::receiveNonBlocking() {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(socket_, FIONBIO, &mode);
#else
    int flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
#endif

    char buffer[buffer_size];
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(srcAddr);

    int bytesRead = ::recvfrom(socket_, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr*)&srcAddr, &addrLen);

#ifdef _WIN32
    mode = 0;
    ioctlsocket(socket_, FIONBIO, &mode);
#else
    flags = fcntl(socket_, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(socket_, F_SETFL, flags);
#endif

    if (bytesRead > 0) {            
        return std::string(buffer, bytesRead);
    }

    return {};
}

void ofxOfeliaStream::sendto(std::string message)
{
    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(send_port_);
    if (convertAddress(&destAddr) <= 0) {
        std::cerr << "Failed to convert IP address." << std::endl;
        return;
    }
    
    ::sendto(socket_, message.c_str(), message.size(), 0,
                    (struct sockaddr*)&destAddr, sizeof(destAddr));
}
