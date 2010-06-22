#include "stdafx.h"
#include "graph.h"
#include "SampleGrabber.h"


/*************************** CGraph methods  ********************************/


//-----------------------------------------------------------------------------
// Name: CGraph()
// Desc: Constructor
//-----------------------------------------------------------------------------

CGraph::CGraph()
: rtMaxTime(MAXLONGLONG), rtMinTime(0), m_hVidWin(0)
{
    // Create the Filter Graph Manager
	HRESULT hr = m_pGraph.CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		throw new bad_hresult(hr);
	}

    // Create the Capture Graph Builder
	hr = m_pBuild.CoCreateInstance(CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC_SERVER);

    // Query for useful interfaces
	if (SUCCEEDED(hr))
	{
		hr = m_pGraph.QueryInterface(&m_pControl);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pGraph.QueryInterface(&m_pEvent);
	}

	if (FAILED(hr))
	{
		m_pGraph.Release();
        m_pBuild.Release();
		throw new bad_hresult(hr);
	}

    // Tell the Capture Graph Builder about the filter graph

    m_pBuild->SetFiltergraph(m_pGraph);
}


//-----------------------------------------------------------------------------
// Name: ~CGraph()
// Desc: Destructor
//-----------------------------------------------------------------------------

CGraph::~CGraph()
{
    OutputDebugString(TEXT("~CGraph Destructor\n"));

	// Clear event notifications
	m_pEvent->SetNotifyWindow(0, 0, 0);
	
    // Stop first, then hide the window. Otherwise, a ghost of the window
    // will appear when we call put_Owner(NULL)
    
    m_pControl->Stop();

	CComQIPtr<IVideoWindow> pVid(m_pGraph);
	if (pVid)
	{
		pVid->put_Visible(OAFALSE);
		pVid->put_Owner(0);  // Clear the window's owner so it stops trying to send messages to it.
	}

}

//-----------------------------------------------------------------------------
// Name: InitVideoWindow()
// Desc: Set up the video window, after graph building
//
// Note: This code assumes we're using the old Video Renderer (or the VMR in
// windowed mode) ... an interesting exercise would be to rework this class to
// handle both the VR and the VMR case.
//-----------------------------------------------------------------------------

