// TVBuff.cpp
//

#include "TVBuff.h"			// This includes all the major include files needed

// Enumerate all of the audio input devices
// Return the filter with a matching friendly name
HRESULT GetAudioInputFilter(IBaseFilter** gottaFilter, wchar_t* matchName)
{
	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the audio input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					wprintf(L"Testing Audio Input Device: %s\n", varName.bstrVal);

					// Do a comparison, find out if it's the right one
					if (wcsncmp(varName.bstrVal, matchName, 
						wcslen(matchName)) == 0) {

						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**) gottaFilter);
						done = true;
					}
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	if (done) {
		return hr;	// found it, return native error
	} else {
		return VFW_E_NOT_FOUND;	// didn't find it error
	}
}

// Enumerate all of the video input devices
// Return the filter with a matching friendly name
HRESULT GetVideoInputFilter(IBaseFilter** gottaFilter, wchar_t* matchName)
{
	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					wprintf(L"Testing Video Input Device: %s\n", varName.bstrVal);

					// Do a comparison, find out if it's the right one
					if (wcsncmp(varName.bstrVal, matchName, 
						wcslen(matchName)) == 0) {

						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**) gottaFilter);
						done = true;
					}
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	if (done) {
		return hr;	// found it, return native error
	} else {
		return VFW_E_NOT_FOUND;	// didn't find it error
	}
}

// This code was also brazenly stolen from the DX9 SDK
// Pass it a file name in wszPath, and it will save the filter graph to that file.
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    IStorage *pStorage = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

	// Next, create a stream to store.
    IStream *pStream;
    hr = pStorage->CreateStream(
		wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

	// The IPersistStream converts a stream into a persistent object.
    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}

// A very useful bit of code 
// Stolen from the DX9 SDK
HRESULT AddFilterByCLSID(
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

//-----------------------------------------------------------------------------
// Name: ConfigureTVAudio()
// Desc: Enable or mute the TV audio. 
//-----------------------------------------------------------------------------
HRESULT ConfigureTVAudio(ICaptureGraphBuilder2* pGraph, IBaseFilter* capFilter)
{
	if (!capFilter) return S_FALSE;

    // Basically we have to grovel the filter graph for a crossbar filter,
    // then try to connect the Audio Decoder Out pin to the Audio Tuner In
    // pin. Some cards have two crossbar filters.

    // Search upstream for a crossbar.
    IAMCrossbar *pXBar1 = NULL;
    HRESULT hr = pGraph->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, capFilter,
        IID_IAMCrossbar, (void**)&pXBar1);
    if (SUCCEEDED(hr)) 
    {
        // Try to connect the audio pins.
        hr = ConnectAudio(pXBar1, true);
        if (FAILED(hr))
        {
            // Search upstream for another crossbar.
            IBaseFilter *pF = NULL;
            hr = pXBar1->QueryInterface(IID_IBaseFilter, (void**)&pF);
            if (SUCCEEDED(hr)) 
            {
                IAMCrossbar *pXBar2 = NULL;
                hr = pGraph->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pF,
                    IID_IAMCrossbar, (void**)&pXBar2);
                pF->Release();
                if (SUCCEEDED(hr))
                {
                    // Try to connect the audio pins.
                    hr = ConnectAudio(pXBar2, true);
                    pXBar2->Release();
                }
            }
        }
        pXBar1->Release();
    }

    return hr;
}

