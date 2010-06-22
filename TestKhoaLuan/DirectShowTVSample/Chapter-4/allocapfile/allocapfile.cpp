// allocapfile.cpp : Defines the entry point for the console application.
//

// capturegraphbuilder2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
 #include "streams.h"
#include <stdio.h>





class CProgress : public CUnknown, public IAMCopyCaptureFileProgress
{
public:

    CProgress(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    	CUnknown(pName, pUnk, phr) {};
    ~CProgress() {};

    STDMETHODIMP_(ULONG) AddRef() {return 1;};
    STDMETHODIMP_(ULONG) Release() {return 0;};
    STDMETHODIMP QueryInterface(REFIID iid, void **p) {
        CheckPointer(p, E_POINTER);
        if (iid == IID_IAMCopyCaptureFileProgress) {
            return GetInterface((IAMCopyCaptureFileProgress *)this, p);
        } else {
            return E_NOINTERFACE;
        }
    };
    STDMETHODIMP Progress(int i) {
        
        printf("Save File Progress: %d%%", i);
        return S_OK;
    };
}; 

IBaseFilter *GetAudioDevice (){
	// Create the system device enumerator.
	ICreateDevEnum *pDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, 
		IID_ICreateDevEnum, (void **)&pDevEnum);

	// Create an enumerator for video capture devices.
	IEnumMoniker *pClassEnum = NULL;
	pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);

	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	IBaseFilter *pSrc = NULL;
	if (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
	  // Bind the first moniker to a filter object.
	 pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		pMoniker->Release();
	}
	pClassEnum->Release();
	pDevEnum->Release();
	return pSrc;
}
int main(int argc, char* argv[])
{
	
	
	IGraphBuilder         *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuilder = NULL;
	IBaseFilter           *pSrc = NULL;
	IBaseFilter           *ppf = NULL;
	IFileSinkFilter       *pSink = NULL;
	IMediaControl         *pMC   = NULL;
	HRESULT hr;
	
	CoInitialize (NULL);

	DbgInitialise (NULL);
	DbgLog ((LOG_TRACE, 5, TEXT("hi")));
	// Create the filter graph.
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
	      IID_IGraphBuilder, (void **)&pGraph);

	// Create the capture graph builder.
	CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
	      IID_ICaptureGraphBuilder2, (void **)&pBuilder);

	pBuilder->SetFiltergraph(pGraph);    
	
	pSrc=GetAudioDevice ();
	// add the first audio filter in the list
	pGraph->AddFilter(pSrc, L"Audio Capture");

 
    hr=	pBuilder->AllocCapFile (L"C:\\temp.avi",100000);
 
	pBuilder->RenderStream(
        &PIN_CATEGORY_CAPTURE,  // Pin category
        &MEDIATYPE_Audio,       // Media type
        pSrc,                   // Capture filter
        NULL,                   // Compression filter (optional)
        ppf                     // Multiplexer or renderer filter
    );


	pGraph->QueryInterface (IID_IMediaControl, (void **) &pMC);
	pMC->Run ();

	MessageBox (NULL, "Stop Recording", NULL, NULL);
	pMC->Stop ();

 	CProgress *pProg = new CProgress(TEXT(""), NULL, &hr);
        IAMCopyCaptureFileProgress *pIProg = NULL;
        
            hr = pProg->QueryInterface(IID_IAMCopyCaptureFileProgress,
                                            (void **)&pIProg);
				 if (FAILED (hr)) printf ("failed copying file");
	 hr = pBuilder->CopyCaptureFile (L"C:\\temp.avi", L"C:\\final.avi", TRUE, pIProg); 
   

	CoUninitialize ();
	
	return 0;
}

