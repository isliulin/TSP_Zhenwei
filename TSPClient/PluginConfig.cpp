/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ������ù���
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include "StdAfx.h"
#include <iosfwd>
#include <fstream>
#include <WinSock2.h>
#include "macros.h"
#include "include\IOCPServer.h"
#include "PluginConfig.h"
#include "PluginGlobalData.h"
#include "Log.h"


// ��ѯ�Ƿ���Ҫ��ж���������ǣ�����
bool CPluginRemover::operator()(DWORD dwMachineID)
{	
	CDBPlugSetting setting;
	CDBPlugInfo info;
	int ret = 0;
	
	info.mid = dwMachineID;
	info.pid = GLOBAL_DATA->nPluginPid;
	// ȡplug_info����û�ж�ر��
	ret = TspDbGet(info,GLOBAL_DATA->dbPool );
	if( ret == 0 )
	{
		//�ҵ�
		if( info.isused == CDBPlugInfo::SET )
		{
			// ж�أ���ж������
			ret = SendRemoveCommand();
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"����ж������ʧ��%\n",ret));
			}

			// ������ƶ�����
			ret = ClearClientData(dwMachineID);
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"���ж������ʧ��%\n",ret));
			}

			return true;
		}
	}
	else if( ret != DDCE_END_OF_FILE )
	{
		LOG((LEVEL_WARNNING,"���ݿ��ѯʧ�ܣ�mid=%d pid=%d ����:%d(%x)\n",dwMachineID,GLOBAL_DATA->nPluginPid,ret,ret));
	}


	return false;
}

// ����ж������
int CPluginRemover::SendRemoveCommand()
{
	LOG((LEVEL_FUNC_IN_OUT,"����ж������(%08x,%08x)\n",m_pContext,m_pContext->m_Socket));

	BYTE	bToken = COMMAND_REMOVE;		
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));	
	return 0;
}

// ���ж�����ݿ�����
int CPluginRemover::ClearClientData(unsigned long dwMachineID)
{
	LOG((LEVEL_FUNC_IN_OUT,"���ж�����ݿ�����(%08x)\n",dwMachineID));

	CTspDbStorage dbStorage(GLOBAL_DATA->dbPool);
	int ret = 0;

	// ����plug_info������,MID�����������
	ostringstream os;
	os << CDBPlugInfo::SQLType::GetDeleteSql() << " where mid="<<dwMachineID;
	string sql = os.str();
	
	ret = dbStorage.ExecuteSql(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"���ж������ʧ��(%08x)\n",dwMachineID));
		return DDCE_DB_OP_ERROR;
	}
	
	if( !RemoveDownloadTmpDir(dwMachineID) )
	{
		LOG((LEVEL_ERROR,"���ж������ʧ��(%08x)\n",dwMachineID));
		return DDCE_CALL_FAIL;	
	}
	return 0;

}


// �������ṩ
BOOL CPluginRemover::RemoveDownloadTmpDir(unsigned long _ulMid)
{
	TCHAR tchDownloadTmpDir[MAX_PATH];
	TCHAR tchConf[MAX_PATH];

	memset(&tchDownloadTmpDir,0,sizeof(tchDownloadTmpDir));
	memset(&tchConf,0,sizeof(tchConf));

	if(0 == GetModuleFileNameA(NULL,tchConf,sizeof(tchConf)))
	{
		return FALSE;
	}

	//printf("%s",tchConf);
	int iLen = strlen(tchConf);
	int i = 0;
	for(i=iLen-1;i>=0;i--)
	{
		if(TCHAR(*(tchConf+i)) == '\\')
		{
			sprintf(tchConf+i+1,"mqxc.ini");
			break;

		}

	}


	::GetPrivateProfileString("conf","downloadDir","NULL",tchDownloadTmpDir,sizeof(tchDownloadTmpDir),tchConf);

	if(0 == strcmp(tchDownloadTmpDir,"NULL"))
	{
		return FALSE;

	}

	sprintf(tchDownloadTmpDir+strlen(tchDownloadTmpDir),"\\%lu\\",_ulMid);


	printf("%s\n",tchDownloadTmpDir);

	//����ֱ��ɾ��tchDownloadTmpDir��ʾ��Ŀ¼�Ϳ�����



	return TRUE;

}

///////////////////////////////////////////////////////////////////////////////////

//constrator
CPluginConfig::CPluginConfig(ClientContext *pContext,CIOCPServer *iocpServer)
	:m_pContext(pContext),m_iocpServer(iocpServer)
{
	//�����������Ŀ¼ "MachinePluginConfig"
	std::string m_strPluginPath;			//���·��
	m_strPluginPath.assign( GLOBAL_DATA->szModulePath );
	m_strPluginPath += TSP_PLUGIN_C_CFG_PATH;
	if( !CreateDirectory(m_strPluginPath.c_str(),NULL) )
	{
		g_log.Log(LEVEL_WARNNING,"����Ŀ¼ʧ��:%s ������:%d(%x).\n",m_strPluginPath.c_str(),GetLastError(),GetLastError());
	}	
}

//destrator
CPluginConfig::~CPluginConfig()
{

}

