/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief 回收站
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include <algorithm>
#include "TspRecycle.h"
#include "Log.h"

int CTspRecycle::Push(const string &strName,DWORD dwFlag)
{	
	// 查找文件名
	if( find(m_list.begin(),m_list.end(),strName) == m_list.end() )
		//进容器
		m_list.push_back( PrivateData(strName,dwFlag) );
	else
	{
		LOG((LEVEL_WARNNING,"重复进入回收站:%s \n",strName.c_str()));
		return -1;
	}

	return 0;
}

int CTspRecycle::Pop(const string &strName,DWORD dwFlag)
{
	BOOL bRet = TRUE;

	// 查找文件名
	list<PrivateData>::iterator iter = find(m_list.begin(),m_list.end(),strName);
	 if( iter != m_list.end() )
	 {
		 // 要删除
		if( dwFlag == FLAG_DEL )
		{
			

			if( iter->flag == FLAG_FILE )
			{
				bRet = DeleteFile(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"删除文件失败:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
					MoveFileEx(iter->name.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}	
			else
			{
				bRet = RemoveDirectory(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"删除文件失败:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
				}
			}
		}

		// 列表中删除
		m_list.erase(iter);
	 }
	 else
	 {
		 LOG((LEVEL_WARNNING,"没找到要删除的文件:%s \n",strName.c_str()));
		 bRet = FALSE;
	 }

	 return bRet?0:-1;
}

int CTspRecycle::PopAll(DWORD dwFlag)
{
	BOOL bRet;

	// 如果需要删除才遍历,反向，避免目录删不掉
	if( dwFlag == FLAG_DEL )
		for( list<PrivateData>::reverse_iterator iter=m_list.rbegin();iter!=m_list.rend();++iter )
		{
			// 要删除
			if( iter->flag == FLAG_FILE )
			{
				bRet = DeleteFile(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"删除文件失败:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
					MoveFileEx(iter->name.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
			else
			{
				bRet = RemoveDirectory(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"删除文件失败:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
				}
			}
		}


	//清空
	m_list.clear();

	return 0;
}
