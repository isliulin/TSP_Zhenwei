#include "StdAfx.h"
#include "TSPClientPublic.h"
#include "WriteLog.h"

#define ONE_PACKAGE 1024
CWriteLog::CWriteLog(void)
{
	m_hLogFile = INVALID_HANDLE_VALUE;
	m_lpBuf = new TCHAR[2*ONE_PACKAGE];
	::RtlZeroMemory(m_lpBuf, 2*ONE_PACKAGE);

	char szPathLog[MAX_PATH] = {0};
	wsprintf(szPathLog, "%s\\log\\", GetExeCurrentDir());
	CreateDirectory(szPathLog, NULL);

	m_nDay = 0;
}

CWriteLog::~CWriteLog(void)
{
//	return;
	CloseLogFile();
	delete[] m_lpBuf;
}


BOOL CWriteLog::CreateNewLogFile(LPCTSTR strLogFileName)
{
//	return TRUE;
	if (0 == lstrlen(strLogFileName))
	{
		return FALSE;
	}

	SAFE_CLOSEFILEHANDLE(m_hLogFile);

	BOOL bRet = PathFileExists(strLogFileName);
	if (bRet)
	{
		m_hLogFile = ::CreateFile(strLogFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE != m_hLogFile)
		{
			DWORD dwRet = ::SetFilePointer(m_hLogFile, 0, NULL, FILE_END);
			if (HFILE_ERROR == dwRet)
			{
				TRACE(_T("设置日志文件到结尾失败\r\n"));
			}
		}
	}
	else
	{
		m_hLogFile = ::CreateFile(strLogFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	return INVALID_HANDLE_VALUE == m_hLogFile ? FALSE : TRUE;
}


BOOL CWriteLog::CloseLogFile()
{
//	return TRUE;
	if (INVALID_HANDLE_VALUE == m_hLogFile)
	{
		return TRUE;
	}

	return ::CloseHandle(m_hLogFile);
}



//调试输出(最大512个英文字符)
BOOL CWriteLog::WriteLog(TCHAR *szFileName,LPCTSTR lpszFormat, ...)
{
//	return TRUE;
	::RtlZeroMemory(m_lpBuf, 2*ONE_PACKAGE);
	CTime dwtime = CTime::GetCurrentTime();
	wsprintf(m_lpBuf, "%04d-%02d-%02d %02d:%02d:%02d ",\
		dwtime.GetYear(), dwtime.GetMonth(), dwtime.GetDay(), dwtime.GetHour(), dwtime.GetMinute(), dwtime.GetSecond());

	TCHAR *pTemp = m_lpBuf + lstrlen(m_lpBuf);
	va_list args = NULL;
	va_start(args, lpszFormat);
	int nLength = _vsntprintf_s(pTemp, ONE_PACKAGE, ONE_PACKAGE, lpszFormat, args);
	va_end(args);
	if (nLength <= 0) 
	{
		return FALSE;
	}

	//确定日志文件名称
// 	TCHAR szFileName[MAX_PATH] = {0};
// 	char *pPosBegin = StrStr(m_lpBuf, "线索编号");
// 	pPosBegin += 8;
// 	char *pPosEnd = StrStr(m_lpBuf, "\r\n");
// 	if (NULL == pPosBegin || 0 == lstrlen(pPosBegin) || pPosBegin == pPosEnd)
// 	{
// 		lstrcpy(szFileName, "other");
// 	}
// 	else
// 	{
// 		strncpy_s(szFileName, sizeof(szFileName)-1, pPosBegin, pPosEnd-pPosBegin);
// 
// 	}
		
	char szLogFileName[MAX_PATH] = {0};
	wsprintf(szLogFileName, "%s\\log\\%s.txt", GetExeCurrentDir(), szFileName);

	if (!CreateNewLogFile(szLogFileName))
	{
		return FALSE;
	}

	nLength = lstrlen(m_lpBuf);
	DWORD dwWrite = 0;
	// 	OutputDebugString(m_lpBuf);
	 ::WriteFile(m_hLogFile, m_lpBuf, nLength*sizeof(TCHAR), &dwWrite, NULL);
	 SAFE_CLOSEFILEHANDLE(m_hLogFile);

	 return TRUE;
}


