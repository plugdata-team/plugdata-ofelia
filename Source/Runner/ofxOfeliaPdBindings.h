#pragma once

#include "ofMain.h"
#include "lua.hpp"
#include "ofxOfeliaLua.h"
#include "ofxOfeliaLua.h"
#include "ofxOfeliaEvents.h"
#include "ofxOfeliaMessageManager.h"
#include <cstdio>
#include <utility>
#include <vector>
#include <deque>

#include "GLFW/glfw3.h"


void createWindow();
void showWindow(glm::vec2 position, int width, int height);

using t_canvas = std::string;
using t_garray = void;
using t_float = float;

#define MAXPDSTRING 12000

const char* gensym(const std::string& sym)
{
    return sym.c_str();
}

void freebytes(void *mem_to_free, size_t nbytes)
{
    free(mem_to_free);
}

void* getbytes(size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = (void *)calloc(nbytes, 1);
    return (ret);
}

void atom_string(const t_atom *a, char *buf, unsigned int bufsize)
{
    char tbuf[30];
    switch(a->a_type)
    {
    case A_SEMI: strcpy(buf, ";"); break;
    case A_COMMA: strcpy(buf, ","); break;
    case A_POINTER:
        strcpy(buf, "(pointer)");
        break;
    case A_FLOAT:
        sprintf(tbuf, "%g", a->a_w.w_float);
        if (strlen(tbuf) < bufsize-1) strcpy(buf, tbuf);
        else if (a->a_w.w_float < 0) strcpy(buf, "-");
        else  strcpy(buf, "+");
        break;
    case A_SYMBOL:
    case A_DOLLSYM:
    {
        const char *sp;
        unsigned int len;
        int quote;
        for (sp = a->a_w.w_symbol.c_str(), len = 0, quote = 0; *sp; sp++, len++)
            if (*sp == ';' || *sp == ',' || *sp == '\\' || *sp == ' ' ||
                (a->a_type == A_SYMBOL && *sp == '$' &&
                    sp[1] >= '0' && sp[1] <= '9'))
                        quote = 1;
        if (quote)
        {
            char *bp = buf, *ep = buf + (bufsize-2);
            sp = a->a_w.w_symbol.c_str();
            while (bp < ep && *sp)
            {
                if (*sp == ';' || *sp == ',' || *sp == '\\' || *sp == ' ' ||
                    (a->a_type == A_SYMBOL && *sp == '$' &&
                        sp[1] >= '0' && sp[1] <= '9'))
                            *bp++ = '\\';
                *bp++ = *sp++;
            }
            if (*sp) *bp++ = '*';
            *bp = 0;
            /* post("quote %s -> %s", a->a_w.w_symbol.c_str(), buf); */
        }
        else
        {
            if (len < bufsize-1) strcpy(buf, a->a_w.w_symbol.c_str());
            else
            {
                strncpy(buf, a->a_w.w_symbol.c_str(), bufsize - 2);
                strcpy(buf + (bufsize - 2), "*");
            }
        }
    }
        break;
    case A_DOLLAR:
        sprintf(buf, "$%d", a->a_w.w_index);
        break;
    default: break;
    }
}


std::string canvas_realizedollar(std::string name, std::string s)
{
    auto* mm = ofxOfeliaMessageManager::instance;
    
    if (s.find('$') == std::string::npos)
    {
        return s;
    }
    
    mm->sendMessage(canvas_realise_dollar, name, s);
    return std::get<0>(mm->waitForReturnValue<std::string>());
}

