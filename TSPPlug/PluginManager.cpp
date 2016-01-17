/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief �������ʵ�ֲ���
* @author ����ΰ chengwei@run.com
* @date 2013-07-31
* @version 1.0
* @note 
*************************************************************************/


#include <winsock2.h>
#include "algorithm"
#include "ddc_util.h"
#include "ddc_dlopen.h"
#include "MD5.h"
#include "crc32.h"

#include "PluginManager.h"
#include "UpdateManager.h"
#include "Log.h"


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
int ComfirmCopyFile(const char *src,const char *desc)
{
	char szDir[MAX_PATH] = {0};
	strncpy(szDir,desc,MAX_PATH);
	char *p = strrchr(szDir,'\\');
	if(p)
		*(p+1) = 0;
	RecurseCreateDirectory(szDir);

	return ::CopyFile(src,desc,FALSE)?0:DDCE_DISK_OP_ERROR;

}

///////////////////////////////////////////////

// ��ʼ��singleton
#ifdef _MSC_VER
CPluginManager* SingletonHolder<CPluginManager>::pInstance_ = 0;
#endif

CPluginManager::CPluginManager()
{
}


int CPluginManager::Init(const char *szModulePath)
{
	LOG((LEVEL_INFO,"CPluginManager::Init(%s).\n",szModulePath));
	if( szModulePath )
		strncpy(m_szModulePath,szModulePath,MAX_PATH);
	else
	{
		//�õ���װ·��
		GetModuleFileName(NULL,m_szModulePath,TSP_PLUGIN_PATH);
		DDC_RIGHT_SLASH(m_szModulePath);
	}	
	
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

	LOG((LEVEL_FUNC_ROUTE,"CPluginManager::CPluginManager \n"));

	// ����Ĭ�ϲ����Ϣ��������
	ReloadPlugin();
	return 0;
}

CPluginManager::~CPluginManager()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::~CPluginManager \n"));

	// ֹͣ���в��
	StatusAll(CPluginInfo::STATUS_UNLOAD);
}


