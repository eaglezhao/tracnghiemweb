#include "stdafx.h"


#include <aviriff.h>   // defines FCC macro


static bool g_bInitSystemMetrics = false;
static int g_cxBorder = 0;
static int g_cyBorder = 0;
static int g_cyCaption = 0;

static void InitSystemMetrics()
{
	if (!g_bInitSystemMetrics)
	{
		g_cxBorder = GetSystemMetrics(SM_CXBORDER);
		g_cyBorder = GetSystemMetrics(SM_CYBORDER);
		g_cyCaption = GetSystemMetrics(SM_CYCAPTION);
		g_bInitSystemMetrics = true;
	}
}

int SystemBorderWidth() 
{ 
	InitSystemMetrics();
	return g_cxBorder; 
}
int SystemBorderHeight()
{ 
	InitSystemMetrics();
	return g_cyBorder; 
}
int SystemCaptionHeight()
{ 
	InitSystemMetrics();
	return g_cyCaption; 
}


/***************** Graph Building Helpers **********************/

// Most of these are in the DShow SDK docs now

// AddFilter: Add a filter by CLSID

HRESULT AddFilter(
    IGraphBuilder *pGraph,  // Pointer to the Filter Graph Manager.
    const GUID& clsid,      // CLSID of the filter to create.
    LPCWSTR wszName,        // A name for the filter.
    IBaseFilter **ppF)      // Receives a pointer to the filter.
{
    if (!pGraph || ! ppF) return E_POINTER;
    *ppF = 0;
    IBaseFilter *pF = 0;
    HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&pF));
    if (SUCCEEDED(hr))
    {
        hr = pGraph->AddFilter(pF, wszName);
        if (SUCCEEDED(hr))
            *ppF = pF;
        else
            pF->Release();
    }
    return hr;
}


// GetUnconnectedPin: Get the first unconnected pin with a given direction

HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else  // Unconnected, this is the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}


// ConnectFilters: Connect an output pin to a filter, using Intelligent Connect

HRESULT ConnectFilters(
    IGraphBuilder *pGraph, // Filter Graph Manager.
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest)    // Downstream filter.
{
    if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }
#ifdef debug
        PIN_DIRECTION PinDir;
        pOut->QueryDirection(&PinDir);
        _ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

    // Find an input pin on the downstream filter.
    IPin *pIn = 0;
    HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr))
    {
        return hr;
    }
    // Try to connect them.
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
    return hr;
}


// ConnectFilters: Connect one filter to another, using Intelligent Connect

HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc,      // Upstream filter
    IBaseFilter *pDest)     // Downstream filter
{
    if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }

    // Find an output pin on the first filter.
    IPin *pOut = 0;
    HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) 
    {
        return hr;
    }
    hr = ConnectFilters(pGraph, pOut, pDest);
    pOut->Release();
    return hr;
}


// FindConnectedFilter: Find a filter connected to another filter, upstream or downstream

// Note: Returns the first matching filter that it finds.

HRESULT FindConnectedFilter(
    IBaseFilter *pSrc,          // Pointer to the starting filter
    PIN_DIRECTION PinDir,       // Directtion to look (input = upstream, output = downstream)
    IBaseFilter **ppConnected)  // Returns a pointer to the filter that is connected to pSrc
{
	if (!pSrc || !ppConnected) return E_FAIL;

	*ppConnected = NULL;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pSrc->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr) && pTmp)  
            {
				// Return the filter that owns this pin.
				PIN_INFO PinInfo;
				pTmp->QueryPinInfo(&PinInfo);
				pTmp->Release();
				pEnum->Release();
				if (PinInfo.pFilter == NULL)
				{
					// Inconsistent pin state. Something is wrong...
					return E_UNEXPECTED;
				}
				else
				{
					*ppConnected = PinInfo.pFilter;
					return S_OK;
				}
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;
}


// FindCrossbarPin: Find a given pin on a crossbar filter. 

// Each pin represents a physical connector on the card.

HRESULT FindCrossbarPin(
    IAMCrossbar *pXBar,  // Pointer to the crossbar.
    PhysicalConnectorType PhysicalType, // Pin type to match.
    BOOL bInput,   // TRUE = input pin, FALSE = output pin.
    long *pIndex)  // Receives the index of the pin, if found.
{
    // Find out how many pins the crossbar has.
    long cOut, cIn;
    HRESULT hr = pXBar->get_PinCounts(&cOut, &cIn);
    if (FAILED(hr)) return hr;
    // Enumerate pins and look for a matching pin.
    long count = (bInput ? cIn : cOut);
    for (long i = 0; i < count; i++)
    {
        long iRelated = 0;
        long ThisPhysicalType = 0;
        hr = pXBar->get_CrossbarPinInfo(bInput, i, &iRelated,
            &ThisPhysicalType);
        if (SUCCEEDED(hr) && ThisPhysicalType == PhysicalType)
        {
            // Found a match, return the index.
            *pIndex = i;
            return S_OK;
        }
    }
    // Did not find a matching pin.
    return E_FAIL;
}