class pdWindow
{
    
public:
    pdWindow() : glVersionMajor(2)
    ,glVersionMinor(1)
    ,glesVersion(1)
    ,title("")
    ,windowMode(0)
    ,width(1024)
    ,height(768)
    ,sizeSet(false)
    ,position(0, 0)
    ,positionSet(false)
    ,owner(false)
    {
    };
    virtual ~pdWindow()
    {
        if (!owner || !exists) return;
        removeWindowListeners();
        windowPtr->setWindowShouldClose();
        mainLoop->loopOnce();
        mainLoop->pollEvents();
    };
    void setGLVersion(int major, int minor)
    {
        this->glVersionMajor = major;
        this->glVersionMinor = minor;
    }
    void setGLESVersion(int version)
    {
        this->glesVersion = version;
    }
    void getPosition(int **posp)
    {
        int *pos = static_cast<int *>(malloc(2 * sizeof(int)));
        pos[0] = static_cast<int>(position.x);
        pos[1] = static_cast<int>(position.y);
        *posp = pos;
    }
    void setPosition(int xpos, int ypos)
    {
        this->position.x = static_cast<float>(xpos);
        this->position.y = static_cast<float>(ypos);
        this->positionSet = true;
    }
    bool isPositionSet()
    {
        return positionSet;
    }
    int getWidth()
    {
        return width;
    }
    int getHeight()
    {
        return height;
    }
    void setSize(int width, int height)
    {
        this->width = width;
        this->height = height;
        this->sizeSet = true;
    }
    bool isSizeSet()
    {
        return sizeSet;
    }
    void create()
    {
        if (owner || exists) return;

        ofSetWindowTitle(title);
        
        mainLoop = ofGetMainLoop().get();
        mainLoop->setEscapeQuitsLoop(false);
        windowPtr = ofGetWindowPtr();
        ofResetElapsedTimeCounter();
        ofSetFrameRate(60);
        ofDisableArbTex();
        owner = true;
        exists = true;
        addWindowListeners();
        windowPtr->events().notifySetup();
        
        showWindow(position, width, height);
    }
    void destroy()
    {
        if (!owner || !exists) return;
#if defined(TARGET_EXTERNAL)
        ofEventArgs e;
        exit(e);
        removeWindowListeners();
        windowPtr->setWindowShouldClose();
        owner = false;
        exists = false;
#elif defined(TARGET_STANDALONE)
        ofExit();
#endif
    };
    static void addListener(std::string s, std::string s2, t_floatarg f)
    {
        ofxOfeliaLua *x = ofxOfeliaLua::getByName(s2);
        if (x == nullptr) return;
        ofxOfeliaEvents::DataPairVec *vecPtr = ofxOfeliaEvents::getTargetDataPairVec(s.c_str());
        if (vecPtr == nullptr) return;
        ofxOfeliaEvents::addDataPair(*vecPtr, x, f);
    }
    static void addListener(std::string s, std::string s2)
    {
        addListener(s, s2, 0);
    }
    static void removeListener(std::string s, std::string s2)
    {
        ofxOfeliaLua *x = ofxOfeliaLua::getByName(s2);
        if (x == nullptr) return;
        ofxOfeliaEvents::DataPairVec *vecPtr = ofxOfeliaEvents::getTargetDataPairVec(s.c_str());
        if (vecPtr == nullptr) return;
        ofxOfeliaEvents::removeDataPair(*vecPtr, x);
    }
    static std::vector<std::pair<ofxOfeliaLua *, float>> getListenerData(std::string s)
    {
        ofxOfeliaEvents::DataPairVec *vecPtr = ofxOfeliaEvents::getTargetDataPairVec(s.c_str());
        if (vecPtr == nullptr) return {};
        ofxOfeliaEvents::sortDataPairVec(*vecPtr);
        return *vecPtr;
    }
    int glVersionMajor;
    int glVersionMinor;
    int glesVersion;
    std::string title;
    int windowMode;
    static bool exists;
private:
    void addWindowListeners()
    {
        ofAddListener(windowPtr->events().setup, this, &pdWindow::setup);
        ofAddListener(windowPtr->events().update, this, &pdWindow::update);
        ofAddListener(windowPtr->events().draw, this, &pdWindow::draw);
        ofAddListener(windowPtr->events().exit, this, &pdWindow::exit);
        ofAddListener(windowPtr->events().keyPressed, this, &pdWindow::keyPressed);
        ofAddListener(windowPtr->events().keyReleased, this, &pdWindow::keyReleased);
        ofAddListener(windowPtr->events().mouseMoved, this, &pdWindow::mouseMoved);
        ofAddListener(windowPtr->events().mouseDragged, this, &pdWindow::mouseDragged);
        ofAddListener(windowPtr->events().mousePressed, this, &pdWindow::mousePressed);
        ofAddListener(windowPtr->events().mouseReleased, this, &pdWindow::mouseReleased);
        ofAddListener(windowPtr->events().mouseScrolled, this, &pdWindow::mouseScrolled);
        ofAddListener(windowPtr->events().mouseEntered, this, &pdWindow::mouseEntered);
        ofAddListener(windowPtr->events().mouseExited, this, &pdWindow::mouseExited);
        ofAddListener(windowPtr->events().windowResized, this, &pdWindow::windowResized);
        ofAddListener(windowPtr->events().messageEvent, this, &pdWindow::messageReceived);
        ofAddListener(windowPtr->events().fileDragEvent, this, &pdWindow::dragged);
        ofAddListener(windowPtr->events().touchCancelled, this, &pdWindow::touchCancelled);
        ofAddListener(windowPtr->events().touchDoubleTap, this, &pdWindow::touchDoubleTap);
        ofAddListener(windowPtr->events().touchDown, this, &pdWindow::touchDown);
        ofAddListener(windowPtr->events().touchMoved, this, &pdWindow::touchMoved);
        ofAddListener(windowPtr->events().touchUp, this, &pdWindow::touchUp);
    }
    void removeWindowListeners()
    {
        ofRemoveListener(windowPtr->events().setup, this, &pdWindow::setup);
        ofRemoveListener(windowPtr->events().update, this, &pdWindow::update);
        ofRemoveListener(windowPtr->events().draw, this, &pdWindow::draw);
        ofRemoveListener(windowPtr->events().exit, this, &pdWindow::exit);
        ofRemoveListener(windowPtr->events().keyPressed, this, &pdWindow::keyPressed);
        ofRemoveListener(windowPtr->events().keyReleased, this, &pdWindow::keyReleased);
        ofRemoveListener(windowPtr->events().mouseMoved, this, &pdWindow::mouseMoved);
        ofRemoveListener(windowPtr->events().mouseDragged, this, &pdWindow::mouseDragged);
        ofRemoveListener(windowPtr->events().mousePressed, this, &pdWindow::mousePressed);
        ofRemoveListener(windowPtr->events().mouseReleased, this, &pdWindow::mouseReleased);
        ofRemoveListener(windowPtr->events().mouseScrolled, this, &pdWindow::mouseScrolled);
        ofRemoveListener(windowPtr->events().mouseEntered, this, &pdWindow::mouseEntered);
        ofRemoveListener(windowPtr->events().mouseExited, this, &pdWindow::mouseExited);
        ofRemoveListener(windowPtr->events().windowResized, this, &pdWindow::windowResized);
        ofRemoveListener(windowPtr->events().messageEvent, this, &pdWindow::messageReceived);
        ofRemoveListener(windowPtr->events().fileDragEvent, this, &pdWindow::dragged);
        ofRemoveListener(windowPtr->events().touchCancelled, this, &pdWindow::touchCancelled);
        ofRemoveListener(windowPtr->events().touchDoubleTap, this, &pdWindow::touchDoubleTap);
        ofRemoveListener(windowPtr->events().touchDown, this, &pdWindow::touchDown);
        ofRemoveListener(windowPtr->events().touchMoved, this, &pdWindow::touchMoved);
        ofRemoveListener(windowPtr->events().touchUp, this, &pdWindow::touchUp);
    }
    void setup(ofEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::setupVec, "setup");
    }
    void update(ofEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::updateVec, "update");
    }
    void draw(ofEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::drawVec, "draw");
    }
    void exit(ofEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::exitVec, "exit");
        removeWindowListeners();
        owner = false;
        exists = false;
    }
    void keyPressed(ofKeyEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::keyPressedVec, "keyPressed", e);
    }
    void keyReleased(ofKeyEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::keyReleasedVec, "keyReleased", e);
    }
    void mouseMoved(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseMovedVec, "mouseMoved", e);
    }
    void mouseDragged(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseDraggedVec, "mouseDragged", e);
    }
    void mousePressed(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mousePressedVec, "mousePressed", e);
    }
    void mouseReleased(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseReleasedVec, "mouseReleased", e);
    }
    void mouseScrolled(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseScrolledVec, "mouseScrolled", e);
    }
    void mouseEntered(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseEnteredVec, "mouseEntered", e);
    }
    void mouseExited(ofMouseEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::mouseExitedVec, "mouseExited", e);
    }
    void windowResized(ofResizeEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::windowResizedVec, "windowResized", e);
    }
    void messageReceived(ofMessage &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::messageReceivedVec, "messageReceived", e);
    }
    void dragged(ofDragInfo &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::draggedVec, "dragged", e);
    }
    void touchCancelled(ofTouchEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::touchCancelledVec, "touchCancelled", e);
    }
    void touchDoubleTap(ofTouchEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::touchDoubleTapVec, "touchDoubleTap", e);
    }
    void touchDown(ofTouchEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::touchDownVec, "touchDown", e);
    }
    void touchMoved(ofTouchEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::touchMovedVec, "touchMoved", e);
    }
    void touchUp(ofTouchEventArgs &e)
    {
        ofxOfeliaEvents::callEventListeners(ofxOfeliaEvents::touchUpVec, "touchUp", e);
    }
    
    ofMainLoop *mainLoop;
    ofAppBaseWindow *windowPtr;
    glm::vec2 position;
    int width;
    int height;
    bool positionSet;
    bool sizeSet;
    bool owner;
};

