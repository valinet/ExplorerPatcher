#ifndef _H_STARTMENUMONITORSTATE_H_
#define _H_STARTMENUMONITORSTATE_H_

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ULONGLONG START_MENU_MONITOR_REQUEST_ID;

START_MENU_MONITOR_REQUEST_ID BeginStartMenuTargetMonitorRequest(void);
BOOL PublishStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId, HMONITOR monitor);
BOOL ConfirmStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId);
BOOL CancelStartMenuTargetMonitorRequest(START_MENU_MONITOR_REQUEST_ID requestId);
START_MENU_MONITOR_REQUEST_ID AcquireStartMenuTargetMonitor(HMONITOR defaultMonitor, HMONITOR* monitor);
BOOL UpdateStartMenuTargetMonitor(START_MENU_MONITOR_REQUEST_ID lifecycleId, HMONITOR monitor);
START_MENU_MONITOR_REQUEST_ID GetStartMenuTargetMonitorLifecycleId(void);
void CompleteStartMenuTargetMonitorLifecycle(START_MENU_MONITOR_REQUEST_ID lifecycleId);

#ifdef __cplusplus
}
#endif

#endif
