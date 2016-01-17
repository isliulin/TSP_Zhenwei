
#include "TSPPlug.h"
#include "ddc_util.h"
#include "ddc_memory.h"
#include "ddc_thread.h"
#include "Log.h"
#include "macros.h"
using namespace nm_ddc;

#include "PluginManager.h"

// ȫ������
static char s_szMoudlePath[MAX_PATH];		// ·��
static DWORD s_dwProcessID = -1;			// ����ID
static bool s_bInitialized = false;			// ��ʼ�����
HANDLE  s_hStopEvent;						// �˳��¼�


// �����̣߳�������Ϣ���¼�
static int work(void *)
{
	LOG((LEVEL_INFO,"Thread work running...\n"));
	int ret;

	//��ʼ�������ڴ�
	XShareMemory mem(TSP_PLUG_COMM_SIGN,4096);
	if( !mem.IsCreated() )
	{
		LOG((LEVEL_ERROR,"���������ڴ�ʧ��.\n"));
		return -1;
	}

	HANDLE hnds[2];
	hnds[0] = s_hStopEvent;
	hnds[1] = mem.m_hMemoryEvent;
	char buff[BUF_LEN_2K];

	while(1 )
	{		
		int nIndex = WaitForMultipleObjects(2,hnds,FALSE,INFINITE);		
		if( nIndex == WAIT_OBJECT_0 )
		{
			// �����߳��˳�����
			LOG((LEVEL_INFO,"hStopEvent signal.\n"));
			break;
		}
		else if( nIndex == WAIT_OBJECT_0+1)
		{
			// ��������������
			// ��������
			if( !mem.Read(buff,BUF_LEN_2K) )
			{
				LOG((LEVEL_WARNNING,"Read SharedMemory Failed.\n"));
				continue;
			}
			//���ּ��
			PLUG_COMM_ST *pheader = (PLUG_COMM_ST *)buff;
			if( pheader->magic != PLUG_COMM_ST::MAGIC )
			{
				LOG((LEVEL_WARNNING,"INVALID magic:%d.\n",pheader->magic));
				continue;
			}
			if( pheader->ver > PLUG_COMM_ST::CURVER )
			{
				LOG((LEVEL_WARNNING,"Unsupport version:cur=%d,remote=%d.\n",PLUG_COMM_ST::CURVER,pheader->ver));
				continue;
			}

			LOG((LEVEL_INFO,"receive cmd %d\n",pheader->cmd));

			switch( pheader->cmd )
			{
			case PLUG_COMM_ST::ECHO:		// ����
			{		
				pheader->request = PLUG_COMM_ST::REPLY;
				// ֱ��д�ڴ棬�������κ��¼�������Ҫ���������У���������ڴ��ǵ����
				PLUG_COMM_ST *pheader = (PLUG_COMM_ST *)mem.GetBuffer();
				pheader->request = PLUG_COMM_ST::REPLY;
				pheader->ver = PLUG_COMM_ST::CURVER;
				break;
			}
			case PLUG_COMM_ST::START:		// ����
				{					
					if( pheader->len != sizeof(PLUGPARAM) )					
					{
						LOG((LEVEL_WARNNING,"data length error:head.len=%d but need=%d\n",pheader->len,sizeof(PLUGPARAM) ));
						continue;
					}
					// ȡ����ȡ·��
					PLUGPARAM *pParam = (PLUGPARAM *)(pheader+1);
					memcpy(&GLOBAL_PLUGIN->m_plugParam,pParam,sizeof(PLUGPARAM));	
					LOG((LEVEL_INFO,"%s %d %s\n",GLOBAL_PLUGIN->m_plugParam.imp.szHost,GLOBAL_PLUGIN->m_plugParam.imp.wPort,GLOBAL_PLUGIN->m_plugParam.imp.chResultPath));
					ret = GLOBAL_PLUGIN->StatusAll(CPluginInfo::STATUS_RUNNING);
				}
				
				break;
			case PLUG_COMM_ST::STOP:		// ֹͣ
				ret = GLOBAL_PLUGIN->StatusAll(CPluginInfo::STATUS_STOP);
				break;
			case PLUG_COMM_ST::REMOVE:		// ɾ��
				ret = GLOBAL_PLUGIN->StatusAll(CPluginInfo::STATUS_DELETE);
				break;
			case PLUG_COMM_ST::UPDATE:		// ����
				{
					char *pConfigName = (char *)(pheader+1);
					if( strlen(pConfigName)+1 > pheader->len )
					{
						LOG((LEVEL_WARNNING,"data length error:head.len=%d but strlen=%d\n",pheader->len,strlen(pConfigName)));
						continue;
					}
					
					ret = GLOBAL_PLUGIN->Update(pConfigName);
				}
				
				break;
			case PLUG_COMM_ST::QUIT:		// �˳�		
				LOG((LEVEL_INFO,"�յ��˳���Ϣ,�����˳�"));

				ret = GLOBAL_PLUGIN->StatusAll(CPluginInfo::STATUS_UNLOAD);
				SetEvent(s_hStopEvent);				
				break;
			default:
				ret = DDCE_UNSUPPORT;
				break;
			}
			if( ret != 0 )
			{
				pheader->request = PLUG_COMM_ST::REPLY;
				// ֱ��д�ڴ棬�������κ��¼�������Ҫ���������У���������ڴ��ǵ����
				PLUG_COMM_ST *pheader = (PLUG_COMM_ST *)mem.GetBuffer();
				pheader->request = PLUG_COMM_ST::REPLY;
				pheader->ver = PLUG_COMM_ST::CURVER;
				char *p = (char *)(pheader+1);
				snprintf(p,10,"%d",ret);
			}
			
		}
		else
		{
			LOG((LEVEL_ERROR,"ret=%d(%x)\n",nIndex,nIndex));
			break;
		}
	}

	LOG((LEVEL_INFO,"Thread work stop.\n"));
	return 0;
}


