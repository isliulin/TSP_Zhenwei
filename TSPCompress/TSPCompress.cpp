// TSPCompress.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "zlib.h"

//#include "..\\WdkPath\common.h"
#pragma comment(lib, "zlib-1.1.4.lib")
//#pragma comment(lib,"foxsdkA.lib")
#include "shlwapi.h"


#include "atlconv.h"
#include   <Objbase.h>
#include   <Gdiplus.h>
#include   <GdiPlusEnums.h>
using   namespace   Gdiplus;
#pragma   comment(lib, "gdiplus.lib")
#include "GdiplusInit.h"

#include "../TSPServer/until.h"
#include "io.h"

enum
{
	ERR_OK = 0,
	ERR_COMPRESS_CREATEFILE,
	ERR_COMPRESS_FILESIZE,
	ERR_COMPRESS_READFILE,
	ERR_COMPRESS_PROCESS,
	ERR_COMPRESS_WRITEFILE,
};



int UnCompressFile(TCHAR *lpSrcFileName, TCHAR*lpDesFileName)
{
	int nRet=0;
	HANDLE hFile=INVALID_HANDLE_VALUE;
	HANDLE hFileOut=INVALID_HANDLE_VALUE;
	BYTE *pBufferRead=NULL;
	BYTE *pBufUnCompress=NULL;

	do 
	{
		hFile= CreateFile (lpSrcFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
		//	PrintString("createfile %s erro %d",lpSrcFileName,GetLastError());
			break;
		}



		int nCompressSize = GetFileSize(hFile, NULL);
		pBufferRead = new BYTE[nCompressSize];
		DWORD dwRead = 0;
		BOOL bRet = ReadFile(hFile, pBufferRead, nCompressSize, &dwRead, NULL);
		if (!bRet || nCompressSize != dwRead)
		{
	//		PrintString("ReadFile %s erro %d",lpSrcFileName,GetLastError());
			break;
		}

		ULONG nUnCompressSize = (ULONG)pBufferRead;

		pBufUnCompress = new BYTE[nUnCompressSize];
		memset(pBufUnCompress,0,nUnCompressSize);
		int nProcess = uncompress(pBufUnCompress, &nUnCompressSize, pBufferRead+4, nCompressSize-4);
		if (Z_OK != nProcess&&strlen((char*)pBufUnCompress)==0)
		{
	//		PrintString("uncompress erro %d %d",nProcess,pBufUnCompress[0]);
			break;
		}

		//写入输出文件

		hFileOut= CreateFile (lpDesFileName, GENERIC_READ|GENERIC_WRITE,0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFileOut)
		{
			//	PrintString("createfile %s erro %d",lpDesFileName,GetLastError());
			break;
		}

		dwRead = 0;
		bRet = WriteFile(hFileOut, pBufUnCompress, nUnCompressSize, &dwRead, NULL);
		if (!bRet || nUnCompressSize != dwRead)
		{
	//		PrintString("WriteFile  %s erro %d ",lpDesFileName,GetLastError());
			break;
		}
		nRet=1;


	} while (FALSE);

	if (pBufUnCompress)  delete pBufUnCompress;
	if(pBufferRead) delete pBufferRead;

	CloseHandle(hFile);
	CloseHandle(hFileOut);

	return nRet;
}


int  GetEncoderClsid(const   WCHAR*   format,   CLSID*   pClsid)
{
	UINT     num   =   0;                     //   number   of   image   encoders
	UINT     size   =   0;                   //   size   of   the   image   encoder   array   in   bytes

	ImageCodecInfo*   pImageCodecInfo   =   NULL;

	GetImageEncodersSize(&num,   &size);
	if(size   ==   0)
		return   -1;     //   Failure

	pImageCodecInfo   =   (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo   ==   NULL)
		return   -1;     //   Failure

	GetImageEncoders(num,   size,   pImageCodecInfo);

	for(UINT   j   =   0;   j   <   num;   ++j)
	{
		if(   wcscmp(pImageCodecInfo[j].MimeType,   format)   ==   0   )
		{
			*pClsid   =   pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return   j;     //   Success
		}        
	}

	free(pImageCodecInfo);
	return   -1;     //   Failure
} 


