#pragma once
#include <windef.h>

int GetConhostInit(void);
int EnableSeDebug(void);
DWORD* GetThreads(DWORD pid);
DWORD GetPIDFromHandle(DWORD OwnerPID, HANDLE h);
DWORD GetRelevantPID(DWORD *conhost, int conhost_count, DWORD console_pid);
DWORD* FindConhost(int *found);
