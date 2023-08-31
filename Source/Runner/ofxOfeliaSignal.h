#pragma once

#include <vector>


class ofxOfeliaLua;
class ofxOfeliaSignal
{
public:
    ofxOfeliaSignal(ofxOfeliaLua *lua)
    : x(lua){};
    
    void perform(std::vector<std::vector<float>> input, int numOutlets);
    
private:
    
    ofxOfeliaLua *x;
};
