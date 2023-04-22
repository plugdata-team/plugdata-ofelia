#include "ofxOfeliaDefine.h"
#include "ofxOfeliaAsync.h"
#include <new>
#include <cstring>
#include <vector>

t_class *ofxOfeliaDefine::pdClass;

void *ofxOfeliaDefine::newMethod(t_symbol *s, int argc, t_atom *argv)
{
    const ofxOfeliaLock ofxLock;
    const ofxOfeliaAudioLock audioLock;
    
    t_symbol *asym = gensym("#A");
    data.argParse(s, argc, argv, true);
    /* bashily unbind #A -- this would create garbage if #A were
     multiply bound but we believe in this context it's at most
     bound to whichever ofelia_define or array was created most recently */
    asym->s_thing = 0;
    /* and now bind #A to us to receive following messages in the
     saved file or copy buffer */
    pd_bind(&data.ob.ob_pd, asym);
    return this;
}

void ofxOfeliaDefine::bangMethod()
{
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this]() {
        if(!_this) return;
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

void ofxOfeliaDefine::floatMethod(t_floatarg f)
{
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, f]() {
        if(!_this) return;
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

void ofxOfeliaDefine::symbolMethod(t_symbol *s)
{
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, s]() {
        if(!_this) return;
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

void ofxOfeliaDefine::pointerMethod(t_gpointer *p)
{
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, p]() {
        if(!_this) return;
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

void ofxOfeliaDefine::listMethod(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, args]() mutable {
        if(!_this) return;
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

void ofxOfeliaDefine::anythingMethod(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, s, args]() mutable {
        if(!_this) return;
        data.lua.setVariableByArgs(s, args.size(), args.data());
    });
}

void ofxOfeliaDefine::clearMethod()
{
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this]() {
        if (!_this || data.isDirectMode) return;
        data.lua.doFreeFunction();
        binbuf_clear(data.binbuf);
        data.textBuf.senditup();
    });
}

void ofxOfeliaDefine::setMethod(t_symbol *s, int argc, t_atom *argv)
{
    auto args = std::vector<t_atom>(argv, argv + argc);
    ofxOfeliaAsync::callAsync([_this = WeakReference<ofxOfeliaDefine>(this), this, args]() mutable {
        if (!_this || data.isDirectMode) return;
        binbuf_restore(data.binbuf, args.size(), args.data());
        data.textBuf.senditup();
        data.lua.doText();
    });
}

void ofxOfeliaDefine::saveMethod(t_object *ob, t_binbuf *bb)
{
    // No need to lock here, because the host should make sure this is thread safe
    binbuf_addv(bb, const_cast<char *>("ssff"), &s__X, gensym("obj"),
                static_cast<float>(ob->te_xpix),
                static_cast<float>(ob->te_ypix));
    binbuf_addbinbuf(bb, ob->ob_binbuf);
    binbuf_addsemi(bb);
    if (data.shouldKeep)
    {
        binbuf_addv(bb, const_cast<char *>("ss"), gensym("#A"), gensym("__set"));
        binbuf_addbinbuf(bb, data.binbuf);
        binbuf_addsemi(bb);
    }
    obj_saveformat(ob, bb);
}

void ofxOfeliaDefine::dspMethod(t_signal **sp)
{
    const ofxOfeliaLock ofxLock;
    
    if (!data.isSignalObject) return;
    data.signal.addDsp(sp);
}

/* notification from GUI that we've been updated */
void ofxOfeliaDefine::notifyMethod()
{
    const ofxOfeliaLock ofxLock;
    
    if (data.isDirectMode) return;
    data.textBuf.senditup();
    data.lua.doText();
}

void ofxOfeliaDefine::freeMethod()
{
    const ofxOfeliaLock ofxLock;
    
    data.lua.doFreeFunction();
    data.textBuf.free();
    data.unbindSym();
    if (!data.isSignalObject) data.io.freeControlIO();
    else data.io.freeSignalIO();
}

void *ofxOfeliaDefine::newWrapper(t_symbol *s, int argc, t_atom *argv)
{
    ofxOfeliaDefine *x = reinterpret_cast<ofxOfeliaDefine *>(pd_new(pdClass));
    new (x) ofxOfeliaDefine();
    return x->newMethod(s, argc, argv);
}

void ofxOfeliaDefine::bangWrapper(ofxOfeliaDefine *x)
{
    x->bangMethod();
}

void ofxOfeliaDefine::floatWrapper(ofxOfeliaDefine *x, t_floatarg f)
{
    x->floatMethod(f);
}

void ofxOfeliaDefine::symbolWrapper(ofxOfeliaDefine *x, t_symbol *s)
{
    x->symbolMethod(s);
}

void ofxOfeliaDefine::pointerWrapper(ofxOfeliaDefine *x, t_gpointer *p)
{
    x->pointerMethod(p);
}

void ofxOfeliaDefine::listWrapper(ofxOfeliaDefine *x, t_symbol *s, int argc, t_atom *argv)
{
    x->listMethod(s, argc, argv);
}

void ofxOfeliaDefine::anythingWrapper(ofxOfeliaDefine *x, t_symbol *s, int argc, t_atom *argv)
{
    x->anythingMethod(s, argc, argv);
}

void ofxOfeliaDefine::clearWrapper(ofxOfeliaDefine *x)
{
    x->clearMethod();
}

void ofxOfeliaDefine::setWrapper(ofxOfeliaDefine *x, t_symbol *s, int argc, t_atom *argv)
{
    x->setMethod(s, argc, argv);
}

void ofxOfeliaDefine::saveWrapper(t_gobj *z, t_binbuf *bb)
{
    ofxOfeliaDefine *x = reinterpret_cast<ofxOfeliaDefine *>(z);
    x->saveMethod(&x->data.ob, bb);
}

void ofxOfeliaDefine::dspWrapper(ofxOfeliaDefine *x, t_signal **sp)
{
    x->dspMethod(sp);
}

void ofxOfeliaDefine::notifyWrapper(ofxOfeliaDefine *x)
{
    x->notifyMethod();
}

void ofxOfeliaDefine::openWrapper(ofxOfeliaData *x)
{
    x->textBuf.openMethod();
}

void ofxOfeliaDefine::closeWrapper(ofxOfeliaData *x)
{
    x->textBuf.closeMethod();
}

void ofxOfeliaDefine::addLineWrapper(ofxOfeliaData *x, t_symbol *s, int argc, t_atom *argv)
{
    x->textBuf.addLineMethod(s, argc, argv);
}

void ofxOfeliaDefine::readWrapper(ofxOfeliaData *x, t_symbol *s, int argc, t_atom *argv)
{
    x->textBuf.readMethod(s, argc, argv);
}

void ofxOfeliaDefine::writeWrapper(ofxOfeliaData *x, t_symbol *s, int argc, t_atom *argv)
{
    x->textBuf.writeMethod(s, argc, argv);
}

void ofxOfeliaDefine::freeWrapper(ofxOfeliaDefine *x)
{
    x->freeMethod();
    x->~ofxOfeliaDefine();
}

void ofxOfeliaDefine::setup()
{
    pdClass = class_new(gensym("ofelia define"),
                        reinterpret_cast<t_newmethod>(newWrapper),
                        reinterpret_cast<t_method>(freeWrapper),
                        sizeof(ofxOfeliaDefine), 0, A_GIMME, 0);
    class_setsavefn(pdClass, saveWrapper);
    CLASS_MAINSIGNALIN(pdClass, ofxOfeliaDefine, data.signal.f);
    class_addbang(pdClass, bangWrapper);
    class_addfloat(pdClass, floatWrapper);
    class_addsymbol(pdClass, symbolWrapper);
    class_addpointer(pdClass, pointerWrapper);
    class_addlist(pdClass, listWrapper);
    class_addanything(pdClass, anythingWrapper);
    class_addmethod(pdClass, reinterpret_cast<t_method>(openWrapper),
                    gensym("click"), A_NULL, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(closeWrapper),
                    gensym("__close"), A_NULL, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(addLineWrapper),
                    gensym("__addline"), A_GIMME, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(dspWrapper),
                    gensym("dsp"), A_CANT, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(notifyWrapper),
                    gensym("__notify"), A_NULL, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(setWrapper),
                    gensym("__set"), A_GIMME, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(clearWrapper),
                    gensym("__clear"), A_NULL, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(writeWrapper),
                    gensym("write"), A_GIMME, 0);
    class_addmethod(pdClass, reinterpret_cast<t_method>(readWrapper),
                    gensym("read"), A_GIMME, 0);
    class_sethelpsymbol(pdClass, gensym("ofelia-object"));
}
