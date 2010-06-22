// DSAudioCap.cpp
//

#include "DSAudioCap.h"			// This includes all the major include files needed

// Code adopted from example in the DX9 SDK.
// This code allows us to find the input pins an audio input filter
// We'll print out a list of them, indicating the enabled one
HRESULT EnumerateAudioInputPins(IBaseFilter *pFilter)
{
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
	PIN_DIRECTION PinDirThis;
	PIN_INFO	pInfo;
	IAMAudioInputMixer *pAMAIM = NULL;
	BOOL pfEnable = FALSE;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return NULL;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll examine it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
		// Get the pin direction
        pPin->QueryDirection(&PinDirThis);
		if (PinDirThis == PINDIR_INPUT) {

			// OK, we've found an input pin on the filter, yay!
			// Now let's get the information on that pin
			// So we can print the name of the pin to the console
			hr = pPin->QueryPinInfo(&pInfo);
			if (SUCCEEDED(hr)) {
				wprintf(L"Input pin: %s\n", pInfo.achName);

				// Now let's get the correct interface
				hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void**) &pAMAIM);
				if (SUCCEEDED(hr)) {

					// OK, so now we can find out if the pin is enabled
					// Meaning it's the active input pin on the filter
					hr = pAMAIM->get_Enable(&pfEnable);
					if (SUCCEEDED(hr)) {
						if (pfEnable) {
							wprintf(L"\tENABLED\n");
						}
					}
					pAMAIM->Release();
				}
				pInfo.pFilter->Release();
			}
		}
        pPin->Release();
    }
    pEnum->Release();
    return hr;  
}

// Enumerate all of the audio input devices
// Return the _first_ of these to the caller
// That should be the one chosen in the control panel.
HRESULT EnumerateAudioInputFilters(void** gottaFilter)
{
	// Once again, code stolen from the DX9 SDK

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
		if (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
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
					wprintf(L"Selecting Audio Input Device: %s\n", varName.bstrVal);
				}
				VariantClear(&varName);

				// To create an instance of the filter, do the following:
				//Remember to release pFilter later
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, gottaFilter);
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return hr;
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

// Find an unconnected pin on a filter
// This too is stolen from the DX9 SDK
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

// Connect two filters together with the filter graph manager
// Stolen from the DX9 SDK
// This is the base version
HRESULT ConnectFilters(
    IGraphBuilder *pGraph, // Filter Graph Manager.
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest)    // Downstream filter.
{
    if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }

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

// Connect two filters together with the filter graph manager
// Again, stolen from the DX9 SDK
// This is an overloaded version
HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc, 
    IBaseFilter *pDest)
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

// A very simple program to capture audio to a file using DirectShow
//
int main(int argc, char* argv[])
{
    IGraphBuilder *pGraph = NULL;	// Filter graph builder object
    IMediaControl *pControl = NULL;	// Media control object
	IFileSinkFilter *pSink = NULL;	// File sink object
	IBaseFilter *pAudioInputFilter = NULL; // Audio Capture object
	IBaseFilter *pFileWriter = NULL;	// File writer object

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
	     // We’ll send our error messages to the console.
        printf("ERROR - Could not initialize COM library");
        return hr;
    }

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr))	// FAILED is a macro that tests the return value
    {
        printf("ERROR - Could not create the Filter Graph Manager.");
        return hr;
    }

	 // Using QueryInterface on the graph builder, 
    // Get the Media Control object.
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr))
    {
        printf("ERROR - Could not create the Media Control object.");
        pGraph->Release();	// Clean up after ourselves.
		CoUninitialize();  // And uninitalize COM
        return hr;
    }

	// OK, so now we want to build the filter graph
	// Using an AudioCapture filter.
	// But there are several to choose from
	// So we need to enumerate them, then pick one.
	hr = EnumerateAudioInputFilters((void**) &pAudioInputFilter);
	hr = EnumerateAudioInputPins(pAudioInputFilter);

	// Add the audio capture filter to the filter graph. 
	hr = pGraph->AddFilter(pAudioInputFilter, L"Capture");

	// Next add the AVIMux (you'll see why)
	IBaseFilter *pAVIMux = NULL;
	hr = AddFilterByCLSID(pGraph, CLSID_AviDest, L"AVI Mux", &pAVIMux);

	// Connect the filters.
	hr = ConnectFilters(pGraph, pAudioInputFilter, pAVIMux);

	// And now we instance a file writer filter
	hr = AddFilterByCLSID(pGraph, CLSID_FileWriter, L"File Writer", &pFileWriter);

	// Set the file name.
	hr = pFileWriter->QueryInterface(IID_IFileSinkFilter, (void**)&pSink);
	pSink->SetFileName(L"C:\\MyAVIFile.AVI", NULL);

	// Connect the filters.
	hr = ConnectFilters(pGraph, pAVIMux, pFileWriter);

    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
			// Wait patiently for completion of the recording
			wprintf(L"Started recording...press Enter to stop recording.\n");

            // Wait for completion.
			char ch;
			ch = getchar();		// We wait for keyboard input
        }

		// And let's stop the filter graph
		hr = pControl->Stop();

		wprintf(L"Stopped recording.\n");	// To the console

		// Before we finish up, save the filter graph to a file.
		SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
    }

	// Now release everything, and clean up.
	pSink->Release();
	pAVIMux->Release();
	pFileWriter->Release();
	pAudioInputFilter->Release();
    pControl->Release();
    pGraph->Release();
    CoUninitialize();

	return 0;
}


