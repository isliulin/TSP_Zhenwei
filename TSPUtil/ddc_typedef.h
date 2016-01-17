#if !defined(__DDC_TYPEDEF_H__)
#define __DDC_TYPEDEF_H__

//#include "tchar.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;
typedef signed char	schar;
typedef	signed short	sshort;
typedef signed int	sint;
typedef signed long	slong;


// ACE Global_Macros.h
# define DDC_BIT_ENABLED(WORD,BIT) (((WORD) & (BIT)) != 0)
# define DDC_BIT_DISABLED(WORD,BIT) (((WORD) & (BIT)) == 0)
# define DDC_BIT_CMP_MASK(WORD,BIT,MASK) (((WORD) & (BIT)) == MASK)
# define DDC_SET_BITS(WORD,BITS) (WORD |= (BITS))
# define DDC_CLR_BITS(WORD,BITS) (WORD &= ~(BITS))

#ifndef UINT_MAX
#define UINT_MAX		0xffffffff
#endif

//disable warning
#ifndef UNUSED
#define UNUSED(x) ( (void)(x) )
#endif

#define MP_CP_CN 936

#define DDC_RIGHT_SLASH(VALUE)	{ char *_ddc_right_slash_p = strrchr(VALUE,DDC_C_SLASH);	if( *_ddc_right_slash_p ) *(_ddc_right_slash_p+1) = 0; }
#define DDC_RIGHT_WSLASH(VALUE)	{ wchar_t *_ddc_right_slash_p = wcsrchr(VALUE,DDC_WC_SLASH);	if( *_ddc_right_slash_p ) *(_ddc_right_slash_p+1) = 0; }


#ifdef _DEBUG
#define DDC_TRY
#define DDC_CATCH
#else
#define DDC_TRY	try
#define DDC_CATCH	\
	catch(CDdcException &e)	\
{	\
	LOG(("Exception","throw %x",e.GetValue()));	\
	return e.GetValue();	\
	}	\
	catch(...)	\
{	\
	LOG(("Exception","throw unknown exception"));	\
	return DDCE_EXCEPTION;	\
	}
#endif

//empty class
struct CEmptyClass{};

//exception base class
class CDdcException
{
public:
	CDdcException(int value):_value(value){};
	int GetValue(void) const { return _value;};	
protected:
	int _value;
};



#endif	//__DDC_THRED_H__

