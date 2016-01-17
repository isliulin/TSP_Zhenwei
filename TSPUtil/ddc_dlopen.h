#if !defined(__VIRUS_DLOPEN_H__)
#define __VIRUS_DLOPEN_H__

#include <memory>
#include <string>
#include <vector>

#define ACE_DEFAULT_SHLIB_MODE 0
#define VIRUS_DEFAULT_DLL_MANAGER_SIZE 255

/**
* @class CVirusDllHandle
* @brief 管理一个动态库，提供动态库加载，卸载，取函数地址（符号）等功能
*/
class CVirusDllHandle
{
public:
	CVirusDllHandle (void);
	~CVirusDllHandle (void);
	const string &dll_name () const;
	int open (const string &,int open_mode,DDC_HANDLE handle);
	int close (int unload = 0);
	uint refcount (void) const;
	void *symbol (const char *symbol_name, int ignore_errors = 0);
	DDC_HANDLE get_handle (int become_owner = 0);
private:
	string error (void);	

	// Disallow copying and assignment since we don't handle them.
	CVirusDllHandle (const CVirusDllHandle &);
	void operator= (const CVirusDllHandle &);

private:
	int open_mode_;
	uint refcount_;	
	string dll_name_;	
	DDC_HANDLE handle_;
	static int open_called_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* @class CVirusDllManagerImp
* @brief 动态库管理类的内部实现
*/
class CVirusDllManagerImp
{
public:	
	enum
	{
		DEFAULT_SIZE = VIRUS_DEFAULT_DLL_MANAGER_SIZE
	};

	
	/// Factory for CVirusDllHandle objects.  If one already exits,
	/// its refcount is incremented.
	CVirusDllHandle *open_dll (const char *dll_name,int openmode,DDC_HANDLE handle);

	/// Close the underlying dll.  Decrements the refcount.
	int close_dll (const char *dll_name);	

	/// Default constructor.
	CVirusDllManagerImp (int size = CVirusDllManagerImp::DEFAULT_SIZE);

protected:
	/// Destructor.
	~CVirusDllManagerImp (void);

	// Allocate handle_vector_.
	int open (int size);

	// Close all open dlls and deallocate memory.
	int close (void);

	// Find dll in handle_vector_.
	CVirusDllHandle *find_dll (const char*dll_name) const;

	// Applies strategy for unloading dll.
	int unload_dll (CVirusDllHandle *dll_handle, int force_unload = 0);

private:
	// Disallow copying and assignment since we don't handle these.
	CVirusDllManagerImp (const CVirusDllManagerImp &);
	void operator= (const CVirusDllManagerImp &);

private:

	/// Vector containing all loaded handle objects.
	vector<CVirusDllHandle *> handle_vector_;

	/// Maximum number of handles.
	uint total_size_;
};

/// singleton模式，动态库管理类全局变量
typedef SingletonHolder<CVirusDllManagerImp> CVirusDllManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
* @class CVirusDll
* @brief 动态库管理类,外部接口
*/
class CVirusDll
{
public:
	explicit CVirusDll (int close_handle_on_destruction = 1);
	explicit CVirusDll (const string &dll_name,int open_mode = ACE_DEFAULT_SHLIB_MODE,	int close_handle_on_destruction = 1);

	/// Copy constructor.
	CVirusDll (const CVirusDll &);

	
	int open (const string &dll_name,
		int open_mode = ACE_DEFAULT_SHLIB_MODE,
		int close_handle_on_destruction = 1);

	/// Call to close the DLL object.
	int close (void);

	~CVirusDll (void);

	void *symbol (const string &symbol_name, int ignore_errors = 0);

	/// Returns a pointer to a string explaining that an error occured.  You
	/// will need to consult the error log for the actual error string
	/// returned by the OS.
	string error (void) const;

	DDC_HANDLE get_handle (int become_owner = 0) const;

	/// Set the handle for the DLL object. By default, the close()
	//operation on / the object will be invoked before it is destroyed.
	int set_handle (DDC_HANDLE handle,int close_handle_on_destruction = 1);
private:

	int open_i (const string &dll_name,int open_mode = ACE_DEFAULT_SHLIB_MODE,int close_handle_on_destruction = 1,
		DDC_HANDLE handle = 0);

	// Disallow assignment since we don't handle it.
	void operator= (const CVirusDll &);

private:

	/// Open mode.
	int open_mode_;

	/// Keep track of the name of the loaded dll, so it can be used
	/// to remove framework components, singletons that live in the dll,
	/// prior to unloading the dll in the close() method.
	string dll_name_;

	/// This flag keeps track of whether we should close the handle
	/// automatically when the object is destroyed.
	int close_handle_on_destruction_;

	CVirusDllHandle *dll_handle_;

	/// Flag to record if the last operation had an error.
	int error_;

};






#endif	//__VIRUS_DLOPEN_H__
