
#include "ddc_util.h"
#include "ddc_memory.h"


namespace nm_ddc{
// Process-wide Thread Manager.
CDdcAllocate *CDdcAllocate::ptr_ = 0;
CDdcAllocate *CDdcAllocate::instance (void)
{
	if (CDdcAllocate::ptr_ == 0)
	{
		if (CDdcAllocate::ptr_ == 0)
		{
			DDC_NEW_RETURN(CDdcAllocate::ptr_,CDdcAllocate,0);
		}
	}

	return CDdcAllocate::ptr_;
}
};		// nm_ddc

/////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
int CVirusFileMap::Open(LPCSTR szFilename,DWORD dwAccess,DWORD dwShareMode,DWORD dwCreateDisposition,DWORD flProtect)
{
	if( strcmp(m_fileName.c_str(),szFilename) )
	{
		m_fileName.assign(szFilename);
		Release();

		struct stat file_stats;
		if( stat(szFilename,&file_stats))
			return 0;

		m_hFile = open(szFilename,O_RDONLY , S_IRUSR | S_IWUSR);
	}
	return 0;
}
	
int CVirusFileMap::Open(const wchar_t *szFilenameW,DWORD dwAccess,DWORD dwShareMode,DWORD dwCreateDisposition,DWORD flProtect)
{
	char szFilename[MAX_PATH];
	wcstombs(szFilename,szFilenameW,MAX_PATH);

	if( strcmp(m_fileName.c_str(),szFilename) )
	{
		m_fileName.assign(szFilename);
		Release();

		//don't create file
		struct stat file_stats;
		if( stat(szFilename,&file_stats))
			return 0;

		m_hFile = open(szFilename,O_RDONLY , S_IRUSR | S_IWUSR);
	}
	return 0;
}

LPVOID CVirusFileMap::GetBuffer(DWORD dwAccess,DWORD dwFileOffsetHigh,DWORD dwFileOffsetLow,size_t dwNumberOfByteToMap)
{	
	if( m_hFile != INVALID_HANDLE_VALUE )
	{			
		if( m_buff )
			munmap( m_buff, m_mapLen);			
		m_mapLen = dwNumberOfByteToMap==0?FileSize():dwNumberOfByteToMap;
		m_buff = (uchar*)mmap(NULL,m_mapLen,PROT_READ, MAP_PRIVATE, m_hFile, dwFileOffsetLow);
		if( MAP_FAILED == m_buff )
			m_buff = NULL;			
	}
	return m_buff;
}



DWORD CVirusFileMap::FileSize(LPDWORD lpdwFileSizeHigh)
{
	
	if(m_hFile ==  INVALID_HANDLE_VALUE )
		return 0;
	
	struct stat sb;
	fstat(m_hFile, &sb);				
	return sb.st_size;		
}

