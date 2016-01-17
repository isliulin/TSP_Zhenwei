#include "StdAfx.h"
#include "winsock2.h"
#include "SockTCP.h"


SockTCP::SockTCP(void):
	m_Socket(INVALID_SOCKET)
{
}

SockTCP::~SockTCP(void)
{
	if (m_Socket != INVALID_SOCKET)
	{
		Disconnect();
	}
}

void SockTCP::Disconnect()
{   
	LINGER lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 0;

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) );

	CancelIo((HANDLE) m_Socket);

	closesocket(m_Socket);
	m_Socket=INVALID_SOCKET;
}

BOOL SockTCP::Connect(sockaddr_in *ServerAddr)
{
	Disconnect();
	BOOL bRet=FALSE;

	do 
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

		if (m_Socket == SOCKET_ERROR)   
		{ 
			break;   
		}


		DWORD TimeOut=10000; //  send time out
		if(::setsockopt(m_Socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
			return bRet;
		}

		TimeOut=6000*10;//recv time out
		if(::setsockopt(m_Socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
			return bRet;
		}

		//设置为非阻塞方式连接
		unsigned long ul = 1;
		ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);

		connect(m_Socket,(const struct sockaddr *)ServerAddr,sizeof(sockaddr_in)); //立即返回

		//select 模型，即设置超时
		struct timeval timeout ;
		fd_set r;

		FD_ZERO(&r);
		FD_SET(m_Socket, &r);
		timeout.tv_sec = 30; //连接超时30秒
		timeout.tv_usec =0;
		if(select(0, 0, &r, 0, &timeout)<=0) 
		{
			break;
		}

		ul = 0;
		ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);

		bRet=TRUE;

	} while (FALSE);

	if (!bRet) Disconnect();
	return bRet;
}

BOOL SockTCP::Connect(char *szHost, UINT nPort)
{

	if (szHost==NULL)
	{
		return FALSE;
	}

	sockaddr_in	ServerAddr;
	ServerAddr.sin_family	= AF_INET;

	ServerAddr.sin_port	= htons((USHORT)nPort);

	DWORD dwIP=inet_addr(szHost);
	if (dwIP==INADDR_NONE)
	{
		hostent *hp = NULL;
		if ((hp = gethostbyname(szHost))== NULL)
		{
			return FALSE;

		}
		memcpy(&dwIP, hp->h_addr, hp->h_length);

	}
	return Connect(&ServerAddr);
}


int SockTCP::RecvData(LPBYTE lpBuffer, UINT nSize)
{
	//select 模型，即设置超时
	struct timeval timeout ;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(m_Socket, &r);
	timeout.tv_sec = 60; //连接超时60秒
	timeout.tv_usec =0;
	if(select(0, 0, &r, 0, &timeout)<=0)
	{
		return SOCKET_ERROR;
	}
	return recv(m_Socket,(char*)lpBuffer,nSize,0);
}

int  SockTCP::SendData(LPBYTE lpData, UINT nSize)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	UINT		size = 0;
	int			nSend = 0;
	int			nSendRetry = 15;

	DWORD nSplitSize=1024;

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