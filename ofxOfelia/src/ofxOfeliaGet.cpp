#include "ofxOfeliaGet.h"
#include "ofxOfeliaMessageManager.h"
#include <new>

t_class *ofxOfeliaGet::pdClass;

void *ofxOfeliaGet::newMethod(t_symbol *s, int argc, t_atom *argv)
{
    const ofxOfeliaLock ofxLock;
    client.data.argParse(s, argc, argv, false);
    return this;
}

void ofxOfeliaGet::bangMethod()
{
    const ofxOfeliaLock ofxLock;
    client.bangMethod();
}

void ofxOfeliaGet::floatMethod(t_floatarg f)
{
    const ofxOfeliaLock ofxLock;
    client.floatMethod(f);
}

void ofxOfeliaGet::symbolMethod(t_symbol *s)
{
    const ofxOfeliaLock ofxLock;
    client.symbolMethod(s);
}

void ofxOfeliaGet::pointerMethod(t_gpointer *p)
{
    const ofxOfeliaLock ofxLock;
    client.pointerMethod(p);
}

void ofxOfeliaGet::listMethod(t_symbol *s, int argc, t_atom *argv)
{
    const ofxOfeliaLock ofxLock;
    client.listMethod(s, argc, argv);
}

void ofxOfeliaGet::anythingMethod(t_symbol *s, int argc, t_atom *argv)
{
    const ofxOfeliaLock ofxLock;
    client.getVariableByArgs(s, argc, argv);
}

void ofxOfeliaGet::dspMethod(t_signal **sp)
{
    const ofxOfeliaLock ofxLock;
    client.dspMethod(sp);
}

void ofxOfeliaGet::freeMethod()
{
    client.freeMethod();
}

void *ofxOfeliaGet::newWrapper(t_symbol *s, int argc, t_atom *argv)
{
    ofxOfeliaGet *x = reinterpret_cast<ofxOfeliaGet *>(pd_new(pdClass));
    new (x) ofxOfeliaGet();
    return x->newMethod(s, argc, argv);
}

void ofxOfeliaGet::bangWrapper(ofxOfeliaGet *x)
{
    x->bangMethod();
}

void ofxOfeliaGet::floatWrapper(ofxOfeliaGet *x, t_floatarg f)
{
    x->floatMethod(f);
}

void ofxOfeliaGet::symbolWrapper(ofxOfeliaGet *x, t_symbol *s)
{
    x->symbolMethod(s);
}

void ofxOfeliaGet::pointerWrapper(ofxOfeliaGet *x, t_gpointer *p)
{
    x->pointerMethod(p);
}

void ofxOfeliaGet::listWrapper(ofxOfeliaGet *x, t_symbol *s, int argc, t_atom *argv)
{
    x->listMethod(s, argc, argv);
}

void ofxOfeliaGet::anythingWrapper(ofxOfeliaGet *x, t_symbol *s, int argc, t_atom *argv)
{
    x->anythingMethod(s, argc, argv);
}

void ofxOfeliaGet::dspWrapper(ofxOfeliaGet *x, t_signal **sp)
{
    x->dspMethod(sp);
}

void ofxOfeliaGet::freeWrapper(ofxOfeliaGet *x)
{
    x->freeMethod();
    x->~ofxOfeliaGet();
}

void ofxOfeliaGet::setup()
{
    pdClass = class_new(gensym("ofelia get"),
                        reinterpret_cast<t_newmethod>(newWrapper),
                        reinterpret_cast<t_method>(freeWrapper),
                        sizeof(ofxOfeliaGet), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(pdClass, ofxOfeliaGet, client.data.signal.f);
    class_addbang(pdClass, bangWrapper);
    class_addfloat(pdClass, floatWrapper);
    class_addsymbol(pdClass, symbolWrapper);
    class_addpointer(pdClass, pointerWrapper);
    class_addlist(pdClass, listWrapper);
    class_addanything(pdClass, anythingWrapper);
    class_addmethod(pdClass, reinterpret_cast<t_method>(dspWrapper), gensym("dsp"), A_CANT, 0);
    class_sethelpsymbol(pdClass, gensym("ofelia-object"));
}
