// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS 

#include <ntstatus.h>

#include <TlHelp32.h>

#include <stdlib.h>
#include <tchar.h>
