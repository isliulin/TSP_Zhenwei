#ifndef _TEXTSP_INJECT_H__
#define _TEXTSP_INJECT_H__



PDWORD	pCodeRemote;

#define THREADSIZE 1024 * 10 //should be big enough

typedef SC_HANDLE	(__stdcall *TOpenSCManager)(LPCTSTR, LPCTSTR, DWORD);
typedef SC_HANDLE	(__stdcall *TOpenService)(SC_HANDLE, LPCTSTR, DWORD);
typedef BOOL		(__stdcall *TQueryServiceStatus)(SC_HANDLE, LPSERVICE_STATUS);
typedef BOOL		(__stdcall *TControlService)(SC_HANDLE, DWORD, LPSERVICE_STATUS);
typedef BOOL		(__stdcall *TStartService)(SC_HANDLE, DWORD, LPCTSTR*);
typedef BOOL		(__stdcall *TDeleteService)(SC_HANDLE);
typedef BOOL		(__stdcall *TCloseServiceHandle)(SC_HANDLE);

typedef DWORD		(__stdcall *TSHDeleteKey)(HKEY, LPCTSTR);

typedef BOOL		(__stdcall	*TCloseHandle)(HANDLE);
typedef VOID		(__stdcall	*TSleep)(DWORD);
typedef BOOL		(__stdcall	*TDeleteFile)(LPCTSTR);




typedef struct
{
	TOpenSCManager		MyOpenSCManager;
	TOpenService		MyOpenService;
	TQueryServiceStatus	MyQueryServiceStatus;
	TControlService		MyControlService;
	TStartService		MyStartService;
	TDeleteService		MyDeleteService;
	TCloseServiceHandle	MyCloseServiceHandle;

	TSHDeleteKey		MySHDeleteKey;

	TCloseHandle		MyCloseHandle;
	TSleep				MySleep;
	TDeleteFile			MyDeleteFile;

	char				strServiceName[100];	//服务名称
	char				strServiceRegKey[100];	//服务在注册表中的位置
}REMOTE_PARAMETER, *PREMOTE_PARAMETER;


typedef HMODULE (WINAPI *TLoadLibrary)(
						   __in          LPCTSTR lpFileName
						   );

typedef BOOL (WINAPI *TFreeLibrary)(
						__in          HMODULE hModule
						);

typedef HANDLE (WINAPI *TCreateEvent)(
						  __in          LPSECURITY_ATTRIBUTES lpEventAttributes,
						  __in          BOOL bManualReset,
						  __in          BOOL bInitialState,
						  __in          LPCTSTR lpName
						  );

typedef DWORD (WINAPI *TWaitForSingleObject)(
								 __in          HANDLE hHandle,
								 __in          DWORD dwMilliseconds
								 );

typedef BOOL (WINAPI *TResetEvent)(
					   __in          HANDLE hEvent
					   );

typedef UINT (WINAPI *TGetSystemDirectory)(
							   __out         LPTSTR lpBuffer,
							   __in          UINT uSize
							   );

typedef HMODULE (WINAPI *TGetModuleHandle)(
							   __in          LPCTSTR lpModuleName
							   );


typedef HANDLE (WINAPI *TOpenEvent)(
						__in          DWORD dwDesiredAccess,
						__in          BOOL bInheritHandle,
						__in          LPCTSTR lpName
						);

typedef BOOL (WINAPI *TInitializeSecurityDescriptor)(
	__out         PSECURITY_DESCRIPTOR pSecurityDescriptor,
	__in          DWORD dwRevision
	);



typedef BOOL (WINAPI *TSetSecurityDescriptorDacl)(
									  __in      PSECURITY_DESCRIPTOR pSecurityDescriptor,
									  __in          BOOL bDaclPresent,
									  __in_opt      PACL pDacl,
									  __in          BOOL bDaclDefaulted
									  );






typedef struct
{
	TLoadLibrary        MyLoadLibrary;
	TFreeLibrary        MyFreeLibrary;
	TCreateEvent        MyCreateEvent;
	TWaitForSingleObject MyWaitForSingleObject;
	TResetEvent         MyResetEvent;
	TGetSystemDirectory MyGetSystemDirectory;
	TGetModuleHandle    MyGetModuleHandle;
	TSleep              MySleep;
	TCloseHandle        MyCloseHandle; 
	TInitializeSecurityDescriptor MyInitializeSecurityDescriptor;
	TSetSecurityDescriptorDacl MySetSecurityDescriptorDacl;

// 	HMODULE             hModule;
 	char				szDllName[100];	//DLL
}REMOTE_LOAD, *PREMOTE_LOAD;

