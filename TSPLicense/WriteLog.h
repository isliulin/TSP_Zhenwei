#pragma once

class CWriteLog
{
public:
	static CWriteLog *CreateLogInstance();
	static void DestroyLogInstance();

	BOOL SetLogLevel(LONG nLevel);
	BOOL WriteLog(LONG nLevel, LPCTSTR lpszFormat, ...);
	LPCTSTR GetLogFileName() {return m_szLogFileName;}

private:
	BOOL OpenLogFile(LPCTSTR strLogFileName);
	BOOL CloseLogFile();
	BOOL InitPath();

	CWriteLog(void);
	~CWriteLog(void);
private:
	static CWriteLog *m_pThis;

	HANDLE m_hLogFile;
	int m_nDay;
	int m_nLevel;
	TCHAR *m_lpBuf;
	TCHAR m_szLogFilePath[MAX_PATH];
	CHAR m_szLogFileName[MAX_PATH];
};
