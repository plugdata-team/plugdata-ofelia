#pragma once
#include <functional>
#include <mutex>

extern void ofelia_set_run_loop(std::function<void()>);
extern void ofelia_call_async(std::function<void()>);
extern void ofelia_audio_lock();
extern void ofelia_audio_unlock();

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

struct ofxOfeliaAsync
{
    static void setRunLoop(std::function<void()> fn);

    static void callAsync(std::function<void()> fn);
};


