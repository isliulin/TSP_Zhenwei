//#include "StdAfx.h"
#include "HttpDownLoad.h"
#include "stdio.h"
#include "Log.h"


CHttpDownLoad::CHttpDownLoad(void)
{
	
	ZeroMemory(m_requestheader,sizeof(m_requestheader));
	ZeroMemory(m_ResponseHeader,sizeof(m_ResponseHeader));
	ZeroMemory(m_szHost,sizeof(m_szHost));
	ZeroMemory(m_szRequestSource,sizeof(m_szRequestSource));
	//ZeroMemory(m_szOutSaveFile,sizeof(m_szOutSaveFile));


	m_hOutFileHandle=INVALID_HANDLE_VALUE;
	m_bResponsed=FALSE;
	m_nResponseHeaderSize=0;
	m_wPort=0;
	m_dwOffset=0;
	m_dwFileLen=0;
}

CHttpDownLoad::~CHttpDownLoad(void)
{
	if (m_hOutFileHandle!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hOutFileHandle);
		m_hOutFileHandle=INVALID_HANDLE_VALUE;
	}
	
}


///发送请求头
BOOL CHttpDownLoad::SendRequest()
{

	if(SendData((LPBYTE)m_requestheader,strlen(m_requestheader))<=0)
	{
		LOG((LEVEL_ERROR,"CHttpDownLoad::SendRequest m_TcpSocket.SendData erro%d\r\n",WSAGetLastError()));
		return FALSE;
	}

//	int nLength;
	GetResponseHeader();

	return (m_bResponsed) ? TRUE: FALSE;


}


//获取HTTP请求的返回头
const char* CHttpDownLoad::GetResponseHeader()
{

	if (!m_bResponsed)
	{
		char c = 0;
		int nIndex = 0;
		BOOL bEndResponse = FALSE;
		while(!bEndResponse && nIndex < MAXHEADERSIZE)
		{
			if(RecvData((LPBYTE)&c,1)<=0)
			{
				LOG((LEVEL_ERROR,"CHttpDownLoad::GetResponseHeader m_TcpSocket.RecvData erro%d\r\n",WSAGetLastError()));
				return NULL;
			}

			m_ResponseHeader[nIndex++] = c;
			c=0;
			if(nIndex >= 4)
			{
				if(m_ResponseHeader[nIndex - 4] == '\r' &&
					m_ResponseHeader[nIndex - 3] == '\n'&&
					m_ResponseHeader[nIndex - 2] == '\r' &&
					m_ResponseHeader[nIndex - 1] == '\n'
					)
				{
					bEndResponse = TRUE;
					m_bResponsed = TRUE;
				}
			}
		}
		m_nResponseHeaderSize = nIndex;
		
	}

	return m_ResponseHeader;
}


///根据请求的相对URL输出HTTP请求头
const char *CHttpDownLoad::FormatRequestHeader()
{


	///第1行:方法,请求的路径,版本

	//	sprintf(m_requestheader,"POST %s HTTP/1.1\r\nHhost:%s\r\n",pObject,);
	strcat(m_requestheader,"GET ");
	strncat(m_requestheader,m_szRequestSource,1024);
	strcat(m_requestheader," HTTP/1.1\r\n");
 
	//Accept: */*\r\n

	strcat(m_requestheader,"Accept: */*\r\n");

	//Accept-Language: zh-cn\r\n
	strcat(m_requestheader,"Accept-Language: zh-cn\r\n");

    //Accept-Encoding: gzip, deflate\r\n
	strcat(m_requestheader,"Accept-Encoding: gzip, deflate\r\n");


    strcat(m_requestheader,"Host:");
	strncat(m_requestheader,m_szHost,128);
    strcat(m_requestheader,"\r\n");

	///第3行:

// 	strcat(m_requestheader,"Content-Length:");
// 	strcat(m_requestheader,"0");
// 	strcat(m_requestheader,"\r\n");

	///第4行:

// 	strcat(m_requestheader,"Cache-Control:");
// 	strcat(m_requestheader,"no-store");
// 	strcat(m_requestheader,"\r\n");

	//Connection: Keep-Alive\r\n
	strcat(m_requestheader,"Connection: Keep-Alive\r\n");

	if (m_dwOffset)
	{
		char szLen[20]={0};
		sprintf(szLen,"%d",m_dwOffset);
		//Range：bytes=1234-
		strcat(m_requestheader,"Range: bytes=");
		strcat(m_requestheader,szLen);
		strcat(m_requestheader,"-\r\n");
	}
	
	///最后一行:空行
	strcat(m_requestheader,"\r\n");

	///返回结果
	return m_requestheader;
}


int CHttpDownLoad::GetField(const char *szSession, char *szValue, int nMaxLength)
{
	//取得某个域值
	if(!m_bResponsed) return -1;

	char *szSession_header=strstr(m_ResponseHeader,szSession);

	if (szSession_header)
	{
		char *szSession_end=strstr(szSession_header+2,"\r\n");
		if (szSession_end)
		{
			DWORD dwSessionLen=szSession_end-szSession_header;
			DWORD headerlen=strlen(szSession)+2;
			dwSessionLen-=headerlen;

			if (dwSessionLen&&dwSessionLen<(DWORD)nMaxLength)
			{
				strncpy(szValue,szSession_header+headerlen,dwSessionLen);
				return dwSessionLen;
			}

		}
	}
	return -1;
}




