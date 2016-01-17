#ifndef __DDC_MISC_H__
#define __DDC_MISC_H__

extern unsigned short crc16(const unsigned char *buf, size_t size);
extern unsigned int crc32(const unsigned char *buf, size_t size);
extern unsigned int ddc_crc32(unsigned int,const unsigned char *,size_t);
extern unsigned int file_crc(const char *file_name);

#endif // __DDC_MISC_H__