BOOL EnableDebugPriv(void)
{
	HANDLE hToken = NULL;
	HANDLE Handle = ::GetCurrentProcess();
	if (!::OpenProcessToken(Handle,TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}
	
	LUID sedebugnameValue = {0};
	if (!::LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&sedebugnameValue))
	{
		::CloseHandle(hToken);
		return FALSE;
	}

	TOKEN_PRIVILEGES tkp = {0};
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		::CloseHandle(hToken);
		return FALSE;
	}

	::CloseHandle(hToken);

	return TRUE;
}

DWORD WINAPI MyFunc(LPVOID lparam)
{
	REMOTE_PARAMETER *pRemoteParam = (REMOTE_PARAMETER *)lparam;

	SC_HANDLE scm = pRemoteParam->MyOpenSCManager(0, 0,	SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE service = pRemoteParam->MyOpenService(scm, pRemoteParam->strServiceName, SERVICE_ALL_ACCESS | DELETE);
	if (scm==NULL && service == NULL)
	{
		return -1;
	}

	SERVICE_STATUS Status = {0};
	if (!pRemoteParam->MyQueryServiceStatus(service, &Status))
	{
		return -1;
	}

	if (Status.dwCurrentState != SERVICE_STOPPED)
	{
		if (!pRemoteParam->MyControlService(service, SERVICE_CONTROL_STOP, &Status))
		{
			return -1;
		}

		//Wait to service stopped
		pRemoteParam->MySleep(1000);
	}

	
	//delete service
	pRemoteParam->MyDeleteService(service);
	//delete regkey
	pRemoteParam->MySHDeleteKey(HKEY_LOCAL_MACHINE, pRemoteParam->strServiceRegKey);

	if (service != NULL)
	{
		pRemoteParam->MyCloseServiceHandle(service);
	}

	if (scm != NULL)
	{
		pRemoteParam->MyCloseServiceHandle(scm);
	}
	
	return 0;
}




DWORD WINAPI MyFunc2(LPVOID lparam)
{
	REMOTE_LOAD *pRemotLoad = (REMOTE_LOAD *)lparam;


	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	pRemotLoad->MyInitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
	pRemotLoad->MySetSecurityDescriptorDacl(&sd,TRUE,NULL,FALSE);
	sa.nLength  =   sizeof (SECURITY_ATTRIBUTES);
	sa.bInheritHandle  =  TRUE;
	sa.lpSecurityDescriptor  =   & sd;

	

	//OpenEvent(EVENT_ALL_ACCESS,FALSE,szDllName);

	HANDLE hEvent = pRemotLoad->MyCreateEvent(&sa,FALSE,FALSE,pRemotLoad->szDllName); 


	while(TRUE)
	{



		pRemotLoad->MyWaitForSingleObject(hEvent,INFINITE);//等待结束后事件对象自动重置为无效
	
		HMODULE hModule=pRemotLoad->MyGetModuleHandle(pRemotLoad->szDllName);
		if (hModule)
		{
			pRemotLoad->MyFreeLibrary(hModule);
			hModule=0;
		}
		else
		{
			//_asm int 3
			char szSibDllPath[MAX_PATH];
		

			pRemotLoad->MyGetSystemDirectory(szSibDllPath,MAX_PATH);
			DWORD dwLen=strlen(szSibDllPath);

			szSibDllPath[dwLen]=0x5c;
			szSibDllPath[dwLen+1]=0;
			//szSibDllPath[strlen(szSibDllPath)]=0;
			//strcat(szSibDllPath,'\\');
			strcat(szSibDllPath,pRemotLoad->szDllName);
			pRemotLoad->MyLoadLibrary(szSibDllPath);
		}
		pRemotLoad->MyResetEvent(hEvent);
	//	pRemotLoad->MyCloseHandle(hEvent);
	//	hEvent=0;

	}

	return 0;
}



BOOL InjectRemoteCode(LPCTSTR lpProcessName, char *szDllName,HMODULE hModule)
{
	EnableDebugPriv();
	HANDLE hProcess;

	void *pDataRemote;


	//HANDLE hThread;

	__try
	{
		if(lpProcessName)
		{
			hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, ::GetProcessID(lpProcessName));
			if (hProcess == NULL){
				return FALSE;
			}

		}
		else hProcess=GetCurrentProcess();


		//////////////////////////////////////////////////////////////////////////
		REMOTE_LOAD remoteLoad = {0};

		HMODULE hKernel32 = ::GetModuleHandle(TEXT("Kernel32"));

		remoteLoad.MyLoadLibrary=(TLoadLibrary)GetProcAddress(hKernel32, "LoadLibraryA");
		remoteLoad.MyFreeLibrary=(TFreeLibrary)GetProcAddress(hKernel32,"FreeLibrary");
		remoteLoad.MyCreateEvent=(TCreateEvent)GetProcAddress(hKernel32,"CreateEventA");
		remoteLoad.MyWaitForSingleObject=(TWaitForSingleObject)GetProcAddress(hKernel32,"WaitForSingleObject");
		remoteLoad.MyResetEvent=(TResetEvent)GetProcAddress(hKernel32,"ResetEvent");
		remoteLoad.MyGetModuleHandle=(TGetModuleHandle)GetProcAddress(hKernel32,"GetModuleHandleA");
		remoteLoad.MyGetSystemDirectory=(TGetSystemDirectory)GetProcAddress(hKernel32,"GetSystemDirectoryA");
		remoteLoad.MySleep=(TSleep)GetProcAddress(hKernel32,"Sleep");
		remoteLoad.MyCloseHandle=(TCloseHandle)GetProcAddress(hKernel32,"CloseHandle");


		remoteLoad.MyInitializeSecurityDescriptor=(TInitializeSecurityDescriptor)GetProcAddress(LoadLibraryA("Advapi32.dll"),"InitializeSecurityDescriptor");
		remoteLoad.MySetSecurityDescriptorDacl=(TSetSecurityDescriptorDacl)GetProcAddress(LoadLibraryA("Advapi32.dll"),"SetSecurityDescriptorDacl");




	
		if (szDllName)
			lstrcpy(remoteLoad.szDllName, szDllName);
		//////////////////////////////////////////////////////////////////////////

		//Write thread parameter to Remote thread
		pDataRemote = (char*)VirtualAllocEx(hProcess, 0, sizeof(remoteLoad),
			MEM_COMMIT, PAGE_READWRITE);
		if (!pDataRemote)
			return FALSE;

		if (!WriteProcessMemory( hProcess, pDataRemote, &remoteLoad, sizeof(remoteLoad), NULL))
			return FALSE;

		//Write Code to Remote thread
		DWORD	cbCodeSize= THREADSIZE; //分配的空间不大一点，会出错, 过大，WriteProcessMemroy会失败

		pCodeRemote = (PDWORD)VirtualAllocEx(hProcess, 0, cbCodeSize, MEM_COMMIT,PAGE_EXECUTE_READWRITE);

		if (!pCodeRemote)	
			return FALSE;

// 
// 		PEXCEPTION_RECORD 
 		//_asm int 3

		memcpy((char*)MyFunc2+0xb,"\x90\x90\x90\x90\x90",5);

		if (!WriteProcessMemory(hProcess, pCodeRemote, &MyFunc2, cbCodeSize, NULL))
			return FALSE;


		if (CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pCodeRemote, pDataRemote, 0, NULL) == NULL)
		{
			return FALSE;
		}


		HANDLE hEvent=OpenEvent(EVENT_MODIFY_STATE,FALSE,szDllName); 
		while(!hEvent)
		{
			hEvent=OpenEvent(EVENT_MODIFY_STATE,FALSE,szDllName);
		}
		SetEvent(hEvent);
		CloseHandle(hEvent);

	}
	__finally
	{
// 		if (hThread){
// 			CloseHandle(hThread);
// 
// 		}
		if (hProcess){
			CloseHandle(hProcess);
		}
	}

	return TRUE;
}

