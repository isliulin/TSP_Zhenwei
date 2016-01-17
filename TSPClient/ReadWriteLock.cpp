
#include "stdafx.h"
#include "TSPClientPublic.h"
#include "windows.h"
#include "ReadWriteLock.h"
#include "Log.h"
#include "public.h"

///////////////////////////////////////////////////////////////////////////////

extern CLog g_logErr;
CReadWriteLock::CReadWriteLock() {
   m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
   m_hsemReaders = CreateSemaphore(NULL, 0, MAXLONG, NULL);//无信号状态
   m_hsemWriters = CreateSemaphore(NULL, 0, MAXLONG, NULL);//无信号状态
   InitializeCriticalSection(&m_cs);
}


///////////////////////////////////////////////////////////////////////////////


CReadWriteLock::~CReadWriteLock() {

#ifdef _DEBUG
   if (m_nActive != 0)
      DebugBreak();
#endif

   m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
   DeleteCriticalSection(&m_cs);
   CloseHandle(m_hsemReaders);
   CloseHandle(m_hsemWriters);
}


///////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteLock::TryWaitToRead(int timeout) {

   if(TryEnterCriticalSection(&m_cs))
   {
		//如果(m_nActive < 0)，那么表示有一个线程正在写操作
	   //如果(m_nWaitingWriters>0)，那么表示有几个个线程正在写操作
	   BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

	   if (fResourceWritePending) {
			//这个读操作的线程必须等待，读操作计数加一
		  m_nWaitingReaders++;
	   } else {
			//这个读者能够读，当前读取该资源的线程的数量+1
		  m_nActive++;
	   }

	   LeaveCriticalSection(&m_cs);

	   if (fResourceWritePending) {
			//如果有线程正在写操作，那么等待读信标，大于0就有信号
		   if(WaitForSingleObject(m_hsemReaders, timeout)==WAIT_OBJECT_0)
			   return TRUE;
	   }else
	   {
			return TRUE;
	   }
   }
   return FALSE;
}

VOID CReadWriteLock::WaitToRead() {

   EnterCriticalSection(&m_cs);
	//如果(m_nActive < 0)，那么表示有一个线程正在写操作
   //如果(m_nWaitingWriters>0)，那么表示有几个个线程正在写操作
   BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

   if (fResourceWritePending) {
		//这个读操作的线程必须等待，读操作计数加一
      m_nWaitingReaders++;
   } else {
		//这个读者能够读，当前读取该资源的线程的数量+1
      m_nActive++;
   }

   LeaveCriticalSection(&m_cs);

   if (fResourceWritePending) {
		//如果有线程正在写操作，那么等待读信标，大于0就有信号
		   WaitForSingleObject(m_hsemReaders, INFINITE);
   }
}


///////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteLock::TryWaitToWrite(int timeout) {

	if(TryEnterCriticalSection(&m_cs))
	{
		//是否有读者或写的访问资源（m_nActive=0表示没有）
		BOOL fResourceOwned = (m_nActive != 0);

		if (fResourceOwned) {
			//这个写者必须等，等待写线程加一
			m_nWaitingWriters++;
		} else {
			//如果能够写，m_nActive置一表示此线程占用资源
			m_nActive = -1;
		}
		LeaveCriticalSection(&m_cs);
		if (fResourceOwned) {
			////如果有线程正在都或写操作，那么等待写信标，大于0就有信号
			if(WaitForSingleObject(m_hsemWriters, timeout)==WAIT_OBJECT_0)
				return TRUE;
		}else
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID CReadWriteLock::WaitToWrite() {

   EnterCriticalSection(&m_cs);
	//是否有读者或写的访问资源（m_nActive=0表示没有）
   BOOL fResourceOwned = (m_nActive != 0);

   if (fResourceOwned) {
		//这个写者必须等，等待写线程加一
      m_nWaitingWriters++;
   } else {
	  //如果能够写，m_nActive置一表示此线程占用资源
      m_nActive = -1;
   }

   LeaveCriticalSection(&m_cs);

   if (fResourceOwned) {
		////如果有线程正在都或写操作，那么等待写信标，大于0就有信号
      WaitForSingleObject(m_hsemWriters, INFINITE);
   }
}


///////////////////////////////////////////////////////////////////////////////


VOID CReadWriteLock::Done() {

   EnterCriticalSection(&m_cs);

   if (m_nActive > 0) {
		//表示有线程正在读资源，那么一个读者必须完成读操作
      m_nActive--;
   } else {
		//m_nActive一定=-1；表示有线程正在写资源，那么一个写者必须完成写操作，使m_nActive=0;
      m_nActive++;
   }

   HANDLE hsem = NULL;  
   LONG lCount = 1;     
   if (m_nActive == 0) {
		//没有线程访问，判断谁应该唤醒呢？
		//如果有写者，就优先写着线程被唤醒
      if (m_nWaitingWriters > 0) {

         m_nActive = -1;         
         m_nWaitingWriters--;   
         hsem = m_hsemWriters;   

      } else if (m_nWaitingReaders > 0) {
		//如果没有写者，但有读者就唤醒读者
         m_nActive = m_nWaitingReaders;  
         m_nWaitingReaders = 0;          
         hsem = m_hsemReaders;           
         lCount = m_nActive;            
      } else {

      }
   }

   LeaveCriticalSection(&m_cs);

   if (hsem != NULL) {
		//如果有写者，唤醒写着，写信号+1后等于1，表示可以进行些操作了
		//如果没有写者，但有读者就唤醒读者，读信号就加所有等待的读者线程。表示可以进行读操作了。
      // Some threads are to be released
      ReleaseSemaphore(hsem, lCount, NULL);
   }
}

CMutexLock::CMutexLock(char *pName,int timeout)
{
	return;
	Init(pName,timeout);
}
BOOL CMutexLock::Init(char *pName,int timeout)
{
//	return TRUE;
	m_nTimeOut=timeout;
	mystrcpy(m_Name,pName,MAX_PATH);
	m_Mutex=NULL;
	m_Mutex=OpenMutex(MUTEX_ALL_ACCESS,TRUE,pName);
	if(m_Mutex==NULL)
	{
		m_Mutex=CreateMutex(NULL,0,pName);
	}
	if(m_Mutex)
		return TRUE;
	return FALSE;
}
CMutexLock::~CMutexLock()
{
//	return;
	CloseHandle(m_Mutex);
}
BOOL CMutexLock::WaitToWrite()
{
//	return TRUE;
	return Lock();
}
BOOL CMutexLock::WaitToRead()
{
//	return TRUE;
	return Lock();
}
BOOL CMutexLock::Done()
{
//	return TRUE;
	return Unlock();
}
BOOL CMutexLock::Lock()
{
//	return 0;
	if(WaitForSingleObject(m_Mutex,m_nTimeOut)==WAIT_TIMEOUT)
	{
		g_logErr.Log(LEVEL_WARNNING,"互斥锁 %s 超时！！！\n",m_Name);
		return FALSE;
	}
	return TRUE;
}
BOOL CMutexLock::Unlock()
{
//	return 0;
	if(m_Mutex)
		return ReleaseMutex(m_Mutex);
	return FALSE;
}