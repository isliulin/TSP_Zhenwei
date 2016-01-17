#ifndef __TSP_SCREENSPY_H__
#define __TSP_SCREENSPY_H__

#include "CursorInfo.h"

//两种算法
enum
{
	ALGORITHM_SCAN = 1,		//速度很快,但碎片太多
	ALGORITHM_DIFF = 2,		//速度很慢,也占CPU,但是数据量都是最小的
};

class CScreenSpy  
{
public:
	CScreenSpy(int biBitCount= 8, BOOL bIsGray= FALSE, UINT nMaxFrameRate = 100);
	virtual ~CScreenSpy();

public:
	LPVOID GetFirstScreen();
	LPVOID GetNextScreen(LPDWORD lpdwBytes);

	void SetAlgorithm(UINT nAlgorithm);
	LPBITMAPINFO GetBI();
	UINT GetBISize();
	UINT GetFirstImageSize();
	void SetCaptureLayer(BOOL bIsCaptureLayer);

private:
	void ScanScreen(HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight); //降低CPU
	int Compare(LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize);
	LPBITMAPINFO ConstructBI(int biBitCount, int biWidth, int biHeight);
	void WriteRectBuffer(LPBYTE	lpData, int nCount);
	BOOL ScanChangedRect(int nStartLine);
	void CopyRect(LPRECT lpRect);
	BOOL SelectInputWinStation();

private:
	BYTE m_bAlgorithm;
	UINT m_nMaxFrameRate;
	BOOL m_bIsGray;
	DWORD m_dwBitBltRop;
	DWORD m_dwLastCapture;
	DWORD m_dwSleep;
	LPBYTE m_rectBuffer;
	UINT m_rectBufferOffset;
	BYTE m_nIncSize;

	int m_nFullWidth;
	int m_nFullHeight;
	int m_nStartLine;

	RECT m_changeRect;

	HDC m_hFullDC;
	HDC m_hLineMemDC;
	HDC m_hFullMemDC;
	HDC m_hRectMemDC;

	HBITMAP m_hLineBitmap;
	HBITMAP m_hFullBitmap;

	LPVOID m_lpvLineBits;
	LPVOID m_lpvFullBits;

	LPBITMAPINFO m_lpbmi_line;
	LPBITMAPINFO m_lpbmi_full;
	LPBITMAPINFO m_lpbmi_rect;

	int	m_biBitCount;
	int	m_nDataSizePerLine;

	LPVOID m_lpvDiffBits; // 差异比较的下一张
	HDC	m_hDiffDC;
	HDC m_hDiffMemDC;
	HBITMAP	m_hDiffBitmap;

	CCursorInfo	m_CursorInfo;
	HWND m_hDeskTopWnd;
};

#endif //__TSP_SCREENSPY_H__
