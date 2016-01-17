//#include "StdAfx.h"

#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#include "SockTCP.h"
#include "Log.h"





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

	//LOG((LEVEL_INFO,"进入Disconnect开始执行setsockopt\n"));

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct) );


	//LOG((LEVEL_INFO,"开始执行Disconnect::CancelIo\n"));

	CancelIo((HANDLE) m_Socket);

	//LOG((LEVEL_INFO,"开始执行Disconnect::closesocket\n"));

	closesocket(m_Socket);


	//LOG((LEVEL_INFO,"执行Disconnect执行完毕\n"));

	m_Socket=INVALID_SOCKET;
}

BOOL SockTCP::Connect(char *szHost, UINT nPort)
{
	Disconnect();
	BOOL bRet=FALSE;

	do 
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

		

		if (m_Socket == SOCKET_ERROR)   
		{ 
			LOG((LEVEL_ERROR,"SockTCP::Connect-socket erro %d\r\n",WSAGetLastError()));

			break;   
		}


		DWORD TimeOut=SOCK_TIME_OUT * 1000; //设置发送超时10秒
		if(::setsockopt(m_Socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
			LOG((LEVEL_ERROR,"SockTCP::Connect-setsockopt1 erro%d\r\n",WSAGetLastError()));

			return bRet;
		}

		TimeOut=SOCK_TIME_OUT * 1000;//设置接收超时秒
		if(::setsockopt(m_Socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){

			LOG((LEVEL_ERROR,"SockTCP::Connect-setsockopt2 erro%d\r\n",WSAGetLastError()));

			return bRet;
		}

		//设置为非阻塞方式连接
		unsigned long ul = 1;
		ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);


		sockaddr_in	ServerAddr;
		ServerAddr.sin_family	= AF_INET;

		ServerAddr.sin_port	= htons((USHORT)nPort);

		DWORD dwIP=inet_addr(szHost);
		if (dwIP==INADDR_NONE)
		{
			hostent *hp = NULL;
			if ((hp = gethostbyname(szHost))== NULL)
			{
				break;

			}
			memcpy(&dwIP, hp->h_addr, hp->h_length);

// 			IN_ADDR inaddr={0};
// 			inaddr.S_un.S_addr=dwIP;
		}

		ServerAddr.sin_addr.s_addr = dwIP;

		connect(m_Socket,(const struct sockaddr *)&ServerAddr,sizeof(ServerAddr)); //立即返回

		//select 模型，即设置超时
		struct timeval timeout ;
		fd_set r;

		FD_ZERO(&r);
		FD_SET(m_Socket, &r);
		timeout.tv_sec = SOCK_TIME_OUT; //连接超时30秒
		timeout.tv_usec =0;
		if(select(0, 0, &r, 0, &timeout)<=0) 
		{
// 			IN_ADDR inaddr={0};
// 			inaddr.S_un.S_addr=nIP;
			LOG((LEVEL_ERROR,"SockTCP::Connect %s -select erro %d\r\n",szHost,GetLastError()));
			break;
		}
		
		ul = 0;
		ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);

		bRet=TRUE;

	} while (FALSE);

	if (!bRet) Disconnect();
	return bRet;

}


int SockTCP::RecvData(LPBYTE lpBuffer, UINT nSize)
{
   /*

	//select 模型，即设置超时
	struct timeval timeout ;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(m_Socket, &r);
	timeout.tv_sec = SOCK_TIME_OUT; //连接超时
	timeout.tv_usec =0;

	

	if(select(0, 0, &r, 0, &timeout)<=0)
	{
		LOG((LEVEL_ERROR,"SockTCP::RecvData-select erro %d\r\n",WSAGetLastError()));

		return SOCKET_ERROR;
	}

	*/


	return recv(m_Socket,(char*)lpBuffer,nSize,0);
}

int  SockTCP::SendData(LPBYTE lpData, UINT nSize)
{
	return send(m_Socket,(char*)lpData,nSize,0);

}