/* Copyright (C) 2013 Nick Voronin <elfy.nv@gmail.com> */

// getconkbl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "getconkbl.h"
#include "..\getconhost\getconhost.h"
BOOL Init = FALSE;


GETKBLAYOUT_DLL_API BOOL Initialize(int debug)
{
/* TODO: Check that OS version == win7 */

	if ( GetConhostInit() ) {
		OutputDebugString(TEXT("Can't get pointers to necessary routines from ntdll.dll\n" ));
		Init = FALSE;
		return Init;
	}

	if ( debug ) {
		EnableSeDebug();
	}

	Init = TRUE;
	OutputDebugString(TEXT("Init getconkbl.dll: OK\n"));
	return Init;
}

GETKBLAYOUT_DLL_API HKL GetConsoleAppKbLayout(DWORD console_pid)
{
	if (Init == FALSE)
		return 0;

	int conhost_count;
	DWORD *conhost = FindConhost(&conhost_count);
	if (conhost == NULL) {
		OutputDebugString(TEXT("Can't enum conhost processes\n"));
		return 0;
	}
	if (conhost_count == 0) {
		OutputDebugString(TEXT("Can't find any conhost processes\n"));
		return 0;
	}

	DWORD conhost_pid = GetRelevantPID(conhost, conhost_count, console_pid);
	free(conhost);

	if (conhost_pid == 0) {
		OutputDebugString(TEXT("Can't find related conhost process\n"));
		return 0;
	}

	DWORD *threads = GetThreads(conhost_pid);

	if (threads == NULL) {
		OutputDebugString(TEXT("Error enumerating threads for conhost\n"));
		return 0;
	}

	// it seems that second thread is relevant for GetKeyboardLayout().
	// second thread might be with lower TID, but it's always second in enumeration
	if (threads[1] == NULL) {
		OutputDebugString(TEXT("Too few threads in conhost process\n"));
		return 0;
	}
	DWORD th = threads[1];
	free(threads);
	return GetKeyboardLayout(th);
}

