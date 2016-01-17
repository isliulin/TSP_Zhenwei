/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 文件系统操作，需要boost库
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#if !defined(__VIRUS_FILE_SYSTEM_H__)
#define __VIRUS_FILE_SYSTEM_H__

#include "ddc_util.h"

#define SAFE_PATH_RESERVED 16				//保留一定的空间用于特殊路径调整 "\\?\"
#define MAX_PATH_SAFE MAX_PATH*2+SAFE_PATH_RESERVED

namespace nm_ddc
{
	//新版遍历目录
	//参数:目录名，是否递归，是否遍历链接，深度优先，文件回调，目录回调，自定义数据
	typedef enum enumDirAction{
		DIRACTION_STOP,					//0 停止
		DIRACTION_CONTINUE,				//1 继续
		DIRACTION_SKIP,					//2 退出当层循环
	};

	/**
	* 目录遍历.
	* @brief 
	* 目录返回DIRACTION_SKIP,跳过此目录；文件返回DIRACTION_SKIP 无意义
	* fileName 不带"\\",带了也能正常使用，拼装目录成"\\\\",在目录排除等用法上要注意返回值无意义
	* @param fileNmae 遍历的目录名
	* @param recurse 是否递归
	* @param followLinks 是否访问链接目录
	* @param 是否深度优先
	* @param 文件回调函数
	* @param 目录回调函数
	* @param 自定义数据，回调函数中返回
	* @return DIRACTION_CONTINUE 继续遍历
	* @return DIRACTION_STOP 停止遍历
	*/
	int recursive_dir_action(const char *fileName,
						 int recurse, int followLinks, int depthFirst,
						 int (*fileAction) (const char *fileName,struct stat * statbuf,void* userData),
						 int (*dirAction) (const char *fileName,struct stat * statbuf,void* userData),
						 void* userData);

	//宽版
	int recursive_dir_action_w(const wchar_t *fileName,
						   int recurse, int followLinks, int depthFirst,
						   int (*fileAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   int (*dirAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   void* userData);


	//文件操作函数，带强制确认子串	
	int VirDeleteFile(const char *fileName,const char *confirmStr=NULL);
	int VirRemoveDirectory(const char *dir,const char *confirmStr);
	int VirClearDirectory(const char *dir,const char *confirmStr);

	//如果不递归，会拷贝第一层所有文件，并建立空目录
	int VirCopyDirectory(const char *s,const char *d,int recurse);
	int VirCreateDirectory(const char *dir);

	/// 文件大小
	uint file_size(const char *);

	/// 文件大小
	uint file_size(const wchar_t *);

	/// 是否目录
	bool is_directory(const char *);

	/// 是否目录
	bool is_directory(const wchar_t *);

	/// 是否空文件
	bool is_empty(const char *);

	/// 是否空文件
	bool is_empty(const wchar_t *);

	/// 文件或目录是否存在
	bool exists(const char *) ;

	/// 文件或目录是否存在
	bool exists(const wchar_t *) ;

	/// 创建目录
	bool create_directory(const wchar_t *);
	
}	// end nm_ddc


#endif	//__VIRUS_FILE_SYSTEM_H__
