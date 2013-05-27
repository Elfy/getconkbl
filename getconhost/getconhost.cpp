/* Copyright (C) 2013 Nick Voronin <elfy.nv@gmail.com> */

#include "stdafx.h"
#include "getconhost.h"
#include "ntintern.h"

NTSTATUS (WINAPI *pNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
NTSTATUS (WINAPI *pNtQueryObject)(HANDLE, UINT, PVOID, ULONG, PULONG);
NTSTATUS (WINAPI *pRtlAdjustPrivilege)(DWORD, BOOLEAN, ADJUST_PRIVILEGE_TYPE, PBOOLEAN);

int GetConhostInit(void)
{
	HMODULE ntdll;
	ntdll = LoadLibrary(_T("ntdll.dll"));
	if(ntdll == NULL) {
		return -1;
	}

	*(FARPROC *)&pNtQuerySystemInformation = GetProcAddress(ntdll, "NtQuerySystemInformation");
	if(pNtQuerySystemInformation == NULL) {
		return -2;
	}
	*(FARPROC *)&pNtQueryObject = GetProcAddress(ntdll, "NtQueryObject");
	if(pNtQueryObject == NULL) {
		return -2;
	}
	*(FARPROC *)&pRtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
	if(pRtlAdjustPrivilege == NULL) {
		return -2;
	}
	return 0;
}

int EnableSeDebug(void)
{
	BOOLEAN prev_enabled = FALSE;
	NTSTATUS status = pRtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, AdjustCurrentProcess, &prev_enabled);
	if (status != STATUS_SUCCESS) {
		OutputDebugString(TEXT("Can't adjust privileges, code %X\n"));
	}
	return status;
}

int EnumerateObTypes(POBJECT_ALL_INFORMATION index, ULONG length)
{
	ULONG ret_length;
	if (index == NULL || length < sizeof(OBJECT_ALL_INFORMATION))
		return -1;

	NTSTATUS status = pNtQueryObject(0, ObjectAllTypesInformation, index, sizeof(OBJECT_ALL_INFORMATION), &ret_length);
	if ( STATUS_INFO_LENGTH_MISMATCH == status ) {
		if (length < ret_length)
			return -1;
		status = pNtQueryObject(0, ObjectAllTypesInformation, index, ret_length, &ret_length);
	}
	if (status == STATUS_SUCCESS) {
		// do something interesting with index
		return ret_length;
	}
	return -1;
}

int GetProcessObIdx(void)
{
	static const int bsize = 4096*2; // should be enough -_-'
	static wchar_t const search_term[]=L"Process";
	char buffer[bsize];
	POBJECT_ALL_INFORMATION index = (POBJECT_ALL_INFORMATION)buffer;
	POBJECT_TYPE_INFORMATION type;
	int length = EnumerateObTypes(index, bsize);
	if (length <= 0) {
		return OB_TYPE_PROCESS; // If something went wrong rely on headers
	}
	type = index->ObjectTypeInformation;
	for (ULONG i = 0; i < index->NumberOfObjectsTypes; ++i) {
		if ( type->TypeName.Length == (sizeof(search_term) - sizeof (wchar_t)) &&
			 0 == memcmp(search_term, type->TypeName.Buffer, type->TypeName.Length) ) {
			return i + 2; // WARNING! This is only for Windows 7
		}
		type = (POBJECT_TYPE_INFORMATION)((char*)type + sizeof(*type) + ((type->TypeName.MaximumLength + 7)&0xFFFFFFF8) );
	}
	return OB_TYPE_PROCESS; // If something went wrong rely on headers
}

// returns NULL on error
// returns array of thread ids terminated with zero
// may return incomplete list if it can't allocate enough memory
DWORD* GetThreads(DWORD pid)
{
	DWORD *threads = NULL;
	int allocated, found;
	if (pid == 0)
		return NULL;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snapshot != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		threads = (DWORD*) malloc(sizeof(DWORD)*16);
		if (threads != NULL) {
			allocated = 16;
			found = 0;
			if (Thread32First(snapshot, &te)) {
				do {
					if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID) && te.th32OwnerProcessID == pid) {
						if (found+2 >= allocated) {
							DWORD *threads_r = (DWORD *)realloc(threads, (allocated + 16)*sizeof(DWORD));
							if (threads_r == NULL) {
								// return incomplete list
								break;
							}
							threads = threads_r;
							allocated += 16;
						}
						found++;
						threads[found-1] = te.th32ThreadID;
//						printf("Process 0x%04x Thread 0x%04x\n", te.th32OwnerProcessID, te.th32ThreadID);

					}
					te.dwSize = sizeof(te);
				} while (Thread32Next(snapshot, &te));
			}
			threads[found] = 0;
		}
		CloseHandle(snapshot);
	}
	return threads;
}

