#ifndef __DDC_THRED_H__
#define __DDC_THRED_H__

#include "ddc_util.h"
using namespace std;


#ifdef _WIN32
#define MP_STILL_ACTIVE STILL_ACTIVE
#else
#define MP_STILL_ACTIVE 259
#endif

namespace nm_ddc
{

typedef unsigned long Time_Value;

/**
 * @class CNullMutex
 *
 * @brief Implement a do nothing <Mutex>, i.e., all the methods are no ops.
 */
class CNullMutex
{
public:
	int Lock(){ return 0; };
	int Lock (Time_Value &) {errno = DDCE_TIMEOUT; return -1;}
	int Unlock(){ return 0; };
	int TryLock(){ return 0; };
};

#ifdef _MSC_VER

	
/**
 * @class CThreadMutex
 *
 * @brief CThreadMutex wrapper (only valid for threads in the same process).
 *
 * This implementation is optimized for locking threads that are in the same process. 
 * It maps to <CRITICAL_SECTION>s on NT and <ACE_mutex_t> with <type> set to <USYNC_THREAD> on UNIX.
 * CThreadMutex is recursive on some platforms (like Win32). However, on most platforms (like Solaris) it is not
 * recursive.  To be totally safe and portable, developers should use CRecursiveThreadMutex when they need a
 * recursive mutex.
 */
class CThreadMutex
{
public:

	/// Constructor.
	CThreadMutex()
	{
		InitializeCriticalSection(&m_CritSect);
	}

	/// Implicitly destroy the mutex.
	virtual ~CThreadMutex()
	{
		DeleteCriticalSection(&m_CritSect);
	}

	int Lock()
	{
		EnterCriticalSection(&m_CritSect);
		return 0;
	}

	int Lock (Time_Value &tv);	
	int TryLock (void);


	int Unlock()
	{
		LeaveCriticalSection(&m_CritSect);
		return 0;
	}

	CRITICAL_SECTION handle(void) const
	{
		return m_CritSect;
	}
private:
	::CRITICAL_SECTION m_CritSect;

private:
	CThreadMutex(const CThreadMutex &);
	CThreadMutex& operator = (const CThreadMutex &);
};

/**
 * @class CProcessMutex
 *
 * @brief A wrapper for mutexes that can be used across processes on
 * the same host machine, as well as within a process, of course.
 */
class CProcessMutex
	{
public:
	CProcessMutex()
	{
		m_mtx = CreateMutex(NULL,FALSE,NULL);
	}
	
	virtual ~CProcessMutex()
	{
		CloseHandle(m_mtx);
	}
	
	int Lock()
	{
		WaitForSingleObject(m_mtx,INFINITE);
		return 0;
	}
	
	int Unlock()
	{
		ReleaseMutex(m_mtx);
		return 0;
	}
	DDC_HANDLE handle(void) const
	{
		return m_mtx;
	}
private:
	DDC_HANDLE m_mtx;
	
private:
	CProcessMutex(const CProcessMutex &);
	CProcessMutex& operator = (const CProcessMutex &);
};

/**
 * @class Manual_Event
 *
 * @brief Manual Events.
 *
 * Specialization of Event mechanism which wakes up all waiting
 * thread on <signal>.  All platforms support process-scope locking
 * support.  However, only Win32 platforms support global naming and
 * system-scope locking support.
 */
class Manual_Event
{
public:
	Manual_Event()
	{
		hnd = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	}
	~Manual_Event()
	{
		CloseHandle( hnd);
	}		
	int wait (uint timeout=INFINITE)
	{
		int ret = ::WaitForSingleObject(hnd,timeout);
		if( WAIT_TIMEOUT == ret )
			return DDCE_TIMEOUT;
		else if( WAIT_FAILED == ret )
			return -1;
		return 0;
	}
	int signal (void)
	{
		SetEvent(hnd);
		return 0;
	}
	int reset(void)
	{
		::ResetEvent(hnd);
		return 0;
	}
	DDC_HANDLE handle() const {return hnd;}
protected:
	DDC_HANDLE hnd;
};

/**
 * @class Auto_Event
 *
 * @brief Auto Events.
 *
 * Specialization of Event mechanism which wakes up one waiting
 * thread on <signal>.  All platforms support process-scope locking
 * support.  However, only Win32 platforms support global naming and
 * system-scope locking support.
 */
class Auto_Event
{
public:
	Auto_Event()
	{
		hnd = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	}
	~Auto_Event()
	{
		CloseHandle( hnd);
	}		
	int wait (uint timeout=INFINITE)
	{			
		int ret = ::WaitForSingleObject(hnd,timeout);
		if( WAIT_TIMEOUT == ret )
			return DDCE_TIMEOUT;
		else if( WAIT_FAILED == ret )
			return -1;			
		return 0;
	}
	int signal (void)
	{
		SetEvent(hnd);
		return 0;
	}
	int reset(void)
	{
		::ResetEvent(hnd);
		return 0;
	}
	DDC_HANDLE handle() const {return hnd;}
protected:
	DDC_HANDLE hnd;
};
#else

/**
 * @class CThreadMutex
 *
 * @brief CThreadMutex wrapper (only valid for threads in the same process).
 *
 * This implementation is optimized for locking threads that are in the same process. 
 * It maps to <CRITICAL_SECTION>s on NT and <ACE_mutex_t> with <type> set to <USYNC_THREAD> on UNIX.
 * CThreadMutex is recursive on some platforms (like Win32). However, on most platforms (like Solaris) it is not
 * recursive.  To be totally safe and portable, developers should use CRecursiveThreadMutex when they need a
 * recursive mutex.
 */
class CThreadMutex
{
public:
	CThreadMutex()
	{
		pthread_mutex_init(&m_CritSect,0);
	}