// ConnectAudio: Try to hook up the audio pins on a TV crossbar filter.

// This function looks for an audio decoder output pin and an audio tuner input
// pin, and route them on the crossbar.

HRESULT ConnectAudio(IAMCrossbar *pXBar, BOOL bActivate)
{
    // Look for the Audio Decoder output pin.
    long i = 0;
    HRESULT hr = FindCrossbarPin(pXBar, PhysConn_Audio_AudioDecoder,
        FALSE, &i);
    if (SUCCEEDED(hr))
    {
        if (bActivate)  // Activate the audio. 
        {
            // Look for the Audio Tuner input pin.
            long j = 0;
            hr = FindCrossbarPin(pXBar, PhysConn_Audio_Tuner, TRUE, &j);
            if (SUCCEEDED(hr))
            {
                return pXBar->Route(i, j);
            }
        }
        else  // Mute the audio
        {
            return pXBar->Route(i, -1);
        }
    }
    return E_FAIL;
}

/***************** Media Type Helpers **********************/

// These do the same thing as the ones in the base class library. 
// I just didn't feel like linking to the base class lib.

void DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt)
	{
		FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}
void FreeMediaType(AM_MEDIA_TYPE& mt)
{
	CoTaskMemFree(mt.pbFormat);
    if (mt.pUnk != 0)
    {
        // pUnk should be NULL always, but this is safest.
        mt.pUnk->Release();
    }
}

HRESULT CopyMediaType(AM_MEDIA_TYPE *pmt, const AM_MEDIA_TYPE *pmtSrc)
{
	if (!pmt) return E_POINTER;
	FreeMediaType(*pmt);

    *pmt = *pmtSrc;
	if (pmtSrc->pbFormat && (pmtSrc->cbFormat > 0))
	{
		pmt->pbFormat = (BYTE*)CoTaskMemAlloc(pmtSrc->cbFormat);
		if (pmt->pbFormat == 0) 
		{
			pmt->cbFormat = 0;
			return E_OUTOFMEMORY;
		}
		memcpy(pmt->pbFormat, pmtSrc->pbFormat, pmtSrc->cbFormat);
	}
	return S_OK;
}



/***************** Misc Helpers **********************/

//-----------------------------------------------------------------------------
// Name: AllocGetWindowText
// Desc: Helper function to get text from a window.
//
// This function allocates a buffer and returns it in pszText. The caller must
// call CoTaskMemFree on the buffer.
//-----------------------------------------------------------------------------

HRESULT AllocGetWindowText(HWND hwnd, TCHAR **pszText)
{
    *pszText = NULL;
    int len = GetWindowTextLength(hwnd) + 1;  // Account for trailing '\0' character 
    if (len > 1) 
    {
        *pszText = (TCHAR*)CoTaskMemAlloc(sizeof(TCHAR) * len);
        
        if (!pszText)
        {
            return E_OUTOFMEMORY;
        }
        int res = GetWindowText(hwnd, *pszText, len);

        // GetWindowText returns 0 if (a) there is no text or (b) it failed.
        // We checked for (a) in GetWindowTextLength, so 0 means failure here.
        return (res > 0 ? S_OK : E_FAIL);
    }
    return S_FALSE; // No text 
}


int AnsiToWide(const char* szSrc, int cchSrc, WCHAR **pwszDest)
{
	WCHAR *wszStr = new WCHAR[cchSrc];
	if (!wszStr)
	{
		return 0;
	}
	int i = MultiByteToWideChar(CP_ACP, 0, szSrc, cchSrc * sizeof(char), 
		wszStr, cchSrc);
	if (i == 0) 
	{
		delete [] wszStr;
	}
	*pwszDest = wszStr;
	return i;
}


