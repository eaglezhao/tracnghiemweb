#pragma once


// Graph building helpers

HRESULT AddFilter(
    IGraphBuilder *pGraph,  // Pointer to the Filter Graph Manager.
    const GUID& clsid,      // CLSID of the filter to create.
    LPCWSTR wszName,        // A name for the filter.
    IBaseFilter **ppF);      // Receives a pointer to the filter.


HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin);           // Receives a pointer to the pin.


HRESULT ConnectFilters(
    IGraphBuilder *pGraph, // Filter Graph Manager.
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest);    // Downstream filter.


HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc, 
    IBaseFilter *pDest);


HRESULT FindConnectedFilter(IBaseFilter *pSrc, PIN_DIRECTION PinDir, IBaseFilter **ppConnected);


HRESULT FindCrossbarPin(
    IAMCrossbar *pXBar,  // Pointer to the crossbar.
    PhysicalConnectorType PhysicalType, // Pin type to match.
    BOOL bInput,   // TRUE = input pin, FALSE = output pin.
    long *pIndex);  // Receives the index of the pin, if found.


HRESULT ConnectAudio(IAMCrossbar *pXBar, BOOL bActivate);



// random helpers
void ReportError(HWND hwnd, const TCHAR* msg);
HBITMAP SetBitmapImg(HINSTANCE hinst, WORD nID, HWND hwnd);
HRESULT AllocGetWindowText(HWND hwnd, TCHAR **pszText);

int AnsiToWide(const char* szSrc, int cchSrc, WCHAR **pwszDest);


int SystemBorderWidth();
int SystemBorderHeight();
int SystemCaptionHeight();


// Media type helpers (I don't feel like linking to strmbase)

void DeleteMediaType(AM_MEDIA_TYPE *pmt);
void FreeMediaType(AM_MEDIA_TYPE& mt);
HRESULT CopyMediaType(AM_MEDIA_TYPE *pmt, const AM_MEDIA_TYPE *pmtSrc);

// These attempt to create names for media types
const TCHAR* VideoSubtypeName(GUID& guid);
const TCHAR* VideoFormatInfo(AM_MEDIA_TYPE *pmt);



