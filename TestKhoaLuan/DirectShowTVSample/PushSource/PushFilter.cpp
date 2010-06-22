// PushFilter.cpp

// This sample shows a generic video push-source filter. The filter draws the frame number
// onto each frame that it delivers, using GDI+. The filter supports seeking, including
// seeking by frame number. It uses the following classes from the DirectShow base-class
// library:
// CSource - Implements the filter.
// CSourceStream - Implements the output pin.
// CSourceSeeking - Provides a framework for seeking.


#include <streams.h>

#pragma warning(disable:4355)  // disable compiler warning: 'this' used in base member initializer list.

#include <stdio.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include "videoutil.h"

#include <initguid.h>
#include "pushguids.h"  // declares our filter CLSID

const double DEFAULT_FRAME_RATE = 30.0;  // 30 fps
const LONG DEFAULT_WIDTH = 320;
const LONG DEFAULT_HEIGHT = 240;
const REFERENCE_TIME DEFAULT_DURATION = UNITS * 10;



// CPushPin class: Output pin that delivers the video frames.

class CPushPin : public CSourceStream, public CSourceSeeking
{
private:

    REFERENCE_TIME m_rtStreamTime;    // Stream time (relative to when the graph started)
    REFERENCE_TIME m_rtSourceTime;    // Source time (relative to ourselves)

    // A note about seeking and time stamps:

    // Suppose you have a file source that is N seconds long. If you play the file from
    // the beginning at normal playback rate (1x), the presentation time for each frame 
    // will match the source file:

    // Frame:  0    1    2 ... N
    // Time:   0    1    2 ... N

    // Now suppose you seek in the file to an arbitrary spot, frame i. After the seek 
    // command, the graph's clock resets to zero. Therefore, the first frame delivered 
    // after the seek has a presentation time of zero:

    // Frame:  i    i+1  i+2 ... 
    // Time:   0    1    2 ...   

    // Therefore we have to track stream time and source time independently.
    // (If you do not support seeking, then source time always equals presentation time.)

    REFERENCE_TIME m_rtFrameLength;   // Frame length
    int m_iFrameNumber;  // Current frame number that we are rendering.

    BOOL m_bDiscontinuity; // If true, set the discontinuity flag.

    CCritSec m_cSharedState;  // Protects our internal state
    ULONG_PTR m_gdiplusToken; // GDI+ initialization token

    // Private function to draw our bitmaps
    HRESULT WriteToBuffer(LPWSTR wszText, BYTE *pData, VIDEOINFOHEADER *pVih);

    // Update our internal state after a seek command
    void UpdateFromSeek();

    // The following methods support seeking using other time formats besides 
    // reference time. If you only want to support seek-by-reference-time, you 
    // do not have to override these methods.
    STDMETHODIMP SetTimeFormat(const GUID *pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID *pFormat);
    STDMETHODIMP IsFormatSupported(const GUID *pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat,
                                   LONGLONG Source, const GUID *pSourceFormat );
    STDMETHODIMP SetPositions(LONGLONG *pCurrent, DWORD CurrentFlags,
			                  LONGLONG *pStop, DWORD StopFlags);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);

    // Conversions between reference times and frame numbers
    LONGLONG FrameToTime(LONGLONG frame) { 
        LONGLONG f = frame * m_rtFrameLength; 
        return f;
    }
    LONGLONG TimeToFrame(LONGLONG rt) { return rt / m_rtFrameLength; }

    GUID m_TimeFormat;  // Which time format is currently active

protected:
    // Override CSourceStream methods
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);

    // The following methods support seeking.
    HRESULT OnThreadStartPlay();
    HRESULT ChangeStart();
    HRESULT ChangeStop();
    HRESULT ChangeRate();
    STDMETHODIMP SetRate(double dRate);

public:

    CPushPin(HRESULT *phr, CSource *pFilter);
    ~CPushPin();

    // Override this to expose IMediaSeeking
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    // We don't support any quality control 
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};

