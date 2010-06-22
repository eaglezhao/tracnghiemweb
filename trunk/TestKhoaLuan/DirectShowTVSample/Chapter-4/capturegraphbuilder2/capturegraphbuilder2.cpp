// capturegraphbuilder2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "streams.h"
#include <dshow.h>
 



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


/*class CProgress : public CUnknown, public IAMCopyCaptureFileProgress
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
        TCHAR tach[80];
        wsprintf(tach, TEXT("Save File Progress: %d%%"), i);
        return S_OK;
    };
};*/

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
	// Create the filter graph.
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
	      IID_IGraphBuilder, (void **)&pGraph);

	// Create the capture graph builder.
	CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
	      IID_ICaptureGraphBuilder2, (void **)&pBuilder);

	pBuilder->SetFiltergraph(pGraph);    
	
	pSrc=GetAudioDevice ();
	// add the first audio filter in the list
	pGraph->AddFilter(pSrc, L"Video Capture");

/*	pBuilder->SetOutputFileName(
		&MEDIASUBTYPE_Avi,
		L"C:\\Example.avi", 
		&ppf, 
		&pSink);*/
//	pBuilder->AllocCapFile (L"C:\\temp.avi", _MAX_PATH);

	pBuilder->RenderStream(
        &PIN_CATEGORY_CAPTURE,  // Pin category
        &MEDIATYPE_Audio,       // Media type
        pSrc,                   // Capture filter
        NULL,                   // Compression filter (optional)
        ppf                     // Multiplexer or renderer filter
    );


 
	REFERENCE_TIME  rtStart = 20000000, 
                rtStop = 50000000;

/*	pBuilder->ControlStream(
        &PIN_CATEGORY_CAPTURE, 
        &MEDIATYPE_Audio, 
        pSrc,       // Source filter
        &rtStart,   // Start time
        &rtStop,    // Stop time
        0,          // Start cookie
        0           // Stop cookie
	 );*/

	pGraph->QueryInterface (IID_IMediaControl, (void **) &pMC);
	pMC->Run ();

	MessageBox (NULL, "Stop Recording", NULL, NULL);
	pMC->Stop ();

/*	CProgress *pProg = new CProgress(TEXT(""), NULL, &hr);
        IAMCopyCaptureFileProgress *pIProg = NULL;
        
            hr = pProg->QueryInterface(IID_IAMCopyCaptureFileProgress,
                                            (void **)&pIProg);
	//pBuilder->CopyCaptureFile (L"C:\\temp.avi", L"C:\\final.avi", TRUE, pIProg);*/
   
	CoUninitialize ();
	
	return 0;
}
