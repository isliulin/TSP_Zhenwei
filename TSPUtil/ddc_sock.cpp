#if !defined (_USE_ACE)

#include "ddc_sock.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

const ACE_Addr ACE_Addr::sap_any;
ACE_Thread_Manager *ACE_Thread_Manager::thr_mgr_ = 0;

namespace ACE_OS
{

#if defined (WIN32)
int socket_init (int version_high,int version_low )
{		
	WSADATA wsa_data;
	return WSAStartup (MAKEWORD (version_high, version_low),&wsa_data);		
}
#else
int socket_init (int version_high ,int version_low ){return 0;}	
#endif

#if defined(WIN32)
int socket_fini (void){	return WSACleanup();}
#else
int socket_fini (void){	return 0;}
#endif

#if defined(WIN32)
int close(ACE_HANDLE s){return closesocket(s);}
#else
int close(ACE_HANDLE s){return ::close(s);}
#endif

int printf( const char *format,... )
{
	char newformat[100];
	va_list variable_argument_list;	
	va_start (variable_argument_list, format);
	vsprintf (newformat, format, variable_argument_list);
	va_end (variable_argument_list);
	return ::printf(newformat);
}

}	//ACE_OS

ACE_INET_Addr::ACE_INET_Addr(unsigned short port,long addr)
{
	memset(reinterpret_cast<void*>(&inet_addr_),0,sizeof(inet_addr_));
	inet_addr_.sin_family = AF_INET;
	inet_addr_.sin_port = htons(port);
	inet_addr_.sin_addr.s_addr = htonl(addr);
}

ACE_INET_Addr::ACE_INET_Addr(unsigned short port,const char host_name[])
{
	memset(reinterpret_cast<void*>(&inet_addr_),0,sizeof(inet_addr_));
	inet_addr_.sin_family = AF_INET;
	inet_addr_.sin_port = htons(port);		
	
	hostent *hp = gethostbyname(host_name);		
	inet_addr_.sin_addr.s_addr = ((in_addr*)hp->h_addr)->s_addr;
}

void *ACE_INET_Addr::get_host_addr (void) const
{
	static char buff[20];		
#if defined( _VIRUS_IPV6 ) || defined(__GNUC__)
	if (0 == ::getnameinfo (reinterpret_cast<const sockaddr*> (get_addr()),
		this->get_size(),buff,20,0, 0, NI_NUMERICHOST))	
#else
	char *ch = inet_ntoa (this->inet_addr_.sin_addr);
	strncpy (buff,ch,20);
#endif
	return buff;	
}

int ACE_SOCK::get_remote_addr (ACE_INET_Addr &sa)
{
	int len = sa.get_size();
	sockaddr *addr = reinterpret_cast<sockaddr *> (sa.get_addr ());
	
	if (::getpeername (this->get_handle (),addr,(socklen_t *)&len) == -1)
		return -1;		
	return 0;
}

ACE_SOCK_Acceptor::ACE_SOCK_Acceptor(const ACE_INET_Addr &sock_addr)
{
	m_bInitalize = false;
	handle_ = socket(PF_INET,SOCK_STREAM,0);	
	if( handle_ == INVALID_SOCKET )
	{
		return;
	}
	if( -1 == bind(handle_,reinterpret_cast<sockaddr *>(sock_addr.get_addr()),sock_addr.get_size()) )
	{
		return;
	}
	if( -1 == listen(handle_,5) )
	{
		return;
	}
	m_bInitalize = true;
}

int ACE_SOCK_Stream::recv_n(char *buff,size_t len,int flag)
{
	int rcv = 0,count;
	while( (size_t)rcv<len)
	{
		count = ::recv(this->get_handle(),buff+rcv,len-rcv,flag);
		rcv += count;
	}
	return len;
}

#endif	//_USE_ACE