// CPushSource class: Our source filter. 

class CPushSource : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance to create it.
    CPushSource(IUnknown *pUnk, HRESULT *phr);

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
};



/********************************  CPushSource Class  ******************************/


CPushSource::CPushSource(IUnknown *pUnk, HRESULT *phr)
: CSource(NAME("PushSource"), pUnk, CLSID_PushSource)
{
    // Create the output pin.
    // The pin magically adds itself to the pin array.
    CPushPin *pPin = new CPushPin(phr, this);

    if (pPin == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
}


//-----------------------------------------------------------------------------
// Name: CreateInstance()
// Desc: Create an instance of the filter.
//
//-----------------------------------------------------------------------------


CUnknown * WINAPI CPushSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CPushSource *pNewFilter = new CPushSource(pUnk, phr );
    if (pNewFilter == NULL) 
    {
        *phr = E_OUTOFMEMORY;
    }
    return pNewFilter;
}



/********************************  CPushPin Class  ******************************/

CPushPin::CPushPin(HRESULT *phr, CSource *pFilter)
: CSourceStream(NAME("CPushPin"), phr, pFilter, L"Out"),
  CSourceSeeking(NAME("PushPin2Seek"), (IPin*)this, phr, &m_cSharedState),
  m_rtStreamTime(0),
  m_rtSourceTime(0),
  m_iFrameNumber(0),
  m_rtFrameLength(Fps2FrameLength(DEFAULT_FRAME_RATE))
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    Status s = GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    if (s != Ok)
    {
        *phr = E_FAIL;
    }

    // SEEKING: Set the source duration and the initial stop time.
    m_rtDuration = m_rtStop = DEFAULT_DURATION;
}


CPushPin::~CPushPin()
{
    // Shut down GDI+
    GdiplusShutdown(m_gdiplusToken);
}


//-----------------------------------------------------------------------------
// Name: NonDelegatingQueryInterface()
// Desc: Implements QueryInterface for our pin. 
//
// We override this to expose the IMediaSeeking interface (via CSourceSeeking);
// for everything else CSourceStream handles it.
//
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IMediaSeeking ) 
    {
        return CSourceSeeking::NonDelegatingQueryInterface( riid, ppv );
    }
    return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}



//-----------------------------------------------------------------------------
// Name: GetMediaType()
// Desc: Propose a media type.  
//
// pMediaType: Fill this in with the media type. We only offer RGB-32.
//
//-----------------------------------------------------------------------------

