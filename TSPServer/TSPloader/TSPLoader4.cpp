#include "stdafx.h"
#include "windows.h"
#include "TCHAR.h"
#include "Tlhelp32.h"
#include "commctrl.h"
#include "shlwapi.h"
#include <Shlobj.h>
#include "objidl.h "

#include "Iphlpapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")




//��֤��������һ��������ϵͳ������������ܶ�����


HRESULT CreateShortcut(LPCSTR pszPathObj,LPSTR pszParam,LPSTR pszPath,LPSTR pszPathLink,LPSTR pszDesc)
{
	HRESULT hres; //���� COM �ӿڷ���֮��ķ���ֵ
	IShellLink *pShellLink;
	IPersistFile *pPersistFile;
	WCHAR wsz[MAX_PATH]; //UNICODE��, ������ſ�ݷ�ʽ�ļ���

	CoInitialize(NULL); //��ʼ�� COM ��
	//���� COM ���󲢻�ȡ��ʵ�ֵĽӿ�
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&pShellLink);
	if(FAILED(hres))
	{
		pShellLink ->Release(); //�ͷ� IShellLink �ӿ�
		CoUninitialize(); //�ر� COM ��, �ͷ����� COM ��Դ
		return FALSE;
	}
	//���ÿ�ݷ�ʽ�ĸ�������
	pShellLink->SetPath(pszPathObj); //��ݷ�ʽ��ָ��Ӧ�ó�����
	pShellLink->SetArguments(pszParam); //����
	pShellLink->SetDescription(pszDesc); //����
	pShellLink->SetWorkingDirectory(pszPath); //���ù���Ŀ¼
	//pShellLink->SetIconLocation("C:\\Icon.ico",0); //��ݷ�ʽ��ͼ��
	//pShellLink->SetHotkey(�ȼ�); //������ݷ�ʽ���ȼ�(ֻ����Ctrl+Alt+_)
	//pShellLink->SetShowCmd(SW_MAXIMIZE); //���з�ʽ(���洰��,���,��С��)
	//��ѯ IShellLink �ӿڴӶ��õ� IPersistFile �ӿ��������ݷ�ʽ
	hres = pShellLink->QueryInterface(IID_IPersistFile,(void **)&pPersistFile);
	if(FAILED(hres))
	{
		pPersistFile ->Release(); //�ͷ� IPersistFile �ӿ�
		pShellLink ->Release(); //�ͷ� IShellLink �ӿ�
		CoUninitialize(); //�ر� COM ��, �ͷ����� COM ��Դ
		return(FALSE);
	}
	//ת�� ANSI ��Ϊ UNICODE ��(COM �ڲ�ʹ�� NUICODE)
	MultiByteToWideChar(CP_ACP, 0, pszPathLink, -1, wsz, MAX_PATH);
	//ʹ�� IPersistFile �ӿڵ� Save() ���������ݷ�ʽ
	hres = pPersistFile ->Save(wsz, TRUE);

	//�ͷ� IPersistFile �ӿ�
	pPersistFile ->Release();
	//�ͷ� IShellLink �ӿ�
	pShellLink ->Release();
	//�ر� COM ��, �ͷ����� COM ��Դ
	CoUninitialize();
	return(hres);
}



void  MoveLinkToStartup4(char *szSrcExeName)
{

	//c:\\Documents and Settings\\All Users\\����ʼ���˵�\\����\\����\\";

	TCHAR szLink[MAX_PATH];
	StrCpyA(szLink,szSrcExeName);

	*(StrRChr(szLink,0,'.'))=0;

	StrCatA(szLink,".lnk");

	CreateShortcut(szSrcExeName,0,0,szLink,0);
	TCHAR szStartLink[MAX_PATH];
 	SHGetSpecialFolderPath(NULL,szStartLink,CSIDL_COMMON_STARTUP,0);
 	StrCatA(szStartLink,StrRChr(szLink,0,'\\'));
	DeleteFile(szStartLink);
	MoveFileEx(szLink,szStartLink,MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING);
	return ;
}

