// COMMON.h: interface for the COMMON class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMON_H__C64953B4_0BDE_4413_BF48_B03069296E42__INCLUDED_)
#define AFX_COMMON_H__C64953B4_0BDE_4413_BF48_B03069296E42__INCLUDED_

#include "WINSOCK.H"

char *DAT_NAME = "Server.dat";
char *INI_NAME = "Config.ini";
char *CLIENT_NAME = "TSPClient.exe";
char *CLIENT_UPDATAINFO_NAME = "ClientUpdataInfo.txt";
char DAT_PATH[MAX_PATH] = {0};
char INI_PATH[MAX_PATH] = {0};
char CLIENT_PATH[MAX_PATH] = {0};
//char CLIENT_UPDATAINFO_PATH[MAX_PATH] = {0};

SOCKET m_hSocket;
HANDLE hLinstenThread = NULL;




#endif // !defined(AFX_COMMON_H__C64953B4_0BDE_4413_BF48_B03069296E42__INCLUDED_)