void CVirusFileMap::Release()
{
	if( m_buff )
		munmap(m_buff,m_mapLen);
	if( m_hFile )
		close(m_hFile);
	
	m_buff = NULL;
	m_mapLen = 0;
	m_hFile = INVALID_HANDLE_VALUE;
}
#else
//win32
int CVirusFileMap::Open(const char *szFilename,DWORD dwAccess,DWORD dwShareMode,DWORD dwCreateDisposition,DWORD flProtect)
{
	if( strcmp(m_fileName.c_str(),szFilename) )
	{
		m_fileName.assign(szFilename);
		Release();
		m_hFile = CreateFile(szFilename,
			dwAccess,
			dwShareMode,
			NULL,
			dwCreateDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if( m_hFile != INVALID_HANDLE_VALUE )
			m_hFileMap = CreateFileMapping(m_hFile, NULL,flProtect , 0, 0, NULL);
	}
	return 0;
}
int CVirusFileMap::Open(const wchar_t *szFilename,DWORD dwAccess,DWORD dwShareMode,DWORD dwCreateDisposition,DWORD flProtect)
{
	char szFileNameA[MAX_PATH];
	WideCharToMultiByte(MP_CP_CN,0,szFilename,-1,szFileNameA,MAX_PATH,0,0);

	if( strcmp(m_fileName.c_str(),szFileNameA) )
	{
		m_fileName.assign(szFileNameA);
		Release();
		m_hFile = CreateFileW(szFilename,
			dwAccess,
			dwShareMode,
			NULL,
			dwCreateDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if( m_hFile != INVALID_HANDLE_VALUE )
			m_hFileMap = CreateFileMapping(m_hFile, NULL,flProtect , 0, 0, NULL);
	}
	return 0;
}

LPVOID CVirusFileMap::GetBuffer(DWORD dwAccess,
				 DWORD dwFileOffsetHigh,
				 DWORD dwFileOffsetLow,
				 size_t dwNumberOfByteToMap)
{		
	if( m_hFileMap )
	{
		if( m_buff )
			UnmapViewOfFile(m_buff);
		m_buff = MapViewOfFile(m_hFileMap, dwAccess,dwFileOffsetHigh,dwFileOffsetLow,dwNumberOfByteToMap);		
	}
	
	return m_buff;
}

void CVirusFileMap::Release()
{
	if( m_buff )
		UnmapViewOfFile(m_buff);
	if( m_hFileMap != NULL )
		CloseHandle(m_hFileMap);
	if( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle(m_hFile);
	
	m_buff = NULL;
	m_hFileMap = NULL;
	m_hFile = INVALID_HANDLE_VALUE;
}

DWORD CVirusFileMap::FileSize(LPDWORD lpdwFileSizeHigh)
{
	if( m_hFile != INVALID_HANDLE_VALUE )
		return GetFileSize(m_hFile,lpdwFileSizeHigh);
	return 0;
}



//////////////////////////////////////////////////////////////////////
//	XShareMemory class
//////////////////////////////////////////////////////////////////////

XShareMemory::XShareMemory()
{
	Init();
	//Create(DEFAULT_MAPNAME, DEFAULT_MAPSIZE);
}

XShareMemory::XShareMemory(LPCTSTR szMapName, DWORD dwSize)
{
	Init();
	Create(szMapName, dwSize);
}

XShareMemory::~XShareMemory()
{
	Destory();
}

void XShareMemory::Init()
{
	m_hFileMap = NULL;
	m_hMemoryEvent = NULL;
	m_hBufEvent = NULL;
	m_lpFileMapBuffer = NULL;	
	m_dwSize = 0;
	m_iCreateFlag = 0;
}

void XShareMemory::Destory()
{
	if (m_lpFileMapBuffer)
	{
		UnmapViewOfFile(m_lpFileMapBuffer);
		m_lpFileMapBuffer = NULL;
	}

	if (m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if(m_hMemoryEvent)
	{
		CloseHandle(m_hMemoryEvent);
		m_hMemoryEvent = NULL;
	}

	if (m_hBufEvent)
	{
		CloseHandle(m_hBufEvent);
		m_hBufEvent = NULL;
	}

	Init();
}

bool XShareMemory::Create( LPCTSTR szMapName, DWORD dwSize)
{
	if (m_iCreateFlag)
		Destory();

	char szEventName[MAX_PATH];
	strcpy(szEventName, szMapName);
	strcat(szEventName, "_EVENT");
	m_hMemoryEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);
	m_hBufEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	if (szMapName)
		m_pMapName.assign(szMapName);
	else
		m_pMapName.assign(DEFAULT_MAPNAME);

	if (dwSize > 0)
		m_dwSize = dwSize;
	else 
		m_dwSize = DEFAULT_MAPSIZE;


	m_hFileMap = CreateFileMapping(
		(HANDLE)INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		m_dwSize,
		m_pMapName.c_str()
		);

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,//FILE_MAP_WRITE|FILE_MAP_READ,
			0,
			0,
			m_dwSize
			);
	}

	if (m_lpFileMapBuffer == NULL)
		return false;

	m_iCreateFlag = 1;
	return true;
}


bool XShareMemory::Open( DWORD dwAccess,LPCTSTR szMapName)
{
	if (m_iCreateFlag)
		Destory();

	//wprintf(L"==%d, %s==/n", dwSize, szMapName);

	char szEventName[MAX_PATH];
	strcpy(szEventName, szMapName);
	strcat(szEventName, "_EVENT");
	m_hMemoryEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);
	m_hBufEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	if (szMapName)
		m_pMapName.assign(szMapName);
	else
		m_pMapName.assign(DEFAULT_MAPNAME);

	m_hFileMap = OpenFileMapping(
		dwAccess,
		TRUE,		
		m_pMapName.c_str()
		);

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(
			m_hFileMap,
			dwAccess,
			0,
			0,
			m_dwSize
			);
	}

	if (m_lpFileMapBuffer == NULL)
		return false;

	m_iCreateFlag = 1;
	return true;
}

bool XShareMemory::IsCreated()
{
	return (m_iCreateFlag == 1)? true : false;
}

LPVOID XShareMemory::GetBuffer()
{
	return (m_lpFileMapBuffer)?(m_lpFileMapBuffer):(NULL);
}

DWORD XShareMemory::GetSize()
{
	return m_dwSize;
}

bool XShareMemory::Write(const void *pData, DWORD dwSize)
{
	void *p = (void*)GetBuffer();
	if(p)
	{
		::WaitForSingleObject(m_hBufEvent, INFINITE);
		memcpy(p, pData, dwSize);
		SetEvent(m_hBufEvent);
		SetEvent(m_hMemoryEvent);
		return true;
	}
	else
		return false;
}

bool XShareMemory::Read(void *pData, DWORD dwSize)
{
	void *p = (void*)GetBuffer();
	if(!p)
		return false;

	::WaitForSingleObject(m_hBufEvent, INFINITE);
	memcpy(pData, p, min(dwSize, m_dwSize));
	SetEvent(m_hBufEvent);

	return true;
}



#endif
