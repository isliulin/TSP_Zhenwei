#pragma  once

#include   <winsock.h>


#include "mysql.h"

#pragma comment(lib,"libmySQL.lib")


#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

#define    DB_CONF_FILE     ".\\DBConfig.ini"





class CMyDataBase
{

public:
	CMyDataBase();
	~CMyDataBase();

	BOOL ConnDB();
	BOOL ReConnDB();

	void CloseDB();

	BOOL ExeSQL(char *psql);

	MYSQL_RES	*GetDataSet(char *psql);

	void FreeResult(MYSQL_RES	*result);
	MYSQL_RES * OpenRecordset(const char* chSql);
	bool ExecuteSql(const char* chSql);
	

private:
	MYSQL *dbConn;


};



