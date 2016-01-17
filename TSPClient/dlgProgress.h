#pragma once
#include "afxcmn.h"


// CdlgProgress �Ի���

class CdlgProgress : public CDialog
{
	DECLARE_DYNAMIC(CdlgProgress)

public:
	CdlgProgress(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CdlgProgress();
	BOOL SetPos(int nSizeCur, int nSizeTotal);
	void ShowClientUpdataInfo(SOCKET socket,TCHAR *szUpdataFile);

	CString m_strUpdataFile;

// �Ի�������
	enum { IDD = IDD_DLG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CProgressCtrl m_ctrlProgress;
	int m_nSizeCur;
	int m_nSizeTotal;
	afx_msg void OnDestroy();

	void MySetWindowText(CString str);
};
