#ifndef __VIRUS_ACE_H__
#define __VIRUS_ACE_H__

//OS_NS_Thread.h
#define THR_NEW_LWP     0
#define THR_JOINABLE    0
#define THR_INHERIT_SCHED 0
#define ACE_DEFAULT_THREAD_PRIORITY 0

#ifndef WIN32
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define WINAPI 
//typedef unsigned long DWORD;
#else
#include <process.h>
#include "WinSock2.h"
#include <Ws2tcpip.h>
#include "IO.h"
#pragma comment(lib, "ws2_32.lib")
#endif


#if !defined(socklen_t)
//typedef int socklen_t;
#endif

typedef unsigned long ACE_thread_t;
typedef int* ACE_hthread_t;
typedef void* ACE_THR_FUNC_RETURN;
typedef unsigned long (WINAPI *ACE_THR_FUNC)(void *);

typedef SOCKET ACE_HANDLE;



namespace ACE_OS
{
	extern int socket_init (int version_high = 2,int version_low = 2);
	extern int socket_fini (void);
	extern int close(ACE_HANDLE);
	int printf( const char *format,... );
}

class ACE_Time_Value;

class ACE_Addr
{	
public:
	virtual void *get_addr (void) const{return 0;}
	virtual void set_addr (void *, int len){}	
	static const ACE_Addr sap_any;
};

class ACE_INET_Addr:public ACE_Addr
{
public:
	ACE_INET_Addr(unsigned short port,long addr = INADDR_ANY);	
	ACE_INET_Addr (unsigned short port,const char host_name[]);
	
	unsigned short get_port(void) const {return inet_addr_.sin_port;}
	unsigned short get_port_number(void) const {return ntohs(get_port());}
	long get_ip_addr(void) const {return inet_addr_.sin_addr.s_addr;}
	void *get_addr (void) const{return (void*)&this->inet_addr_;}
	void *get_host_addr (void) const;		
	int get_size (void) const
	{    return static_cast<int>(sizeof(this->inet_addr_));	}
	
private:
	struct sockaddr_in inet_addr_;
};

class ACE_IPC_SAP
{
public:
	ACE_IPC_SAP(void):handle_(INVALID_SOCKET){}
	ACE_IPC_SAP(ACE_HANDLE h):handle_(h){}
	~ACE_IPC_SAP(){ if( INVALID_SOCKET != handle_) ACE_OS::close(handle_);}
	void set_handle(ACE_HANDLE h){
		if( INVALID_SOCKET != handle_)
			ACE_OS::close(handle_);
		handle_ = h;
	}
	ACE_HANDLE get_handle(void) const {return handle_;}
protected:
	ACE_HANDLE handle_;
};

class ACE_SOCK:public ACE_IPC_SAP
{
public:
	ACE_SOCK(void){}
	ACE_SOCK(ACE_HANDLE h):ACE_IPC_SAP(h){}
	int get_remote_addr (ACE_INET_Addr &sa);
};

class ACE_SOCK_Stream:public ACE_SOCK
{
public:
	ACE_SOCK_Stream(void){};
	ACE_SOCK_Stream(ACE_HANDLE h):ACE_SOCK(h){}
	int send(const char *buff,size_t len,int flag = 0)
	{
		return ::send(this->get_handle(),buff,len,flag);
	}
	int recv(char *buff,size_t len,int flag = 0)
	{
		return ::recv(this->get_handle(),buff,len,flag);
	}

	int recv_n(char *buff,size_t len,int flag = 0);	
};


class ACE_SOCK_Acceptor:public ACE_SOCK
{
public:
	ACE_SOCK_Acceptor(const ACE_INET_Addr &sock_addr);	
	int accept (ACE_SOCK_Stream &new_stream,
		ACE_Addr *remote_addr = 0,
		ACE_Time_Value *timeout = 0,
		int restart = 1,
        int reset_new_handle = 0) const
	{		
		new_stream.set_handle(::accept(handle_,0,0));
		if( new_stream.get_handle() == INVALID_SOCKET )
			return -1;
		else
			return 0;
	}
private:
	bool m_bInitalize;
};

class ACE_SOCK_Connector
{
public:
	ACE_SOCK_Connector(){};
	ACE_SOCK_Connector (ACE_SOCK_Stream &new_stream,
		const ACE_INET_Addr &remote_sap,
		const ACE_Time_Value *timeout = 0,
		const ACE_Addr &local_sap = ACE_Addr::sap_any,
		int reuse_addr = 0,
		int flags = 0,
		int perms = 0,
		int protocol = 0)
	{
		this->connect(new_stream,remote_sap,timeout,local_sap,reuse_addr,flags,perms,protocol);
	}

	int connect (ACE_SOCK_Stream &new_stream,
		const ACE_INET_Addr &remote_sap,
		const ACE_Time_Value *timeout = 0,
		const ACE_Addr &local_sap = ACE_Addr::sap_any,
		int reuse_addr = 0,
		int flags = 0,
		int perms = 0,
		int protocol = 0)
	{
		return ::connect (new_stream.get_handle(),reinterpret_cast<sockaddr *> (remote_sap.get_addr ()),remote_sap.get_size ());
	}
	int complete (ACE_SOCK_Stream &new_stream,
		ACE_Addr *remote_sap = 0,
		const ACE_Time_Value *timeout = 0);
};

class ACE_Thread_Manager
{
public:
	static ACE_Thread_Manager *instance (void)
	{
		if (ACE_Thread_Manager::thr_mgr_ == 0)
			ACE_Thread_Manager::thr_mgr_ = new ACE_Thread_Manager();
		
		return ACE_Thread_Manager::thr_mgr_;
	}

	int spawn (ACE_THR_FUNC func,
		void *arg = 0,
		long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
		ACE_thread_t * t_id= 0,
		ACE_hthread_t *t_handle = 0,
		long priority = ACE_DEFAULT_THREAD_PRIORITY,
		int grp_id = -1,
		void *stack = 0,
		size_t stack_size = 0)
	{		
#ifdef __GNUC__
		pthread_create((pthread_t*)&t_id,NULL,(void*(*)(void*))func,arg);
#else
		CreateThread(NULL,stack_size,func,arg,flags,t_id);
#endif
		return 0;
	}

protected:
	ACE_Thread_Manager(){};
private:
	static ACE_Thread_Manager *thr_mgr_;
	ACE_Thread_Manager(const ACE_Thread_Manager &);
	void operator=(const ACE_Thread_Manager &);
};




#endif	//__VIRUS_ACE_H__
