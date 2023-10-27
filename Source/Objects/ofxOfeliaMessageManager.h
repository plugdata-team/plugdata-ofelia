/*
 // Copyright (c) 2023 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once
#include "../Shared/ofxOfeliaStream.h"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <vector>
#include <tuple>
#include <map>
#include <atomic>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <z_libpd.h>

class TimerThread
{
public:
    TimerThread() : running(false) {}

    virtual ~TimerThread() {
        stop();
    }

    void startTimer(int intervalMillis)
    {
        if (!running)
        {
            running = true;
            timerThread = std::thread([this, intervalMillis]() {
                while (running)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    callbackLock.lock();
                    timerCallback();
                    callbackLock.unlock();
                    auto end = std::chrono::high_resolution_clock::now();

                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    auto sleepTime = std::chrono::milliseconds(intervalMillis) - duration;

                    if (sleepTime > std::chrono::milliseconds::zero())
                        std::this_thread::sleep_for(sleepTime);
                }
            });
        }
    }

    void stop()
    {
        running = false;
        callbackLock.lock();
        timerThread.join();
        callbackLock.unlock();
    }

    virtual void timerCallback() = 0;

private:
    std::thread timerThread;
    std::atomic<bool> running;
    std::mutex callbackLock;
};


class Semaphore {
    std::mutex mutex_;
    std::condition_variable condition_;
    bool isLocked_ = true; // Initialized as locked.

public:
    void release() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        isLocked_ = false;
        condition_.notify_one();
    }

    void acquire() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while (isLocked_)
            condition_.wait(lock);
    }
};

struct ofxOfeliaMessageListener {
    virtual void receiveMessage(ofxMessageType type, const std::string& message) = 0;
};

struct ofxOfeliaMessageManager : public TimerThread, public ofxOfeliaMessageListener {
    
    ofxOfeliaMessageManager(t_pdinstance* pdInstance) : pdthis(pdInstance)
    {
    }
    
    ~ofxOfeliaMessageManager()
    {
        sendMessage(ofx_quit);
    }
    
    bool bind(int port)
    {
        pipe.initialise(port + 1, port);
        returnPipe.initialise(port + 3, port + 2);
        
        auto bound1 = pipe.bind();
        auto bound2 = returnPipe.bind();
        
        auto success = bound1 && bound2;

        if(success) {
            
            // Blocking receive on returnPipe to wait for startup signal
            //returnPipe.receive(true);
            instancesLock.lock();
            initWaits[pdthis].release();
            instancesLock.unlock();
            
            startTimer(3);
        }
        
        return success;
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
    
    static ofxOfeliaMessageManager* get()
    {
        
        auto* pdthis = libpd_this_instance();
        
        // If necessary, wait until child process is initialised
        instancesLock.lock();
        auto& instanceWait = initWaits[pdthis];
        instancesLock.unlock();
        
        instanceWait.acquire();
        
        instancesLock.lock();
        auto* instance = instances[pdthis].get();
        instancesLock.unlock();
        
        return instance;
    }
    static ofxOfeliaMessageManager* initialise()
    {
        auto* pdthis = libpd_this_instance();
    
        auto* messageManager = new ofxOfeliaMessageManager(pdthis);
        
        instancesLock.lock();
        instances[pdthis].reset(messageManager);
        instancesLock.unlock();
        
        messageManager->addListener(messageManager);
        
        return messageManager;
    }

    void timerCallback() override
    {
        libpd_set_instance(pdthis);
        
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
            
            listenerLock.lock();
            for(auto listener : listeners)
            {
                listener->receiveMessage(messageType, args);
            }
            listenerLock.unlock();
           
            message = pipe.receive();
        }
    }
    
    void addListener(ofxOfeliaMessageListener* listener)
    {
        listenerLock.lock();
        listeners.push_back(listener);
        listenerLock.unlock();
    }
    
    void removeListener(ofxOfeliaMessageListener* listener)
    {
        listenerLock.lock();
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
        listenerLock.unlock();
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
            case pd_send_bang:
            {
                auto [send] = parseMessage<std::string>(message);
                
                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_bang(target->s_thing);
                sys_unlock();
                
                break;
            }
            case pd_send_float:
            {
                auto [send, value] = parseMessage<std::string, float>(message);
                
                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_float(target->s_thing, value);
                sys_unlock();
                
                break;
            }
            case pd_send_symbol:
            {
                auto [send, symbol] = parseMessage<std::string, std::string>(message);
                
                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_symbol(target->s_thing, gensym(symbol.c_str()));
                sys_unlock();

                break;
            }
            case pd_send_pointer:
            {
                auto [identifier, send, ptr] = parseMessage<std::string, std::string, intptr_t>(message);
                
                t_gpointer gpointer;
                gpointer_init(&gpointer);
                gpointer.gp_stub = reinterpret_cast<t_gstub*>(ptr);
                
                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_pointer(target->s_thing, &gpointer);
                sys_unlock();
                
                break;
            }
            case pd_send_list:
            {
                auto [identifier, send, atoms] = parseMessage<std::string, std::string, std::vector<t_atom>>(message);

                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_list(target->s_thing, gensym("list"), atoms.size(), atoms.data());
                sys_unlock();
                
                break;
            }
            case pd_send_anything:
            {
                auto [identifier, send, symbol, atoms] = parseMessage<std::string, std::string, std::string, std::vector<t_atom>>(message);
                
                sys_lock();
                auto* target = gensym(send.c_str());
                if(target->s_thing) pd_anything(target->s_thing, gensym(symbol.c_str()), atoms.size(), atoms.data());
                sys_unlock();
                
                break;
            }
                
            case pd_value_get:
            {
                auto [name] = parseMessage<std::string>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                float value = 0.0f;
                if(auto* valPtr = value_get(sym)) value = *valPtr;
                sys_unlock();
                
                sendReturnValue<float>(value);
                
                break;
            }
            case pd_value_set:
            {
                auto [name, value] = parseMessage<std::string, float>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                if(auto* valPtr = value_get(sym)) *valPtr = value;
                sys_unlock();
                
                break;
            }
            case pd_array_get:
            {
                auto [name, values] = parseMessage<std::string, std::vector<t_atom>>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                auto* a = reinterpret_cast<t_garray *>(pd_findbyclass(sym, garray_class));
                
                int size; t_word *vec;
                if (!a || !garray_getfloatwords(a, &size, &vec))
                {
                    pd_error(NULL, "ofelia: bad template for array '%s'", sym->s_name);
                    sys_unlock();
                    
                    sendReturnValue(std::vector<t_atom>());
                    break;
                }
                
                std::vector<t_atom> atoms;
                for(int i = 0; i < size; i++)
                {
                    atoms[i].a_type = A_FLOAT;
                    atoms[i].a_w = vec[i];
                }
                sys_unlock();
                
                sendReturnValue(atoms);

                break;
            }
            case pd_array_set:
            {
                auto [name, values, onset] = parseMessage<std::string, std::vector<t_atom>, int>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                auto* a = reinterpret_cast<t_garray *>(pd_findbyclass(sym, garray_class));
                
                int size; t_word *vec;
                if (!a || !garray_getfloatwords(a, &size, &vec))
                {
                    pd_error(NULL, "ofelia: bad template for array '%s'", sym->s_name);
                    sys_unlock();
                    break;
                }
                
                if (onset < 0) onset = 0;
                for (int i = 0; i < values.size(); ++i)
                {
                    int io = i + onset;
                    if (io < size) vec[io].w_float = atom_getfloat(&values[i]);
                    else break;
                }
                
                garray_redraw(a);
                sys_unlock();
                
                break;
            }
            case pd_array_get_size:
            {
                auto [name] = parseMessage<std::string>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                auto* a = reinterpret_cast<t_garray *>(pd_findbyclass(sym, garray_class));
                
                if(!a)
                {
                    sys_unlock();
                    break;
                }
                
                int size = garray_npoints(a);
                sys_unlock();
                
                sendReturnValue(size);
                
                break;
            }
            case pd_array_set_size:
            {
                
                auto [name, size] = parseMessage<std::string, int>(message);
                
                sys_lock();
                auto* sym = gensym(name.c_str());
                auto* a = reinterpret_cast<t_garray *>(pd_findbyclass(sym, garray_class));
                
                if(!a)
                {
                    sys_unlock();
                    break;
                }
                
                garray_resize_long(a, size);
                sys_unlock();

                break;
            }
                
            default: break;
        }
    }
        
    std::recursive_mutex listenerLock;
    
private:
    ofxOfeliaStream pipe;
    ofxOfeliaStream returnPipe;
    
    t_pdinstance* pdthis;
    static inline std::map<t_pdinstance*, Semaphore> initWaits;
    static inline std::map<t_pdinstance*, std::unique_ptr<ofxOfeliaMessageManager>> instances;
    static inline std::mutex instancesLock;
    
    std::vector<ofxOfeliaMessageListener*> listeners;
    
};