HRESULT CPushPin::GetMediaType(CMediaType *pMediaType)
{
    CheckPointer(pMediaType, E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Call our helper function that fills in the media type.
    return CreateRGBVideoType(pMediaType, 32, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_FRAME_RATE);
}


//-----------------------------------------------------------------------------
// Name: CheckMediaType()
// Desc: Check whether a media type is valid. We only accept RGB-32
//
// pMediaType: Contains the media type information.
//
//-----------------------------------------------------------------------------


HRESULT CPushPin::CheckMediaType(const CMediaType *pMediaType)
{
    CAutoLock lock(m_pFilter->pStateLock());

    // Is it a video type?
    if (pMediaType->majortype != MEDIATYPE_Video)
    {
        return E_FAIL;
    }

    // Is it 32-bit RGB?
    if ((pMediaType->subtype != MEDIASUBTYPE_RGB32))
    {
        return E_FAIL;
    }

    // Is it a VIDEOINFOHEADER type? 
    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
        (pMediaType->pbFormat != NULL))
    {
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pMediaType->pbFormat;

        // We don't do source rects
        if (!IsRectEmpty(&(pVih->rcSource)))
        {
            return E_FAIL;
        }

        // Valid frame rate? 
        if (pVih->AvgTimePerFrame != m_rtFrameLength)
        {
            return E_FAIL;
        }

        // Everything checked out.
        return S_OK;
    }
        
    return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: DecideBufferSize()
// Desc: Set allocator properties, such as buffer size and number of buffers.
//
// pAlloc:   Pointer to the allocator object.
// pRequest: Contains the downstream filter's buffer request. Zero in any field
//           means "don't care"
//
//-----------------------------------------------------------------------------

HRESULT CPushPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    HRESULT hr;

    VIDEOINFO *pvi = (VIDEOINFO*) m_mt.Format();
    ASSERT(pvi != NULL);

    if (pRequest->cBuffers == 0)
    {
        pRequest->cBuffers = 1;  // We need at least one buffer.
    }
    
    // Buffer size must be at least big enough hold our image. Bigger is OK.
    if ((long)pvi->bmiHeader.biSizeImage > pRequest->cbBuffer)
    {
        pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;
    }

    // Try to set these properties.
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);

    if (FAILED(hr)) 
    {
        return hr;
    }

    // Check what we actually got. 

    if (Actual.cbBuffer < pRequest->cbBuffer) 
    {
        return E_FAIL;
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: FillBuffer()
// Desc: Fill the buffer with our image.
//
// pSample: Pointer to the media sample.
//-----------------------------------------------------------------------------

HRESULT CPushPin::FillBuffer(IMediaSample *pSample)
{
    HRESULT hr;

    BYTE *pData;
    long cbData;

    WCHAR msg[256];

    // Get a pointer to the buffer.
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check if the downstream filter is changing the format.

    CMediaType *pmt;
    hr = pSample->GetMediaType((AM_MEDIA_TYPE**)&pmt);
    if (hr == S_OK)
    {
        SetMediaType(pmt);
        DeleteMediaType(pmt);
    }

    // Get our format information

    ASSERT(m_mt.formattype == FORMAT_VideoInfo);
    ASSERT(m_mt.cbFormat >= sizeof(VIDEOINFOHEADER));

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    {   
        // Scope for the state lock, which protects the frame number and ref times

        CAutoLock cAutoLockShared(&m_cSharedState);

        // Have we reached the stop time yet?
        if (m_rtSourceTime >= m_rtStop) 
        {
            // This will cause the base class to send an EndOfStream downstream.
            return S_FALSE;  
        }


        // Time stamp the sample.
        REFERENCE_TIME rtStart, rtStop;

        rtStart = (REFERENCE_TIME)(m_rtStreamTime / m_dRateSeeking);
        rtStop  = rtStart + (REFERENCE_TIME)(pVih->AvgTimePerFrame / m_dRateSeeking);

        pSample->SetTime(&rtStart, &rtStop);

        // Write the frame number into our text buffer
        swprintf(msg, L"%d", m_iFrameNumber);

        // Increment the frame number and ref times for the next time through the loop.
        m_iFrameNumber++;
        m_rtSourceTime += pVih->AvgTimePerFrame;
        m_rtStreamTime += pVih->AvgTimePerFrame;

    }

    // Private method to draw the image.
    hr = WriteToBuffer(msg, pData, pVih);

    if (FAILED(hr)) 
    {
        return hr;
    }

    // Every frame is a key frame.
    pSample->SetSyncPoint(TRUE);

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: WriteToBuffer()
// Desc: Fill the buffer with our image.
//-----------------------------------------------------------------------------

HRESULT CPushPin::WriteToBuffer(LPWSTR wszText, BYTE *pData, VIDEOINFOHEADER *pVih)
{
    ASSERT(pVih->bmiHeader.biBitCount == 32);

    DWORD dwWidth, dwHeight;
    long lStride;
    BYTE *pbTop;

    // Get the width, height, top row of pixels, and stride.
    GetVideoInfoParameters(pVih, pData, &dwWidth, &dwHeight, &lStride, &pbTop, false);

    // Create a GDI+ bitmap object to manage our image buffer.
    Bitmap bitmap((int)dwWidth, (int)dwHeight, abs(lStride), PixelFormat32bppRGB, pData);

    // Create a GDI+ graphics object to manage the drawing.
    Graphics g(&bitmap);

    // Turn on anti-aliasing
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);


    // Erase the background
    g.Clear(Color(0x0, 0, 0, 0));

    // GDI+ is top-down by default, so if our image format is bottom-up, we need
    // to set a transform on the Graphics object to flip the image. 
    if (pVih->bmiHeader.biHeight > 0)
    {
        g.ScaleTransform(1.0, -1.0); // Flip the image around the X axis
        g.TranslateTransform(0, (float)dwHeight, MatrixOrderAppend);  // Move it back into place
    }


    SolidBrush brush(Color(0xFF, 0xFF, 0xFF, 0));   // Yellow brush
    Font       font(FontFamily::GenericSerif(), 48); // Big serif type
    RectF      rcBounds(30, 30, (float)dwWidth, (float)dwHeight);  // Bounding rectangle

    // Draw the text
    g.DrawString(
        wszText, -1, 
        &font, 
        rcBounds, 
        StringFormat::GenericDefault(), 
        &brush);

    return S_OK;
}




// Seeking methods

//-----------------------------------------------------------------------------
// Name: UpdateFromSeek()
// Desc: Private method to update our internal state after a seek command.
//
// If the graph is active (paused or running), then we need to flush the
// graph after the seek command. This is a several-stage process:
//
// 1. Call BeginFlush downstream - tells the downstream filter that we're flushing,
//    so it will reject new samples. This prevents the worker thread from blocking 
//    inside the Receive or GetBuffer methods.
// 2. Stop our worker thread, so we don't deliver any more samples.
// 3. Call EndFlush downstream - tells the downstream filter that we're ready to start.
// 4. Start the worker thread again.

// The next two things happen elsewhere:
// 5. Send a NewSegment call downstream - this happens in OnThreadStartPlay 
// 6. Set the discontinuity flag on the next sample - this happens in FillBuffer
//
// Make sure NOT to hold the m_cSharedState critsec before calling this method,
// because the worker thread could be holding it and blocked waiting for something.
//-----------------------------------------------------------------------------


void CPushPin::UpdateFromSeek()
{
    if (ThreadExists())   // Filter is active?
    {
        DeliverBeginFlush();

        // Shut down the thread and stop pushing data.
        Stop();
        DeliverEndFlush();

        // Restart the thread and start pushing data again.
        Pause();

        // We'll set the discontinuity flag on the next sample.
    }
}

//-----------------------------------------------------------------------------
// Name: OnThreadStartPlay()
// Desc: Called when our thread starts.
//-----------------------------------------------------------------------------

HRESULT CPushPin::OnThreadStartPlay()
{
    m_bDiscontinuity = TRUE; // Set the discontinuity flag on the next sample

    // Send a NewSegment call downstream.
    return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}


//-----------------------------------------------------------------------------
// Name: ChangeStart()
// Desc: Called when the graph seeks to a new position
//-----------------------------------------------------------------------------

HRESULT CPushPin::ChangeStart( )
{
    // Reset stream time to zero and the source time to m_rtStart
    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        m_rtStreamTime = 0;
        m_rtSourceTime = m_rtStart;
        m_iFrameNumber = (int)(m_rtStart / m_rtFrameLength);
    }
    UpdateFromSeek();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ChangeStop()
// Desc: Called when the graph sets a new stop time
//-----------------------------------------------------------------------------

HRESULT CPushPin::ChangeStop( )
{
    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        if (m_rtSourceTime < m_rtStop)
        {
            return S_OK;
        }
    }

    // We're already past the new stop time. Flush the graph.
    UpdateFromSeek();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: SetRate()
// Desc: Called when the graph sets a new rate.
//
// I override SetRate to fix a minor problem in the CSourceSeeking implementation
// where it doesn't validate the new rate before it overwrites the old rate.
//-----------------------------------------------------------------------------

HRESULT CPushPin::SetRate(double dRate)
{
    if (dRate <= 0.0)
    {
        return E_INVALIDARG;
    }
    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        m_dRateSeeking = dRate;
    }
    UpdateFromSeek();
    return S_OK;
}

