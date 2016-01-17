#if !defined(DDC_OS_LINUX_H__)
#define DDC_OS_LINUX_H__

#include "pthread.h"
#include "ctype.h"
#include "wchar.h"
#include "unistd.h"
#include "string.h"
#include "signal.h"
#include "errno.h"
#include "ctype.h"


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define _ASSERT assert
//#define min(x,y) ((x)<(y)?(x):(y))
#define _strdup(x) strdup(x)

#define _stricmp(x,y) strcmp(x,y)
#define memicmp(x,y,z) memcmp(x,y,z)

typedef bool BOOL;
typedef int __int32;
typedef long long __int64;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int UINT32;
typedef int INT;
typedef long LONG;
typedef long * PLONG;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef bool BOOLEAN;
typedef long LARGE_INTEGER;
typedef long long DDC_HANDLE;
typedef void * PVOID;
typedef void * LPVOID;
typedef const void * LPCVOID;
typedef void VOID;
typedef const char * LPCSTR;
typedef unsigned long * LPDWORD;
typedef unsigned long * PDWORD;
typedef unsigned short * PWORD;
typedef unsigned short * PWORD;
typedef char TCHAR;
typedef char *PTCHAR;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef wchar_t *PWCHAR;
typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef unsigned char *PBYTE;
typedef const unsigned char *LPBYTE;
typedef const char * LPCTSTR;
typedef const wchar_t *LPCWSTR;
typedef wchar_t *LPWSTR;
typedef char *LPSTR;
typedef char *LPTSTR;
typedef char *PSTR;
typedef char *PCHAR;
typedef unsigned char *PUCHAR;
typedef wchar_t *PCWSTR;
typedef BOOL * LPBOOL;

#define FIELD_OFFSET(type, field)    ((long)(int*)&(((type *)0)->field))
#define MOVEFILE_REPLACE_EXISTING 0x00000001
#define ZeroMemory(BUFF,LEN) memset((BUFF),0,(LEN))

#define IN
#define OUT
#define WINAPI
#define WINBASEAPI
#define APIENTRY WINAPI
#define __cdecl

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#define __declspec(dllexport)
#define __descspec(dllimport)

#define __stdcall
#define _stdcall
#define MAX_PATH 260

#define DDC_C_SLASH '/'
#define DDC_S_SLASH "/"
#define DDC_WC_SLASH L'/'

#define MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) 0
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 0

#define MPV_MM_HANDLE DDC_HANDLE
#define MPV_FILE_HANDLE DDC_HANDLE
#define MPV_MAP_HANDLE DDC_HANDLE
#define HINSTANCE void *
#define HMODULE HINSTANCE

#define LPSECURITY_ATTRIBUTES void *
#define LPOVERLAPPED void *
#define INFINITE 0

#define THREAD_PRIORITY_LOWEST 1
#define THREAD_PRIORITY_BELOW_NORMAL 3
#define THREAD_PRIORITY_NORMAL 7
#define THREAD_PRIORITY_ABOVE_NORMAL 10
#define THREAD_PRIORITY_TIME_CRITICAL 15

#define wcsicmp wcscasecmp
inline wchar_t towlower(wchar_t c){char c1=wctob(c);c1=tolower(c1);return btowc(c1);}
#define SetFileAttributesA
#define _splitpath

/////////////////////
#define STILL_ACTIVE 0
inline DDC_HANDLE WaitForEvent(int count,DDC_HANDLE handles[],int timeout);
inline bool SetFileAttributesW(const wchar_t *,int){return true;};
inline int _snwprintf(wchar_t *,int,const wchar_t *,...){return 0;};
inline int GetFileAttributes(const char *){return 0;};
///////////////////


//PE struct
typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
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
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;

    //
    // NT additional fields.
    //

    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef PIMAGE_NT_HEADERS32                 PIMAGE_NT_HEADERS;


