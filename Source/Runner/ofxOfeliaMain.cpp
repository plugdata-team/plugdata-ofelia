#pragma once

#include "ofMain.h"
#include "ofxOfeliaMessageManager.h"
#include "ofxOfeliaLua.h"
#include "GLFW/glfw3.h"

struct ofApp : public ofBaseApp, public ofxOfeliaMessageManager
{
    bool hideWindow = true;
    
    void update() override {
        if(hideWindow) {
            glfwHideWindow(window->getGLFWWindow());
        }
    }
    
    void draw() override {
        run();
    }
    
    void receiveMessage(ofxMessageType type, const std::string& message) override
    {
        switch(type)
        {
            case ofx_lua_init:
            {
                auto [id] = parseMessage<std::string>(message);
                ofxOfeliaLua::createInstance(this, id);
                break;
            }
            case ofx_lua_init_sym:
            {
                auto [id, name] = parseMessage<std::string, std::string>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->setName(name);
                break;
            }
            case ofx_lua_do_function_s:
            {
                auto [id, symbol] = parseMessage<std::string, std::string>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFunction(symbol.c_str());
                break;
            }
            case ofx_lua_do_function_sp:
            {
                auto [id, symbol, pointer] = parseMessage<std::string, std::string, t_gpointer>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFunction(symbol.c_str(), pointer);
                break;
            }
            case ofx_lua_do_function_ss:
            {
                auto [id, symbol1, symbol2] = parseMessage<std::string, std::string, std::string>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFunction(symbol1.c_str(), symbol2.c_str());
                break;
            }
            case ofx_lua_do_function_sf:
            {
                auto [id, symbol, number] = parseMessage<std::string, std::string, float>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFunction(symbol.c_str(), number);
                break;
            }
            case ofx_lua_do_function_sa:
            {
                auto [id, symbol, atoms] = parseMessage<std::string, std::string, std::vector<t_atom>>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFunction(symbol.c_str(), atoms);
                break;
            }
            case ofx_lua_do_free_function:
            {
                auto [id] = parseMessage<std::string>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doFreeFunction();
                break;
            }
            case ofx_lua_get_var_by_args:
            {
                auto [id, symbol, atoms] = parseMessage<std::string, std::string, std::vector<t_atom>>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->getVariableByArgs(symbol.c_str(), atoms);
                break;
            }
            case ofx_lua_set_var_by_args:
            {
                auto [id, symbol, atoms] = parseMessage<std::string, std::string, std::vector<t_atom>>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->setVariableByArgs(symbol.c_str(), atoms);
                break;
            }
            case ofx_lua_do_string:
            {
                auto [id, str] = parseMessage<std::string, std::string>(message);
                if(auto* ofxLua = ofxOfeliaLua::getPtr(id)) ofxLua->doString(str);
                break;
            }
                
            default: break;
        }
    }
    
    ofAppGLFWWindow* window = nullptr;
};


ofApp* mainApp;
ofMainLoop* mainLoop = nullptr;


ofApp* app;


void createWindow()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(2, 1);
    settings.windowMode = OF_WINDOW;
    settings.setSize(1, 1);
    settings.setPosition({0, 0});
    app->hideWindow = true;
    
    auto window = ofCreateWindow(settings);
    app->window = dynamic_cast<ofAppGLFWWindow*>(window.get());
    
    auto* mainLoop = ofGetMainLoop().get();
    mainLoop->setEscapeQuitsLoop(false);
    ofResetElapsedTimeCounter();
    ofSetFrameRate(60);
    ofDisableArbTex();
}

void showWindow(glm::vec2 position, int width, int height)
{
    ofSetWindowShape(width, height);
    ofSetWindowPosition(position.x, position.y);
    app->hideWindow = false;
    glfwShowWindow(app->window->getGLFWWindow());
}


 int main()
 {
     ofxOfeliaLua::init();
     
     app = new ofApp();
     createWindow();
     
     while(true)
     {
         ofRunApp(app);
         app = new ofApp();
         createWindow();
     }
 }