//������������ļ������ز�װ��
BOOL CPluginManager::Update(const string &strPluginDownLoadFile)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::ProcessConfig\n"));

	int ret;	

	// ��������Ŀ¼
	CreateDirectory(m_szPluginDownLoadPath.c_str(),NULL);
	m_recycle.Push(m_szPluginDownLoadPath,CTspRecycle::FLAG_DIR);
	
	// ��װ��������ļ��ļ���
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	std::string strPluginDownLoadTmpFile;

	// ��ס���������������IO���첽�����������Ҫͬ��
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);	

	// ��ȡ������Ϣ����ʱ������
	std::list<CPluginInfo> l;
	if( !ReadPluginIniFile<CPluginInfo>(l,strPluginDownLoadFile,true) )	
	{
		LOG((LEVEL_WARNNING,"������������ļ�ʧ��.%s\n",strPluginDownLoadFile.c_str()));
		return FALSE;
	}
	// ȥ�������ڴ˹��ܹ���Ĳ��
	ExceptPluginFile(l);

	// �¾�����ĵ���������
	std::list<CPluginInfo>::iterator iterNew=l.begin();
	std::list<CPluginInfo>::iterator iterOld;
	char md5[33] = {0};
	BOOL bRet = FALSE;

	// �Ƚ������б�ȷ��Ҫ���ص���
	for(;iterNew!=l.end();++iterNew)
	{
		// �����б��в���
		iterOld = find(m_PluginList.begin(),m_PluginList.end(),iterNew->m_strName);
		if( iterOld != m_PluginList.end() )
		{
			// �ҵ�����һ�����Ҽ�⣬��ֹ�����ļ���ʵ���ļ���һ�µ��²�����
			bRet = MD5_Caculate_File ( iterOld->m_strFullName.c_str(),md5);

			// �Ƚ�md5ֵ
			if( bRet && strncmp(iterNew->m_md5,iterOld->m_md5,32) == 0 )
			{
				// ��Ҫ��strFullName�����һ����˵��û�仯���������Ƹ���־λ�����潻�������б���
				if( iterOld->m_strFullName == iterNew->m_strFullName && 
					iterOld->m_pid == iterNew->m_pid )	
				{
					*iterNew = *iterOld;
					
					LOG((LEVEL_INFO,"(the same dll,don't update)  %s : %d\n",iterOld->m_strFullName.c_str(),iterOld->m_status));

					// �ó�ʼ״̬�������״̬����ֹ����ʱ��ж��
					if( iterOld->m_status == CPluginInfo::STATUS_RUNNING )
						iterOld->m_status = CPluginInfo::STATUS_INIT;
					else
					{
						// ����״̬��ж���ټ��ذ�
						LOG((LEVEL_INFO,"���δ�仯���������н���ж��:%s,״̬:%d.\n",iterOld->m_strFullName.c_str(),iterOld->m_status));
						iterOld->RemovePlug();	
					}								
				}
				else
				{
					// ֱ�ӿ�������·��
					ret = ComfirmCopyFile(iterOld->m_strFullName.c_str(),iterNew->m_strFullName.c_str());					
					if( ret != 0 )
					{
						LOG((LEVEL_ERROR,"�����ļ�����:(%s==>%s),Lasterror=%d\n",iterOld->m_strFullName.c_str(),iterNew->m_strFullName.c_str(),GetLastError()));
					}
					iterNew->m_status = CPluginInfo::STATUS_UNLOAD;	

					// ������ȥ�Ĳ���϶��������ɹ������ٺ��ϲ��һ����״̬��������������ϲ����Ϣ��ȫɾ��
					iterOld->RemovePlug();
				}
				
				// �¼�¼���Ѿ����ˣ��ϼ�¼��ɾ��
				m_PluginList.erase(iterOld);				
			}
			else
				// ׼������
				iterNew->m_status = CPluginInfo::STATUS_WAITDOWNLOAD;
		}
		else
			// ׼������
			iterNew->m_status = CPluginInfo::STATUS_WAITDOWNLOAD;
	}

	// ����
	for_each( l.begin(),l.end(),bind2nd(mem_fun_ref(&CPluginInfo::DownloadPlug),m_szPluginDownLoadPath) );

	// ��������������ļ�¼
	for( iterNew=l.begin();iterNew!=l.end();++iterNew )
	{
		// ������������
		iterOld = find(m_PluginList.begin(),m_PluginList.end(),iterNew->m_strName.c_str() );

		if( iterNew->m_status == CPluginInfo::STATUS_DOWNLOAD )
		{
			// ��װ�����ļ���
			strPluginDownLoadTmpFile.assign(m_szPluginDownLoadPath);
			strPluginDownLoadTmpFile += iterNew->m_strName;

			//�ȼ������վ
			m_recycle.Push(strPluginDownLoadTmpFile);

			// ����ҵ���ͣ�ã�ɾ��
			if( iterOld != m_PluginList.end() )
			{
				ret = iterOld->RemovePlug();
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"ж�ز�� %s ����:%d(%x)\n",iterNew->m_strName.c_str()));
				}
			}

			//������ļ�������Ŀ�ĵ�
			ret = ComfirmCopyFile(strPluginDownLoadTmpFile.c_str(),iterNew->m_strFullName.c_str());					
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"�����ļ�����:(%s==>%s),Lasterror=%d\n",strPluginDownLoadTmpFile.c_str(),iterNew->m_strFullName.c_str(),GetLastError()));
			}

			// ��״̬Ϊ׼������
			iterNew->m_status = CPluginInfo::STATUS_UNLOAD;
		}
		else if( iterNew->m_status == CPluginInfo::STATUS_ERROR )
		{
			// ��������ʧ�ܣ�Ҳ������ǰ��ʧ�ܣ�����������������Ϣ���ƹ�ȥ
			if( iterOld != m_PluginList.end() )
				*iterNew = *iterOld;
		}

		//��urlɾ����Ϊ�˲��ں���д�������ļ��У�д�����ļ��Ǹ�ͨ�õĳ���
		iterNew->m_strURL = "";
	}

	// �����ϲ�������ļ��������������û��,ɾ��
	for( iterOld=m_PluginList.begin();iterOld!=m_PluginList.end();++iterOld )
	{
		// ������������
		iterNew = find(l.begin(),l.end(),iterOld->m_strName.c_str() );

		// ���û�ҵ�,ж��
		if( iterNew == l.end() )
			iterOld->RemovePlug();
			// l.push_back(*iterOld); //����
	}

	// ����������д����ini�ļ�
	if( !WritePluginIniFile(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"д�����ļ���:%s",strPluginConfigFile.c_str()));
	}

	// �����ֳ�
	m_recycle.PopAll();

	// ������б�Ϊ�˱�������ʱ�Զ�ж�ز��������ǿ�ƽ�״̬��Ϊɾ��
	for_each( m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::Status),CPluginInfo::STATUS_DELETE) );
	m_PluginList.clear();

	// �������ϲ���б�
	m_PluginList.swap(l);

	//��INIT״̬�ĸ�ΪRUNNING
	LOG((LEVEL_INFO,"m_PluginList.size()=%d.\n",m_PluginList.size()));
	for( iterOld=m_PluginList.begin();iterOld!=m_PluginList.end();++iterOld )
	{
		LOG((LEVEL_INFO,"%s : %d\n",iterOld->m_strFullName.c_str(),iterOld->m_status));
		if( iterOld->m_status == CPluginInfo::STATUS_INIT )
			iterOld->m_status = CPluginInfo::STATUS_RUNNING;
	}

	// ����
	StatusAll(CPluginInfo::STATUS_RUNNING);
	

	return TRUE;
}
	
