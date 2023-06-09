#pragma once
#include <CoreGraphics/CoreGraphics.h>

struct Bounds
{
    int width;
    int height;
};

struct Window
{
    pid_t processId;
    CGWindowID windowId;
    Bounds display;
    Bounds window;
    int length;
};