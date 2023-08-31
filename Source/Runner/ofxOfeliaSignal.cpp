#include "ofxOfeliaSignal.h"
#include "ofxOfeliaLua.h"
#include "ofxOfeliaMessageManager.h"


void ofxOfeliaSignal::perform(std::vector<std::vector<float>> input, int numOutlets)
{
    lua_State *L = x->L;
    
    int numInlets = input.size();
    int nSamples = input.empty() ? 0 : input[0].size();
    
    auto output = std::vector<std::vector<float>>(numOutlets, std::vector<float>(nSamples));
    
    int top;
    if (x->isFunction(gensym("perform"), top))
    {
        for (int i = 0; i < numInlets; ++i)
        {
            lua_newtable(L);
            auto& in = input[i];
            for (int j = 0; j < nSamples; ++j)
            {
                lua_pushinteger(L, static_cast<lua_Integer>(j + 1));
                lua_pushnumber(L, static_cast<lua_Number>(in[j]));
                lua_settable(L, -3);
            }
        }
        if (lua_pcall(L, numInlets, numOutlets, 0))
        {
            error("ofelia: %s", lua_tostring(L, -1));
            lua_pop(L, 2);
            goto error;
        }
        if (!lua_istable(L, -1))
        {
            const char *s = "ofelia: 'perform' function should return";
            if (numOutlets == 1)
                error("%s %s", s, "a table");
            else if (numOutlets > 1)
                error("%s %d %s", s, numOutlets, "tables");
            lua_pop(L, 1 + numOutlets);
            goto error;
        }
        for (int i = numOutlets - 1; i >= 0; --i)
        {
            auto& out = output[i];
            for (int j = 0; j < nSamples; ++j)
            {
                lua_pushinteger(L, static_cast<lua_Integer>(j + 1));
                lua_gettable(L, -2);
                if (lua_isnumber(L, -1))
                    out[j] = static_cast<t_float>(lua_tonumber(L, -1));
                else if (lua_isboolean(L, -1))
                    out[j] = static_cast<t_float>(lua_toboolean(L, -1));
                else
                    out[j] = 0.0f;
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    else
    {
    error: // silence the audio if something is wrong
        for (int i = 0; i < numOutlets; ++i)
        {
            auto& out = output[i];
            for (int j = 0; j < nSamples; ++j) out[j] = 0;
        }
    }
    
    x->messageManager->sendMessage(pd_audio_block, x->getUniqueId(), output);
}
