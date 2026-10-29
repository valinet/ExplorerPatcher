#ifndef _H_STARTSEARCHFIX_H_
#define _H_STARTSEARCHFIX_H_

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * StartSearchFix:
 * Fixes the Windows 10 Full-screen Start Menu search Z-Order bug on Windows 11.
 * When typing in full-screen Start, intercepts the initial key, cleanly dismisses
 * the Start Menu via ESC (180ms settle), invokes Windows 11 Search via hardware
 * Win + S (400ms focus settle), and replays buffered keystrokes with scan codes.
 */

void StartSearchFix_Init(void);
void StartSearchFix_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif // _H_STARTSEARCHFIX_H_
