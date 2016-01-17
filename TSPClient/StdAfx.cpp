// stdafx.cpp : source file that includes just the standard includes
//	TSPClient.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


char * GetExeCurrentDir()
{
//	CString strCurPath;
	static char szFullPath[MAX_PATH] = {0};
	//TCHAR szDir[_MAX_DIR] = {0};
	//TCHAR szDrive[_MAX_DRIVE] = {0};
	::GetModuleFileName(NULL, szFullPath, MAX_PATH);
	char *p=strrchr(szFullPath,'\\');
	if(p)
	{
		*p=0;
	}
	
	//_tsplitpath_s(szFullPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, NULL, NULL, NULL);
	//strCurPath.Format(_T("%s%s"), szDrive, szDir);

	return szFullPath;
}
void PrintString1(TCHAR *tszStr,...)
{
	TCHAR szFormat[MAX_PATH*2] = {0};

	va_list argList;
	va_start(argList,tszStr);
	_vsntprintf_s(szFormat, MAX_PATH*2-1,tszStr, argList );
	va_end(argList);
	OutputDebugString(szFormat);
}