void CGraph::InitVideoWindow()
{
	if (m_hVidWin)
	{
		OutputDebugString(TEXT("Initializing the video window...\n"));

        // Check whether there is a video window by QI'ing for IVideoWindow.
		CComQIPtr<IVideoWindow> pVid(m_pGraph);
		if (pVid)
		{
            // Find the client rect of the parent window
            RECT grc;
            GetClientRect(m_hVidWin, &grc);

            // Set the parent window as the owner.
			pVid->put_Owner((OAHWND)m_hVidWin);

            // Set the right style bits.
			pVid->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

            // Can find the video dimensions? If so, use them to position the video window.
			CComQIPtr<IBasicVideo> pVideo(m_pGraph);
			if (pVideo)
			{
				long lHeight = 0, lWidth = 0;
				if (SUCCEEDED(pVideo->GetVideoSize(&lWidth, &lHeight)))
				{
                    // Scale the video window to the video dimensions or to the parent
                    // window's client area, whichever is smaller. (This might stretch
                    // the video in one dimension or the other...)
					lHeight = min(lHeight, grc.bottom);
					lWidth = min(lWidth, grc.right);

                    // Center the video inside the parent window
					long dx = (grc.right - lWidth ) / 2;
					long dy = (grc.bottom - lHeight) / 2;

					pVid->SetWindowPosition(dx, dy, lWidth, lHeight);
					return;
				}
			}
			// Can't find the video dimensions ... just stretch to the client area.
			pVid->SetWindowPosition(0, 0, grc.right, grc.bottom);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: TearDownGraph()
// Desc: Remove everything from the graph. 
//
// This is less annoying than releasing all of our graph pointers and CoCreating
// everything again. Also it means you can hang onto filters and re-insert them
// into the graph after this call.
//-----------------------------------------------------------------------------

void CGraph::TearDownGraph()
{
	OutputDebugString(TEXT("TearDownGraph()\n"));

    // Can't do this while the graph is running
	m_pControl->Stop();

    // Enumerate all the filters
	CComPtr<IEnumFilters> pEnum;
	HRESULT hr = m_pGraph->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		CComPtr<IBaseFilter> pFilter;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
            // Remove this filter.
			m_pGraph->RemoveFilter(pFilter);

            // Reset the enumerator to the beginning, because removing the
            // filter makes the enumerator go out of sync.
			pEnum->Reset();

			pFilter.Release();  // Release this pointer for the next loop.
		}
	}
}



//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Run the graph.
//-----------------------------------------------------------------------------

HRESULT CGraph::Run()
{
	FILTER_STATE fs;
	HRESULT hr = m_pControl->GetState(0, (OAFilterState*)&fs);
	if (SUCCEEDED(hr) && (fs == State_Running))
	{
        // We're already running, nothing to do.
		return S_FALSE;
	}

	InitVideoWindow(); 
	hr = m_pControl->Run(); 

	if (SUCCEEDED(hr))
	{
		ShowWindow(TRUE);
	}
	else
	{
        // The safest thing on failure is to get rid of any stray filters...
		TearDownGraph();
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: Stop()
// Desc: Stop the graph.
//-----------------------------------------------------------------------------

HRESULT CGraph::Stop()
{
	FILTER_STATE fs;
	HRESULT hr = m_pControl->GetState(0, (OAFilterState*)&fs);
	if (SUCCEEDED(hr) && (fs == State_Stopped))
	{
        // We're already stopped, nothing to do.
		return S_FALSE;
	}

	InitVideoWindow();
	hr = m_pControl->Stop(); 

	if (SUCCEEDED(hr))
	{
		ShowWindow(FALSE);  // Hide the video window
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: ShowWindow()
// Desc: Show or hide the video window.
//
// bShow: TRUE = show, FALSE = hide
//-----------------------------------------------------------------------------

HRESULT CGraph::ShowWindow(BOOL bShow)
{
	CComQIPtr<IVideoWindow> pVid(m_pGraph);
	if (pVid)
	{
		return pVid->put_Visible(bShow ? OATRUE : OAFALSE);
	}
    // Lack of an IVideoWindow ptr is not a failure, just means there's no video window.
	return S_FALSE;  
}


//-----------------------------------------------------------------------------
// Name: SetAudio()
// Desc: Set the volume. 
//
// lVolume: -10000 is silent, 0 is full volume
//-----------------------------------------------------------------------------

HRESULT CGraph::SetAudio(long lVolume)
{
	CComQIPtr<IBasicAudio> pAudio(m_pGraph);
	if (pAudio)
	{
		return pAudio->put_Volume(lVolume);
	}
	// no audio
	return S_FALSE;
}

//-----------------------------------------------------------------------------
// Name: HandleEvent()
// Desc: Call this whenever your app gets a WM_GRAPH_MESSAGE window message.
//
// GraphEvent: Reference to your callback class.
//-----------------------------------------------------------------------------

HRESULT CGraph::HandleEvent(CGraphEventHandler &GraphEvent)
{
	long lEventCode = 0, lParam1 = 0, lParam2 = 0;
	while (S_OK == m_pEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0))
	{
		GraphEvent.OnGraphEvent(lEventCode, lParam1, lParam2);
		m_pEvent->FreeEventParams(lEventCode, lParam1, lParam2);
	}
	return S_OK;
}



/*************************** CCaptureGraph methods  ********************************/


//-----------------------------------------------------------------------------
// Name: CCaptureGraph()
// Desc: constructor
//-----------------------------------------------------------------------------

CCaptureGraph::CCaptureGraph()
{
    m_szAudioInput[0] = L'\0';
}


//-----------------------------------------------------------------------------
// Name: AddCaptureDevice()
// Desc: Create a video capture filter from a device moniker and add it to 
// the graph. Removes everything from the graph first (including the audio
// capture filter, if it exists).
//
// pMoniker: Pointer to the moniker.
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::AddCaptureDevice(IMoniker *pMoniker)
{
    // If we've already got one, get rid of everything and tear down the graph.
	if (m_pCap)
	{
        ReleaseDevice();
		TearDownGraph();
	}

    // Instantiate the filter.
	HRESULT hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pCap);
	if (SUCCEEDED(hr))
	{
		hr = m_pGraph->AddFilter(m_pCap, L"Capture Filter");
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: AddAudioCaptureDevice()
// Desc: Create an audio capture filter from a device moniker and add it to 
// the graph. Make sure to add the video capture filter first. 
//
// pMoniker: Pointer to the moniker.
// wszInputPinName: Name of the input pin to use.
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::AddAudioCaptureDevice(IMoniker *pMoniker, const WCHAR *wszInputPinName)
{
    // If we've already got one, get rid of it.
	if (m_pAudioCap)
	{
		m_pGraph->RemoveFilter(m_pAudioCap);
		m_pAudioCap.Release();
	}

    // Instantiate the filter.
	HRESULT hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pAudioCap);
	if (SUCCEEDED(hr))
	{
		hr = m_pGraph->AddFilter(m_pAudioCap, L"Audio Capture Filter");

		if (SUCCEEDED(hr))
		{
            wcsncpy(m_szAudioInput, wszInputPinName, MAX_PIN_NAME);
        }
        else
        {
			m_pAudioCap.Release();
            m_szAudioInput[0] = L'\0';
		}

        // By default, turn off audio capture. Client can turn it on as needed.
		EnableAudioCapture(false);
	}
	return hr;
}



//-----------------------------------------------------------------------------
// Name: TearDownGraph()
// Desc: Overrides the CGraph version. This version tears down the graph but 
// then re-inserts the video and audio capture filters (if they exist).
//
// The idea is that you can build up a graph, tear it down, then start over
// using the same device. To tear down the graph for real, just release the
// two capture filters before you call TearDownGraph.
//-----------------------------------------------------------------------------

void CCaptureGraph::TearDownGraph()
{
    CGraph::TearDownGraph();

	// Reinsert the audio capture filter
	if (m_pAudioCap)
	{
		m_pGraph->AddFilter(m_pAudioCap, L"Audio Capture Filter");
	}

	// Reinsert the video capture filter
	if (m_pCap)
	{
		m_pGraph->AddFilter(m_pCap, L"Capture Filter");

		// ??? This seems to work around a bug (?) where sequential transmit
		// graphs fail to play correctly.
		CComQIPtr<IMediaFilter> pMF(m_pGraph);
		pMF->SetSyncSource(0); // Get rid of the clock
		m_pGraph->SetDefaultSyncSource(); // Set the default clock
	}
}

//-----------------------------------------------------------------------------
// Name: GetDevice()
// Desc: Return a pointer to the video capture filter.
//-----------------------------------------------------------------------------


HRESULT CCaptureGraph::GetDevice(IBaseFilter **ppCap)
{
	if (!ppCap) return E_POINTER;

	if (m_pCap)
	{
		*ppCap = m_pCap.p;
		(*ppCap)->AddRef(); // Caller must release.
		return S_OK;
	}
	return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: GetAudioDevice()
// Desc: Return a pointer to the audio capture filter.
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::GetAudioDevice(IBaseFilter **ppCap)
{
	if (!ppCap) return E_POINTER;

	if (m_pAudioCap)
	{
		*ppCap = m_pAudioCap.p;
		(*ppCap)->AddRef();  // Caller must release.
		return S_OK;
	}
	return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: EnableAudioCapture()
// Desc: Enable or disable audio capture.
//
// bEnable: TRUE = enable, FALSE = disable
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::EnableAudioCapture(bool bEnable)
{
	HRESULT hr = S_FALSE;
	if (m_pAudioCap)
	{

        // Use ControlStream to set the start and stop times for the
        // audio capture pin.

		if (bEnable)
		{
			hr = m_pBuild->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
				m_pAudioCap, 
                &rtMinTime,  // Start on Run()
                &rtMaxTime,  // Stop never
                0, 0); 
		}
		else 
		{
			hr = m_pBuild->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
				m_pAudioCap, 
                &rtMaxTime,  // Start never
                &rtMinTime,  // Stop immediately
                0, 0);
		}
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: RenderPreview()
// Desc: Build the preview section of the graph.
//
// bRenderCC: Try to render CC? 
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::RenderPreview(BOOL bRenderCC)
{
	HRESULT hr;

	OutputDebugString(TEXT("RenderPreview()\n"));

	if (!m_pCap) return E_FAIL;


    // First try to render an interleaved stream (DV)
	hr = m_pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved,
		m_pCap, 0, 0);
	if (FAILED(hr))
	{
        // Next try a video stream.
		hr = m_pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
			m_pCap, 0, 0);
	}

	// Try to render CC. If it fails, we still count preview as successful.
	if (SUCCEEDED(hr) && bRenderCC)
	{
        // Try VBI pin category, then CC pin category. 
		HRESULT hrCC = m_pBuild->RenderStream(&PIN_CATEGORY_VBI, 0, m_pCap, 0, 0);
		if (FAILED(hrCC))
		{
			hrCC = m_pBuild->RenderStream(&PIN_CATEGORY_CC, 0, m_pCap, 0, 0);
		}
	}

	if (SUCCEEDED(hr))
	{
		InitVideoWindow();       // Set up the video window
		ConfigureTVAudio(TRUE);  // Try to get TV audio going
	}

	return hr;
}


//-----------------------------------------------------------------------------
// Name: RenderAviCapture()
// Desc: Build the capture portion of the graph
//
// szFileName: Target file name
// cchFileName: Size of file name in characters, including '\0'
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::RenderAviCapture(const TCHAR* szFileName, int cchFileName)
{
	OutputDebugString(TEXT("RenderAviCapture()\n"));

	if (!m_pCap) return E_FAIL;
	m_pControl->Stop();

	HRESULT hr;
	CComPtr<IBaseFilter> pMux;

    // Build the Mux -> File Writer part of the graph. 

    // I'm using TCHAR strings for input but the API wants wide char only

#ifdef _UNICODE
	hr = m_pBuild->SetOutputFileName(&MEDIATYPE_Avi, szFileName, &pMux, 0);
#else
	WCHAR *wszFileName;
	int i = AnsiToWide(szFileName, cchFileName, &wszFileName);
	if (!wszFileName)
	{
		return E_FAIL;
	}

	hr = m_pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, wszFileName, &pMux, 0);
	delete [] wszFileName;
#endif

	if (FAILED(hr))
	{
		return hr;
	}
	
    // First try to render an interleaved stream (DV)
	hr = m_pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		m_pCap, 0, pMux);
	if (FAILED(hr))
	{
        // Next try a video stream.
		hr = m_pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			m_pCap, 0, pMux);

        // If we have an audio capture device, render the audio stream.
        // Don't do this for DV because it's already got audio.

        // But first! Enable the audio input that the user selected. This is pretty ugly but the 
        // idea is simple. An audio capture device can have multiple inputs. Each one is represented 
        // by an input pin. The name of each pin indicates which input it controls. For example, 
        // "Line In" or "CD Audio." Alas, the names are not standardized, so the best we can do 
        // is ask the user to pick one. Then we have to enumerate all the input pins and ask
        // each one for its name. If it matches, we enable that pin.

		if (SUCCEEDED(hr) && m_pAudioCap)
		{
            // Enumerate the pins.
            CComPtr<IEnumPins> pEnum;
            CComPtr<IPin> pPin;
            m_pAudioCap->EnumPins(&pEnum);
            while (S_OK == pEnum->Next(1, &pPin, NULL))
            {
                // QI for IAMAudioInputMixer interface.
                CComQIPtr<IAMAudioInputMixer> pMix(pPin);
                if (pMix)
                {
                    // Get the pin info, which includes the name and the 
                    PIN_INFO PinInfo;
                    hr = pPin->QueryPinInfo(&PinInfo);
                    if (FAILED(hr))
                    {
                        break;
                    }
                    PinInfo.pFilter->Release();
                    if (0 == wcscmp(PinInfo.achName, m_szAudioInput))
                    {
                        pMix->put_Enable(TRUE);
                        pMix->put_MixLevel(1.0);
                        break;
                    }
                }
                pPin.Release();

                // Possibly we did not get the pin we want, but let's render it anyway and hope for the best...
                hr = m_pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
                    m_pAudioCap, 0, pMux);

            } // while
		}

	}

	if (FAILED(hr))
	{
		// Abject failure. Try to recover the preview, at least.
		TearDownGraph();
		RenderPreview();
	}

	StopCapture();  // Don't capture now, but get ready to do so.
	m_pControl->Run();

	return hr;
}



