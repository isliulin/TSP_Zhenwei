#include "ddc_util.h"
#include "ddc_filesystem.h"
#include "ddc_debuglog.h"

#ifdef _DDC_BOOST
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
#endif	//_DDC_BOOST

namespace nm_ddc
{
//删除文件
int VirDeleteFile(const char *fileName,const char *confirmStr)
{
	if( confirmStr )
		if( strstr(fileName,confirmStr) == NULL )
		{
			DDC_LOG(("Warning","VirDeleteFile(%s)没找到确认子�?%s)",fileName,confirmStr));
			return DDCE_DISK_OP_ERROR;
		}

		return ::DeleteFile(fileName)?DDCE_SUCCESS:DDCE_DISK_OP_ERROR;
}

//删除目录,递归删文�?最后删目录
int VirRemoveDirectory(const char *dir,const char *confirmStr)
{
	DDC_POINTNULL_RETURN(dir,DDCE_ARGUMENT_NULL);
	DDC_POINTNULL_RETURN(confirmStr,DDCE_ARGUMENT_NULL);
	int ret;

	if( strstr(dir,confirmStr) == NULL )
	{
		DDC_LOG(("Warning","VirRemoveDirectory(%s)没找到确认子�?%s)",dir,confirmStr));
		return DDCE_DISK_OP_ERROR;
	}
	
	ret = nm_ddc::VirClearDirectory(dir,confirmStr);
	DDC_BOOL_RETURN(ret!=0,DDCE_DISK_OP_ERROR);

	if( !RemoveDirectory(dir) )
	{
		DDC_LOG(("Wraning","RemoveDirectory(%s) fail.lasterror=%x",dir,GetLastError()));
		return DDCE_DISK_OP_ERROR;
	}		
	
	return DDCE_SUCCESS;
}

int clearFileAction(const char *fileName,struct stat * statbuf,void* userData)
{
	if( VirDeleteFile(fileName,(char *)userData) != 0 )
	{
		DDC_LOG(("Warning","delete file(%s) fail.lasterror=%x",fileName,GetLastError()));
		return DIRACTION_STOP;
	}
	return DIRACTION_CONTINUE;
}

int clearDirAction(const char *fileName,struct stat * statbuf,void* userData)
{
	if( VirRemoveDirectory(fileName,(char *)userData) != 0 )
	{
		DDC_LOG(("Warning","删除目录\"%s\"失败.lasterror=%x",fileName,GetLastError()));
		return DIRACTION_STOP;
	}
	return DIRACTION_CONTINUE;
}

//清空目录，非递归遍历，是目录调用VirRemoveDirectory(),文件直接�?
int VirClearDirectory(const char *dir,const char *confirmStr)
{
	DDC_POINTNULL_RETURN(dir,DDCE_ARGUMENT_NULL);

	int ret = recursive_dir_action(dir,FALSE,FALSE,TRUE,clearFileAction,clearDirAction,(void *)confirmStr);
	return (DIRACTION_CONTINUE == ret)?DDCE_SUCCESS:DDCE_DISK_OP_ERROR;	
}

int VirCreateDirectory(const char *dir)
{	
	if( dir[0] == 0 )
		return DDCE_ARGUMENT_ERROR;

	DDC_BOOL_RETURN( nm_ddc::exists(dir),DDCE_SUCCESS );		

	char dirFather[MAX_PATH_SAFE];
	strncpy(dirFather,dir,MAX_PATH_SAFE);
	char *p = strrchr(dirFather,DDC_C_SLASH);
	if( p )
		*p = 0;
	
	while( !nm_ddc::exists(dirFather) )
	{		
		int ret = VirCreateDirectory(dirFather);
		if( ret != DDCE_SUCCESS )
			return ret;
	}

	//目录结尾不带"\\"
	if( dir[strlen(dir)-1] != DDC_C_SLASH )
		return CreateDirectory(dir,NULL)==TRUE?0:DDCE_DISK_OP_ERROR;

	return DDCE_SUCCESS;
}

int copyDirAction(const char *fileName,struct stat * statbuf,void* userData)
{
	pair<const char *,const char *> *dirs = static_cast<pair<const char *,const char *> *>(userData);

	if( memcmp(fileName,dirs->first,strlen(dirs->first)) != 0 )	
	{
		DDC_LOG(("Error","Copy directioies,file \"%s\" not in source dir\"%s\".",fileName,dirs->first));
		return DIRACTION_STOP;
	}

	ostringstream os;
	os<<dirs->second<<&fileName[strlen(dirs->first)];

	if( !nm_ddc::exists(os.str().c_str() ) )
		if( VirCreateDirectory(os.str().c_str()) != 0 )
			return DIRACTION_STOP;		

	return DIRACTION_CONTINUE;
}

int copyFileAction(const char *fileName,struct stat * statbuf,void* userData)
{
	pair<const char *,const char *> *dirs = static_cast<pair<const char *,const char *> *>(userData);

	if( memcmp(fileName,dirs->first,strlen(dirs->first)) != 0 )	
	{
		DDC_LOG(("Error","Copy directioies,file \"%s\" not in source dir\"%s\".",fileName,dirs->first));
		return DIRACTION_STOP;
	}

	ostringstream os;
	os<<dirs->second<<&fileName[strlen(dirs->first)];

	return CopyFile(fileName,os.str().c_str(),FALSE)?DIRACTION_CONTINUE:DIRACTION_STOP;
}

//拷贝目录
int VirCopyDirectory(const char *s,const char *d,int recurse)
{
	DDC_POINTNULL_RETURN(s,DDCE_ARGUMENT_NULL);
	DDC_POINTNULL_RETURN(d,DDCE_ARGUMENT_NULL);

	pair<const char *,const char *> dirs(s,d);

	DDC_BOOL_RETURN( !nm_ddc::exists(s),DDCE_ARGUMENT_ERROR );		

	int ret = 0;
	ret = VirCreateDirectory(d);
	DDC_BOOL_RETURN(ret!=0,DDCE_DISK_OP_ERROR);	

	ret = recursive_dir_action(s,recurse,FALSE,TRUE,copyFileAction,copyDirAction,&dirs);
	return (DIRACTION_CONTINUE == ret)?DDCE_SUCCESS:DDCE_DISK_OP_ERROR;
}

#ifdef _DDC_BOOST
uint file_size(const char *file_name)
{		
	try{
		if( exists(file_name))
			return static_cast<uint>(boost::filesystem::file_size(file_name));	
	}catch(...)	{}
	return 0xFFFFFFFF;
}

uint file_size(const wchar_t *file_name)
{
	try{
		if( exists(file_name))
			return static_cast<uint>(boost::filesystem::file_size(file_name));	
	}catch(...)	{}
	return 0xFFFFFFFF;
}

bool is_directory(const char *file_name)
{
	try	{		
		return boost::filesystem::is_directory(file_name);
	}
	catch(...)	{		return false;	}
	
}

bool is_directory(const wchar_t *file_name)
{
	try	{		
		return boost::filesystem::is_directory(file_name);
	}
	catch(...)	{		return false;	}
}

bool is_empty(const char *file_name)
{
	try{
		return boost::filesystem::is_empty(file_name);
	}	
	catch(...)	{		return false;	}	
}

bool is_empty(const wchar_t *file_name)
{
	try{
		return boost::filesystem::is_empty(file_name);
	}	
	catch(...)	{		return false;	}
}

bool exists(const char *file_name) 
{
	if( file_name == NULL || file_name[0] == 0 )
		return false;

	try{
		return boost::filesystem::exists(file_name);
	}	
	catch(...)	{		return false;	}
}

bool exists(const wchar_t *file_name)
{
	if( file_name == NULL || file_name[0] == 0 )
		return false;
	try{
		return boost::filesystem::exists(file_name);
	}	
	catch(...)	{		return false;	}
}

bool create_directory(const wchar_t *dir_name)
{
	if( dir_name == NULL || dir_name[0] == 0 )
		return false;
	try{
		return boost::filesystem::create_directory(dir_name);		
	}	
	catch(...)	{		return false;	}
}
#else
uint file_size(const char *){return UINT_MAX;}
uint file_size(const wchar_t *){return UINT_MAX;}
bool is_directory(const char *){return false;}
bool is_directory(const wchar_t *){return false;}
bool is_empty(const char *){return true;}
bool is_empty(const wchar_t *){return true;}
bool exists(const char *file_name) 
{
#ifdef _WIN32
	if( (_access( file_name, 0 )) != -1 )
		return true;
	return false;
#endif

}
bool exists(const wchar_t *) {return false;}
bool create_directory(const wchar_t *){return false;}

#endif	//_DDC_BOOST

#ifdef _MSC_VER		//mscv
//Walk down all the directories under the specified location, and do something (something specified
//by the fileAction and dirAction function pointers).

int recursive_dir_action(const char *fileName,
					int recurse, int followLinks, int depthFirst,
					int (*fileAction) (const char *fileName,struct stat * statbuf,void* userData),
					int (*dirAction) (const char *fileName,struct stat * statbuf,void* userData),
					void* userData)
{
	WIN32_FIND_DATA FileData; 
	DDC_HANDLE hSearch; 
	CHAR szFullPath[MAX_PATH_SAFE];
	int ret = DIRACTION_CONTINUE;
	szFullPath[MAX_PATH_SAFE-SAFE_PATH_RESERVED] = szFullPath[MAX_PATH_SAFE-1] = 0;

	if( !nm_ddc::exists(fileName) )
		return DIRACTION_CONTINUE;

	if( !nm_ddc::is_directory(fileName) )
		return fileAction?(*fileAction)(fileName,NULL,userData):DIRACTION_CONTINUE;
	
	_snprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,"%s\\%s",fileName,L"*.*");

