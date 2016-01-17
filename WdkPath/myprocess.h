#if !defined PROCESS_h
#define PROCESS_h
//
//#include <windows.h>
//#include "afx.h"
/////////////////////////////////////////////////////////////////////////////
// CRegistry window


/////////////////////////////////////////////////////////////////////////////
// CRegKey

class CProcess
{
public:
	CProcess();

	CProcess(LPCTSTR lpszProcessName);

	~CProcess();

	// Attributes
private:

	DWORD     m_dwProcessID;



	// Operations
public:
	
	/************************************************************************/
	/* //��̬���� ����һ�������ùؼ���static�����������ú���������"����::����
	��"	��ʽ���ʣ��������ø����ʵ��������������ʵ�����Բ�����            */
	/************************************************************************/
	static DWORD WINAPI FindProcessID(LPCTSTR lpszProcessName);
	BOOL  KillProcess();
	static	DWORD WINAPI KillAllProcessByName(LPCTSTR lpszProcessName); 
	BOOL  RemoteLoadLibrary(IN LPCTSTR lpszDllName);
	BOOL  RemoteFreeLibrary(IN LPCTSTR lpszDllName);
	static BOOL  GetProcessFullPath(OUT LPTSTR szFilePath,DWORD dwProcessID);

};

/////////////////////////////////////////////////////////////////////////////

#endif 