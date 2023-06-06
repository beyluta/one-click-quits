#include <iostream>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

const int defaultDelay = 1;

class Vector2D
{
public:
    int width;
    int height;
    int x;
    int y;
};

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

unsigned int *getActiveWindows()
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
    unsigned int *previous = getActiveWindows();
    while (1)
    {
        sleep(defaultDelay);
        unsigned int *current = getActiveWindows();
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

            if (!found && !isWindowMinimized(previous[i]) && getWindowCoordinates(previous[i]).width != getMonitorResolution(previous[i]).width)
            {
                kill(previous[i], SIGTERM);
            }
        }
        delete[] previous;
        previous = current;
    }

    return 0;
}