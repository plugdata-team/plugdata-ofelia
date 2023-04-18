#pragma once

extern void ofelia_set_run_loop(std::function<void()>);
extern void ofelia_call_async(std::function<void()>);
extern void ofelia_lock();
extern void ofelia_unlock();
extern void ofelia_audio_lock();
extern void ofelia_audio_unlock();

struct ofxOfeliaAsync
{
    static void setRunLoop(std::function<void()> fn)
    {
        ofelia_lock();
        ofelia_set_run_loop(fn);
        ofelia_unlock();
    }

    static void callAsync(std::function<void()> fn)
    {
        ofelia_call_async(fn);
    }
};

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
        ofelia_lock();
    }

    ~ofxOfeliaLock()
    {
        ofelia_unlock();
    }
};
