
#include "ddc_util.h"
#include "ddc_thread.h"
#include "ddc_debuglog.h"

namespace nm_ddc
{

int DDC_Thread::spawn (THR_FUNC func,void *arg,long flags,int *t_id,DDC_HANDLE *t_handle,long priority,void *stack,size_t stack_size)
{
	return thr_create(func,arg,flags,t_id,t_handle,priority,stack,stack_size);	
}
int DDC_Thread::suspend (DDC_HANDLE t_handle)
{
	return thr_suspend (t_handle);	
}

int DDC_Thread::resume (DDC_HANDLE t_handle)
{
	return thr_resume(t_handle);
}

int DDC_Thread::wait(DDC_HANDLE t_handle,int timeout)
{
	DDC_LOG (("Trace","DDC_Thread::wait:%d(%d)",t_handle,timeout));
	return thr_wait(t_handle,timeout);
}

int DDC_Thread::kill (DDC_HANDLE handle, int signum)
{
	DDC_LOG (("Trace","DDC_Thread::kill:%d",handle));
	return thr_kill(handle,signum);
}

int DDC_Thread::setprio (DDC_HANDLE ht_id, int priority, int policy)
{
	return thr_setprio(ht_id,priority,policy);
}

int DDC_Thread::sleep (uint mi)
{
	DDC_LOG (("Trace","DDC_Thread::sleep:%d",mi));
	return thr_sleep(mi);
}

int DDC_Thread::thr_state (DDC_HANDLE handle, uint& state)
{
#ifdef WIN32
	::GetExitCodeThread(handle,(LPDWORD)&state);
#else
	int ret = thr_kill(handle,0);
	if( !ret )
		state = MP_STILL_ACTIVE;
	else
		state = (uint)-1;
#endif
	return 0;
}

int DDC_Thread::cancel(DDC_HANDLE handle)
{
#ifdef WIN32
#else
#endif
	return 0;
}

// Process-wide Thread Manager.
Thread_Manager *Thread_Manager::thr_mgr_ = 0;
CThreadMutex Thread_Manager::lock_;

Thread_Manager *Thread_Manager::instance (void)
{
	if (Thread_Manager::thr_mgr_ == 0)
	{
		// Perform Double-Checked Locking Optimization.
		CLockGuard<CThreadMutex> lock(&Thread_Manager::lock_);

		if (Thread_Manager::thr_mgr_ == 0)
		{
			DDC_NEW_RETURN(Thread_Manager::thr_mgr_,Thread_Manager,0);
		}
	}

	return Thread_Manager::thr_mgr_;
}

Thread_Manager::Thread_Manager ():grp_id_ (1)
{
}

Thread_Manager::~Thread_Manager (void)
{
	DDC_LOG (("Trace","Thread_Manager::~Thread_Manager"));
	this->close ();
}

int Thread_Manager::close ()
{
	DDC_LOG (("Trace","Thread_Manager::close"));

	this->remove_thr_all ();
	return 0;
}

int Thread_Manager::spawn (THR_FUNC func,void *args,long flags,int *t_id,DDC_HANDLE *t_handle,long priority,int grp_id,void *stack,size_t stack_size)
{
	DDC_LOG(("Trace","Thread_Manager::spawn"));

	CLockGuard<CThreadMutex> lock(&Thread_Manager::lock_);
	
	if (grp_id == -1)
		grp_id = this->grp_id_++; // Increment the group id.

	DDC_HANDLE hnd;
	int tid;
	if( !t_handle )
		t_handle = &hnd;
	if( !t_id )
		t_id = &tid;
	if (this->spawn_i (func,args,flags,t_id,t_handle,priority,grp_id,stack,stack_size,0) == -1)
		return -1;

	return grp_id;
}

int Thread_Manager::spawn_i (THR_FUNC func, void *args, long flags,int *t_id,DDC_HANDLE *t_handle,long priority, int grp_id,void *stack, size_t stack_size,int *task)
{	
	int ret = DDC_Thread::spawn (func,args,flags,t_id,t_handle,priority,stack,stack_size);	
	if (ret != 0)
	{
		DDC_LOG(("Error","Spawn error return %d",ret));
		return ret;
	}
	
	Thread_Desc *thr_desc;
	DDC_NEW_RETURN(thr_desc,Thread_Desc,DDCE_MEMORY_ALLOC_ERROR);
	thr_desc->Set(*t_id,*t_handle,grp_id,0/* *task*/,flags);
	this->thr_list_.push_back(thr_desc);
	return 0;
}

int Thread_Manager::wait (const Time_Value timeout, int tid)
{
	DDC_LOG (("Trace","Thread_Manager::waitall",tid));
	remove_thr_all();	
	return 0;
}
int Thread_Manager::wait_grp (int grp_id,const Time_Value timeout)
{
	DDC_LOG (("Trace","Thread_Manager::wait_grp:%d",grp_id));
	int ret = 0;
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=this->thr_list_.end();++iter)
	{
		if( (*iter)->grp_id_ == grp_id )
			ret = DDC_Thread::wait( (*iter)->thr_handle_,timeout);
	}
	return ret;
}

