#pragma once

#include "m_pd.h"
#include <vector>

template <typename T, size_t Size>
class RingBuffer {
public:
    RingBuffer() : buffer_(Size), head_(0), tail_(0), count_(0) {}

    bool isEmpty() const {
        return count_ == 0;
    }

    bool isFull() const {
        return count_ == Size;
    }

    void push(const std::vector<T>& values) {
        if (values.size() > Size - count_) {
            return;
        }

        for (const T& value : values) {
            buffer_[head_] = value;
            head_ = (head_ + 1) % Size;
            ++count_;
        }
    }

    std::vector<T> pop(size_t numItems) {
        if (numItems > count_) {
            numItems = count_;
        }

        std::vector<T> poppedItems;
        poppedItems.reserve(numItems);

        for (size_t i = 0; i < numItems; ++i) {
            poppedItems.push_back(buffer_[tail_]);
            tail_ = (tail_ + 1) % Size;
            --count_;
        }

        return poppedItems;
    }

private:
    std::vector<T> buffer_;
    size_t head_;
    size_t tail_;
    size_t count_;
};

class ofxOfeliaData;
class ofxOfeliaSignal
{
public:
    ofxOfeliaSignal(ofxOfeliaData *dataPtr)
    :dataPtr(dataPtr){};
    void addDsp(t_signal **sp);
    static t_int *perform(t_int *w);
    
    void receiveAudioSamples(std::vector<std::vector<float>>& samples);
    
    t_int **w;
    t_float f; // variable for main signal inlet
        
private:
    ofxOfeliaData *dataPtr; // pointer to data
    RingBuffer<float, 4096> audioRingBuffer[8];
};
