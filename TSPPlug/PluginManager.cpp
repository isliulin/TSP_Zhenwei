/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件管理实现部分
* @author 刘成伟 chengwei@run.com
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


/// 需要移动到公共目录下，判断目录是否存在
static bool dir_exists(const char *dir)
{
	if( strlen(dir)==2 && dir[1]==':')
		return true;

	DWORD dwAttr = GetFileAttributes(dir );
	if( dwAttr == -1 )
		return false;
	return (dwAttr&FILE_ATTRIBUTE_DIRECTORY)!=0;	
}

/// 需要移动到公共目录下，递归创建目录
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

	//目录结尾不带"\\"
	if( dir[strlen(dir)-1] != DDC_C_SLASH )	
		return CreateDirectory(dir,NULL)==TRUE?0:DDCE_DISK_OP_ERROR;

	return 0;
}

/// 需要移动到公共目录，拷贝文件，没有目录就创建目录
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

// 初始化singleton
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
		//得到安装路径
		GetModuleFileName(NULL,m_szModulePath,TSP_PLUGIN_PATH);
		DDC_RIGHT_SLASH(m_szModulePath);
	}	
	
	//插件目录初始化
	m_szPluginConfigPath.assign(m_szModulePath);
	m_szPluginConfigPath += TSP_PLUGIN_S_CONFIG_PATH;

	//插件下载目录初始化
	// update path to temp by menglz
	
	//m_szPluginDownLoadPath.assign(m_szModulePath);


	char chTempPath[MAX_PATH+1] = {0};

	GetTempPathA(MAX_PATH,chTempPath);
	
	m_szPluginDownLoadPath.assign(chTempPath);

	m_szPluginDownLoadPath += TSP_PLUGIN_S_DOWNLOAD_PATH;

	LOG((LEVEL_FUNC_ROUTE,"CPluginManager::CPluginManager \n"));

	// 加载默认插件信息，不运行
	ReloadPlugin();
	return 0;
}

CPluginManager::~CPluginManager()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::~CPluginManager \n"));

	// 停止所有插件
	StatusAll(CPluginInfo::STATUS_UNLOAD);
}


