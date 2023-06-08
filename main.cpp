#include <iostream>
#include <unistd.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h>
#include "header/hashset.h"

const int m_delay = 1;
Hashset m_windows;

bool isWindowOnScreen(const Window window)
{
    const CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++)
    {
        const CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        const CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;
        CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid);

        if (pid == window.processId)
        {
            CFRelease(windows);
            return true;
        }
    }

    CFRelease(windows);
    return false;
}

bool isWindowValid(const Window window)
{
    const CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++)
    {
        const CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        const CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;
        CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid);
        const CFNumberRef widNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
        pid_t wid;
        CFNumberGetValue(widNumber, kCFNumberSInt32Type, &wid);

        if (pid == window.processId && wid == window.windowId)
        {
            CFRelease(windows);
            return true;
        }
    }

    CFRelease(windows);
    return false;
}

bool isWindowMinimized(const Window window)
{
    const AXUIElementRef appRef = AXUIElementCreateApplication(window.processId);
    CFTypeRef minimizedValue;
    bool isMinimized = false;

    if (AXUIElementCopyAttributeValue(appRef, kAXWindowsAttribute, &minimizedValue) == kAXErrorSuccess)
    {
        const CFIndex windowCount = CFArrayGetCount(static_cast<CFArrayRef>(minimizedValue));
        isMinimized = windowCount > 0;
        CFRelease(minimizedValue);
    }

    CFRelease(appRef);
    return isMinimized;
}

Hashset getAllWindows()
{
    Hashset set;
    const CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
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
            set.add(window);
        }
    }

    delete[] pids;
    CFRelease(windows);
    return set;
}

bool isBackgroundWindow(const Window window)
{
    ProcessSerialNumber psn = {0, kNoProcess};
    while (GetNextProcess(&psn) == noErr)
    {
        pid_t processPID;
        if (GetProcessPID(&psn, &processPID) == noErr && processPID == window.processId)
        {
            ProcessInfoRec processInfo;
            memset(&processInfo, 0, sizeof(ProcessInfoRec));
            processInfo.processInfoLength = sizeof(ProcessInfoRec);
            if (GetProcessInformation(&psn, &processInfo) == noErr)
            {
                if (processInfo.processMode & modeOnlyBackground)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

int main()
{
    while (1)
    {
        m_windows = getAllWindows();
        sleep(m_delay);
        const Window *windows = m_windows.toArray();
        for (int i = 0; i < windows[0].length - 1; i++)
        {
            if (!isWindowValid(windows[i]) &&
                !isWindowOnScreen(windows[i]) &&
                !isWindowMinimized(windows[i]) &&
                !isBackgroundWindow(windows[i]) &&
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