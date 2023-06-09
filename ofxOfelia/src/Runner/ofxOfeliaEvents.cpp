#include "ofxOfeliaEvents.h"
#include <cstring>
#include <algorithm>

ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::setupVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::updateVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::drawVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::exitVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::keyPressedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::keyReleasedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseMovedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseDraggedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mousePressedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseReleasedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseScrolledVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseEnteredVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::mouseExitedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::windowResizedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::messageReceivedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::draggedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::touchCancelledVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::touchDoubleTapVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::touchDownVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::touchMovedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::touchUpVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::lostFocusVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::gotFocusVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::gotMemoryWarningVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::deviceOrientationChangedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::launchedWithURLVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::swipeVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::pauseVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::stopVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::resumeVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::reloadTexturesVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::backPressedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::okPressedVec;
ofxOfeliaEvents::DataPairVec ofxOfeliaEvents::cancelPressedVec;

ofxOfeliaEvents::DataPairVec *ofxOfeliaEvents::getTargetDataPairVec(const char *s)
{
    if (!std::strcmp(s, "setup"))
        return &setupVec;
    else if (!std::strcmp(s, "update"))
        return &updateVec;
    else if (!std::strcmp(s, "draw"))
        return &drawVec;
    else if (!std::strcmp(s, "exit"))
        return &exitVec;
    else if (!std::strcmp(s, "keyPressed"))
        return &keyPressedVec;
    else if (!std::strcmp(s, "keyReleased"))
        return &keyReleasedVec;
    else if (!std::strcmp(s, "mouseMoved"))
        return &mouseMovedVec;
    else if (!std::strcmp(s, "mouseDragged"))
        return &mouseDraggedVec;
    else if (!std::strcmp(s, "mousePressed"))
        return &mousePressedVec;
    else if (!std::strcmp(s, "mouseReleased"))
        return &mouseReleasedVec;
    else if (!std::strcmp(s, "mouseScrolled"))
        return &mouseScrolledVec;
    else if (!std::strcmp(s, "mouseEntered"))
        return &mouseEnteredVec;
    else if (!std::strcmp(s, "mouseExited"))
        return &mouseExitedVec;
    else if (!std::strcmp(s, "windowResized"))
        return &windowResizedVec;
    else if (!std::strcmp(s, "messageReceived"))
        return &messageReceivedVec;
    else if (!std::strcmp(s, "dragged"))
        return &draggedVec;
    else if (!std::strcmp(s, "touchCancelled"))
        return &touchCancelledVec;
    else if (!std::strcmp(s, "touchDoubleTap"))
        return &touchDoubleTapVec;
    else if (!std::strcmp(s, "touchDown"))
        return &touchDownVec;
    else if (!std::strcmp(s, "touchMoved"))
        return &touchMovedVec;
    else if (!std::strcmp(s, "touchUp"))
        return &touchUpVec;
    else if (!std::strcmp(s, "lostFocus"))
        return &lostFocusVec;
    else if (!std::strcmp(s, "gotFocus"))
        return &gotFocusVec;
    else if (!std::strcmp(s, "gotMemoryWarning"))
        return &gotMemoryWarningVec;
    else if (!std::strcmp(s, "deviceOrientationChanged"))
        return &deviceOrientationChangedVec;
    else if (!std::strcmp(s, "launchedWithURL"))
        return &launchedWithURLVec;
    else if (!std::strcmp(s, "swipe"))
        return &swipeVec;
    else if (!std::strcmp(s, "pause"))
        return &pauseVec;
    else if (!std::strcmp(s, "stop"))
        return &stopVec;
    else if (!std::strcmp(s, "resume"))
        return &resumeVec;
    else if (!std::strcmp(s, "reloadTextures"))
        return &reloadTexturesVec;
    else if (!std::strcmp(s, "backPressed"))
        return &backPressedVec;
    else if (!std::strcmp(s, "okPressed"))
        return &okPressedVec;
    else if (!std::strcmp(s, "cancelPressed"))
        return &cancelPressedVec;
    else
        error("ofelia: unknown listener method '%s'", s);
    return nullptr;
}

ofxOfeliaEvents::DataPairVec::iterator ofxOfeliaEvents::findDataPair(DataPairVec &vec, ofxOfeliaLua *dataPtr)
{
    return std::find_if(vec.begin(), vec.end(),
                        [&](const DataPair &ref) {return ref.first == dataPtr;});
}

void ofxOfeliaEvents::addDataPair(DataPairVec &vec, ofxOfeliaLua *x, t_floatarg f)
{
    auto it = findDataPair(vec, x);
    if (it != vec.end())
    {
        it->second = f;
        return;
    }
    vec.push_back(std::make_pair(x, f));
}

void ofxOfeliaEvents::removeDataPair(DataPairVec &vec, ofxOfeliaLua *x)
{
    auto it = findDataPair(vec, x);
    if (it != vec.end())
        vec.erase(it);
}

