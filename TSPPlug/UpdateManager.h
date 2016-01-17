#pragma once
#include <wininet.h>
#include <shlobj.h>

#include <Windows.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shell32.lib")  

struct UpdateFileIoFO
{
	char m_sSoftwareName[128];
	//int  m_sSoftwareVer;
	int  m_sSoftwaresize;
	char m_sSoftwareURL[MAX_PATH];
	char m_sSoftwareMD5[33];

};

class CUpdateManager
{
public:
	CUpdateManager(void);
	~CUpdateManager(void);

	int  GetFileFromServer(char *szURl,char *szSrcFileName,char *szDesPath,char *szDesFileName,int SrcFileSize);
	static int GetFileFromServerImp(const char *szURl,const char *szDes,int SrcFileSize);

public:
	char              m_strMD5[33];
	char              m_szUrl[MAX_PATH];
	char              m_szDownloadFileName[MAX_PATH];
	int               m_port;
	//int               m_nTime;
	int               m_nUpdateSoftNum;
	UpdateFileIoFO    m_pUpdateFileInfo[30];

};