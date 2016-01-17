#if !defined __TSP_PLUGIN_INI_HPP__
#define __TSP_PLUGIN_INI_HPP__

#include "ddc_util.h"
#include "TspRecycle.h"
#include "crc32.h"

#define TSP_PLUGIN_PATH 520
#define TSP_PLUGIN_SHORT_PATH 64
#define TSP_PLUGIN_CFG_HEADER "PluginFile"				// 插件配置文件的头部标签
#define TSP_KEY 0xda

// 插件卸载的pid
#define TSP_PLUGIN_REMOVE_PID 7
#define TSP_PLUGIN_KEYBOARD 3
#define TSP_PLUGIN_MANAGE 11
#define TSP_PLUGIN_CAP_PID 0


// 文件头
typedef struct _Tag_TSP_FILE_HEADER
{
	uint magic;				// 魔术字
	uint crc32;				// 校验和或者crc
	ushort len;				// 文件体长度
	ushort reserved;			// 保留
	uchar type;				// 类型
	uchar ver;				// 版本
	uchar enc;				// 加密方式
	uchar compress;			// 压缩方式
}TSP_FILE_HEADER,*PTSP_FILE_HEADER;

#define TSP_FILE_HEADER_LEN sizeof(TSP_FILE_HEADER)

// 加密方法1
#define TSP_ENC_NONE 0xDE		// 明文
#define TSP_ENC_3DES 0x3D		// 3des
#define TSP_ENC_XOR  0x4E		// xor
#define TSP_COMPRESS_NONE 0xDE	// 未压缩

// ini文件魔术字
#define MAGIC_TSP_INIFILE 0xbdca02d4

// ini文件当前版本
#define VER_TSP_INIFILE 0x11
#define TSP_FILE_TYPE_INI 0xDC	// 文件类型


// 填充文件头部
inline int CompleteFileHeader(const string &filename)
{
	// filemap类还不能写
	return 0;
	
	CVirusFileMap fileMap(filename.c_str(),GENERIC_READ|GENERIC_WRITE);
	if( !fileMap )
		return DDCE_INVALID_HANDLE;

	size_t len = fileMap.FileSize() - TSP_FILE_HEADER_LEN;
	PTSP_FILE_HEADER pheader = (PTSP_FILE_HEADER)fileMap.GetBuffer();
	pheader->len = len;
	uchar *p = (uchar *)(pheader+1);
	pheader->crc32 = crc32(p,len);

	fileMap.Release();
	return 0;
}

/**
* @class CAutoCLocale
* @brief 自动本地化类,构造时设置为本地，析构设置为C
*/
struct CAutoCLocale
{
	CAutoCLocale()	
	{		
		oldLocale = std::locale::global(std::locale(""));
	}

	~CAutoCLocale()
	{
		std::locale::global(oldLocale);
	}
private:
	std::locale oldLocale;
};

inline int EncPluginIniFile(const string &filename)
{
	// 设置locale
	CAutoCLocale localeGuard;

	int ret;
	CVirusFileMap fileMap(filename.c_str(),GENERIC_READ);
	if( !fileMap )
		return DDCE_INVALID_HANDLE;

	size_t len = fileMap.FileSize();	

	string tmpFileName(filename);
	tmpFileName += ".tmp";

	std::ofstream ofile(tmpFileName.c_str(),ios::trunc|ios::out|ios::binary);
	if( !ofile )
		return false;
	
	// 写文件头
	TSP_FILE_HEADER header;
	memset(&header,0,TSP_FILE_HEADER_LEN);
	header.type = TSP_FILE_TYPE_INI;
	header.magic = MAGIC_TSP_INIFILE;
	header.ver = VER_TSP_INIFILE;
	header.enc = TSP_ENC_XOR;
	header.compress = TSP_COMPRESS_NONE;
	
		
	ofile.write((char *)&header,TSP_FILE_HEADER_LEN);

	char *p = (char *)fileMap.GetBuffer();
	while(len--)
	{
		char ch = *p^TSP_KEY;
		++p;
		ofile.write(&ch,1);
	}
	
	fileMap.Release();
	ofile.close();
	
	ret = CompleteFileHeader(tmpFileName);	
	DeleteFile(filename.c_str());
	MoveFile(tmpFileName.c_str(),filename.c_str());	

	return ret;
}



