// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GETKBLAYOUT_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GETKBLAYOUT_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GETKBLAYOUT_DLL_EXPORTS
#define GETKBLAYOUT_DLL_API __declspec(dllexport)
#else
#define GETKBLAYOUT_DLL_API __declspec(dllimport)
#endif

extern "C" GETKBLAYOUT_DLL_API BOOL Initialize(BOOL debug);

extern "C" GETKBLAYOUT_DLL_API HKL GetConsoleAppKbLayout(DWORD console_pid);
