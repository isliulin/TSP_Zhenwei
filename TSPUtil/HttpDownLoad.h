#pragma once
#include "windows.h"
#include "SockTCP.h"

#define MAXHEADERSIZE 4096

class CHttpDownLoad:public SockTCP
{
public:
	
	static int UrlDownLoad2File(const char *pUrl,const char *_chFullFileName);
	
	enum
	{
		S_FULLDOWN,
		S_PARTDOWN,
		S_ERRO,
	};



	int UrlDownLoadToFile(const char *pUrl,const char *_chFullFileName);

	

	CHttpDownLoad(void);
	~CHttpDownLoad(void);
private:

	BOOL SendRequest();
	const char* GetResponseHeader();
	
	int GetField(const char *szSession, char *szValue, int nMaxLength);
	const char *FormatRequestHeader();
	int UrlDownLoad();
	BOOL GetHostAndPortFormUrl(char *_pUrl);


private:
	//SockTCP m_TcpSocket;

	char m_szHost[MAX_PATH];
	WORD m_wPort;
	char m_szRequestSource[MAXHEADERSIZE];
	HANDLE m_hOutFileHandle;

	DWORD m_dwFileLen;
	DWORD m_dwOffset;

	//char m_szOutSaveFile[MAX_PATH];


	char m_requestheader[MAXHEADERSIZE];

	char m_ResponseHeader[MAXHEADERSIZE];	//��Ӧͷ

	BOOL m_bResponsed;				//�Ƿ��Ѿ�ȡ���˷���ͷ
	int m_nResponseHeaderSize;		//��Ӧͷ�Ĵ�С
};