	virtual ~CThreadMutex()
	{
		pthread_mutex_destroy(&m_CritSect);
	}

	int Lock()
	{
		pthread_mutex_lock(&m_CritSect);
		return 0;
	}

	int Unlock()
	{
		pthread_mutex_unlock(&m_CritSect);
		return 0;
	}
	
	pthread_mutex_t &handle(void) {	return m_CritSect;	}
	
private:
	::pthread_mutex_t m_CritSect;

private:
	CThreadMutex(const CThreadMutex &);
	CThreadMutex& operator = (const CThreadMutex &);
};	

typedef CThreadMutex CProcessMutex;

/**
 * @class Manual_Event
 *
 * @brief Manual Events.
 *
 * Specialization of Event mechanism which wakes up all waiting
 * thread on <signal>.  All platforms support process-scope locking
 * support.  However, only Win32 platforms support global naming and
 * system-scope locking support.
 */
class Manual_Event
{
public:
	Manual_Event()
	{
		pthread_cond_init(&hnd,NULL);
		bsign = false;
	}
	~Manual_Event()
	{
		pthread_cond_destroy(&hnd);
	}
	int wait (void)
	{	
		mtx.Lock();
		while( !bsign )
			pthread_cond_wait(&this->hnd,&mtx.handle() );
		mtx.Unlock();
		return 0;
	}
	int wait (uint timeout)
	{	
		mtx.Lock();
		int ret = 0;
		while( !bsign )
		{				
			timespec abstime;
			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec += timeout/1000;
			timeout %= 1000;
			abstime.tv_nsec += timeout*1000000;
			ret = pthread_cond_timedwait(&hnd,&mtx.handle(),&abstime );				
			if( ETIMEDOUT == ret )
			{
				ret = DDCE_TIMEOUT;
				break;
			}
			else if( ret != 0 )
			{
				ret = -1;
				break;
			}
		}
		mtx.Unlock();
		return ret;
	}
	int signal (void)
	{
		mtx.Lock();
		bsign = true;
		pthread_cond_signal(&hnd);
		mtx.Unlock();
		return 0;
	}
	int reset(void)
	{			
		mtx.Lock();
		bsign = false;		
		mtx.Unlock();
		return 0;
	}
	pthread_mutex_t &handle() {return mtx.handle();}
protected:
	CThreadMutex mtx;
	pthread_cond_t  hnd;
	bool bsign;
};


/**
 * @class Auto_Event
 *
 * @brief Auto Events.
 *
 * Specialization of Event mechanism which wakes up one waiting
 * thread on <signal>.  All platforms support process-scope locking
 * support.  However, only Win32 platforms support global naming and
 * system-scope locking support.
 */
class Auto_Event
{
public:
	Auto_Event()
	{
		pthread_cond_init(&hnd,NULL);
		bsign = false;
	}
	~Auto_Event()
	{
		pthread_cond_destroy(&hnd);
	}		
	int wait (void)
	{	
		mtx.Lock();
		while( !bsign )
			pthread_cond_wait(&this->hnd,&mtx.handle() );
		mtx.Unlock();
		this->reset();
		return 0;
	}
	int wait (uint timeout)
	{				
		mtx.Lock();	
		int ret = 0;
		while( !bsign )
		{				
			timespec abstime;
			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec += timeout/1000;
			timeout %= 1000;
			abstime.tv_nsec += timeout*1000000;
			ret = pthread_cond_timedwait(&hnd,&mtx.handle(),&abstime );				
			if( ETIMEDOUT == ret )
			{
				ret = DDCE_TIMEOUT;
				break;
			}
			else if( ret != 0 )
			{
				ret = -1;
				break;
			}
		}
		mtx.Unlock();
		return ret;
	}
	int signal (void)
	{
		mtx.Lock();
		bsign = true;
		pthread_cond_signal(&hnd);
		mtx.Unlock();
		return 0;
	}
	int reset(void)
	{			
		mtx.Lock();
		bsign = false;		
		mtx.Unlock();
		return 0;
	}
	pthread_mutex_t &handle() {return mtx.handle();}
protected:
	CThreadMutex mtx;
	pthread_cond_t  hnd;
	bool bsign;
};

#endif

/**
* @class CLockGuard
*
* @brief This data structure is meant to be used within a method or function,
* It performs automatic aquisition and release of a <Mutex>.
*/
template<class Mutex>
class CLockGuard
{
public:
	CLockGuard(Mutex *pLockable)
	{
		m_pLockable = pLockable;
		m_pLockable->Lock();
	}
	~CLockGuard()
	{
		m_pLockable->Unlock();
	}
private:
	Mutex *m_pLockable;
};




///////////////////////////////////////////////////////////////////////////////////////




#define THR_NEW_LWP 0
#define THR_JOINABLE 1
#define DEFAULT_THREAD_PRIORITY 0

class DDC_Thread
{
public:
	static int spawn (THR_FUNC func,void *arg = 0,long flags = THR_NEW_LWP | THR_JOINABLE,int *t_id = 0,DDC_HANDLE *t_handle = 0,long priority = DEFAULT_THREAD_PRIORITY,
			void *stack = 0,size_t stack_size = 0);

