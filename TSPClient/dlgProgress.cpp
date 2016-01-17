// dlgProgress.cpp : 实现文件
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "dlgProgress.h"


// CdlgProgress 对话框

IMPLEMENT_DYNAMIC(CdlgProgress, CDialog)

CdlgProgress::CdlgProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CdlgProgress::IDD, pParent)
{
	m_nSizeCur = 0;
	m_nSizeTotal = 0;
	char szTemp[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szTemp, sizeof(szTemp));
	char *pPos = StrRChr(szTemp, NULL, '\\');
	*pPos = 0;
	m_strUpdataFile.Format("%s\\%s",szTemp,"readme.txt");
}

CdlgProgress::~CdlgProgress()
{
}

void CdlgProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
}


BEGIN_MESSAGE_MAP(CdlgProgress, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()




// CdlgProgress 消息处理程序

BOOL CdlgProgress::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);

	m_nSizeTotal=0;
	m_nSizeCur=0;


	SetTimer(NULL, 100, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CdlgProgress::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (0 != m_nSizeTotal)
	{
		m_ctrlProgress.SetPos(m_nSizeCur * 100 / m_nSizeTotal);
	}

	if (m_nSizeCur>=m_nSizeTotal)
	{
		m_nSizeTotal=0;
		m_nSizeCur=0;
	}

	CDialog::OnTimer(nIDEvent);
}


BOOL CdlgProgress::SetPos(int nSizeCur, int nSizeTotal)
{
	if (0 != nSizeTotal)
	{
		m_nSizeTotal = nSizeTotal;
		m_nSizeCur = nSizeCur;
	}

	return TRUE;
}

void CdlgProgress::MySetWindowText(CString str)
{
	CEdit  *cEditWnd=(CEdit*)GetDlgItem(IDC_EDIT_UNDATAINFO);

	int   nLength=cEditWnd->GetWindowTextLength();
	//CEdit 

	cEditWnd->SetSel(nLength,nLength);      
	cEditWnd->ReplaceSel(str);  

	// 	CString Orignstr;
	// 	(GetDlgItem(IDC_POINTEDIT))->GetWindowText(Orignstr);
	// 	Orignstr+=str;
	// 	(GetDlgItem(IDC_POINTEDIT))->SetWindowText(Orignstr);
	// 	Sleep(10);

}


void CdlgProgress::ShowClientUpdataInfo(SOCKET sockInt,TCHAR *szUpdataFile)
{
	DWORD dwBytes = 0;;
	HANDLE hFile = CreateFile(m_strUpdataFile, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );

	SetFilePointer(hFile,0,0,FILE_END);


	BYTE request[256] = {0};
	//request[0] = GET_CLIENTUPDATAINFO;

	request[0] = DOWN_FILE;

	StrCpyA((char*)(&request[1]),szUpdataFile);

	send( sockInt, (char*)&request, sizeof(request),NULL);

	
	do 
	{
	//	send( sockInt, (char*)&request, sizeof(request),NULL);

		//接收文件的大小
		DWORD dwFileSize=0;
		recv( sockInt,(char*)&dwFileSize,sizeof(DWORD),NULL);

		char *RecvData=new char[RECV_MAX_BUF];
		DWORD recvsize = 0;

		while (TRUE)
		{
			memset(RecvData, 0,RECV_MAX_BUF);

			int dwRetSize = recv( sockInt,RecvData,RECV_MAX_BUF,NULL);
			if ( dwRetSize == 0 || dwRetSize == SOCKET_ERROR )
			{
				break;
			}

			char *pData=new char[dwRetSize];
			memset(pData, 0,dwFileSize);
			StrNCpy(pData,RecvData,strlen(RecvData));
			MySetWindowText(pData);
			delete pData;

			recvsize += dwRetSize;

			if ( recvsize>=dwFileSize ) 
			{

				WriteFile(hFile,RecvData,dwRetSize,&dwBytes, NULL);				
				break;
			}
			WriteFile(hFile,RecvData,dwRetSize,&dwBytes,NULL);
			//		send( sockInt, laji, sizeof(laji),NULL);
		}

	} while (FALSE);


// 	DWORD dwFileSize = GetFileSize(hFile, NULL);
// 	BYTE *pData=new BYTE[dwFileSize];
// 	memset(pData, 0,dwFileSize);
// 	DWORD dwRead;

	//SetFilePointer(hFile,0,0,FILE_BEGIN);

	//ReadFile(hFile,pData,dwFileSize,&dwRead,NULL);
	SAFE_CLOSEHANDLE(hFile);

	//::SetWindowText(::GetDlgItem(this->m_hWnd,IDC_EDIT_UNDATAINFO),(LPCSTR)pData);
	//delete pData;

	//关闭文件
	//CloseHandle(hFile);

	return;
}


void CdlgProgress::OnDestroy()
{
	CDialog::OnDestroy();

	KillTimer(NULL);
	// TODO: 在此处添加消息处理程序代码
}
