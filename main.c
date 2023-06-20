#include <stdio.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h>

#define MAX_PROC_COUNT 99999

struct ProcInfo
{
    char name[256];
    pid_t pid;
    pid_t wid;
};

struct ProcInfo *getProcs()
{
    struct ProcInfo *procs = malloc(sizeof(struct ProcInfo) * MAX_PROC_COUNT);
    const CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements, kCGNullWindowID);

    for (CFIndex i = 0; i < CFArrayGetCount(windows); ++i)
    {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
        CFStringRef nameRef = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerName);
        CFStringGetCString(nameRef, procs[i].name, 256, kCFStringEncodingUTF8);

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
            procs[i].pid = pid;
            procs[i].wid = wid;
        }
    }

    CFRelease(windows);
    return procs;
}

int getProcInstanceCount(struct ProcInfo proc)
{
    int windowCount = 0;
    AXUIElementRef appRef = AXUIElementCreateApplication(proc.pid);
    CFArrayRef windowList;
    AXError result = AXUIElementCopyAttributeValues(appRef, kAXWindowsAttribute, 0, 9999, &windowList);

    if (result == kAXErrorSuccess)
    {
        windowCount = CFArrayGetCount(windowList);
        CFRelease(windowList);
    }

    CFRelease(appRef);
    return windowCount;
}

int main(int argc, const char *argv[])
{
    struct ProcInfo *o_proc_table = getProcs();
    const int maxTimerCount = 2;
    int timerCount = 0;

    while (1)
    {
        if (timerCount >= maxTimerCount)
        {
            struct ProcInfo *c_proc_table = getProcs();
            for (int i = 0; i < MAX_PROC_COUNT; i++)
            {
                if (o_proc_table[i].pid < 100)
                {
                    continue;
                }

                bool isProcessAlive = false;

                for (int j = 0; j < MAX_PROC_COUNT; j++)
                {
                    if (
                        c_proc_table[j].pid == o_proc_table[i].pid &&
                        c_proc_table[j].wid == o_proc_table[i].wid &&
                        strcmp(c_proc_table[j].name, o_proc_table[i].name) == 0)
                    {
                        isProcessAlive = true;
                        break;
                    }
                }

                if (!isProcessAlive && getProcInstanceCount(o_proc_table[i]) <= 0)
                {
                    kill(o_proc_table[i].pid, SIGTERM);
                }
            }

            free(o_proc_table);
            o_proc_table = c_proc_table;
            timerCount = 0;
            continue;
        }

        timerCount++;
        sleep(1);
    }

    return 0;
}