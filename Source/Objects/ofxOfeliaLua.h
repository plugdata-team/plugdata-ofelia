#pragma once

#define MAXOFXSTRING 12000

#include "m_pd.h"
#include "g_canvas.h"
#include <string>
#include <format>

#include "ofxOfeliaMessageManager.h"
#include "ofxOfeliaData.h"


class ofxOfeliaData;
class ofxOfeliaLua : public ofxOfeliaMessageListener
{
public:
    ofxOfeliaLua(ofxOfeliaData *ptr)
    : dataPtr(ptr)
    {
        messageManager = ofxOfeliaMessageManager::getOrCreate();
        messageManager->sendMessage(ofx_lua_init, uid());
        messageManager->addListener(this);
    }

    ~ofxOfeliaLua()
    {
        messageManager->removeListener(this);
    }
    
    void receiveMessage(ofxMessageType type, const std::string& message)
    {
        switch(type)
        {
            case pd_outlet_bang:
            {
                auto [identifier, index] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                
                if(dataPtr->io.hasControlOutlet && dataPtr->io.numOutlets > index)
                {
                    outlet_bang(dataPtr->io.outlets[index]);
                }
                
                break;
            }
            case pd_outlet_float:
            {
                auto [identifier, index, value] = messageManager->parseMessage<std::string, int, float>(message);
                
                if(identifier != uid()) return;
                
                if(dataPtr->io.hasControlOutlet && dataPtr->io.numOutlets > index)
                {
                    outlet_float(dataPtr->io.outlets[index], value);
                }
                
                break;
            }
            case pd_outlet_symbol:
            {
                auto [identifier, index, symbol] = messageManager->parseMessage<std::string, int, std::string>(message);
                
                if(identifier != uid()) return;
                
                if(dataPtr->io.hasControlOutlet && dataPtr->io.numOutlets > index)
                {
                    outlet_symbol(dataPtr->io.outlets[index], gensym(symbol.c_str()));
                }
                
                break;
                
                break;
            }
            case pd_outlet_pointer:
            {
                /*
                auto [identifier, index, symbol] = messageManager->parseMessage<std::string, int, float>(message);
                
                if(identifier != uid()) return;
                
                if(dataPtr->io.hasControlOutlet && dataPtr->io.numOutlets > index)
                {
                    outlet_symbol(dataPtr->io.outlets[index], gensym(symbol.c_str()));
                } */
                
                break;
            }
            case pd_outlet_list:
            {
                auto [identifier, index, atoms, distribute] = messageManager->parseMessage<std::string, int, std::vector<t_atom>, bool>(message);

                if(identifier != uid()) return;
                
                auto& io = dataPtr->io;
                int numOutlets = io.numOutlets;
                if (numOutlets > 1 && distribute)
                {
                    int last = (numOutlets >= atoms.size() ? atoms.size() : numOutlets) - 1;
                    for (int i = last; i >= 0; --i)
                    {
                        if (atoms[i].a_type == A_FLOAT)
                            outlet_float(io.outlets[i], atoms[i].a_w.w_float);
                        else if (atoms[i].a_type == A_SYMBOL)
                            outlet_symbol(io.outlets[i], atoms[i].a_w.w_symbol);
                        else if (atoms[i].a_type == A_POINTER)
                            outlet_pointer(io.outlets[i], reinterpret_cast<t_gpointer*>(atoms[i].a_w.w_gpointer));
                    }
                }
                else
                    outlet_list(io.outlets[index], gensym("list"), atoms.size(), atoms.data());
                
                break;
            }
            case pd_outlet_anything:
            {
                auto [identifier, index, symbol, atoms] = messageManager->parseMessage<std::string, int, std::string, std::vector<t_atom>>(message);
                outlet_anything(dataPtr->io.outlets[index], gensym(symbol.c_str()), atoms.size(), atoms.data());
                break;
            }
            case canvas_realise_dollar:
            {
                auto [name, symbol] = messageManager->parseMessage<std::string, std::string>(message);
                auto* glist = reinterpret_cast<t_glist*>(pd_findbyclass(gensym(name.c_str()), canvas_class));
                auto* retsym = gensym(symbol.c_str());
                if(glist) {
                    retsym = canvas_realizedollar(glist, retsym);
                }
                messageManager->sendReturnValue(std::string(retsym->s_name));
                break;
            }
            case canvas_get_dollar_zero:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (!canvas) return;
                
                auto* retsym = gensym("$0");
                if(canvas) {
                    retsym = canvas_realizedollar(canvas, retsym);
                }
                
                messageManager->sendReturnValue(std::string(retsym->s_name));
                break;
            }
            case canvas_get_name:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (!canvas) return;
                
                char buf[MAXPDSTRING];
                std::snprintf(buf, MAXPDSTRING, ".x%lx.c", reinterpret_cast<unsigned long>(canvas));
                messageManager->sendReturnValue<std::string>(std::string(buf));
                break;
            }
            case canvas_get_args:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (!canvas) return;
                
