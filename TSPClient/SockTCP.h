#pragma once
#include "windows.h"

class SockTCP
{
public:
	SockTCP(void);
	~SockTCP(void);
	BOOL SockTCP::Connect(sockaddr_in *ServerAddr);
	BOOL Connect(char *szHost, UINT nPort);
	int SendData(LPBYTE lpData, UINT nSize);
	int RecvData(LPBYTE lpBuffer, UINT nSize);

private:
	void Disconnect();
private:
	SOCKET m_Socket;
};
