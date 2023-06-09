#include "ofxOfeliaLog.h"
#include "ofxOfeliaMessageManager.h"
#include "ofUtils.h"

std::shared_ptr<ofxOfeliaLog> ofxOfeliaLog::loggerChannel;

void ofxOfeliaLog::setLoggerChannel()
{
    loggerChannel = std::make_shared<ofxOfeliaLog>();
    ofSetLoggerChannel(loggerChannel);
}

void ofxOfeliaLog::log(ofLogLevel level, const std::string &module, const std::string &message)
{
    ofxOfeliaMessageManager::instance->sendMessage(pd_log, false, std::string(module + ": " + message));
}

void ofxOfeliaLog::log(ofLogLevel level, const std::string &module, const char* format, ...)
{
    char buf[1024];
    std::va_list args;
    va_start(args, format);
    std::vsnprintf(buf, 1023, format, args);
    va_end(args);
    

    ofxOfeliaMessageManager::instance->sendMessage(pd_log, false, std::string(module + ": " + std::string(buf)));
}

void ofxOfeliaLog::log(ofLogLevel level, const std::string &module, const char* format, std::va_list args)
{
    char buf[1024];
    std::snprintf(buf, 1023, "%s", ofVAArgsToString(format, args).c_str());
    
    ofxOfeliaMessageManager::instance->sendMessage(pd_log, false, std::string(module + ": " + std::string(buf)));
}
