#include "ofxOfeliaAsync.h"

extern void ofelia_set_run_loop(std::function<void()>);
extern void ofelia_call_async(std::function<void()>);
extern void ofelia_audio_lock();
extern void ofelia_audio_unlock();

ofxOfeliaAudioLock::ofxOfeliaAudioLock()
{
    ofelia_audio_lock();
}

ofxOfeliaAudioLock::~ofxOfeliaAudioLock()
{
    ofelia_audio_unlock();
}


ofxOfeliaLock::ofxOfeliaLock()
{
    ofeliaLock.lock();
}

ofxOfeliaLock::~ofxOfeliaLock()
{
    ofeliaLock.unlock();
}

std::recursive_mutex ofxOfeliaLock::ofeliaLock;

void ofxOfeliaAsync::setRunLoop(std::function<void()> fn)
{
    const ofxOfeliaLock ofxLock;
    ofelia_set_run_loop(fn);
}

void ofxOfeliaAsync::callAsync(std::function<void()> fn)
{
    ofelia_call_async([fn](){
        const ofxOfeliaLock ofxLock;
        fn();
    });
}

