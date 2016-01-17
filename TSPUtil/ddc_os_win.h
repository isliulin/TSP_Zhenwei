#if !defined(DDC_OS_H__)

#define DDC_OS_H__



#include "IO.h"


// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.
/*
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif
*/

#define snprintf _snprintf
#if _MSC_VER < 1500
#	define vsnprintf _vsnprintf
#else
#	define uint32_t UINT32

#endif	// MSC_VER < 1500

#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define DDC_C_SLASH '\\'
#define DDC_S_SLASH "\\"
#define DDC_WC_SLASH L'\\'

#define DDC_HANDLE HANDLE

#define MPV_MM_HANDLE HANDLE
#define MPV_FILE_HANDLE HANDLE
#define MPV_MAP_HANDLE HANDLE

#define CreateFile_MM CreateFile
#define GetFileSize_MM GetFileSize
#define CloseHandle_MM CloseHandle
#define CreateFileW_MM CreateFileW

#define symlink(x,y) NULL
inline int GetRandomDir(char buffer[16],char c){memcpy(buffer,"Otesttempdirectory",16);return 0;}

namespace nm_ddc
{
typedef unsigned long (*THR_FUNC)(void *);
inline int thr_create( THR_FUNC func,void *arg,long flags,int *t_id,DDC_HANDLE *t_handle,long priority,void *stack,size_t stack_size)
{
	*t_handle = ::CreateThread(0,stack_size,LPTHREAD_START_ROUTINE(func),arg,flags,(LPDWORD)t_id);	
	SetThreadPriority(*t_handle,priority);
	return 0;
}

inline HANDLE WaitForEvent(int count,DDC_HANDLE handles[],int timeout)
{
	DWORD dwRet = WaitForMultipleObjects(count,handles,FALSE,timeout);
	for(int i=0;i<count;++i)
		if( WAIT_OBJECT_0 +i == dwRet )	//½áÊø
		{
			return handles[i];
		}
	return NULL;
}
inline int thr_suspend (DDC_HANDLE handle){ ::SuspendThread(handle); return 0;}
inline int thr_resume(DDC_HANDLE handle){ ::ResumeThread(handle); return 0;}
inline int thr_wait(DDC_HANDLE handle,int timeout){ int ret = ::WaitForSingleObject(handle,timeout); return  ret==WAIT_TIMEOUT?/*DDC_TIMEOUT_ERROR*/0:0;}
inline int thr_kill (DDC_HANDLE handle, int signum){ ::TerminateThread(handle,-1); return 0;}
inline int thr_setprio(DDC_HANDLE handle,int priority,int policy){ ::SetThreadPriority(handle,priority); return 0;}
inline int thr_sleep(DWORD mi){::Sleep(mi); return 0;};
}

#endif	//DDC_OS_LINUX_H__

