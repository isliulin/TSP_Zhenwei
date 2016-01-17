/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief �ļ�ϵͳ��������Ҫboost��
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#if !defined(__VIRUS_FILE_SYSTEM_H__)
#define __VIRUS_FILE_SYSTEM_H__

#include "ddc_util.h"

#define SAFE_PATH_RESERVED 16				//����һ���Ŀռ���������·������ "\\?\"
#define MAX_PATH_SAFE MAX_PATH*2+SAFE_PATH_RESERVED

namespace nm_ddc
{
	//�°����Ŀ¼
	//����:Ŀ¼�����Ƿ�ݹ飬�Ƿ�������ӣ�������ȣ��ļ��ص���Ŀ¼�ص����Զ�������
	typedef enum enumDirAction{
		DIRACTION_STOP,					//0 ֹͣ
		DIRACTION_CONTINUE,				//1 ����
		DIRACTION_SKIP,					//2 �˳�����ѭ��
	};

	/**
	* Ŀ¼����.
	* @brief 
	* Ŀ¼����DIRACTION_SKIP,������Ŀ¼���ļ�����DIRACTION_SKIP ������
	* fileName ����"\\",����Ҳ������ʹ�ã�ƴװĿ¼��"\\\\",��Ŀ¼�ų����÷���Ҫע�ⷵ��ֵ������
	* @param fileNmae ������Ŀ¼��
	* @param recurse �Ƿ�ݹ�
	* @param followLinks �Ƿ��������Ŀ¼
	* @param �Ƿ��������
	* @param �ļ��ص�����
	* @param Ŀ¼�ص�����
	* @param �Զ������ݣ��ص������з���
	* @return DIRACTION_CONTINUE ��������
	* @return DIRACTION_STOP ֹͣ����
	*/
	int recursive_dir_action(const char *fileName,
						 int recurse, int followLinks, int depthFirst,
						 int (*fileAction) (const char *fileName,struct stat * statbuf,void* userData),
						 int (*dirAction) (const char *fileName,struct stat * statbuf,void* userData),
						 void* userData);

	//���
	int recursive_dir_action_w(const wchar_t *fileName,
						   int recurse, int followLinks, int depthFirst,
						   int (*fileAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   int (*dirAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   void* userData);


	//�ļ�������������ǿ��ȷ���Ӵ�	
	int VirDeleteFile(const char *fileName,const char *confirmStr=NULL);
	int VirRemoveDirectory(const char *dir,const char *confirmStr);
	int VirClearDirectory(const char *dir,const char *confirmStr);

	//������ݹ飬�´����һ�������ļ�����������Ŀ¼
	int VirCopyDirectory(const char *s,const char *d,int recurse);
	int VirCreateDirectory(const char *dir);

	/// �ļ���С
	uint file_size(const char *);

	/// �ļ���С
	uint file_size(const wchar_t *);

	/// �Ƿ�Ŀ¼
	bool is_directory(const char *);

	/// �Ƿ�Ŀ¼
	bool is_directory(const wchar_t *);

	/// �Ƿ���ļ�
	bool is_empty(const char *);

	/// �Ƿ���ļ�
	bool is_empty(const wchar_t *);

	/// �ļ���Ŀ¼�Ƿ����
	bool exists(const char *) ;

	/// �ļ���Ŀ¼�Ƿ����
	bool exists(const wchar_t *) ;

	/// ����Ŀ¼
	bool create_directory(const wchar_t *);
	
}	// end nm_ddc


#endif	//__VIRUS_FILE_SYSTEM_H__
