/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief crc32
* @author 刘成伟 chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef _WIX_LZ77_COMPRESS_HEADER_001_
#define _WIX_LZ77_COMPRESS_HEADER_001_

/**
* @brief 计算crc
* @param buf 缓冲区指针
* @param size 缓冲区长度
* @return crc32值
*/
extern unsigned int crc32(const unsigned char *buf, size_t size);

/**
* @brief 计算文件crc
* @param file_name 文件名
* @return 文件crc32值
*/
extern unsigned int file_crc(const char *file_name);

/**
* @brief 计算crc，用于分部计算个缓冲区的crc值，最终结果要取反
* 
* @param old_value crc计算初值
* @param buf 缓冲区指针
* @param size 缓冲区长度
* @return crc32值
*/
extern unsigned int crc32_process(unsigned int old_value,const unsigned char *buf ,size_t size);



#endif // _WIX_LZW_COMPRESS_HEADER_001_



