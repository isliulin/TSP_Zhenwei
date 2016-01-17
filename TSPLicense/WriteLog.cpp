#include "StdAfx.h"
#include "WriteLog.h"

CWriteLog* CWriteLog::m_pThis = NULL;

CWriteLog *CWriteLog::CreateLogInstance()
{
	if (NULL == m_pThis)
	{
		m_pThis = new CWriteLog();
	}

	return m_pThis;
}

void CWriteLog::DestroyLogInstance()
{
	SAFE_DELETE(m_pThis);
}


CWriteLog::CWriteLog(void)
{
	m_hLogFile = INVALID_HANDLE_VALUE;
	m_nLevel = LOG_LEVEL_3;
	m_lpBuf = new TCHAR[2*ONE_PACKAGE];
	::ZeroMemory(m_lpBuf, 2*ONE_PACKAGE);
	::ZeroMemory(m_szLogFilePath, sizeof(m_szLogFilePath));
	::ZeroMemory(m_szLogFileName, sizeof(m_szLogFileName));
	
	m_nDay = 0;
	InitPath();
}

CWriteLog::~CWriteLog(void)
{
	CloseLogFile();
	SAFE_DELARRAY(m_lpBuf);
}


BOOL CWriteLog::OpenLogFile(LPCTSTR strLogFileName)
{
	if (0 == lstrlen(strLogFileName))
	{
		return FALSE;
	}

	lstrcpy(m_szLogFileName, strLogFileName);

	CloseLogFile();

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
	if (INVALID_HANDLE_VALUE == m_hLogFile)
	{
		return TRUE;
	}

	::CloseHandle(m_hLogFile);
	m_hLogFile = INVALID_HANDLE_VALUE;

	return TRUE;
}


BOOL CWriteLog::SetLogLevel(LONG nLevel)
{
	if (nLevel < 0 || nLevel > LOG_LEVEL_3)
	{
		return FALSE;
	}

	m_nLevel = nLevel;

	return TRUE;
}

//调试输出(最大512个英文字符)
BOOL CWriteLog::WriteLog(LONG nLevel, LPCTSTR lpszFormat, ...)
{
	//_asm int 3
	if (nLevel > m_nLevel)
	{
		return FALSE;
	}

	::RtlZeroMemory(m_lpBuf, 2*ONE_PACKAGE);
	CTime dwtime = CTime::GetCurrentTime();

	if (nLevel==LOG_LEVEL_1){
		wsprintf(m_lpBuf, "%04d-%02d-%02d %02d:%02d:%02d ",\
			dwtime.GetYear(), dwtime.GetMonth(), dwtime.GetDay(), dwtime.GetHour(), dwtime.GetMinute(), dwtime.GetSecond());
	}

	if (m_nDay != dwtime.GetDay())
	{
		CHAR szLogFileName[MAX_PATH] = {0};
		wsprintf(szLogFileName, _T("%s\\%04d%02d%02d.txt"), m_szLogFilePath, dwtime.GetYear(), dwtime.GetMonth(), dwtime.GetDay());

		OpenLogFile(szLogFileName);
	}

	TCHAR *pTemp = m_lpBuf + lstrlen(m_lpBuf);
	va_list args = NULL;
	va_start(args, lpszFormat);
	int nLength = _vsntprintf_s(pTemp, ONE_PACKAGE, ONE_PACKAGE, lpszFormat, args);
	va_end(args);
	if (nLength <= 0) 
	{
		return FALSE;
	}

	//OutputDebugString(m_lpBuf);
	nLength = lstrlen(m_lpBuf);
	DWORD dwWrite = 0;
	return ::WriteFile(m_hLogFile, m_lpBuf, nLength*sizeof(TCHAR), &dwWrite, NULL);
}

//
////返回的路径末尾是带"\"分隔符的
CString GetExeCurrentDir()
{
	CString strCurPath;
	TCHAR szFullPath[MAX_PATH] = {0};
	TCHAR szDir[_MAX_DIR] = {0};
	TCHAR szDrive[_MAX_DRIVE] = {0};
	::GetModuleFileName(NULL, szFullPath, MAX_PATH);
	_tsplitpath_s(szFullPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, NULL, NULL, NULL);
	strCurPath.Format(_T("%s%s"), szDrive, szDir);

	return strCurPath;
}

BOOL CWriteLog::InitPath()
{
	wsprintf(m_szLogFilePath, _T("%s\\log\\"), GetExeCurrentDir());

	return CreateDirectory(m_szLogFilePath, NULL);
}