
#ifndef __DDC_DEBUG_LOG_H__
#define __DDC_DEBUG_LOG_H__


#ifdef _DDC_DEBUG

//show error message
#define Breakpoint() 

extern const char *l_ddcDebugFileName;
//init log and release log
extern void ddc_DebugLogInit(const char *log_file);
extern void ddc_DebugLogRelease(void);


//add log class
void Log_AddLoggingClasses(void);



//set log type and get log type
void Log_Enable (const char * log_class);
int Log_Enabled (const char * log_class);

#define DDC_DEBUGLOG_CONTINUE 0x00000001
#define DDC_DEBUGLOG_ENC 0x00000002
#define DDC_DEBUGLOG_LIMIT 0x00000004
void Log_Control(unsigned int flag);

void ddc_DebugLogBegin(const char *,int);
void ddc_DebugLogMiddle(const char * log_class,const char *format, ...);
void ddc_DebugLogEnd(void);


//record log function
#define DDC_LOG(x)										\
	{												\
		ddc_DebugLogBegin(__FILE__, __LINE__);	\
		ddc_DebugLogMiddle x;						\
		ddc_DebugLogEnd();						\
	}

#else
#define DDC_LOG(x) ( (void) (x) )
#endif		//_ddc_DEBUG





#endif //__DDC_DEBUG_LOG_H__


