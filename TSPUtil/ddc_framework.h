
#if !defined(__DDC_FRAMEWORK_H__)
#define __DDC_FRAMEWORK_H__

#if defined (_DDC_ACE)
	#include <ace/Task.h>
	#include <ace/Task_T.h>
	#include <ace/Thread.h>
	#include <ace/Thread_Manager.h>
	#include <ace/Event_Handler.h>
	#include <ace/Synch.h>
	#include <ace/Reactor.h>
	#include <ace/Thread_Exit.h>
	#include <ace/Singleton.h>
	#include <ace/Token.h>
	#include <ace/Log_Msg.h>
	#include <ace/SOCK_Connector.h>
	#include <ace/Init_ACE.h>
	#include <ace/CDR_Stream.h>
	#include <ace/Service_Config.h>

	#if defined (_MSC_VER)
	#	pragma warning(disable:4786)
	#	ifdef _DEBUG
	#		pragma comment(lib,"ACEmfcd.lib")
	#	else
	#		pragma comment(lib,"ACEmfc.lib")
	#	endif
	#elif defined(__GNUC__)
	#endif
#endif

#if defined (_DDC_BOOST)
	#include <boost/lexical_cast.hpp>
	#include <boost/shared_ptr.hpp>
	#include <boost/scoped_ptr.hpp>
	#include <boost/crc.hpp>
	#include <boost/array.hpp>
	#include <boost/lambda/lambda.hpp>
	#include <boost/lambda/bind.hpp>
	#include <boost/thread/lockable_adapter.hpp>
	#include <boost/serialization/singleton.hpp>
	#include <boost/tokenizer.hpp>
	#include <boost/program_options.hpp>
#endif

#endif	//__DDC_FRAMEWORK_H__
