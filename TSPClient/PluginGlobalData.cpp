
#include "PluginGlobalData.h"
#include "TspPluginIni.hpp"
#include "Log.h"

// 初始化singleton
#ifdef _MSC_VER
CGlobalData* SingletonHolder<CGlobalData>::pInstance_ = 0;
#endif

// 初始化全局变量
void CGlobalData::Init()
{

	// 取安装路经
	GetModuleFileName(NULL,szModulePath,MAX_PATH*2);
	DDC_RIGHT_SLASH(szModulePath);

#define TSP_DB_INI_FILE "DBConfig.ini"

	char szDbHost[MAX_PATH],szDbUser[MAX_PATH],szDbPasswd[MAX_PATH],szDbDb[MAX_PATH];
	std::string szDbConfigIni(szModulePath);
	szDbConfigIni += TSP_DB_INI_FILE;
	
	if( _access(szDbConfigIni.c_str(),0)==-1 )	
	{
		LOG((LEVEL_ERROR,"DB Config file does not exist.%s.\n",szDbConfigIni.c_str()));
	}

	GetPrivateProfileString("Config","DBHost","",szDbHost,MAX_PATH,szDbConfigIni.c_str());
	GetPrivateProfileString("Config","DBUser","",szDbUser,MAX_PATH,szDbConfigIni.c_str());
	GetPrivateProfileString("Config","DBPasswd","",szDbPasswd,MAX_PATH,szDbConfigIni.c_str());
	GetPrivateProfileString("Config","DBDb","",szDbDb,MAX_PATH,szDbConfigIni.c_str());

	// 初始化数据库
	dbPool.Set(szDbHost,szDbUser,szDbPasswd,szDbDb);


	LOG((LEVEL_INFO,"pluginmanager 初始化数据库 DBHost=%s,DBUser=%s,DBPasswd=%s,DBDb=%s\n",szDbHost,szDbUser,szDbPasswd,szDbDb));


	nPluginPid = TSP_PLUGIN_REMOVE_PID;

	strncpy(szConfigPath,szModulePath,MAX_PATH*2);

	// 取Application Data路经
	//SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,szAppPath);
	//strcat(szAppPath,"\\");

	// 数据库中取插件配置信息plugin_setting，放入全局数据中

	LOG((LEVEL_INFO,"开始执行LoadPlugSetting\n"));

	LoadPlugSetting();

	LOG((LEVEL_INFO,"执行LoadPlugSetting完毕\n"));

};

// 数据库中取插件配置信息plugin_setting，放入全局数据中
int CGlobalData::LoadPlugSetting(void)
{
	//锁

	
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);


	plugSetting.clear();


	CTspDbStorage dbStorage(dbPool);


	string sql(CDBPlugSetting::SQLType::GetSelectSql());

	int ret = dbStorage.Prepare(sql);


	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"准备数据失败(%s),ret:%d\n",sql.c_str(),ret));
		return ret;
	}

	

	CDBPlugSetting data;

	

	while( 0 == ( ret=dbStorage.Get(data) ))
	{
	
		plugSetting.push_back(make_pair(data.pid,data.used));


	}

	

	dbStorage.Finish();	

	

	return ret;
}

// 
uint CGlobalData::GetPlugSetting(uint pid)
{
	for( std::vector<std::pair<uint,uint> >::const_iterator iter=plugSetting.begin();iter!=plugSetting.end();++iter)
		if( iter->first == pid )
			return iter->second;

	// 返回未启用
	return CDBPlugSetting::UNUSE;
}

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int OnDbNotify(vector<pair<int,int> > &vec)
{
	bool bNeedLoadPluginGlobalSetting = false;

	// 查看数据库表 client_notify
	CTspDbStorage dbStorage( GLOBAL_DATA->dbPool );

	string sql(CDBClientNotify::SQLType::GetSelectSql());

	//LOG((LEVEL_INFO,"sql %s\n",sql.c_str())); //zhenyu

	int ret = dbStorage.Prepare(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"准备数据失败(%s),ret:%d\n",sql.c_str(),ret));
		return ret;
	}

	CDBClientNotify data;

	while( 0 == ( ret=dbStorage.Get(data) ))
	{
		if( data.no == CDBClientNotify::PLUG_INFO )
		{
			// 插件设置修改记录,插入数组中，最后统一处理
			vec.push_back(make_pair(data.arg1,data.arg2));
		}
		else if( data.no == CDBClientNotify::PLUG_SETTING )
			bNeedLoadPluginGlobalSetting = true;			
	}

	dbStorage.Finish();	

	// 如果插件设置修改，重新读取全局变量
	if( bNeedLoadPluginGlobalSetting )
		GLOBAL_DATA->LoadPlugSetting();

	// 清空client_notify表
	sql.assign( CDBClientNotify::SQLType::GetDeleteSql());
	ret = dbStorage.ExecuteSql(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"执行sql失败(%s),ret:%d\n",sql.c_str(),ret));
		return ret;
	}

	return 0;
}


// 首次上线，数据库中插入记录,chengwei 2013-08-06
int TSP2_PluginOnLine(DWORD dwMachineID)
{
	// 直接删除info表mid数据
	CTspDbStorage dbStorage( GLOBAL_DATA->dbPool );

	CDBPlugInfo info;
	CDBPlugInfo::SQLType sqlType(info);
	ostringstream os;
	string sql;

	os << CDBPlugInfo::SQLType::GetDeleteSql() << " where mid="<<dwMachineID;
	sql = os.str();

	
	int ret = dbStorage.ExecuteSql(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_WARNNING,"执行sql失败(%s),ret:%d\n",sql.c_str(),ret));	
	}

	memset(&info,0,sizeof(info));
	// 插入默认数据
	for( std::vector<std::pair<uint,uint> >::const_iterator iter=GLOBAL_DATA->plugSetting.begin();iter!=GLOBAL_DATA->plugSetting.end();++iter)
	{
		info.mid = dwMachineID;
		info.pid = iter->first;
		info.isused = iter->second;
		
		sqlType.GetInsertSql(sql);
		ret = dbStorage.ExecuteSql(sql);
		if( ret != 0 )
		{
			LOG((LEVEL_WARNNING,"执行sql失败(%s),ret:%d\n",sql.c_str(),ret));	
		}
	}

	return 0;
}