int Thread_Manager::suspend_grp (int grp_id)
{
	DDC_LOG (("Trace","Thread_Manager::suspend_grp:%d",grp_id));
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=this->thr_list_.end();++iter)
	{
		if( (*iter)->grp_id_ == grp_id )
			DDC_Thread::suspend( (*iter)->thr_handle_);
	}
	return 0;
}

int Thread_Manager::resume_grp (int grp_id)
{
	DDC_LOG (("Trace","Thread_Manager::resume_grp:%d",grp_id));
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=this->thr_list_.end();++iter)
	{
		if( (*iter)->grp_id_ == grp_id )
			DDC_Thread::resume( (*iter)->thr_handle_);
	}
	return 0;
}


int Thread_Manager::cancel_grp (int grp_id, int async_cancel)
{
	DDC_LOG (("Trace","Thread_Manager::cancel_grp:%d",grp_id));
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=this->thr_list_.end();++iter)
	{
		if( (*iter)->grp_id_ == grp_id )
			remove_thr( (*iter));
	}
	return 0;
}

int Thread_Manager::kill_grp (int grp_id, int async_cancel)
{
	DDC_LOG (("Trace","Thread_Manager::kill_grp:%d",grp_id));
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=this->thr_list_.end();++iter)
	{
		if( (*iter)->grp_id_ == grp_id )
			DDC_Thread::kill( (*iter)->thr_handle_,async_cancel);
	}
	return 0;
}

void Thread_Manager::remove_thr (Thread_Desc *td)
{
	DDC_LOG(("Trace","Thread_Manager::remove_thr"));
	DDC_Thread::wait(td->thr_handle_,INFINITE);
#if defined (_WIN32)	
		::CloseHandle (td->thr_handle_);
#endif
	DDC_DELETE(td);
}

void Thread_Manager::remove_thr_all (void)
{
#ifdef _WIN32
	vector<DDC_HANDLE> vec;
	list<Thread_Desc *>::const_iterator iter=thr_list_.begin();
	for(;iter!=thr_list_.end();++iter)
		vec.push_back((*iter)->thr_handle_);

	if( !vec.size() )
		return;
	WaitForMultipleObjects(vec.size(),&*vec.begin(),TRUE,INFINITE);
#else
	list<Thread_Desc *>::const_iterator iter=thr_list_.begin();
	for(;iter!=thr_list_.end();++iter)
		::pthread_join((*iter)->thr_handle_,NULL);		
#endif
	thr_list_.clear();
}

int Thread_Manager::get_handle(int grp_id,DDC_HANDLE &handle)
{
	DDC_LOG (("Trace","Thread_Manager::cancel_grp:%d",grp_id));
	list<Thread_Desc *>::const_iterator iter = this->thr_list_.begin();
	for(;iter!=thr_list_.end();++iter)
		if( (*iter)->grp_id_ == grp_id )
		{
			handle = (*iter)->thr_handle_;
			return 0;
		}

	return -1;
}

int Thread_Manager::setprio (DDC_HANDLE handle, int priority, int policy )
{
	return DDC_Thread::setprio(handle,priority,policy);	
}

int Thread_Manager::thr_state (DDC_HANDLE handle, uint& state)
{
	return DDC_Thread::thr_state( handle,state);	
}

}	//nm_ddc