//解析插件配置文件，下载并装载
BOOL CPluginManager::Update(const string &strPluginDownLoadFile)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::ProcessConfig\n"));

	int ret;	

	// 创建下载目录
	CreateDirectory(m_szPluginDownLoadPath.c_str(),NULL);
	m_recycle.Push(m_szPluginDownLoadPath,CTspRecycle::FLAG_DIR);
	
	// 组装插件配置文件文件名
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	std::string strPluginDownLoadTmpFile;

	// 锁住这个链表，由于网络IO是异步，这个操作需要同步
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);	

	// 读取配置信息到临时链表中
	std::list<CPluginInfo> l;
	if( !ReadPluginIniFile<CPluginInfo>(l,strPluginDownLoadFile,true) )	
	{
		LOG((LEVEL_WARNNING,"解析插件配置文件失败.%s\n",strPluginDownLoadFile.c_str()));
		return FALSE;
	}
	// 去除不属于此功能管理的插件
	ExceptPluginFile(l);

	// 新旧链表的迭代器声明
	std::list<CPluginInfo>::iterator iterNew=l.begin();
	std::list<CPluginInfo>::iterator iterOld;
	char md5[33] = {0};
	BOOL bRet = FALSE;

	// 比较新老列表，确定要下载的项
	for(;iterNew!=l.end();++iterNew)
	{
		// 在老列表中查找
		iterOld = find(m_PluginList.begin(),m_PluginList.end(),iterNew->m_strName);
		if( iterOld != m_PluginList.end() )
		{
			// 找到先做一次自我检测，防止配置文件和实体文件不一致导致不更新
			bRet = MD5_Caculate_File ( iterOld->m_strFullName.c_str(),md5);

			// 比较md5值
			if( bRet && strncmp(iterNew->m_md5,iterOld->m_md5,32) == 0 )
			{
				// 还要看strFullName，如果一样，说明没变化，仅仅复制个标志位，后面交换到老列表中
				if( iterOld->m_strFullName == iterNew->m_strFullName && 
					iterOld->m_pid == iterNew->m_pid )	
				{
					*iterNew = *iterOld;
					
					LOG((LEVEL_INFO,"(the same dll,don't update)  %s : %d\n",iterOld->m_strFullName.c_str(),iterOld->m_status));

					// 用初始状态替代运行状态，防止析构时被卸载
					if( iterOld->m_status == CPluginInfo::STATUS_RUNNING )
						iterOld->m_status = CPluginInfo::STATUS_INIT;
					else
					{
						// 其他状态就卸载再加载吧
						LOG((LEVEL_INFO,"插件未变化，但升级中将被卸载:%s,状态:%d.\n",iterOld->m_strFullName.c_str(),iterOld->m_status));
						iterOld->RemovePlug();	
					}								
				}
				else
				{
					// 直接拷贝到新路径
					ret = ComfirmCopyFile(iterOld->m_strFullName.c_str(),iterNew->m_strFullName.c_str());					
					if( ret != 0 )
					{
						LOG((LEVEL_ERROR,"拷贝文件出错:(%s==>%s),Lasterror=%d\n",iterOld->m_strFullName.c_str(),iterNew->m_strFullName.c_str(),GetLastError()));
					}
					iterNew->m_status = CPluginInfo::STATUS_UNLOAD;	

					// 拷贝过去的插件肯定能启动成功（最少和老插件一样的状态），所以这里把老插件信息完全删除
					iterOld->RemovePlug();
				}
				
				// 新纪录中已经有了，老记录中删除
				m_PluginList.erase(iterOld);				
			}
			else
				// 准备下载
				iterNew->m_status = CPluginInfo::STATUS_WAITDOWNLOAD;
		}
		else
			// 准备下载
			iterNew->m_status = CPluginInfo::STATUS_WAITDOWNLOAD;
	}

	// 下载
	for_each( l.begin(),l.end(),bind2nd(mem_fun_ref(&CPluginInfo::DownloadPlug),m_szPluginDownLoadPath) );

	// 根据下载情况，改记录
	for( iterNew=l.begin();iterNew!=l.end();++iterNew )
	{
		// 在老配置中找
		iterOld = find(m_PluginList.begin(),m_PluginList.end(),iterNew->m_strName.c_str() );

		if( iterNew->m_status == CPluginInfo::STATUS_DOWNLOAD )
		{
			// 组装下载文件名
			strPluginDownLoadTmpFile.assign(m_szPluginDownLoadPath);
			strPluginDownLoadTmpFile += iterNew->m_strName;

			//先加入回收站
			m_recycle.Push(strPluginDownLoadTmpFile);

			// 如果找到，停用，删除
			if( iterOld != m_PluginList.end() )
			{
				ret = iterOld->RemovePlug();
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"卸载插件 %s 出错:%d(%x)\n",iterNew->m_strName.c_str()));
				}
			}

			//将插件文件拷贝到目的地
			ret = ComfirmCopyFile(strPluginDownLoadTmpFile.c_str(),iterNew->m_strFullName.c_str());					
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"拷贝文件出错:(%s==>%s),Lasterror=%d\n",strPluginDownLoadTmpFile.c_str(),iterNew->m_strFullName.c_str(),GetLastError()));
			}

			// 改状态为准备启用
			iterNew->m_status = CPluginInfo::STATUS_UNLOAD;
		}
		else if( iterNew->m_status == CPluginInfo::STATUS_ERROR )
		{
			// 可能下载失败，也可能以前就失败，不管怎样，都把信息复制过去
			if( iterOld != m_PluginList.end() )
				*iterNew = *iterOld;
		}

		//把url删掉，为了不在后面写入配置文件中；写配置文件是个通用的程序
		iterNew->m_strURL = "";
	}

	// 遍历老插件配置文件，如果新配置中没有,删除
	for( iterOld=m_PluginList.begin();iterOld!=m_PluginList.end();++iterOld )
	{
		// 在新配置中找
		iterNew = find(l.begin(),l.end(),iterOld->m_strName.c_str() );

		// 如果没找到,卸载
		if( iterNew == l.end() )
			iterOld->RemovePlug();
			// l.push_back(*iterOld); //插入
	}

	// 根据新配置写加密ini文件
	if( !WritePluginIniFile(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"写配置文件错:%s",strPluginConfigFile.c_str()));
	}

	// 清理现场
	m_recycle.PopAll();

	// 清空老列表，为了避免析构时自动卸载插件，这里强制将状态设为删除
	for_each( m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::Status),CPluginInfo::STATUS_DELETE) );
	m_PluginList.clear();

	// 交换新老插件列表
	m_PluginList.swap(l);

	//把INIT状态的改为RUNNING
	LOG((LEVEL_INFO,"m_PluginList.size()=%d.\n",m_PluginList.size()));
	for( iterOld=m_PluginList.begin();iterOld!=m_PluginList.end();++iterOld )
	{
		LOG((LEVEL_INFO,"%s : %d\n",iterOld->m_strFullName.c_str(),iterOld->m_status));
		if( iterOld->m_status == CPluginInfo::STATUS_INIT )
			iterOld->m_status = CPluginInfo::STATUS_RUNNING;
	}

	// 启用
	StatusAll(CPluginInfo::STATUS_RUNNING);
	

	return TRUE;
}
	
