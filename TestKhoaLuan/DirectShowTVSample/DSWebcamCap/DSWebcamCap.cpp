// DSWebcamCap.cpp
//

#include "DSWebcamCap.h"			// This includes all the major include files needed

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

// Show the property pages for a filter
// This is stolen from the DX9 SDK
HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter) {

	/* Obtain the filter's IBaseFilter interface. (Not shown) */
	ISpecifyPropertyPages *pProp;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo); 
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // Reserved
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			&pFilterUnk,            // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL                 // Reserved
		);

		// Clean up.
		pFilterUnk->Release();
		FilterInfo.pGraph->Release(); 
		CoTaskMemFree(caGUID.pElems);
	}
	return hr;
}

// A very simple program to capture a webcam & audio to a file using DirectShow
//
int main(int argc, char* argv[])
{
    ICaptureGraphBuilder2 *pCaptureGraph = NULL;	// Capture graph builder object
	IGraphBuilder *pGraph = NULL;	// Graph builder object
    IMediaControl *pControl = NULL;	// Media control object
	IFileSinkFilter *pSink = NULL;	// File sink object
	IBaseFilter *pAudioInputFilter = NULL; // Audio Capture filter
	IBaseFilter *pVideoInputFilter = NULL; // Video Capture filter
	IBaseFilter *pASFWriter = NULL;	// WM ASF File config interface

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
	     // We’ll send our error messages to the console.
        printf("ERROR - Could not initialize COM library");
        return hr;
    }

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICaptureGraphBuilder2, (void **)&pCaptureGraph);
    if (FAILED(hr))	// FAILED is a macro that tests the return value
    {
        printf("ERROR - Could not create the Filter Graph Manager.");
        return hr;
    }

	// Use a method of the capture graph builder
	// To create an output path for the stream 
	hr = pCaptureGraph->SetOutputFileName(&MEDIASUBTYPE_Asf, 
		L"C:\\MyWebcam.ASF", &pASFWriter, &pSink);

	// Now configure the ASF Writer
	// Present the property pages for this filter
	hr = ShowFilterPropertyPages(pASFWriter);

	// Now get the filter graph manager
	// That's part of the capture graph builder
	hr = pCaptureGraph->GetFiltergraph(&pGraph);

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

	// Get an AudioCapture filter.
	// But there are several to choose from
	// So we need to enumerate them, and pick one.
	// Then add the audio capture filter to the filter graph. 
	hr = GetAudioInputFilter(&pAudioInputFilter, L"Logitech");
	if (SUCCEEDED(hr)) {
		hr = pGraph->AddFilter(pAudioInputFilter, L"Webcam Audio Capture");
	}

	// Now create the video input filter from the webcam
	hr = GetVideoInputFilter(&pVideoInputFilter, L"Logitech");
	if (SUCCEEDED(hr)) {
		hr = pGraph->AddFilter(pVideoInputFilter, L"Webcam Video Capture");
	}

	// Add a video renderer
	//IBaseFilter *pVideoRenderer = NULL;
	//hr = AddFilterByCLSID(pGraph, CLSID_VideoRenderer, L"Video Renderer", &pVideoRenderer);

	// Use another method of the capture graph builder
	// To provide a render path for video preview
	IBaseFilter *pIntermediate = NULL;
	hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
		pVideoInputFilter, NULL, NULL);

	// Now add the video capture to the output file
	hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pVideoInputFilter, NULL, pASFWriter);
	
	// And do the same for the audio
	hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
		pAudioInputFilter, NULL, pASFWriter);

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
	pASFWriter->Release();
	pVideoInputFilter->Release();
	pAudioInputFilter->Release();
    pControl->Release();
    pGraph->Release();
	pCaptureGraph->Release();
    CoUninitialize();

	return 0;
}


