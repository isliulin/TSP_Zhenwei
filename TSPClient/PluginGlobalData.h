/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ȫ�������࣬Ŀǰ�����������
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGINGLOBAL_H__
#define __TSP_PLUGINGLOBAL_H__

#include "ddc_util.h"
#include "ddc_thread.h"
#include "PluginDatabase.h"


/**
* @class CGlobalData
*
* @brief ȫ�����ݽṹ
*/
struct CGlobalData
{	
	/// ��ʼ��
	void Init();

	/**
	* ���¼��ز��������Ϣ
	* @brief ��plugin_setting���޸ĺ󴥷�������client_notify����д���¼����ѯ�߳��жϵ��и��£������ô˲��������²������ȫ������
	* @return 0 �ɹ�
	*/	
	int LoadPlugSetting();
	
	/**
	* ��setting������ȡ��pid������
	* @param pid pid
	* @return �Ƿ�����
	*/
	uint GetPlugSetting(uint pid);

	CDbPool dbPool;					/// ���ݿ����ӳ�
	int nPluginPid;					/// ���ݿ�������ľ�����ı��
	char szModulePath[MAX_PATH*2];	/// ��װĿ¼����'\\'
	char szConfigPath[MAX_PATH*2];		/// config�ļ�Ŀ¼����'\\'

	std::vector<std::pair<uint,uint> > plugSetting;	/// ���������Ϣ,pair����pid��isused
private:	
	nm_ddc::CThreadMutex m_lock;		/// ͬ����

};

/// ȫ�ֶ���singletonģʽ
typedef SingletonHolder<CGlobalData> Global_Data;
#define GLOBAL_DATA Global_Data::instance()


/**
* ��ѯ���ݿ��鿴�����Ƿ��޸� (�ȷ�����)
* ���PLUGIN_SETTING�����������¼���ȫ��plugSetting����
* @param vec [OUT] PLUG_INFO�����ݿ�仯�ļ�¼
* @return 0 �ɹ�
*/
int OnDbNotify(std::vector< std::pair<int,int> > &vec);

/**
* �״����ߣ����ݿ��в����¼,liucw 2013-08-06
* @param dwMachineID ����ID
* @return 0 �ɹ�
*/
int TSP2_PluginOnLine(DWORD dwMachineID);

#endif	//__TSP_PLUGINGLOBAL_H__