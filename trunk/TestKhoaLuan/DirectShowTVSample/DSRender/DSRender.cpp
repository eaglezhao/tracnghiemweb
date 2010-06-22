// DSRender.cpp
//

#include "DSRender.h"			// This includes all the major include files needed

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
	ofn.lpstrTitle = "Select file to render...";
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

// DSRender.cpp
// A very simple program to render media files using DirectShow
//
int main(int argc, char* argv[])
{
    IGraphBuilder *pGraph = NULL;	// Filter graph builder object
    IMediaControl *pControl = NULL;	// Media control object
    IMediaEvent   *pEvent = NULL;	// Media event object

	if (!GetMediaFileName()) {		// local function to get a file name
		return(0);				// If we didn’t get it, exit
	}

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

	 // And get the Media Event object, too.
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
 if (FAILED(hr))
    {
        printf("ERROR - Could not create the Media Event object.");
        pGraph->Release();	// Clean up after ourselves.
        pControl->Release();
	    CoUninitialize();  // And uninitalize COM
        return hr;
    }

// To build the filter graph, only one call is required.
 // We make the RenderFile() call to the filter graph builder
 // Which we pass with the name of the media file
#ifndef UNICODE
    WCHAR wFileName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName, MAX_PATH);

	// This is all that’s required to create a filter graph
    // Which will render a media file!
    hr = pGraph->RenderFile((LPCWSTR)wFileName, NULL);
#else
    hr = pGraph->RenderFile((LPCWSTR)g_PathFileName, NULL);
#endif

    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
            // Wait for completion.
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);

            // Note: Do not use INFINITE in a real application, 
// because it can block indefinitely.
        }

		// And let's stop the filter graph
		hr = pControl->Stop();

		// Before we finish up, save the filter graph to a file.
		SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
    }

	// Now release everything, and clean up.
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();

	return 0;
}


