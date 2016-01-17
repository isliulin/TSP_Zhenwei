// Process.cpp : implementation file

#include "stdafx.h"
#include <windows.h>
#include <TLHELP32.H>
#include "Psapi.h"
#include "tchar.h"
#include "myprocess.h"
#include "common.h"

//#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
/////////////////////////////////////////////////////////////////////////////
// CProcess

/////////////////////////////////////////////////////////////////////////////




CProcess::CProcess()
{
	m_dwProcessID = 0;
//	memset(szProcessName,0,MAX_PATH);
}

CProcess::~CProcess()
{
	m_dwProcessID=0;
}


CProcess::CProcess(LPCTSTR lpszProcessName)
{
	m_dwProcessID=FindProcessID(lpszProcessName);
}

inline LPTSTR ProcessPathToName(LPTSTR lpszProcessName)
{
	
	LPTSTR p=tstrrchr(lpszProcessName,'\\');
	if(p) return p+1;
	else  return lpszProcessName;
}


////////////////////////////////////////////////////////////////////////////////////////////
// 函数名    ：FindProcessID
// 功能      : 通过进程名查找进程ID  
//
// 输入参数  : lpszProcess--------要查找的进程名 
//返回值     : 成功返回进程ID,否则返回0    
//autor      : yx 2009-06-04 2010/12/27
/////////////////////////////////////////////////////////////////////////////////////////////

DWORD CProcess::FindProcessID(LPCTSTR lpszProcessName)
{
	TCHAR szProcessName[MAX_PATH]={0};
	tstrcpy(szProcessName,lpszProcessName,MAX_PATH);
	tstrlwr(szProcessName);

	CProcess cp;
	DWORD dwRetProcessID=0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	PROCESSENTRY32 pe32;

	pe32.dwSize = sizeof( PROCESSENTRY32 );

	Process32First( hSnapshot, &pe32 );
	do
	{
		if (tstricmp(pe32.szExeFile,ProcessPathToName(szProcessName))==0)//
		{
			TCHAR  lpszProcessFullPath[MAX_PATH*2];
			memset(lpszProcessFullPath,0,MAX_PATH*2);

			if(cp.GetProcessFullPath(lpszProcessFullPath,pe32.th32ProcessID))
			{
				tstrlwr(lpszProcessFullPath); //
				if (tstrstr(lpszProcessFullPath,szProcessName))
				{
					dwRetProcessID=pe32.th32ProcessID;
					break;
				}
			}
		}

	} while (Process32Next( hSnapshot, &pe32 ));

	CloseHandle( hSnapshot );
	return dwRetProcessID;
}


////////////////////////////////////////////////////////////////////////////////////////////
// 函数名    ：GetProcessPath
// 功能      : 通过进程ID获取进程的完整路径
//
// 输入参数  : dwPID--------要获取的进程ID 
//返回值     : 成功返回TRUEL    
//autor      : yx 2009-06-04
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CProcess::GetProcessFullPath(OUT LPTSTR szFilePath,DWORD dwProcessID)
{

	BOOL bRet=FALSE;
	HANDLE hProcess =OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,dwProcessID);
	if (hProcess)
	{
		if(GetModuleFileNameEx(hProcess,0,szFilePath,MAX_PATH)){
			bRet=TRUE;
		}
		CloseHandle(hProcess);
	}

	return bRet;
}

VOID EnableDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			dprintf((_T("OpenProcessToken erro :%d"),GetLastError()));
			return; 
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {
		dprintf((_T("LookupPrivilegeValue erro :%d"),GetLastError()));
		CloseHandle(hToken);
		return;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
		dprintf((_T("AdjustTokenPrivileges erro :%d"),GetLastError()));
		CloseHandle(hToken);
		return;
	}

	CloseHandle(hToken);
	return;
}

BOOL CProcess::KillProcess()
{
	if (!m_dwProcessID)
	{
		dprintf((_T("m_dwProcessID=0 in CProcess::KillProcess")));
		return FALSE;
	}
	EnableDebugPriv();
	BOOL bRet=FALSE;


	HANDLE hProc = OpenProcess(PROCESS_TERMINATE,FALSE,m_dwProcessID);
	if(hProc!=NULL)
	{
		DWORD dwExitCode=0;
		GetExitCodeProcess(hProc,&dwExitCode);
		if(TerminateProcess(hProc,dwExitCode)){
			bRet=TRUE;
		}
		CloseHandle(hProc);
		dprintf((_T("TerminateProcess erro :%d"),GetLastError()));
	}

	dprintf((_T("OpenProcess erro :%d"),GetLastError()));
	return bRet;
}

