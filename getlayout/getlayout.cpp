/* Copyright (C) 2013 Nick Voronin <elfy.nv@gmail.com> */

// getlayout.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "../getconhost/ntintern.h"
#include "../getconhost/getconhost.h"

void printhelp(void)
{
	printf("Usage:\n\tgetlayout.exe <Console App PID>\n");
}

/* Test program */

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD console_pid;
	NTSTATUS status;
	if (argc != 2 || (console_pid = _wtoi(argv[1])) == 0) {
		printhelp();
		return 0;
	}

	if ( GetConhostInit() ) {
		printf ( "Can't get pointers to necessary routines from ntdll.dll\n" );
		return 1;
	}

	// Is not necessary to have SeDebugPrivilege to open process owned by the same user.
	// Normally lack of SeDebugPrivilege is not a problem. For intended use in ahk even if ahk itself is run
	// as administrator from nonadmin user account it will still be able to access processes
	// of this user account (but not processes run from admin account itself).
	// On the other hand on system with default settings it is impossible to get SeDebugPrivilege anyway, even for admin.
	status = EnableSeDebug();
	if ( status != STATUS_SUCCESS ) {
		printf("Can't adjust privileges, code %X\n", status);
	}

	int conhost_count;
	DWORD *conhost = FindConhost(&conhost_count);
	if (conhost == NULL) {
		printf("Can't enum conhost processes\n");
		return 1;
	}
	if (conhost_count == 0) {
		printf("Can't find any conhost processes\n");
		return 1;
	}

	DWORD conhost_pid = GetRelevantPID(conhost, conhost_count, console_pid);
	free(conhost);

	if (conhost_pid == 0) {
		printf("Can't find related conhost process\n");
		return 0;
	}

	DWORD *threads = GetThreads(conhost_pid);

	if (threads == NULL) {
		printf("Error enumerating threads for conhost\n");
		return 0;
	}

	int i = 0;
	while (threads[i] != NULL) {
		// it seems that second thread is relevant for GetKeyboardLayout().
		// second thread might be with lower TID, but it's always second in enumeration
		printf("TID:%04X KeyboardLayout:%04X\n", threads[i], GetKeyboardLayout(threads[i]));
		++i;
	}
	free(threads);
	// wait
	scanf("%d", &i);

	return 0;
}

