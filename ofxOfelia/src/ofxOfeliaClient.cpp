#include "ofxOfeliaClient.h"
#include "ofxOfeliaAsync.h"
#include <cstring>
#include <vector>

bool ofxOfeliaClient::isDataValid()
{
    ofxOfeliaData *y = ofxOfeliaData::getPtr(data.sym);
    if (y == nullptr) return false;
    data.lua.isChunkRun = y->lua.isChunkRun;
    return true;
}

void ofxOfeliaClient::bangMethod()
{
    ofxOfeliaAsync::callAsync([this](){
        if (!isDataValid()) return;
        if (data.io.hasMultiControlInlets)
        {
            int ac = data.io.numInlets - 1;
            t_atom *av = static_cast<t_atom *>(getbytes(sizeof(t_atom) * ac));
            data.io.doList(ac, av);
            freebytes(av, sizeof(t_atom) * ac);
            return;
        }
        data.lua.doFunction(&s_bang);
    });
}

void ofxOfeliaClient::floatMethod(t_floatarg f)
{
    ofxOfeliaAsync::callAsync([this, f](){
        if (!isDataValid()) return;
        if (data.io.hasMultiControlInlets)
        {
            int ac = data.io.numInlets;
            t_atom *av = static_cast<t_atom *>(getbytes(sizeof(t_atom) * ac));
            SETFLOAT(av, f);
            data.io.doList(ac, av);
            freebytes(av, sizeof(t_atom) * ac);
            return;
        }
        data.lua.doFunction(&s_float, f);
    });
}

void ofxOfeliaClient::symbolMethod(t_symbol *s)
{
    ofxOfeliaAsync::callAsync([this, s](){
        if (!isDataValid()) return;
        if (data.io.hasMultiControlInlets)
        {
            int ac = data.io.numInlets;
            t_atom *av = static_cast<t_atom *>(getbytes(sizeof(t_atom) * ac));
            SETSYMBOL(av, s);
            data.io.doList(ac, av);
            freebytes(av, sizeof(t_atom) * ac);
            return;
        }
        data.lua.doFunction(&s_symbol, s);
    });
}

void ofxOfeliaClient::pointerMethod(t_gpointer *p)
{
    ofxOfeliaAsync::callAsync([this, p](){
        if (!isDataValid()) return;
        if (data.io.hasMultiControlInlets)
        {
            int ac = data.io.numInlets;
            t_atom *av = static_cast<t_atom *>(getbytes(sizeof(t_atom) * ac));
            SETPOINTER(av, p);
            data.io.doList(ac, av);
            freebytes(av, sizeof(t_atom) * ac);
            return;
        }
        data.lua.doFunction(&s_pointer, p);
    });
}

void ofxOfeliaClient::listMethod(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([this, s, args]() mutable {
        if (!isDataValid()) return;
        if (!args.size())
        {
            bangMethod();
            return;
        }
        if (args.size() == 1)
        {
            if (args[0].a_type == A_FLOAT)
                floatMethod(args[0].a_w.w_float);
            else if (args[0].a_type == A_SYMBOL)
                symbolMethod(args[0].a_w.w_symbol);
            else if (args[0].a_type == A_POINTER)
                pointerMethod(args[0].a_w.w_gpointer);
            return;
        }
        if (data.io.hasMultiControlInlets)
        {
            int ac = data.io.numInlets + args.size() - 1;
            t_atom *av = static_cast<t_atom *>(getbytes(sizeof(t_atom) * ac));
            std::memcpy(av, args.data(), sizeof(t_atom) * args.size());
            data.io.doList(ac, av);
            freebytes(av, sizeof(t_atom) * ac);
            return;
        }
        data.lua.doFunction(&s_list, args.size(), args.data());
    });
}

void ofxOfeliaClient::getVariableByArgs(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([this, s, args]() mutable {
        if (!isDataValid()) return;
        data.lua.getVariableByArgs(s, args.size(), args.data());
    });
}

void ofxOfeliaClient::setVariableByArgs(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([this, s, args]() mutable {
        if (!isDataValid()) return;
        data.lua.setVariableByArgs(s, args.size(), args.data());
    });
}

void ofxOfeliaClient::dspMethod(t_signal **sp)
{
    if (!data.isSignalObject) return;
    if (!isDataValid()) return;
    data.signal.addDsp(sp);
}

void ofxOfeliaClient::freeMethod()
{
    ofxOfeliaAsync::callAsync([this](){
        if (!data.isSignalObject) data.io.freeControlIO();
        else data.io.freeSignalIO();
    });
}