// rundll32 ���õĺ���
void start(HWND hwnd,HINSTANCE hinst,LPSTR lpszCmdLine,int nCmdShow)
{
	
	WSADATA wsa={0};
	WSAStartup(MAKEWORD(2,2),&wsa);

	
	LOG((LEVEL_INFO,"start(%x,%x,%s,%x)\n",hwnd,hinst,lpszCmdLine,nCmdShow));

	if( s_bInitialized )
	{
		__try
		{
			work(hinst);

		}
		__except(1){}

	}
		// �������̷߳�ʽ����
	else
		LOG((LEVEL_INFO,"��ʼ�����ʧ��.\n"));
	//Thread_Manager::instance()->spawn(THR_FUNC(work),hinst);
}

// û��
void stop(HWND hwnd,HINSTANCE hinst,LPSTR lpszCmdLine,int nCmdShow)
{
	LOG((LEVEL_INFO,"stop(%x,%x,%s,%x)\n",hwnd,hinst,lpszCmdLine,nCmdShow));

	WSACleanup();
	
	SetEvent(s_hStopEvent);
}


// ��ʼ��
int Init(HMODULE hModule)
{
	//��־�ļ���ʼ��	
	char LogFile[MAX_PATH];
	GetModuleFileName(hModule,s_szMoudlePath,MAX_PATH);
	char *p = strrchr(s_szMoudlePath,'\\');
	if(p) *(p+1) = 0;	
	
	s_dwProcessID = ::GetCurrentProcessId();
#ifdef _MYDEBUG
	sprintf(LogFile,"%slog\\plugin_%d.log",s_szMoudlePath,::GetCurrentProcessId() );
	g_log.Open(LogFile,OUT_MODE_TO_FILE,EXT_MODE_LOCAL_TIME|EXT_MODE_THREAD_ID|EXT_MODE_LINE|EXT_MODE_TRUNC,1000,64*1024*1024);
#endif
	LOG((LEVEL_FUNC_IN_OUT,"\n"));


	s_hStopEvent = ::CreateEvent(NULL, FALSE, FALSE,"Global\\_DbFukjPOTShellokitty");
	if( s_hStopEvent == INVALID_HANDLE_VALUE )
	{
		LOG((LEVEL_ERROR,"�����˳��¼�ʧ��,lasterror=%d\n",GetLastError()));
		return -1;
	}

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		LOG((LEVEL_WARNNING,"�Ѿ�����һ��ʵ������.\n"));
		return -1;
	}


	// ��ʼ�����������
	int ret = GLOBAL_PLUGIN->Init(s_szMoudlePath);
	if( ret != 0 )
	{
		LOG((LEVEL_ERROR,"��ʼ���������������:%d(%x).\n",ret,ret));
		return ret;
	}

	s_bInitialized = true;
	return 0;
}


// ж��
int Release(HMODULE hModule)
{
	LOG((LEVEL_FUNC_IN_OUT,"Uninit\n"));

	if( s_bInitialized )
	{
		CloseHandle(s_hStopEvent);
		s_hStopEvent = NULL;
	}
	s_bInitialized = false;

#ifdef _MYDEBUG
	g_log.Close();
#endif	
	return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init(hModule);
		break;
	case DLL_THREAD_ATTACH:
		//LOG((LEVEL_INFO,"Thread attach.\n"));
		break;
	case DLL_THREAD_DETACH:
		//LOG((LEVEL_INFO,"Thread detach.\n"));
		break;
	case DLL_PROCESS_DETACH:
		LOG((LEVEL_INFO,"Process detach.\n"));
		Release(hModule);
		break;
	}
	return TRUE;
}


#ifdef _CONSOLE
int main(int argc, char **argv)
{

	Init(NULL);
	start(0,0,"",0);

	Thread_Manager::instance()->wait();
	Thread_Manager::instance()->remove_thr_all();
	Release(NULL);
	return 0;
}
#endif

