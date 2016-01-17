/**
* Copyright(C)2013,RUN  All rights reserved.
* @file
* @brief crc32
* @author ����ΰ chengwei@run.com
* @date 2013-07-30
* @version 1.0
* @note 
*************************************************************************/

#ifndef _WIX_LZ77_COMPRESS_HEADER_001_
#define _WIX_LZ77_COMPRESS_HEADER_001_

/**
* @brief ����crc
* @param buf ������ָ��
* @param size ����������
* @return crc32ֵ
*/
extern unsigned int crc32(const unsigned char *buf, size_t size);

/**
* @brief �����ļ�crc
* @param file_name �ļ���
* @return �ļ�crc32ֵ
*/
extern unsigned int file_crc(const char *file_name);

/**
* @brief ����crc�����ڷֲ��������������crcֵ�����ս��Ҫȡ��
* 
* @param old_value crc�����ֵ
* @param buf ������ָ��
* @param size ����������
* @return crc32ֵ
*/
extern unsigned int crc32_process(unsigned int old_value,const unsigned char *buf ,size_t size);



#endif // _WIX_LZW_COMPRESS_HEADER_001_