// ����config�ļ�ʵ��
int CPluginConfig::CreateConfigFileImp(unsigned long _ulMachineID,std::list<PluginData> &l)
{
	LOG((LEVEL_FUNC_IN_OUT,"���ɲ�������ļ�,id=%lu.\n",_ulMachineID));

	int ret;
	PluginData data;
	memset(&data,0,sizeof(data));

	// ��ȡ�ܵ�update.ini�����ļ�
	string str(GLOBAL_DATA->szConfigPath);
	str += TSP_PLUGIN_M_CFG_NAME;
	
	std::list<PluginData> pluginConfigList;
	if( !ReadPluginIniFile<PluginData>(pluginConfigList,str) )	
	{
		LOG((LEVEL_WARNNING,"������������ļ�ʧ��,ret:.\n"));
		return DDCE_READ_FILE_ERROR;
	}

	// ����ȡ�����Ϣ�����ݿ������ã��ȽϺ�д�뵥���������	
	CDBPlugInfo info;

	l.clear();
	for( std::list<PluginData>::iterator iter=pluginConfigList.begin();iter!=pluginConfigList.end();++iter)
	{
		// plugin_info��ȡ����
		info.pid = iter->m_pid;
		info.mid = _ulMachineID;		
		
		ret = TspDbGet(info,GLOBAL_DATA->dbPool);		
		if( ret == 0 )
		{
			// û������
			if( info.isused == CDBPlugInfo::UNSET )
				continue;
		}		
		else if( ret != DDCE_END_OF_FILE )
		{
			// ���ݿ�δ�ҵ�����ȫ������
			LOG((LEVEL_WARNNING,"���ݿ���ȡ������Ϣ��,�����ñ�ȡ:plugin_info(%d,%d) ret:%d(%x).\n",info.pid,info.mid,ret,ret));
			
			// ����δ����
			if( GLOBAL_DATA->GetPlugSetting(iter->m_pid) == CDBPlugInfo::UNSET )			
				continue;		
		}
		else
		{
			// �����ߣ�ʹ��Ĭ������
			if( GLOBAL_DATA->GetPlugSetting(iter->m_pid) == CDBPlugInfo::UNSET )			
				continue;
		}

		// �����������
		if( iter->m_pid == TSP_PLUGIN_CAP_PID )
		{
			CDBCaptureSetting cap;
			cap.mid = _ulMachineID;
			ret = TspDbGet(cap,GLOBAL_DATA->dbPool );
			if( ret == 0 )
			{
				iter->m_dwReserved=cap.m_intervel<<16; //add by yx
				iter->m_dwReserved+= cap.capquality;
				strncpy(iter->m_szReserved,cap.capset,TSP_PLUGIN_SHORT_PATH);
			}
			else
			{
				// ��Ĭ�ϲ���
				iter->m_dwReserved = 120<<16; //add by yx Ĭ�Ͻ���ʱ��Ϊ2����
				iter->m_dwReserved += 4; //��Ϊ��
				
				//modify by yx
				//strncpy(iter->m_szReserved,"1,1,1",10);		
				strncpy(iter->m_szReserved,"0,0,0,1",10);//	Ĭ��Ϊ�Զ�����			

			}
				
		}

		// ��������
		l.push_back(*iter);
		


	}
	
	return 0;
}

// ����config�ļ�
BOOL CPluginConfig::CreateConfigFile(unsigned long _ulMachineID,TCHAR _tchMachineConfigFile[TSP_PLUGIN_PATH])
{
	std::ostringstream os;

	//����������Ϊ������Ŀ¼
	os<< GLOBAL_DATA->szModulePath <<TSP_PLUGIN_C_CFG_PATH<<_ulMachineID<<"\\";
	DWORD dwAttr = GetFileAttributes(os.str().c_str() );
	if( dwAttr == -1 || (dwAttr&FILE_ATTRIBUTE_DIRECTORY) == 0 )		
		if( !CreateDirectory(os.str().c_str(),NULL ) )
		{
			g_log.Log(LEVEL_ERROR,"�������Ŀ¼ʧ��:%s,������:%d(%x).\n",os.str().c_str(),GetLastError(),GetLastError());
			return FALSE;
		}


	std::list<PluginData> l;
	std::string strFileName;
	if( 0 != CreateConfigFileImp(_ulMachineID,l) )
	{
		LOG((LEVEL_ERROR,"���ɲ������ʧ��.\n"));
		return FALSE;
	}

	//�ļ���
	os<<TSP_PLUGIN_C_CFG_NAME;
	strncpy(_tchMachineConfigFile,os.str().c_str(),TSP_PLUGIN_PATH);

	// ���һ�������Ǽ���
	if( !WritePluginIniFile<PluginData>(l,_tchMachineConfigFile,true) )
		{	LOG((LEVEL_ERROR,"���������ļ�ʧ��.\n"));	}

	return TRUE;
}

// ����ini�ļ���server
BOOL CPluginConfig::SendFile(const char *szMachineConfigFile)
{	
	DDC_BOOL_RETURN(szMachineConfigFile==NULL,FALSE);

	CVirusFileMap fileMap(szMachineConfigFile);
	if( !fileMap )
	{
		LOG((LEVEL_ERROR,"�������ļ�ʧ��\n"));
		return FALSE;
	}
	uint len = fileMap.FileSize()+1;
	uchar *pBuff = new uchar[len];	
	if( !pBuff )
	{
		LOG((LEVEL_ERROR,"�����ڴ�ʧ��\n"));
		return FALSE;
	}
	
	// д�ļ�����,buff����дһ��token

	

	pBuff[0] = COMMAND_PLUGIN_REQUEST;
	memcpy(&pBuff[1],fileMap.GetBuffer(),len-1);
	
	m_iocpServer->Send(m_pContext, pBuff,len);

	DDC_DELETE(pBuff);

	return TRUE;
}