// Because we override SetRate, the ChangeRate method won't ever be called. (It is only
// ever called by SetRate.) But it's pure virtual, so it needs a dummy implementation.
HRESULT CPushPin::ChangeRate() { return S_OK; }


// NOTE: The next bunch of CPushPin methods are overridden to support seeking by frames,
// in addition to the standard seek-by-reference-time. If you only want to support
// reference times, you do not have to override these methods.

// Critical sections: The m_pFilter->pStateLock() method returns a pointer to the
// critical section object that protects the filter state. 

//-----------------------------------------------------------------------------
// Name: SetTimeFormat()
// Desc: Sets a new time format for seeking.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::SetTimeFormat(const GUID *pFormat)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    CheckPointer(pFormat, E_POINTER);

    if (m_pFilter->IsActive())
    {
        // Cannot switch formats while running.
        return VFW_E_WRONG_STATE;
    }

    if (S_OK != IsFormatSupported(pFormat))
    {
        // We don't support this time format.
        return E_INVALIDARG;
    }
    m_TimeFormat = *pFormat;
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetTimeFormat()
// Desc: Gets the current time format for seeking.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::GetTimeFormat(GUID *pFormat)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    CheckPointer(pFormat, E_POINTER);
    *pFormat = m_TimeFormat;
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: IsUsingTimeFormat
// Desc: Check if we are currently using a given time format.
//
// Returns S_OK if we are, S_FALSE if we aren't.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::IsUsingTimeFormat(const GUID *pFormat)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    CheckPointer(pFormat, E_POINTER);
    return (*pFormat == m_TimeFormat ? S_OK : S_FALSE);
}

