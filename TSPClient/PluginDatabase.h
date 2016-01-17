/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ������ݿ������
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#if !defined(__TSP_PLUGIN_DATABASE_H__)
#define __TSP_PLUGIN_DATABASE_H__

#include "ddc_mysql.h"

// ǰ������
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
* @brief TSP���ݿ�����࣬�ṩTSP����������ݿ��ȡ����
*/
struct CTspDbStorage:public CDbStorage
{
	CTspDbStorage(CDbPool &pool)
		:CDbStorage(pool){};

	///ȡ�����������	 
	int GetIdentity(const string &sql,uint &identity);

	/// ���ݿ���ȡCDBClientNotify��
	int Get(CDBClientNotify &data);

	/// ���ݿ���ȡCDBPlugInfo
	int Get(CDBPlugInfo &data);

	/// ���ݿ���ȡCDBPlugSetting
	int Get(CDBPlugSetting &data);

	/// ���ݿ���ȡCDBCaptureSetting
	int Get(CDBCaptureSetting &data);
};

/**
* @class CDBClientNotify
*
* @brief ��Ӧ���ݿ��client_notify����
*/
class CDBClientNotify
{
public:
	// ��־λ����
	enum { PLUG_INFO=1,PLUG_SETTING=2 };			// plug_info������noֵΪ1��pluggin_setting������noֵΪ2

	// sql����ඨ��
	typedef CSQLClientNotify SQLType;
	friend class CSQLClientNotify;

	/// Constrator
	CDBClientNotify()
	{
		// ����಻�����κ��麯����Ӱ���ڴ沼�ֵ�����
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
* @brief ��Ӧ���ݿ��plug_info����
*/
class CDBPlugInfo
{
public:
	// ��־λ����	
	enum {UNSET=0,SET=1};

	// sql����ඨ��
	typedef CSQLPlugInfo SQLType;
	friend class CSQLPlugInfo;

	/// Constrator
	CDBPlugInfo()
	{
		// ����಻�����κ��麯����Ӱ���ڴ沼�ֵ�����
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
* @brief ��Ӧ���ݿ��plugin_setting����
*/
class CDBPlugSetting
{
public:
	// ��־λ����	
	enum {UNUSE=0,USE=1};	

	// sql����ඨ��
	typedef CSQLPlugSetting SQLType;
	friend class CSQLPlugSetting;

	/// Constrator
	CDBPlugSetting()
	{
		// ����಻�����κ��麯����Ӱ���ڴ沼�ֵ�����
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
	ushort flag;			// Ԥ��

private:
	int ret;
};

/**
* @class CDBCaptureSetting
*
* @brief ��Ӧ���ݿ��capture_setting����
*/
class CDBCaptureSetting
{	
public:
	enum {CAPSET_LEN = 64};

	// sql����ඨ��
	typedef CSQLCaptureSetting SQLType;
	friend class CSQLCaptureSetting;

	/// Constrator
	CDBCaptureSetting()
	{
		// ����಻�����κ��麯����Ӱ���ڴ沼�ֵ�����
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
* @brief SQL����������
*
*/
struct CSheetSqlBase
{	
};


/**
* @class CSQLClientNotify
*
* @brief ���ز���CDBClientNotify��SQL���
*
*/
class CSQLClientNotify:public CSheetSqlBase
{
public:
	///constrator
	CSQLClientNotify( CDBClientNotify &msg)
		:m_data(msg) {}

	/// ɾ������Ϣ��SQL������岿��
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM client_notify");
		return str;
	};

	/// ѡ����Ϣ��SQL������岿��
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

	/// ���ݿ�����������
	static std::string &GetMasterIDName(void)
	{
		static std::string str("id");
		return str;
	}

	/// ������Ϣ��SQL������岿��
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
* @brief ���ز���CDBPlugInfo��SQL���
*
*/
class CSQLPlugInfo:public CSheetSqlBase
{
public:
	///constrator
	CSQLPlugInfo( CDBPlugInfo &msg)
		:m_data(msg) {}

	/// ɾ������Ϣ��SQL������岿��
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM plug_info");
		return str;
	};

	/// ѡ����Ϣ��SQL������岿��
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

	/// ���ݿ�����������
	static std::string &GetMasterIDName(void)
	{
		static std::string str("mid");
		return str;
	}

	/// ������Ϣ��SQL������岿��
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
* @brief ���ز���CPlugSetting��SQL���
*
*/
class CSQLPlugSetting:public CSheetSqlBase
{
public:
	///constrator
	CSQLPlugSetting( CDBPlugSetting &msg)
		:m_data(msg) {}

	/// ɾ������Ϣ��SQL������岿��
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM plugin_setting");
		return str;
	};

	/// ѡ����Ϣ��SQL������岿��
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

	/// ���ݿ�����������
	static std::string &GetMasterIDName(void)
	{
		static std::string str("pid");
		return str;
	}

	/// ������Ϣ��SQL������岿��
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
* @brief ���ز���CDBCaptureSetting��SQL���
*
*/
class CSQLCaptureSetting:public CSheetSqlBase
{
public:
	///constrator
	CSQLCaptureSetting( CDBCaptureSetting &msg)
		:m_data(msg) {}

	/// ɾ������Ϣ��SQL������岿��
	static std::string &GetDeleteSql()
	{		
		static std::string str("DELETE FROM capture_setting");
		return str;
	};

	/// ѡ����Ϣ��SQL������岿��
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

	/// ���ݿ�����������
	static std::string &GetMasterIDName(void)
	{
		static std::string str("mid");
		return str;
	}

	/// ������Ϣ��SQL������岿��
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
* @brief ���ݿ��и�������ȡ���ݵĺ���
*
* ��ģ��Ҫ��ģ�����:
* @concept Ψһ����
* @concept ʵ��GetMasterIDName
* @concept ʵ��GetID
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
// �ػ�CDBPlugInfo���ȡֵ����Ϊ����������
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