#include "StartMenuMonitorState.h"

static SRWLOCK g_startMenuTargetMonitorLock = SRWLOCK_INIT;
static START_MENU_MONITOR_REQUEST_ID g_nextStartMenuTargetMonitorRequestId = 0;
static START_MENU_MONITOR_REQUEST_ID g_latestStartedStartMenuTargetMonitorRequestId = 0;
static START_MENU_MONITOR_REQUEST_ID g_pendingStartMenuTargetMonitorRequestId = 0;
static HMONITOR g_pendingStartMenuTargetMonitor = NULL;
static BOOL g_pendingStartMenuTargetMonitorConfirmed = FALSE;
static START_MENU_MONITOR_REQUEST_ID g_activeStartMenuTargetMonitorLifecycleId = 0;
static HMONITOR g_activeStartMenuTargetMonitor = NULL;

static START_MENU_MONITOR_REQUEST_ID AllocateStartMenuTargetMonitorRequestId(void)
{
    do
    {
        ++g_nextStartMenuTargetMonitorRequestId;
    } while (!g_nextStartMenuTargetMonitorRequestId);
    return g_nextStartMenuTargetMonitorRequestId;
}

START_MENU_MONITOR_REQUEST_ID BeginStartMenuTargetMonitorRequest(void)
{
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    START_MENU_MONITOR_REQUEST_ID requestId = AllocateStartMenuTargetMonitorRequestId();
    g_latestStartedStartMenuTargetMonitorRequestId = requestId;
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return requestId;
}

BOOL PublishStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId, HMONITOR monitor)
{
    BOOL published = FALSE;
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (requestId && requestId == g_latestStartedStartMenuTargetMonitorRequestId)
    {
        g_pendingStartMenuTargetMonitorRequestId = requestId;
        g_pendingStartMenuTargetMonitor = monitor;
        g_pendingStartMenuTargetMonitorConfirmed = FALSE;
        published = TRUE;
    }
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return published;
}

BOOL ConfirmStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId)
{
    BOOL confirmed = FALSE;
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (requestId && requestId == g_activeStartMenuTargetMonitorLifecycleId)
    {
        confirmed = TRUE;
    }
    else if (requestId && requestId == g_pendingStartMenuTargetMonitorRequestId)
    {
        g_pendingStartMenuTargetMonitorConfirmed = TRUE;
        if (!g_activeStartMenuTargetMonitorLifecycleId)
        {
            g_activeStartMenuTargetMonitorLifecycleId = g_pendingStartMenuTargetMonitorRequestId;
            g_activeStartMenuTargetMonitor = g_pendingStartMenuTargetMonitor;
            g_pendingStartMenuTargetMonitorRequestId = 0;
            g_pendingStartMenuTargetMonitor = NULL;
            g_pendingStartMenuTargetMonitorConfirmed = FALSE;
        }
        confirmed = TRUE;
    }
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return confirmed;
}

BOOL CancelStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId)
{
    BOOL cancelled = FALSE;
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (requestId && requestId == g_pendingStartMenuTargetMonitorRequestId)
    {
        g_pendingStartMenuTargetMonitorRequestId = 0;
        g_pendingStartMenuTargetMonitor = NULL;
        g_pendingStartMenuTargetMonitorConfirmed = FALSE;
        cancelled = TRUE;
    }
    if (requestId && requestId == g_activeStartMenuTargetMonitorLifecycleId)
    {
        g_activeStartMenuTargetMonitorLifecycleId = 0;
        g_activeStartMenuTargetMonitor = NULL;
        cancelled = TRUE;
    }
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return cancelled;
}

START_MENU_MONITOR_REQUEST_ID AcquireStartMenuTargetMonitor(HMONITOR defaultMonitor, HMONITOR* monitor)
{
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (!g_activeStartMenuTargetMonitorLifecycleId)
    {
        if (g_pendingStartMenuTargetMonitorRequestId)
        {
            g_activeStartMenuTargetMonitorLifecycleId = g_pendingStartMenuTargetMonitorRequestId;
            g_activeStartMenuTargetMonitor = g_pendingStartMenuTargetMonitor
                ? g_pendingStartMenuTargetMonitor
                : defaultMonitor;
            g_pendingStartMenuTargetMonitorRequestId = 0;
            g_pendingStartMenuTargetMonitor = NULL;
            g_pendingStartMenuTargetMonitorConfirmed = FALSE;
        }
        else
        {
            g_activeStartMenuTargetMonitorLifecycleId = AllocateStartMenuTargetMonitorRequestId();
            g_activeStartMenuTargetMonitor = defaultMonitor;
        }
    }
    START_MENU_MONITOR_REQUEST_ID lifecycleId = g_activeStartMenuTargetMonitorLifecycleId;
    if (!g_activeStartMenuTargetMonitor)
    {
        g_activeStartMenuTargetMonitor = defaultMonitor;
    }
    *monitor = g_activeStartMenuTargetMonitor;
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return lifecycleId;
}

BOOL UpdateStartMenuTargetMonitor(START_MENU_MONITOR_REQUEST_ID lifecycleId, HMONITOR monitor)
{
    BOOL updated = FALSE;
    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (lifecycleId && lifecycleId == g_activeStartMenuTargetMonitorLifecycleId)
    {
        g_activeStartMenuTargetMonitor = monitor;
        updated = TRUE;
    }
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
    return updated;
}

START_MENU_MONITOR_REQUEST_ID GetStartMenuTargetMonitorLifecycleId(void)
{
    AcquireSRWLockShared(&g_startMenuTargetMonitorLock);
    START_MENU_MONITOR_REQUEST_ID lifecycleId = g_activeStartMenuTargetMonitorLifecycleId;
    if (!lifecycleId && g_pendingStartMenuTargetMonitorConfirmed)
    {
        lifecycleId = g_pendingStartMenuTargetMonitorRequestId;
    }
    ReleaseSRWLockShared(&g_startMenuTargetMonitorLock);
    return lifecycleId;
}

void CompleteStartMenuTargetMonitorLifecycle(START_MENU_MONITOR_REQUEST_ID lifecycleId)
{
    if (!lifecycleId)
    {
        return;
    }

    AcquireSRWLockExclusive(&g_startMenuTargetMonitorLock);
    if (lifecycleId == g_activeStartMenuTargetMonitorLifecycleId)
    {
        g_activeStartMenuTargetMonitorLifecycleId = 0;
        g_activeStartMenuTargetMonitor = NULL;
    }
    if (lifecycleId == g_pendingStartMenuTargetMonitorRequestId)
    {
        g_pendingStartMenuTargetMonitorRequestId = 0;
        g_pendingStartMenuTargetMonitor = NULL;
        g_pendingStartMenuTargetMonitorConfirmed = FALSE;
    }
    ReleaseSRWLockExclusive(&g_startMenuTargetMonitorLock);
}