                int argcp;
                t_atom* argvp;
                t_symbol* canvas_name;
                t_binbuf *binbuf = canvas->gl_obj.te_binbuf;
                if (!binbuf)
                {
                    canvas_setcurrent(canvas);
                    canvas_getargs(&argcp, &argvp);
                    canvas_unsetcurrent(canvas);
                    return;
                }
                argcp = binbuf_getnatom(binbuf) - 1;
                argvp = binbuf_getvec(binbuf) + 1;
                
                char buf[MAXPDSTRING];
                snprintf(buf, MAXPDSTRING, ".x%lx", (long unsigned int)(canvas->gl_owner ? canvas->gl_owner : canvas));
                canvas_name = gensym(buf);
                
                messageManager->sendReturnValue(std::string(canvas_name->s_name), std::vector<t_atom>(argvp, argvp + argcp));
                break;
            }
            case canvas_set_args:
            {
                auto [identifier, parent, args] = messageManager->parseMessage<std::string, int, std::vector<t_atom>>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
                t_binbuf *binbuf = canvas->gl_obj.te_binbuf;
                if (!binbuf) return;
                t_atom av[1];
                SETSYMBOL(av, binbuf_getvec(binbuf)[0].a_w.w_symbol);
                binbuf_clear(binbuf);
                binbuf_add(binbuf, 1, av);
                binbuf_add(binbuf, args.size(), args.data());
                
                break;
            }
            case canvas_get_dir:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
                auto* dir = canvas_getdir(canvas);
                messageManager->sendReturnValue(std::string(dir->s_name));
                break;
            }
            case canvas_make_filename:
            {
                auto [identifier, parent, symbol] = messageManager->parseMessage<std::string, int, std::string>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
                char buf[MAXPDSTRING];
                canvas_makefilename(canvas, symbol.c_str(), buf, MAXPDSTRING);
                messageManager->sendReturnValue(std::string(buf));
                break;
            }
            case canvas_get_index:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
                t_glist *glist = static_cast<t_glist*>(canvas);
                t_gobj *obj;
                int index = 0;
                for (obj = glist->gl_list; obj && obj != (t_gobj*)glist; obj = obj->g_next) index++;
                messageManager->sendReturnValue(index);
            }
            case canvas_get_position:
            {
                auto [identifier, parent] = messageManager->parseMessage<std::string, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
                messageManager->sendReturnValue(static_cast<int>(canvas->gl_obj.te_xpix), static_cast<int>(canvas->gl_obj.te_ypix));
            }
            case canvas_set_position:
            {
                auto [identifier, parent, xpos, ypos] = messageManager->parseMessage<std::string, int, int, int>(message);
                
                if(identifier != uid()) break;
                auto* canvas = getParentCanvas(dataPtr->canvas, parent);
                if (canvas == nullptr) break;
                
            
                int dx = xpos - canvas->gl_obj.te_xpix;
                int dy = ypos - canvas->gl_obj.te_ypix;
                if (!dx && !dy) return;
                
                if (canvas->gl_owner && glist_isvisible(canvas->gl_owner))
                {
                    gobj_displace(reinterpret_cast<t_gobj *>(canvas), canvas->gl_owner, dx, dy);
                    canvas_fixlinesfor(canvas->gl_owner, reinterpret_cast<t_text *>(canvas));
                }
                else
                {
                    canvas->gl_obj.te_xpix += dx;
                    canvas->gl_obj.te_ypix += dy;
                }
            }

                
            default: break;
        }
    }
    
    t_canvas* getParentCanvas(t_canvas* canvas, int index)
    {
        if (!canvas) return;
        
        while (index)
        {
            if (!canvas->gl_owner) break;
            canvas = canvas->gl_owner;
            index--;
        }
        
        return canvas;
    }
    
    void doFunction(t_symbol* symbol)
    {
        messageManager->sendMessage(ofx_lua_do_function_s, uid(), std::string(symbol->s_name));
    }
    
    void doFunction(t_symbol* symbol, t_gpointer* pointer)
    {
        messageManager->sendMessage(ofx_lua_do_function_sp, uid(), std::string(symbol->s_name));
    }
    void doFunction(t_symbol* symbol1, t_symbol* symbol2)
    {
        messageManager->sendMessage(ofx_lua_do_function_ss, uid(), std::string(symbol1->s_name), std::string(symbol2->s_name));
    }
    void doFunction(t_symbol* symbol, float number)
    {
        messageManager->sendMessage(ofx_lua_do_function_sf, uid(), std::string(symbol->s_name), number);
    }
    void doFunction(t_symbol* symbol, int argc, t_atom* argv)
    {
        messageManager->sendMessage(ofx_lua_do_function_sa, uid(), std::string(symbol->s_name), std::vector<t_atom>(argv, argv + argc));
    }
    void doFreeFunction()
    {
        messageManager->sendMessage(ofx_lua_do_free_function, uid());
    }
    void getVariableByArgs(t_symbol* symbol, int argc, t_atom* argv)
    {
        messageManager->sendMessage(ofx_lua_get_var_by_args, uid(), std::string(symbol->s_name), std::vector<t_atom>(argv, argv + argc));
    }
    void setVariableByArgs(t_symbol* symbol, int argc, t_atom* argv)
    {
        messageManager->sendMessage(ofx_lua_set_var_by_args, uid(), std::string(symbol->s_name), std::vector<t_atom>(argv, argv + argc));
    }
    
    void doArgs(int argc, t_atom* argv)
    {
        std::ostringstream ss;
        
        for (int i = 0; i < argc; i++)
        {
            if (argv[i].a_type == A_SYMBOL)
                ss << argv[i].a_w.w_symbol->s_name;
            else if (argv[i].a_type == A_FLOAT)
                ss << argv[i].a_w.w_float;
            ss << ' ';
        }
        
        auto s = ss.str();
        doString(s);
    }
    
    void doString(const std::string& s)
    {
        std::ostringstream ss;
        
        auto* name = dataPtr->sym->s_name;
        ss << "package.preload['" << name << "'] = nil package.loaded['" << name << "'] = nil\n"
        << "package.preload['" << name << "'] = function(this) local ofelia = {} local M = ofelia\n";
        
        auto isSignalObject = false;
        
        if (!dataPtr->isFunctionMode)
            ss << s;
        else if (!isSignalObject)
            {
                ss << "function M.bang() return M.anything(nil) end function M.float(f) return M.anything(f) end function M.symbol(s) return M.anything(s) end function M.pointer(p) return M.anything(p) end function M.list(l) return M.anything(l) end function M.anything(a)\n" << s << "\nend";
            }
        else
        {
            ss << "function M.perform(";
            for (int i = 0; i < dataPtr->io.numInlets; ++i)
            {
                if (i) ss << ',';
                ss << 'a' << i + 1;
            }
            ss << ")\n" << s << "\nend";
        }
        ss << "\nreturn M\n end";
        
        messageManager->sendMessage(ofx_lua_do_string, uid(), ss.str());
    }
    
    void doText()
    {
        int lengthp;
        char *bufp;
        realizeDollar(&bufp, &lengthp);
        doString(std::string(bufp, lengthp));
        freebytes(bufp, lengthp);
    }
    
    std::string uid()
    {
        return std::string(dataPtr->getUniqueSym()->s_name);
    }
    
    void realizeDollar(char **bufp, int *lengthp)
    {
        /* get the string from text editor gui */
        binbuf_gettext(dataPtr->binbuf, bufp, lengthp);

        char *buf = static_cast<char *>(getbytes(0)); // output buffer
        int length = 0; // output length
        for (int i = 0; i < *lengthp; ++i)
        {
            if ((*bufp)[i] == '$')
            {
                int first = i + 1; // index to first digit
                char dbuf[MAXOFXSTRING] = "$"; // buffer to store dollarsym
                int dlen = 1; // length of dollarsym
                while (first < *lengthp && std::isdigit((*bufp)[first]))
                    dbuf[dlen++] = (*bufp)[first++];
                int tlen = dlen - 1; // number of trailing digits
                if (tlen)
                {
                    dbuf[dlen] = '\0';
                    t_symbol *s = canvas_realizedollar(dataPtr->canvas, gensym(dbuf));
                    std::strcpy(dbuf, s->s_name);
                    dlen = std::strlen(s->s_name);
                    int newlength = length + dlen;
                    buf = static_cast<char *>(resizebytes(buf, length, newlength));
                    std::strcpy(buf + length, dbuf);
                    length = newlength;
                    i += tlen;
                }
                else goto append;
            }
            else
            {
            append: // append a character
                buf = static_cast<char *>(resizebytes(buf, length, length + 1));
                buf[length++] = (*bufp)[i];
            }
        }
    }

    
    ofxOfeliaData* dataPtr;
    ofxOfeliaMessageManager* messageManager;
    
public:
    bool isChunkRun = false;
};
