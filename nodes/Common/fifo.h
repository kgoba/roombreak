#pragma once
#include <stdint.h>

#include "util.h"

template<typename T, byte N>
struct FIFO {
    T buffer[N];
    byte head;
    byte count;
    
    FIFO() : head(0), count(0) { }
    
    bool push(T b) {
        if (count == N) return false;
        byte tail = head + count;
        if (tail >= N) tail -= N;
        buffer[tail] = b;
        count++;
        return true;
    }
    
    const T pop() {
        if (count == 0) return 0;
        T b = buffer[head];
        head++;
        if (head == N) head = 0;
        count--;
        return b;
    }
    
    bool empty() const {
        return (count == 0);
    }
    
    bool full() const {
        return (count == N);
    }
    
    byte available() const {
        return count;
    }
};