// ȥ�������ڴ˲��ֹ���Ĳ��
int CPluginManager::ExceptPluginFile(std::list<CPluginInfo> &l)
{
	for( std::list<CPluginInfo>::iterator iter=l.begin();iter!=l.end();)
	{
		//���̲���͹���������������ܣ��������Լ�Ҳ������
		if( iter->m_pid == TSP_PLUGIN_MANAGE ||
			iter->m_pid == TSP_PLUGIN_KEYBOARD ||
			iter->m_pid == TSP_PLUGIN_REMOVE_PID )
			l.erase(iter++);
		else
			iter++;
	}
	return 0;
}

// �������ļ��������ɲ���б�
int CPluginManager::ReloadPlugin(DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::ReloadPlugin,flag = %d\n",dwFlag));	
	
	// ж�����в��
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::UnLoadPlug),0));
	
	// �������
	m_PluginList.clear();

	// ���´������ļ���
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	// ��������ļ�����ȷ����������ģ��ܿ��ܵ��ô˺��������Բ����ش���������ͨ���������µ��µ�
	if( !ReadPluginIniFile<CPluginInfo>(m_PluginList,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"��ȡ��������ļ���\n"));
		return DDCE_CALL_FAIL;
	}
	
	// ȥ�������ڴ˲��ֹ���Ĳ��,�����жϷ���ֵ
	ExceptPluginFile(m_PluginList);

	// �������в��״̬Ϊδ����	
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::Status),CPluginInfo::STATUS_UNLOAD));			

	return 0;
}

//�ı���״̬
int CPluginManager::Status(int pid,int flag)
{
	int ret;

	// ��������
	std::list<CPluginInfo>::iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( iter->m_pid == pid )
		{
			if( flag == CPluginInfo::STATUS_RUNNING )
				ret = iter->StartPlug(&m_plugParam);
			else if( flag == CPluginInfo::STATUS_UNLOAD )
				ret = iter->UnLoadPlug(0);					
			else if( flag == CPluginInfo::STATUS_LOAD )
				ret = iter->LoadPlug(0);					
			else if( flag == CPluginInfo::STATUS_STOP )
				ret = iter->StopPlug(0);					
			else if( flag == CPluginInfo::STATUS_DELETE )
			{
				ret = iter->RemovePlug(0);	

				// ��Ҫɾ�������м�¼
				m_PluginList.erase(iter);
			}
			return ret;
		}
	
	LOG((LEVEL_WARNNING,"�������δ�ҵ�:%d\n",pid));
	return CPluginInfo::STATUS_ERROR;	
}

//��ò��״̬
int CPluginManager::Status(int pid)
{
	// ��������
	std::list<CPluginInfo>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( iter->m_pid == pid )
			return iter->m_status;
	
	LOG((LEVEL_WARNNING,"�������δ�ҵ�:%d\n",pid));
	return CPluginInfo::STATUS_ERROR;
}



//�ı����в��״̬
int CPluginManager::StatusAll(int flag)
{
	if( flag == CPluginInfo::STATUS_RUNNING )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::StartPlug),&m_plugParam));
	else if( flag == CPluginInfo::STATUS_UNLOAD )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::UnLoadPlug),0));
	else if( flag == CPluginInfo::STATUS_LOAD )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::LoadPlug),0));
	else if( flag == CPluginInfo::STATUS_STOP )
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::StopPlug),0));
	else if( flag == CPluginInfo::STATUS_DELETE )
	{
		// ��Ҫɾ�������м�¼
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::RemovePlug),0));
		m_PluginList.clear();

		// ���˲��ԣ��������ֻ��ж�ػ����������Լ��������ļ�Ҳɾ��		
		std::string strPluginConfigFile(m_szPluginConfigPath);
		strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;
		
		if( !DeleteFile(strPluginConfigFile.c_str()) )
		{
			LOG((LEVEL_ERROR,"ɾ�����ʧ��(%s):%d \n",strPluginConfigFile.c_str(),GetLastError()));
		}

	}
	else
	{
		LOG((LEVEL_WARNNING,"���״̬δ�ҵ�:%d\n",flag));
	}

	return 0;
}

