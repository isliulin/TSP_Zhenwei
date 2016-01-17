#if !defined(__TSP_PLUG_DLL_H__)
#define __TSP_PLUG_DLL_H__

#include "windows.h"

//共享内存通讯头
typedef struct _TagPlugCommSt
{
	void reset()
	{ 
		memset(this,0,sizeof(PLUG_COMM_ST)); 
		magic=MAGIC;
		ver=CURVER;
		request=REQUEST;
		pid = 0xFFFFFFFF;
	};

	enum{ECHO,START,STOP,QUIT,REMOVE,UPDATE};
	enum{CURVER=1,MAGIC=78};	
	enum{REQUEST=1,REPLY=2};
	
	BYTE magic;
	BYTE ver;
	BYTE cmd;
	BYTE request;
	WORD crc16;
	WORD len;
	DWORD pid;
}PLUG_COMM_ST;

// 共享内存名称
#define TSP_PLUG_COMM_SIGN "DkjeB73@#$SSR#@#$jfiis09i7-"

#ifdef TSPPLUG_EXPORTS
#define TSPPLUGIN_API __declspec(dllexport)
#else
#define TSPPLUGIN_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	TSPPLUGIN_API void start(HWND hwnd,HINSTANCE hinst,LPSTR lpszCmdLine,int nCmdShow);	
	TSPPLUGIN_API void stop(HWND hwnd,HINSTANCE hinst,LPSTR lpszCmdLine,int nCmdShow);	

#ifdef __cplusplus
};
#endif


#endif	// __TSP_PLUG_DLL_H__

