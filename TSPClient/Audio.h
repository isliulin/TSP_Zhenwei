// Audio.h: interface for the CAudio class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUDIO_H__4DD286A3_85E9_4492_A1A5_C3B2D860BD1A__INCLUDED_)
#define AFX_AUDIO_H__4DD286A3_85E9_4492_A1A5_C3B2D860BD1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <WinSock2.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>


class CAudio
{
public:
	CAudio();
	virtual ~CAudio();

private:
	typedef enum
	{
		AUDIOCAPBUF_NUM		= 5,
	};

	int m_nBufferLength;

	LPBYTE m_lpInAudioData[AUDIOCAPBUF_NUM]; // 保持声音的连继
	LPBYTE m_lpCurBuffer;
	LPBYTE m_lpOutAudioData[AUDIOCAPBUF_NUM];

	HWAVEIN m_hWaveIn;
	int m_nWaveInIndex;
	int m_nWaveOutIndex;

	HANDLE	m_hEventWaveIn; // MM_WIM_DATA触发
	HANDLE	m_hStartRecord; // getRecordBuffer 返回后触发

public:
	LPBYTE getRecordBuffer(LPDWORD lpdwBytes);
	BOOL playBuffer(LPBYTE lpWaveBuffer, DWORD dwBytes);

private:


	HANDLE	m_hThreadCallBack;

	LPWAVEHDR m_lpInAudioHdr[AUDIOCAPBUF_NUM];
	LPWAVEHDR m_lpOutAudioHdr[AUDIOCAPBUF_NUM];

	HWAVEOUT m_hWaveOut;
	
	BOOL	m_bIsWaveInUsed;
	BOOL	m_bIsWaveOutUsed;
	GSM610WAVEFORMAT m_GSMWavefmt;

	BOOL InitializeWaveIn();
	BOOL InitializeWaveOut();

	static DWORD WINAPI waveInCallBack(LPVOID lpVoid);
};

#endif // !defined(AFX_AUDIO_H__4DD286A3_85E9_4492_A1A5_C3B2D860BD1A__INCLUDED_)
