#pragma once 
#include <Windows.h>
#include "mysql.h"


extern "C" _declspec(dllimport) bool TSPsqlExecuteSql(const char* chSql);  //执行指定SQL查询。
extern "C" _declspec(dllimport) MYSQL_RES * TSPsqlOpenRecordset(const char* chSql); //检索完整的结果集
extern "C" _declspec(dllimport) void TSPsqlFreeResult(MYSQL_RES *result); //释放结果集使用的内存
extern "C" _declspec(dllimport) bool TSPsqlOpen();     //连接数据库
extern "C" _declspec(dllimport) bool TSPsqlGetConState();   //获取连接状态
extern "C" _declspec(dllimport) bool TSPsqlReConnect();  //重连数据库函数
extern "C" _declspec(dllimport) void TSPsqlDestoryLogobj(); //关闭数据库连接