	// Start searching for .* files in the current directory. 
	hSearch = FindFirstFile(szFullPath, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		return DIRACTION_CONTINUE;
	} 
	
	// "." or ".." do nothing
	if (0x002e == *(PWORD)FileData.cFileName || 0x2e2e == *(PWORD)FileData.cFileName )
	{}
	else
	{
		_snprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,"%s\\%s",fileName,FileData.cFileName);
		
		if (nm_ddc::is_directory(szFullPath) )
		{				
			if( dirAction )
				ret = (*dirAction)(szFullPath,NULL,userData);

			//递归
			if( TRUE == recurse && ret == DIRACTION_CONTINUE )
				ret = recursive_dir_action(szFullPath,recurse,followLinks,depthFirst,fileAction,dirAction,userData);
		}
		else
			if( fileAction )
				ret = (*fileAction)(szFullPath,NULL,userData);		
	}

	//停止遍历
	if( ret == DIRACTION_STOP )
	{
		FindClose(hSearch);
		return DIRACTION_STOP;
	}

	//改返回值，到此可以继续遍历
	ret = DIRACTION_CONTINUE;

	// Find the rest of files
	while( FindNextFile(hSearch, &FileData) )
	{		
		_snprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,"%s\\%s",fileName,FileData.cFileName);

		// "." or ".." do nothing			
		if (0x002e == *(PWORD)FileData.cFileName || 0x2e2e == *(PWORD)FileData.cFileName )
			continue;


		if (nm_ddc::is_directory(szFullPath) )
		{					
			if( dirAction )
				ret = (*dirAction)(szFullPath,NULL,userData);
			
			if( ret == DIRACTION_STOP )
				break;
	
			//递归
			if( TRUE == recurse && ret == DIRACTION_CONTINUE )
			{
				ret = recursive_dir_action(szFullPath,recurse,followLinks,depthFirst,fileAction,dirAction,userData);
				if( ret == DIRACTION_STOP )
					break;				
			}

			//改返回值，到此可以继续遍历
			ret = DIRACTION_CONTINUE;
		}
		else
		{	//file		
			if( fileAction )
				ret = (*fileAction)(szFullPath,NULL,userData);

			if( ret == DIRACTION_STOP )
				break;
		}		
	
	}	//end while

	// Close the search handle.  
	FindClose(hSearch);	

	return ret;	
}