// SetBitmapImg - Set a bitmap image on a window
HBITMAP SetBitmapImg(HINSTANCE hinst, WORD nImgId, HWND hwnd)
{
	HBITMAP hBitmap = LoadBitmap(hinst, MAKEINTRESOURCE(nImgId));
	if (hBitmap)
	{
		SendMessage(hwnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		return hBitmap;
	}
	return 0;
}


#define GUID_Data2     0
#define GUID_Data3     0x10
#define GUID_Data4_1   0xaa000080
#define GUID_Data4_2   0x719b3800

// GuidIsFcc:
// Returns TRUE if a GUID is a valid subtype that represents a specified FOURCC code.
// FOURCCs are mapped to a special range of GUIDs. For more info, see FOURCCMap class
// in the DShow SDK.

bool GuidIsFcc(GUID& guid, DWORD fcc)
{
	return ((guid.Data1 == fcc) &&
			(guid.Data2 == GUID_Data2) &&
			(guid.Data3 == GUID_Data3) &&
			( ((DWORD*)guid.Data4)[0] == GUID_Data4_1) &&
		    ( ((DWORD*)guid.Data4)[1] == GUID_Data4_2));
}




// VideoSubtypeName: Return the name of the video subtype, if known
//                   (I'm only checking for common video capture formats.)

const TCHAR* VideoSubtypeName(GUID& guid)
{
	if (guid == MEDIASUBTYPE_RGB565)
	{
		return TEXT("RGB565");
	}
	else if (guid == MEDIASUBTYPE_RGB555)
	{
		return TEXT("RGB555");
	}
	else if (guid == MEDIASUBTYPE_RGB24)
	{
		 return TEXT("RGB24");
	}
	else if (guid == MEDIASUBTYPE_RGB32)
	{
		 return TEXT("RGB32");
	}
	else if ((guid == MEDIASUBTYPE_YVU9) || (guid == MEDIASUBTYPE_Y411))
	{
		 return TEXT("YVU9");
	}
	else if (guid == MEDIASUBTYPE_YUY2)
	{
		 return TEXT("YUY2");
	}
	else if (guid == MEDIASUBTYPE_UYVY)
	{
		 return TEXT("UYVY");
	}
	else if (guid == MEDIASUBTYPE_Y211)
	{
		 return TEXT("Y211");
	}
	else if (guid == MEDIASUBTYPE_IYUV)
	{
		 return TEXT("IYUV");
	}
	else if (guid == MEDIASUBTYPE_dvsd)
	{
		return TEXT("DV(SD)");
	}

	// what is MEDIASUBTYPE_dvc ?

	else
	{

		// Some YUV types don't get a proper GUID.

		if (GuidIsFcc(guid, FCC('I420')))
		{
			 return TEXT("I420");
		}

		if (GuidIsFcc(guid, FCC('YV12')))
		{
			return TEXT("YV12");
		}
	}

	return TEXT("????");
}

// VideoFormatInfo: Return a string that describes a media type, as best we can.


// Uncomment this if you want verbosity:
#define VERBOSE_FORMAT_STRINGS

const TCHAR* VideoFormatInfo(AM_MEDIA_TYPE *pmt)
{
	static TCHAR msg[512];

	TCHAR *FormatName = TEXT("");
	RECT *pSrc, *pTrg;

	if (pmt == 0)
	{
		return TEXT("NULL");
	}
	
	if (pmt->majortype != MEDIATYPE_Video)
	{
		return TEXT("Non-video type");
	}

	DWORD dwWidth = 0, dwHeight = 0;

	if (pmt->formattype == FORMAT_VideoInfo)
	{
		if (pmt->cbFormat >= sizeof(VIDEOINFOHEADER))
		{
			VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)(pmt->pbFormat);
			dwWidth = pVIH->bmiHeader.biWidth;
			dwHeight = pVIH->bmiHeader.biHeight;

			FormatName = TEXT("VideoInfo");

			pSrc = &pVIH->rcSource;
			pTrg = &pVIH->rcTarget;
		}
	}
	else if (pmt->formattype == FORMAT_VideoInfo2)
	{
		if (pmt->cbFormat >= sizeof(VIDEOINFOHEADER2))
		{
			VIDEOINFOHEADER2 *pVIH = (VIDEOINFOHEADER2*)(pmt->pbFormat);
			dwWidth = pVIH->bmiHeader.biWidth;
			dwHeight = pVIH->bmiHeader.biHeight;

			FormatName = TEXT("VideoInfo2");

			pSrc = &pVIH->rcSource;
			pTrg = &pVIH->rcTarget;
		}
	}
	else
	{
		return TEXT("Unknown Format");
	}
	
#ifdef VERBOSE_FORMAT_STRINGS

	wsprintf(msg, "%s, %d x %d (%s) Src={%d,%d,%d,%d} Trg={%d,%d,%d,%d}", 
		VideoSubtypeName(pmt->subtype), 
		dwWidth, dwHeight, 
		FormatName,
		pSrc->left, pSrc->top, pSrc->right, pSrc->bottom,
		pTrg->left, pTrg->top, pTrg->right, pTrg->bottom
		);
#else

	wsprintf(msg, "%s, %d x %d", 
		VideoSubtypeName(pmt->subtype), 
		dwWidth, dwHeight
		);
#endif

	return msg;

}
