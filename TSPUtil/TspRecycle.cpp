/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief ����վ
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#include <algorithm>
#include "TspRecycle.h"
#include "Log.h"

int CTspRecycle::Push(const string &strName,DWORD dwFlag)
{	
	// �����ļ���
	if( find(m_list.begin(),m_list.end(),strName) == m_list.end() )
		//������
		m_list.push_back( PrivateData(strName,dwFlag) );
	else
	{
		LOG((LEVEL_WARNNING,"�ظ��������վ:%s \n",strName.c_str()));
		return -1;
	}

	return 0;
}

int CTspRecycle::Pop(const string &strName,DWORD dwFlag)
{
	BOOL bRet = TRUE;

	// �����ļ���
	list<PrivateData>::iterator iter = find(m_list.begin(),m_list.end(),strName);
	 if( iter != m_list.end() )
	 {
		 // Ҫɾ��
		if( dwFlag == FLAG_DEL )
		{
			

			if( iter->flag == FLAG_FILE )
			{
				bRet = DeleteFile(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"ɾ���ļ�ʧ��:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
					MoveFileEx(iter->name.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}	
			else
			{
				bRet = RemoveDirectory(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"ɾ���ļ�ʧ��:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
				}
			}
		}

		// �б���ɾ��
		m_list.erase(iter);
	 }
	 else
	 {
		 LOG((LEVEL_WARNNING,"û�ҵ�Ҫɾ�����ļ�:%s \n",strName.c_str()));
		 bRet = FALSE;
	 }

	 return bRet?0:-1;
}

int CTspRecycle::PopAll(DWORD dwFlag)
{
	BOOL bRet;

	// �����Ҫɾ���ű���,���򣬱���Ŀ¼ɾ����
	if( dwFlag == FLAG_DEL )
		for( list<PrivateData>::reverse_iterator iter=m_list.rbegin();iter!=m_list.rend();++iter )
		{
			// Ҫɾ��
			if( iter->flag == FLAG_FILE )
			{
				bRet = DeleteFile(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"ɾ���ļ�ʧ��:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
					MoveFileEx(iter->name.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
			else
			{
				bRet = RemoveDirectory(iter->name.c_str());
				if( !bRet )
				{
					LOG((LEVEL_WARNNING,"ɾ���ļ�ʧ��:%s LastError=%d \n",iter->name.c_str(),GetLastError()));
				}
			}
		}


	//���
	m_list.clear();

	return 0;
}