int recursive_dir_action_w(const wchar_t *fileName,
					 int recurse, int followLinks, int depthFirst,
					 int (*fileAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
					 int (*dirAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
					 void* userData)
{
	WIN32_FIND_DATAW FileData; 
	DDC_HANDLE hSearch; 
	WCHAR szFullPath[MAX_PATH_SAFE];
	int ret = DIRACTION_CONTINUE;
	szFullPath[MAX_PATH_SAFE-SAFE_PATH_RESERVED] = szFullPath[MAX_PATH_SAFE-1] = 0;

	
	if( _waccess(fileName,0) != 0 )
		return DIRACTION_CONTINUE;

	if( !nm_ddc::is_directory(fileName) )
		return fileAction?(*fileAction)(fileName,NULL,userData):DIRACTION_CONTINUE;
	
	_snwprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,L"%s\\%s",fileName,L"*.*");	

	// Start searching for .* files in the current directory. 		
	hSearch = FindFirstFileW(szFullPath, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		return DIRACTION_CONTINUE;
	} 
	
	// "." or ".." do nothing
	if (0x0000002e == *(PDWORD)FileData.cFileName || 0x002e002e == *(PDWORD)FileData.cFileName )
	{}
	else
	{
		_snwprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,L"%s\\%s",fileName,FileData.cFileName);			
		if (nm_ddc::is_directory(szFullPath) )
		{				
			if( dirAction )
				ret = (*dirAction)(szFullPath,NULL,userData);

			//递归
			if( TRUE == recurse && ret == DIRACTION_CONTINUE )
				ret = recursive_dir_action_w(szFullPath,recurse,followLinks,depthFirst,fileAction,dirAction,userData);
		}
		else
			if( fileAction )
				ret = (*fileAction)(szFullPath,NULL,userData);
	}

	//停止遍历
	if( ret == DIRACTION_STOP )
	{
		FindClose(hSearch);
		return DIRACTION_STOP;
	}

	//改返回值，到此可以继续遍历
	ret = DIRACTION_CONTINUE;

	// Find the rest of the .c files
	while( FindNextFileW(hSearch, &FileData) )
	{		
		_snwprintf(szFullPath,MAX_PATH_SAFE-SAFE_PATH_RESERVED,L"%s\\%s",fileName,FileData.cFileName);			
		
		// "." or ".." do nothing			
		if (0x0000002e == *(PDWORD)FileData.cFileName || 0x002e002e == *(PDWORD)FileData.cFileName )
			continue;
		
		if (nm_ddc::is_directory(szFullPath) )
		{					
			if( dirAction )
				ret = (*dirAction)(szFullPath,NULL,userData);

			if( ret == DIRACTION_STOP )
				break;			

			//递归
			if( TRUE == recurse && ret == DIRACTION_CONTINUE )
			{
				ret = recursive_dir_action_w(szFullPath,recurse,followLinks,depthFirst,fileAction,dirAction,userData);
				if( ret == DIRACTION_STOP )
					break;				
			}
			
			//改返回值，到此可以继续遍历
			ret = DIRACTION_CONTINUE;
		}
		else
		{	//file
			if( fileAction )
				ret = (*fileAction)(szFullPath,NULL,userData);

			if( ret == DIRACTION_STOP )
				break;

		}	//end Directory	
	}	//end while

	// Close the search handle.  
	FindClose(hSearch);

	return ret;
}


