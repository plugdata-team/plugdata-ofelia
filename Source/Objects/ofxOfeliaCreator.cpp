#include "ofxOfeliaCreator.h"
#include "ofxOfeliaLua.h"
#include "ofxOfeliaDefine.h"
#include "ofxOfeliaGet.h"
#include "ofxOfeliaSet.h"
#include "ofxOfeliaExists.h"
#include <z_libpd.h>
#include <cstring>


void *ofxOfeliaCreator::newWrapper(t_symbol *s, int argc, t_atom *argv)
{
    auto* pdthis = libpd_this_instance();
    
    if (!argc || argv[0].a_type != A_SYMBOL)
        pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaDefine::newWrapper(s, argc, argv));
    else
    {
        const char *str = argv[0].a_w.w_symbol->s_name;
        if (!std::strcmp(str, "d") || !std::strcmp(str, "define"))
            pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaDefine::newWrapper(gensym("define"), argc - 1, argv + 1));
        else if (!std::strcmp(str, "f") || !std::strcmp(str, "function"))
            pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaFunction::newWrapper(gensym("function"), argc - 1, argv + 1));
        else if (!std::strcmp(str, "get"))
            pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaGet::newWrapper(gensym("get"), argc - 1, argv + 1));
        else if (!std::strcmp(str, "set"))
            pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaSet::newWrapper(gensym("set"), argc - 1, argv + 1));
        else if (!std::strcmp(str, "e") || !std::strcmp(str, "exists"))
            pdthis->pd_newest = static_cast<t_pd *>(ofxOfeliaExists::newWrapper(gensym("exists"), argc - 1, argv + 1));
        else
        {
            pd_error(NULL, "list %s: unknown function", str);
            pdthis->pd_newest = 0;
        }
    }
    return pdthis->pd_newest;
}

void ofxOfeliaCreator::setup()
{
    class_addcreator(reinterpret_cast<t_newmethod>(newWrapper), gensym("ofelia"), A_GIMME, 0);
}
