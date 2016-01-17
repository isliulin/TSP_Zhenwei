#if !defined(__DDC_UTIL_H__)
#define __DDC_UTIL_H__


#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <list>

//private type and macro define
#include "ddc_typedef.h"

#if defined (_MSC_VER)
#pragma warning(disable:4786)
//# pragma pack (push, 8)
#include "windows.h"
#include "ddc_os_win.h"

#elif defined(__GNUC__)
#include "ddc_os_linux.h"
#endif

//ddc errorcode define
#include "ddc_errcode.h"


//singleton
#include "ddc_singleton.hpp"

//ddc memory allocation strategy
#include "ddc_memory.h"

#endif	//__DDC_UTIL_H__

