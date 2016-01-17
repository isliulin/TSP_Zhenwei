/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 控制端插件配置类
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef __TSP_PLUGINCONFIG_H__
#define __TSP_PLUGINCONFIG_H__

#include "ddc_util.h"
#include "PluginDatabase.h"
#include "TspPluginIni.hpp"

//前置声明，避免包含IOCP头文件
struct ClientContext;
class CIOCPServer;


#define TSP_PLUGIN_PATH 520
#define TSP_PLUGIN_C_CFG_PATH "MachinePluginConfig\\"				// 插件配置所在的目录(后面还有机器号，程序中拼接)
#define TSP_PLUGIN_C_CFG_NAME "config.ini"						// 插件配置文件名
#define TSP_PLUGIN_M_CFG_NAME "PluginUpdate.ini"						// 认证服务器下载的插件总配置文件名



///单个插件配置信息
struct PluginData
{
	void reset(){};
	string m_strName;					/// 插件名称，不带路经信息
	string m_strFullName;				/// 插件名称,带路经信息
	TCHAR m_md5[TSP_PLUGIN_SHORT_PATH];	/// md5
	TCHAR m_ver[TSP_PLUGIN_SHORT_PATH];	/// ver
	string m_strURL;					/// url,由于内存原因，使用string类
	DWORD m_dwSize;						/// 插件下载列表需要
	DWORD m_dwFlag;						/// 标志
	DWORD m_pid;						/// 数据库标志位
	string m_strRoot;					/// 系统目录名
	string m_strDir;					/// 子目录
	string m_strRealName;				/// 实际文件名	
	std::string file;					/// 文件名
	std::string url;					/// url
	TCHAR ver[50];						/// 版本号
	TCHAR md5[50];						/// md5
	DWORD m_dwReserved;						/// 附加数据
	char m_szReserved[TSP_PLUGIN_SHORT_PATH];	/// 附加数据
};


/**
* @class CPluginConfig
*
* @brief 这个类实现单个被控机插件配置文件的生成和下发
*/
class CPluginConfig
{
	//测试类
	friend class testPluginConfig;
public:

	/// 类变量的初始化，以及从配置文件中获得原始插件列表文件的位置，并赋值给类属性tchSuperConfigFile
	CPluginConfig(ClientContext *pContext,CIOCPServer *iocpServer);

	/// 类变量的释放
	~CPluginConfig();

	/**
	* 根据公共的插件列表和数据库中对插件的配置（启用或停用），生成针对单个被控机的插件配置文件，
	* 生成的文件默认保存到与程序同一目录下的MachinePluginConfig下，
	* 这个文件夹下分机器ID保存单个机器的插件配置文件
	* @param _ulMachineID 输入，被控机机器ID
	* @param tchMachineConfigFile	输出，文件单个被控机的插件配置文件路径+文件名称
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL CreateConfigFile(unsigned long _ulMachineID,TCHAR _tchMachineConfigFile[TSP_PLUGIN_PATH]);

	/**
	* brief 向被控端发送由CreateConfigFile函数产生的配置文件
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL SendFile(const char *_tchMachineConfigFile);

private:
	/**
	* 根据公共的插件列表和数据库中对插件的配置（启用或停用），生成针对单个被控机的插件配置文件，
	* 生成的文件默认保存到与程序同一目录下的MachinePluginConfig下，
	* 这个文件夹下分机器ID保存单个机器的插件配置文件
	* @param _ulMachineID 输入，被控机机器ID
	* @param l 插件信息链表
	* @return 成功返回TRUE，否则返回FALSE
	*/
	BOOL CreateConfigFileImp(unsigned long _ulMachineID,std::list<PluginData> &l);

private:	
	TCHAR *tchSuperConfigFile;				/// 原始的插件列表文件的存放位置（路径+文件名称）
	ClientContext *m_pContext;				/// 发送上下文，iocpServer需要
	CIOCPServer *m_iocpServer;				/// 完成端口指针
};



/**
* @class CPluginRemover
*
* @brief 这个类实现插件卸载的检测和消息发送
*/
class CPluginRemover
{
	//测试类
	friend class testPluginConfig;
public:

	/// 类变量的初始化，以及从配置文件中获得原始插件列表文件的位置，并赋值给类属性tchSuperConfigFile
	CPluginRemover(ClientContext *pContext,CIOCPServer *iocpServer)
		:m_pContext(pContext),m_iocpServer(iocpServer){};

	/** 判断并发卸载命令
	* @param _ulMid 机器ID
	* @return TRUE 成功
	*/
	bool operator()(DWORD dwMachineID);
	


	/// 发送卸载命令
	int SendRemoveCommand();
private:
	
	/**
	* 卸载时清除控制端数据
	* @param dwMachineID 机器ID
	* @return 0 成功
	*/
	int ClearClientData(DWORD dwMachineID);

			
	/**
	* 删除临时数据
	* @author 孟雷子
	* @param _ulMid 机器ID
	* @return TRUE 成功
	*/
	BOOL RemoveDownloadTmpDir(unsigned long _ulMid);


private:
	ClientContext *m_pContext;				/// 发送上下文，iocpServer需要
	CIOCPServer *m_iocpServer;				/// 完成端口指针
};

#endif	//	__TSP_PLUGINCONFIG_H__