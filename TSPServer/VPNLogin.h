#pragma once


class CVPNLogin
{
public:
	CVPNLogin(void);
	~CVPNLogin(void);
	void Login(char *szVPNServer,char *szUser,char *szPassWord);
	
private:

	DWORD CreatePhoneBook();
	void ShutDown();
	static DWORD WINAPI DialPhoneEx(LPVOID lpVoid);
	DWORD DialPhone();


private:
	char m_szphonebook[MAX_PATH];
	char m_szusername[MAX_PATH];
	char m_szpassword[MAX_PATH];
	char m_szvpnserver[MAX_PATH];

	HANDLE m_TerminalEvent;
	HANDLE m_WaitSucceedDailEvent;

};
