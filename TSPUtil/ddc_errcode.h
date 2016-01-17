#if !defined(__DDC_ERRCODE_H__)
#define __DDC_ERRCODE_H__


//errorcode define
#define DDC_ERROR_BASE				100000

#define DDCE_SUCCESS						0							//success
#define DDCE_UNKNOWN						DDC_ERROR_BASE+0			//unknown
#define DDCE_UNSUPPORT						DDC_ERROR_BASE+1			// un support
#define DDCE_NO_ACCESS						DDC_ERROR_BASE+2			// access failed
#define DDCE_EXCEPTION						DDC_ERROR_BASE+3			// exception
#define DDCE_TIMEOUT						DDC_ERROR_BASE+4			// time out
#define DDCE_CALL_FAIL						DDC_ERROR_BASE+5			//call other function error

#define DDCE_MEMORY_ALLOC_ERROR				DDC_ERROR_BASE+10			//alloc memory error
#define DDCE_MEMORY_FREE_ERROR				DDC_ERROR_BASE+11			//free memory error
#define DDCE_OUT_OF_BOUNDER					DDC_ERROR_BASE+12			//
#define DDCE_UNSUPPORT_VERSION				DDC_ERROR_BASE+13			//
#define DDCE_PACKET_LEN_ERROR				DDC_ERROR_BASE+14			//origin packet length < 0
#define DDCE_PACKET_NULL					DDC_ERROR_BASE+15			//origin packet buffer ptr NULL
#define DDCE_CONVERT_VERSION_ERROR			DDC_ERROR_BASE+16			//version convert error
#define DDCE_RECORD_NOT_FOUND				DDC_ERROR_BASE+17			//record not found
#define DDCE_ENCRYPT_ERROR					DDC_ERROR_BASE+18			//
#define DDCE_DECRYPT_ERROR					DDC_ERROR_BASE+19			//

#define DDCE_INVALID_HANDLE					DDC_ERROR_BASE+20			//invalid error
#define DDCE_INVALID_PATH					DDC_ERROR_BASE+21			//invalid path
#define	DDCE_AERGUMENT_LENGTH_LIMIT			DDC_ERROR_BASE+22			//argument length limit
#define DDCE_ARGUMENT_ERROR					DDC_ERROR_BASE+23			//argument error
#define DDCE_ARGUMENT_NULL					DDC_ERROR_BASE+24			//argument null
#define DDCE_NOT_INITIALIZE					DDC_ERROR_BASE+25			//not initialize
#define DDCE_TYPE_ERROR						DDC_ERROR_BASE+26			//type error
#define DDCE_COMPRESS_ERROR					DDC_ERROR_BASE+27			//compress error
#define DDCE_DECOMPRESS_ERROR				DDC_ERROR_BASE+28			//decompress error
#define DDCE_NOT_MATCH						DDC_ERROR_BASE+29			//record does not match

#define DDCE_DISK_OP_ERROR					DDC_ERROR_BASE+30			//
#define DDCE_FILE_TYPE_ERROR				DDC_ERROR_BASE+31			//file type error
#define DDCE_FILE_LENGTH_ERROR				DDC_ERROR_BASE+32			//
#define DDCE_OPEN_FILE_ERROR				DDC_ERROR_BASE+33			//open file error
#define DDCE_READ_FILE_ERROR				DDC_ERROR_BASE+34			//read file error
#define DDCE_WRITE_FILE_ERROR				DDC_ERROR_BASE+35			//write file error
#define	DDCE_CLOSE_FILE_ERROR				DDC_ERROR_BASE+36			//close file error
#define DDCE_SIZE_FILE_ERROR				DDC_ERROR_BASE+37			//file size error
#define DDCE_END_OF_FILE					DDC_ERROR_BASE+38			//end of file
#define DDCE_FILE_NOT_EXIST					DDC_ERROR_BASE+39			//file not exist

#define DDCE_MODULE_ERROR					DDC_ERROR_BASE+40			//dll error
#define DDCE_STATUS_ERROR					DDC_ERROR_BASE+41			//status error

#define	DDCE_SOCKET_ERROR					DDC_ERROR_BASE+50			//
#define DDCE_NET_CONNECT_FAIL				DDC_ERROR_BASE+51			//
#define DDCE_NET_SEND_ERROR					DDC_ERROR_BASE+52			//
#define DDCE_NET_RECEIVE_ERROR				DDC_ERROR_BASE+53			//
#define DDCE_NET_TIMEOUT					DDC_ERROR_BASE+54			//

#define DDCE_DB_OP_ERROR					DDC_ERROR_BASE+60			// database operator error
#define DDCE_DB_QUERY_ERROR					DDC_ERROR_BASE+61			// database query error
#define	DDCE_DB_CONNECT_ERROR				DDC_ERROR_BASE+62			// database connect error

#endif	//__DDC_ERRCODE_H__

