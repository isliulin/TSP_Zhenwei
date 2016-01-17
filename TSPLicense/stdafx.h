// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#ifndef SAFE_CLOSEHANDLE
#define SAFE_CLOSEHANDLE(p)		{if (NULL != p) {CloseHandle(p); p = NULL;}}
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����
#include "..\TSPClient\LicenseDef.h"


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "shlwapi.h"

void GetClientDNS(SOCKET AcceptSocket,TCHAR *szDNS);



#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if (NULL != p) {p->Release(); p = NULL;}}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (NULL != p) {delete p; p = NULL;}}
#endif

#ifndef SAFE_DELARRAY
#define SAFE_DELARRAY(p) {if (NULL != p) {delete[] p; p = NULL;}}
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)	(sizeof(a)/sizeof(a[0]))
#endif

#ifndef SAFE_CLOSE_SOCKET
#define SAFE_CLOSE_SOCKET(s)	{if (INVALID_SOCKET != s) {shutdown(s, SD_BOTH); closesocket(s); s=INVALID_SOCKET;}}
#endif

#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(handle)		{if(handle != NULL) {CloseHandle(handle);handle = NULL;}};
#endif

#ifndef SAFE_CLOSE_FILEHANDLE
#define SAFE_CLOSE_FILEHANDLE(handle)	{if(handle != INVALID_HANDLE_VALUE) {CloseHandle(handle);handle = INVALID_HANDLE_VALUE;}};
#endif


//��־����
enum LOG_LEVEL
{
	LOG_LEVEL_1 = 1,
	LOG_LEVEL_2,
	LOG_LEVEL_3,
	LOG_LEVEL_4,
	LOG_LEVEL_5,
};

#define  ONE_PACKAGE 1024
#define SIZE_HEAD_EXT 16



#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")



#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


