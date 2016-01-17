#include "ddc_util.h"
#include "UpdateManager.h"
#include "Log.h"


//#include <UrlMon.h>
//#pragma comment(lib, "urlmon.lib")


#include "../TSPUtil/HttpDownLoad.h"



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




/* //###########################add by menglz



BOOL bThreadExeOK = FALSE;


typedef struct _download_param
{
	char *pURL;
	char *pDestPath;

}stu_download_param;




DWORD WINAPI DownloadFileThread(LPVOID lpParam)
{
	bThreadExeOK = FALSE;


	char chUrl[1024];
	memset(&chUrl,0,sizeof(chUrl));
	stu_download_param *pStuDownloadParam = NULL;

	

	
	if(NULL == lpParam)
	{
		LOG((LEVEL_ERROR,"�����ļ��̳߳�������ΪNULL\n"));
		goto DOWNLOAD_FILE_THREAD_FUNC_END;
	}

	pStuDownloadParam = (stu_download_param *)lpParam;


	if(NULL ==  pStuDownloadParam->pURL || NULL == pStuDownloadParam->pDestPath)
	{
		LOG((LEVEL_ERROR,"�����ļ��̳߳���������һ��ΪNULL\n"));
		goto DOWNLOAD_FILE_THREAD_FUNC_END;

	}


	DeleteUrlCacheEntry (pStuDownloadParam->pURL); // �建��

	
	

	time_t now;
	time(&now);

	char chTime[50];
	char TmpFileName[MAX_PATH+50+1];

	memset(&chTime,0,sizeof(chTime));
	memset(&TmpFileName,0,sizeof(TmpFileName));

	strftime(chTime,sizeof(chTime),"%Y%m%d%H%M%S",localtime(&now));

	
	strncpy(TmpFileName,pStuDownloadParam->pDestPath,MAX_PATH);

	strcat(TmpFileName,chTime);

	
	HRESULT hr = URLDownloadToFile(0,pStuDownloadParam->pURL,TmpFileName,0,NULL);

	if(S_OK == hr)
	{
		
		
		if(rename(TmpFileName,pStuDownloadParam->pDestPath))
		{
			
			LOG((LEVEL_INFO,"rename[%s]����\n",TmpFileName));
		}
		
		LOG((LEVEL_INFO,"ִ�� URLDownloadToFile�ɹ�\n"));

		bThreadExeOK = TRUE;
		
	}
	else
	{
		LOG((LEVEL_ERROR,"ִ�� URLDownloadToFileʧ��	[%s][%d]",__FILE__,__LINE__));
		remove(TmpFileName);
	}

	

	


DOWNLOAD_FILE_THREAD_FUNC_END:



	bThreadExeOK = TRUE;
	return 0;
}








int CUpdateManager::GetFileFromServerImp(const char *szURl,const char *szDes,int dwFileSize)  //����С�ļ������ļ���������
{

	
	stu_download_param  stuDownloadParam;
	HANDLE hDownload = NULL;
	int iRes = -1;

	
	stuDownloadParam.pURL = (char *)szURl;
	stuDownloadParam.pDestPath = (char *)szDes;

	
	
	hDownload = CreateThread(NULL,0,DownloadFileThread,&stuDownloadParam,0,NULL);

	if(NULL == hDownload)
	{
		LOG((LEVEL_ERROR,"�����ļ������߳�ʧ��\n"));

		goto GET_FILE_FROM_SERVER_FUNC_END;
		
	}

	DWORD dwWaitCode = WaitForSingleObject(hDownload,1000*30);  //�ȴ�10����

	if(WAIT_TIMEOUT == dwWaitCode)
	{
		TerminateThread(hDownload,0);

		LOG((LEVEL_ERROR,"���ز��%s��ʱ��ǿ���˳�\n",szURl));

		//exit(-1);  //��������˳���Ҫ��ǿ���˳��������ļ���ռ��

		goto GET_FILE_FROM_SERVER_FUNC_END;

	}

	if(NULL != hDownload)
	{

		CloseHandle(hDownload);
		hDownload = NULL;

	}


	if(bThreadExeOK)
	{
		iRes = 0;
	}


GET_FILE_FROM_SERVER_FUNC_END:


	bThreadExeOK = FALSE;
	return iRes;
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

