#include "ofxOfeliaSetup.h"
#include "ofxOfeliaLua.h"
#include "ofxOfeliaDefine.h"
#include "ofxOfeliaFunction.h"
#include "ofxOfeliaGet.h"
#include "ofxOfeliaSet.h"
#include "ofxOfeliaExists.h"
#include "ofxOfeliaCreator.h"
#include "ofxOfeliaTextBuf.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

#ifdef __linux__
#include <dlfcn.h>
#endif

int ofeliaVersionMajor = OFELIA_MAJOR_VERSION;
int ofeliaVersionMinor = OFELIA_MINOR_VERSION;
int ofeliaVersionBugFix = OFELIA_BUGFIX_VERSION;


// On Linux, we need to set the library where we search for gstreamer plugins
// For now we'll do without this, and let the user install gstreamer plugins themselves
/*
void setup_gstreamer_env()
{
#ifdef __linux__
    
    char buf[PATH_MAX];
    
    Dl_info info;
    if (dladdr(static_cast<void*>(&ofeliaVersionMajor), &info))
    {
        realpath(info.dli_fname, buf);
        
        auto ofeliaPath = std::string(buf);
        auto gstPath = ofFilePath::join(ofeliaPath, "./libs/gstreamer-1.0");
        auto envVar = "GST_PLUGIN_PATH_1_0=" + gstPath + ";.";

        putenv(const_cast<char*>(envVar.c_str()));
    }
    

#endif
} */


void ofelia_setup()
{
    /* check for pd version compatibility */
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    if (major == PD_MAJOR_VERSION_REQUIRED && minor < PD_MINOR_VERSION_REQUIRED)
    {
        pd_error(NULL, "ofelia requires Pd-%d.%d-0 or higher", PD_MAJOR_VERSION_REQUIRED, PD_MINOR_VERSION_REQUIRED);
        return;
    }

    ofxOfeliaDefine::setup();
    ofxOfeliaFunction::setup();
    ofxOfeliaGet::setup();
    ofxOfeliaSet::setup();
    ofxOfeliaExists::setup();
    ofxOfeliaCreator::setup();
    ofxOfeliaTextBuf::loadScript();
    //ofxOfeliaLog::setLoggerChannel();
    
    const std::string &fileName = "ofelia/CHANGES.txt";
    std::string dirResult, fileNameResult;
    if (ofxOfeliaTextBuf::canvasOpen(canvas_getcurrent(), fileName, dirResult, fileNameResult))
    {
        const std::string &fullPath = dirResult + "/" + fileNameResult;
        std::ifstream input(fullPath);
        std::string firstLine;
        std::getline(input, firstLine);
        std::sscanf(firstLine.c_str(), "v%d.%d.%d", &ofeliaVersionMajor, &ofeliaVersionMinor, &ofeliaVersionBugFix);
        input.close();
    }
    /* print version info if loaded successfully */
    post("ofelia v%d.%d.%d", ofeliaVersionMajor, ofeliaVersionMinor, ofeliaVersionBugFix);
}
