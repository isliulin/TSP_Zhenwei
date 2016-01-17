/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ������ݿ������ʵ��
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/
#include "PluginDatabase.h"

#include "Log.h"



#define DBROW_USHORT(length,row,index) length[index]?(ushort)strtoul(row[index],NULL,10):0
#define DBROW_INT(length,row,index) length[index]?atoi(row[index]):0
#define DBROW_UINT(length,row,index) length[index]?strtoul(row[index],NULL,10):0
#define DBROW_CHAR(length,row,index) (length[index]?(char *)row[index]:"")

// ��ʼ��singleton
#ifdef _MSC_VER
CDbPool* SingletonHolder<CDbPool>::pInstance_ = 0;
#endif

// ȡCDBPlugInfo������
int CTspDbStorage::Get(CDBPlugInfo &data)
{
	if( !m_res )
		return DDCE_END_OF_FILE;

	memset(&data,0,sizeof(data));
	MYSQL_ROW row;
	row = mysql_fetch_row(m_res);
	ulong *lengths = mysql_fetch_lengths(m_res);
	if( row && !lengths )
		return DDCE_DB_QUERY_ERROR;
	if( row )
	{
		data.mid = DBROW_UINT(lengths,row,0);
		data.pid = DBROW_INT(lengths,row,1);
		data.isused = DBROW_USHORT(lengths,row,2);
	}

	return row?0:DDCE_END_OF_FILE;
}

// ȡCDBPlugSetting������
int CTspDbStorage::Get(CDBPlugSetting &data)
{
	
	//LOG((LEVEL_INFO,"��ʼ����Get\n"));
	
	if( !m_res )
		return DDCE_END_OF_FILE;

	memset(&data,0,sizeof(data));
	MYSQL_ROW row;
	row = mysql_fetch_row(m_res);
	ulong *lengths = mysql_fetch_lengths(m_res);
	if( row && !lengths )
		return DDCE_DB_QUERY_ERROR;

	if( row )
	{
		data.pid = DBROW_INT(lengths,row,0);
		strncpy(data.pname,DBROW_CHAR(lengths,row,1),MAX_PATH);
		data.used = DBROW_UINT(lengths,row,2);
		data.setting = DBROW_USHORT(lengths,row,3);
		data.flag = DBROW_USHORT(lengths,row,4);
	}

	//LOG((LEVEL_INFO,"�˳�Get\n"));

	return row?0:DDCE_END_OF_FILE;
}

// ȡCDBClientNotify������
int CTspDbStorage::Get(CDBClientNotify &data)
{
	if( !m_res )
		return DDCE_END_OF_FILE;

	memset(&data,0,sizeof(data));
	MYSQL_ROW row;
	row = mysql_fetch_row(m_res);
	ulong *lengths = mysql_fetch_lengths(m_res);
	if( row && !lengths )
		return DDCE_DB_QUERY_ERROR;

	if( row )
	{
		data.id = DBROW_INT(lengths,row,0);
		data.no = DBROW_INT(lengths,row,1);
		data.arg1 = DBROW_UINT(lengths,row,2);
		data.arg2 = DBROW_UINT(lengths,row,3);
		data.arg3 = DBROW_UINT(lengths,row,4);
		data.arg4  = DBROW_UINT(lengths,row,5);
		strncpy(data.arg5,DBROW_CHAR(lengths,row,6),260);
				
	}

	return row?0:DDCE_END_OF_FILE;
}

// ȡCDBPlugInfo������
int CTspDbStorage::Get(CDBCaptureSetting &data)
{
	if( !m_res )
		return DDCE_END_OF_FILE;

	memset(&data,0,sizeof(data));
	MYSQL_ROW row;
	row = mysql_fetch_row(m_res);
	ulong *lengths = mysql_fetch_lengths(m_res);
	if( row && !lengths )
		return DDCE_DB_QUERY_ERROR;
	if( row )
	{
		data.mid = DBROW_UINT(lengths,row,0);
		data.capquality = DBROW_INT(lengths,row,1);
		strncpy(data.capset,DBROW_CHAR(lengths,row,2),CDBCaptureSetting::CAPSET_LEN);
		data.m_intervel = DBROW_UINT(lengths,row,3); ////add by yx
	}

	return row?0:DDCE_END_OF_FILE;
}


// ȡ����������1
int CTspDbStorage::GetIdentity(const string &sql,uint &identity)
{
	int ret = Prepare(sql);
	if( ret != 0 )
		return ret;
	
	// ȡ��1��
	MYSQL_ROW row = mysql_fetch_row(m_res);
	if( row && row[0] )
		identity = atoi(row[0])+1;
	else
		identity = 1;
	Finish();
	return 0;
}
