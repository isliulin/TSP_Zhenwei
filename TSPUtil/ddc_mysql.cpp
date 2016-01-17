/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ���ݿ������
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include "ddc_mysql.h"
#include "ddc_util.h"
#include "Log.h"
using namespace nm_ddc;

//ȡ��һ������
CDatabase * CDbPool::Acquire()
{
	// �ӻ�����ȡһ��
	CDatabase * pConnect = InteriorAcquire();

	// ���´�һ��
	if( pConnect == NULL )
	{
		LOG((LEVEL_INFO,"new db connect acquire.\n"));
		pConnect = InteriorOpen();	
	}

	return pConnect;
}

//����һ������
void CDbPool::Release(CDatabase * lpConnect,bool bClose)
{
	if ( !lpConnect )
	{

	}

	// ���Ҫ��ر�
	if( bClose )
	{
		InteriorClose(lpConnect);
		return;
	}
	
	//���ﵽ�����������ʱ�򣬽���InteriorClose����
	if (!InteriorRelease(lpConnect))
		InteriorClose(lpConnect);
}


//�ͷ���������(�ر�)
void CDbPool::ReleaseAll()
{
	CDatabase * pConnect = NULL;
	
	//ѭ���ӳ���ȡ�в��ر�����
	while( pConnect = InteriorAcquire() )
		InteriorClose(pConnect);
	
	m_pool.clear();
	
}

//����(��)���ݿ�
CDatabase * CDbPool::InteriorOpen()
{	
	//�������ݿ�
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

//�ر�����
void CDbPool::InteriorClose(CDatabase * lpConnect)
{
	if( lpConnect )
	{
		mysql_close(lpConnect);
		DDC_DELETE(lpConnect);
	}
}

//��ӵ����ӳ�
//�����Ӳ�Ϊ�ն���δ�ﵽ������������ʱ��������
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

//�����ӳ���ȡ��
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
		//ACE_ERROR_RETURN((LM_ERROR,"[%T %l(%t)] ��ȡ���ݿ�����ʧ��.\n"),DDCE_DB_CONNECT_ERROR);

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
		//ACE_ERROR_RETURN((LM_ERROR,"[%T %l(%t)] ��ȡ���ݿ�����ʧ��.\n"),VIRUS_DB_CONNECT_ERROR);
	
	int ret = DDCE_DB_OP_ERROR;
	ret = mysql_real_query(pdb, str.c_str(),str.length() );				

	m_dbpool.Release(pdb,ret!=0);

	return ret==0?0:DDCE_DB_OP_ERROR;	
}
