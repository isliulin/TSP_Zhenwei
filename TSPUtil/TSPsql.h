#pragma once 
#include <Windows.h>
#include "mysql.h"


extern "C" _declspec(dllimport) bool TSPsqlExecuteSql(const char* chSql);  //ִ��ָ��SQL��ѯ��
extern "C" _declspec(dllimport) MYSQL_RES * TSPsqlOpenRecordset(const char* chSql); //���������Ľ����
extern "C" _declspec(dllimport) void TSPsqlFreeResult(MYSQL_RES *result); //�ͷŽ����ʹ�õ��ڴ�
extern "C" _declspec(dllimport) bool TSPsqlOpen();     //�������ݿ�
extern "C" _declspec(dllimport) bool TSPsqlGetConState();   //��ȡ����״̬
extern "C" _declspec(dllimport) bool TSPsqlReConnect();  //�������ݿ⺯��
extern "C" _declspec(dllimport) void TSPsqlDestoryLogobj(); //�ر����ݿ�����






