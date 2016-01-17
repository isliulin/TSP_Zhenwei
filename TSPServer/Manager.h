#ifndef __TSP_MANAGER_H__
#define __TSP_MANAGER_H__

#include "windows.h"
#include "macros.h"
#include "until.h"

//»•µÙ“¿¿µ
class CClientSocket;

class CManager  
{
	friend class CClientSocket;

public:
	CManager(CClientSocket *pClient);
	virtual ~CManager();

public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	int Send(LPBYTE lpData, UINT nSize);

	CClientSocket *m_pClient;
	HANDLE m_hEventDlgOpen;

	void WaitForDialogOpen();
	void NotifyDialogIsOpen();
};

#endif //__TSP_MANAGER_H__