bool pdWindow::exists = false;

class pdCanvas
{
public:
    pdCanvas(std::string s)
    :x(ofxOfeliaLua::getByName(s)), parent(0)
    {
    };
    pdCanvas(std::string s, t_floatarg f)
    :x(ofxOfeliaLua::getByName(s)), parent(f)
    {
    };
    std::string getDollarZero()
    {
        x->messageManager->sendMessage(canvas_get_dir, x->getUniqueId(), parent);
        return std::get<0>(x->messageManager->waitForReturnValue<std::string>());
    }
    std::string getName()
    {
        x->messageManager->sendMessage(canvas_get_name, x->getUniqueId(), parent);
        return std::get<0>(x->messageManager->waitForReturnValue<std::string>());
    }
    int getIndex()
    {
        x->messageManager->sendMessage(canvas_get_index, x->getUniqueId(), parent);
        return std::get<0>(x->messageManager->waitForReturnValue<int>());
    }
    
    void getArgs(int *argcp, t_atom **argvp, t_canvas* canvasp)
    {
        x->messageManager->sendMessage(canvas_get_args, x->getUniqueId(), parent);
        auto [canvasName, args] = x->messageManager->waitForReturnValue<std::string, std::vector<t_atom>>();
        
        // Store on the class to prevent them from being deleted
        lastCanvasArgs = args;
        
        *argcp = lastCanvasArgs.size();
        *argvp = lastCanvasArgs.data();
        *canvasp = canvasName;
    }
    void setArgs(int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        x->messageManager->sendMessage(canvas_set_args, x->getUniqueId(), parent, std::vector<t_atom>(argv, argv + argc));
    }
    void getPosition(int **posp)
    {
        x->messageManager->sendMessage(canvas_get_position, x->getUniqueId(), parent);
        auto [xPos, yPos] = x->messageManager->waitForReturnValue<int, int>();
        int *pos = static_cast<int *>(getbytes(2 * sizeof(int)));
        pos[0] = xPos;
        pos[1] = yPos;
        *posp = pos;
    }
    
