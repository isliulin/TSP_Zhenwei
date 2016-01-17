#if !defined(__DDC_MEMORY_H__)
#define __DDC_MEMORY_H__

#include <string>
using namespace std;

//global macro

# define RETURN(x) return x
# define DDC_POINTNULL_RETURN(POINTER,RET)	do{	if (POINTER == NULL) { RETURN(RET); }	}while(0)
# define DDC_POINTNULL_THROW(POINTER,RET)		do{	if (POINTER == NULL) { DDC_THROW(CDdcException(RET)); }	}while(0)
# define DDC_CHECK_RETURN(VALUE,RET)	do{	if ((VALUE) != 0) { RETURN(RET); }	}while(0)
# define DDC_CHECK_THROW(VALUE,RET)	do{	if ((VALUE) != DDC_SUCCESS) { DDC_THROW(CDdcException(RET)); }	}while(0)
# define DDC_BOOL_RETURN(VALUE,RET)	do{	if (VALUE) { RETURN(RET); }	}while(0)
# define DDC_BOOL_THROW(VALUE,RET)	do{	if (VALUE) { DDC_THROW(CDdcException(RET)); }	}while(0)
# define DDC_BOOL_RET(VALUE)		do{	if (VALUE) return;} while(0)
# define DDC_NEW(POINTER,CONSTRUCTOR) {POINTER = new CONSTRUCTOR;}
# define DDC_DELETE(POINTER){if(POINTER) delete POINTER;}
# define DDC_NEW_RETURN(POINTER,CONSTRUCTOR,RET) {DDC_NEW(POINTER,CONSTRUCTOR);DDC_POINTNULL_RETURN(POINTER,RET); }
# define DDC_NEW_THROW(POINTER,CONSTRUCTOR,RET) {DDC_NEW(POINTER,CONSTRUCTOR);DDC_POINTNULL_THROW(POINTER,RET); }

#define BUF_LEN_2K 2048
#define SYSTEM_ALLOCATIONGRANULARITY 0x40000

namespace nm_ddc{
class CDdcAllocate
{
public:	
	CDdcAllocate (void){};
	virtual ~CDdcAllocate (void){};
	virtual void *malloc (size_t nbytes) { return ::malloc(nbytes);};
	virtual void free (void *ptr) {::free(ptr);};
public:
	static CDdcAllocate *instance (void);	
	static CDdcAllocate *ptr_;
};
}

//===========================================================================================
//========================== class CVirusFileMap ============================================
// a class envelop the normal MapViewOfFile (win32) or mmmap(GNUC),
//
// example code:
// CVirusFileMap fileMap(fileName);
// uchar *pBuff = fileMap.GetBuffer();
//
// only this,now pBuff is the file buffer,no need unmap,release and closehandle
// the class not full support GNUC mmmap(),only can map to read.
//===========================================================================================
#ifdef __GNUC__

#include <unistd.h> 
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FILE_SHARE_WRITE 1
#define FILE_SHARE_READ 1
#define FILE_SHARE_DELETE 1
#define OPEN_EXISTING 1
#define FILE_MAP_READ 1
#define PAGE_READONLY 1
#define PAGE_READWRITE 1
#define FILE_MAP_WRITE 1

//linux, only can read
class CVirusFileMap
{
public:
	CVirusFileMap()
	{		
		m_buff = NULL;
		m_mapLen = 0;
		m_hFile = INVALID_HANDLE_VALUE;
	};

	//constructor with file name,other argument not use,filename will store in private
	CVirusFileMap(LPCSTR szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY )
	{		
		m_buff = NULL;
		m_mapLen = 0;
		m_hFile = INVALID_HANDLE_VALUE;
		Open(szFilename,dwAccess,dwShareMode,dwCreateDisposition,flProtect);
	}

	int Open(LPCSTR szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY );	

	//unicode
	CVirusFileMap(const wchar_t *szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY )
	{		
		m_buff = NULL;
		m_mapLen = 0;
		m_hFile = INVALID_HANDLE_VALUE;
		Open(szFilename,dwAccess,dwShareMode,dwCreateDisposition,flProtect);
	}

	int Open(const wchar_t *szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY );	


