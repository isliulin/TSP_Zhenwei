#include "stdafx.h"
#include "TSPClientPublic.h"
#include <crtdbg.h>
#include "stdio.h"
#include "macro.h"
#include "Shlwapi.h"
#include "Shellapi.h"

#include <ostream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
using namespace stdext;
#pragma comment(lib, "Shlwapi.lib")
bool mystrcpy(char *strDestination,char *strSource,int MaxLen)   
{
	if(strlen(strSource)>=MaxLen)
	{
		strDestination[MaxLen-1]='\0';
		memcpy(strDestination,strSource,MaxLen-1);
	}else
		strcpy(strDestination,strSource);
	return true;
}
bool mymemcpy(char *strDestination,char *strSource,int Len)
{
	if(Len>0)
	{
		memcpy(strDestination,strSource,Len);
		return true;
	}
	return false;
}
bool mystrcat(char *strDestination,char *strSource,int MaxLen)   
{
	if(strlen(strSource)+strlen(strDestination)<MaxLen)
	{
		strcat(strDestination,strSource);
		return true;
	}				
	return false;
}	
void SplitString(char *str,vector<string> spitList,vector<string> &list)
{
	char *pBegin=str;
	char *pEnd=str;
	char Temp[MAX_PATH];
	while(pEnd<str+strlen(str))
	{
		BOOL Find=FALSE;
		for(int i=0;i<spitList.size();i++)
		{
			if(memcmp(pEnd,spitList[i].data(),spitList[i].length())==0)
			{
				memcpy(Temp,pBegin,pEnd-pBegin);
				Temp[pEnd-pBegin]=0;
				list.push_back(string(Temp));
				pEnd=pEnd+spitList[i].length();
				pBegin=pEnd;
				Find=TRUE;
				break;
			}
		}
		if(!Find) pEnd++;
	}
	if(pEnd>pBegin)
	{
		list.push_back(string(pBegin));
	}
}

