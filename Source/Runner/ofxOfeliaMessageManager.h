/*
 // Copyright (c) 2023 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once
#include "ofxOfeliaStream.h"
#include <iostream>
#include <sstream>
#include <string>


struct ofxOfeliaMessageManager {
    
    ofxOfeliaMessageManager()
    {
        pipe.bind(12015, 12014); // TODO: terminate if this fails
        returnPipe.bind(12017, 12016);
        
        instance = this;
    }
    
    virtual void receiveMessage(ofxMessageType type, const std::string& message) = 0;
    
    // Formats message to stringstream and sends it to the other process
    template <typename... Types>
    void sendMessage(Types... args)
    {
        pipe.sendMessage(args...);
    }
    
    template <typename... Types>
    static std::tuple<Types...> parseMessage(const std::string &message)
    {
        return ofxOfeliaStream::parseMessage<Types...>(message);
    }
    
    void run(bool blocking = false)
    {
        auto message = pipe.receive(blocking);

        while(!message.empty())
        {
            std::string args;
            
            auto istream = std::stringstream(message);

            // Remove type of message to return later
            ofxMessageType messageType;
            istream.read(reinterpret_cast<char *>(&messageType), sizeof(ofxMessageType));

            // Put args into string
            char ch;
            while (istream.get(ch)) {
                args += ch;  // Append each character to the string
            }
            
            receiveMessage(messageType, args);
            
            if(!blocking) {
                message = pipe.receive(blocking);
            }
            else {
                message = "";
            }
        }
    }
    
    template <typename... Types>
    std::tuple<Types...> waitForReturnValue()
    {
        auto message = returnPipe.receive(true);
        return ofxOfeliaStream::parseMessage<Types...>(message);
    }
    
    static inline ofxOfeliaMessageManager* instance;
    
private:

    ofxOfeliaStream pipe;
    ofxOfeliaStream returnPipe;
};