#include "ddc_util.h"
#include "UpdateManager.h"
#include "Log.h"


#include "../TSPUtil/HttpDownLoad.h"


#include <UrlMon.h>
#pragma comment(lib, "urlmon.lib")



#define DownloadFilePath ("\\TSPUpdate\\")


CUpdateManager::CUpdateManager(void):
m_nUpdateSoftNum(0),
m_port(8808)
{
	memset(m_strMD5,0,sizeof(m_strMD5));             
	memset(m_szUrl,0,sizeof(m_szUrl));             
	memset(m_szDownloadFileName,0,sizeof(m_szDownloadFileName));
	memset(m_pUpdateFileInfo,0,sizeof(m_pUpdateFileInfo));

}

CUpdateManager::~CUpdateManager(void)
{

}





/*


int CUpdateManager::GetFileFromServerImp(const char *szURl,const char *szDes,int dwFileSize)
{

	HRESULT hr = URLDownloadToFile(0,szURl,szDes,0,NULL);

	if(S_OK != hr)
	{

		LOG((LEVEL_ERROR,"ִ�� URLDownloadToFileʧ��	[%s][%d]",__FILE__,__LINE__));
		return -1;
	}

	LOG((LEVEL_INFO,"ִ�� URLDownloadToFile�ɹ�\n"));

	return 0;


}



*/




int CUpdateManager::GetFileFromServerImp(const char *szURl,const char *szDes,int dwFileSize)
{

	//DeleteUrlCacheEntry (szURl); // �建��


	//CHttpDownLoad  HttpDownload;


	int iRes = CHttpDownLoad::UrlDownLoad2File(szURl,szDes);

	//HttpDownload.Disconnect();

	if(iRes < 0)
	{

		LOG((LEVEL_ERROR,"[%s]����ʧ��\n",szURl));
		return -1;

	}



	return 0;
}






/*


int CUpdateManager::GetFileFromServerImp(const char *szURl,const char *szDes,int dwFileSize)
{
	HINTERNET hNet = NULL;
	HINTERNET hUrlFile = NULL;

	hNet = ::InternetOpen("3322", PRE_CONFIG_INTERNET_ACCESS,NULL, INTERNET_INVALID_PORT_NUMBER, 0); 
	if (NULL == hNet)
	{
		LOG((LEVEL_ERROR,"%s,	[%s][%d]","����InternetOpen����ʧ��",__FILE__,__LINE__));
		return -1;
	}

	hUrlFile = ::InternetOpenUrl(hNet, (char *)szURl, NULL, 0, INTERNET_FLAG_RELOAD, 0); 
	if (NULL == hUrlFile)
	{
		LOG((LEVEL_ERROR,"%s,	[%s][%d]","����InternetOpenUrl����ʧ��",__FILE__,__LINE__));
		InternetCloseHandle(hNet);
		return -1;
	}

	
	DWORD byteread=0; 
	BOOL hwrite; 
	DWORD written; 
	HANDLE createfile = INVALID_HANDLE_VALUE;
	char* pbuffer = NULL;
	


	try
	{
		if ((dwFileSize <= 0)||(dwFileSize >= 0x06400000))  //dwFileSize��С��0-100M֮��
		{
			LOG((LEVEL_ERROR,"%s,	[%s][%d]","�����ļ���С���ں���Χ��",__FILE__,__LINE__));
			throw "err";

		}
		pbuffer = new char[dwFileSize] ;
		if (!pbuffer)
		{
			LOG((LEVEL_ERROR,"%s,	[%s][%d]","ָ�����ռ�ʧ��",__FILE__,__LINE__));
			throw "err";
		}

		createfile=CreateFile(szDes,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0); 
		if (createfile==INVALID_HANDLE_VALUE)
		{ 
			LOG((LEVEL_ERROR,"%s,	[%s][%d]","����CreateFile����ʧ��",__FILE__,__LINE__));
			InternetCloseHandle(hUrlFile); 
			InternetCloseHandle(hNet); 
			delete pbuffer;
			pbuffer = NULL;
			throw "err";
		} 

		BOOL internetreadfile; 
		while(1)
		{ 
			internetreadfile=InternetReadFile(hUrlFile,pbuffer,dwFileSize,&byteread); 
			if(byteread==0) 
			{
				break; 
			}
			hwrite=WriteFile(createfile,pbuffer,dwFileSize,&written,NULL); 
			if (0 == hwrite)
			{
				LOG((LEVEL_ERROR,"%s,	[%s][%d]","����WriteFile����ʧ��",__FILE__,__LINE__));
				CloseHandle(createfile); 
				InternetCloseHandle(hUrlFile); 
				InternetCloseHandle(hNet); 
				delete pbuffer;
				pbuffer = NULL;
				throw "err";
			}
		}
		if( pbuffer )
			delete pbuffer;

		CloseHandle(createfile); 
		InternetCloseHandle(hUrlFile); 
		InternetCloseHandle(hNet); 
	}	
	catch (...)
	{
		if( pbuffer )
			delete pbuffer;

		if( createfile != INVALID_HANDLE_VALUE )
			CloseHandle(createfile); 
		if( hUrlFile != NULL )
			InternetCloseHandle(NULL); 
		if( hNet != INVALID_HANDLE_VALUE )
			InternetCloseHandle(hNet); 

		return -1;

	}
	
	pbuffer = NULL;	

	return 0;
}



*/





///�ɹ�����0��ʧ�ܷ���-1�� //��������о���Ҫ�޸��£���������о���
int CUpdateManager::GetFileFromServer(char *szURl,char *szSrcFileName,char *szDesPath,char *szDesFileName,int SrcFileSize)
{
	if ((NULL == szURl) &&(NULL == szSrcFileName)&&(NULL == szDesPath)&&(NULL == szDesPath))
	{
		return -1;
	}

	char tempSrcPath[MAX_PATH];
	memset(tempSrcPath,0,sizeof(tempSrcPath));
	strncpy(tempSrcPath,szURl,MAX_PATH);
	strncat(tempSrcPath,szSrcFileName,MAX_PATH);

	char tempDesPath[MAX_PATH];
	memset(tempDesPath,0,sizeof(tempDesPath));
	strncpy(tempDesPath,szDesPath,MAX_PATH);
	::CreateDirectory(tempDesPath,NULL);
	strcat_s(tempDesPath,szDesFileName);

	return GetFileFromServerImp(tempSrcPath,tempDesPath,SrcFileSize);
}

