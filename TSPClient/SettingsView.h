#if !defined(AFX_SETTINGSVIEW_H__0BE25EB6_DFFA_4CEB_A4E7_BD98236BB73A__INCLUDED_)
#define AFX_SETTINGSVIEW_H__0BE25EB6_DFFA_4CEB_A4E7_BD98236BB73A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettingsView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "control/HoverEdit.h"
#include "control/WinXPButtonST.h"
#include "dlgProgress.h"
#include "afxcmn.h"
#include "afxwin.h"
//#define CButton	CCJFlatButton

#ifndef SAFE_CLOSE_FILE
#define SAFE_CLOSE_FILE(p) {if (INVALID_HANDLE_VALUE != p) {CloseHandle(p); p = INVALID_HANDLE_VALUE;}}
#endif
class CSettingsView : public CFormView
{
protected:
	DECLARE_DYNCREATE(CSettingsView)

// Form Data
public:
	//{{AFX_DATA(CSettingsView)
	enum { IDD = IDD_SETTINGS };
	CString	m_remote_host;
	CString	m_remote_port;
	UINT	m_listen_port;
	UINT	m_max_connections;
	BOOL	m_connect_auto;
	BOOL	m_bIsDisablePopTips;
	BOOL	m_bIsSaveAsDefault;
	CString	m_encode;
	CString	m_ServiceDisplayName;
	CString	m_ServiceDescription;
	CString	m_pass;
	CString	m_username;
	CString	m_userpass;
	BOOL	m_delete;

	//add by zhenyu vpn
	UINT    m_iVpnFlag;
	CString m_VpnServerIP;
	CString m_VpnTspIP;
	CString m_VpnUsername;
	CString m_VpnPassd;

	CString m_Mmuser;
	char    m_szMmID[2][8];


	//add end
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

	CSettingsView();           // public constructor used by dynamic creation
	virtual ~CSettingsView();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsView)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSettingsView)
	afx_msg void OnChangeConfig(UINT id);
	afx_msg void OnResetport();
	afx_msg void OnConnectAuto();
	afx_msg void OnCheckAuth();
	afx_msg void OnTestProxy();
	afx_msg void OnCheckProxy();
	afx_msg void OnTestMaster();
	afx_msg void OnDisablePoptips();
	afx_msg void OnSaveasDefault();
	afx_msg void OnShengCheng();
	afx_msg void OnChangePass();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static	DWORD WINAPI	TestProxy(LPVOID lparam);
	static	DWORD WINAPI	TestMaster(LPVOID lparam);
	static DWORD WINAPI    DownloadServerEx(LPVOID lpVoid);				//ÏÂÔØ·þÎñ¶Ë
	DWORD  DownloadServer();
	DWORD DownloadServer2();
	void UpdateProxyControl();
	SOCKET LoginLicenseSvr();
	void WriteClueToMumaFile(LPCSTR szMumaFile,LPCSTR szCase,LPCSTR szClue,int iOffsetXX);
	BOOL m_bFirstShow;
	CHoverEdit	m_Edit[9];
	CWinXPButtonST	m_Btn[3];
	CdlgProgress m_dlgProgress;
	HANDLE m_hThreadHandle;
	CString m_strServerFile;
public:
//	afx_msg void OnIpnFieldchangedIpaddressalert(NMHDR *pNMHDR, LRESULT *pResult);
	//CIPAddressCtrl m_ctrAlertIP;
// 	afx_msg void OnBnClickedAddakertip();
// 	afx_msg void OnBnClickedDeletealertip();
	afx_msg void OnBnClickedBaojian();
	BOOL m_filterUnknown;
	afx_msg void OnBnClickedFilterUnknown();
	BOOL m_autoSelectVisible;
	afx_msg void OnBnClickedAutoSelectVisible();
	int m_MaxConnect;
	afx_msg void OnEnChangeMaxConnect();
//	afx_msg void OnCbnSelchangeCombo2();
	CComboBox m_UserChoice;
/*	afx_msg void OnCbnSetfocusCombo2();*/
	//virtual void OnInitialUpdate();
	afx_msg void OnCbnDropdownCombo2();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSVIEW_H__0BE25EB6_DFFA_4CEB_A4E7_BD98236BB73A__INCLUDED_)