#elif defined(__GNUC__)	//__GNUC__

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ddc_util.h"


int recursive_dir_action(const char *fileName,
					 int recurse, int followLinks, int depthFirst,
					 int (*fileAction) (const char *fileName,struct stat * statbuf,	void* userData),
					int (*dirAction) (const char *fileName,struct stat * statbuf,	void* userData),
					void* userData)
{
	int ret;
	struct stat statbuf;
	struct dirent *next;

	if (followLinks == TRUE)
		ret = stat(fileName, &statbuf);
	else
		ret = lstat(fileName, &statbuf);

	if (ret < 0) {

		DDC_LOG(("Error","status=%d followLinks=%d TRUE=%d\n",ret, followLinks, TRUE));

		return DIRACTION_STOP;
	}

	if( !nm_ddc::exists(fileName) )
		return DIRACTION_CONTINUE;


	if( !(S_ISDIR(statbuf.st_mode)))
		return fileAction?(*fileAction)(fileName,&statbuf,userData):DIRACTION_CONTINUE;


	DIR *dir;
	char szFullPath[MAX_PATH_SAFE];

	if (dirAction != NULL && depthFirst == FALSE) 
	{
		ret = dirAction(fileName, &statbuf, userData);
		if (ret == DIRACTION_STOP) 
		{
			DDC_LOG(("Error","%s status == FALSE",fileName));			
		}
		return ret;
	}

	dir = opendir(fileName);
	if (!dir) {
		return DIRACTION_CONTINUE;
	}

	ret = DIRACTION_CONTINUE;



	// Find the rest of the .c files
	while ((next = readdir(dir)) != NULL)
	{		
		if ((strcmp(next->d_name, "..") == 0)
			|| (strcmp(next->d_name, ".") == 0))
				continue;
		
		sprintf(szFullPath,"%s/%s",fileName,next->d_name);
		lstat(szFullPath, &statbuf);

		if (S_ISDIR(statbuf.st_mode) )
		{					
			if( dirAction )
				ret = (*dirAction)(szFullPath,&statbuf,userData);

			if( ret == DIRACTION_STOP )
				break;			

			//递归
			if( TRUE == recurse && ret == DIRACTION_CONTINUE )
			{
				ret = recursive_dir_action(szFullPath,recurse,followLinks,depthFirst,fileAction,dirAction,userData);
				if( ret == DIRACTION_STOP )
					break;				
			}

			//改返回值，到此可以继续遍历
			ret = DIRACTION_CONTINUE;
		}
		else
		{	//file
			if( fileAction )
				ret = (*fileAction)(szFullPath,&statbuf,userData);

			if( ret == DIRACTION_STOP )
				break;

		}	//end Directory	
	}	//end while

	// Close the search handle.  
	closedir(dir);
	
	return ret;
}