// A basic program to buffer playback from a TV Tuner using DirectShow
int main(int argc, char* argv[])
{
    ICaptureGraphBuilder2 *pCaptureGraph = NULL;	// Capture graph builder object
	IGraphBuilder *pGraph = NULL;	// Graph builder object for sink
	IMediaControl *pControl = NULL;	// Media control interface for sink

	IGraphBuilder *pGraphSource = NULL;	// Graph builder object for source
	IMediaControl *pControlSource = NULL; // Filter control interface for source

	IBaseFilter *pVideoInputFilter = NULL; // Video Capture filter
	IBaseFilter *pDVEncoder = NULL;			// DV Encoder Filter
	IBaseFilter *pAudioInputFilter = NULL;	// Audio Capture filter

	IStreamBufferSink *pBufferSink = NULL;

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
	     // We’ll send our error messages to the console.
        printf("ERROR - Could not initialize COM library");
        return hr;
	} else {

		// Create the filter graph manager and query for interfaces.
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
							IID_ICaptureGraphBuilder2, (void **)&pCaptureGraph);

		if (FAILED(hr))	// FAILED is a macro that tests the return value
		{
			printf("ERROR - Could not create the Filter Graph Manager.");
			return hr;
		}

		// Create the filter graph manager and query for interfaces.
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
							IID_IGraphBuilder, (void **)&pGraph);

		// Now get the filter graph manager
		// That's part of the capture graph builder
		hr = pCaptureGraph->SetFiltergraph(pGraph);

		// Using QueryInterface on the graph builder, 
		// Get the Media Control object.
		hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		if (FAILED(hr))
		{
			printf("ERROR - Could not create the Media Control object.");
			pCaptureGraph->Release();
			pGraph->Release();	// Clean up after ourselves.
			CoUninitialize();  // And uninitalize COM
			return hr;
		}

		// Now create the video input filter from the TV Tuner
		hr = GetVideoInputFilter(&pVideoInputFilter, L"ATI");
		if (SUCCEEDED(hr)) {
			hr = pGraph->AddFilter(pVideoInputFilter, L"TV Tuner");
		}

		// Now, let's add a DV Encoder, to get a format the SBE can use
		hr = AddFilterByCLSID(pGraph, CLSID_DVVideoEnc, L"DV Encoder", &pDVEncoder);

		// Now that the capture sources have been added to the filter graph
		// We need to add the Stream Buffer Engine Sink filter to the graph.
		// Add the Stream Buffer Source filter to the graph.
		CComPtr<IStreamBufferSink> bufferSink;
		hr = bufferSink.CoCreateInstance(CLSID_StreamBufferSink);
		CComQIPtr<IBaseFilter> pSinkF(bufferSink);
		hr = pGraph->AddFilter(pSinkF, L"SBESink");

		// Now add the video capture to the output file
		hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			pVideoInputFilter, pDVEncoder, pSinkF);

		// Now we've got to wire the Audio Crossbar for the TV signal
		hr = ConfigureTVAudio(pCaptureGraph, pVideoInputFilter);

		// Now we instance an audio capture filter
		// Which should be picking up the audio from the TV tuner...
		hr = GetAudioInputFilter(&pAudioInputFilter, L"SoundMAX Digital Audio");
		if (SUCCEEDED(hr)) {
			hr = pGraph->AddFilter(pAudioInputFilter, L"TV Tuner Audio");
		}
		
		// And now we add the audio capture to the sink
		hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			pAudioInputFilter, NULL, pSinkF);

		// And now lock the Sink filter, like we're supposed to.
		hr = bufferSink->QueryInterface(&pBufferSink);
		hr = pBufferSink->LockProfile(NULL);

		// Before we finish up, save the filter graph to a file.
		SaveGraphFile(pGraph, L"C:\\MyGraph_Sink.GRF");

		// OK now we're going to create an entirely independent filter graph.
		// This will be handling the Stream Buffer Engine Source
		// Which will be passed along to the appropriate renderers.
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
							IID_IGraphBuilder, (void **)&pGraphSource);

		// Using QueryInterface on the graph builder, 
		// Get the Media Control object.
		hr = pGraphSource->QueryInterface(IID_IMediaControl, (void **)&pControlSource);

		// Now instance the StreamBufferEngine Source
		// And add it to this filter graph
		// Add the Stream Buffer Source filter to the graph.
		CComPtr<IStreamBufferSource> pSource;
		hr = pSource.CoCreateInstance(CLSID_StreamBufferSource);
		CComQIPtr<IBaseFilter> pSourceF(pSource);
		hr = pGraphSource->AddFilter(pSourceF, L"SBESource");
		hr = pSource->SetStreamSink(bufferSink);
		CComQIPtr<IStreamBufferMediaSeeking> pSourceSeeking(pSource);

		// Now, all we need to do is enumerate the output pins on the source.
		// These should match the streams that have been setup on the sink.
		// These will need to be rendered.
		// Render each output pin.
		CComPtr<IPin> pSrcOut;
		CComPtr<IEnumPins> pPinEnum;
		hr = pSourceF->EnumPins(&pPinEnum);
		while (hr = pPinEnum->Next(1, &pSrcOut, 0), hr == S_OK)
		{
			hr = pGraphSource->Render(pSrcOut);
			pSrcOut.Release();
		}

		// Before we finish up, save the filter graph to a file.
		// However, for reasons known only to DShow, I can't read it.
		SaveGraphFile(pGraphSource, L"C:\\MyGraph_Source.GRF");

		if (SUCCEEDED(hr))
		{
			// Run the graphs.  Both of them.
			hr = pControl->Run();
			hr = pControlSource->Run();

			if (SUCCEEDED(hr))
			{
				// Wait patiently for completion of the recording
				wprintf(L"ENTER to stop, SPACE to pause, BACKSPACE to rewind, F to fastforward.\n");

				bool done = false;
				bool paused = false;
				while (!done) {

					// Wait for completion.
					int ch;
					ch = _getch();		// We wait for keyboard input
					switch (ch) {
						case 0x0d:		// ENTER
							done = true;
							break;

						case 0x20:		// SPACE
							if (paused) {
								wprintf(L"Playing...\n");
								pControlSource->Run();
								paused = false;
							} else {
								wprintf(L"Pausing...\n");
								pControlSource->Pause();
								paused = true;
							}
							break;

						case 0x08:		// BACKSPACE - Rewind one second, if possible.

							// First, let's find out how much play we have.
							// We do this by finding the earliest, latest
							// Current and stop positions
							// These are in units of 100 nanoseconds.  Supposedly.
							LONGLONG earliest, latest, current, stop, rewind;
							hr = pSourceSeeking->GetAvailable(&earliest, &latest);
							hr = pSourceSeeking->GetPositions(&current, &stop);

							// We would like to rewind 1 second, or 10000000 units
							if ((current - earliest) > 10000000) {		// Can we?
								rewind = current - 10000000;				// Yes.
							} else {
								rewind = earliest;	// Just back up as far as we can
							}

							// If we can, change the current position 
							// Without changing the stop position
							hr = pSourceSeeking->SetPositions(&rewind, AM_SEEKING_AbsolutePositioning, 
								NULL, AM_SEEKING_NoPositioning);

							break;

						case 0x46:		// That's F
						case 0x66:		// And f - Fast Forward one second, if possible.

							// First, let's find out how much play we have.
							// We do this by finding the earliest, latest
							// Current and stop positions
							// These are in units of 100 nanoseconds.  Supposedly.
							LONGLONG fastforward;
							hr = pSourceSeeking->GetAvailable(&earliest, &latest);
							hr = pSourceSeeking->GetPositions(&current, &stop);

							// We would like to fast-forward 1 second, or 10000000 units
							if ((latest - current) > 10000000) {		// Can we?
								fastforward = current + 10000000;				// Yes.
							} else {
								fastforward = latest;	// Just go forward
							}

							// If we can, change the current position 
							// Without changing the stop position
							hr = pSourceSeeking->SetPositions(&fastforward, AM_SEEKING_AbsolutePositioning, 
								NULL, AM_SEEKING_NoPositioning);

							break;

						default:		// Ignore Other Keys
							break;
					}
				}
			}

			// And let's stop the filter graph
			hr = pControlSource->Stop();
			hr = pControl->Stop();

			wprintf(L"Stopped.\n");	// To the console
		}

		// Now release everything, and clean up.
		pDVEncoder->Release();
		pVideoInputFilter->Release();
		pAudioInputFilter->Release();
		pControlSource->Release();
		pGraphSource->Release();
		pControl->Release();
		pGraph->Release();
		pCaptureGraph->Release();
		pBufferSink->Release();
	}
    CoUninitialize();

	return 0;
}


