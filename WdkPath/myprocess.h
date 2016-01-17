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
	/* //静态函数 若在一个类中用关键字static声明函数，该函数可以用"类名::函数
	名"	方式访问，无需引用该类的实例，甚至这个类的实例可以不存在            */
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