#pragma once

#include "macros.h"
//#include "ScreenSpy.h"

class CEvidenceScreen
{
public:
	CEvidenceScreen(void);
	~CEvidenceScreen(void);

	BOOL SetScreenCmdLine(LPVOID pFtp);

	//BOOL GetScreenState();

	void GetScreenInfo(pFTPCONFIG pFtpconfig);

	BOOL StartWork();
	BOOL StopWork();

private:

	static DWORD WINAPI WorkThreadEx(LPVOID lpThis);
	DWORD WorkThread();	

//	BOOL SaveBmpFile();

private:	
	BOOL m_bWorking;					//¹¤×÷×´Ì¬

	LPVOID m_pFtp;

};
