#include <cstring>
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


void ofxOfeliaStream::initialise(int recv_port, int snd_port)
{
    sendSocket = std::make_unique<SocketType>(snd_port);
    receiveSocket = std::make_unique<SocketType>(recv_port);
}

bool ofxOfeliaStream::bind() {
    
#ifdef PD
    sendSocket->acceptConnection();
    receiveSocket->acceptConnection();
#else
    receiveSocket->openConnection();
    sendSocket->openConnection();
#endif
    
    return true;
}

void ofxOfeliaStream::quit()
{
    receiveSocket->sendData(reinterpret_cast<char*>(ofx_quit), sizeof(ofxMessageType));
}

void ofxOfeliaStream::send(std::string toSend)
{
    sendSocket->sendData(toSend.data(), toSend.size());
}

std::string ofxOfeliaStream::receive()
{
    auto receivedLength = receiveSocket->receiveData(buffer);
        
    if(receivedLength > 0)
    {
        return std::string(buffer.data(), receivedLength);
    }
    
    return {};
}

