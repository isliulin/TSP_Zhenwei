/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 数据库管理类
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include "ddc_mysql.h"
#include "ddc_util.h"
#include "Log.h"
using namespace nm_ddc;

//取得一个连接
CDatabase * CDbPool::Acquire()
{
	// 从缓存中取一个
	CDatabase * pConnect = InteriorAcquire();

	// 重新打开一个
	if( pConnect == NULL )
	{
		LOG((LEVEL_INFO,"new db connect acquire.\n"));
		pConnect = InteriorOpen();	
	}

	return pConnect;
}

//回收一个连接
void CDbPool::Release(CDatabase * lpConnect,bool bClose)
{
	if ( !lpConnect )
	{

	}

	// 如果要求关闭
	if( bClose )
	{
		InteriorClose(lpConnect);
		return;
	}
	
	//当达到最大连接数的时候，交由InteriorClose回收
	if (!InteriorRelease(lpConnect))
		InteriorClose(lpConnect);
}


//释放所有连接(关闭)
void CDbPool::ReleaseAll()
{
	CDatabase * pConnect = NULL;
	
	//循环从池中取中并关闭连接
	while( pConnect = InteriorAcquire() )
		InteriorClose(pConnect);
	
	m_pool.clear();
	
}

//连接(打开)数据库
CDatabase * CDbPool::InteriorOpen()
{	
	//连接数据库
	CDatabase * pConnect;
	DDC_NEW_RETURN(pConnect,CDatabase,NULL);

	try
	{		
		if (!mysql_init(pConnect))
		{
			DDC_DELETE(pConnect);
			return NULL;
		}
		if (!mysql_real_connect(pConnect,m_host.c_str(),m_user.c_str(),m_passwd.c_str(),m_db.c_str(), 33068, NULL, 0)) 
		{
			DDC_DELETE(pConnect);
			return NULL;
		}
		mysql_set_character_set(pConnect,"GBK");

	}
	catch(...)
	{
		DDC_DELETE(pConnect);
		return NULL;
	}
	
	return pConnect;
}

//关闭连接
void CDbPool::InteriorClose(CDatabase * lpConnect)
{
	if( lpConnect )
	{
		mysql_close(lpConnect);
		DDC_DELETE(lpConnect);
	}
}

//添加到连接池
//当连接不为空而且未达到最大空闲连接数时，返回真
bool CDbPool::InteriorRelease(CDatabase * lpConnect)
{
	if (lpConnect)
	{
		nm_ddc::CLockGuard<CThreadMutex> guard(&m_lock);		

		if ( m_pool.size() < m_size )
		{
			m_pool.push_back(lpConnect);
			return true;
		}
	}

	return false;
}

//从连接池中取出
CDatabase * CDbPool::InteriorAcquire()
{
	CDatabase * pConnect = NULL;

	nm_ddc::CLockGuard<CThreadMutex> guard(&m_lock);

	if( !m_pool.empty() )
	{
		pConnect = m_pool.back();
		m_pool.pop_back();
	}

	return pConnect;
}


//////////////////////////////////////////////////

void CDbStorage::Finish() 
{
	if (m_res)
	{
		mysql_free_result(m_res);
		m_res = NULL;
	}
	if( m_db )
	{
		m_dbpool.Release(m_db);
		m_db = NULL;
	}
	
}

int CDbStorage::Prepare(const std::string &sql)
{	

	m_db = m_dbpool.Acquire();
	if( !m_db )
		RETURN( DDCE_DB_CONNECT_ERROR );
		//ACE_ERROR_RETURN((LM_ERROR,"[%T %l(%t)] 获取数据库连接失败.\n"),DDCE_DB_CONNECT_ERROR);

	int ret = 0;
	
	ret = mysql_real_query(m_db, sql.c_str(),sql.length() );


	//if (0 != ret)
	//{
	//	LOG((LEVEL_INFO,"sql %s\n",mysql_error(m_db))); //zhenyu
	//	LOG((LEVEL_INFO,"yyyyyyyyyyyyyyyyy \n")); //zhenyu
	//}

	//LOG((LEVEL_INFO,"xxxxxxxxxxxxxxxxxxxx \n")); //zhenyu

	m_res = mysql_store_result(m_db);

	return ret;
};


int CDbStorage::ExecuteSql( const string &str )
{
	if (str == "" )//|| m_bIsStoping)
		RETURN( DDCE_ARGUMENT_NULL );
		//ACE_ERROR_RETURN((LM_NOTICE,"[%T %l(%t)] ExecuteSQL() parameter error! pszSQL = %s.\n", str.c_str()),DDCE_DB_QUERY_ERROR);

	CDatabase *pdb = m_dbpool.Acquire();
	if( !pdb )
		RETURN(DDCE_DB_CONNECT_ERROR);
		//ACE_ERROR_RETURN((LM_ERROR,"[%T %l(%t)] 获取数据库连接失败.\n"),VIRUS_DB_CONNECT_ERROR);
	
	int ret = DDCE_DB_OP_ERROR;
	ret = mysql_real_query(pdb, str.c_str(),str.length() );				

	m_dbpool.Release(pdb,ret!=0);

	return ret==0?0:DDCE_DB_OP_ERROR;	
}
