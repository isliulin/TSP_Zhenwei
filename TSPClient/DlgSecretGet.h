#pragma once

#include "macros.h"
#include "afxwin.h"
#include "TSPClientView.h"
// CDlgSecretGet 对话框

class CDlgSecretGet : public CDialog
{
	DECLARE_DYNAMIC(CDlgSecretGet)

public:
	CDlgSecretGet(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // 标准构造函数
	virtual ~CDlgSecretGet();
	typedef struct  
	{ 
		int nCommandId;
		int nResourceId;
	}ID_CONTRAST, *PID_CONTRAST;

	typedef struct  
	{ 
		int nDeepID;
		CHAR szDesDeep[16];
	}DESDEEP, *PDESDEEP;

	void OnReceiveComplete();


// 对话框数据
	enum { IDD = IDD_DLGSECRET_GET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnUpplugsrc();
	DECLARE_MESSAGE_MAP()

	BOOL GetCheckBoxParam();
	void UpdateState();


private:
	INFOSTATE m_infoSate;
	ClientContext* m_pContext;
	CTSPClientView *m_pCTSPView;
	static ID_CONTRAST m_sIDContrast[];
	static DESDEEP m_sScreenBitDeep[];
	static LPSTR m_szStateDes[];
	static ID_CONTRAST m_sAlarmSrnConst[];
	CIOCPServer *m_iocpServer;
	INFOSTATE m_infoSateNew;
	DWORD m_dwSerVer;
	int m_nSizeofSstrcttate;
	CComboBox m_ctlCombBitScreen;
	CString m_strScrTime;
public:
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedChkcleversrc();
};
