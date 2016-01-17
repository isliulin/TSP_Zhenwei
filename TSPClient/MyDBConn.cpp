#include "stdafx.h"
#include "MyDBConn.h"
#include "Log.h"





CMyDataBase::CMyDataBase()
{
	
	dbConn = NULL;




}


CMyDataBase::~CMyDataBase()
{

	CloseDB();



}



void CMyDataBase::CloseDB()
{


	if(NULL != dbConn)
	{

		mysql_close(dbConn);
		free(dbConn);
		dbConn = NULL;
	}



}



void CMyDataBase::FreeResult(MYSQL_RES	*result)
{

	mysql_free_result(result);
	
}



BOOL CMyDataBase::ConnDB()
{


	BOOL bRes = FALSE;

	TCHAR tchHost[16];
	TCHAR tchUser[100];
	TCHAR tchPass[100];


	


	dbConn = (MYSQL *)malloc(sizeof(MYSQL));
	if(NULL == dbConn)
	{

#ifdef _MYDEBUG
		g_log.Log(LEVEL_ERROR,"数据库句柄申请内存失败\n");

#endif
		goto CONN_TO_DB_FUNC_END;
	}

	memset(dbConn,0,sizeof(MYSQL));



	mysql_init(dbConn);


	memset(tchHost,0,sizeof(tchHost));
	memset(tchUser,0,sizeof(tchUser));
	memset(tchPass,0,sizeof(tchPass));

	GetPrivateProfileString("Config","DBHost","10.0.16.9",tchHost,16,DB_CONF_FILE);
	GetPrivateProfileString("Config","DBUser","runvista",tchUser,100,DB_CONF_FILE);
	GetPrivateProfileString("Config","DBPasswd","runco",tchPass,100,DB_CONF_FILE);

	if(!mysql_real_connect(dbConn,tchHost,tchUser,tchPass,"tspcenter",33068,NULL,0))
	{
		


#ifdef _MYDEBUG
		g_log.Log(LEVEL_ERROR,"CMydatabase数据库连接失败\n");
		g_log.Log(LEVEL_ERROR,"%s\n",mysql_error(dbConn));

#endif


		mysql_close(dbConn);
		free(dbConn);
		dbConn = NULL;

		goto CONN_TO_DB_FUNC_END;
	}

	bRes = TRUE;
	

CONN_TO_DB_FUNC_END:
	

	return bRes;

}



BOOL CMyDataBase::ReConnDB()
{

	CloseDB();
	return ConnDB();

}


BOOL CMyDataBase::ExeSQL(char *psql)
{
	if(NULL == psql)
	{
		return FALSE;
	}
	
	BOOL bRes = FALSE;

	if(0 == mysql_query(dbConn,psql))
	{
		
		bRes = TRUE;
	}
	else
	{

#ifdef _MYDEBUG
		g_log.Log(LEVEL_ERROR,"CMydatabase执行[%s]时出错\n",psql);
		g_log.Log(LEVEL_ERROR,"%s\n",mysql_error(dbConn));

#endif

	}


	return bRes;

}



MYSQL_RES	*CMyDataBase::GetDataSet(char *psql)
{
	if(NULL == psql)
	{
		return NULL;
	}

	

	if(0 != mysql_query(dbConn,psql))
	{

#ifdef _MYDEBUG
		g_log.Log(LEVEL_ERROR,"CMydatabase执行[%s]时出错\n",psql);
		g_log.Log(LEVEL_ERROR,"%s\n",mysql_error(dbConn));

#endif

		return NULL;

	}

	return mysql_store_result(dbConn);



}

MYSQL_RES * CMyDataBase::OpenRecordset(const char* chSql)
{
	MYSQL_RES *rs = NULL;
	if (ExecuteSql(chSql)) 
	{
		if ((rs = mysql_store_result(dbConn)) == NULL)
		{
		//	g_log.Log(LEVEL_ERROR,"%s,	[%s][%d]","调用mysql_store_result函数失败",__FILE__,__LINE__);
			//g_log.Log(LEVEL_ERROR,"%s,	[%s][%d]",mysql_error(&mysql),__FILE__,__LINE__);
		}
	}
	return rs;
}

bool CMyDataBase::ExecuteSql(const char* chSql)
{

	if (mysql_real_query(dbConn, chSql, strlen(chSql)) == 0)
	{
		return true;
	}

}