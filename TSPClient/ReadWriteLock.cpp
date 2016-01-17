
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
   m_hsemReaders = CreateSemaphore(NULL, 0, MAXLONG, NULL);//���ź�״̬
   m_hsemWriters = CreateSemaphore(NULL, 0, MAXLONG, NULL);//���ź�״̬
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
		//���(m_nActive < 0)����ô��ʾ��һ���߳�����д����
	   //���(m_nWaitingWriters>0)����ô��ʾ�м������߳�����д����
	   BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

	   if (fResourceWritePending) {
			//������������̱߳���ȴ���������������һ
		  m_nWaitingReaders++;
	   } else {
			//��������ܹ�������ǰ��ȡ����Դ���̵߳�����+1
		  m_nActive++;
	   }

	   LeaveCriticalSection(&m_cs);

	   if (fResourceWritePending) {
			//������߳�����д��������ô�ȴ����ű꣬����0�����ź�
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
	//���(m_nActive < 0)����ô��ʾ��һ���߳�����д����
   //���(m_nWaitingWriters>0)����ô��ʾ�м������߳�����д����
   BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

   if (fResourceWritePending) {
		//������������̱߳���ȴ���������������һ
      m_nWaitingReaders++;
   } else {
		//��������ܹ�������ǰ��ȡ����Դ���̵߳�����+1
      m_nActive++;
   }

   LeaveCriticalSection(&m_cs);

   if (fResourceWritePending) {
		//������߳�����д��������ô�ȴ����ű꣬����0�����ź�
		   WaitForSingleObject(m_hsemReaders, INFINITE);
   }
}


///////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteLock::TryWaitToWrite(int timeout) {

	if(TryEnterCriticalSection(&m_cs))
	{
		//�Ƿ��ж��߻�д�ķ�����Դ��m_nActive=0��ʾû�У�
		BOOL fResourceOwned = (m_nActive != 0);

		if (fResourceOwned) {
			//���д�߱���ȣ��ȴ�д�̼߳�һ
			m_nWaitingWriters++;
		} else {
			//����ܹ�д��m_nActive��һ��ʾ���߳�ռ����Դ
			m_nActive = -1;
		}
		LeaveCriticalSection(&m_cs);
		if (fResourceOwned) {
			////������߳����ڶ���д��������ô�ȴ�д�ű꣬����0�����ź�
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
	//�Ƿ��ж��߻�д�ķ�����Դ��m_nActive=0��ʾû�У�
   BOOL fResourceOwned = (m_nActive != 0);

   if (fResourceOwned) {
		//���д�߱���ȣ��ȴ�д�̼߳�һ
      m_nWaitingWriters++;
   } else {
	  //����ܹ�д��m_nActive��һ��ʾ���߳�ռ����Դ
      m_nActive = -1;
   }

   LeaveCriticalSection(&m_cs);

   if (fResourceOwned) {
		////������߳����ڶ���д��������ô�ȴ�д�ű꣬����0�����ź�
      WaitForSingleObject(m_hsemWriters, INFINITE);
   }
}


///////////////////////////////////////////////////////////////////////////////


VOID CReadWriteLock::Done() {

   EnterCriticalSection(&m_cs);

   if (m_nActive > 0) {
		//��ʾ���߳����ڶ���Դ����ôһ�����߱�����ɶ�����
      m_nActive--;
   } else {
		//m_nActiveһ��=-1����ʾ���߳�����д��Դ����ôһ��д�߱������д������ʹm_nActive=0;
      m_nActive++;
   }

   HANDLE hsem = NULL;  
   LONG lCount = 1;     
   if (m_nActive == 0) {
		//û���̷߳��ʣ��ж�˭Ӧ�û����أ�
		//�����д�ߣ�������д���̱߳�����
      if (m_nWaitingWriters > 0) {

         m_nActive = -1;         
         m_nWaitingWriters--;   
         hsem = m_hsemWriters;   

      } else if (m_nWaitingReaders > 0) {
		//���û��д�ߣ����ж��߾ͻ��Ѷ���
         m_nActive = m_nWaitingReaders;  
         m_nWaitingReaders = 0;          
         hsem = m_hsemReaders;           
         lCount = m_nActive;            
      } else {

      }
   }

   LeaveCriticalSection(&m_cs);

   if (hsem != NULL) {
		//�����д�ߣ�����д�ţ�д�ź�+1�����1����ʾ���Խ���Щ������
		//���û��д�ߣ����ж��߾ͻ��Ѷ��ߣ����źžͼ����еȴ��Ķ����̡߳���ʾ���Խ��ж������ˡ�
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
		g_logErr.Log(LEVEL_WARNNING,"������ %s ��ʱ������\n",m_Name);
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