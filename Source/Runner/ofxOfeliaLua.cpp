#include "ofxOfeliaLua.h"
#include "ofxOfeliaGL.h"
#include "ofxOfeliaMessageManager.h"
#include <algorithm>
#include <cctype>
#include <deque>
#include <cstring>
#include <sstream>

/*
 calling ofxOfeliaLua::doString(const char *s) function
 is like running the following script in Lua:
 
 * [ofelia define] object:
 
 local name = "foo" -- module name set as ofelia object argument
 package.preload[name] = nil
 package.loaded[name] = nil
 package.preload[name] = function(this)
 local ofelia = {}
 local M = ofelia
 s -- a written script in pd
 return M
 end
 
 * [ofelia function] object:
 
 local name = "foo" -- module name set as ofelia object argument
 package.preload[name] = nil
 package.loaded[name] = nil
 package.preload[name] = function(this)
 local ofelia = {}
 local M = ofelia
 function M.bang()
 return M.anything(nil)
 end
 function M.float(f)
 return M.anything(f)
 end
 function M.symbol(s)
 return M.anything(s)
 end
 function M.pointer(p)
 return M.anything(p)
 end
 function M.list(l)
 return M.anything(l)
 end
 function M.anything(a)
 s -- a written script in pd
 end
 return M
 end
 
 sending 'bang' message to [ofelia define] or [ofelia function] object
 calls ofxOfeliaLua::doFunction(t_symbol *s) function and it is like running
 the following script in Lua:
 
 local m = require("foo") -- module name set as ofelia object argument
 if type(m.bang) == "function" then
 return m:bang() -- ofelia object will output the returned value through its outlet
 end
 */

lua_State *ofxOfeliaLua::L;

extern "C"
{
    int luaopen_of(lua_State *L);
    int luaopen_pd(lua_State *L);
    int luaopen_print(lua_State *L);
}

/* redefined print function for the pd window */
int l_my_print(lua_State *L)
{
    /* TODO: implement this!
    int argv.size() = lua_gettop(L);
    if (argv.size()) startpost(luaL_tolstring(L, 1, nullptr));
    for (int i = 2; i <= argv.size(); ++i)
    {
        poststring(luaL_tolstring(L, i, nullptr));
        lua_pop(L, 1); // remove the string
    }
    endpost(); */
    return 0;
}

const struct luaL_Reg printlib[] =
{
    {"print", l_my_print},
    {NULL, NULL} /* end of array */
};

int luaopen_print(lua_State *L)
{
    lua_getglobal(L, "_G");
#if LUA_VERSION_NUM < 502
    luaL_register(L, NULL, printlib);
#else
    luaL_setfuncs(L, printlib, 0);
#endif
    lua_pop(L, 1);
    return 1;
}

void ofxOfeliaLua::unpackModule(lua_State *L, const std::string &moduleName, const std::string &prefix)
{
    std::string upperPrefix = prefix;
    std::transform(upperPrefix.begin(), upperPrefix.end(),upperPrefix.begin(), ::toupper);
    upperPrefix += "_"; /* prefix for constants and enums */
    lua_getglobal(L, moduleName.c_str());
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        const std::string &type = luaL_typename(L, -1);
        const std::string &str = lua_tostring(L, -2);
        std::string renamedStr;
        lua_getfield(L, -3, str.c_str());
        if ((type == "table" || type == "userdata") && (::isupper(str[0]) || ::isdigit(str[0]))) /* classes and structs */
            renamedStr = prefix + str;
        else if (type == "function") /* global functions */
        {
            renamedStr = str;
            renamedStr[0] = static_cast<char>(::toupper(str[0]));
            renamedStr = prefix + renamedStr;
        }
        else if (type == "number" || type == "string" || type == "boolean")
        {
            if (std::any_of(str.begin(), str.end(), ::islower)) /* static member variables */
                renamedStr = prefix + str;
            else /* constants or enums */
                renamedStr = upperPrefix + str;
        }
        lua_setglobal(L, renamedStr.c_str());
        lua_pop(L, 1);
        lua_pushnil(L); /* assign nil to original element */
        lua_setfield(L, -3, str.c_str());
    }
    lua_pop(L, 1);
    lua_pushnil(L); /* assign nil to module */
    lua_setglobal(L, moduleName.c_str());
}

