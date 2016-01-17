#include <iostream>

#include "ddc_util.h"
#include "ddc_dlopen.h"
#include "ddc_debuglog.h"

int CVirusDllHandle::open_called_ = 0;

CVirusDllHandle::CVirusDllHandle (void)
	: refcount_ (0),dll_name_ (""),handle_ (INVALID_HANDLE_VALUE),open_mode_(0)
{
	DDC_LOG(("Trace","CVirusDllHandle::CVirusDllHandle\n"));
}

CVirusDllHandle::~CVirusDllHandle (void)
{
	DDC_LOG(("Trace","CVirusDllHandle::~CVirusDllHandle\n"));
	this->close (1);	
}

const string &CVirusDllHandle::dll_name (void) const
{
	DDC_LOG(("Trace","CVirusDllHandle::dll_name\n"));
	return this->dll_name_;
}

int CVirusDllHandle::open (const string &name, int open_mode, DDC_HANDLE handle)
{
	DDC_LOG(("Trace","CVirusDllHandle::open(%s)\n",name.c_str() ));	

	// Once dll_name_ has been set, it can't be changed..
	this->dll_name_ = name;
	open_mode_ = open_mode;

	if (!this->open_called_)
		this->open_called_ = 1;

	// If it hasn't been loaded yet, go ahead and do that now.
	if (this->handle_ == INVALID_HANDLE_VALUE )
	{
		if (handle)
			this->handle_ = handle;
		else
		{
			this->handle_ = (DDC_HANDLE)LoadLibrary(this->dll_name_.c_str ());
			if (this->handle_ == 0 )   // Good one
			{
				DDC_LOG(("Error","CVirusDllHandle::open: calling dlopen on \"%s\",Lasterror=%d\n", name.c_str(),GetLastError()));
				return -1;
			}
		}
	}
	
	DDC_LOG(("Trace","CVirusDllHandle::open: loading %s (%d)\n",this->dll_name_.c_str(),this->handle_));

	++this->refcount_;
	return 0;
}

int CVirusDllHandle::close (int unload)
{
	DDC_LOG(("Trace","CVirusDllHandle::close\n"));	

	int ret = 0;

	// Since we don't actually unload the dll as soon as the refcount
	// reaches zero, we need to make sure we don't decrement it below
	// zero.
	if (this->refcount_ > 0)
		--this->refcount_;
	else
		this->refcount_ = 0;

	if (this->refcount_ == 0 &&	this->handle_ != INVALID_HANDLE_VALUE && unload == 1)
	{
		DDC_LOG(("Warning","CVirusDllHandle::close: unloading %s (%d)\n",this->dll_name_.c_str(),this->handle_));
		
		ret = FreeLibrary((HMODULE)this->handle_) >0?0:DDCE_MODULE_ERROR;
		this->handle_ = INVALID_HANDLE_VALUE;
	}

	if (ret != 0 )
		DDC_LOG(("Error","CVirusDllHandle::close error: \"%s\".\n",this->error ().c_str ()));

	return ret;
}

uint CVirusDllHandle::refcount (void) const
{
	return this->refcount_;
}

void *CVirusDllHandle::symbol (const char*sym_name, int ignore_errors)
{
	DDC_LOG(("Trace","CVirusDllHandle::symbol\n"));	
	if (this->handle_ != INVALID_HANDLE_VALUE)
	{
		void *sym =  (void *)GetProcAddress((HMODULE)this->handle_, sym_name);		

		if (!sym && ignore_errors != 1)
		{
			DDC_LOG(("Error","CVirusDllHandle::symbol (\"%s\") \"%s\".\n",dll_name_.c_str(),this->error ().c_str ()));

			return 0;
		}
		return sym;
	}
	return 0;
}

DDC_HANDLE CVirusDllHandle::get_handle (int become_owner)
{
	DDC_LOG(("Trace","CVirusDllHandle::get_handle\n"));
	

	DDC_HANDLE handle = INVALID_HANDLE_VALUE;

	if (this->refcount_ == 0 && become_owner != 0)
	{
		DDC_LOG(("Error","CVirusDllHandle::get_handle: cannot become owner, refcount == 0.\n"));

		return INVALID_HANDLE_VALUE;
	}

	handle = this->handle_;

	if (become_owner != 0)
	{
		if (--this->refcount_ == 0)
			this->handle_ = INVALID_HANDLE_VALUE;
	}

	DDC_LOG(("Trace","CVirusDllHandle::get_handle: post call: handle %s, refcount %d\n",
			this->handle_ == INVALID_HANDLE_VALUE?"invalid": "valid",this->refcount_));

	return handle;
}

// This method is used return the last error of a library operation.

