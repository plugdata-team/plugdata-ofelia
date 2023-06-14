/*
 // Copyright (c) 2023 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "ofxOfeliaMessageManager.h"


ofxOfeliaMessageManager::ofxOfeliaMessageManager(int portNumber)
{
    pipe.bind(portNumber, portNumber + 1); // TODO: terminate if this fails
    returnPipe.bind(portNumber + 2, portNumber + 3);
    
    // Let plugdata know we have started up
    returnPipe.sendMessage(ofx_lua_init);
    
    shouldQuit = false;
    
    udpThread = std::thread(&ofxOfeliaMessageManager::run, this);
    
    instance = this;
}

ofxOfeliaMessageManager::~ofxOfeliaMessageManager()
{
    shouldQuit = true;
    udpThread.join();
}


void ofxOfeliaMessageManager::run()
{
    std::string message;
    while (!shouldQuit)
    {
        message = pipe.receive();
        
        if(message.empty())
        {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }
            
        std::string args;
        
        auto istream = std::stringstream(message);

        // Remove type of message to return later
        ofxMessageType messageType;
        istream.read(reinterpret_cast<char *>(&messageType), sizeof(ofxMessageType));

        /*
        if (messageType == ofx_audio_block)
        {
            signalQueue.enqueue();
            continue;
        } */
        
        // Put args into string
        char ch;
        while (istream.get(ch)) {
            args += ch;  // Append each character to the string
        }
        
        messageQueue.enqueue({messageType, args});
    }
}

bool ofxOfeliaMessageManager::receiveMessage(ofxMessageType& type, std::string& message)
{
    std::pair<ofxMessageType, std::string> result;
    auto dequeued = messageQueue.try_dequeue(result);
    if(dequeued)
    {
        type = result.first;
        message = result.second;
        return true;
    }
    
    return false;
}
ofxOfeliaMessageManager* ofxOfeliaMessageManager::instance = nullptr;

