// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Buffer.h"
#include "Math.h"
#include "Log.h"
extern CLog g_log;
extern CLog g_logErr;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CReadWriteLock g_AllocMemoryLock;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CBuffer
// 
// DESCRIPTION:	Constructs the buffer with a default size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
CBuffer::CBuffer()
{
	// Initial size
	m_nSize = 0;
	m_pPtr = m_pBase = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	~CBuffer
// 
// DESCRIPTION:	Deallocates the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
CBuffer::~CBuffer()
{
	g_AllocMemoryLock.WaitToWrite();
	if (m_pBase)
	{
		delete m_pBase;
		//VirtualFree(m_pBase,0,MEM_RELEASE);
		m_pBase=NULL;
	}
	g_AllocMemoryLock.Done();
}
	

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Write
// 
// DESCRIPTION:	Writes data into the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(PBYTE pData, UINT nSize)
{


	//	m_lock.WaitToWrite();
		if(!ReAllocateBuffer(nSize + GetBufferLen()))
		{
	//		m_lock.Done();
			return FALSE;
		}
		if (m_pPtr&&pData)
		{
			CopyMemory(m_pPtr,pData,nSize);
			// Advance Pointer
			m_pPtr+=nSize;
		}


//	m_lock.Done();
	return nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Insert
// 
// DESCRIPTION:	Insert data into the buffer 
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(PBYTE pData, UINT nSize)
{


	//	m_lock.WaitToWrite();
		if(!ReAllocateBuffer(nSize + GetBufferLen()))
		{
	//		m_lock.Done();
			return FALSE;
		}

			MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);
			CopyMemory(m_pBase,pData,nSize);
			m_pPtr+=nSize;

	// Advance Pointer

//	m_lock.Done();
	return nSize;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Read
// 
// DESCRIPTION:	Reads data from the buffer and deletes what it reads
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Read(PBYTE pData, UINT nSize)
{

	//	m_lock.WaitToWrite();
		// Trying to byte off more than ya can chew - eh?
		if (nSize)
		{
			if (pData&&m_pBase)
			{
				// Copy over required amount and its not up to us
				// to terminate the buffer - got that!!!

				if (nSize > GetMemSize())
				{
	//				m_lock.Done();
					return 0;
				}

				// all that we have 
				if (nSize > GetBufferLen())
					nSize = GetBufferLen();
				CopyMemory(pData,m_pBase,nSize);
				// Slide the buffer back - like sinking the data
				MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);
				m_pPtr -= nSize;

			}
		}

		DeAllocateBuffer(GetBufferLen());



	//m_lock.Done();
	return nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetMemSize
// 
// DESCRIPTION:	Returns the phyical memory allocated to the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetMemSize() 
{
	__try{
	//	m_lock.WaitToRead();
		return m_nSize;
	}
	__finally{
	//	m_lock.Done();
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBufferLen
// 
// DESCRIPTION:	Get the buffer 'data' length
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetBufferLen() 
{
	if (m_pBase == NULL)
		return 0;



		int nSize = m_pPtr - m_pBase;
		if(nSize<0)
		{
			m_pPtr=m_pBase;
			nSize=0;
		}

		return nSize;


	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	ReAllocateBuffer
// 
// DESCRIPTION:	ReAllocateBuffer the Buffer to the requested size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if(nRequestedSize<=0)
	{
		g_log.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 分配内存参数错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
		g_logErr.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 分配内存参数错误！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError());
		return 0;
	}
	if (nRequestedSize < GetMemSize())
	{
		return GetMemSize();
	}

	// Allocate new size
	
//	UINT nNewSize = (UINT) ceil(nRequestedSize / 1024.0) * 1024;
	UINT nNewSize =nRequestedSize;
	// New Copy Data Over
	g_AllocMemoryLock.WaitToWrite();

	//PBYTE pNewBuffer = (PBYTE) VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);
	PBYTE pNewBuffer = (PBYTE) new BYTE[nNewSize+1];

	g_AllocMemoryLock.Done();
	if(pNewBuffer==NULL)
	{
		g_log.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 没有足够的内存(%08x,%08x)！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError(),nNewSize,nRequestedSize);
		g_logErr.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 没有足够的内存(%08x,%08x)！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError(),nNewSize,nRequestedSize);
		return 0;
	}
	UINT nBufferLen = GetBufferLen();
	if(m_pBase)
	{

			if(nBufferLen>0)
			CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	}


	g_AllocMemoryLock.WaitToWrite();
//	if (m_pBase)VirtualFree(m_pBase,0,MEM_RELEASE);
	if (m_pBase)
	{
		delete m_pBase;
		//m_pBase = NULL;   //add by zhenyu
	}

	g_AllocMemoryLock.Done();


	// Hand over the pointer
	m_pBase = pNewBuffer;
	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;
	m_nSize = nNewSize;

	//delete[]pNewBuffer;

	return m_nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	DeAllocateBuffer
// 
// DESCRIPTION:	DeAllocates the Buffer to the requested size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
		return 0;

	// Allocate new size
//	UINT nNewSize = (UINT) ceil(nRequestedSize / 1024.0) * 1024;
	UINT nNewSize = nRequestedSize;

	//if(nNewSize<=0)
	//	nNewSize=1024;
	if (nNewSize < GetMemSize())
		return 0;

	// New Copy Data Over
	g_AllocMemoryLock.WaitToWrite();

	//PBYTE pNewBuffer = (PBYTE) VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);
	PBYTE pNewBuffer = new BYTE[nNewSize+1];
	g_AllocMemoryLock.Done();

	if(pNewBuffer==NULL)
	{
		g_log.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 没有足够的内存(%08x)！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError(),nNewSize);
		g_logErr.Log(LEVEL_ERROR,"%s,%s()::%d=%08x 没有足够的内存(%08x)！！！\n",__FILE__,__FUNCTION__,__LINE__,GetLastError(),nNewSize);
		return 0;
	}
	UINT nBufferLen = GetBufferLen();
	if(m_pBase)
	{
			if(nBufferLen>0)
				CopyMemory(pNewBuffer,m_pBase,nBufferLen);
	
	}


	g_AllocMemoryLock.WaitToWrite();
	//VirtualFree(m_pBase,0,MEM_RELEASE);
	if(m_pBase)delete m_pBase;
	g_AllocMemoryLock.Done();

	// Hand over the pointer
	m_pBase = pNewBuffer;

	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;

	m_nSize = nNewSize;

	return m_nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Scan
// 
// DESCRIPTION:	Scans the buffer for a given byte sequence
// 
// RETURNS:		Logical offset
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
int CBuffer::Scan(PBYTE pScan,UINT nPos)
{
	if (nPos > GetBufferLen() )
		return -1;


		//m_lock.WaitToWrite();
		PBYTE pStr = (PBYTE) strstr((char*)(m_pBase+nPos),(char*)pScan);
		
		int nOffset = 0;

		if (pStr)
			nOffset = (pStr - m_pBase) + strlen((char*)pScan);
		//m_lock.Done();
		return nOffset;


	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	ClearBuffer
// 
// DESCRIPTION:	Clears/Resets the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CBuffer::ClearBuffer()
{
	// Force the buffer to be empty
	//m_lock.WaitToWrite();
	m_pPtr = m_pBase;
	DeAllocateBuffer(GetMemSize());

	//m_lock.Done();
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Write
// 
// DESCRIPTION:	Writes a string a the end of the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(CString& strData)
{
	int nSize = strData.GetLength();
	return Write((PBYTE) strData.GetBuffer(nSize), nSize);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Insert
// 
// DESCRIPTION:	Insert a string at the beginning of the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(CString& strData)
{
	int nSize = strData.GetLength();
	return Insert((PBYTE) strData.GetBuffer(nSize), nSize);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Copy
// 
// DESCRIPTION:	Copy from one buffer object to another...
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CBuffer::Copy(CBuffer& buffer)
{

	//	m_lock.WaitToWrite();
		int nReSize = buffer.GetMemSize();
		int nSize = buffer.GetBufferLen();
		ClearBuffer();
		if(!ReAllocateBuffer(nReSize))
		{
	//		m_lock.Done();
			return;
		}
		if(m_pPtr&&m_pBase)
		{
			m_pPtr = m_pBase + nSize;
			if(buffer.GetBufferLen()>0)
				CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
		}


	//m_lock.Done();

}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBuffer
// 
// DESCRIPTION:	Returns a pointer to the physical memory determined by the offset
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
PBYTE CBuffer::GetBuffer(UINT nPos)
{
	__try{
	//	m_lock.WaitToRead();
		return m_pBase+nPos;
	}
	__finally
	{
	//	m_lock.Done();
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBuffer
// 
// DESCRIPTION:	Returns a pointer to the physical memory determined by the offset
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
///////////////////////////////////////////////////////////////////////////////
void CBuffer::FileWrite(const CString& strFileName)
{
	CFile file;

	if (file.Open(strFileName, CFile::modeWrite | CFile::modeCreate))
	{
		file.Write(m_pBase,GetBufferLen());
		file.Close();
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Delete
// 
// DESCRIPTION:	Delete data from the buffer and deletes what it reads
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Delete(UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?

	//	m_lock.WaitToWrite();
		if (nSize > GetMemSize())
		{
	//		m_lock.Done();
			return 0;
		}
		if (nSize>0)
		{	
			if (nSize > GetBufferLen())
				nSize = GetBufferLen();
			if((GetMemSize() - nSize)>0)
				MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);
			m_pPtr -= nSize;
		}

	DeAllocateBuffer(GetBufferLen());

//	m_lock.Done();
	return TRUE;
}