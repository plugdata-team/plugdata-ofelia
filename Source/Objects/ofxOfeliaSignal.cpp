#include "ofxOfeliaSignal.h"
#include "ofxOfeliaData.h"
#include "ofxOfeliaLua.h"

void ofxOfeliaSignal::addDsp(t_signal **sp)
{
    dataPtr->lua->doFunction(gensym("dsp"));
    const ofxOfeliaIO &io = dataPtr->io;
    int sum = io.numInlets + io.numOutlets;
    t_int **w = this->w;
    w[0] = reinterpret_cast<t_int *>(dataPtr);
    w[1] = reinterpret_cast<t_int *>(sp[0]->s_n);
    for (int i = 0; i < sum; i++)
        w[i + 2] = reinterpret_cast<t_int *>(sp[i]->s_vec);
    dsp_addv(perform, sum + 2, reinterpret_cast<t_int *>(w));
}


void ofxOfeliaSignal::receiveAudioSamples(std::vector<std::vector<float>>& samples)
{
    for(int i = 0; i < samples.size(); i++)
    {
        audioRingBuffer[i].push(samples[i]);
    }
}

t_int *ofxOfeliaSignal::perform(t_int *w)
{
    ofxOfeliaData *x = reinterpret_cast<ofxOfeliaData *>(w[1]);
    const int nSamples = static_cast<int>(w[2]);
    const int numInlets = x->io.numInlets;
    const int numOutlets = x->io.numOutlets;
    
    std::vector<std::vector<float>> input = std::vector<std::vector<float>>(numInlets, std::vector<float>(nSamples));
    
    for(int i = 0; i < numInlets; i++)
    {
        auto* in = reinterpret_cast<float*>(w[i + 3]);
        std::copy(in, in + nSamples, input[i].begin());
    }
    
    // send message to runner
    x->lua->messageManager->sendMessage(ofx_lua_audio_block, std::string(x->getUniqueSym()->s_name), input, numOutlets);
    
    for(int i = 0; i < numOutlets; i++)
    {
        auto outputSamples = x->signal.audioRingBuffer[i].pop(nSamples);
        std::copy(outputSamples.begin(), outputSamples.end(), reinterpret_cast<float*>(w[i + 3 + numInlets]));
    }
        
    return w + numInlets + numOutlets + 3;
}
