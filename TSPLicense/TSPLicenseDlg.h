// TSPLoginSrvDlg.h : ͷ�ļ�
//

#pragma once
#include "TrayIcon.h"

BOOL SendUTransmission(SOCKET sock, char *Buffer);

// CTSPLoginSrvDlg �Ի���
class CTSPLoginSrvDlg : public CDialog
{
// ����
public:
	CTSPLoginSrvDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TSPLICENSE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CTrayIcon m_TrayIcon;
	static UINT m_nTaskCreatedMSG;
	static UINT m_nTaskTrayNotify;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSelfile();
	afx_msg void OnBnClickedBtnStartsrv();
	afx_msg void OnBnClickedBtnStopsrv();
	afx_msg void OnBnClickedBtnStopsrv2();
	afx_msg void OnBnClickedBtnExit();
private:
	int m_port;
	CString m_strFileName;

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnHide();
	afx_msg void OnShow();
	afx_msg void OnExit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
