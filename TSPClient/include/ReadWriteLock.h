

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
   HANDLE m_hsemReaders;     //�����д���������ȴ�
   HANDLE m_hsemWriters;     //����ж�������д�ȴ�
   int    m_nWaitingReaders; //�ȴ��Ķ��߸���
   int    m_nWaitingWriters; //�ȴ���д�߸���
   int    m_nActive;         //
};

#endif