inline int DecPluginIniFile(const string &filename)
{
	// 设置locale
	CAutoCLocale localeGuard;

	int ret;
	CVirusFileMap fileMap(filename.c_str(),GENERIC_READ);
	if( !fileMap )
		return DDCE_INVALID_HANDLE;

	size_t len = fileMap.FileSize();
	if( len < TSP_FILE_HEADER_LEN )
		return DDCE_FILE_LENGTH_ERROR;

	len -= TSP_FILE_HEADER_LEN;

	// 判断检测
	PTSP_FILE_HEADER pheader = (PTSP_FILE_HEADER)fileMap.GetBuffer();
	if( pheader->magic != MAGIC_TSP_INIFILE ||
		pheader->type != TSP_FILE_TYPE_INI ||
		pheader->ver > VER_TSP_INIFILE )
		return DDCE_UNSUPPORT_VERSION;
		
	// 解压
	if( pheader->compress != TSP_COMPRESS_NONE )
		return DDCE_UNSUPPORT;

	if( pheader->enc != TSP_ENC_XOR )
		return DDCE_UNSUPPORT;
	
	string tmpFileName(filename);
	tmpFileName += ".tmp";

	std::ofstream ofile(tmpFileName.c_str(),ios::trunc|ios::out|ios::binary);
	if( !ofile )
		return false;
	
	char *p = (char *)(pheader+1);
	while(len--)
	{
		char ch = *p^TSP_KEY;
		++p;
		ofile.write(&ch,1);
	}

	fileMap.Release();
	ofile.close();

	ret = CompleteFileHeader(tmpFileName);	
	DeleteFile(filename.c_str());
	MoveFile(tmpFileName.c_str(),filename.c_str());	

	return ret;
}

/**
* 读取插件配置文件，到链表中
* @param l 链表
* @param strPluginConfigFile 插件配置文件名
* @return 0 成功
*/	
template<class T>
bool ReadPluginIniFile(std::list<T> &l,const string &strfileName,bool bEncrypt = false)
{
	// 中文路径	
	CAutoCLocale localeGuard;

	// 文件名
	std::string strPluginConfigFile = strfileName;
	CTspRecycle recycle;

	// 如果加密了，拷贝一份，解密，并且把拷贝的加入回收站
	if( bEncrypt )
	{
		std::string strPluginConfigFileOld(strPluginConfigFile);
		strPluginConfigFile += ".tmp";
		CopyFile(strPluginConfigFileOld.c_str(),strPluginConfigFile.c_str(),TRUE);
		recycle.Push(strPluginConfigFile);

		// 解密
		if( 0 != DecPluginIniFile(strPluginConfigFile) )
			return false;
	}

	//读取出插件名串
	char szFiles[520];
	::GetPrivateProfileString(TSP_PLUGIN_CFG_HEADER,"files","",szFiles,520,strPluginConfigFile.c_str() );

	if( szFiles[0] == 0 )
		return false;	//这个不算程序的错误，但是要通知外面加载，所以返回FALSE
	
	//解析
	T info;	
	char szBuff[MAX_PATH];
	const char *szDelim = ",";
	char *p = strtok(szFiles,szDelim);
	char szURL[TSP_PLUGIN_PATH];
	while(p)
	{
		info.reset();	//初始化

		info.m_strName.assign(p);		
		GetPrivateProfileString(p,"url","",szURL,TSP_PLUGIN_PATH,strPluginConfigFile.c_str());
		info.m_strURL.assign(szURL);
		GetPrivateProfileString(p,"md5","",info.m_md5,TSP_PLUGIN_SHORT_PATH,strPluginConfigFile.c_str());
		GetPrivateProfileString(p,"ver","",info.m_ver,TSP_PLUGIN_SHORT_PATH,strPluginConfigFile.c_str());
		info.m_dwSize = GetPrivateProfileInt(p,"size",-1,strPluginConfigFile.c_str());
		info.m_dwFlag = GetPrivateProfileInt(p,"flag",0,strPluginConfigFile.c_str());
		info.m_pid = GetPrivateProfileInt(p,"pid",0,strPluginConfigFile.c_str());
		GetPrivateProfileString(p,"root","",szBuff,MAX_PATH,strPluginConfigFile.c_str());
		info.m_strRoot.assign(szBuff);
		GetPrivateProfileString(p,"dir","",szBuff,MAX_PATH,strPluginConfigFile.c_str());
		info.m_strDir.assign(szBuff);
		GetPrivateProfileString(p,"realname","",szBuff,MAX_PATH,strPluginConfigFile.c_str());
		info.m_strRealName.assign(szBuff);

		if( info.m_strRoot != "" )
		{
			if( !::GetEnvironmentVariable(info.m_strRoot.c_str(),szBuff,MAX_PATH) )
				return false;
			
			info.m_strFullName.assign(szBuff);
			info.m_strFullName += "\\";			
		}

		info.m_strFullName += info.m_strDir;
		info.m_strFullName += info.m_strRealName;

		info.m_dwReserved = GetPrivateProfileInt(p,"dwReserved",0,strPluginConfigFile.c_str());
		GetPrivateProfileString(p,"szReserved","",info.m_szReserved,TSP_PLUGIN_SHORT_PATH,strPluginConfigFile.c_str());


		//放入链表
		l.push_back(info);
		p = strtok(NULL,szDelim);

	}
	
	return true;
};