void ofxOfeliaEvents::sortDataPairVec(DataPairVec &vec)
{
    std::stable_sort(vec.begin(), vec.end(),
                     [](const DataPair &a, const DataPair &b) {return a.second < b.second;});
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s)
{
    x->doFunction(s);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofKeyEventArgs &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(e.type));
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, static_cast<lua_Integer>(e.key));
    lua_setfield(L, -2, "key");
    lua_pushinteger(L, static_cast<lua_Integer>(e.keycode));
    lua_setfield(L, -2, "keycode");
    lua_pushinteger(L, static_cast<lua_Integer>(e.scancode));
    lua_setfield(L, -2, "scancode");
    lua_pushinteger(L, static_cast<lua_Integer>(e.codepoint));
    lua_setfield(L, -2, "codepoint");
    lua_pushboolean(L, static_cast<int>(e.isRepeat));
    lua_setfield(L, -2, "isRepeat");
    lua_pushinteger(L, static_cast<lua_Integer>(e.modifiers));
    lua_setfield(L, -2, "modifiers");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofMouseEventArgs &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(e.type));
    lua_setfield(L, -2, "type");
    lua_pushnumber(L, static_cast<lua_Number>(e.x));
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, static_cast<lua_Number>(e.y));
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, static_cast<lua_Integer>(e.button));
    lua_setfield(L, -2, "button");
    lua_pushnumber(L, static_cast<lua_Number>(e.scrollX));
    lua_setfield(L, -2, "scrollX");
    lua_pushnumber(L, static_cast<lua_Number>(e.scrollY));
    lua_setfield(L, -2, "scrollY");
    lua_pushinteger(L, static_cast<lua_Integer>(e.modifiers));
    lua_setfield(L, -2, "modifiers");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofResizeEventArgs &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(e.width));
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, static_cast<lua_Integer>(e.height));
    lua_setfield(L, -2, "height");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofMessage &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushstring(L, e.message.c_str());
    lua_setfield(L, -2, "message");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofDragInfo &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L); /* outermost table */
    lua_newtable(L); /* subtable "files" */
    for (size_t i = 0; i < e.files.size(); ++i)
    {
        lua_pushinteger(L, i + 1);
        lua_pushstring(L, e.files[i].c_str());
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "files");
    lua_newtable(L); /* subtable "position" */
    lua_pushnumber(L, e.position.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, e.position.y);
    lua_setfield(L, -2, "y");
    lua_setfield(L, -2, "position");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, ofTouchEventArgs &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(e.type));
    lua_setfield(L, -2, "type");
    lua_pushnumber(L, static_cast<lua_Number>(e.x));
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, static_cast<lua_Number>(e.y));
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, static_cast<lua_Integer>(e.id));
    lua_setfield(L, -2, "id");
    lua_pushinteger(L, static_cast<lua_Integer>(e.time));
    lua_setfield(L, -2, "time");
    lua_pushinteger(L, static_cast<lua_Integer>(e.numTouches));
    lua_setfield(L, -2, "numTouches");
    lua_pushnumber(L, static_cast<lua_Number>(e.width));
    lua_setfield(L, -2, "width");
    lua_pushnumber(L, static_cast<lua_Number>(e.height));
    lua_setfield(L, -2, "height");
    lua_pushnumber(L, static_cast<lua_Number>(e.angle));
    lua_setfield(L, -2, "angle");
    lua_pushnumber(L, static_cast<lua_Number>(e.minoraxis));
    lua_setfield(L, -2, "minoraxis");
    lua_pushnumber(L, static_cast<lua_Number>(e.majoraxis));
    lua_setfield(L, -2, "majoraxis");
    lua_pushnumber(L, static_cast<lua_Number>(e.pressure));
    lua_setfield(L, -2, "pressure");
    lua_pushnumber(L, static_cast<lua_Number>(e.xspeed));
    lua_setfield(L, -2, "xspeed");
    lua_pushnumber(L, static_cast<lua_Number>(e.yspeed));
    lua_setfield(L, -2, "yspeed");
    lua_pushnumber(L, static_cast<lua_Number>(e.xaccel));
    lua_setfield(L, -2, "xaccel");
    lua_pushnumber(L, static_cast<lua_Number>(e.yaccel));
    lua_setfield(L, -2, "yaccel");
    x->callFunction(top);
    lua_pop(L, 1);
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, const int e)
{
    x->doFunction(s, static_cast<t_floatarg>(e));
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, const std::string &e)
{
    x->doFunction(s, e.c_str());
}

void ofxOfeliaEvents::callEventListener(ofxOfeliaLua *x, const char *s, const std::pair<int, int> &e)
{
    int top; if (!x->isFunction(s, top)) return;
    lua_State *L = x->L;
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(e.first));
    lua_setfield(L, -2, "dir");
    lua_pushinteger(L, static_cast<lua_Integer>(e.second));
    lua_setfield(L, -2, "id");
    x->callFunction(top);
    lua_pop(L, 1);
}
