
#include "PluginGlobalData.h"
#include "TspPluginIni.hpp"
#include "Log.h"

// ��ʼ��singleton
#ifdef _MSC_VER
CGlobalData* SingletonHolder<CGlobalData>::pInstance_ = 0;
#endif

// ��ʼ��ȫ�ֱ���
void CGlobalData::Init()
{

	// ȡ��װ·��
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

	// ��ʼ�����ݿ�
	dbPool.Set(szDbHost,szDbUser,szDbPasswd,szDbDb);


	LOG((LEVEL_INFO,"pluginmanager ��ʼ�����ݿ� DBHost=%s,DBUser=%s,DBPasswd=%s,DBDb=%s\n",szDbHost,szDbUser,szDbPasswd,szDbDb));


	nPluginPid = TSP_PLUGIN_REMOVE_PID;

	strncpy(szConfigPath,szModulePath,MAX_PATH*2);

	// ȡApplication Data·��
	//SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,szAppPath);
	//strcat(szAppPath,"\\");

	// ���ݿ���ȡ���������Ϣplugin_setting������ȫ��������

	LOG((LEVEL_INFO,"��ʼִ��LoadPlugSetting\n"));

	LoadPlugSetting();

	LOG((LEVEL_INFO,"ִ��LoadPlugSetting���\n"));

};

// ���ݿ���ȡ���������Ϣplugin_setting������ȫ��������
int CGlobalData::LoadPlugSetting(void)
{
	//��

	
	nm_ddc::CLockGuard<nm_ddc::CThreadMutex> guard(&m_lock);


	plugSetting.clear();


	CTspDbStorage dbStorage(dbPool);


	string sql(CDBPlugSetting::SQLType::GetSelectSql());

	int ret = dbStorage.Prepare(sql);


	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"׼������ʧ��(%s),ret:%d\n",sql.c_str(),ret));
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

	// ����δ����
	return CDBPlugSetting::UNUSE;
}

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int OnDbNotify(vector<pair<int,int> > &vec)
{
	bool bNeedLoadPluginGlobalSetting = false;

	// �鿴���ݿ�� client_notify
	CTspDbStorage dbStorage( GLOBAL_DATA->dbPool );

	string sql(CDBClientNotify::SQLType::GetSelectSql());

	//LOG((LEVEL_INFO,"sql %s\n",sql.c_str())); //zhenyu

	int ret = dbStorage.Prepare(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"׼������ʧ��(%s),ret:%d\n",sql.c_str(),ret));
		return ret;
	}

	CDBClientNotify data;

	while( 0 == ( ret=dbStorage.Get(data) ))
	{
		if( data.no == CDBClientNotify::PLUG_INFO )
		{
			// ��������޸ļ�¼,���������У����ͳһ����
			vec.push_back(make_pair(data.arg1,data.arg2));
		}
		else if( data.no == CDBClientNotify::PLUG_SETTING )
			bNeedLoadPluginGlobalSetting = true;			
	}

	dbStorage.Finish();	

	// �����������޸ģ����¶�ȡȫ�ֱ���
	if( bNeedLoadPluginGlobalSetting )
		GLOBAL_DATA->LoadPlugSetting();

	// ���client_notify��
	sql.assign( CDBClientNotify::SQLType::GetDeleteSql());
	ret = dbStorage.ExecuteSql(sql);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"ִ��sqlʧ��(%s),ret:%d\n",sql.c_str(),ret));
		return ret;
	}

	return 0;
}


// �״����ߣ����ݿ��в����¼,chengwei 2013-08-06
int TSP2_PluginOnLine(DWORD dwMachineID)
{
	// ֱ��ɾ��info��mid����
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
		LOG((LEVEL_WARNNING,"ִ��sqlʧ��(%s),ret:%d\n",sql.c_str(),ret));	
	}

	memset(&info,0,sizeof(info));
	// ����Ĭ������
	for( std::vector<std::pair<uint,uint> >::const_iterator iter=GLOBAL_DATA->plugSetting.begin();iter!=GLOBAL_DATA->plugSetting.end();++iter)
	{
		info.mid = dwMachineID;
		info.pid = iter->first;
		info.isused = iter->second;
		
		sqlType.GetInsertSql(sql);
		ret = dbStorage.ExecuteSql(sql);
		if( ret != 0 )
		{
			LOG((LEVEL_WARNNING,"ִ��sqlʧ��(%s),ret:%d\n",sql.c_str(),ret));	
		}
	}

	return 0;
}
