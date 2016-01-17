// LOGIN.cpp : implementation file
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "LOGIN.h"
#include "EnDeCode.h"
#include "Log.h"
#include "MyDBCOnn.h"
extern CLog g_log;
extern CLog g_logErr;

extern char	g_szUserName[];
extern char	g_szPassword[];


MYSQL_RES *res = NULL;
MYSQL_ROW row;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LOGIN dialog

LOGIN::LOGIN(CWnd* pParent /*=NULL*/)
	: CDialog(LOGIN::IDD, pParent)
{
	//{{AFX_DATA_INIT(LOGIN)
	m_username = _T("");
	m_userpass = _T("");
	m_baocun = FALSE;
	//m_onlinepass = _T("");
	//}}AFX_DATA_INIT
}


void LOGIN::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LOGIN)
	DDX_Text(pDX, IDC_username, m_username);
	DDX_Text(pDX, IDC_userpass, m_userpass);
	DDX_Check(pDX, IDC_CHECK1, m_baocun);
	//DDX_Text(pDX, IDC_onlinepass, m_onlinepass);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(LOGIN, CDialog)
	//{{AFX_MSG_MAP(LOGIN)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_BN_CLICKED(IDC_LOGIN, OnLogin)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LOGIN message handlers

void LOGIN::MySetWindowText(CString str)
{
	CEdit  *cEditWnd=(CEdit*)GetDlgItem(IDC_POINTEDIT);

	int   nLength=cEditWnd->GetWindowTextLength();
	//CEdit 

	cEditWnd->SetSel(nLength,nLength);      
	cEditWnd->ReplaceSel(str);  

// 	CString Orignstr;
// 	(GetDlgItem(IDC_POINTEDIT))->GetWindowText(Orignstr);
// 	Orignstr+=str;
// 	(GetDlgItem(IDC_POINTEDIT))->SetWindowText(Orignstr);
// 	Sleep(10);
	
}


void LOGIN::OnLogin() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString str;
	//goto hihi;
	if ( m_username.GetLength() == 0 || m_userpass.GetLength() == 0 )
	{
		MySetWindowText("请填写用户名和密码!\r\n");
		return;
	}

	MySetWindowText("正在获取服务器IP和端口...");



	char tgtIP[30] = {0};
	struct hostent *hp = NULL;
	if ((hp = gethostbyname(LICSERVER_IPADDR))==NULL)
	{
		MySetWindowText("出错\r\n");


		(GetDlgItem(IDC_POINTEDIT))->GetWindowText(str);
		str+="自动刷新DNS缓存.\r\n";
		(GetDlgItem(IDC_POINTEDIT))->SetWindowText(str);

		MySetWindowText("正在自动刷新DNS缓存...");

		if(FARPROC  pfnDnsFlushResolverCache=(GetProcAddress(LoadLibrary("Dnsapi.dll"),"DnsFlushResolverCache")))
		{
			pfnDnsFlushResolverCache();
		}

		MySetWindowText("完成\r\n");
		MySetWindowText("重新解析DNS...");

		if ((hp = gethostbyname(LICSERVER_IPADDR))==NULL)
		{
			MySetWindowText("出错,请检查网络是否畅通\r\n");
			return;
		}
		
	}
    MySetWindowText("成功\r\n");

	//////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD recvsize = 0;
	SOCKET sockInt;

	//创建SOCK
	sockInt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockInt == INVALID_SOCKET)
	{
		MySetWindowText("socket error!\r\n");
		return;
	}


	in_addr in;

	memcpy(&in, hp->h_addr, hp->h_length);
	lstrcpy(tgtIP,inet_ntoa(in));


	//struct hostent *hp;
	//获取服务器IP和端口
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(tgtIP);
	serverAddr.sin_port = htons(LICSERVER_LISPORT);

	MySetWindowText("正在连接服务器...");

	//连接服务
	if(connect(sockInt, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		CString str;
		str.Format("失败:%d\r\n",GetLastError());

		MySetWindowText(str);
		closesocket(sockInt);
		return;
	}

	MySetWindowText("成功\r\n");

	char USERIMFOR[256] = {0}, buff[256] = {0};
	wsprintf( USERIMFOR, "Login:%s@%s", m_username, m_userpass );

	MySetWindowText("验证账号密码...");

	if( send(sockInt, USERIMFOR, sizeof(USERIMFOR), 0) == SOCKET_ERROR )
	{
		MySetWindowText("出错\r\n");		
		return;
	}

	Sleep(50);
	int Ret = recv( sockInt, buff, sizeof(buff), NULL );
	if ( Ret == 0 || Ret == SOCKET_ERROR )
	{
		MySetWindowText("账号或者密码错误!\r\n");
		closesocket(sockInt);		
		return;
	}
	
	if ( lstrcmpi( buff, "Logined" ) == 0 )//通过验证
	{
//hihi:
		MySetWindowText("成功\r\n");
		dLogin = GetTickCount();
		closesocket(sockInt);
		
		lstrcpy(g_szUserName, m_username);
		lstrcpy(g_szPassword, m_userpass);
		m_TempPassword = m_userpass;

		if ( m_baocun )
		{
			m_inifile.SetString( "USER", "username", m_username );
			m_inifile.SetString( "USER", "userpass", EnDeCode::Encode(EnDeCode::encrypt(m_userpass.GetBuffer(0))) );
			m_inifile.SetString( "Settings", "OnLinePass", EnDeCode::Encode(EnDeCode::encrypt(m_onlinepass.GetBuffer(0))) );

			char szBuf[MAX_PATH]={0};
			//sprintf(szBuf,"%sclue_tsp\\",GetExeCurrentDir());

			sprintf(szBuf,"C:\\input\\clue_tsp\\");

			m_inifile.SetString( "Filter", "Path",szBuf);
		}
		//add by zhenyu
		char strInser[256] ={0};
		sprintf(strInser,"insert into tspuser(tspname, tsppwd,isvalid) values('%s','%s',1)",EnDeCode::Encode(EnDeCode::encrypt(m_username.GetBuffer(0))),EnDeCode::Encode(EnDeCode::encrypt(m_TempPassword.GetBuffer(0))));
		
		char strDelete[256] = "delete from tspuser";



		CMyDataBase mdb;


		if(mdb.ConnDB())   // modify by menglz 2013-11-26
		{

			if(!mdb.ExeSQL(strDelete))//清空表
			{
				g_logErr.Log(LEVEL_ERROR,"清空数据库中tspuser表失败\r\n");
			}
			if(!mdb.ExeSQL(strInser))
			{
				g_logErr.Log(LEVEL_ERROR,"插入数据库tspuser表中用户名密码失败\r\n");
			}

		}

		
		//add end

		//this->MessageBox("登录成功!",0,0);

		OnCancel();
	}
	MySetWindowText("出错\r\n");
}