#include <Tlhelp32.h>
BOOL KillProcess(TCHAR *strProc)
{
	//	CString strProc(_T("ebizbdc.exe"));
	HANDLE hSnapshot=::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	HANDLE hd;
	BOOL is_ok;
	int iiii;
	if(::Process32First(hSnapshot,&pe))
	{
		do
		{
#ifdef UNICODE
			if(wcscmp(pe.szExeFile,strProc)==0)
#else
			if(strcmp(pe.szExeFile,strProc)==0)
#endif
			{
				hd = ::OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe.th32ProcessID);
				if (hd == NULL)
				{
					iiii = GetLastError();
				}
				is_ok = ::TerminateProcess(hd, 0);
				if (FALSE == is_ok)
				{
					iiii = GetLastError();
				}
			}
		}while(Process32Next(hSnapshot,&pe));
	}
	CloseHandle(hSnapshot);
	return TRUE;
}
BOOL ReadKey(HKEY mainkey,LPCTSTR subkey,int index, char *value,int *valuelen,BYTE *data,int *datelen,UINT *type)
{
	HKEY key;
	if(RegOpenKeyEx(mainkey,subkey,0,KEY_READ ,&key)==ERROR_SUCCESS)
	{ 
		LONG retCode;
		char Value[MAX_PATH];
		BYTE ValueData[MAX_PATH];
	//	ValueData=new BYTE[MAX_PATH];
		DWORD ValueBufferSize=MAX_PATH ,TypeCode,ValueDataSize=MAX_PATH;

		if((retCode=RegEnumValueA(key,index,Value,&ValueBufferSize,
			NULL,&TypeCode,ValueData,&ValueDataSize))==ERROR_SUCCESS)
		{
			*valuelen=(int)ValueBufferSize;
			memcpy(value,Value,*valuelen);
			memcpy(data,ValueData,ValueDataSize);
			*datelen=(int)ValueDataSize;
			RegCloseKey(key);
			return TRUE;
		}
		else
		{
			RegCloseKey(key);
			return FALSE;
		}
	}
	else return FALSE;
}
BOOL WriteKey(HKEY mainkey,LPCTSTR subkey, LPCTSTR value,BYTE *data,int datelen,UINT type)
{
	HKEY hKeyMyRoot;
	if(RegCreateKey(mainkey,subkey,&hKeyMyRoot)==ERROR_SUCCESS)
	{
		RegSetValueEx(hKeyMyRoot,value,0,type,(const BYTE *)data,datelen);
		RegCloseKey(hKeyMyRoot);
		return TRUE;
	}
	else
		return FALSE;
}
BOOL FindKeyFromReg(HKEY mainkey,LPCTSTR subkey,LPCTSTR Title,OUT char *pOut,int MaxLen)
{
	char Value[MAX_PATH];
	int valuelen=MAX_PATH;
	BYTE Data[MAX_PATH];
	int datelen=MaxLen;
	UINT type;
	BOOL result=0;
	int Index=0;
	for(;;)
	{
		result=ReadKey(mainkey,subkey,Index,Value,&valuelen,(BYTE*)Data,&datelen,&type);
		if(!result)break;
		if(memcmp(Value,Title,strlen(Title))==0)
		{
			if(pOut)
			{
				memcpy(pOut,Data,datelen);
				pOut[datelen]=0;
			}
			return TRUE;
		}
		Index++;
	}
	return FALSE;
}
BOOL GetCommandLineDirectory(char *Path)
{
	LPTSTR CommandLine=GetCommandLine();
	if(CommandLine==NULL)return FALSE;
	if(CommandLine[0]=='"')
	{
		mystrcpy(Path,CommandLine+1,MAX_PATH);
	}
	else
	{
		mystrcpy(Path,CommandLine,MAX_PATH);
	}

	//对于有参数的命令行，在第二个'"'结束前的内容才是命令行目录。
	for(int i=1;i<strlen(Path);i++)
	{
		if(Path[i]=='"')
		{
			Path[i]=0;
			break;
		}
	}
	BOOL Find=FALSE;
	for(int i=strlen(Path)-1;i>=0;i--)
	{
		if(Path[i]=='\\')
		{
			Path[i]=0;
			Find=TRUE;
			break;
		}
	}
	if(!Find)
	{
		GetCurrentDirectory(MAX_PATH,Path);
	}

	return TRUE;
}
void BinToBinStr(unsigned char *Key,int Len,OUT char *pStr)
{
	int Index=0;
	for(int i=0;i<Len;i++)
	{
		pStr[Index]=(Key[i]&0xF0)>>4;
		if((pStr[Index]>=0)&&(pStr[Index]<=9))
			pStr[Index]='0'+pStr[Index];
		else
			pStr[Index]='a'+pStr[Index]-0x0A;
		Index++;
		pStr[Index]=(Key[i]&0x0F);
		if((pStr[Index]>=0)&&(pStr[Index]<=9))
			pStr[Index]='0'+pStr[Index];
		else
			pStr[Index]='a'+pStr[Index]-0x0A;
		Index++;
	}
	pStr[Index]=0;
}
const char *GetDataPath()
{
	static char Path[MAX_PATH];
	memset(Path,0,MAX_PATH);
	if(FindKeyFromReg(HKEY_LOCAL_MACHINE,"SOFTWARE\\CAJC\\KTD","szKTDPath",Path,MAX_PATH))
	{
		if(Path[strlen(Path)-1]=='\\')
		{
			mystrcat(Path,"qe",MAX_PATH);
		}
		else
		{
			mystrcat(Path,"\\qe",MAX_PATH);
		}
		return Path;
	}
	if(GetCommandLineDirectory(Path))
		return Path;
	return NULL;
}


BOOL GetCommandLineDirectory(char *Path);
int GetCurFileVerName(char *FileVerName)
{
		struct LANGANDCODEPAGE {
		  WORD wLanguage;
		  WORD wCodePage;
		} *lpTranslate;
		VOID *FileVer=NULL;
		UINT FileVerLen=0;
	TCHAR SubBlock[80];
		TCHAR FileName[MAX_PATH]=_T("");	
		if(!GetCommandLineDirectory(FileName))
			return FALSE;
		strcat(FileName,"\\TSPClient.exe");

		DWORD blockSize=GetFileVersionInfoSize(FileName,0);
		if(blockSize<=0)return FALSE;
		TCHAR *pBlock=new TCHAR[blockSize];
		if(!GetFileVersionInfo(FileName,0,blockSize,pBlock))
			return FALSE;
		UINT cbTranslate=sizeof(LANGANDCODEPAGE);
		VerQueryValue(pBlock, TEXT("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);
		for(int  i=0; i < (int)(cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
		{
			wsprintf( SubBlock, 
				TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),
				lpTranslate[i].wLanguage,
				lpTranslate[i].wCodePage);
			VerQueryValueA(pBlock, 
				SubBlock, 
				&FileVer, 
				&FileVerLen); 

		}
		if(FileVerName)
		{
			memcpy(FileVerName,FileVer,FileVerLen);
			FileVerName[FileVerLen]=0;
		}
		delete pBlock;
		return FileVerLen;
}