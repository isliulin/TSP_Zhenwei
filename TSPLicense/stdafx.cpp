// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// TSPLoginSrv.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

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