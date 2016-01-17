#pragma once
//extern "C" _declspec(dllexport) bool TSPSMSAlarm(int id);  //传入上线机器ID

extern "C" _declspec(dllimport) bool TSPSMSAlarm(int id);  //传入上线机器ID