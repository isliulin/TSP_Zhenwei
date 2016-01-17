#ifndef __TSPCOMMON_LICENSEDEF_H__
#define  __TSPCOMMON_LICENSEDEF_H__

enum
{
	DOWN_SERVER = 0x09,
//	DOWN_SERVER_NEW,
	FILE_BUFFER,
	FILE_FINISH,
	GET_CONFIG,
	GET_VERSION,
	DOWN_CLIENT,
	GET_VERSIONSER,
	GET_CLIENTUPDATAINFO,
	GET_SERVERUPDATAINFO,
	DOWN_FILE,
	DOWN_SERVER_NEW,
	DOWN_WUHAN_SERVER,
	GET_FILE_VERSION,
	DOWN_LINYI_SERVER,
	DOWN_UTRANSMISSION,
	DOWN_TSP20_SERVER,
};

typedef struct
{
	BYTE	Flags;
	DWORD   dwSizeTotal;
	DWORD	Buffer_Size;
	BYTE	Buffer[1024];
}NET_DATA, *LPNET_DATA;

#define RECV_MAX_BUF 1024*8

inline int MySend(SOCKET sock,LPBYTE lpData, UINT nSize)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	int			nSendRetry = 15;
	int i;
	for (i = 0; i < nSendRetry; i++)
	{
		nRet = send(sock, pbuf, nSize,0);
		if (nRet>0)
			return nRet;
	}
	if (i == nSendRetry)
		return SOCKET_ERROR;
	return 0;
}

inline int MyRecv(SOCKET sock,LPBYTE lpData, UINT nSize)
{
	int			nRet = 0;
	char	*pbuf = (char *)lpData;
	int			nRecvRetry = 15;
	int i;
	for (i = 0; i < nRecvRetry; i++)
	{
		nRet = recv(sock, pbuf, nSize,0);
		if (nRet>0)
			return nRet;
	}
	if (i == nRecvRetry)
		return SOCKET_ERROR;
	return 0;
}


#endif//__TSPCOMMON_LICENSEDEF_H__