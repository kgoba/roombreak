#pragma once
#include <stdint.h>

#include "util.h"

#define FIFO(name, type, size) \
                               \
static struct {                \
  type buffer[size];           \
  byte head;                   \
  byte count;                  \
} name;

#define FIFO_INIT(name)     name.head = name.count = 0
#define FIFO_SIZE(name)     ARRAY_SIZE(name.buffer)
#define FIFO_EMPTY(name)    (name.count == 0)
#define FIFO_FULL(name)     (name.count == FIFO_SIZE(name))
#define FIFO_COUNT(name)    (name.count)

#define FIFO_PUSH(name, b)     \
if (!FIFO_FULL(name)) {         \
  byte tail = name.head + name.count;   \
  if (tail >= FIFO_SIZE(name)) tail -= FIFO_SIZE(name);   \
  name.buffer[tail] = b;    \
  name.count++;             \
}

#define FIFO_HEAD(name)      ((name.count > 0) ? name.buffer[name.head] : 0)
#define FIFO_POP(name)      \
if (name.count > 0) {       \
  name.head++;              \
  if (name.head == FIFO_SIZE(name)) name.head = 0;      \
  name.count--;             \
}


/*
template<typename T, byte N>
struct FIFO {
    T buffer[N];
    byte head;
    volatile byte count;
    
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
    
    bool full() {
        return (count == N);
    }
    
    byte available() const {
        return count;
    }
};
*/