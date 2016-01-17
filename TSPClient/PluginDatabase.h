/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件数据库操作类
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#if !defined(__TSP_PLUGIN_DATABASE_H__)
#define __TSP_PLUGIN_DATABASE_H__

#include "ddc_mysql.h"

// 前置声明
class CDBClientNotify;
class CDBPlugInfo;
class CDBPlugSetting;
class CDBCaptureSetting;

class CSQLPlugSetting;
class CSQLPlugInfo;
class CSQLClientNotify;
class CSQLCaptureSetting;

enum{
	SHEET_DB_SELECT_SUCC,
	SHEET_DB_SELECT_FAIL,
	SHEET_DB_INSERT_SUCC,
	SHEET_DB_INSERT_FAIL,
	SHEET_DB_UPDATE_SUCC,
	SHEET_DB_UPDATE_FAIL,
	SHEET_DB_DELETE_SUCC,
	SHEET_DB_DELETE_FAIL	
};

typedef SingletonHolder<CDbPool> CDBPOOL;


/**
* @class CTspDbStorage
*
* @brief TSP数据库操作类，提供TSP几个类的数据库存取功能
*/
struct CTspDbStorage:public CDbStorage
{
	CTspDbStorage(CDbPool &pool)
		:CDbStorage(pool){};

	///取最大的主键编号	 
	int GetIdentity(const string &sql,uint &identity);

	/// 数据库中取CDBClientNotify类
	int Get(CDBClientNotify &data);

	/// 数据库中取CDBPlugInfo
	int Get(CDBPlugInfo &data);

	/// 数据库中取CDBPlugSetting
	int Get(CDBPlugSetting &data);

	/// 数据库中取CDBCaptureSetting
	int Get(CDBCaptureSetting &data);
};

/**
* @class CDBClientNotify
*
* @brief 对应数据库表client_notify的类
*/
class CDBClientNotify
{
public:
	// 标志位定义
	enum { PLUG_INFO=1,PLUG_SETTING=2 };			// plug_info表触发的no值为1，pluggin_setting表触发的no值为2

	// sql语句类定义
	typedef CSQLClientNotify SQLType;
	friend class CSQLClientNotify;

	/// Constrator
	CDBClientNotify()
	{
		// 这个类不能有任何虚函数等影响内存布局的声明
		memset(this,0,sizeof(CDBClientNotify));
	}

	/// Set the Primary Key
	void SetID(DWORD arg){id = (WORD)arg;};

	/// Get Key
	DWORD GetID(void) const {return id;};

	WORD id;	
	int no;
	uint arg1;
	uint arg2;
	uint arg3;
	uint arg4;
	char arg5[260];

private:
	int ret;
};

/**
* @class CDBPlugInfo
*
* @brief 对应数据库表plug_info的类
*/
class CDBPlugInfo
{
public:
	// 标志位定义	
	enum {UNSET=0,SET=1};

	// sql语句类定义
	typedef CSQLPlugInfo SQLType;
	friend class CSQLPlugInfo;

	/// Constrator
	CDBPlugInfo()
	{
		// 这个类不能有任何虚函数等影响内存布局的声明
		memset(this,0,sizeof(CDBPlugInfo));
	}

	/// Set the Primary Key
	void SetID(DWORD id){mid = id;};

	/// Get Key
	DWORD GetID(void) const {return mid;};

	uint mid;
	int pid;
	ushort isused;	// UNSET SET

private:
	int ret;
};

/**
* @class CDBPlugSetting
*
* @brief 对应数据库表plugin_setting的类
*/
class CDBPlugSetting
{
public:
	// 标志位定义	
	enum {UNUSE=0,USE=1};	

	// sql语句类定义
	typedef CSQLPlugSetting SQLType;
	friend class CSQLPlugSetting;

	/// Constrator
	CDBPlugSetting()
	{
		// 这个类不能有任何虚函数等影响内存布局的声明
		memset(this,0,sizeof(CDBPlugSetting));
	}

	/// Set the Primary Key
	void SetID(DWORD id){pid = id;};

	/// Get Key
	DWORD GetID(void) const {return pid;};

	int pid;
	char pname[MAX_PATH];
	uint used;				// USE UNUSE
	ushort setting;
	ushort flag;			// 预留

private:
	int ret;
};

/**
* @class CDBCaptureSetting
*
* @brief 对应数据库表capture_setting的类
*/
class CDBCaptureSetting
{	
public:
	enum {CAPSET_LEN = 64};

	// sql语句类定义
	typedef CSQLCaptureSetting SQLType;
	friend class CSQLCaptureSetting;

	/// Constrator
	CDBCaptureSetting()
	{
		// 这个类不能有任何虚函数等影响内存布局的声明
		memset(this,0,sizeof(CDBCaptureSetting));
	}

