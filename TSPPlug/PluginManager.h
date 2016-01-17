/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ���������
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGINMANAGER_H__
#define __TSP_PLUGINMANAGER_H__

#include <list>

#include "ddc_util.h"
#include "ddc_thread.h"
#include "ddc_singleton.hpp"
#include "PluginInfo.h"
#include "TspPluginIni.hpp"
#include "TspRecycle.h"


#define TSP_PLUGIN_S_CONFIG_PATH ""						//����ļ�����Ŀ¼
#define TSP_PLUGIN_S_CONFIG_NAME "cfgm.ini"				//��������ļ���
#define TSP_PLUGIN_S_DOWNLOAD_PATH "dd_02gcv\\"			//��������ļ�����Ŀ¼,�»��ߺ��������д��������ʱ�ļ����г�ͻ
#define TSP_PLUGIN_S_CONFIG_DLNAME "download_cfg.ini"	//�������ʱ������ļ�����

/**
* @class CPluginManager
*
* @brief ���������ʵ��socket�ͻ��ˣ��ṩ��socket����˽������Ӻ��ͷ�����
*/
class CPluginManager
{	
	friend class pluginManager;		// cppunit test class
public:
	/// constrator
	CPluginManager();

	/// destrator
	virtual ~CPluginManager();

	/**
	* ��ʼ��
	* @param szModulePath ·��
	* @return 0 �ɹ�
	*/
	int Init(const char *szModulePath = NULL);

	/**
	* �ı���״̬
	* @param pid �������
	* @param flag Ҫ�ı��״̬
	* @return 0 �ɹ�
	*/
	int Status(int pid,int flag);

	/**
	* ��ò��״̬
	* @param pid �������
	* @return ���״̬,ȡֵCPluginInfo::ö����
	*/
	int Status(int pid);

	/**
	* �ı���״̬	
	* @param flag Ҫ�ı��״̬
	* @return 0 �ɹ�
	*/
	int StatusAll(int flag);

	/**
	* ������������ļ������������ļ�ֹͣ��������������Ĺ���
	* @param strPluginDownLoadFile ���صĲ������·��
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL Update(const string &strPluginDownLoadFile); 

	std::list<CPluginInfo> m_PluginList;			/// ����б�
	PLUGPARAM m_plugParam;							/// ����䴫�ݵĲ���
private:	
	
	/**
	* ���ز��
	* @param dwFlag ��־λ
	* @return 0 �ɹ�
	*/
	int ReloadPlugin(DWORD dwFlag = 0);	

	/**
	* �ų�����
	* ȥ��list�в������ⲿ�ֹ���Ĳ��������:ľ�����Լ��������̲��
	* @param l ����
	* @return 0 �ɹ�
	*/
	int ExceptPluginFile(std::list<CPluginInfo> &l);
private:

	nm_ddc::CThreadMutex m_lock;			/// ͬ����	
	CTspRecycle m_recycle;							/// ����վ�࣬����ɾ����ʱ�ļ�	

	std::string m_szPluginDownLoadPath;				/// �������Ŀ¼
	std::string m_szPluginConfigPath;				/// �������Ŀ¼	
	TCHAR m_szModulePath[TSP_PLUGIN_PATH];			/// ��װ·�������÷����Ӧ����ȫ�ִ�����	

};

/// ȫ�ֶ���singletonģʽ
typedef SingletonHolder<CPluginManager> Global_PlugManager;
#define GLOBAL_PLUGIN Global_PlugManager::instance()


#endif	//__TSP_PLUGINMANAGER_H__
