/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ���ƶ˲��������
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGINCONFIG_H__
#define __TSP_PLUGINCONFIG_H__

#include "ddc_util.h"
#include "PluginDatabase.h"
#include "TspPluginIni.hpp"

//ǰ���������������IOCPͷ�ļ�
struct ClientContext;
class CIOCPServer;


#define TSP_PLUGIN_PATH 520
#define TSP_PLUGIN_C_CFG_PATH "MachinePluginConfig\\"				// ����������ڵ�Ŀ¼(���滹�л����ţ�������ƴ��)
#define TSP_PLUGIN_C_CFG_NAME "config.ini"						// ��������ļ���
#define TSP_PLUGIN_M_CFG_NAME "PluginUpdate.ini"						// ��֤���������صĲ���������ļ���



///�������������Ϣ
struct PluginData
{
	void reset(){};
	string m_strName;					/// ������ƣ�����·����Ϣ
	string m_strFullName;				/// �������,��·����Ϣ
	TCHAR m_md5[TSP_PLUGIN_SHORT_PATH];	/// md5
	TCHAR m_ver[TSP_PLUGIN_SHORT_PATH];	/// ver
	string m_strURL;					/// url,�����ڴ�ԭ��ʹ��string��
	DWORD m_dwSize;						/// ��������б���Ҫ
	DWORD m_dwFlag;						/// ��־
	DWORD m_pid;						/// ���ݿ��־λ
	string m_strRoot;					/// ϵͳĿ¼��
	string m_strDir;					/// ��Ŀ¼
	string m_strRealName;				/// ʵ���ļ���	
	std::string file;					/// �ļ���
	std::string url;					/// url
	TCHAR ver[50];						/// �汾��
	TCHAR md5[50];						/// md5
	DWORD m_dwReserved;						/// ��������
	char m_szReserved[TSP_PLUGIN_SHORT_PATH];	/// ��������
};


/**
* @class CPluginConfig
*
* @brief �����ʵ�ֵ������ػ���������ļ������ɺ��·�
*/
class CPluginConfig
{
	//������
	friend class testPluginConfig;
public:

	/// ������ĳ�ʼ�����Լ��������ļ��л��ԭʼ����б��ļ���λ�ã�����ֵ��������tchSuperConfigFile
	CPluginConfig(ClientContext *pContext,CIOCPServer *iocpServer);

	/// ��������ͷ�
	~CPluginConfig();

	/**
	* ���ݹ����Ĳ���б�����ݿ��жԲ�������ã����û�ͣ�ã���������Ե������ػ��Ĳ�������ļ���
	* ���ɵ��ļ�Ĭ�ϱ��浽�����ͬһĿ¼�µ�MachinePluginConfig�£�
	* ����ļ����·ֻ���ID���浥�������Ĳ�������ļ�
	* @param _ulMachineID ���룬���ػ�����ID
	* @param tchMachineConfigFile	������ļ��������ػ��Ĳ�������ļ�·��+�ļ�����
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL CreateConfigFile(unsigned long _ulMachineID,TCHAR _tchMachineConfigFile[TSP_PLUGIN_PATH]);

	/**
	* brief �򱻿ض˷�����CreateConfigFile���������������ļ�
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL SendFile(const char *_tchMachineConfigFile);

private:
	/**
	* ���ݹ����Ĳ���б�����ݿ��жԲ�������ã����û�ͣ�ã���������Ե������ػ��Ĳ�������ļ���
	* ���ɵ��ļ�Ĭ�ϱ��浽�����ͬһĿ¼�µ�MachinePluginConfig�£�
	* ����ļ����·ֻ���ID���浥�������Ĳ�������ļ�
	* @param _ulMachineID ���룬���ػ�����ID
	* @param l �����Ϣ����
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL CreateConfigFileImp(unsigned long _ulMachineID,std::list<PluginData> &l);

private:	
	TCHAR *tchSuperConfigFile;				/// ԭʼ�Ĳ���б��ļ��Ĵ��λ�ã�·��+�ļ����ƣ�
	ClientContext *m_pContext;				/// ���������ģ�iocpServer��Ҫ
	CIOCPServer *m_iocpServer;				/// ��ɶ˿�ָ��
};



/**
* @class CPluginRemover
*
* @brief �����ʵ�ֲ��ж�صļ�����Ϣ����
*/
class CPluginRemover
{
	//������
	friend class testPluginConfig;
public:

	/// ������ĳ�ʼ�����Լ��������ļ��л��ԭʼ����б��ļ���λ�ã�����ֵ��������tchSuperConfigFile
	CPluginRemover(ClientContext *pContext,CIOCPServer *iocpServer)
		:m_pContext(pContext),m_iocpServer(iocpServer){};

	/** �жϲ���ж������
	* @param _ulMid ����ID
	* @return TRUE �ɹ�
	*/
	bool operator()(DWORD dwMachineID);
	


	/// ����ж������
	int SendRemoveCommand();
private:
	
	/**
	* ж��ʱ������ƶ�����
	* @param dwMachineID ����ID
	* @return 0 �ɹ�
	*/
	int ClearClientData(DWORD dwMachineID);

			
	/**
	* ɾ����ʱ����
	* @author ������
	* @param _ulMid ����ID
	* @return TRUE �ɹ�
	*/
	BOOL RemoveDownloadTmpDir(unsigned long _ulMid);


private:
	ClientContext *m_pContext;				/// ���������ģ�iocpServer��Ҫ
	CIOCPServer *m_iocpServer;				/// ��ɶ˿�ָ��
};

#endif	//	__TSP_PLUGINCONFIG_H__