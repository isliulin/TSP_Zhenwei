#pragma once

class CWriteLog
{
public:
	CWriteLog(void);
	~CWriteLog(void);

	BOOL CreateNewLogFile(LPCTSTR strLogFileName);
	BOOL CloseLogFile();
	BOOL WriteLog(TCHAR *szFileName,LPCTSTR lpszFormat, ...);

private:
	HANDLE m_hLogFile;
	int m_nDay;
	TCHAR *m_lpBuf;
};