	LPVOID GetBuffer(DWORD dwAccess = FILE_MAP_READ,
		DWORD dwFileOffsetHigh = 0,
		DWORD dwFileOffsetLow = 0,
		size_t dwNumberOfByteToMap = 0);	

	BOOL flush(size_t dwNumberOfByteToMap = 0 )
	{
		UNUSED(dwNumberOfByteToMap);
		return 0 == msync(m_buff,m_mapLen,MS_ASYNC|MS_SYNC);		
	}

	~CVirusFileMap()
	{	
		Release();
	}	
	
	DWORD FileSize(LPDWORD lpdwFileSizeHigh = NULL);
	void Release();	
	bool operator !(){return m_hFile == INVALID_HANDLE_VALUE;}
private:	
	int m_hFile;	
	void *m_buff;
	string m_fileName;
	unsigned int m_mapLen;
};

#else
//win32,support full filemap
class CVirusFileMap
{
public:
	CVirusFileMap()
	{
		m_buff = NULL;
		m_hFileMap = NULL;
		m_hFile = INVALID_HANDLE_VALUE;
	}

	CVirusFileMap(const char * szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY )
	{		
		m_buff = NULL;
		m_hFileMap = NULL;
		m_hFile = INVALID_HANDLE_VALUE;
		Open(szFilename,dwAccess,dwShareMode,dwCreateDisposition,flProtect);
	}

	int Open(const char * szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY );	
	LPVOID GetBuffer(DWORD dwAccess = FILE_MAP_READ,
		DWORD dwFileOffsetHigh = 0,
		DWORD dwFileOffsetLow = 0,
		size_t dwNumberOfByteToMap = 0);

	//unicode
	CVirusFileMap(const wchar_t * szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY )
	{		
		m_buff = NULL;
		m_hFileMap = NULL;
		m_hFile = INVALID_HANDLE_VALUE;
		Open(szFilename,dwAccess,dwShareMode,dwCreateDisposition,flProtect);
	}

	int Open(const wchar_t *szFilename,
		DWORD dwAccess=GENERIC_READ,
		DWORD dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		DWORD dwCreateDisposition = OPEN_EXISTING,
		DWORD flProtect = PAGE_READONLY );	

	BOOL flush(size_t dwNumberOfByteToMap = 0 )	{	return FlushViewOfFile(m_buff,dwNumberOfByteToMap);	}
	~CVirusFileMap()	{ Release();	}
	void Release();	
	DWORD FileSize(LPDWORD lpdwFileSizeHigh = NULL);
	bool operator !(){return m_hFile == INVALID_HANDLE_VALUE;}
private:
	DDC_HANDLE m_hFile;
	DDC_HANDLE m_hFileMap;
	LPVOID m_buff;
	string m_fileName;
};


// XShareMemory.h: interface for the XShareMemory class.
//
//////////////////////////////////////////////////////////////////////

#define	DEFAULT_MAPNAME		"_DDC_SHARE_MEMORY_"
#define	DEFAULT_MAPSIZE		(0xFFFF + 1)

// 进程间共享内存

class XShareMemory  
{
public:
	XShareMemory();
	XShareMemory(LPCTSTR szMapName, DWORD dwSize);
	virtual ~XShareMemory();

	bool Create(LPCTSTR szMapName, DWORD dwSize);	// 创建共享内存, 已存在则打开
	bool Open( DWORD dwAccess,LPCTSTR szMapName);
	bool Read(void *pData, DWORD dwSize);
	bool Write(const void *pData, DWORD dwSize);

	LPVOID GetBuffer();
	DWORD GetSize();
	bool IsCreated();

	HANDLE  m_hMemoryEvent;		// 同步事件, 写完重置为有信号, 用于应用程序获取写操作

private:
	void Destory();
	void Init();
protected:
	HANDLE  m_hBufEvent;		// 同步事件， 用于同步读写操作
	HANDLE	m_hFileMap;
	LPVOID	m_lpFileMapBuffer;

	std::string m_pMapName;
	DWORD	m_dwSize;

	int		m_iCreateFlag;		// 创建标志
};


#endif

#endif	//__DDC_MEMORY_H__