    void setPosition(int xpos, int ypos)
    {
        x->messageManager->sendMessage(canvas_set_position, x->getUniqueId(), parent, xpos, ypos);
    }
    std::string getDir()
    {
        x->messageManager->sendMessage(canvas_get_dir, x->getUniqueId(), parent);
        return std::get<0>(x->messageManager->waitForReturnValue<std::string>());
    }
    std::string makeFileName(std::string s)
    {
        x->messageManager->sendMessage(canvas_get_dir, x->getUniqueId(), parent, s);
        return std::get<0>(x->messageManager->waitForReturnValue<std::string>());
    }

private:
    
    // Removing a canvas is disabled for now
    /*
    void remove()
    {
       
        if (x == nullptr) return;
        clock = clock_new(this, reinterpret_cast<t_method>(removeCanvas));
        clock_delay(clock, 0.0);
    }
    
    static void removeCanvas(pdCanvas *canvas)
    {
        clock_free(canvas->clock);
        int dspstate = canvas_suspend_dsp();
        glist_delete(static_cast<t_glist*>(canvas->canvas), canvas->gobj);
        canvas_resume_dsp(dspstate);
    } */
    
    ofxOfeliaLua *x;
    
    int parent;
    std::vector<t_atom> lastCanvasArgs;
};

