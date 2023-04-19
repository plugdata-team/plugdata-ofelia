#pragma once
#include <functional>
#include <mutex>

extern void ofelia_set_run_loop(std::function<void()>);
extern void ofelia_call_async(std::function<void()>);
extern void ofelia_audio_lock();
extern void ofelia_audio_unlock();

struct ofxOfeliaAudioLock
{
    ofxOfeliaAudioLock()
    {
        ofelia_audio_lock();
    }

    ~ofxOfeliaAudioLock()
    {
        ofelia_audio_unlock();
    }
};

struct ofxOfeliaLock
{
    ofxOfeliaLock()
    {
        ofeliaLock.lock();
    }

    ~ofxOfeliaLock()
    {
        ofeliaLock.unlock();
    }
    
    static inline std::recursive_mutex ofeliaLock;
};

struct ofxOfeliaAsync
{
    static void setRunLoop(std::function<void()> fn)
    {
        const ofxOfeliaLock ofxLock;
        ofelia_set_run_loop(fn);
    }

    static void callAsync(std::function<void()> fn)
    {
        ofelia_call_async([fn](){
            const ofxOfeliaLock ofxLock;
            fn();
        });
    }
};


