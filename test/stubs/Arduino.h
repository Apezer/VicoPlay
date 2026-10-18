#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
using std::max;
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define OUTPUT 4
#define WHITE 1
#define BLACK 0

inline uint32_t millis() {
    return 0;
}

inline void delay(int) {}

inline void pinMode(int, int) {}

inline int digitalRead(int) {
    return HIGH;
}

inline void digitalWrite(int, int) {}

inline void randomSeed(unsigned) {}

inline long random(long n) {
    return rand() % n;
}

template <class T> T constrain(T x, T a, T b) {
    return std::max(a, std::min(x, b));
}

struct SerialStub {
    void begin(int) {}

    void println(const char*) {}
};

extern SerialStub Serial;