string CVirusDllHandle::error (void)
{
	//DDC_LOG(("Trace","CVirusDllHandle::error\n"));	ª·µ›πÈÀ¿À¯
	int e = GetLastError();
	if( e )
		return "error";
	else
		return "no error";
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pointer to the Singleton instance.

#ifdef _MSC_VER
CVirusDllManagerImp* SingletonHolder<CVirusDllManagerImp>::pInstance_ = 0;
#endif

CVirusDllManagerImp::CVirusDllManagerImp(int size)
	: handle_vector_ (0),total_size_ (0)
{
	DDC_LOG(("Trace","CVirusDllManagerImp::CVirusDllManagerImp\n"));

	if (this->open (size) != 0 )
		DDC_LOG(("Error","CVirusDllManagerImpctor failed to allocate handle_vector_.\n"));
}

CVirusDllManagerImp::~CVirusDllManagerImp(void)
{
	DDC_LOG(("Trace","CVirusDllManagerImp::~CVirusDllManagerImp\n"));

	if (this->close () != 0 )
		DDC_LOG(("Error","CVirusDllManagerImpdtor failed to close properly.\n"));
}

CVirusDllHandle *CVirusDllManagerImp::open_dll (const char *dll_name,int open_mode,DDC_HANDLE handle)
{
	DDC_LOG(("Trace","CVirusDllManagerImp::open_dll\n"));	

	CVirusDllHandle *dll_handle = this->find_dll (dll_name);
	if (!dll_handle)
	{
		if ( this->handle_vector_.size() < this->total_size_)
		{
			DDC_NEW_RETURN(dll_handle,CVirusDllHandle,0);
			
			this->handle_vector_.push_back(dll_handle);			
		}
	}

	if (dll_handle)
	{
		if (dll_handle->open (dll_name, open_mode, handle) != 0)
		{
			// Don't worry about freeing the memory right now, since
			// the handle_vector_ will be cleaned up automatically
			// later.

			DDC_LOG(("Error","CVirusDllManagerImp::open_dll: Could not open dll %s.",dll_name));

			return 0;
		}
	}
	return dll_handle;
}

int CVirusDllManagerImp::close_dll (const char *dll_name)
{
	DDC_LOG(("Trace","CVirusDllManagerImp::close_dll\n"));
	
	CVirusDllHandle *handle = this->find_dll (dll_name);
	if (handle)
	{
		return this->unload_dll (handle, 1);		// force unload
	}

	return -1;
}

int CVirusDllManagerImp::open (int size)
{
	DDC_LOG(("Trace", "CVirusDllManagerImp::open\n"));
	 this->total_size_ = size;
	return 0;
}

int CVirusDllManagerImp::close (void)
{
	DDC_LOG(("Trace","CVirusDllManagerImp::close\n"));

	int force_close = 1;

	// Delete components in reverse order.		
	for(vector<CVirusDllHandle *>::reverse_iterator iter=this->handle_vector_.rbegin();iter!=this->handle_vector_.rend();++iter)
	{
		if( *iter )
		{
			this->unload_dll(*iter,force_close);
			delete (*iter);
		}
	}

	this->handle_vector_.clear();		
	
	return 0;
}

CVirusDllHandle *CVirusDllManagerImp::find_dll (const char*dll_name) const
{
	DDC_LOG(("Trace","CVirusDllManagerImp::find_dll\n"));

	;
	for (uint i = 0; i < this->handle_vector_.size(); i++)
		if (this->handle_vector_[i] &&
			strcmp (this->handle_vector_[i]->dll_name ().c_str(), dll_name) == 0)
		{
			return this->handle_vector_[i];
		}

		return 0;
}

int CVirusDllManagerImp::unload_dll (CVirusDllHandle *dll_handle, int force_unload)
{
	DDC_LOG(("Trace", "CVirusDllManagerImp::unload_dll\n"));

	if (dll_handle)
	{
		int unload = force_unload;

		if (dll_handle->close (unload) != 0)
		{
			DDC_LOG(("Error","CVirusDllManagerImp::unload error.\n"));
			return -1;
		}
	}
	else
	{
		DDC_LOG(("Error","CVirusDllManagerImp::unload_dll called with null pointer.\n"));
		return -1;
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



CVirusDll::CVirusDll (int close_handle_on_destruction)
				: open_mode_ (0),
				close_handle_on_destruction_ (close_handle_on_destruction),
				dll_handle_ (0),
				error_ (0)
{
	DDC_LOG(("Trace","CVirusDll::CVirusDll (int)\n"));
}

CVirusDll::CVirusDll (const CVirusDll &rhs)
				: open_mode_ (0),
				close_handle_on_destruction_ (0),
				dll_handle_ (0),
				error_ (0)
{
	DDC_LOG(("Trace","CVirusDll::CVirusDll (const CVirusDll &)\n"));

	if (rhs.dll_name_!=""
		// This will automatically up the refcount.
		&& this->open (rhs.dll_name_,rhs.open_mode_,rhs.close_handle_on_destruction_) != 0)
			DDC_LOG(("Error","CVirusDll::copy_ctor: error: %s",this->error ().c_str()));
}

// If the library name and the opening mode are specified than on
// object creation the library is implicitly opened.

CVirusDll::CVirusDll (const string &dll_name,
				  int open_mode,
				  int close_handle_on_destruction)
				  : open_mode_ (open_mode),				  
				  close_handle_on_destruction_ (close_handle_on_destruction),
				  dll_handle_ (0),
				  error_ (0)
{
	DDC_LOG(("Trace","CVirusDll::CVirusDll\n"));

	if (this->open (dll_name, this->open_mode_, close_handle_on_destruction) != 0)
		DDC_LOG(("Error","CVirusDll::open: error calling open: %s",this->error ().c_str()));
}


CVirusDll::~CVirusDll (void)
{
	DDC_LOG(("Trace","CVirusDll::~CVirusDll\n"));

	this->close ();
}


int CVirusDll::open (const string &dll_filename,
			   int open_mode,
			   int close_handle_on_destruction)
{
	DDC_LOG(("Trace","CVirusDll::open\n"));

	return open_i (dll_filename, open_mode, close_handle_on_destruction);
}

int
CVirusDll::open_i (const string &dll_filename,
				 int open_mode,
				 int close_handle_on_destruction,
				 DDC_HANDLE handle)
{
	DDC_LOG(("Trace","CVirusDll::open_i\n"));

	this->error_ = 0;
	
	if ( dll_filename == "" )
	{
		DDC_LOG(("Error","CVirusDll::open_i: dll_name is %s",this->dll_name_ == "" ? "(null)": this->dll_name_.c_str()));
		return -1;
	}

	if (this->dll_handle_)
	{
		// If we have a good handle and its the same name, just return.
		if ( this->dll_name_ == dll_filename )
			return 0;
		else
			this->close ();
	}

	this->dll_name_ = dll_filename;
	this->open_mode_ = open_mode;
	this->close_handle_on_destruction_ = close_handle_on_destruction;

	this->dll_handle_ = CVirusDllManager::Instance().open_dll (this->dll_name_.c_str(),this->open_mode_,handle);

	if (!this->dll_handle_)
		this->error_ = 1;

	return this->error_ ? -1 : 0;
}

// The symbol refernce of the name specified is obtained.

void *
CVirusDll::symbol (const string &sym_name, int ignore_errors)
{
	DDC_LOG(("Trace","CVirusDll::symbol\n"));

	this->error_ = 0;

	void *sym = 0;
	if (this->dll_handle_)
		sym = this->dll_handle_->symbol (sym_name.c_str(), ignore_errors);

	if (!sym)
		this->error_ = 1;

	return sym;
}

// The library is closed using the DDC_SHLIB_HANDLE object, i.e., the
// shared object is now disassociated form the current process.

int CVirusDll::close (void)
{
	DDC_LOG(("Trace","CVirusDll::close\n"));

	int retval = 0;

	if (this->dll_handle_
		&& this->close_handle_on_destruction_
		&& this->dll_name_==""
		&& (retval = CVirusDllManager::Instance ().close_dll (this->dll_name_.c_str())) != 0)
		this->error_ = 1;

	// Even if close_dll() failed, go ahead and cleanup.
	this->dll_handle_ = 0;	
	this->close_handle_on_destruction_ = 0;

	return retval;
}

// This method is used return the last error of a library operation.

string CVirusDll::error (void) const
{
	DDC_LOG(("Trace","CVirusDll::error\n"));
	if (this->error_)
		return 	"Error: check log for details.";

	return "";
}

// Return the handle to the user either temporarily or forever, thus
// orphaning it. If 0 means the user wants the handle forever and if 1
// means the user temporarily wants to take the handle.

DDC_HANDLE CVirusDll::get_handle (int become_owner) const
{
	DDC_LOG(("Trace","CVirusDll::get_handle\n"));

	DDC_HANDLE handle = INVALID_HANDLE_VALUE;

	if (this->dll_handle_)
		handle = this->dll_handle_->get_handle (become_owner);

	return handle;
}

// Set the handle for the DLL. By default, the object will be closed
// before it is destroyed.

int CVirusDll::set_handle (DDC_HANDLE handle,int close_handle_on_destruction)
{
	DDC_LOG(("Trace","CVirusDll::set_handle\n"));

	return DDCE_UNSUPPORT;

	// Create a unique name.  Note that this name is only quaranteed
	// to be unique for the life of this object.
	char temp[MAX_PATH];
	//DDC_OS::unique_name (this, temp, DDC_UNIQUE_NAME_LEN);

	return this->open_i (temp, 1, close_handle_on_destruction, handle);
}


