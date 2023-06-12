#include <iostream>
#include <unistd.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include "header/hashset.h"

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

void updateDisplayBounds(Window &window)
{
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef ownerPIDRef = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);

        pid_t ownerPID;
        if (CFNumberGetValue(ownerPIDRef, kCFNumberSInt32Type, &ownerPID) && ownerPID == window.processId)
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
                    window.display.width = displayBounds.size.width;
                    window.display.height = displayBounds.size.height;
                }
            }
        }
    }

    CFRelease(windows);
}

Hashset getAllWindows()
{
    Hashset set;
    const CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements, kCGNullWindowID);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); ++i)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFNumberRef pidNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);
        pid_t pid;

        CFNumberRef widNumber = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
        pid_t wid;
        CFNumberGetValue(widNumber, kCFNumberSInt32Type, &wid);

        CGRect bounds;
        CFDictionaryRef boundsRef = (CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds);
        CGRectMakeWithDictionaryRepresentation(boundsRef, &bounds);

        if (CFNumberGetValue(pidNumber, kCFNumberSInt32Type, &pid))
        {
            Window window;
            window.processId = pid;
            window.windowId = wid;
            window.window.width = bounds.size.width;
            window.window.height = bounds.size.height;
            updateDisplayBounds(window);
            set.add(window);
        }
    }

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

void onWindowClose(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *context)
{
    pid_t pid;
    AXUIElementGetPid(element, &pid);
    kill(pid, SIGTERM);
}

int main()
{
    m_windows = getAllWindows();
    const Window *windowArray = m_windows.toArray();
    for (int i = 0; i < windowArray[0].length; i++)
    {
        AXUIElementRef program = AXUIElementCreateApplication(windowArray[i].processId);
        AXObserverRef observer;
        AXObserverCreate(windowArray[i].processId, onWindowClose, &observer);
        CFStringRef notification = kAXUIElementDestroyedNotification;
        AXObserverAddNotification(observer, program, notification, NULL);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(observer), kCFRunLoopDefaultMode);
    }
    delete[] windowArray;
    CFRunLoopRun();

    // m_windows = getAllWindows();
    // const Window *windows = m_windows.toArray();
    // for (int i = 0; i < windows[0].length - 1; i++)
    // {
    //     AXUIElementRef program = AXUIElementCreateApplication(windows[i].processId);
    //     AXObserverRef observer;
    //     AXObserverCreate(windows[i].processId, onWindowClose, &observer);
    //     CFStringRef notification = kAXUIElementDestroyedNotification;
    //     AXObserverAddNotification(observer, program, notification, NULL);
    //     CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(observer), kCFRunLoopDefaultMode);
    //     CFRunLoopRun();
    // }
    // return 0;
}

// #include <iostream>
// #include <ApplicationServices/ApplicationServices.h>

// void WindowCloseCallback(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void* context) {
//     std::cout << "A window was closed!" << std::endl;
// }

// int main() {
//     // Create an application reference for Safari
//     AXUIElementRef safariApp = AXUIElementCreateApplication(8115); // Process ID of Safari

//     // Create an observer for window close events
//     AXObserverRef observer;
//     AXObserverCreate(8115, WindowCloseCallback, &observer);

//     // Specify the event to observe
//     CFStringRef notification = kAXUIElementDestroyedNotification;
//     AXObserverAddNotification(observer, safariApp, notification, NULL);

//     // Run the main event loop
//     CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(observer), kCFRunLoopDefaultMode);
//     CFRunLoopRun();

//     return 0;
// }
