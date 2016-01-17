/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件管理类，每个实例代表一个插件
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGININFO_H__
#define __TSP_PLUGININFO_H__

#include "ddc_util.h"
#include "TspPluginIni.hpp"
#include "macros.h"

/// 规定的插件中函数名称 start
#define DLL_START_SYM "start"

/// 规定的插件中函数名称 stop
#define DLL_STOP_SYM "stop"



/**
* @class CPluginInfo
* 
* @brief 单个插件信息
*/
class CPluginInfo
{
public:
	friend class CPluginManager;

	//插件状态
	enum { STATUS_INIT,					// 类初始化，dll名称等都还没有
		STATUS_DELETE,					// 已经删除
		STATUS_WAITDOWNLOAD,			// 未下载
		STATUS_LOAD,					// 已经加载,dll_handle不为空，函数指针不为空
		STATUS_UNLOAD,					// 已经卸载,或者还未加载，dll_handle为空
		STATUS_RUNNING,					// 运行中
		STATUS_STOP,					// 停止，状态与STATUS_LOAD相同，但外部需要确认调用的是哪个函数，内部不会出现这个状态
		STATUS_ERROR,					// 出错
		STATUS_DOWNLOAD,				// 下载成功		
	};	

	/// constrator
	CPluginInfo();

	/// destrator
	~CPluginInfo();

	/**
	* 实现加载插件功能
	* @param 标志位
	* @return 成功返回0
	*/	
	int LoadPlug( DWORD dwFlag = 0 );

	/**
	* 实现卸载插件功能
	* @param 标志位
	* @return 成功返回0
	*/	
	int UnLoadPlug( DWORD dwFlag = 0 );

	/**
	* 实现启动插件功能	
	* @param param 参数列表
	* @return 成功返回0
	*/	
	int StartPlug( PLUGPARAM *param = NULL );

	/**
	* 停止单个插件的运行线程	
	* @param 标志位
	* @return 成功返回0
	*/
	int StopPlug( DWORD dwFlag = 0 );  	

	/**
	* 卸载插件	
	* @param 标志位
	* @return 成功返回0
	*/
	int RemovePlug( DWORD dwFlag=0); 

	/**
	* 下载单个插件
	* @param strPluginDownLoadPath 下载保存的目录
	* @return 成功返回0
	*/	
	int DownloadPlug( std::string strPluginDownLoadPath );

	/**
	* 用于比较操作，比较插件名称
	*/
	bool operator==(const std::string &other){return m_strName==other;};

	/**
	* 赋值操作符
	*/
	CPluginInfo& operator=(const CPluginInfo &other);

	/**
	* 所有字段清空，设初值
	*/
	void reset(void);

	/**
	* 设置状态
	*/
	void Status(int status) {m_status = status;};

	/**
	* 取状态
	*/
	int GetStatus(void) const{return m_status;};

protected:

	//start和stop函数签名
	typedef int (*FN_START)(int,void *); 
	typedef void (*FN_STOP)(void);
	
public:		// 需要给全局函数ReadPluginIniFile<>访问，模板不知道怎么声明为友元 
	FN_START m_pfnStart;				/// 函数指针，启动
	FN_STOP m_pfnStop;					/// 函数指针，停止

	string m_strName;					/// 插件名称，不带路经信息
	string m_strFullName;				/// 插件名称,带路经信息
	TCHAR m_md5[TSP_PLUGIN_SHORT_PATH];	/// md5
	TCHAR m_ver[TSP_PLUGIN_SHORT_PATH];	/// ver
	string m_strURL;					/// url,由于内存原因，使用string类
	DWORD m_dwSize;						/// 插件下载列表需要
	DWORD m_dwFlag;						/// 标志
	DWORD m_status;						/// 状态
	DWORD m_pid;						/// 数据库标志位
	string m_strRoot;					/// 系统目录名
	string m_strDir;					/// 子目录
	string m_strRealName;				/// 实际文件名
	DWORD m_dwReserved;						/// 附加数据
	char m_szReserved[TSP_PLUGIN_SHORT_PATH];	/// 附加数据
	HMODULE m_hnd;		/// dll handle,没用，handle已经被 dllmanager类管理了
};



// 截屏插件参数
#define PLUGIN_CAP_QQ 1
#define PLUGIN_CAP_WEB 2
#define PLUGIN_CAP_FOX 4


#endif	//__TSP_PLUGININFO_H__