	/// Set the Primary Key
	void SetID(DWORD id){mid = id;};

	/// Get Key
	DWORD GetID(void) const {return mid;};

	uint mid;
	int capquality;
	uint m_intervel; //add by yx
	char capset[CAPSET_LEN];
private:
	int ret;
};


////////////////////////////////////////////////

/**
* @class CSheetSqlBase
*
* @brief SQL语句操作基类
*
*/
struct CSheetSqlBase
{	
};


/**
* @class CSQLClientNotify
*
* @brief 返回操作CDBClientNotify的SQL语句
*
*/
class CSQLClientNotify:public CSheetSqlBase
{
public:
	///constrator
	CSQLClientNotify( CDBClientNotify &msg)
		:m_data(msg) {}

	/// 删除此消息的SQL语句主体部分
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM client_notify");
		return str;
	};

	/// 选择消息的SQL语句主体部分
	static std::string &GetSelectSql()
	{		
		static std::string str("SELECT id,no,arg1,arg2,arg3,arg4,arg5 FROM client_notify");
		return str;
	};

	/// Identity SQL
	static std::string &GetIdentitySql()
	{	
		static std::string str("select auto_increment from information_schema.tables where table_name='client_notify'");
		return str;
	};

	/// 数据库中主键名称
	static std::string &GetMasterIDName(void)
	{
		static std::string str("id");
		return str;
	}

	/// 插入消息的SQL语句主体部分
	std::string &GetInsertSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "INSERT INTO client_notify(no,arg1,arg2,arg3,arg4,arg5) values (";		
		os<< (int)m_data.no<< ",";
		os<< (uint)m_data.arg1 << ",";
		os<< (uint)m_data.arg2 << ",";
		os<< (uint)m_data.arg3 << ",";
		os<< (uint)m_data.arg4 << ",";
		os<< "'" << m_data.arg5 << "')";

		str = os.str();
		return str;
	};

	/// Update SQL
	std::string &GetUpdateSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "UPDATE client_notify set ";				
		os<< "no=" << (int)m_data.no << ",";
		os<< "arg1=" << (uint)m_data.arg1 << ",";		
		os<< "arg2=" << (uint)m_data.arg2 << ",";		

		os<< "arg3=" << (uint)m_data.arg3 << ",";		
		os<< "arg4=" << (uint)m_data.arg4 << ",";		
		os<< "arg5='" << m_data.arg5 << "'";
		os<<" where id=" << (int)m_data.id;
		str = os.str();
		return str;
	};	


private:
	CDBClientNotify &m_data;
};

/**
* @class CSQLPlugInfo
*
* @brief 返回操作CDBPlugInfo的SQL语句
*
*/
class CSQLPlugInfo:public CSheetSqlBase
{
public:
	///constrator
	CSQLPlugInfo( CDBPlugInfo &msg)
		:m_data(msg) {}

	/// 删除此消息的SQL语句主体部分
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM plug_info");
		return str;
	};

	/// 选择消息的SQL语句主体部分
	static std::string &GetSelectSql()
	{		
		static std::string str("SELECT mid,pid,isused FROM plug_info");
		return str;
	};

	/// Identity SQL
	static std::string &GetIdentitySql()
	{	
		static std::string str("SELECT max(mid) as 'Identity' FROM plug_info");
		return str;
	};

	/// 数据库中主键名称
	static std::string &GetMasterIDName(void)
	{
		static std::string str("mid");
		return str;
	}

	/// 插入消息的SQL语句主体部分
	std::string &GetInsertSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "INSERT INTO plug_info(mid,pid,isused) values (";		
		os<< (uint)m_data.mid << ",";
		os<< (int)m_data.pid << ",";
		os<< (uint)m_data.isused << ")";
		
		str = os.str();
		return str;
	};

	/// Update SQL
	std::string &GetUpdateSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "UPDATE plug_info set ";				
		os<< "isused='" << (uint)m_data.isused << "'";		
		os<<" where mid=" << (uint)m_data.mid << " and pid=" << (int)m_data.pid;
		str = os.str();
		return str;
	};	


private:
	CDBPlugInfo &m_data;
};


/**
* @class CSQLPlugSetting
*
* @brief 返回操作CPlugSetting的SQL语句
*
*/
class CSQLPlugSetting:public CSheetSqlBase
{
public:
	///constrator
	CSQLPlugSetting( CDBPlugSetting &msg)
		:m_data(msg) {}