BOOL CHttpDownLoad::GetHostAndPortFormUrl(char *_pUrl)
{

	BOOL  bRes = FALSE;
	char *pPos1 = NULL;
	char *pPos2 = NULL;
	
	do 
	{
		pPos1 = strstr(_pUrl,":");
		if(NULL ==  pPos1){
			break;
		}
		pPos1 = pPos1 + 3;
		pPos2 = strstr(pPos1,":");
		if(NULL ==  pPos2)
		{
			break;
		}

		//解析域名或者是IP
		strncpy(m_szHost,pPos1,pPos2-pPos1);

		//解析端口
		pPos2++;
		pPos1 = strstr(pPos2,"/");
		if(NULL==pPos1)
		{
			break;
		}

		char chPort[MAX_PATH]={0};
		strncpy(chPort,pPos2,(pPos1-pPos2)%9);
		m_wPort = atoi(chPort);
		strncpy(m_szRequestSource,pPos1,1023);

		bRes = TRUE;
		
	} while (FALSE);
	return bRes;
}

int CHttpDownLoad::UrlDownLoad2File(const char *pUrl,const char *_chFullFileName)
{
	DeleteFile(_chFullFileName);
	int iRes = -1;
	CHttpDownLoad *pHttp;

	for (int i=0;i<=10;i++)
	{
		pHttp=NULL;
		pHttp=new CHttpDownLoad;
		
		if (pHttp==NULL){
			break;
		}

		iRes=pHttp->UrlDownLoadToFile(pUrl,_chFullFileName);
		delete pHttp;
		if (iRes!=CHttpDownLoad::S_PARTDOWN)
		{
			break;
		}
		
		LOG((LEVEL_ERROR,"[%s]第%d次下载失败\n",pUrl,i));

		Sleep(5);

	}

	return iRes==CHttpDownLoad::S_FULLDOWN?0:-1;
}

int CHttpDownLoad::UrlDownLoadToFile(const char *_pUrl,const char *_chFullFileName)
{

	int iRes = S_ERRO;

	if(NULL == _pUrl || NULL == _chFullFileName)
	{
		LOG((LEVEL_ERROR,"参数不合法\n"));
		return iRes;

	}
	do 
	{

		m_hOutFileHandle = CreateFileA(_chFullFileName,GENERIC_WRITE|GENERIC_READ,0,
			NULL,OPEN_ALWAYS,NULL,NULL);
		if(INVALID_HANDLE_VALUE == m_hOutFileHandle)
		{
			LOG((LEVEL_ERROR,"CreateFileA %s erro=%d\n",_chFullFileName,GetLastError()));
			break;
		}

		//_asm int 3
		m_dwOffset=GetFileSize(m_hOutFileHandle,0);
		//LOG((LEVEL_INFO,"m_dwOffset =%d\r\n",m_dwOffset));
		//printf("m_dwOffset =%d\r\n",m_dwOffset);

		if(!GetHostAndPortFormUrl((char *)_pUrl))
		{
			LOG((LEVEL_ERROR,"解析域名和端口错误\n"));
			break;
		}

		iRes=UrlDownLoad();

	} while (FALSE);

	return iRes;
}

int CHttpDownLoad::UrlDownLoad()
{

	int iRet=S_ERRO;
	FormatRequestHeader();

	if(!Connect(m_szHost,m_wPort)) 
	{
		return S_PARTDOWN;
	}
	
	if(!SendRequest()) return S_PARTDOWN;

	char szValue[30]={0};

	if(GetField("Content-Length",szValue,30)==-1)
	{
		LOG((LEVEL_ERROR,"CHttpDownLoad::UrlDownLoad  Not Get Content_Lenth\r\n"));

		return iRet;
	}


	DWORD nFileSize = atoi(szValue);
	if (m_dwOffset==0){
		m_dwFileLen=nFileSize;
	}

	if (nFileSize==0||nFileSize>1024*1024*2){
		return iRet;
	}

	//Content-Range: bytes 1000-5000/5001
	memset(szValue,0,30);

	if(GetField("Content-Range",szValue,30)!=-1)
	{
		char *p=strstr(szValue,"/");
		if (!p)
		{
			return iRet;
		}
		p++;
		m_dwFileLen= atoi(p);
		if (m_dwFileLen==0)
		{
			return iRet;
		}
	}

	DWORD dwRecvLen=MAXHEADERSIZE;
	char *pData =new char [dwRecvLen];
	if (pData==NULL)
	{
		LOG((LEVEL_ERROR,"CHttpDownLoad::UrlDownLoad  new pData erro:%d\r\n",GetLastError()));
		return iRet;
	}

	int nReceSize = 0;
	DWORD nCompletedSize = 0;
	
	SetFilePointer(m_hOutFileHandle,0,0,FILE_END);
	DWORD iWriteFileSize;

	while(nCompletedSize < nFileSize)
	{
		memset(pData,0,dwRecvLen);
		nReceSize = RecvData((LPBYTE)pData,dwRecvLen);
		if(nReceSize<=0){
			LOG((LEVEL_ERROR,"CHttpDownLoad::UrlDownLoad- m_TcpSocket.RecvData erro :%d\r\n",WSAGetLastError()));
			break;
		}


		if ((DWORD)nReceSize>nFileSize-nCompletedSize){
			break;
		}

		WriteFile(m_hOutFileHandle,pData,nReceSize,&iWriteFileSize,NULL);

		nCompletedSize += nReceSize;
	}

	delete [] pData;

// 	if (m_dwFileLen){
// 		iRes=m_dwFileLen==GetFileSize(m_hOutFileHandle,0)?1:-1;
// 	}


	iRet=nCompletedSize==nFileSize?S_FULLDOWN:S_PARTDOWN;

	return iRet;

}