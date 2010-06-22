#pragma once

/******************************************************************************
 *  CGraphEventHandler Class
 *
 *  Abstract class that defines a callback for the application to handle
 *  DirectShow filter graph events.
 * 
 *  Usage:
 *  - Implement this class in your app.
 *  - Call CGraph::SetEventWindow() with a handle to your window.
 *  - When you get a CGraph::WM_GRAPH_MESSAGE message, call CGraph::HandleEvent and
 *    pass it your CGraphEventHandler class
 *  - The CGraph object calls OnGraphEvent on your object
 *
 *  This might seem screwy but it's pretty easy to use. The CGraph object handles
 *  getting the events and then freeing the event resources.
 *****************************************************************************/

class CGraphEventHandler
{
public:
	virtual void OnGraphEvent(long lEvent, long lParam1, long lParam2) = 0;
};


/******************************************************************************
 *  CGraphEventHandler Class
 *  Manages the filter graph. 
 *
 *  This class handles generic filter graph operations, while the CCaptureGraph 
 *  class has capture-specific functions. (But CGraph creates a Capture Graph 
 *  Builder object (ICaptureGraphBuilder2) because it's just too damn useful.)
 *****************************************************************************/

class CGraph
{
protected:
	CComPtr<IGraphBuilder> m_pGraph;            // Filter Graph Manager
	CComPtr<ICaptureGraphBuilder2> m_pBuild;    // Capture Graph Builder
	CComPtr<IMediaControl> m_pControl;          
	CComPtr<IMediaEventEx> m_pEvent;
	HWND m_hVidWin;                             // Handle to the video window (can be NULL)

	void InitVideoWindow();     

public:

	static const long WM_GRAPH_MESSAGE = WM_APP + 1;   // Custom Win32 event to signal DShow events
	
    REFERENCE_TIME rtMaxTime;  // Maximum reference time 
	REFERENCE_TIME rtMinTime;  // Minimum reference time. 
    // (These are variables because some functions take pointer to ref times, so you cannot
    // use constant values. I'm thinking of ControlStream in particular...)

	CGraph();
	virtual ~CGraph();

    virtual void TearDownGraph();  // Remove everything from the graph

	// Graph Control
	HRESULT Stop();
	HRESULT Run();
	HRESULT HandleEvent(CGraphEventHandler &GraphEvent);

	// Configuration
	void SetVideoWindow(HWND hwnd) { m_hVidWin = hwnd; }
	HRESULT ShowWindow(BOOL bShow);
	HRESULT SetAudio(long lVolume);

	HRESULT SetEventWindow(HWND hwnd)   // Set an application window to receive DShow event notices
	{ 
		_ASSERTE(m_pEvent);
		return m_pEvent->SetNotifyWindow((OAHWND)hwnd, WM_GRAPH_MESSAGE, 0);
	}


	// Return a pointer to our ICaptureGraphBuilder2 interface, because often the app needs it.
	ICaptureGraphBuilder2 * const CaptureGraphBuilder()
    {
        _ASSERTE(m_pBuild != NULL);
       	return m_pBuild.p;
    }
        // Note: This kind of accessor function is technically a violation of COM rules. 
        // The caller should not hold onto the pointer. Use it like this:
        //     m_pGraph->CaptureGraphBuilder()->SomeMethod()

};


/******************************************************************************
 *  CCaptureGraph Class
 *  Manages a capture graph. 
 *****************************************************************************/

class CCaptureGraph : public CGraph
{
protected:
    
	CComPtr<IBaseFilter> m_pCap;        // (Video)Capture filter
	CComPtr<IBaseFilter> m_pAudioCap;   // Audio capture filter

    WCHAR m_szAudioInput[MAX_PIN_NAME];  // Name of the audio input pin

	HRESULT RenderType1Transmit(const WCHAR* wszFilename);
	HRESULT RenderType2Transmit(const WCHAR* wszFilename);


public:

    CCaptureGraph();

  	// Graph building
	HRESULT AddCaptureDevice(IMoniker *pMoniker);
	HRESULT AddAudioCaptureDevice(IMoniker *pMoniker, const WCHAR *wszInputPinName);

	HRESULT RenderPreview(BOOL bRenderCC = TRUE);
	HRESULT RenderAviCapture(const TCHAR* szFileName, int cchFileName);
	HRESULT RenderTransmit(const TCHAR* szFilename, int cchFileName);
	HRESULT RenderStillPin(const TCHAR* szFilename, int cchFileName);
	void TearDownGraph();

    void ReleaseDevice()
    {
        m_pCap.Release();
        m_pAudioCap.Release();
    }


    // Graph Control
	HRESULT StartCapture() { return m_pBuild->ControlStream(&PIN_CATEGORY_CAPTURE, 0, 0, &rtMinTime, &rtMaxTime, 0, 0); }
	HRESULT StopCapture() { return m_pBuild->ControlStream(&PIN_CATEGORY_CAPTURE, 0, 0, &rtMaxTime, &rtMinTime, 0, 0); }
	HRESULT EnableAudioCapture(bool bEnable);
	HRESULT TakeSnapShot();
	HRESULT ConfigureTVAudio(BOOL bActivate);

	// Accessors
	HRESULT GetDevice(IBaseFilter **ppCap);  // Return the video capture device
	HRESULT GetAudioDevice(IBaseFilter **ppCap);  // Return the audio capture device

};



// This little base class provides a common interface for initializing
// an object that manages some aspect of the device, e.g. TV tuning.
// (Kind of stupid actually...)

class CDeviceHelper
{
protected:
	CCaptureGraph *m_pGraph;  // Pointer to our Filter Graph helper object

public:
	virtual void InitDevice(IBaseFilter *pFilter) = 0;
	void SetGraph(CCaptureGraph *pGraph) { m_pGraph = pGraph; }

};