//Polymorphic Engine
//引擎初始化

void ASCIIToUnicode(char* ASCIIString,wchar_t* WideChar)
{
	int AStrLen;


	AStrLen=MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,0);

	//	AStrLen=WideCharToMultiByte(CP_ACP,0,WideChar,wcslen(WideChar),ASCIIString,0,NULL,NULL);


	MultiByteToWideChar(CP_ACP,0,ASCIIString,strlen(ASCIIString),WideChar,AStrLen);
}


BOOL BmpToPng(TCHAR *szBmp,TCHAR *szPng)
{

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	wchar_t wBmpFile[MAX_PATH]={0};
	wchar_t wPngFile[MAX_PATH]={0};

	ASCIIToUnicode(szBmp,wBmpFile);
	ASCIIToUnicode(szPng,wPngFile);
	//OutputDebugStringA(szPng);
	//OutputDebugStringW(wPngFile);


		Status  stat;
	//A2W

	//Image*   image = new Image(A2W(szBmp));

	Image*   image = new Image(wBmpFile);

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);

	stat=image->Save(wPngFile, &encoderClsid, NULL);
	delete image;
	GdiplusShutdown(gdiplusToken);
	if(stat == Ok)
		DeleteFileA(szBmp);

	return TRUE;

}


void CopyFile(char *szOldFile,char *szNewFile)
{

	HANDLE	hOldFile = CreateFile(szOldFile,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD dwSize = GetFileSize(hOldFile, NULL);

	HANDLE	hNewFile = CreateFile(szNewFile, GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	SetFilePointer(hNewFile, 0, 0, FILE_END);

	LPBYTE	lpBuffer = new BYTE[dwSize];
	memset(lpBuffer,0,dwSize);
	DWORD dwBytesWrite = 0;
	ReadFile(hOldFile,lpBuffer,dwSize,&dwBytesWrite,0);

	for (DWORD i = 0; i < dwSize; i++)
	{
		lpBuffer[i] ^= XOR_ENCODE_VALUE;
	}

	WriteFile(hNewFile, lpBuffer, dwSize, &dwBytesWrite, NULL);
	CloseHandle(hNewFile);
	CloseHandle(hOldFile);
	delete lpBuffer;
}

BOOL   CopyDirectory(LPTSTR strSrcPath, LPTSTR strDestPath)
{
	HANDLE                     HResult;
	WIN32_FIND_DATA   FindFileData={0};
	//BOOL                         blResult;
	TCHAR szBMPName[MAX_PATH]={0};
	TCHAR szDesBMPName[MAX_PATH]={0};

	HResult   =     FindFirstFile(strSrcPath,&FindFileData);

	if(HResult==INVALID_HANDLE_VALUE){
		return  FALSE;
	}

	FindClose(HResult);

	if(strSrcPath[strlen(strSrcPath)-1]=='\\') strSrcPath[strlen(strSrcPath)-1]='\0'; 

	if(strDestPath[strlen(strDestPath)-1]=='\\')	strDestPath[strlen(strDestPath)-1]='\0';

	HResult=FindFirstFile(strDestPath,&FindFileData);

	if(HResult==INVALID_HANDLE_VALUE){
		CreateDirectory(strDestPath,NULL);
	}
	FindClose(HResult);


	TCHAR strSrcFileName[MAX_PATH]={0};
	wsprintf(strSrcFileName,"%s\\*",strSrcPath);

	HResult =FindFirstFile(strSrcFileName,&FindFileData);

	if(HResult==INVALID_HANDLE_VALUE)
	{
		return   FALSE;
	}
	TCHAR strDestFileName[MAX_PATH];

	do 
	{

		memset(strSrcFileName,0,MAX_PATH);
		memset(strDestFileName,0,MAX_PATH);

		if(StrCmp(FindFileData.cFileName,_T("."))&&StrCmp(FindFileData.cFileName,_T("..")))
		{
			wsprintf(strSrcFileName,"%s\\%s",strSrcPath,FindFileData.cFileName);
			wsprintf(strDestFileName,"%s\\%s",strDestPath,FindFileData.cFileName);


			if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
			{
				CopyDirectory(strSrcFileName,strDestFileName);
			}
			else
			{
				LPSTR strPos=strrchr(strDestFileName,'.');

				if(_access(strSrcFileName,6)==-1) continue;

				//if (!strPos) strPos=strrchr(strDestFileName,'dat');
				
				if (strPos&&*(strPos+4)==0)
				{
					if (strPos&&strstr(strPos,"inf")) CopyFile(strSrcFileName,strDestFileName);
					else CopyFile(strSrcFileName,strDestFileName,FALSE);
					DeleteFile(strSrcFileName);

				}
				else
				{
					if(UnCompressFile(strSrcFileName,strDestFileName))
					{
						char szpng[MAX_PATH] = {0};
						strcpy(szpng,strDestFileName);
						char *szPos=strrchr(szpng,'.');
						if (szPos&&strstr(strPos,"bmp"))
						{
							*szPos=0;
							strcat(szpng,".png");
							BmpToPng(strDestFileName,szpng);
						}
// 						else if (szPos&&StrStrI(strPos,"Msg2.0.dbdat"))
// 						{
// 							*(szPos+3)=0;
// 							MoveFileEx(strDestFileName,szpng,MOVEFILE_REPLACE_EXISTING);
// 						}
						else if(szPos)
						{
							*(szPos+4)=0;
							MoveFileEx(strDestFileName,szpng,MOVEFILE_REPLACE_EXISTING);
						}
						DeleteFile(strSrcFileName);

					}

				}

			}
		}

	} while (FindNextFile(HResult,&FindFileData));

	FindClose(HResult);
	return TRUE;


}

// BOOL   CopyDirectory1(LPTSTR strSrcPath, LPTSTR strDestPath)
// {
// 	HANDLE                     HResult;
// 	WIN32_FIND_DATA   FindFileData={0};
// 	BOOL                         blResult;
// 	TCHAR szBMPName[MAX_PATH]={0};
// 	TCHAR szDesBMPName[MAX_PATH]={0};
// 
// 	HResult   =     FindFirstFile(strSrcPath,&FindFileData);
// 
// 	if(HResult==INVALID_HANDLE_VALUE){
// 		return  FALSE;
// 	}
// 
// 	FindClose(HResult);
// 
// 	if(strSrcPath[strlen(strSrcPath)-1]=='\\')
// 	{
// 		strSrcPath[strlen(strSrcPath)-1]='\0';
// 	}
// 
// 	if(strSrcPath[strlen(strDestPath)-1]=='\\')
// 	{
// 		strSrcPath[strlen(strDestPath)-1]='\0';
// 	}
// 
// 	HResult=FindFirstFile(strDestPath,&FindFileData);
// 
// 	if(HResult==INVALID_HANDLE_VALUE)
// 	{
// 		CreateDirectory(strDestPath,NULL);
// 	}
// 	FindClose(HResult);
// 
// 
// 	TCHAR strSrcFileName[MAX_PATH]={0};
// 	StrCpy(strSrcFileName,strSrcPath);
// 	StrCat(strSrcFileName, "\\*");
// 
// 	HResult =FindFirstFile(strSrcFileName,&FindFileData);
// 
// 	if(HResult==INVALID_HANDLE_VALUE)
// 	{
// 		return   FALSE;
// 	}
// 
// 
// 	TCHAR strDestFileName[MAX_PATH]={0};
// 
// 	if(StrCmp(FindFileData.cFileName,_T( "."))&& StrCmp(FindFileData.cFileName ,_T( "..")))
// 	{
// 		StrCpy(strSrcFileName,strSrcPath);
// 		StrCat(strSrcFileName, "\\");
// 		StrCat(strSrcFileName,FindFileData.cFileName);
// 		StrCpy(strDestFileName,strDestPath);
// 		StrCat(strDestFileName, "\\");
// 		StrCat(strDestFileName,FindFileData.cFileName);
// 
// 		if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
// 
// 		{
// 			CopyDirectory(strSrcFileName,strDestFileName);
// 		}
// 		else
// 		{
// 			LPSTR strPos=strrchr(strDestFileName,'.');
// 			if (*(strPos+4)==0)
// 			{
// 				if (strstr(strPos,"inf")) CopyFile(strSrcFileName,strDestFileName);
// 				else CopyFile(strSrcFileName,strDestFileName,FALSE);
// 				DeleteFile(strSrcFileName);
// 			}
// 			else
// 			{
// 				if(UnCompressFile(strSrcFileName,strDestFileName))
// 				{
// 					
// 					char szpng[MAX_PATH] = {0};
// 					strcpy(szpng,strDestFileName);
// 					char *szPos=strrchr(szpng,'.');
// 					if (strstr(strPos,"bmp"))
// 					{
// 						*szPos=0;
// 						strcat(szpng,".png");
// 						BmpToPng(strDestFileName,szpng);
// 					}
// 					else
// 					{
// 						*(szPos+4)=0;
// 						MoveFileEx(strDestFileName,szpng,MOVEFILE_REPLACE_EXISTING);
// 					}
// 					DeleteFile(strSrcFileName);
// 
// 
// 				}
// 
// 
// 
// 			}
// 		}
// 	}
// 	while(1)
// 	{
// 		blResult = FindNextFile(HResult,&FindFileData);
// 		if(!blResult)
// 			break;
// 
// 		if(StrCmp(FindFileData.cFileName,_T("."))&&StrCmp(FindFileData.cFileName,_T("..")))
// 		{
// 			StrCpy(strSrcFileName,strSrcPath);
// 			StrCat(strSrcFileName, "\\");
// 			StrCat(strSrcFileName,FindFileData.cFileName);
// 			StrCpy(strDestFileName,strDestPath);
// 			StrCat(strDestFileName, "\\");
// 			StrCat(strDestFileName,FindFileData.cFileName);
// 
// 			if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
// 			{
// 				CopyDirectory(strSrcFileName,strDestFileName);
// 			}
// 			else
// 			{
// 				LPSTR strPos=strrchr(strDestFileName,'.');
// 
// 				if (*(strPos+4)==0)
// 				{
// 					if (strstr(strPos,"inf")) CopyFile(strSrcFileName,strDestFileName);
// 					else CopyFile(strSrcFileName,strDestFileName,FALSE);
// 					DeleteFile(strSrcFileName);
// 				}
// 				else
// 				{
// 					char szpng[MAX_PATH] = {0};
// 					strcpy(szpng,strDestFileName);
// 					char *szPos=strrchr(szpng,'.');
// 					if (strstr(strPos,"bmp"))
// 					{
// 						*szPos=0;
// 						strcat(szpng,".png");
// 						BmpToPng(strDestFileName,szpng);
// 					}
// 					else
// 					{
// 						*(szPos+4)=0;
// 						MoveFileEx(strDestFileName,szpng,MOVEFILE_REPLACE_EXISTING);
// 					}
// 					DeleteFile(strSrcFileName);
// 				}
// 
// 			}
// 		}
// 	}
// 
// 	FindClose(HResult);
// 
// 
// 	return   TRUE;
// } 

int UnCompressFolder(TCHAR *lpSrcFolder, TCHAR*lpDesFolder)
{
// 	TCHAR szBMPName[MAX_PATH]={0};
// 	TCHAR szDesBMPName[MAX_PATH]={0};



		while(TRUE)
		{
			CopyDirectory(lpSrcFolder,lpDesFolder);				

			Sleep(1000*10);
		}

	return 0;

}


int CompressFile(TCHAR *lpSrcFileName, TCHAR*lpDesFileName)
{
	HANDLE hFile = CreateFile(lpSrcFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return ERR_COMPRESS_CREATEFILE;
	}

	HANDLE hFileOut = CreateFile(lpDesFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFileOut)
	{
		CloseHandle(hFile);
		return ERR_COMPRESS_CREATEFILE;
	}

	int nRet = 0;
	do 
	{
		int nUnCompressSize = GetFileSize(hFile, NULL);
		if (nUnCompressSize > 1024*1024)
		{
			nRet = ERR_COMPRESS_FILESIZE;
			break;
		}

		BYTE *pBufferRead = new BYTE[nUnCompressSize];
		DWORD dwRead = 0;
		BOOL bRet = ReadFile(hFile, pBufferRead, nUnCompressSize, &dwRead, NULL);
		if (!bRet || nUnCompressSize != dwRead)
		{
			nRet = ERR_COMPRESS_READFILE;
			break;
		}

		ULONG nCompressSize = nUnCompressSize;

		BYTE *pBufCompress = new BYTE[nCompressSize];
		int nProcess = compress(pBufCompress, &nCompressSize, pBufferRead, nUnCompressSize);
		if (Z_OK != nProcess || 0 >= nUnCompressSize)
		{
			nRet = ERR_COMPRESS_PROCESS;
			break;
		}

		//写入输出文件
		dwRead = 0;
		bRet = WriteFile(hFileOut, &nUnCompressSize, sizeof(nUnCompressSize), &dwRead, NULL);
		if (!bRet || sizeof(nUnCompressSize) != dwRead)
		{
			nRet = ERR_COMPRESS_WRITEFILE;
			break;
		} 

		dwRead = 0;
		bRet = WriteFile(hFileOut, pBufCompress, nCompressSize, &dwRead, NULL);
		if (!bRet || nCompressSize != dwRead)
		{
			nRet = ERR_COMPRESS_WRITEFILE;
			break;
		}

	} while (FALSE);

	CloseHandle(hFile);
	CloseHandle(hFileOut);

	return nRet;
}

bool IsDirectory(char* filename)
{

	DWORD dwAttr = ::GetFileAttributes(filename);  //得到文件属性

	if (dwAttr == 0xFFFFFFFF)    // 文件或目录不存在

		return false;

	else if (dwAttr&FILE_ATTRIBUTE_DIRECTORY)  // 如果是目录

		return true; 

	else

		return false;

}

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR szMessage[256] = {0};
	int nRet = 0;
	do 
	{	
		if (4 > argc)
		{
//			sprintf_s(szMessage, _T("useage: TSPCompress.exe d  srcFolder desFolder bmp"));		
 
			sprintf_s(szMessage, _T("useage: TSPCompress.exe d/e srcFilename desFilename szExtentName"));		
			break;
		}

		TCHAR *pFileSrc = argv[2];
		TCHAR *pFileDes = argv[3];




 		if (!stricmp(argv[1],"d"))
		{
			if (IsDirectory(pFileSrc))
			{
				nRet=UnCompressFolder(pFileSrc,pFileDes);
			}
			else 
				nRet = UnCompressFile(pFileSrc, pFileDes);
		}
		else if (!stricmp(argv[1],"e"))
		{
			if (!PathFileExists(pFileSrc))
			{
				wsprintf(szMessage, _T("srcFile not found"));
				break;
			}

			nRet = CompressFile(pFileSrc, pFileDes);
		}

		wsprintf(szMessage, _T("process over return code = %d"), nRet);

	} while (FALSE);

	printf(szMessage);

	return 0;
}

