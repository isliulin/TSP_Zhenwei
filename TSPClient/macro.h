//#include <winsock2.h>
#ifndef __MACRO_DEFINE__
#define __MACRO_DEFINE__
/************************************************************************************/
//������ƶ���
/************************************************************************************/
#define TCP_PROTOCOL


/************************************************************************************/
//��������
/************************************************************************************/
#define MAX_UPDATE_TIME	3*24*60*60//3*24*60*60��
#define MIN_STRING			""
#define MAX_SOCKET_IO_DATA_LEN		8192
#define KTD_QUERY 1006

/************************************************************************************/
//�������Ͷ���
/************************************************************************************/
//#define EXPORT	__declspec( dllexport )

/************************************************************************************/
//�꺯������
/************************************************************************************/
bool mystrcpy(char *strDestination,char *strSource,int MaxLen); 
bool mystrcat(char *strDestination,char *strSource,int MaxLen);   

/////////////////////////////////////////////////////////////
//S_FileInfo�ṹwParam����
#define FILE_FLAG_BLACK		1//��λ=1,��ʾ���ļ��ں�������
#define FILE_FALG_READ		2//��λ=1,��ʾ���ļ����ж��������ԣ��ļ�����ģ��ʹ��
#define FILE_FALG_WRITE		4//��λ=1,��ʾ���ļ�����д�������ԣ��ļ�����ģ��ʹ��
#define FILE_FALG_RENAME	8//��λ=1,��ʾ���ļ����з����������ԣ��ļ�����ģ��ʹ��
#define FILE_FALG_DELETE	16//��λ=1,��ʾ���ļ����з�ɾ�����ԣ��ļ�����ģ��ʹ��
#define FILE_FALG_COPY		32//��λ=1,��ʾ���ļ����з��������ԣ��ļ�����ģ��ʹ��
#define FILE_FALG_MOVE		64//��λ=1,��ʾ���ļ����з��ƶ����ԣ��ļ�����ģ��ʹ��

//�ļ���Ϣ����
#define FILE_INFO_FIND_DATA		0x01
#define FILE_INFO_MD5			0x02
#define FILE_INFO_CERT_INFO		0x04
#define FILE_INFO_SHELL_INFO	0x08
//��־����
#define LOG_ERROR		0x01
#define LOG_WARNING		0x02
#define LOG_LOG			0x04
//ģ��
#define LOG_MODULE_SERVER	0x01
#define LOG_MODULE_SEARCH	0x02
#define LOG_MODULE_TASK		0x04












#endif