#pragma once

struct Preferences {
    void begin(const char*, bool) {}

    int getInt(const char*, int n) {
        return n;
    }

    void putInt(const char*, int) {}
};
