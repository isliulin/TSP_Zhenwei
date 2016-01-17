

#ifndef __READ_WRITE_LOCK__
#define __READ_WRITE_LOCK__


///////////////////////////////////////////////////////////////////////////////
class CMutexLock {
public:
	int    m_nTimeOut;
	CMutexLock(){m_nTimeOut=-1;};
	CMutexLock(int timeout){m_nTimeOut=timeout;};
	CMutexLock(char *pName,int timeout);
	~CMutexLock();
	BOOL Init(char *pName,int timeout);
	BOOL Lock();
	BOOL Unlock();
	BOOL WaitToWrite();
	BOOL WaitToRead();
	BOOL Done();

private:
	HANDLE m_Mutex;
	char m_Name[MAX_PATH];
};

class CReadWriteLock {
public:
	CReadWriteLock();
   ~CReadWriteLock();                

   VOID WaitToRead();        
   VOID WaitToWrite();  

   BOOL TryWaitToRead(int timeout);        
   BOOL TryWaitToWrite(int timeout);
   VOID Done();

private:
   CRITICAL_SECTION m_cs;    
   HANDLE m_hsemReaders;     //如果有写操作，读等待
   HANDLE m_hsemWriters;     //如果有读操作，写等待
   int    m_nWaitingReaders; //等待的读者个数
   int    m_nWaitingWriters; //等待的写者个数
   int    m_nActive;         //
};

#endif