class pdSend
{
public:
    pdSend(std::string s)
    :sym(s){};
    void sendBang()
    {
        //if (sym->s_thing) pd_bang(sym->s_thing);
    }
    void sendFloat(float f)
    {
        //if (sym->s_thing) pd_float(sym->s_thing, f);
    }
    void sendSymbol(std::string s)
    {
        //if (sym->s_thing) pd_symbol(sym->s_thing, s);
    }
    void sendPointer(t_gpointer *p)
    {
        /*
        int userDataRef = luaL_ref(ofxOfeliaLua::L, LUA_REGISTRYINDEX);
        if (sym->s_thing) pd_pointer(sym->s_thing, reinterpret_cast<t_gpointer *>(&userDataRef));
        luaL_unref(ofxOfeliaLua::L, LUA_REGISTRYINDEX, userDataRef); */
    }
    void sendList(int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        //if (sym->s_thing) pd_list(sym->s_thing, &s_list, argc, argv);
    }
    void sendAnything(int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        /*
        if (sym->s_thing)
        {
            if (argv[0].a_type == A_SYMBOL)
                typedmess(sym->s_thing, argv[0].a_w.w_symbol, argc - 1, argv + 1);
            else if (argv[0].a_type == A_FLOAT)
                pd_list(sym->s_thing, &s_list, argc, argv);
        } */
    }
private:
    std::string sym;
};

class pdInlet
{
public:
    pdInlet(std::string s)
    //:x(ofxOfeliaLua::getByName(s))
    {};
    
    void setFloatInlet(int n, float f)
    {
        /*
        if (!doesPassiveInletExist()) return;

        n = ofClamp(n, 0, x->io.numInlets - 2);
        if (x->io.av[n].a_type != A_FLOAT)
        {
            postWrongPassiveInletArgTypeError();
            return;
        }
        x->io.av[n].a_w.w_float = f; */
    }
    void setSymbolInlet(int n, std::string s)
    {
        /*
        if (!doesPassiveInletExist()) return;

        n = ofClamp(n, 0, x->io.numInlets - 2);
        if (x->io.av[n].a_type != A_SYMBOL)
        {
            postWrongPassiveInletArgTypeError();
            return;
        }
        x->io.av[n].a_w.w_symbol = s; */
    }
    void setInlets(int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        /*
        if (!doesPassiveInletExist()) return;

        const int numPassiveInlets = x->io.numInlets - 1;
        if (argc > numPassiveInlets)
            argc = numPassiveInlets;
        for (int i = 0; i < argc; ++i)
        {
            if (x->io.av[i].a_type != argv[i].a_type)
            {
                postWrongPassiveInletArgTypeError();
                return;
            }
            x->io.av[i].a_w = argv[i].a_w;
        } */
    }
    void setSignalInlet(t_floatarg f)
    {
        /*
        if (!doesSignalInletExist()) return;

        x->signal.f = f; */
    }
private:
    bool doesPassiveInletExist()
    {
        /*
        if (x == nullptr) return false;
        if (!x->io.hasMultiControlInlets)
        {
            pd_error(NULL, "ofelia: passive inlet does not exist");
            return false;
        } */
        return true;
    }
    bool doesSignalInletExist()
    {
        /*
        if (x == nullptr) return false;
        if (!x->isSignalObject)
        {
            pd_error(NULL, "ofelia: signal inlet does not exist");
            return false;
        }
        return true;  */
        return false;
    }
    void postWrongPassiveInletArgTypeError()
    {
        //pd_error(NULL, "ofelia: wrong passive inlet argument type to set");
    }
    