bool ofxOfeliaLua::addGlobals(lua_State *L)
{
    lua_settop(L, 0);
    const char *s =
    "ofRequire = require\n"
    "function ofTable(...)\n"
    "  return {...}\n"
    "end\n";
    const int ret = luaL_dostring(L, s);
    if (ret != LUA_OK)
    {
        error("ofelia: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::init()
{
    /* initialize lua */
    L = luaL_newstate();
    if (L == nullptr)
    {
        error("ofelia: failed initializing lua");
        return false;
    }
    /* open libs */
    luaL_openlibs(L);
    luaopen_of(L);
    luaopen_pd(L);
    luaopen_print(L);
    
    /* clear stack since opening libs leaves tables on the stack */
    lua_settop(L, 0);
    
    /* unpack module elements into global namespace with the new prefix */
    unpackModule(L, "of", "of");
    unpackModule(L, "pd", "of");
    
    /* add GL preprocessor defines */
    ofxOfeliaGL::addDefines(L);
    
    /* add global variables needed */
    if (!addGlobals(L)) return false;
    
    /* make garbage collector run more frequently (default: 200) */
    lua_gc(L, LUA_GCSETPAUSE, 100);
    return true;
}

void ofxOfeliaLua::createInstance(ofxOfeliaMessageManager* messageMan, const std::string& uid)
{
    ofxLuaInstances[uid].reset(new ofxOfeliaLua(messageMan, uid));
}

ofxOfeliaLua* ofxOfeliaLua::getByName(const std::string& name)
{
    for(auto& [id, instance] : ofxLuaInstances)
    {
        if(instance->getName() == name)
        {
            return instance.get();
        }
    }
    
    return nullptr;
}

ofxOfeliaLua* ofxOfeliaLua::getPtr(const std::string& uid)
{
    if(ofxLuaInstances.count(uid))
    {
        return ofxLuaInstances[uid].get();
    }
    else {
        std::cerr << "ofxOfeliaLua: instance not found" << std::endl;
        return nullptr;
    }
    
    return nullptr;
}
    
bool ofxOfeliaLua::require()
{
    if (!isChunkRun) return false;
    lua_getglobal(L, "require");
    
    lua_pushstring(L, getName().c_str());
    
    if (lua_pcall(L, 1, LUA_MULTRET, 0))
    {
        error("ofelia: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        isChunkRun = false;
        return false;
    }
    return true;
}

int ofxOfeliaLua::getType(const char *s)
{
    lua_getfield(L, -1, s);
    return lua_type(L, -1);
}

int ofxOfeliaLua::getType(const char *s, int &top)
{
    top = lua_gettop(L);
    lua_getfield(L, -1, s);
    return lua_type(L, -1);
}

bool ofxOfeliaLua::isNil(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TNIL)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isBoolean(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TBOOLEAN)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isNumber(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TNUMBER)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isString(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TSTRING)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isUserData(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TUSERDATA)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isTable(const char *s)
{
    if (!require())
        return false;
    if (getType(s) != LUA_TTABLE)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool ofxOfeliaLua::isFunction(const char *s, int &top)
{
    if (!require())
        return false;
    if (getType(s, top) != LUA_TFUNCTION)
    {
        lua_pop(L, 2);
        return false;
    }
    return true;
}

void ofxOfeliaLua::pushUserData(t_gpointer *p)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, static_cast<lua_Integer>(*reinterpret_cast<int *>(p)));
}

void ofxOfeliaLua::setVariable(const char *s)
{
    lua_pushnil(L);
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::setVariable(const char *s, bool b)
{
    lua_pushboolean(L, static_cast<int>(b));
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::setVariable(const char *s, t_floatarg f)
{
    lua_pushnumber(L, static_cast<lua_Number>(f));
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::setVariable(const char* s, const char *s2)
{
    lua_pushstring(L, s2);
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::setVariable(const char *s, t_gpointer *p)
{
    pushUserData(p);
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::setVariable(const char *s, std::vector<t_atom> argv)
{
    lua_newtable(L);
    for (int i = 0; i < argv.size(); ++i)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
        if (argv[i].a_type == A_FLOAT)
            lua_pushnumber(L, static_cast<lua_Number>(argv[i].a_w.w_float));
        else if (argv[i].a_type == A_SYMBOL)
            lua_pushstring(L, argv[i].a_w.w_symbol.c_str());
        else if (argv[i].a_type == A_POINTER)
            pushUserData(argv[i].a_w.w_gpointer);
        lua_settable(L, -3);
    }
    lua_setfield(L, -3, s);
    lua_pop(L, 2);
}

void ofxOfeliaLua::doVariable(const char *s)
{
    if (!isNil(s)) return;
    setVariable(s);
}

void ofxOfeliaLua::doVariable(const char *s, bool b)
{
    if (!isBoolean(s)) return;
    setVariable(s, b);
}

void ofxOfeliaLua::doVariable(const char *s, t_floatarg f)
{
    if (!isNumber(s)) return;
    setVariable(s, f);
}

void ofxOfeliaLua::doVariable(const char *s, const char *s2)
{
    if (!isString(s)) return;
    setVariable(s, s2);
}

void ofxOfeliaLua::doVariable(const char *s, t_gpointer *p)
{
    if (!isString(s)) return;
    setVariable(s, p);
}

void ofxOfeliaLua::doVariable(const char *s, std::vector<t_atom> argv)
{
    if (!isTable(s)) return;
    setVariable(s, argv);
}

void ofxOfeliaLua::outletUserData()
{
    int userDataRef = luaL_ref(L, LUA_REGISTRYINDEX);
    messageManager->sendMessage(pd_outlet_pointer, uniqueId, 0, userDataRef);
    luaL_unref(L, LUA_REGISTRYINDEX, userDataRef);
}

void ofxOfeliaLua::outletTable()
{
    lua_pushvalue(L, -1);
    lua_pushnil(L);
    int ac = 0;
    auto av = std::vector<t_atom>();
    std::deque<int> userDataRef;
    while (lua_next(L, -2))
    {
        if (lua_type(L, -2) == LUA_TSTRING) // if the table has a string key
        {
            lua_pop(L, 2);
            outletUserData(); // treat the table as a userdata
            return;
        }
        av.resize(ac + 1);

        if (lua_isboolean(L, -1))
        {
            av[ac].a_type = A_FLOAT;
            av[ac].a_w.w_float = static_cast<float>(lua_toboolean(L, -1));
            lua_pop(L, 1);
        }
        else if (lua_isnumber(L, -1))
        {
            av[ac].a_type = A_FLOAT;
            av[ac].a_w.w_float = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        }
        else if (lua_isstring(L, -1))
        {
            av[ac].a_type = A_SYMBOL;
            av[ac].a_w.w_symbol = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
        else if (lua_isuserdata(L, -1) || lua_istable(L, -1))
        {
            av[ac].a_type = A_POINTER;
            userDataRef.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
            av[ac].a_w.w_gpointer = reinterpret_cast<t_gpointer *>(&userDataRef.back());
        }
        ac++;
    }
    lua_pop(L, 1);
    
    messageManager->sendMessage(pd_outlet_list, uniqueId, 0, av, true);
    
    for (const int i : userDataRef)
        luaL_unref(L, LUA_REGISTRYINDEX, i);
}

void ofxOfeliaLua::outletRet(int nret)
{

    if (!nret) return;

    if (lua_isnil(L, -1))
        messageManager->sendMessage(pd_outlet_bang, uniqueId, 0);
    else if (lua_isboolean(L, -1))
        messageManager->sendMessage(pd_outlet_float, uniqueId, 0, static_cast<float>(lua_toboolean(L, -1)));
    else if (lua_isnumber(L, -1))
        messageManager->sendMessage(pd_outlet_float, uniqueId, 0, static_cast<float>(lua_tonumber(L, -1)));
    else if (lua_isstring(L, -1))
        messageManager->sendMessage(pd_outlet_symbol, uniqueId, 0, std::string(lua_tostring(L, -1)));
    else if (lua_isuserdata(L, -1))
    {
        outletUserData();
        return;
    }
    else if (lua_istable(L, -1))
        outletTable();
    lua_pop(L, nret);
}

void ofxOfeliaLua::callFunction(int top)
{
    /* note: it currently passes only one argument */
    if (lua_pcall(L, 1, LUA_MULTRET, 0))
    {
        error("ofelia: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }
    /* outlet return value if any exists */
    outletRet(lua_gettop(L) - top);
}

void ofxOfeliaLua::setFunction(int top)
{
    lua_pushnil(L);
    callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaLua::setFunction(t_floatarg f, int top)
{
    lua_pushnumber(L, static_cast<lua_Number>(f));
    callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaLua::setFunction(const char *s, int top)
{
    lua_pushstring(L, s);
    callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaLua::setFunction(t_gpointer *p, int top)
{
    pushUserData(p);
    callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaLua::setFunction(std::vector<t_atom> argv, int top)
{
    lua_newtable(L);
    for (int i = 0; i < argv.size(); ++i)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
        if (argv[i].a_type == A_FLOAT)
            lua_pushnumber(L, static_cast<lua_Number>(argv[i].a_w.w_float));
        else if (argv[i].a_type == A_SYMBOL)
            lua_pushstring(L, argv[i].a_w.w_symbol.c_str());
        else if (argv[i].a_type == A_POINTER)
            pushUserData(argv[i].a_w.w_gpointer);
        lua_settable(L, -3);
    }
    callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaLua::doFunction(const char *s)
{
    int top; if (!isFunction(s, top)) return;
    setFunction(top);
}

void ofxOfeliaLua::doFunction(const char *s, t_floatarg f)
{
    int top; if (!isFunction(s, top)) return;
    setFunction(static_cast<lua_Number>(f), top);
}

void ofxOfeliaLua::doFunction(const char *s, const char *s2)
{
    int top; if (!isFunction(s, top)) return;
    setFunction(s2, top);
}

void ofxOfeliaLua::doFunction(const char *s, t_gpointer *p)
{
    int top; if (!isFunction(s, top)) return;
    setFunction(p, top);
}

void ofxOfeliaLua::doFunction(const char *s, std::vector<t_atom> argv)
{
    int top; if (!isFunction(s, top)) return;
    setFunction(argv, top);
}

void ofxOfeliaLua::doNewFunction()
{
    isChunkRun = true;
    doFunction("new"); /* isChunkRun will set to false if fail */
}

void ofxOfeliaLua::doFreeFunction()
{
    doFunction("free");
    isChunkRun = false;
}

void ofxOfeliaLua::doString(std::string s)
{
    lua_settop(L, 0); // empty the stack
    // run the lua chunk
    const int ret = luaL_dostring(L, s.c_str());
    if (ret != LUA_OK)
    {
        std::cout << s << std::endl;
        error("ofelia: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }
    
    // call the new function
    doNewFunction();
}


void ofxOfeliaLua::getVariableByArgs(const char *s, std::vector<t_atom> argv)
{
    if (!require()) return;
    int top; switch (getType(s, top))
    {
        case LUA_TNIL:
            messageManager->sendMessage(pd_outlet_bang, uniqueId, 0);
            break;
        case LUA_TBOOLEAN:
            messageManager->sendMessage(pd_outlet_float, uniqueId, 0, static_cast<float>(lua_toboolean(L, -1)));
            break;
        case LUA_TNUMBER:
            messageManager->sendMessage(pd_outlet_float, uniqueId, 0, static_cast<float>(lua_tonumber(L, -1)));
            break;
        case LUA_TSTRING:
            messageManager->sendMessage(pd_outlet_symbol, uniqueId, 0, std::string(lua_tostring(L, -1)));
            break;
        case LUA_TUSERDATA:
            outletUserData();
            break;
        case LUA_TTABLE:
            outletTable();
            break;
        case LUA_TFUNCTION:
            if (!argv.size())
                setFunction(top);
            else if (argv.size() == 1)
            {
                if (argv[0].a_type == A_FLOAT)
                    setFunction(argv[0].a_w.w_float, top);
                else if (argv[0].a_type == A_SYMBOL)
                    setFunction(argv[0].a_w.w_symbol.c_str(), top);
                else if (argv[0].a_type == A_POINTER)
                    setFunction(argv[0].a_w.w_gpointer, top);
            }
            else
                setFunction(argv, top);
            break;
        default:
            break;
    }
}

void ofxOfeliaLua::setVariableByArgs(const char *s, std::vector<t_atom> argv)
{
    if (!require()) return;
    int top; switch (getType(s, top))
    {
        case LUA_TNIL:
            if (!argv.size())
                setVariable(s);
            else if (argv.size() == 1)
            {
                if (argv[0].a_type == A_FLOAT)
                    setVariable(s, argv[0].a_w.w_float);
                else if (argv[0].a_type == A_SYMBOL)
                    setVariable(s, argv[0].a_w.w_symbol.c_str());
                else if (argv[0].a_type == A_POINTER)
                    setVariable(s, argv[0].a_w.w_gpointer);
            }
            else
                setVariable(s, argv);
            break;
        case LUA_TBOOLEAN:
            if (!argv.size())
                setVariable(s);
            else if (argv[0].a_type == A_FLOAT)
                setVariable(s, argv[0].a_w.w_float != 0);
            else
                error("ofelia: failed to set the boolean variable '%s'", s);
            break;
        case LUA_TNUMBER:
            if (!argv.size())
                setVariable(s);
            else if (argv[0].a_type == A_FLOAT)
                setVariable(s, argv[0].a_w.w_float);
            else
                error("ofelia: failed to set the number variable '%s'", s);
            break;
        case LUA_TSTRING:
            if (!argv.size())
                setVariable(s);
            else if (argv[0].a_type == A_SYMBOL)
                setVariable(s, argv[0].a_w.w_symbol.c_str());
            else
                error("ofelia: failed to set the string variable '%s'", s);
            break;
        case LUA_TUSERDATA:
            if (!argv.size())
                setVariable(s);
            else if (argv[0].a_type == A_POINTER)
                setVariable(s, argv[0].a_w.w_gpointer);
            else
                error("ofelia: failed to set the pointer variable '%s'", s);
            break;
        case LUA_TTABLE:
            if (!argv.size())
                setVariable(s);
            else
                setVariable(s, argv);
            break;
        case LUA_TFUNCTION:
            if (!argv.size())
                setFunction(top);
            else if (argv.size() == 1)
            {
                if (argv[0].a_type == A_FLOAT)
                    setFunction(argv[0].a_w.w_float, top);
                else if (argv[0].a_type == A_SYMBOL)
                    setFunction(argv[0].a_w.w_symbol.c_str(), top);
                else if (argv[0].a_type == A_POINTER)
                    setFunction(argv[0].a_w.w_gpointer, top);
            }
            else
                setFunction(argv, top);
            break;
        default:
            break;
    }
}

const std::string& ofxOfeliaLua::getUniqueId()
{
    return uniqueId;
}

const std::string& ofxOfeliaLua::getName()
{
    return name;
}

void ofxOfeliaLua::setName(std::string& newName)
{
    name = newName;
}

std::map<std::string, std::unique_ptr<ofxOfeliaLua>> ofxOfeliaLua::ofxLuaInstances = std::map<std::string, std::unique_ptr<ofxOfeliaLua>>();

void error(const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, fmt);
    vsnprintf(buf, 1023, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");
    
    fprintf(stderr, "error: %s", buf);
}

