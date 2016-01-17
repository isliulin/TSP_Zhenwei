//#include <winsock2.h>
#ifndef __MACRO_DEFINE__
#define __MACRO_DEFINE__
/************************************************************************************/
//编译控制定义
/************************************************************************************/
#define TCP_PROTOCOL


/************************************************************************************/
//常量定义
/************************************************************************************/
#define MAX_UPDATE_TIME	3*24*60*60//3*24*60*60秒
#define MIN_STRING			""
#define MAX_SOCKET_IO_DATA_LEN		8192
#define KTD_QUERY 1006

/************************************************************************************/
//数据类型定义
/************************************************************************************/
//#define EXPORT	__declspec( dllexport )

/************************************************************************************/
//宏函数定义
/************************************************************************************/
bool mystrcpy(char *strDestination,char *strSource,int MaxLen); 
bool mystrcat(char *strDestination,char *strSource,int MaxLen);   

/////////////////////////////////////////////////////////////
//S_FileInfo结构wParam定义
#define FILE_FLAG_BLACK		1//此位=1,表示此文件在黑名单中
#define FILE_FALG_READ		2//此位=1,表示此文件具有读保护属性，文件保护模块使用
#define FILE_FALG_WRITE		4//此位=1,表示此文件具有写保护属性，文件保护模块使用
#define FILE_FALG_RENAME	8//此位=1,表示此文件具有防重命名属性，文件保护模块使用
#define FILE_FALG_DELETE	16//此位=1,表示此文件具有防删除属性，文件保护模块使用
#define FILE_FALG_COPY		32//此位=1,表示此文件具有防拷贝属性，文件保护模块使用
#define FILE_FALG_MOVE		64//此位=1,表示此文件具有防移动属性，文件保护模块使用

//文件信息掩码
#define FILE_INFO_FIND_DATA		0x01
#define FILE_INFO_MD5			0x02
#define FILE_INFO_CERT_INFO		0x04
#define FILE_INFO_SHELL_INFO	0x08
//日志定义
#define LOG_ERROR		0x01
#define LOG_WARNING		0x02
#define LOG_LOG			0x04
//模块
#define LOG_MODULE_SERVER	0x01
#define LOG_MODULE_SEARCH	0x02
#define LOG_MODULE_TASK		0x04












#endif