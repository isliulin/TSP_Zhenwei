#pragma once
#include "afxcmn.h"


// CdlgProgress 对话框

class CdlgProgress : public CDialog
{
	DECLARE_DYNAMIC(CdlgProgress)

public:
	CdlgProgress(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CdlgProgress();
	BOOL SetPos(int nSizeCur, int nSizeTotal);
	void ShowClientUpdataInfo(SOCKET socket,TCHAR *szUpdataFile);

	CString m_strUpdataFile;

// 对话框数据
	enum { IDD = IDD_DLG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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
