/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief mysql数据库操作类
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#if !defined(__DDC_MYSQL_H__)
#define	__DDC_MYSQL_H__

#include "ddc_util.h"
#include "ddc_thread.h"
#ifdef _MSC_VER
#include <WinSock.h>
#pragma comment(lib,"libmysql.lib")
#endif

#include "mysql.h"

typedef MYSQL CDatabase;

/**
* @class CDbStorage
*
* @brief 数据库连接池,提供获得连接和释放连接功能，内部做缓存
*/
class CDbPool
{	
	friend class mysql;	//单元测试类
public:	

	/**
	* constrator	
	* @param options 选项，预留
	* @param size 连接池大小
	*/
	CDbPool(uint options = 0,uint size = 10	)
		:m_options(options),m_size(size){};

	/**
	* constrator.
	* @param host 主机ip
	* @param user 用户名
	* @param passwd 密码
	* @param db 数据库名
	* @param options 选项，预留
	* @size 连接池大小
	*/
	CDbPool(const char *host, const char *user, const char *passwd, const char *db,
			uint options = 0,
			uint size = 10
		)
		:m_host(host),m_user(user),m_passwd(passwd),m_db(db),m_options(options),m_size(size){};

	/** @brief destrator */
	virtual ~CDbPool()
	{
		ReleaseAll();
	};
	
	/**
	* 设置参数
	* @param host 主机ip
	* @param user 用户名
	* @param passwd 密码
	* @param db 数据库名
	* @param options 选项，预留
	* @size 连接池大小
	*/
	void Set(const char *host, const char *user, const char *passwd, const char *db,
			uint options = 0,
			uint size = 10		)
	{
		m_host.assign(host);
		m_user.assign(user);
		m_passwd.assign(passwd);
		m_db.assign(db);
		m_options = options;
		m_size = size;
	}
		
public:
	///取得一个连接
	virtual CDatabase * Acquire();

	///回收一个连接
	virtual void Release(CDatabase * lpConnect,bool bClose=false);

protected:

	///释放所有连接(关闭)
	void ReleaseAll();

	///连接(打开)数据库
	virtual CDatabase * InteriorOpen();
	///关闭连接
	virtual void InteriorClose(CDatabase * lpConnect);

	///添加到连接池
	virtual bool InteriorRelease(CDatabase * lpConnect);

	///从连接池中取出
	virtual CDatabase * InteriorAcquire();

public:	
	nm_ddc::CThreadMutex m_lock;		/// 同步锁 @see nm_ddc::LockableCriticalSection
	std::list<CDatabase *> m_pool;				/// 连接池
	string m_host;								/// 主机ip
	string m_user;								/// 用户名
	string m_passwd;							/// 密码
	string m_db;								/// 数据库名
	uint m_options;								/// 选项
	uint m_size;								/// 连接池大小
};

/**
* @class CDbStorage
*
* @brief  数据库操作类
*/
class CDbStorage
{
public:
	friend class mysql;	//单元测试类

	///构造函数，传入连接池
	CDbStorage(CDbPool &pool)
		:m_dbpool(pool),m_db(NULL),m_res(NULL) {};
	

	///打开记录集
	int Prepare(const std::string &);

	///关闭记录集
	void Finish();	

	///执行SQL语句
	int ExecuteSql(const string &);

	/// 获得连接池
	CDbPool &DBPool(void) { return m_dbpool; };

protected:
	CDbPool &m_dbpool;			/// 连接池引用
	CDatabase *m_db;			/// 用于featch语句保持连接
	MYSQL_RES *m_res;			/// 用于featch语句保持上下文
};

#endif	// __DDC_MYSQL_H__
