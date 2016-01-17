#pragma once
//#include "windows.h"



#include <windows.h>



//#define LOG(x) ( (void) (x) )
//#define LEVEL_ERROR   11

#define  SOCK_TIME_OUT  60  //超时时间为1分钟


class SockTCP
{
public:
	SockTCP(void);
	~SockTCP(void);
	BOOL Connect(char *szHost, UINT nPort);
	BOOL SendData(LPBYTE lpData, UINT nSize);
	int RecvData(LPBYTE lpBuffer, UINT nSize);


	void Disconnect();
private:
	SOCKET m_Socket;
};