// 去除不属于此部分管理的插件
int CPluginManager::ExceptPluginFile(std::list<CPluginInfo> &l)
{
	for( std::list<CPluginInfo>::iterator iter=l.begin();iter!=l.end();)
	{
		//键盘插件和管理插件由主程序本身管；主程序自己也不管理
		if( iter->m_pid == TSP_PLUGIN_MANAGE ||
			iter->m_pid == TSP_PLUGIN_KEYBOARD ||
			iter->m_pid == TSP_PLUGIN_REMOVE_PID )
			l.erase(iter++);
		else
			iter++;
	}
	return 0;
}

// 从配置文件重新生成插件列表
int CPluginManager::ReloadPlugin(DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPluginManager::ReloadPlugin,flag = %d\n",dwFlag));	
	
	// 卸载所有插件
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::UnLoadPlug),0));
	
	// 清除链表
	m_PluginList.clear();

	// 重新从配置文件读
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	// 如果配置文件不正确，由于下载模块很可能调用此函数，所以不返回错，这样可以通过升级更新到新的
	if( !ReadPluginIniFile<CPluginInfo>(m_PluginList,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"读取插件配置文件错\n"));
		return DDCE_CALL_FAIL;
	}
	
	// 去除不属于此部分管理的插件,不用判断返回值
	ExceptPluginFile(m_PluginList);

	// 设置所有插件状态为未加载	
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::Status),CPluginInfo::STATUS_UNLOAD));			

	return 0;
}

//改变插件状态
int CPluginManager::Status(int pid,int flag)
{
	int ret;

	// 遍历链表
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

				// 需要删除链表中记录
				m_PluginList.erase(iter);
			}
			return ret;
		}
	
	LOG((LEVEL_WARNNING,"插件类型未找到:%d\n",pid));
	return CPluginInfo::STATUS_ERROR;	
}

//获得插件状态
int CPluginManager::Status(int pid)
{
	// 遍历链表
	std::list<CPluginInfo>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( iter->m_pid == pid )
			return iter->m_status;
	
	LOG((LEVEL_WARNNING,"插件类型未找到:%d\n",pid));
	return CPluginInfo::STATUS_ERROR;
}



//改变所有插件状态
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
		// 需要删除链表中记录
		for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun_ref(&CPluginInfo::RemovePlug),0));
		m_PluginList.clear();

		// 除了测试，其他情况只有卸载会调这个，把自己的配置文件也删了		
		std::string strPluginConfigFile(m_szPluginConfigPath);
		strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;
		
		if( !DeleteFile(strPluginConfigFile.c_str()) )
		{
			LOG((LEVEL_ERROR,"删除插件失败(%s):%d \n",strPluginConfigFile.c_str(),GetLastError()));
		}

	}
	else
	{
		LOG((LEVEL_WARNNING,"插件状态未找到:%d\n",flag));
	}

	return 0;
}