typedef int (*_actionW) (const wchar_t *fileName,struct stat * statbuf,void* userData);

int fileAction_a2w (const char *fileName,struct stat * statbuf,	void* userData)
{
	wchar_t fileNameW[520];
	mbstowcs(fileNameW,fileName,520);

	
	pair<void *,pair<_actionW,_actionW> > *pairUserData = (pair<void *,pair<_actionW,_actionW> >*)userData;
	_actionW fileAction = pairUserData->second.first;
	if( fileAction )
		return fileAction(fileNameW,statbuf,pairUserData->first);
	return DIRACTION_CONTINUE;
}

int dirAction_a2w (const char *fileName,struct stat * statbuf,	void* userData)
{
	wchar_t fileNameW[520];
	mbstowcs(fileNameW,fileName,520);

	pair<void *,pair<_actionW,_actionW> > *pairUserData = (pair<void *,pair<_actionW,_actionW> >*)userData;
	_actionW dirAction = pairUserData->second.second;
	if( dirAction)
		return dirAction(fileNameW,statbuf,pairUserData->first);
	return DIRACTION_CONTINUE;
}

int recursive_dir_action_w(const wchar_t *fileName,
						   int recurse, int followLinks, int depthFirst,
						   int (*fileAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   int (*dirAction) (const wchar_t *fileName,struct stat * statbuf,void* userData),
						   void* userData)
{
	char fileNameA[1024];
	wcstombs(fileNameA,fileName,wcslen(fileName)+1);
	
	pair<void *,pair<_actionW,_actionW> > pairUserData(userData,make_pair(fileAction,dirAction) );
	return recursive_dir_action(fileNameA,recurse,followLinks,depthFirst,fileAction?fileAction_a2w:NULL,dirAction?dirAction_a2w:NULL,&pairUserData);
}
#endif	// PlatForm

}//namespace nm_ddc
