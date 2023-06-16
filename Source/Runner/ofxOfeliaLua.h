#pragma once

#include "ofxPdInterface.h"
#include "../Shared/ofxOfeliaStream.h"
#include "lua.hpp"
#include <string>
#include <map>
#include <memory>
#include <cstring>

using t_gpointer = int;
using t_floatarg = float;


class ofxOfeliaMessageManager;
class ofxOfeliaData;
class ofxOfeliaLua
{
public:
    ofxOfeliaLua(ofxOfeliaMessageManager* messageMan, const std::string& uid)
    : isChunkRun(false), uniqueId(uid), messageManager(messageMan), name(uid)
    {};
    
    static bool init();
    
    static void createInstance(ofxOfeliaMessageManager* messageMan, const std::string& uid);
    
    static ofxOfeliaLua* getByName(const std::string& name);
    static ofxOfeliaLua* getPtr(const std::string& uid);
    
    bool require();
    int  getType(const char *s);
    int  getType(const char *s, int &top);
    bool isNil(const char *s);
    bool isBoolean(const char *s);
    bool isNumber(const char *s);
    bool isString(const char *s);
    bool isUserData(const char *s);
    bool isTable(const char *s);
    bool isFunction(const char *s, int &top);
    void pushUserData(t_gpointer *p);
    void deleteUserData(t_gpointer *p);
    void setVariable(const char *s);
    void setVariable(const char *s, bool b);
    void setVariable(const char *s, t_floatarg f);
    void setVariable(const char *s, const char *s2);
    void setVariable(const char *s, t_gpointer *p);
    void setVariable(const char *s, std::vector<t_atom> argv);
    void doVariable(const char *s);
    void doVariable(const char *s, bool b);
    void doVariable(const char *s, t_floatarg f);
    void doVariable(const char *s, const char *s2);
    void doVariable(const char *s, t_gpointer *p);
    void doVariable(const char *s, std::vector<t_atom> argv);
    void outletUserData();
    void outletTable();
    void outletRet(int nret);
    void callFunction(int top);
    void setFunction(int top);
    void setFunction(t_floatarg f, int top);
    void setFunction(const char *s, int top);
    void setFunction(t_gpointer *p, int top);
    void setFunction(std::vector<t_atom> argv, int top);
    void doFunction(const char *s);
    void doFunction(const char *s, t_floatarg f);
    void doFunction(const char *s, const char *s2);
    void doFunction(const char *s, t_gpointer *p);
    void doFunction(const char *s, std::vector<t_atom> argv);
    void doNewFunction();
    void doFreeFunction();
    void doString(std::string s);
    void getVariableByArgs(const char *s, std::vector<t_atom> argv);
    void setVariableByArgs(const char *s, std::vector<t_atom> argv);
    
    const std::string& getUniqueId();
    const std::string& getName();
    void setName(std::string& newName);
    
    
    static lua_State *L;
    bool isChunkRun; /* whether the chunk is run or not */
    ofxOfeliaMessageManager* messageManager;
    
private:
    
    static std::map<std::string, std::unique_ptr<ofxOfeliaLua>> ofxLuaInstances;
    
    static void unpackModule(lua_State *L, const std::string &moduleName,  const std::string &prefix);
    static bool addGlobals(lua_State *L);
    
    std::string uniqueId;
    std::string name;
};

void error(const char *fmt, ...);

