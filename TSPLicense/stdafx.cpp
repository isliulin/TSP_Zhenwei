// stdafx.cpp : 只包括标准包含文件的源文件
// TSPLoginSrv.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
//#include "windows.h"

void GetClientDNS(SOCKET AcceptSocket,TCHAR *szDNS)
{
	if (!szDNS){
		return;
	}
	sockaddr_in clientaddr;
	memset(&clientaddr,0,sizeof(sockaddr_in));
	int dwclientaddrsize=sizeof(sockaddr_in);
	getpeername(AcceptSocket,(struct sockaddr*)&clientaddr,&dwclientaddrsize);

 	StrCpy(szDNS,inet_ntoa(clientaddr.sin_addr));
//	TCHAR szPort[MAX_PATH]={0};
//	_itoa_s(ntohs(clientaddr.sin_port),szPort,10);
//	StrCat(szDNS,szPort);
//	wsprintf(szDNS,"%s:%s",inet_ntoa(clientaddr.sin_addr),szPort);

	return;
}