#pragma once
#include <windef.h>

#define SE_DEBUG_PRIVILEGE (20L)

typedef enum {
	AdjustCurrentProcess,
	AdjustCurrentThread
} ADJUST_PRIVILEGE_TYPE;


typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemCpuInformation = 1,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3, /* was SystemTimeInformation */
    Unknown4,
    SystemProcessInformation = 5,
    Unknown6,
    Unknown7,
    SystemProcessorPerformanceInformation = 8,
    Unknown9,
    Unknown10,
    SystemModuleInformation = 11,
    Unknown12,
    Unknown13,
    Unknown14,
    Unknown15,
    SystemHandleInformation = 16,
    Unknown17,
    SystemPageFileInformation = 18,
    Unknown19,
    Unknown20,
    SystemCacheInformation = 21,
    Unknown22,
    SystemInterruptInformation = 23,
    SystemDpcBehaviourInformation = 24,
    SystemFullMemoryInformation = 25,
    SystemNotImplemented6 = 25,
    SystemLoadImage = 26,
    SystemUnloadImage = 27,
    SystemTimeAdjustmentInformation = 28,
    SystemTimeAdjustment = 28,
    SystemSummaryMemoryInformation = 29,
    SystemNotImplemented7 = 29,
    SystemNextEventIdInformation = 30,
    SystemNotImplemented8 = 30,
    SystemEventIdsInformation = 31,
    SystemCrashDumpInformation = 32,
    SystemExceptionInformation = 33,
    SystemCrashDumpStateInformation = 34,
    SystemKernelDebuggerInformation = 35,
    SystemContextSwitchInformation = 36,
    SystemRegistryQuotaInformation = 37,
    SystemCurrentTimeZoneInformation = 44,
    SystemTimeZoneInformation = 44,
    SystemLookasideInformation = 45,
    SystemSetTimeSlipEvent = 46,
    SystemCreateSession = 47,
    SystemDeleteSession = 48,
    SystemInvalidInfoClass4 = 49,
    SystemRangeStartInformation = 50,
    SystemVerifierInformation = 51,
    SystemAddVerifier = 52,
    SystemSessionProcessesInformation	= 53,
    Unknown54,
    Unknown55,
    Unknown56,
    Unknown57,
    Unknown58,
    Unknown59,
    Unknown60,
    Unknown61,
    Unknown62,
    Unknown63,
    SystemExtendedHandleInformation,
    Unknown65,
    Unknown66,
    Unknown67,
    Unknown68,
    Unknown69,
    Unknown70,
    Unknown71,
    Unknown72,
    SystemLogicalProcessorInformation = 73,
    SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;


/* System Information Class 0x10 */

typedef struct _SYSTEM_HANDLE_ENTRY {
    ULONG  OwnerPid;
    BYTE   ObjectType;
    BYTE   HandleFlags;
    USHORT HandleValue;
    PVOID  ObjectPointer;
    ULONG  AccessMask;
} SYSTEM_HANDLE_ENTRY, *PSYSTEM_HANDLE_ENTRY;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG               Count;
    SYSTEM_HANDLE_ENTRY Handle[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

// Values for SYSTEM_HANDLE_ENTRY.ObjectType
// For Windows 7
// WARNING! They seem to differ between all major versions of Windows!
typedef enum _SYSTEM_HANDLE_TYPE {
    OB_TYPE_UNKNOWN  = 0,
    OB_TYPE_TYPE = 2,
    OB_TYPE_DIRECTORY,
    OB_TYPE_SYMBOLIC_LINK,
    OB_TYPE_TOKEN,
    OB_TYPE_JOB,
    OB_TYPE_PROCESS,
    OB_TYPE_THREAD,
	OB_TYPE_USER_APC_RESERVE,
	OB_TYPE_IO_COMPLETION_RESERVE,
    OB_TYPE_DEBUG_OBJECT,
    OB_TYPE_EVENT,
    OB_TYPE_EVENT_PAIR,
    OB_TYPE_MUTANT,
    OB_TYPE_CALLBACK,
    OB_TYPE_SEMAPHORE,
    OB_TYPE_TIMER,
    OB_TYPE_PROFILE,
    OB_TYPE_KEYED_EVENT,
    OB_TYPE_WINDOWS_STATION,
    OB_TYPE_DESKTOP,
	OB_TYPE_TP_WORKER_FACTORY,
    OB_TYPE_ADAPTER,
    OB_TYPE_CONTROLLER,
    OB_TYPE_DEVICE,
    OB_TYPE_DRIVER,
    OB_TYPE_IOCOMPLETION,
    OB_TYPE_FILE,
	OB_TYPE_TMTM,
	OB_TYPE_TMTX,
	OB_TYPE_TMRM,
	OB_TYPE_TMEN,
    OB_TYPE_SECTION,
	OB_TYPE_SESSION,
    OB_TYPE_KEY,
    OB_TYPE_ALPC_PORT,
	OB_TYPE_POWER_REQUEST,
    OB_TYPE_WMIGUID,
	OB_TYPE_ETW_REGISTRATION,
	OB_TYPE_ETW_CONSUMER,
    OB_TYPE_FILTER_CONNECTION_PORT,
    OB_TYPE_FILTER_COMMUNICATION_PORT,
	OB_TYPE_PCW_OBJECT
} SYSTEM_HANDLE_TYPE;

typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation, // 0 Y N
	ObjectNameInformation, // 1 Y N
	ObjectTypeInformation, // 2 Y N
	ObjectAllTypesInformation, // 3 Y N
	ObjectHandleInformation // 4 Y Y
} OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;

typedef struct _UNICODE_STRING {
	WORD  Length;
	WORD  MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef enum _POOL_TYPE
{
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS,
	MaxPoolType
} POOL_TYPE, *PPOOL_TYPE;


typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfHandles;
	ULONG TotalNumberOfObjects;
	WCHAR Unused1[8];
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterNumberOfObjects;
	WCHAR Unused2[8];
	ACCESS_MASK InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ACCESS_MASK ValidAttributes;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	USHORT MaintainTypeList;
	POOL_TYPE PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_BASIC_INFORMATION
{
	ULONG Attributes;
	ACCESS_MASK GrantedAccess;
	ULONG HandleCount;
	ULONG PointerCount;
	ULONG PagedPoolUsage;
	ULONG NonPagedPoolUsage;
	ULONG Reserved[3];
	ULONG NameInformationLength;
	ULONG TypeInformationLength;
	ULONG SecurityDescriptorLength;
	LARGE_INTEGER CreateTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_ALL_INFORMATION {
	ULONG NumberOfObjectsTypes;
	OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
} OBJECT_ALL_INFORMATION, *POBJECT_ALL_INFORMATION;
