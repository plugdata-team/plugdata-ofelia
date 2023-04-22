#pragma once

/* ofelia compiled version info */
#define OFELIA_MAJOR_VERSION 4
#define OFELIA_MINOR_VERSION 1
#define OFELIA_BUGFIX_VERSION 0

/* minimum required pd version info */
#define PD_MAJOR_VERSION_REQUIRED 0
#define PD_MINOR_VERSION_REQUIRED 49

extern int ofeliaVersionMajor;
extern int ofeliaVersionMinor;
extern int ofeliaVersionBugFix;

#include "m_pd.h"

extern "C"
{
#ifdef _MSC_VER
    __declspec(dllexport) void ofelia_setup();
#else
    void ofelia_setup();
#endif
}