#ifdef _BIG_ENDIAN
#define IMAGE_DOS_SIGNATURE 0x4d5a
#define IMAGE_NT_SIGNATURE 0x50450000
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x0b01
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x0b02
#else
#define IMAGE_DOS_SIGNATURE 0x5a4d
#define IMAGE_NT_SIGNATURE 0x00004550
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#endif



#define CP_ACP 0
int
WINAPI
MultiByteToWideChar(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    OUT LPWSTR  lpWideCharStr,
    IN int      cchWideChar);
int
WINAPI
WideCharToMultiByte(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCWSTR  lpWideCharStr,
    IN int      cchWideChar,
    OUT LPSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    IN LPCSTR   lpDefaultChar,
    OUT LPBOOL  lpUsedDefaultChar);

unsigned long GetModuleFileName(HMODULE,char *,unsigned long);
unsigned long GetModuleFileNameW(HMODULE,wchar_t *,unsigned long);



DWORD GetTickCount();
DWORD GetLastError();

#define _snprintf snprintf

////////////////////////////////////////////////
//memory 
#define MEM_COMMIT 0x1000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 1
#define SIZE_T size_t
LPVOID
WINAPI
VirtualAlloc(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect
    );

BOOL
WINAPI
VirtualFree(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    );

DDC_HANDLE
WINAPI
CreateFileMapping(
    IN DDC_HANDLE hFile,
    IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    IN DWORD flProtect,
    IN DWORD dwMaximumSizeHigh,
    IN DWORD dwMaximumSizeLow,
    IN LPCSTR lpName
    );

LPVOID
WINAPI
MapViewOfFile(
    IN DDC_HANDLE hFileMappingObject,
    IN DWORD dwDesiredAccess,
    IN DWORD dwFileOffsetHigh,
    IN DWORD dwFileOffsetLow,
    IN SIZE_T dwNumberOfBytesToMap
    );

BOOL
WINAPI
UnmapViewOfFile(
    IN LPCVOID lpBaseAddress
    );

typedef int (*FARPROC)();

HMODULE
WINAPI
LoadLibrary(
    IN LPCSTR lpLibFileName
    );

WINBASEAPI
FARPROC
WINAPI
GetProcAddress(
    IN HMODULE hModule,
    IN LPCSTR lpProcName
    );

//file
#define FILE_BEGIN SEEK_SET
#define FILE_ATTRIBUTE_NORMAL 0x00000000
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)
#define INVALID_HANDLE_VALUE ((DDC_HANDLE)(PLONG)-1)

DWORD
WINAPI
SetFilePointer(
    IN DDC_HANDLE hFile,
    IN LONG lDistanceToMove,
    IN PLONG lpDistanceToMoveHigh,
    IN DWORD dwMoveMethod
    );

BOOL
WINAPI
ReadFile(
    IN DDC_HANDLE hFile,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfBytesToRead,
    OUT LPDWORD lpNumberOfBytesRead,
    IN LPOVERLAPPED lpOverlapped
    );

DDC_HANDLE
WINAPI
CreateFile(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN DDC_HANDLE hTemplateFile
    );

DWORD
WINAPI
GetFileSize_MM(
    IN DDC_HANDLE hFile,
    OUT LPDWORD lpFileSizeHigh
    );

DDC_HANDLE
WINAPI
CreateFile_MM(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN DDC_HANDLE hTemplateFile
    );

DDC_HANDLE
WINAPI
CreateFileW(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN DDC_HANDLE hTemplateFile
    );

DDC_HANDLE
WINAPI
CreateFileW_MM(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN DDC_HANDLE hTemplateFile
    );

BOOL
CloseHandle(
    IN OUT DDC_HANDLE hObject
    );

BOOL
CloseHandle_File(
    IN OUT DDC_HANDLE hObject
    );

DWORD
WINAPI
GetFileSize(
    IN DDC_HANDLE hFile,
    OUT LPDWORD lpFileSizeHigh
    );