//-----------------------------------------------------------------------------
// Name: IsFormatSupported
// Desc: Check whether we support a given time format for seeking.
//
// Returns S_OK if we do, S_FALSE if we don't.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::IsFormatSupported( const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
    {
        return S_OK;
    }
    else if (*pFormat == TIME_FORMAT_FRAME)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: QueryPreferredFormat
// Desc: Return the time format we prefer.
//
// If we _prefer_ a certain format, and the application sets that format, then
// the Filter Graph Manager is likely to use our filter for seeking. 
//
// If we _support_ a format but do not prefer it, then the Filter Graph Manager
// will not use us for seeking if another filter prefers the format.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_FRAME; // doesn't really matter which we prefer
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ConvertTimeFormat()
// Desc: Converts a time value from one time format to another.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::ConvertTimeFormat(
    LONGLONG *pTarget,            // Receives the converted time value.
    const GUID *pTargetFormat,    // Specifies the target format for the conversion.
    LONGLONG Source,              // Time value to convert.
    const GUID *pSourceFormat)    // Time format for the Source time.
{
    CheckPointer(pTarget, E_POINTER);

    // Either of the format GUIDs can be NULL, which means "use the current time
    // format" 
    GUID TargetFormat, SourceFormat;
    TargetFormat = (pTargetFormat == NULL ? m_TimeFormat : *pTargetFormat );
    SourceFormat = (pSourceFormat == NULL ? m_TimeFormat : *pSourceFormat );

    if (TargetFormat == TIME_FORMAT_MEDIA_TIME)
    {
        if (SourceFormat == TIME_FORMAT_FRAME)
        {
            *pTarget = FrameToTime(Source);
            return S_OK;
        }
        if (SourceFormat == TIME_FORMAT_MEDIA_TIME)
        {
            // no-op
            *pTarget = Source;
            return S_OK;
        }
        return E_INVALIDARG;  // Invalid source format.
    }

    if (TargetFormat == TIME_FORMAT_FRAME)
    {
        if (SourceFormat == TIME_FORMAT_MEDIA_TIME)
        {
            *pTarget = TimeToFrame(Source);
            return S_OK;
        }
        if (SourceFormat == TIME_FORMAT_FRAME)
        {
            // no-op
            *pTarget = Source;
            return S_OK;
        }
        return E_INVALIDARG;  // Invalid source format.
    }

    return E_INVALIDARG;  // Invalid target format.
}

