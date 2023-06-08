#include <iostream>
#include <unistd.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h>
#include "header/hashset.h"

const int m_delay = 1;
Hashset m_windows;

bool isWindowOnScreen(Window window)
{
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    for (CFIndex i = 0; i < CFArrayGetCount(windowList); i++)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;
        CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid);

        if (pid == window.processId)
        {
            CFRelease(windowList);
            return true;
        }
    }

    CFRelease(windowList);
    return false;
}

bool isWindowValid(Window window)
{
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    for (CFIndex i = 0; i < CFArrayGetCount(windowList); i++)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;
        CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid);
        CFNumberRef widNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
        pid_t wid;
        CFNumberGetValue(widNumber, kCFNumberSInt32Type, &wid);

        if (pid == window.processId && wid == window.windowId)
        {
            CFRelease(windowList);
            return true;
        }
    }

    CFRelease(windowList);
    return false;
}

bool isWindowMinimized(Window window)
{
    AXUIElementRef appRef = AXUIElementCreateApplication(window.processId);
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

Hashset getAllWindows()
{
    Hashset newSet;
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    pid_t *pids = new pid_t[CFArrayGetCount(windows)];
    pids[0] = CFArrayGetCount(windows);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); ++i)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;

        CFNumberRef widNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
        pid_t wid;
        CFNumberGetValue(widNumber, kCFNumberSInt32Type, &wid);

        if (CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid))
        {
            Window window;
            window.processId = pid;
            window.windowId = wid;
            newSet.add(window);
        }
    }

    CFRelease(windows);
    return newSet;
}

int main()
{
    while (1)
    {
        m_windows = getAllWindows();
        sleep(m_delay);
        Window *windows = m_windows.toArray();
        for (int i = 0; i < windows[0].length - 1; i++)
        {
            if (!isWindowValid(windows[i]) &&
                !isWindowOnScreen(windows[i]) &&
                !isWindowMinimized(windows[i]) &&
                windows[i].processId > 0 &&
                windows[i].windowId > 0)
            {
                kill(windows[i].processId, SIGTERM);
            }
        }
        delete[] windows;
    }

    return 0;
}