    //ofxOfeliaLua *x;
};

class pdOutlet
{
public:
    pdOutlet(std::string s)
    :x(ofxOfeliaLua::getByName(s))
    {};
    
    void outletBang(int index)
    {
        x->messageManager->sendMessage(pd_outlet_bang, x->getUniqueId(), index);
    }
    void outletFloat(int index, t_floatarg f)
    {
          x->messageManager->sendMessage(pd_outlet_float, x->getUniqueId(), index, f);
    }
    void outletSymbol(int index, std::string s)
    {
        x->messageManager->sendMessage(pd_outlet_float, x->getUniqueId(), index, s);
    }
    void outletPointer(int index, t_gpointer *p)
    {
        int userDataRef = luaL_ref(ofxOfeliaLua::L, LUA_REGISTRYINDEX);
        x->messageManager->sendMessage(pd_outlet_pointer, x->getUniqueId(), index, userDataRef);
        luaL_unref(ofxOfeliaLua::L, LUA_REGISTRYINDEX, userDataRef);
    }
    void outletList(int index, int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        x->messageManager->sendMessage(pd_outlet_list, x->getUniqueId(), index, std::vector<t_atom>(argv, argv + argc), false);
    }
    void outletAnything(int index, int argc, t_atom *argv, std::deque<int> userDataRef)
    {
        if (argv[0].a_type == A_SYMBOL)
            x->messageManager->sendMessage(pd_outlet_anything, x->getUniqueId(), index,  argv[0].a_w.w_symbol, std::vector<t_atom>(argv + 1, argv + argc));
        else if (argv[0].a_type == A_FLOAT)
            x->messageManager->sendMessage(pd_outlet_list, x->getUniqueId(), index, std::vector<t_atom>(argv, argv + argc), false);
    }
private:

    ofxOfeliaLua *x;
};

class pdValue
{
public:
    pdValue(std::string s)
    :sym(s)
    ,floatstar(/*value_get(sym)*/ 0.0f){};
    virtual ~pdValue()
    {
        //value_release(sym);
    };
    float get()
    {
//        return *floatstar;
        return 0;
    }
    void set(t_floatarg f)
    {
        //*floatstar = f;
    }
private:
    std::string sym;
    float floatstar;
};

class pdArray
{
public:
    pdArray(std::string s)
    :sym(s){};
    float getAt(int n)
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//        {
//            if (n < 0) n = 0;
//            else if (n >= size) n = size - 1;
//            return size ? vec[n].w_float : 0;
//        }
        return 0;
    }
    void setAt(int n, t_floatarg f)
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//        {
//            if (n < 0) n = 0;
//            else if (n >= size) n = size - 1;
//            vec[n].w_float = f;
//            garray_redraw(a);
//        }
    }
    float __getitem__(int n)
    {
//        return getAt(n);
        return 0.0f;
    }
    void __setitem__(int n, t_floatarg f)
    {
//        setAt(n, f);
    }
    void get(t_word **vecp, int *sizep, int onset)
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//        {
//            if (onset < 0) onset = 0;
//            *vecp = vec + onset;
//            *sizep = size - onset;
//        }
    }
    void set(int n, t_floatarg *f, int onset)
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//        {
//            if (onset < 0) onset = 0;
//            for (int i = 0; i < n; ++i)
//            {
//                int io = i + onset;
//                if (io < size) vec[io].w_float = f[i];
//                else break;
//            }
//            garray_redraw(a);
//        }
    }
    int getSize()
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//            return size;
//        return 0;
        return 0;
    }
    void setSize(long n)
    {
//        t_garray *a; int size; t_word *vec;
//        if (exists(&a) && getData(a, &size, &vec))
//        {
//            garray_resize_long(a, n);
//            garray_redraw(a);
//        }
    }
private:
    bool exists(t_garray **a)
    {
//        *a = reinterpret_cast<t_garray *>(pd_findbyclass(sym, garray_class));
//        return (*a != nullptr);
        return false;
    }
    bool getData(t_garray *a, int *size, t_word **vec)
    {
//        if (!garray_getfloatwords(a, size, vec))
//        {
//            pd_error(NULL, "ofelia: bad template for array '%s'", sym);
              return false;
//        }
        
//        return true;
    }
    std::string sym;
};