//-----------------------------------------------------------------------------
// Name: RenderTransmit()
// Desc: Build a graph to transmit from DV file to VTR tape.
//
// szFileName: Source file name
// cchFileName: Size of file name in characters, including '\0'
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::RenderTransmit(const TCHAR* szFileName, int cchFileName)
{
	OutputDebugString(TEXT("RenderTransmit()\n"));

	if (!m_pCap) return E_FAIL;

    // Transmit graph looks totally different from preview/capture graph, so tear
    // down and start over.
	TearDownGraph();

	HRESULT hr;
	
#ifndef _UNICODE
	WCHAR *wszFileName;
	int i = AnsiToWide(szFileName, cchFileName, &wszFileName);
	if (!wszFileName)
	{
		return E_FAIL;
	}
#endif

    // We don't know if it's a type-1 or type-2 file. Try type-1 first...

#ifdef _UNICODE
	hr = RenderType1Transmit(szFileName);
#else
	hr = RenderType1Transmit(wszFileName);
#endif

	if (FAILED(hr))
	{
		// Not a type-1 file, let's try type 2
		TearDownGraph();
#ifdef _UNICODE
		hr = RenderType2Transmit(szFileName);
#else
		hr = RenderType2Transmit(wszFileName);
#endif
		if (FAILED(hr))
		{
			// Failed -- maybe it wasn't a DV file at all! Try to recover the preview graph....
			TearDownGraph();
			RenderPreview();
			m_pControl->Run();
		}
	}

#ifndef _UNICODE
	delete [] wszFileName;
#endif

	// If we build the transmit graph successfully, we do *not* run the graph,
	// because the caller needs to coordinate with the DV device.
	return hr;
}


