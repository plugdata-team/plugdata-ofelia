#pragma once
#include <iostream>
#include <string>
#include <cstring>


#define FAKE_PD_INTERFACE 1

using t_gpointer = int;
using t_canvas = std::string;
using t_garray = void;
using t_float = float;

// Normally this is a union but we don't need to care about that here
// Using a struct allows us to store std::strings, which is very convenient
struct t_word
{
    float w_float;
    std::string w_symbol;
    t_gpointer* w_gpointer;
    int w_index;
} ;

enum t_atomtype
{
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR,
    A_DOLLSYM,
    A_GIMME,
    A_CANT
};

struct t_atom
{
    t_atomtype a_type;
    t_word a_w;
};



#define MAXPDSTRING 12000

static const char* gensym(const std::string& sym)
{
    return sym.c_str();
}

static void freebytes(void *mem_to_free, size_t nbytes)
{
    free(mem_to_free);
}

static void* getbytes(size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = (void *)calloc(nbytes, 1);
    return (ret);
}

static void atom_string(const t_atom *a, char *buf, unsigned int bufsize)
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


