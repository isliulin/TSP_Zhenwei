/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief mysql���ݿ������
* @author ����ΰ chengwei@run.com
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
* @brief ���ݿ����ӳ�,�ṩ������Ӻ��ͷ����ӹ��ܣ��ڲ�������
*/
class CDbPool
{	
	friend class mysql;	//��Ԫ������
public:	

	/**
	* constrator	
	* @param options ѡ�Ԥ��
	* @param size ���ӳش�С
	*/
	CDbPool(uint options = 0,uint size = 10	)
		:m_options(options),m_size(size){};

	/**
	* constrator.
	* @param host ����ip
	* @param user �û���
	* @param passwd ����
	* @param db ���ݿ���
	* @param options ѡ�Ԥ��
	* @size ���ӳش�С
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
	* ���ò���
	* @param host ����ip
	* @param user �û���
	* @param passwd ����
	* @param db ���ݿ���
	* @param options ѡ�Ԥ��
	* @size ���ӳش�С
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
	///ȡ��һ������
	virtual CDatabase * Acquire();

	///����һ������
	virtual void Release(CDatabase * lpConnect,bool bClose=false);

protected:

	///�ͷ���������(�ر�)
	void ReleaseAll();

	///����(��)���ݿ�
	virtual CDatabase * InteriorOpen();
	///�ر�����
	virtual void InteriorClose(CDatabase * lpConnect);

	///��ӵ����ӳ�
	virtual bool InteriorRelease(CDatabase * lpConnect);

	///�����ӳ���ȡ��
	virtual CDatabase * InteriorAcquire();

public:	
	nm_ddc::CThreadMutex m_lock;		/// ͬ���� @see nm_ddc::LockableCriticalSection
	std::list<CDatabase *> m_pool;				/// ���ӳ�
	string m_host;								/// ����ip
	string m_user;								/// �û���
	string m_passwd;							/// ����
	string m_db;								/// ���ݿ���
	uint m_options;								/// ѡ��
	uint m_size;								/// ���ӳش�С
};

/**
* @class CDbStorage
*
* @brief  ���ݿ������
*/
class CDbStorage
{
public:
	friend class mysql;	//��Ԫ������

	///���캯�����������ӳ�
	CDbStorage(CDbPool &pool)
		:m_dbpool(pool),m_db(NULL),m_res(NULL) {};
	

	///�򿪼�¼��
	int Prepare(const std::string &);

	///�رռ�¼��
	void Finish();	

	///ִ��SQL���
	int ExecuteSql(const string &);

	/// ������ӳ�
	CDbPool &DBPool(void) { return m_dbpool; };

protected:
	CDbPool &m_dbpool;			/// ���ӳ�����
	CDatabase *m_db;			/// ����featch��䱣������
	MYSQL_RES *m_res;			/// ����featch��䱣��������
};

#endif	// __DDC_MYSQL_H__