BOOL InjectRemoveService(LPCTSTR lpProcessName, char *lpServiceName)
{
	EnableDebugPriv();
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, ::GetProcessID(lpProcessName));
	if (hProcess == NULL)
	{
		return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////
	REMOTE_PARAMETER remoteParameter = {0};

	HMODULE hAdvapi32 = LoadLibrary("advapi32.dll");

	
	remoteParameter.MyOpenSCManager = (TOpenSCManager)GetProcAddress(hAdvapi32, "OpenSCManagerA");
	remoteParameter.MyOpenService = (TOpenService)GetProcAddress(hAdvapi32, "OpenServiceA");
	remoteParameter.MyQueryServiceStatus = (TQueryServiceStatus)GetProcAddress(hAdvapi32, "QueryServiceStatus");
	remoteParameter.MyControlService = (TControlService)GetProcAddress(hAdvapi32, "ControlService");
	remoteParameter.MyStartService = (TStartService)GetProcAddress(hAdvapi32, "StartServiceA");
	remoteParameter.MyDeleteService = (TDeleteService)GetProcAddress(hAdvapi32, "DeleteService");
	remoteParameter.MyCloseServiceHandle = (TCloseServiceHandle)GetProcAddress(hAdvapi32, "CloseServiceHandle");
	
//	FreeLibrary(hAdvapi32);	

	HMODULE	hShlwapi = LoadLibrary("shlwapi.dll");
	remoteParameter.MySHDeleteKey = (TSHDeleteKey)GetProcAddress(hShlwapi, "SHDeleteKeyA");
//	FreeLibrary(hShlwapi);

	HMODULE	hKernel32 = LoadLibrary("kernel32.dll");
	remoteParameter.MySleep = (TSleep)GetProcAddress(hKernel32, "Sleep");
	remoteParameter.MyCloseHandle = (TCloseHandle)GetProcAddress(hKernel32, "CloseHandle");
//	FreeLibrary(hKernel32);
	
	lstrcpy(remoteParameter.strServiceName, lpServiceName);
	lstrcpy(remoteParameter.strServiceRegKey, "SYSTEM\\CurrentControlSet\\Services\\");
	lstrcat(remoteParameter.strServiceRegKey, remoteParameter.strServiceName);
	//////////////////////////////////////////////////////////////////////////

	//Write thread parameter to Remote thread
	void *pDataRemote = (char*)VirtualAllocEx(hProcess, 0, sizeof(remoteParameter),
		MEM_COMMIT, PAGE_READWRITE);
	if (!pDataRemote)
		return FALSE;

	if (!WriteProcessMemory( hProcess, pDataRemote, &remoteParameter, sizeof(remoteParameter), NULL))
		return FALSE;

	//Write Code to Remote thread
	DWORD	cbCodeSize= THREADSIZE; //分配的空间不大一点，会出错, 过大，WriteProcessMemroy会失败
	
	PDWORD	pCodeRemote = (PDWORD)VirtualAllocEx(hProcess, 0, cbCodeSize, MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);

	if (!pCodeRemote)	
		return FALSE;

	
	if (!WriteProcessMemory(hProcess, pCodeRemote, &MyFunc, cbCodeSize, NULL))
		return FALSE;

	if (CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pCodeRemote, pDataRemote, 0, NULL) == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

#ifdef UNICODE

#define szLoadLibrary "LoadLibraryW"
#define szGetModuleHandle "GetModuleHandleW"

#else

#define szLoadLibrary "LoadLibraryA"
#define szGetModuleHandle "GetModuleHandleA"

#endif

BOOL RemoteLoadLibrary(DWORD m_dwProcessID,IN LPCTSTR lpszDllName)
{


	BOOL    bRet=FALSE;	
	HMODULE hKernel32 = ::GetModuleHandle(TEXT("Kernel32"));


	//提升权限
	EnableDebugPriv();

	HANDLE hProcess =OpenProcess(PROCESS_CREATE_PROCESS | PROCESS_VM_OPERATION |PROCESS_VM_WRITE,FALSE,m_dwProcessID);

	if (!hProcess){
		return bRet;
	}

	LPVOID pLibRemote =VirtualAllocEx(hProcess,NULL,MAX_PATH,MEM_COMMIT,PAGE_EXECUTE_READWRITE);

	if (!pLibRemote){
		//dprintf((TEXT("VirtualAllocEx erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		return bRet;
	}

	if(!WriteProcessMemory(hProcess,pLibRemote,(void*)lpszDllName,MAX_PATH,NULL))
	{
	//	dprintf((TEXT("WriteProcessMemory erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		::VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE );
		return bRet;
	}

	HANDLE hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32,szLoadLibrary),
		pLibRemote, 0, NULL );

	if (!hThread){
	//	dprintf((TEXT("CreateRemoteThread erro :%d"),GetLastError()));
		CloseHandle(hProcess);
		::VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE);
		return bRet;
	}

	//
	WaitForSingleObject(hThread,INFINITE);

	DWORD   hLibModule;      //已加载的DLL的基地址（HMODULE）;

	//取得DLL的基地址
	GetExitCodeThread(hThread,&hLibModule);
	CloseHandle(hThread );

	//扫尾工作
	bRet=VirtualFreeEx(hProcess, pLibRemote,0,MEM_RELEASE );
	CloseHandle(hProcess);
	return bRet;

}

#endif //_TEXTSP_INJECT_H__
