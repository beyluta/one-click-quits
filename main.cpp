#include <iostream>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

const int defaultDelay = 1;

unsigned int *getActiveWindowsPID()
{
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenBelowWindow, kCGNullWindowID);
    unsigned int *pids = new unsigned int[CFArrayGetCount(windows)];
    pids[0] = CFArrayGetCount(windows);

    for (CFIndex i = 1; i < CFArrayGetCount(windows); ++i)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        unsigned int pid;
        if (CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid))
        {
            pids[i] = pid;
        }
    }

    CFRelease(windows);
    return pids;
}

bool isWindowMinimized(unsigned int pid)
{
    AXUIElementRef appRef = AXUIElementCreateApplication(pid);
    CFTypeRef minimizedValue;
    bool isMinimized = false;

    if (AXUIElementCopyAttributeValue(appRef, kAXWindowsAttribute, &minimizedValue) == kAXErrorSuccess)
    {
        CFIndex windowCount = CFArrayGetCount(static_cast<CFArrayRef>(minimizedValue));
        isMinimized = windowCount > 0;
        CFRelease(minimizedValue);
    }

    CFRelease(appRef);
    return isMinimized;
}

int main()
{
    unsigned int *previous = getActiveWindowsPID();
    while (1)
    {
        sleep(defaultDelay);
        unsigned int *current = getActiveWindowsPID();
        for (int i = 1; i < previous[0]; ++i)
        {
            bool found = false;
            for (int j = 1; j < current[0]; ++j)
            {
                if (previous[i] == current[j])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (!isWindowMinimized(previous[i]))
                {
                    kill(previous[i], SIGTERM);
                }
            }
        }
        delete[] previous;
        previous = current;
    }

    return 0;
}