void LOGIN::OnExit() 
{
	// TODO: Add your control notification handler code here
	ExitProcess(0);
}

BOOL LOGIN::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char strFindName[128] = "select tspname,tsppwd from tspuser";


	CMyDataBase mdb;

	if(!mdb.ConnDB())
	{

		AfxMessageBox("初始化登录对话框时，连接数据库失败,程序退出！");
		exit(0);
	}


	
	res = mdb.GetDataSet(strFindName);
	//modify by yx 2013-11-04
	if (res==NULL)
	{
		AfxMessageBox("TSPsqlOpenRecordset 出错，具体查看TSPSql.log日志");
		exit(0);
	}
	//modify end...
	
	if (0 != res->row_count)//取不到的时候
	{
		row = mysql_fetch_row(res);
		CString tspnamep = row[0];
		if ( tspnamep.GetLength() != 0 )
		{
			SetDlgItemText( IDC_username, EnDeCode::decrypt(EnDeCode::Decode(tspnamep.GetBuffer(0))));
		}
		CString tsppwd = row[1];
		if ( tsppwd.GetLength() != 0 )
		{
			SetDlgItemText( IDC_userpass, EnDeCode::decrypt(EnDeCode::Decode(tsppwd.GetBuffer(0))));
			OnLogin();
		}
	}

	UpdateData();
	m_baocun = TRUE;
	UpdateData(FALSE);


	CBitmap * pBmpOld;
	RECT rectClient;
	VERIFY(m_brush=(HBRUSH)GetStockObject(HOLLOW_BRUSH));
	VERIFY(m_Bitmap.LoadBitmap(IDB_SPLASH));

	m_Bitmap.GetObject(sizeof(BITMAP),&m_bmInfo);
	GetClientRect(&rectClient);
	m_size.cx=rectClient.right;
	m_size.cy=rectClient.bottom;
	m_pt.x=rectClient.left;
	m_pt.y=rectClient.top;
	CClientDC dc(this);
	VERIFY(m_dcMem.CreateCompatibleDC(&dc));
	VERIFY(pBmpOld=m_dcMem.SelectObject(&m_Bitmap));
	VERIFY(m_hBmpOld=(HBITMAP)pBmpOld->GetSafeHandle()); 

	MySetWindowText("欢迎使用TSP取证系统\r\n");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH LOGIN::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

 //	pDC->SetBkMode(TRANSPARENT);
// 
	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC:
	case IDC_CHECK1:
		pDC->SetBkColor(RGB(255,255,255));
		hbr=CreateSolidBrush(RGB(255,255,255)); //背景色
		break;


	case IDC_POINTEDIT:
		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkMode(TRANSPARENT);
	//	pDC->SetBkColor(RGB(255,255,255));
		hbr=CreateSolidBrush(RGB(255,255,255)); //背景色

	//	hbr=CreateSolidBrush(NULL_BRUSH); //背景色
		break;
	}

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void LOGIN::OnDestroy()
{
	CDialog::OnDestroy();

	ASSERT(m_hBmpOld);
	VERIFY(m_dcMem.SelectObject(CBitmap::FromHandle(m_hBmpOld)));

	m_Bitmap.DeleteObject(); 

	// TODO: 在此处添加消息处理程序代码
}

BOOL LOGIN::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	pDC->StretchBlt(m_pt.x,m_pt.y,m_size.cx,m_size.cy,&m_dcMem,
		0,0,m_bmInfo.bmWidth-1,m_bmInfo.bmHeight-1,SRCCOPY);
	return TRUE;

	//return CDialog::OnEraseBkgnd(pDC);
}
