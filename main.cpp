#include <iostream>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include "header/hashset.h"
#include "header/vector2d.h"

const int defaultDelay = 1;
Hashset previous;
Hashset current;

Vector2D getWindowCoordinates(unsigned int pid)
{
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        unsigned int windowPid;
        if (CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &windowPid))
        {
            if (windowPid == pid)
            {
                Vector2D vector;
                CGRect bounds;
                CFDictionaryRef boundsRef = (CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds);
                CGRectMakeWithDictionaryRepresentation(boundsRef, &bounds);
                vector.width = bounds.size.width;
                vector.height = bounds.size.height;
                vector.x = bounds.origin.x;
                vector.y = bounds.origin.y;
                return vector;
            }
        }
    }
    return Vector2D();
}

Vector2D getMonitorResolution(unsigned int pid)
{
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef ownerPIDRef = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);

        pid_t ownerPID;
        if (CFNumberGetValue(ownerPIDRef, kCFNumberSInt32Type, &ownerPID) && ownerPID == pid)
        {
            CGRect windowBounds;
            CFDictionaryRef boundsRef = (CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds);
            CGRectMakeWithDictionaryRepresentation(boundsRef, &windowBounds);

            int monitorNumber = 0;
            CGDirectDisplayID displayIDs[10];
            uint32_t displayCount;
            CGGetDisplaysWithPoint(windowBounds.origin, 10, displayIDs, &displayCount);

            for (uint32_t j = 0; j < displayCount; j++)
            {
                if (CGDisplayIsActive(displayIDs[j]))
                {
                    CGRect displayBounds = CGDisplayBounds(displayIDs[j]);
                    Vector2D vector;
                    vector.width = displayBounds.size.width;
                    vector.height = displayBounds.size.height;
                    return vector;
                }
            }
        }
    }

    CFRelease(windows);
    return Vector2D();
}

Hashset getActiveWindows()
{
    Hashset newSet;
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenBelowWindow, kCGNullWindowID);
    unsigned int *pids = new unsigned int[CFArrayGetCount(windows)];
    pids[0] = CFArrayGetCount(windows);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); ++i)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        unsigned int pid;
        if (CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid))
        {
            newSet.add(pid);
        }
    }

    CFRelease(windows);
    return newSet;
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
    previous = getActiveWindows();
    while (1)
    {
        sleep(defaultDelay);
        current = getActiveWindows();
        int *previousArray = previous.toArray();
        for (int i = 1; i < previousArray[0]; i++)
        {
            if (!current.contains(previousArray[i]))
            {
                unsigned int PID = previousArray[i];
                Vector2D window = getWindowCoordinates(PID);
                Vector2D monitor = getMonitorResolution(PID);

                if (window.width != monitor.width && window.height != monitor.height && !isWindowMinimized(PID))
                {
                    kill(PID, SIGTERM);
                }
            }
        }
        delete[] previousArray;
        previous = current;
    }

    return 0;
}