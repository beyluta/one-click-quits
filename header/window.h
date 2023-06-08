#pragma once
#include <CoreGraphics/CoreGraphics.h>

struct Window
{
    pid_t processId;
    CGWindowID windowId;
    int length;
};