//-----------------------------------------------------------------------------
// Name: SetPositions()
// Desc: Sets the current and stop positions. 
// Note: This method is overridden to interpret the position values in terms
//       of the current time format. Not needed if you only support ref times.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::SetPositions(
    LONGLONG *pCurrent,  // New current position (can be NULL!)
    DWORD CurrentFlags,
    LONGLONG *pStop,     // New stop position (can be NULL!)
    DWORD StopFlags)
{
    HRESULT hr;

    if (m_TimeFormat == TIME_FORMAT_FRAME)
    {
        REFERENCE_TIME rtCurrent = 0, rtStop = 0;
        if (pCurrent)
        {
            rtCurrent = FrameToTime(*pCurrent);
        }
        if (pStop)
        {
            rtStop = FrameToTime(*pStop);
        }
        hr = CSourceSeeking::SetPositions(&rtCurrent, CurrentFlags, &rtStop, StopFlags);
        if (SUCCEEDED(hr))
        {
            // The AM_SEEKING_ReturnTime flag means the caller wants the input times
            // converted to the current time format. 
            if (pCurrent && (CurrentFlags & AM_SEEKING_ReturnTime))
            {
                *pCurrent = rtCurrent;
            }
            if (pStop && (StopFlags & AM_SEEKING_ReturnTime))
            {
                *pStop = rtStop;
            }
        }
    }
    else
    {
        // Simple pass thru'
        hr = CSourceSeeking::SetPositions(pCurrent, CurrentFlags, pStop, StopFlags);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Name: GetDuration()
// Desc: Returns the duration of the source. 
// Note: This method is overridden to return the duration in terms of the 
//       current time format. Not needed if you only support ref times.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::GetDuration(LONGLONG *pDuration)
{
    HRESULT hr = CSourceSeeking::GetDuration(pDuration);
    if (SUCCEEDED(hr))
    {
        if (m_TimeFormat == TIME_FORMAT_FRAME)
        {
            *pDuration = TimeToFrame(*pDuration);
        }
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetStopPosition()
// Desc: Returns the current stop position. 
// Note: This method is overridden to return the stop position in terms of the 
//       current time format. Not needed if you only support ref times.
//-----------------------------------------------------------------------------

STDMETHODIMP CPushPin::GetStopPosition(LONGLONG *pStop)
{
    HRESULT hr = CSourceSeeking::GetStopPosition(pStop);
    if (SUCCEEDED(hr))
    {
        if (m_TimeFormat == TIME_FORMAT_FRAME)
        {
            *pStop = TimeToFrame(*pStop);
        }
    }
    return S_OK;
}


/**********************************************
 *  COM Junk
 **********************************************/

static const WCHAR g_wszName[] = L"Generic Push Source";

// NOTE: The filter does not register itself for Intelligent Connect in release builds.
// An application should create the filter via CoCreateInstance

#ifdef DEBUG
AMOVIESETUP_PIN sudOutputPin = {
    L"",            // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &GUID_NULL,     // Obsolete.
    NULL,           // Obsolete.
    0,              // Number of media types.
    0   // Pointer to media types.
};

AMOVIESETUP_FILTER sudFilterReg = {
    &CLSID_PushSource,      // Filter CLSID.
    g_wszName,              // Filter name.
    MERIT_DO_NOT_USE,           // Merit.
    1,                      // Number of pin types.
    &sudOutputPin           // Pointer to pin information.
};
#endif

CFactoryTemplate g_Templates[1] = 
{
    { 
      g_wszName,                     // Name
      &CLSID_PushSource,             // CLSID
      CPushSource::CreateInstance,   // Method to create an instance of the filter
      NULL,                          // Initialization function
#ifdef DEBUG
      &sudFilterReg
#else
      NULL                           // Set-up information 
#endif
    }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    


STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}



//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

