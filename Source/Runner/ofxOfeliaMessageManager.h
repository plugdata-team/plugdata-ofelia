/*
 // Copyright (c) 2023 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once
#include "ofxPdInterface.h"
#include "../Shared/ofxOfeliaStream.h"
#include "../../Libraries/concurrentqueue/concurrentqueue.h"

#include <iostream>
#include <sstream>
#include <string>
#include <atomic>

struct ofxOfeliaMessageManager {
    
    ofxOfeliaMessageManager(int portNumber);
    
    ~ofxOfeliaMessageManager();
    
    bool receiveMessage(ofxMessageType& type, std::string& message);
    
    // Formats message to stringstream and sends it to the other process
    template <typename... Types>
    static void sendMessage(Types... args)
    {
        instance->pipe.sendMessage(args...);
    }
    
    template <typename... Types>
    static std::tuple<Types...> parseMessage(const std::string &message)
    {
        return ofxOfeliaStream::parseMessage<Types...>(message);
    }
    
    void run();
    
    template <typename... Types>
    static std::tuple<Types...> waitForReturnValue()
    {
        auto message = instance->returnPipe.receive(true);
        return ofxOfeliaStream::parseMessage<Types...>(message);
    }
    
    static ofxOfeliaMessageManager* instance;
    std::atomic<bool> shouldQuit;
    
private:
    std::thread udpThread;
    moodycamel::ConcurrentQueue<std::pair<ofxMessageType, std::string>> messageQueue;
    moodycamel::ConcurrentQueue<std::vector<float>> signalQueue;
    
    ofxOfeliaStream pipe;
    ofxOfeliaStream returnPipe;

};
