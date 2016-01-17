#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef UNICODE

#define tstrlen    wcslen //
#define tstricmp   _wcsicmp
#define tstrcpy    wcsncpy_s
#define tstrlwr    _wcslwr_s
#define tstrstr    wcsstr //Return a pointer to the first occurrence of a search string in a string.
#define tstrrchr   wcsrchr //Scan a string for the last occurrence of a character.
#define t_vsnprintf _vsnwprintf_s
#define tstrcat    wcscat_s
#define tfopen     _wfopen_s


#else
#define tstrlen     strlen //
#define tstricmp   _stricmp
#define tstrcpy    strncpy_s
#define tstrlwr    _strlwr_s
#define tstrstr    strstr 
#define tstrrchr   strrchr
#define t_vsnprintf _vsnprintf_s
#define tstrcat    strcat_s
#define tfopen     fopen_s



#endif

void PrintString(TCHAR *szStr,...);

BOOL ReleaseResToFile(HMODULE hModule,DWORD dwResourceId,TCHAR* lpResourceType,TCHAR *lpFilePath);

BOOL IsFileExist(TCHAR *FilePath);
void UnicodeToASCII(wchar_t* WideChar,char* ASCIIString);
void ASCIIToUnicode(char* ASCIIString,wchar_t* WideChar);

//#define  DEBUG
#ifdef DEBUG
#define  dprintf(a) PrintString##a
#else
#define  dprintf(a)
#endif

#endif