/**
* 由链表中的数据生成插件配置文件
* @param l 链表
* @param strPluginConfigFile 插件配置文件名
* @return 0 成功
*/	
template<class T>
bool WritePluginIniFile(const std::list<T> &l,const string &strPluginConfigFile,bool bEncrypt = false )
{
	// 中文路径	
	CAutoCLocale localeGuard;
	
	//打开文件
	string strFileName;
	std::list<T>::const_iterator iter;

	std::ofstream ofile(strPluginConfigFile.c_str(),ios::trunc|ios::out|ios::binary);
	if( !ofile )
		return false;
	ofile.imbue(std::locale("C"));

	//写[PluginFile]
	ofile<<"["<<TSP_PLUGIN_CFG_HEADER<<"]\r\n";

	//拼装文件列表 a.dll,b.dll,c.exe		
	for(iter=l.begin();iter!=l.end();++iter)
	{
		if( iter != l.begin() )
			strFileName += ",";
		strFileName += iter->m_strName;
	}

	ofile<<"files="<<strFileName.c_str()<<"\r\n";

	//写单个插件信息
	for(iter=l.begin();iter!=l.end();++iter)
	{
		ofile<<"["<<iter->m_strName<<"]\r\n";
		ofile<<"ver="<<iter->m_ver<<"\r\n";
		ofile<<"url="<<iter->m_strURL<<"\r\n";
		ofile<<"pid="<<iter->m_pid<<"\r\n";
		ofile<<"md5="<<iter->m_md5<<"\r\n";		
		ofile<<"size="<<iter->m_dwSize<<"\r\n";		
		ofile<<"root="<<iter->m_strRoot<<"\r\n";
		ofile<<"dir="<<iter->m_strDir<<"\r\n";
		ofile<<"realname="<<iter->m_strRealName<<"\r\n";
		ofile<<"dwReserved="<<iter->m_dwReserved<<"\r\n";
		ofile<<"szReserved="<<iter->m_szReserved<<"\r\n";		
	}		

	ofile.close();

	// 如果要加密
	if( bEncrypt && 0 != EncPluginIniFile(strPluginConfigFile) )
		return false;
	
	return true;
};


#endif	//__TSP_PLUGIN_INI_HPP__