// DSRender.cpp
//

#include "DSBuild.h"			// This includes all the major include files needed

// And now a few globals
char g_fileName[256];
char g_PathFileName[512];

BOOL GetMediaFileName(void)
{
	OPENFILENAME ofn;

	// Let's have fun with GetOpenFileName, kids!
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = (char*) calloc(1, 512);			// Allocate space, we'll free it later...
	ofn.nMaxFile = 511;
	ofn.lpstrFileTitle = (char*) calloc(1, 256);
	ofn.nMaxFileTitle = 255;
	ofn.lpstrInitialDir = NULL;				// Point to current directory
	ofn.lpstrTitle = "Select media file to render...";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	if (!GetOpenFileName(&ofn)) {
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
		return(false);
	} else {
		strcpy(g_PathFileName, ofn.lpstrFile);
		strcpy(g_fileName, ofn.lpstrFileTitle);
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
	}
	return(true);
}

// This code was also brazenly stolen from the DX9 SDK
// Pass it a file name in wszPath, and it will save the filter graph to that file.
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    
    IStorage *pStorage = NULL;
    hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

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

// More code brazenly stolen from the DX9 SDK.
// This code allows us to find a pin (input or output) on a filter, and return it.
IPin *GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
    BOOL       bFound = FALSE;
    IEnumPins  *pEnum;
    IPin       *pPin;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return NULL;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll return with it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (bFound = (PinDir == PinDirThis))
            break;
        pPin->Release();
    }
    pEnum->Release();
    return (bFound ? pPin : NULL);  
}

// 
// DSBuild implements a very simple program to render audio files, 
// Or the audio portion of movies.
//
int main(int argc, char* argv[])
{
    IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEvent   *pEvent = NULL;
	IBaseFilter	  *pInputFileFilter = NULL;
	IBaseFilter   *pDSoundRenderer = NULL;
	IPin		  *pFileOut = NULL, *pWAVIn = NULL;

	// Get the name of an audio or movie file to play
	if (!GetMediaFileName()) {
		return(0);
	}

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("ERROR - Could not initialize COM library");
        return hr;
    }

    // Create the filter graph manager object.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr))
    {
        printf("ERROR - Could not create the Filter Graph Manager.");
		CoUninitialize();
        return hr;
    }

	// Now get the media control object
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	if (FAILED(hr)) {
		pGraph->Release();
		CoUninitialize();
		return hr;
	}

	// And the media event object
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	if (FAILED(hr)) {
		pControl->Release();
		pGraph->Release();
		CoUninitialize();
		return hr;
	}

    // Build the graph.
	// Step one is to invoke AddSourceFilter, with the file name we picked out earlier.
	// Should be an audio file (or a movie file with an audio track) to work correctly.
	// This member function instances the source filter and adds it to the graph.
#ifndef UNICODE
    WCHAR wFileName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName, MAX_PATH);
    hr = pGraph->AddSourceFilter(wFileName, wFileName, &pInputFileFilter);
#else
    hr = pGraph->AddSourceFilter(wFileName, wFileName, &pInputFileFilter);
#endif

	if (SUCCEEDED(hr)) {

		// Now, create an instance of the audio renderer
		hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pDSoundRenderer);

		if (SUCCEEDED(hr)) {

			// And add the it to the filter graph
			// Using the member function AddFilter
			hr = pGraph->AddFilter(pDSoundRenderer, L"Audio Renderer");

			if (SUCCEEDED(hr)) {

				// Now we need to connect the output pin of the source 
				// to the input pin of the parser.
				// obtain the output pin of the source filter
				// We use the filter's member function GetPin to do this.
				pFileOut = GetPin(pInputFileFilter, PINDIR_OUTPUT);

				if (pFileOut != NULL) {	// Is good?

					// again - obtain the input pin of the WAV parser
					pWAVIn = GetPin(pDSoundRenderer, PINDIR_INPUT);

					if (pWAVIn != NULL) {	// Is good?

						// And now, connect the pins together, intelligently
						// We use the filter graph builder's member function Connect
						// Which uses intelligent connect, if need be.
						// If this fails, DirectShow couldn't render the media file
						hr = pGraph->Connect(pFileOut, pWAVIn);
					}
				}
			}
		}
	}

    if (SUCCEEDED(hr))
    {
        // Run the graph.
		printf("Beginning to play media file...\n");
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
            // Wait for completion.
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);

            // Note: Do not use INFINITE in a real application 
			// because it can block indefinitely.
        }
		hr = pControl->Stop();
    }

	// Before we finish up, save the filter graph to a file.
	SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");

	// Now release everything we instanced
	// That is, if it got instanced
	if(pFileOut) {		// If it exists, non-NULL
		pFileOut->Release();	// Then release it
	}
	if (pWAVIn) {
		pWAVIn->Release();
	}
	if (pInputFileFilter) {
		pInputFileFilter->Release();
	}
	if (pDSoundRenderer) {
		pDSoundRenderer->Release();
	}
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();

	return 0;
}

