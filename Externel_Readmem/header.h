#pragma once
#include <ntddk.h>

#define _NO_CRT_STDIO_INLINE
#define IOCTL_IMGBASE CTL_CODE(FILE_DEVICE_UNKNOWN,0x888,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_IMGBASEOUT CTL_CODE(FILE_DEVICE_UNKNOWN,0x889,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_READMEM CTL_CODE(FILE_DEVICE_UNKNOWN,0x999,METHOD_BUFFERED,FILE_ANY_ACCESS)

//SYMLINK 
#define SYMLINK L"\\DosDevices\\ReadMem"

#define POOL_TAG '1gaT'
#define dmsg(...) {DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0,"[*] ");DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0,__VA_ARGS__);DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0,"\n");}


extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriver, IN PUNICODE_STRING pRegPath);
VOID UnloadDriver(IN PDRIVER_OBJECT pDriver);

//NtQuerySystemInformation 
typedef NTSTATUS(*NTAPI NtQuerySystemInformation_t)(
    ULONG SystemInforationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );

NtQuerySystemInformation_t(NtQuerySystemInformation) = nullptr;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45,
    SystemCodeIntegrityInformation = 103,
    SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

typedef unsigned char BYTE;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    BYTE Reserved1[48];
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    PVOID Reserved2;
    ULONG HandleCount;
    ULONG SessionId;
    PVOID Reserved3;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG Reserved4;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    PVOID Reserved5;
    SIZE_T QuotaPagedPoolUsage;
    PVOID Reserved6;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved7[6];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef SYSTEM_PROCESS_INFORMATION SYSTEM_PROCINFO;

//PsLookupProcessByProcessId 함수 원형 선언 
typedef NTSTATUS(*NTAPI PsLookupProcessByProcessId_t)(
    long int    ProcessId,
    PEPROCESS* Process
    );
PsLookupProcessByProcessId_t(PsLookupProcessByProcessId) = nullptr;

//PsGetProcessImageFileName  함수 원형 선언
typedef UCHAR* (*NTAPI PsGetProcessImageFileName_t)(
    PEPROCESS Process
    );
PsGetProcessImageFileName_t(PsGetProcessImageFileName) = nullptr;


//PsGetProcessPed 함수 원형 선언
typedef DWORD64* (*NTAPI PsGetProcessPeb_t)(
    PEPROCESS Process
    );
PsGetProcessPeb_t(PsGetProcessPeb) = nullptr;

//KeAttachProcess 함수 원형 선언
typedef  void (*NTAPI KeAttachProcess_t)(
    PEPROCESS Process
    );
KeAttachProcess_t(KeAttachProcess) = nullptr;

//KeDetachProcess 함수 원형 선언
typedef  void (*NTAPI KeDetachProcess_t)(
    PEPROCESS Process
    );
KeDetachProcess_t(KeDetachProcess) = nullptr;



typedef struct _MAL_GLBOAL {
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT DeviceObject;
}MAL_GLOBAL, * PMAL_GLOBAL;

extern MAL_GLOBAL g_MalGlobal;

//------------------------------------------------
