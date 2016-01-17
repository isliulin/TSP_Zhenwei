

#if !defined(AFX_LOG_H__8B1293E1_2CAA_11D4_8588_0050BAF16779__INCLUDED_)
#define AFX_LOG_H__8B1293E1_2CAA_11D4_8588_0050BAF16779__INCLUDED_

#include "windows.h"
#include "stdio.h"





#include <string>

#define OUT_MODE_TO_FILE	1
#define OUT_MODE_TO_CONSOLE	2

#define EXT_MODE_THREAD_ID	1
#define EXT_MODE_LOCAL_TIME	2
#define EXT_MODE_COUNT		4
#define EXT_MODE_LINE		8
#define EXT_MODE_TRUNC		0x10

#define LEVEL_ERROR		1
#define LEVEL_WARNNING	2
#define LEVEL_FUNC_ROUTE	3//线路主流程
#define LEVEL_FUNC_IN_OUT	4//函数入口和出口变量
#define LEVEL_MEMORY	4//内存
#define LEVEL_TEMP		5//
#define LEVEL_INFO		6//
#define LEVEL_TRACE		7//



#ifdef _MYDEBUG
class CLog
{
private:
	char  m_PathName[MAX_PATH];   //全局的路径
	FILE  *m_fp;
	ULONG m_OutMode;
	ULONG m_ExtMode;
	char *m_pBuffer;
	ULONG m_MaxBuffer;
	ULONG m_Count;
	DWORD m_maxLogFileSize;

	DWORD m_line;
	std::string m_file;
	CRITICAL_SECTION m_cs;
public:
	CLog(){ InitializeCriticalSection(&m_cs);}
	~CLog()
	{ 
		Close(); 
		DeleteCriticalSection(&m_cs);
	}

	DWORD GetFileLen();
	BOOL Open(char *fileName,ULONG outmode,ULONG extmode,ULONG maxBuffer,DWORD maxLogFileSize);
	void Close();
	BOOL Log(ULONG Level,const char *fmt, ...);
	void Begin(const char *szFile,DWORD dwLine)
	{
		EnterCriticalSection(&m_cs);
		const char *p = strrchr(szFile,'\\');
		if(!p)
			p = szFile;
		else
			++p;
		m_file.assign(p);
		m_line = dwLine;
	}
	void End(void)
	{
		LeaveCriticalSection(&m_cs);
	}
};


extern CLog g_log;


//日志宏，不能用于不带括号的if..else中
#define LOG(x)	 \
{ \
	g_log.Begin(__FILE__,__LINE__);	\
	g_log.Log x	;\
	g_log.End(); \
}


#else	//不使用日志


#define LOG(x) ( (void) (x) )


#endif


#endif // !defined(AFX_LOG_H__8B1293E1_2CAA_11D4_8588_0050BAF16779__INCLUDED_)
