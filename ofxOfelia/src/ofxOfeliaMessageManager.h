#pragma once
#include <functional>
#include <mutex>

struct ofxOfeliaAudioLock
{
    ofxOfeliaAudioLock();
    ~ofxOfeliaAudioLock();
};

struct ofxOfeliaLock
{
    ofxOfeliaLock();
    ~ofxOfeliaLock();
    
    static std::recursive_mutex ofeliaLock;
};

struct ofxOfeliaMessageManager
{
    static void setRunLoop(std::function<void()> fn);

    static void callOnMessageThread(std::function<void()> fn);
};

