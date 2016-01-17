#pragma once 
#include <Windows.h>

#define LOG_ERROR   0
#define LOG_INFO    1
#define LOG_DEBUG   2

extern "C" _declspec(dllimport) void SetLogFileName(TCHAR *_tchFName);
extern "C" _declspec(dllimport) BOOL OpenLogFile();
extern "C" _declspec(dllimport) BOOL LogInfo(int _logLevel,char *msg,...);
extern "C" _declspec(dllimport) void SetLevel(int iLevel);
extern "C" _declspec(dllimport) void DestoryLogobj();
extern "C" _declspec(dllimport) void SetLogFileSize(int iSize);