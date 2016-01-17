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

#include <list>

#include "ddc_util.h"
#include "ddc_thread.h"
#include "ddc_singleton.hpp"
#include "PluginInfo.h"
#include "TspPluginIni.hpp"
#include "TspRecycle.h"


#define TSP_PLUGIN_S_CONFIG_PATH ""						//插件文件所在目录
#define TSP_PLUGIN_S_CONFIG_NAME "cfgm.ini"				//插件配置文件名
#define TSP_PLUGIN_S_DOWNLOAD_PATH "dd_02gcv\\"			//插件下载文件所在目录,下划线后面是随机写，怕在临时文件夹中冲突
#define TSP_PLUGIN_S_CONFIG_DLNAME "download_cfg.ini"	//插件下载时保存的文件名称

/**
* @class CPluginManager
*
* @brief 这个类用来实现socket客户端，提供和socket服务端建立连接和释放连接
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
	* 初始化
	* @param szModulePath 路径
	* @return 0 成功
	*/
	int Init(const char *szModulePath = NULL);

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
	* 改变插件状态	
	* @param flag 要改变的状态
	* @return 0 成功
	*/
	int StatusAll(int flag);

	/**
	* 解析插件配置文件，根据配置文件停止和启动单个插件的功能
	* @param strPluginDownLoadFile 下载的插件配置路径
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL Update(const string &strPluginDownLoadFile); 

	std::list<CPluginInfo> m_PluginList;			/// 插件列表
	PLUGPARAM m_plugParam;							/// 插件间传递的参数
private:	
	
	/**
	* 加载插件
	* @param dwFlag 标志位
	* @return 0 成功
	*/
	int ReloadPlugin(DWORD dwFlag = 0);	

	/**
	* 排除函数
	* 去除list中不属于这部分管理的插件，包括:木马本身，自己本身，键盘插件
	* @param l 链表
	* @return 0 成功
	*/
	int ExceptPluginFile(std::list<CPluginInfo> &l);
private:

	nm_ddc::CThreadMutex m_lock;			/// 同步锁	
	CTspRecycle m_recycle;							/// 回收站类，负责删除临时文件	

	std::string m_szPluginDownLoadPath;				/// 插件下载目录
	std::string m_szPluginConfigPath;				/// 插件所在目录	
	TCHAR m_szModulePath[TSP_PLUGIN_PATH];			/// 安装路径，不该放这里，应该是全局传进来	

};

/// 全局对象，singleton模式
typedef SingletonHolder<CPluginManager> Global_PlugManager;
#define GLOBAL_PLUGIN Global_PlugManager::instance()


#endif	//__TSP_PLUGINMANAGER_H__