	/// 删除此消息的SQL语句主体部分
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM plugin_setting");
		return str;
	};

	/// 选择消息的SQL语句主体部分
	static std::string &GetSelectSql()
	{		
		static std::string str("SELECT pid,pname,used,setting,flag FROM plugin_setting");
		return str;
	};

	/// Identity SQL
	static std::string &GetIdentitySql()
	{	
		static std::string str("SELECT max(pid) as identity FROM plugin_setting");
		return str;
	};

	/// 数据库中主键名称
	static std::string &GetMasterIDName(void)
	{
		static std::string str("pid");
		return str;
	}

	/// 插入消息的SQL语句主体部分
	std::string &GetInsertSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "INSERT INTO plugin_setting(pid,pname,used,setting,flag) values (";		
		os<< (int)m_data.pid << ",";
		os<< "'" << m_data.pname << "',";
		os<< (uint)m_data.used << ",";
		os<< (uint)m_data.setting << ",";
		os<<(uint) m_data.flag << ")";

		str = os.str();
		return str;
	};

	/// Update SQL
	std::string &GetUpdateSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "UPDATE plugin_setting set ";	
		os<< "pname='" << m_data.pname << "',";
		os<< "used=" << (uint)m_data.used << ",";
		os<< "flag=" << (uint)m_data.flag << "";
		os<<" where pid=" <<(int) m_data.pid << "";
		str = os.str();
		return str;
	};	


private:
	CDBPlugSetting &m_data;
};

/**
* @class CSQLCaptureSetting
*
* @brief 返回操作CDBCaptureSetting的SQL语句
*
*/
class CSQLCaptureSetting:public CSheetSqlBase
{
public:
	///constrator
	CSQLCaptureSetting( CDBCaptureSetting &msg)
		:m_data(msg) {}

	/// 删除此消息的SQL语句主体部分
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM capture_setting");
		return str;
	};

	/// 选择消息的SQL语句主体部分
	static std::string &GetSelectSql()
	{
		//modify by yx 2014-07-31 start
		
		//static std::string str("SELECT mid,capquality,capset FROM capture_setting");
		
		static std::string str("SELECT mid,capquality,capset,`interval` FROM capture_setting"); //add a vaule:interval
		//modify by yx 2014-07-31 end
		
		return str;
	};

	/// Identity SQL
	static std::string &GetIdentitySql()
	{	
		static std::string str("SELECT max(mid) as 'Identity' FROM capture_setting");
		return str;
	};

	/// 数据库中主键名称
	static std::string &GetMasterIDName(void)
	{
		static std::string str("mid");
		return str;
	}

	/// 插入消息的SQL语句主体部分
	std::string &GetInsertSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "INSERT INTO capture_setting(mid,capquality,`interval`,capset) values (";		
		os<< (uint)m_data.mid << ",";
		os<< (int)m_data.capquality << ",";
		os<< (int)m_data.m_intervel << ",";
		os<< "'" << m_data.capset << "')";

		str = os.str();
		return str;
	};

	/// Update SQL

	std::string &GetUpdateSql(std::string &str)
	{
		std::ostringstream os;
		os.str("");

		os << "UPDATE capture_setting set ";				
		os<< "capquality=" << (int)m_data.capquality << ",";
		os<< "`interval`=" << (int)m_data.m_intervel << ",";
		os<< "capset='" << m_data.capset << "'";
		os<<" where mid=" << (uint)m_data.mid;
		str = os.str();
		return str;
	};	


private:
	CDBCaptureSetting &m_data;
};


/**
* @brief 数据库中根据主健取数据的函数
*
* 主模板要求模板参数:
* @concept 唯一主健
* @concept 实现GetMasterIDName
* @concept 实现GetID
*/
template<class T>
inline int TspDbGet(T &data,CDbPool &pool)
{
	CTspDbStorage dbStorage(pool);	
	typename T::SQLType sqlType(data);
	int ret;

	ostringstream os;
	os<<sqlType.GetSelectSql() << " where " <<sqlType.GetMasterIDName() << "=" << data.GetID();
	ret = dbStorage.Prepare(os.str());
	if( ret != 0 )
		return ret;

	ret = dbStorage.Get(data);
	dbStorage.Finish();	

	return ret;
}
// 特化CDBPlugInfo类的取值，因为有两个主健
template<>
inline int TspDbGet( CDBPlugInfo &data,CDbPool &pool )
{
	CTspDbStorage dbStorage(pool);	
	CDBPlugInfo::SQLType sqlType(data);
	int ret;

	ostringstream os;
	os<<sqlType.GetSelectSql() << " where " <<sqlType.GetMasterIDName() << "=" << data.GetID() << " and pid="<< data.pid;
	ret = dbStorage.Prepare(os.str());
	if( ret != 0 )
		return ret;

	ret = dbStorage.Get(data);
	dbStorage.Finish();	

	return ret;
}

#endif	//__TSP_PLUGIN_DATABASE_H__