DWORD CProcess::KillAllProcessByName(LPCTSTR lpszProcessName)
{
	CProcess cp;
	while(cp.m_dwProcessID=cp.FindProcessID(lpszProcessName))
	{
		cp.KillProcess();
	}
	return cp.m_dwProcessID;
}

#ifdef UNICODE

#define szLoadLibrary "LoadLibraryW"
#define szGetModuleHandle "GetModuleHandleW"

#else

#define szLoadLibrary "LoadLibraryA"
#define szGetModuleHandle "GetModuleHandleA"

#endif

BOOL CProcess::RemoteLoadLibrary(IN LPCTSTR lpszDllName)
{


	BOOL    bRet=FALSE;	
	HMODULE hKernel32 = ::GetModuleHandle(_T("Kernel32"));


	//提升权限
	EnableDebugPriv();

	HANDLE hProcess =OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |PROCESS_VM_WRITE,FALSE,m_dwProcessID);

	if (!hProcess){
		dprintf((_T("OpenProcess erro :%d"),GetLastError()));
		return bRet;
	}

	LPVOID pLibRemote =VirtualAllocEx(hProcess,NULL,MAX_PATH,MEM_COMMIT,PAGE_READWRITE);

	if (!pLibRemote){
		dprintf((_T("VirtualAllocEx erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		return bRet;
	}

	if(!WriteProcessMemory(hProcess,pLibRemote,(void*)lpszDllName,MAX_PATH,NULL))
	{
		dprintf((_T("WriteProcessMemory erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		::VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE );
		return bRet;
	}

	HANDLE hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32,szLoadLibrary),
		pLibRemote, 0, NULL );

	if (!hThread){
		dprintf((_T("CreateRemoteThread erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		::VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE);
		return bRet;
	}

	//
	WaitForSingleObject(hThread,INFINITE);

	DWORD   hLibModule;      //已加载的DLL的基地址（HMODULE）;

	//取得DLL的基地址
	GetExitCodeThread(hThread,&hLibModule);
	CloseHandle(hThread );

	//扫尾工作
	bRet=VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE );
	CloseHandle(hProcess);
	return bRet;

}


BOOL CProcess::RemoteFreeLibrary(IN LPCTSTR lpszDllName)
{
	BOOL   bRet=FALSE; 

	HMODULE hKernel32 = GetModuleHandle(_T("Kernel32")); 

	//提升权限
	EnableDebugPriv();

	HANDLE hProcess =OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |PROCESS_VM_WRITE,FALSE,m_dwProcessID);

	if (!hProcess){
		dprintf((_T("OpenProcess erro :%d"),GetLastError()));
		return bRet;		
	}

	LPVOID pLibRemote =VirtualAllocEx(hProcess, NULL, 100,MEM_COMMIT,PAGE_READWRITE);

	if(!pLibRemote){
		dprintf((_T("VirtualAllocEx erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		return bRet;
	}

	if(!WriteProcessMemory(hProcess,pLibRemote,(LPVOID)lpszDllName,MAX_PATH,NULL))
	{
		dprintf((_T("WriteProcessMemory erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		// 释放目标进程中申请的空间
		VirtualFreeEx(hProcess,pLibRemote,0,MEM_DECOMMIT);
		return bRet;
	}

	// 使目标进程调用GetModuleHandle，获得DLL在目标进程中的句柄
	DWORD dwHandle, dwID;

	HANDLE hThread = CreateRemoteThread(hProcess,NULL,0,
		(LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32,szGetModuleHandle),
		pLibRemote, 0, &dwID);


	WaitForSingleObject(hThread, INFINITE );

	// 获得GetModuleHandle的返回值
	GetExitCodeThread(hThread,&dwHandle);

	// 释放目标进程中申请的空间
	VirtualFreeEx(hProcess,pLibRemote,0,MEM_RELEASE);

	CloseHandle(hThread);

	// 使目标进程调用FreeLibrary，卸载DLL

	hThread = CreateRemoteThread(hProcess,NULL,0,
		(LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32,"FreeLibrary"),
		(LPVOID)(dwHandle),0,&dwID);

	WaitForSingleObject(hThread,INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
    return TRUE;
}




