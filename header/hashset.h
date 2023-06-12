#pragma once
#include <iostream>
#include "../header/window.h"

#define MAX_PID 99999

class Hashset
{
private:
    Window *data;
    Window *indices;
    int length = 0;

public:
    Hashset();
    ~Hashset();
    void add(Window value);
    bool contains(Window value);
    Window getWindowByPID(pid_t processId);
    Window *toArray();
};