	static int suspend (DDC_HANDLE);
	static int resume (DDC_HANDLE);
	static int wait(DDC_HANDLE,int timeout);
	static int getprio (DDC_HANDLE ht_id, int &priority);
	static int getprio (DDC_HANDLE ht_id, int &priority, int &policy);
	static int setprio (DDC_HANDLE ht_id, int priority, int policy = -1);
	static int thr_state(DDC_HANDLE handle, uint& state);
	static int kill (DDC_HANDLE, int signum);
	static void yield (void);
	static int sleep(uint mi);	
	static int cancel(DDC_HANDLE handle);
};	//class DDC_Thread



class Thread_Desc
{
public:
	void Set(int t_id,DDC_HANDLE t_handle,int g_id,int task,int flags)
	{
		thr_id_ = t_id;
		thr_handle_ = t_handle;
		grp_id_ = g_id;
		task_ = task;
		flags_ = flags;
	}
	int thr_id_;
	DDC_HANDLE thr_handle_;
	int grp_id_;
	int task_;
	int flags_;
};


class Thread_Manager
{
public:		
	Thread_Manager();
	virtual ~Thread_Manager (void);

	int open (size_t size = 0);
	int close (void);
	int spawn (THR_FUNC func,void *arg = 0,	long flags = 0,int * = 0,DDC_HANDLE *t_handle = 0,long priority = 0,	int grp_id = -1,void *stack = 0,size_t stack_size = 0);
	int spawn_n (size_t n,THR_FUNC func,void *arg = 0,long flags = 0,long priority = 0,int grp_id = -1,int *task = 0,DDC_HANDLE thread_handles[] = 0,void *stack[] = 0,size_t stack_size[] = 0);
	int spawn_i (THR_FUNC func, void *args, long flags,int *t_id,DDC_HANDLE *t_handle,long priority, int grp_id,void *stack, size_t stack_size,int *task);
	int wait (const Time_Value timeout = (Time_Value)INFINITE,int abandon_detached_threads = 0);
	int join (DDC_HANDLE tid, unsigned long *status = 0);
	int wait_grp (int grp_id,const Time_Value timeout);
	int suspend_all (void);
	int suspend (DDC_HANDLE);
	int suspend_grp (int grp_id);
	int resume_all (void);
	int resume (DDC_HANDLE);
	int resume_grp (int grp_id);
	int kill_all (int signum);
	int kill (DDC_HANDLE,int signum);
	int kill_grp (int grp_id,int signum);
	int cancel_all (int async_cancel = 0);
	int cancel (DDC_HANDLE, int async_cancel = 0);
	int cancel_grp (int grp_id, int async_cancel = 0);
	int set_grp (DDC_HANDLE,int grp_id);
	int get_grp (DDC_HANDLE,int &grp_id);
	int get_handle(int grp_id,DDC_HANDLE &);
	int setprio (DDC_HANDLE handle, int priority, int policy = -1);
	int thr_state (DDC_HANDLE handle, uint& state);

	void remove_thr (Thread_Desc *td);
	void remove_thr_all (void);

protected:		
	int grp_id_;
	list<Thread_Desc * > thr_list_;

public:
	static Thread_Manager *instance (void);
	static Thread_Manager *instance (Thread_Manager *);
	static CThreadMutex lock_;
	static Thread_Manager *thr_mgr_;

};


}		//namespace nm_ddc

#endif	//__DDC_THRED_H__

