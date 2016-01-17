//#include "../stdafx.h"
#ifdef _MYDEBUG

#include "Log.h"
#include "stdio.h"
#include "stdlib.h"

CLog g_log;

	DWORD CLog::GetFileLen()
	{
		if(m_fp)
		{
			fseek(m_fp,0,SEEK_END);
			return ftell(m_fp);
		}
		return 0;
	}
	BOOL CLog::Open(char *fileName,ULONG outmode,ULONG extmode,ULONG maxBuffer,DWORD maxLogFileSize)
	{
		m_fp=NULL;
		if(fileName)
		{
			if( extmode & EXT_MODE_TRUNC )
				m_fp = fopen(fileName,"w+");
			else
				m_fp=fopen(fileName,"a+");
		}
		strncpy(m_PathName,fileName,MAX_PATH);


		m_OutMode=outmode;
		m_ExtMode=extmode;
		m_MaxBuffer=maxBuffer;
		m_Count=0;
		m_maxLogFileSize=maxLogFileSize;
		m_pBuffer=(char*)malloc(m_MaxBuffer+1);
		if(m_fp)
			return TRUE;
		else
			return FALSE;
	}
	void CLog::Close()
	{
		if(m_pBuffer)
		{
			free(m_pBuffer);
			m_pBuffer=NULL;
		}
		if(m_fp)
		{
			fclose(m_fp);
			m_fp=NULL;
		}
	}
	BOOL CLog::Log(ULONG Level,const char *fmt, ...)
	{
		if( Level > 6 )
			return TRUE;

		ULONG ret=0;

		if(m_maxLogFileSize>0)
		{
			WIN32_FIND_DATA findData;
			HANDLE handle=FindFirstFile(m_PathName,&findData);
			if(handle!=INVALID_HANDLE_VALUE)
			{
				if(findData.nFileSizeLow>m_maxLogFileSize)
				{
					Close();
					DeleteFile(m_PathName);
					if(!Open(m_PathName,m_OutMode,m_ExtMode,m_MaxBuffer,m_maxLogFileSize))
					{
						return FALSE;
					}
				}
				FindClose(handle);
			}	
		}


		va_list ap;
		if(m_pBuffer==NULL)
		{			
			return FALSE;
		}
		if(m_fp==NULL)
		{		
			return FALSE;
		}


		

		m_pBuffer[0]=0;
		if(m_ExtMode&EXT_MODE_COUNT)
		{
			char Temp[100];
			sprintf(Temp,"%08d:",m_Count+1);
			strncat(m_pBuffer,Temp,m_MaxBuffer);
			m_Count++;
		}
		if(m_ExtMode&EXT_MODE_THREAD_ID)
		{
			char Temp[100];
			sprintf(Temp,"%08X:",GetCurrentThreadId());
			strncat(m_pBuffer,Temp,m_MaxBuffer);
		}
		if(m_ExtMode&EXT_MODE_LOCAL_TIME)
		{
			char Temp[100];
			SYSTEMTIME time;
			GetLocalTime(&time);
			sprintf(Temp,"%04d-%02d-%02d %02d:%02d:%02d ",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);
			strncat(m_pBuffer,Temp,m_MaxBuffer);
		}
		if(Level==LEVEL_ERROR)
		{
			strncat(m_pBuffer,"error:",m_MaxBuffer);
		}else if(Level==LEVEL_WARNNING)
		{
			strncat(m_pBuffer,"warnning:",m_MaxBuffer);
		}
		
		//文件名行号
		if(m_ExtMode&EXT_MODE_LINE)		
		{
			char Temp[100];
			sprintf(Temp," %s:%d\t",m_file.c_str(),m_line);
			strncat(m_pBuffer,Temp,m_MaxBuffer);
		}
		

		va_start(ap, fmt);
		ret=vsprintf (m_pBuffer+strlen(m_pBuffer), fmt, ap);
		if(strlen(m_pBuffer)>m_MaxBuffer)
		{
			return FALSE;
		}

		if(m_OutMode&OUT_MODE_TO_CONSOLE)
		{
			printf(m_pBuffer);
		}
		if(m_OutMode&OUT_MODE_TO_FILE)
		{
			if(m_fp)
			{
				fseek(m_fp,0,SEEK_END);
				fprintf(m_fp,m_pBuffer);
				fflush(m_fp);
			}
		}
		va_end(ap);
		

		return TRUE;
	}

#endif	// _MYDEBUG