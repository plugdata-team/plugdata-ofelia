/*
 // Copyright (c) 2023 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once
#include "../Shared/ofxOfeliaStream.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <tuple>
#include <z_libpd.h>

class TimerThread
{
public:
    TimerThread() : running(false) {}

    virtual ~TimerThread() {}

    void startTimer(int intervalMillis)
    {
        if (!running)
        {
            running = true;
            std::thread([this, intervalMillis]() {
                while (running)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    timerCallback();
                    auto end = std::chrono::high_resolution_clock::now();

                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    auto sleepTime = std::chrono::milliseconds(intervalMillis) - duration;

                    if (sleepTime > std::chrono::milliseconds::zero())
                        std::this_thread::sleep_for(sleepTime);
                }
            }).detach();
        }
    }

    void stop()
    {
        running = false;
    }

    virtual void timerCallback() = 0;

private:
    bool running;
};

class OfeliaChildProcess
{
public:
    OfeliaChildProcess() : running(false) {}
    
    void start(const std::string& command)
    {
        if (!running)
        {
            running = true;
            std::thread([this, command]() {
                int exitCode = std::system(command.c_str());
                //handleExit(exitCode);
                running = false;
            }).detach();
        }
    }

    /*
    void kill()
    {
#ifdef _WIN32
        if (processId > 0)
        {
            HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
            if (processHandle != NULL)
            {
                TerminateProcess(processHandle, 0);
                CloseHandle(processHandle);
            }
        }
#else
        if (processId > 0)
            ::kill(processId, SIGTERM);
#endif
        running = false;
    } */
    
private:
    std::atomic_bool running;
};

struct ofxOfeliaMessageListener {
    virtual void receiveMessage(ofxMessageType type, const std::string& message) = 0;
};



struct ofxOfeliaMessageManager : public TimerThread, public ofxOfeliaMessageListener {
    
    ofxOfeliaMessageManager()
    {
        pdthis = libpd_this_instance();
        
        if (pipe.bind(12014, 12015)) {
            startTimer(5);
        }
        
        returnPipe.bind(12016, 12017);
    }
    
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
    
    static ofxOfeliaMessageManager* getOrCreate()
    {
        if(!instance)
        {
            instance = new ofxOfeliaMessageManager();
        }
     
        addListener(instance);
        return instance;
    }

    ~ofxOfeliaMessageManager()
    {
        //ofelia.kill();
    }

    void timerCallback() override
    {
        libpd_set_instance(pdthis);
        /*
        if(!ofelia.isRunning())
        {
            usleep(50000);
            if(!ofelia.isRunning())
            {
                ofelia.start("/Users/timschoen/Library/plugdata/0.8.0-0/ofelia");
            }
        } */
        
        auto message = pipe.receive();

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
            
            for(auto listener : listeners)
            {
                listener->receiveMessage(messageType, args);
            }
           
            message = pipe.receive();
        }
    }
    
    static void addListener(ofxOfeliaMessageListener* listener)
    {
        instance->listeners.push_back(listener);
    }
    
    static void removeListener(ofxOfeliaMessageListener* listener)
    {
        instance->listeners.erase(std::remove(instance->listeners.begin(), instance->listeners.end(), listener), instance->listeners.end());
    }
    
    template <typename... Types>
    void sendReturnValue(Types... args)
    {
        returnPipe.sendMessage(args...);
    }
    
    void receiveMessage(ofxMessageType type, const std::string& message) override
    {
        switch(type)
        {
            case pd_log:
            {
                auto [isError, mess] = parseMessage<bool, std::string>(message);
                if(isError)
                {
                    post(mess.c_str(), 1);
                }
                else {
                    logpost(NULL, 2, "%s", mess.c_str());
                }
                break;
            }
                
            default: break;
        }
    }
    
    t_pdinstance* pdthis;
    OfeliaChildProcess ofelia;
    ofxOfeliaStream pipe;
    ofxOfeliaStream returnPipe;
    static inline ofxOfeliaMessageManager* instance = nullptr;
    
private:
    std::vector<ofxOfeliaMessageListener*> listeners;
    
};
