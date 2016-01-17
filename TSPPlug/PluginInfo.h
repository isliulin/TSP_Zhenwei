/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ��������࣬ÿ��ʵ������һ�����
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGININFO_H__
#define __TSP_PLUGININFO_H__

#include "ddc_util.h"
#include "TspPluginIni.hpp"
#include "macros.h"

/// �涨�Ĳ���к������� start
#define DLL_START_SYM "start"

/// �涨�Ĳ���к������� stop
#define DLL_STOP_SYM "stop"



/**
* @class CPluginInfo
* 
* @brief ���������Ϣ
*/
class CPluginInfo
{
public:
	friend class CPluginManager;

	//���״̬
	enum { STATUS_INIT,					// ���ʼ����dll���Ƶȶ���û��
		STATUS_DELETE,					// �Ѿ�ɾ��
		STATUS_WAITDOWNLOAD,			// δ����
		STATUS_LOAD,					// �Ѿ�����,dll_handle��Ϊ�գ�����ָ�벻Ϊ��
		STATUS_UNLOAD,					// �Ѿ�ж��,���߻�δ���أ�dll_handleΪ��
		STATUS_RUNNING,					// ������
		STATUS_STOP,					// ֹͣ��״̬��STATUS_LOAD��ͬ�����ⲿ��Ҫȷ�ϵ��õ����ĸ��������ڲ�����������״̬
		STATUS_ERROR,					// ����
		STATUS_DOWNLOAD,				// ���سɹ�		
	};	

	/// constrator
	CPluginInfo();

	/// destrator
	~CPluginInfo();

	/**
	* ʵ�ּ��ز������
	* @param ��־λ
	* @return �ɹ�����0
	*/	
	int LoadPlug( DWORD dwFlag = 0 );

	/**
	* ʵ��ж�ز������
	* @param ��־λ
	* @return �ɹ�����0
	*/	
	int UnLoadPlug( DWORD dwFlag = 0 );

	/**
	* ʵ�������������	
	* @param param �����б�
	* @return �ɹ�����0
	*/	
	int StartPlug( PLUGPARAM *param = NULL );

	/**
	* ֹͣ��������������߳�	
	* @param ��־λ
	* @return �ɹ�����0
	*/
	int StopPlug( DWORD dwFlag = 0 );  	

	/**
	* ж�ز��	
	* @param ��־λ
	* @return �ɹ�����0
	*/
	int RemovePlug( DWORD dwFlag=0); 

	/**
	* ���ص������
	* @param strPluginDownLoadPath ���ر����Ŀ¼
	* @return �ɹ�����0
	*/	
	int DownloadPlug( std::string strPluginDownLoadPath );

	/**
	* ���ڱȽϲ������Ƚϲ������
	*/
	bool operator==(const std::string &other){return m_strName==other;};

	/**
	* ��ֵ������
	*/
	CPluginInfo& operator=(const CPluginInfo &other);

	/**
	* �����ֶ���գ����ֵ
	*/
	void reset(void);

	/**
	* ����״̬
	*/
	void Status(int status) {m_status = status;};

	/**
	* ȡ״̬
	*/
	int GetStatus(void) const{return m_status;};

protected:

	//start��stop����ǩ��
	typedef int (*FN_START)(int,void *); 
	typedef void (*FN_STOP)(void);
	
public:		// ��Ҫ��ȫ�ֺ���ReadPluginIniFile<>���ʣ�ģ�岻֪����ô����Ϊ��Ԫ 
	FN_START m_pfnStart;				/// ����ָ�룬����
	FN_STOP m_pfnStop;					/// ����ָ�룬ֹͣ

	string m_strName;					/// ������ƣ�����·����Ϣ
	string m_strFullName;				/// �������,��·����Ϣ
	TCHAR m_md5[TSP_PLUGIN_SHORT_PATH];	/// md5
	TCHAR m_ver[TSP_PLUGIN_SHORT_PATH];	/// ver
	string m_strURL;					/// url,�����ڴ�ԭ��ʹ��string��
	DWORD m_dwSize;						/// ��������б���Ҫ
	DWORD m_dwFlag;						/// ��־
	DWORD m_status;						/// ״̬
	DWORD m_pid;						/// ���ݿ��־λ
	string m_strRoot;					/// ϵͳĿ¼��
	string m_strDir;					/// ��Ŀ¼
	string m_strRealName;				/// ʵ���ļ���
	DWORD m_dwReserved;						/// ��������
	char m_szReserved[TSP_PLUGIN_SHORT_PATH];	/// ��������
	HMODULE m_hnd;		/// dll handle,û�ã�handle�Ѿ��� dllmanager�������
};



// �����������
#define PLUGIN_CAP_QQ 1
#define PLUGIN_CAP_WEB 2
#define PLUGIN_CAP_FOX 4


#endif	//__TSP_PLUGININFO_H__
