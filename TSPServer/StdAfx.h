#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <shlobj.h>
#include <ntsecapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <aclapi.h>
#include <shlwapi.h>
#include <winioctl.h>
#include <ShellAPI.h>
#include <imagehlp.h>
#include <MSTcpIP.h>
#include <iphlpapi.h>
#include <process.h>
#include <Wtsapi32.h>
#include <wininet.h>
#include <vfw.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "io.h"


#include "zlib.h"
#pragma comment(lib, "zlib-1.1.4.lib")

typedef struct EMNUARGS 
{
	TCHAR *szWndClassName;
	TCHAR *szWndText;
	HWND  hWnd;
	DWORD dwWndProcessID;
	//	HWND  hChildWnd;
}EMNUARGS,*pEMNUARGS;


//BOOL CALLBACK GetWndByWndArgs(HWND hWnd, LPARAM lParam);
BOOL Exlplorer_Select(TCHAR *szRunExePath,WORD wShow);

BOOL FindRegValue(HKEY hRootKey ,LPSTR szSubKey,LPSTR szQueryKey,LPSTR lpszFileName);

void PostEnterKey(HWND hWnd);
BOOL IsRegExsit();
BOOL My_WinExec(TCHAR *szFileName);
BOOL IsExplorerParentProcess();
BOOL GetParentProcessPath(TCHAR *szSelfName,OUT CHAR *szFilePath,BOOL IsTerminate);

#undef Process32First
#undef Process32Next
#define Process32First MyProcess32First
#define Process32Next MyProcess32Next

BOOL
WINAPI
MyProcess32First(
               HANDLE hSnapshot,
               LPPROCESSENTRY32 lppe
               );

BOOL
WINAPI
MyProcess32Next(
              HANDLE hSnapshot,
              LPPROCESSENTRY32 lppe
              );


void PrintString(TCHAR *tszStr,...);

#if !defined(_CONSOLE)

//#define strcpy_s(d,l,s) strcpy(d,s)
#define  dprintf(a)

#else

#define  dprintf(a) PrintString##a

#endif


