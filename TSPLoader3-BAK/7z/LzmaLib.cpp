/* LzmaLib.c -- LZMA library wrapper
2008-08-05
Igor Pavlov
Public domain */
#include "stdafx.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"
#include "LzmaLib.h"

/*#ifdef __cplusplus*/
extern "C" {
/*#endif*/

static void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };



SRes LzmaUncompress(unsigned char *dest, size_t  *destLen, const unsigned char *src, size_t  *srcLen,
  const unsigned char *props, size_t propsSize)
{
  ELzmaStatus status;
  return LzmaDecode(dest, destLen, src, srcLen, props, (unsigned)propsSize, LZMA_FINISH_ANY, &status, &g_Alloc);
}


/*#ifdef __cplusplus*/
}
/*#endif*/