int DeleteFile(const char *);
int DeleteFileW(const wchar_t *);
int CreateDirectory(const char *,void *);
int CreateDirectoryW(const wchar_t *,void *);
int ClearDirectory(const char *);
int RemoveDirectory(const char *);

inline DWORD GetTempPathW(DWORD /*buflen*/,wchar_t *buff)
{
	wcscpy(buff,L"/tmp/");
	return 0;
}

// Move file
// parameter:
//      const char *source file name
//      const char *dest file name
// return value:
//      0       success
//      -1      file not exist
//      other   access denied
// *****************************************************
extern int MoveFile(const char *,const char *);
extern int MoveFileEx(const char *,const char *,int);

// *****************************************************
// file access mode
// parameter:
//      const char *    file full pathname
//      int     mode(not use)
// return value:
//      -1      argument not allowed
//      0       success
// *****************************************************
extern int _access(const char *,int);
int _waccess(const wchar_t *,int);

// *****************************************************
// copy file    
// parameter:
//      char *  source file name
//      char *  dest file name
//      bool    overwrite
// return value:
//      0       success
// *****************************************************
extern int CopyFile(const char *,const char *,bool);
extern int CopyFileW(const wchar_t *,const wchar_t *,bool);

//ini file
void GetPrivateProfileString(   const char      *szIndex,
                                const char      *szKey,
                                const char      *pDefault,
                                char    *szBuffer,
                                int     nLength,
                                const char      *szFilePath);

void WritePrivateProfileString( const char      *szIndex,
                                const char      *szKey,
                                const char      *UpdateList,
                                const char      *szFileName);

int GetPrivateProfileInt(       const char      *szIndex,
                                const char      *szKey,
                                int     nDefault,
                                const char      *szFilePath);

void WritePrivateProfileInt(    const char      *szIndex,
                                const char      *szKey,
                                const char      *szFilePath,
                                int     nValue);


typedef void *(*THR_FUNC)(void *);
int thr_create( THR_FUNC func,void *arg,long flags,int *t_id,DDC_HANDLE *t_handle,long priority,void *stack,size_t stack_size);

/*
inline int thr_suspend(DDC_HANDLE handle)
{
	if (pthread_pause == false)
	{
		pthread_mutex_lock( &wqy_mutex_pause );
		pthread_pause = true;
		pthread_mutex_unlock( &wqy_mutex_pause );    
	}
	return 0;
}
inline int thr_resume(DDC_HANDLE handle)
{ 
    if (pthread_pause == true)
    {
        pthread_mutex_lock(&wqy_mutex_pause);
        wqy_pthread_pause = false;
        pthread_cond_broadcast(&wqy_cond_pause);
        pthread_mutex_unlock(&wqy_mutex_pause);    
    }
	return 0;
} 

inline int pthread_pause_location(void)
{
    pthread_mutex_lock(&wqy_mutex_pause);
    while(wqy_pthread_pause)
    {
        pthread_cond_wait(&wqy_cond_pause, &wqy_mutex_pause);
    }
    pthread_mutex_unlock(&wqy_mutex_pause); 
} 

inline int thr_wait(DDC_HANDLE handle,int timeout){ ::wait(handle,timeout); return 0;}
inline int thr_setprio(DDC_HANDLE handle,int priority,int policy){ ::pthread_setprio(handle,priority); return 0;}
*/

int thr_kill (DDC_HANDLE handle, int signum);
inline int thr_suspend (DDC_HANDLE /*handle*/){  return 0;}
inline int thr_resume(DDC_HANDLE /*handle*/){ return 0;}
inline int thr_wait(DDC_HANDLE handle,int /*timeout*/){ ::pthread_join(handle,NULL);return 0;}
inline int thr_setprio(DDC_HANDLE /*handle*/,int /*priority*/,int /*policy*/){ return 0;}
inline int thr_sleep(int timeout)
{
	return usleep(timeout*1000);
}



#endif	//DDC_OS_H__

