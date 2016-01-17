#include "StdAfx.h"
#include "windows.h"
#include "stdio.h"
#include "common.h"
#include <tchar.h>




void PrintString(TCHAR *tszStr,...)
{
	TCHAR szFormat[MAX_PATH*2] = {0};

	va_list argList;
	va_start(argList,tszStr);
	t_vsnprintf(szFormat, MAX_PATH*2-1,tszStr, argList );

	va_end(argList);
	OutputDebugString(szFormat);
}

//////////////////////////////////////////////////////////////////////////
// 检测文件是否存在
//
BOOL IsFileExist(TCHAR *FilePath)
{
	HANDLE hFile =CreateFile(FilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		CloseHandle(hFile);
		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
// 把指定资源保存为文件
//
BOOL WriteResToFile(HMODULE hModule,DWORD dwResourceId,TCHAR* lpResourceType,TCHAR *lpFilePath)
{
//	return TRUE;

	TCHAR szBuff[MAX_PATH];
	if(tstrlen(lpFilePath)>MAX_PATH)
		return FALSE;


	GetTempPath(MAX_PATH,szBuff);
	tstrcat(szBuff,_T("~DFE0F5.tmp"));
	HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(dwResourceId), lpResourceType);
	if(!hRes)
	{
		return FALSE;
	}
	
	HGLOBAL			hGlobalRes = LoadResource(hModule,hRes);
	if(!hGlobalRes)
	{
		return FALSE;
	}


	unsigned long dwResourceSize = SizeofResource(hModule, hRes);
		
	unsigned char* pResource = (unsigned char *)LockResource(hGlobalRes);

	if(!pResource)
	{
		return FALSE;
	}

	if(IsFileExist(szBuff))
	{
		DeleteFile(szBuff);
	}

	FILE * pfile=NULL;
	tfopen(&pfile,szBuff,_T("wb+"));

	if (!pfile)
	{
		return FALSE;
	}
	//#define COMPRESS

#ifdef COMPRESS
	LZARI		lzARI;
	int			iSize=0;

	//解压缩
	iSize=lzARI.GetDecompressLen((const BYTE*)pResource);

	BYTE* pBuff=new BYTE[iSize];
	INT dwBuffSize=iSize;

	if(!lzARI.DeCompress((const BYTE*)pResource,dwResourceSize,(BYTE*)pBuff,dwBuffSize))
	{	
		delete []pBuff;
		return FALSE;
	}
	fseek(pfile,0,SEEK_SET);

	fwrite(pBuff,1,dwBuffSize,pfile);

	if(pBuff)
	{
		delete []pBuff;
	}
#else

	fwrite(pResource,1,dwResourceSize,pfile);
#endif

	fclose(pfile);


	if(!CopyFile(szBuff,lpFilePath,FALSE))
	{
		DeleteFile(szBuff);
		return FALSE;
	}
	else
	{
		DeleteFile(szBuff);
		return TRUE;
	}
}

BOOL ReleaseResToFile(HMODULE hModule, DWORD dwResourceId,TCHAR* lpResourceType,TCHAR *lpFilePath)
{
	return WriteResToFile(hModule,dwResourceId,lpResourceType,lpFilePath);
}

void ASCIIToUnicode(char* ASCIIString,wchar_t* WideChar)
{
	int AStrLen;


	AStrLen=MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,0);

	//	AStrLen=WideCharToMultiByte(CP_ACP,0,WideChar,wcslen(WideChar),ASCIIString,0,NULL,NULL);


	MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,AStrLen);
}

void UnicodeToASCII(wchar_t* WideChar,char* ASCIIString)
{
	int AStrLen;


	AStrLen=WideCharToMultiByte(CP_ACP,0,WideChar,wcslen(WideChar),ASCIIString,0,NULL,NULL);


	WideCharToMultiByte(CP_ACP,0,WideChar,wcslen(WideChar),ASCIIString,AStrLen,NULL,NULL);
}