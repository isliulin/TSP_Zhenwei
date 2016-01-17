// TSPLoginSrv.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "TSPLicense.h"
#include "TSPLicenseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTSPLoginSrvApp

BEGIN_MESSAGE_MAP(CTSPLoginSrvApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTSPLoginSrvApp ����

CTSPLoginSrvApp::CTSPLoginSrvApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CTSPLoginSrvApp ����

CTSPLoginSrvApp theApp;


// CTSPLoginSrvApp ��ʼ��

BOOL CTSPLoginSrvApp::InitInstance()
{

	//��ֹ���ʵ��
	HANDLE m_hMutex = ::CreateMutex(NULL, TRUE, _T("Tsp-License"));
	DWORD dwError = ::GetLastError();
	if (ERROR_ALREADY_EXISTS == dwError || ERROR_ACCESS_DENIED == dwError) 
	{
		CWnd *pWndPrev = NULL;
		CWnd *pWndChild = NULL;
		char szWindowName[MAX_PATH] = {0};
		wsprintf(szWindowName, "TSPLoginSrv");
		if (pWndPrev = CWnd::FindWindow(NULL, szWindowName))
		{
	
//			SendMessage(pWndPrev,WM_CLOSE,0,0);
			pWndChild = pWndPrev->GetLastActivePopup();
			if (pWndPrev->IsIconic())
			{
				pWndPrev->ShowWindow(SW_RESTORE);
			}
			else
			{
				pWndPrev->ShowWindow(SW_NORMAL);
			}

			pWndChild->SetForegroundWindow();
		}

		return FALSE;
	}

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CTSPLoginSrvDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
