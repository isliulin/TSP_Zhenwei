/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 全局数据类，目前仅插件部分用
* @author 刘成伟 chengwei@run.com
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
* @brief 全局数据结构
*/
struct CGlobalData
{	
	/// 初始化
	void Init();

	/**
	* 重新加载插件配置信息
	* @brief 当plugin_setting被修改后触发器会在client_notify表中写入记录，轮询线程判断到有更新，将调用此操作，更新插件配置全局数据
	* @return 0 成功
	*/	
	int LoadPlugSetting();
	
	/**
	* 在setting缓存中取出pid的数据
	* @param pid pid
	* @return 是否启用
	*/
	uint GetPlugSetting(uint pid);

	CDbPool dbPool;					/// 数据库连接池
	int nPluginPid;					/// 数据库配置中木马程序的编号
	char szModulePath[MAX_PATH*2];	/// 安装目录，带'\\'
	char szConfigPath[MAX_PATH*2];		/// config文件目录，带'\\'

	std::vector<std::pair<uint,uint> > plugSetting;	/// 插件配置信息,pair中是pid和isused
private:	
	nm_ddc::CThreadMutex m_lock;		/// 同步锁

};

/// 全局对象，singleton模式
typedef SingletonHolder<CGlobalData> Global_Data;
#define GLOBAL_DATA Global_Data::instance()


/**
* 轮询数据库表查看设置是否修改 (先放这里)
* 如果PLUGIN_SETTING表被触发，重新加载全局plugSetting数据
* @param vec [OUT] PLUG_INFO表数据库变化的记录
* @return 0 成功
*/
int OnDbNotify(std::vector< std::pair<int,int> > &vec);

/**
* 首次上线，数据库中插入记录,liucw 2013-08-06
* @param dwMachineID 机器ID
* @return 0 成功
*/
int TSP2_PluginOnLine(DWORD dwMachineID);

#endif	//__TSP_PLUGINGLOBAL_H__