class pdClock
{
    
    class TimerThread : public ofThread {
    public:
        
        void startTimer(int durationMillis) {
            duration = durationMillis;
            startThread();
        }

        void stopTimer() {
            stopThread();
        }

        void threadedFunction() {
            ofSleepMillis(duration);
            ofNotifyEvent(timerCompleteEvent);
        }

        ofEvent<void> timerCompleteEvent;

    private:
        int duration;
    };

public:
    pdClock(std::string s)
    :
    x(ofxOfeliaLua::getByName(s)),
    sym("delayed")
    {
    };
    
    pdClock(std::string s, std::string s2)
    :
    x(ofxOfeliaLua::getByName(s)),
    sym(s2)
    {};
    
    virtual ~pdClock()
    {
    };
    void delay(double delayTime)
    {
        ofAddListener(timerThread.timerCompleteEvent, this, &pdClock::delayedFunction);
        timerThread.startTimer(delayTime);
    };
    void unset()
    {
        timerThread.stopTimer();
    };
private:
    void delayedFunction()
    {
        x->doFunction(sym.c_str());
        timerThread.stopTimer();
    };

    // This clock runs on the audio thread, but this may get deleted on the message thread
    ofxOfeliaLua* x;
    
    TimerThread timerThread;
    std::string sym;
};

class pdLog
{
public:
    pdLog()
    :sym(""), messageManager(ofxOfeliaMessageManager::instance)
    {};
    
    pdLog(std::string s)
    :sym(s), messageManager(ofxOfeliaMessageManager::instance)
    {};
    
    void post(const std::string &str, int level = 2)
    {
        std::string message;
        if(!sym.empty())
        {
            message += sym + ": ";
        }
        message += str;
        
        messageManager->sendMessage(pd_log, false, message);
    }
    void error(const std::string &str)
    {
        std::string message;
        if(!sym.empty())
        {
            message += sym + ": ";
        }
        message += str;
        
        messageManager->sendMessage(pd_log, true, message);
    }
private:
    std::string sym; /* module name */
    ofxOfeliaMessageManager* messageManager;
};

/*
static void pdSysGui(std::string str)
{
    str += '\n';
    sys_gui(const_cast<char*>(str.c_str()));
}

static int pdGetBlockSize()
{
    return sys_getblksize();
}

static t_float pdGetSampleRate()
{
    return sys_getsr();
}

static int pdGetNumInChannels()
{
    return sys_get_inchannels();
}

static int pdGetNumOutChannels()
{
    return sys_get_outchannels();
}

static bool pdGetDspState()
{
    return pd_getdspstate() != 0;
}

static int pdGetMaxString()
{
    return MAXPDSTRING;
}

static int pdGetFloatSize()
{
    return PD_FLOATSIZE;
}

static t_float pdGetMinFloat()
{
    return std::numeric_limits<t_float>::lowest();
}

static t_float pdGetMaxFloat()
{
    return std::numeric_limits<t_float>::max();
}

static bool pdIsBadFloat(t_floatarg f)
{
    return PD_BADFLOAT(f) != 0;
}

static bool pdIsBigOrSmall(t_floatarg f)
{
    return PD_BIGORSMALL(f) != 0;
} */

static void pdSysGui(std::string str)
{
}

static int pdGetBlockSize()
{
    return 0;
}

static float pdGetSampleRate()
{
    return 0;
}

static int pdGetNumInChannels()
{
    return 0;
}

static int pdGetNumOutChannels()
{
    return 0;
}

static bool pdGetDspState()
{
    return 0;
}

static int pdGetMaxString()
{
    return MAXPDSTRING;
}

static int pdGetFloatSize()
{
    return 32;
}

static float pdGetMinFloat()
{
    return std::numeric_limits<float>::lowest();
}

static float pdGetMaxFloat()
{
    return std::numeric_limits<float>::max();
}

static bool pdIsBadFloat(t_floatarg f)
{
    return 0;
}

static bool pdIsBigOrSmall(t_floatarg f)
{
    return 0;
}
