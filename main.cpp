#include <iostream>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

const int defaultDelay = 1;

unsigned int *getActiveWindowsPID()
{
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
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
                kill(previous[i], SIGTERM);
            }
        }
        delete[] previous;
        previous = current;
    }

    return 0;
}