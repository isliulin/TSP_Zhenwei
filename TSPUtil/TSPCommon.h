#pragma once

extern "C" _declspec(dllimport) BOOL MD5Calc_FILE(char *szFileName,char *strMD5 );  //计算MD5函数
extern "C" _declspec(dllimport) int Compress(const char * DestName,const char *SrcName);//压缩函数。
extern "C" _declspec(dllimport) int UnCompress(const char * DestName,const char *SrcName);//解压缩函数
extern "C" _declspec(dllimport) int qqencrypt( unsigned char* instr, int instrlen, unsigned char* key,
											  unsigned char* outstr,int* outstrlen_ptr);  //加密函数
extern "C" _declspec(dllimport) int qqdecrypt( unsigned char* instr, int instrlen, unsigned char* key,
											  unsigned char* outstr,int* outstrlen_ptr);  //解密函数

