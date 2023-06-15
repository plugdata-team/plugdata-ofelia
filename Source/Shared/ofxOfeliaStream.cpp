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

#include "ofxOfeliaStream.h"


bool ofxOfeliaStream::bind(int recv_port, int snd_port) {
    send_port = snd_port;
    
    sendSocket = std::make_unique<SocketType>(snd_port);
    receiveSocket = std::make_unique<SocketType>(recv_port);
    
#ifdef PD
    sendSocket->acceptConnction();
    receiveSocket->acceptConnection();
#else
    sendSocket->openConnection();
    receiveSocket->openConnection();
#endif
    
    return true;
}

void ofxOfeliaStream::send(std::string toSend)
{
    sendSocket->sendData(reinterpret_cast<const void*>(toSend.data()), toSend.size());
}

std::string ofxOfeliaStream::receive(bool blocking)
{
    char data[60000];
    auto receivedLength = receiveSocket->receiveData(data, 60000);
        
    if(receivedLength > 0)
    {
        return std::string(data, receivedLength);
    }
    
    return {};
}