//-----------------------------------------------------------------------------
// Name: RenderType1Transmit()
// Desc: Build a graph to transmit from type-1 DV file to VTR tape.
//
// wszFileName: Source file name - note this is WCHAR not TCHAR
// cchFileName: Size of file name in characters, including '\0'
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::RenderType1Transmit(const WCHAR* wszFileName)
{
   	_ASSERTE(m_pCap != NULL);
 
    HRESULT hr;

    // Target graph looks like this:

    // File src -> AVI Splitter -> Inf Pin Tee --> MSDV (transmit)
    //                                         --> DV Splitter --> audio / video renderers (preview)

	// Add the Infinite Pin Tee filter to the graph
	CComPtr<IBaseFilter> pTee;
    hr = AddFilter(m_pGraph, CLSID_InfTee, L"Tee", &pTee);
	if (FAILED(hr))
	{
		return hr;
	}

	// Add the File Source filter to the graph.
	CComPtr<IBaseFilter> pFileSource;
	hr = m_pGraph->AddSourceFilter(wszFileName, L"Source", &pFileSource);

	if (SUCCEEDED(hr))
	{
        // Connect the File Src to the Tee
		hr = m_pBuild->RenderStream(0, 0, pFileSource, 0, pTee);
	}

	if (SUCCEEDED(hr))
	{
        // Connect the Tee to MSDV 
		hr = ConnectFilters(m_pGraph, pTee, m_pCap);
	}
	if (SUCCEEDED(hr))
	{
        // Find the unconnected output pin on the Tee and render it.
		CComPtr<IPin> pOut;
		hr = GetUnconnectedPin(pTee, PINDIR_OUTPUT, &pOut);
		if (SUCCEEDED(hr))
		{
			hr = m_pBuild->RenderStream(0, 0, pOut, 0, 0);  // Connect to default renderers.

            // This automatically adds the DV Splitter, DV Dec, (old) Video Renderer, and DSound Renderer.
		}
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: RenderType2Transmit()
// Desc: Build a graph to transmit from type-2 DV file to VTR tape.
//
// wszFileName: Source file name - note this is WCHAR not TCHAR
// cchFileName: Size of file name in characters, including '\0'
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::RenderType2Transmit(const WCHAR* wszFileName)
{
	_ASSERTE(m_pCap != NULL);

    HRESULT hr;

    // Target graph looks like this:

    // File Src -> AVI Split -> DV Mux -> Inf Pin Tee -> MSDV (transmit)
    //                                                -> DV Split -> ... renderers (for preview)

	CComPtr<IBaseFilter> pDVMux;
	CComPtr<IBaseFilter> pFileSource;

    // Add the DV Mux filter

    hr = AddFilter(m_pGraph, CLSID_DVMux, L"DV Mux", &pDVMux);
	if (FAILED(hr))
	{
		return hr;
	}

    // Add the File Source
	hr = m_pGraph->AddSourceFilter(wszFileName, L"Source", &pFileSource);
	if (FAILED(hr))
	{
		return hr;
	}

    // Connect the File Source to the DV Mux. This will add the splitter and connect one pin.
	hr = ConnectFilters(m_pGraph, pFileSource, pDVMux);
	if (SUCCEEDED(hr))
	{
        // Find the AVI Splitter, which should be the filter downstream from the File Source
		CComPtr<IBaseFilter> pSplit;
		hr = FindConnectedFilter(pFileSource, PINDIR_OUTPUT, &pSplit);
		if (SUCCEEDED(hr))
		{
            // Connect the second pin from the AVI Splitter to the DV Mux
			hr = ConnectFilters(m_pGraph, pSplit, pDVMux);
		}
	}

	if (FAILED(hr))
	{
		return hr;
	}

    // Add the Infinite Pin Tee.
	CComPtr<IBaseFilter> pTee;
	hr = AddFilter(m_pGraph, CLSID_InfTee, L"Tee", &pTee);

	if (FAILED(hr))
	{
		return hr;
	}

    // Connect the DV Mux to the Tee
	hr = ConnectFilters(m_pGraph, pDVMux, pTee);
	if (FAILED(hr))
	{
		return hr;
	}

    // Connect the Tee to MSDV
	hr = ConnectFilters(m_pGraph, pTee, m_pCap);
	if (FAILED(hr))
	{
		return hr;
	}

    // Render the other pin on the Tee, for preview.
	hr = m_pBuild->RenderStream(0, &MEDIATYPE_Interleaved, pTee, 0, 0);

	return hr;
}



//-----------------------------------------------------------------------------
// Name: RenderStillPin()
// Desc: Render a still pin on the device filter, if there is one
//
// szFileName: [Name of the target file to save the bitmap.] <<< Currently ignored! :-)
// cchFileName: Size of file name in characters, including '\0'
//-----------------------------------------------------------------------------


HRESULT CCaptureGraph::RenderStillPin(const TCHAR *szFilename, int cchFileName)
{
	OutputDebugString(TEXT("RenderStillPin()\n"));

	if (!m_pCap)
	{
		return E_FAIL;
	}

	HRESULT hr;

    // The goal is to connect a still pin to the Sample Grabber filter. But before
    // we waste time configuring the Sample Grabber, let's check if there is a still pin.

    CComPtr<IPin> pStillPin;
    hr = m_pBuild->FindPin(m_pCap, PINDIR_OUTPUT, &PIN_CATEGORY_STILL, 0, FALSE, 0, &pStillPin);

    if (FAILED(hr))
    {
        // No still pin ... give up now.
        return hr;
    }

    // Create the callback object for the Sample Grabber filter.
	
    StillCap *pStillCapCB_Obj = new StillCap();
    if (pStillCapCB_Obj == 0)
    {
		return E_OUTOFMEMORY;
    }
    
    CComQIPtr<ISampleGrabberCB> pStillCapCB(pStillCapCB_Obj);

    _ASSERTE(pStillCapCB);


    // Add the Sample Grabber Filter
	CComPtr<IBaseFilter> pSG_Filter;
	hr = AddFilter(m_pGraph, CLSID_SampleGrabber, L"Sample Grabber", &pSG_Filter);
	if (FAILED(hr))
	{
		return hr;
	}

    // QI the Sample Grabber for it's ISampleGrabber interface.
	CComQIPtr<ISampleGrabber> pSG(pSG_Filter);
	if (!pSG)
	{
		m_pGraph->RemoveFilter(pSG_Filter);
		return E_NOINTERFACE; // There's no reason this should happen!
	}

    // We'll use this media type to force the Sample Grabber to connect with RGB types.
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;

	pSG->SetOneShot(FALSE);  // No one-shot mode (refer to the docs for details)
	pSG->SetBufferSamples(TRUE); // Yes, copy each sample into a buffer
	pSG->SetCallback(pStillCapCB, 1); // Set the callback. 

    // Add the Null Renderer filter to the graph. 
	CComPtr<IBaseFilter> pNull;
	hr = AddFilter(m_pGraph, CLSID_NullRenderer, L"Null Renderer", &pNull);
	if (FAILED(hr))
	{
		m_pGraph->RemoveFilter(pSG_Filter);
		return hr;
	}


    // These are the types we'll try to connect with. 
    const GUID *pTypes[] = { &MEDIASUBTYPE_RGB32, &MEDIASUBTYPE_RGB24, &MEDIASUBTYPE_RGB565 };

   
    // The problem here is that we only want uncompressed RGB types, but we can only configure 
    // the Sample Grabber one type at a time. So, we'll just loop over them until one works. 
    // (You could also modify the "Grabber" sample that shipped with DX 8.1.)

    for (int i = 0; i < 2; i++)
    {
        mt.subtype = *(pTypes[i]);
    	pSG->SetMediaType(&mt);  // Set the type on the Sample Grabber.

        // Connect still pin -> Sample Grabber -> Null Renderer
        hr = m_pBuild->RenderStream(NULL, NULL, pStillPin, pSG_Filter, pNull);

        // You could also do it this way:
        //    hr = m_pBuild->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, m_pCap, pSG_Filter, pNull);
        // This way searches the filter for a still pin. But we've already got the pin, so we can
        // use the simpler call.

        if (SUCCEEDED(hr))
        {
            break;
        }
    }
	if (FAILED(hr))
	{
        // This didn't work, so remove the filters.
		m_pGraph->RemoveFilter(pSG_Filter);
		m_pGraph->RemoveFilter(pNull);
        return hr;
	}

    // We're OK. Now tell our callback object about the final media type the Sample Grabber
    // connected with. (Again, modifying the "Grabber" sample might make our lives easier here.)
	hr = pSG->GetConnectedMediaType(&mt);
	if (SUCCEEDED(hr))
	{
		pStillCapCB_Obj->SetMediaType(mt);
		FreeMediaType(mt);
	}

	return hr;
}


//-----------------------------------------------------------------------------
// Name: TakeSnapShot()
// Desc: Take a snapshot by triggering the still pin (if there is one)
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::TakeSnapShot()
{
	if (!m_pCap)
	{
		return E_FAIL;
	}

    // Query the device filter for IAMVideoControl
	CComQIPtr<IAMVideoControl> pVidControl(m_pCap);
    if (pVidControl)
    {
        // Find the still pin.
        CComPtr<IPin> pPin;
        HRESULT hr = m_pBuild->FindPin(m_pCap, PINDIR_OUTPUT, &PIN_CATEGORY_STILL, 0, FALSE, 0, &pPin);

        if (SUCCEEDED(hr))
        {
            // Trigger this pin.
            hr = pVidControl->SetMode(pPin,  VideoControlFlag_Trigger  );
        }
		return hr;
    }
	// This device does not support IAMVideoControl
	return E_FAIL;
}


//-----------------------------------------------------------------------------
// Name: ConfigureTVAudio()
// Desc: Enable or mute the TV audio. 
//
// bActivate: TRUE = enable, FALSE = mute
//-----------------------------------------------------------------------------

HRESULT CCaptureGraph::ConfigureTVAudio(BOOL bActivate)
{
	if (!m_pCap) return S_FALSE;

    // Basically we have to grovel the filter graph for a crossbar filter,
    // then try to connect the Audio Decoder Out pin to the Audio Tuner In
    // pin. Some cards have two crossbar filters.

    // Search upstream for a crossbar.
    IAMCrossbar *pXBar1 = NULL;
    HRESULT hr = m_pBuild->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, m_pCap,
        IID_IAMCrossbar, (void**)&pXBar1);
    if (SUCCEEDED(hr)) 
    {
        // Try to connect the audio pins.
        hr = ConnectAudio(pXBar1, bActivate);
        if (FAILED(hr))
        {
            // Search upstream for another crossbar.
            IBaseFilter *pF = NULL;
            hr = pXBar1->QueryInterface(IID_IBaseFilter, (void**)&pF);
            if (SUCCEEDED(hr)) 
            {
                IAMCrossbar *pXBar2 = NULL;
                hr = m_pBuild->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pF,
                    IID_IAMCrossbar, (void**)&pXBar2);
                pF->Release();
                if (SUCCEEDED(hr))
                {
                    // Try to connect the audio pins.
                    hr = ConnectAudio(pXBar2, bActivate);
                    pXBar2->Release();
                }
            }
        }
        pXBar1->Release();
    }

    return hr;
}