DWORD GetPIDFromHandle(DWORD OwnerPID, HANDLE h)
{
	HANDLE OwnerProcessH;
	DWORD pid_from_handle = (DWORD)-1;
	OwnerProcessH = OpenProcess(PROCESS_DUP_HANDLE /*| PROCESS_QUERY_INFORMATION | PROCESS_VM_READ*/, FALSE, OwnerPID);
	if (OwnerProcessH != INVALID_HANDLE_VALUE) {
		HANDLE dup_h;
		DWORD result;
		// Interesting how HandleValue here is only USHORT. I suppose it won't work for processes with more than 64k handles.
		// Which is no matter for this particular case, of course. But for reference SystemExtendedHandleInformation returns
		// proper HANDLE http://forum.sysinternals.com/discussion-howto-enumerate-handles_topic19403_page9.html
		result = DuplicateHandle(OwnerProcessH, h, GetCurrentProcess(), &dup_h, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if (result != FALSE) {
			POBJECT_TYPE_INFORMATION type_info = (POBJECT_TYPE_INFORMATION)malloc(sizeof(OBJECT_TYPE_INFORMATION));
			if (type_info != NULL)
			{
				NTSTATUS status;
				ULONG length;
				status = pNtQueryObject(dup_h, ObjectTypeInformation, type_info, sizeof(OBJECT_TYPE_INFORMATION), &length);
				if ( STATUS_INFO_LENGTH_MISMATCH == status ) {
					PVOID buffer = realloc(type_info, length);
					if (buffer != NULL) {
						type_info = (POBJECT_TYPE_INFORMATION)buffer;
						status = pNtQueryObject(dup_h, ObjectTypeInformation, type_info, length, &length);
					}
				}
				if (status == STATUS_SUCCESS && length >= sizeof(OBJECT_TYPE_INFORMATION))
				{
					// double check that we found process handle
					if (type_info->TypeName.Buffer != NULL) {
//						printf("Type: %S\n", type_info->TypeName.Buffer);
						if (!wcscmp(L"Process", type_info->TypeName.Buffer)) {
							// dup_h handle should have PROCESS_QUERY_INFORMATION access right. check OBJECT_BASIC_INFORMATION.DesiredAccess?
							pid_from_handle = GetProcessId(dup_h);
						}
					}
//					printf("name: %S\n", info->Name.Buffer);

				}
				free(type_info);
			}
			CloseHandle(dup_h);
		}
		CloseHandle(OwnerProcessH);
	}
	return pid_from_handle;
}

/* Returns true if item is present in array a */
/* Not very generic :) Just call with the same array and reset before switching to the new one. */
/* To reset call isDwInArrayOpt(NULL, 0, %number other than first actual tested item%) */
bool inline isDwInArrayOpt(DWORD *a, int size, DWORD item) {
	static DWORD last_item = 0xFFFFFFFF;
	static bool is_present = false;
	if (item != last_item) {
		last_item = item;
		is_present = false;
		for (int i = 0; i < size; ++i) {
			if ( a[i] == last_item ) {
				is_present = true;
				break;
			}
		}
	}
	return is_present;
}

DWORD GetRelevantPID(DWORD *conhost, int conhost_count, DWORD console_pid)
{
	DWORD conhost_pid = 0;
	ULONG rsize = 0;
	PVOID buffer = NULL;
	NTSTATUS status;
	static int ProcessObIdx = GetProcessObIdx();
	PSYSTEM_HANDLE_INFORMATION hi = (PSYSTEM_HANDLE_INFORMATION)malloc(sizeof(SYSTEM_HANDLE_INFORMATION));
	if (hi == NULL)
		goto cleanup;

	status = pNtQuerySystemInformation(SystemHandleInformation, hi, sizeof(SYSTEM_HANDLE_INFORMATION), &rsize);
	if ( STATUS_INFO_LENGTH_MISMATCH == status ) {
		rsize += 16*1024;
		buffer = realloc(hi, rsize);
		if (buffer == NULL)
			goto cleanup;
		hi = (PSYSTEM_HANDLE_INFORMATION)buffer;
		status = pNtQuerySystemInformation(SystemHandleInformation, hi, rsize, &rsize);
	}

	if (status != STATUS_SUCCESS || rsize < sizeof(SYSTEM_HANDLE_INFORMATION))
		goto cleanup;

	isDwInArrayOpt(NULL, 0, (DWORD)-1);
	for (unsigned int i = 0; i < hi->Count; ++i) { // go through handles
		if ( hi->Handle[i].ObjectType == ProcessObIdx ) { // check all handles of type "Process"
			if (isDwInArrayOpt(conhost, conhost_count, hi->Handle[i].OwnerPid)) { // owned by conhost processes
				// For simplicity I do not optimize for the case when GetHandleInfo is called many times for same pid.
				// Besides I've never seen conhost owning more than one process handle.
				DWORD owned_pid = GetPIDFromHandle(hi->Handle[i].OwnerPid, (HANDLE)hi->Handle[i].HandleValue);
				if (console_pid == owned_pid) { // got it!
					conhost_pid = hi->Handle[i].OwnerPid;
					break;
				}
			}
		}
	}
cleanup:
	free(hi);
	return conhost_pid;
}


/* on error: returns NULL, *found is undefined
   on success: returns pointer to array of pids and number of elements in it in *found */
DWORD* FindConhost(int *found)
{
	static int const allocate_chunk = 16;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		goto end;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(snapshot, &pe32)) {
		return NULL;
	}

	DWORD *pids = NULL;
	int allocated = 0;
	*found = 0;
	pids = (DWORD*)malloc(sizeof(DWORD)*allocate_chunk);
	if (pids == NULL) {
		goto cleanup;
	}
	allocated = allocate_chunk;

	do {
		/*  Notice that no further check is done to ensure that this process is indeed genuine part of the OS.
			First conhost.exe owning handle of process in question will be reported.
			Proposed improvement: check full path, check for characteristic event name or ALPC Port name, check parent process */
		if (wcscmp(pe32.szExeFile, L"conhost.exe") == 0) {
			// record pid
			++*found;
			if (*found > allocated) {
				DWORD *pids_r = (DWORD *)realloc(pids, allocated + allocate_chunk);
				if (pids_r == NULL) {
					free (pids);
					pids = NULL;
					goto cleanup;
				} else {
					pids = pids_r;
					allocated += allocate_chunk;
				}
			}
			pids[*found-1] = pe32.th32ProcessID;
		}
//		printf ("%S\n", pe32.szExeFile);
	} while (Process32Next(snapshot, &pe32));
cleanup:
	CloseHandle(snapshot);
end:
	return pids;
}
