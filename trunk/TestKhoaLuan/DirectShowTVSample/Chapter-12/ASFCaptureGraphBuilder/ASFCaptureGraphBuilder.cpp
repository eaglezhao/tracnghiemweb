// ASFCaptureGraphBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "streams.h"
#include "wmsdk.h"
#include <dshowasf.h>
#include "C:\WMSDK\WMFSDK\samples\genprofile\lib\genprofile.h"

// Declare and implement a key provider class derived from IServiceProvider.

class CKeyProvider : public IServiceProvider {
public:
    // IUnknown interface
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    CKeyProvider();

    // IServiceProvider
    STDMETHODIMP QueryService(REFIID siid, REFIID riid, void **ppv);
    
private:
    ULONG m_cRef;
};

CKeyProvider::CKeyProvider() : m_cRef(0)
{
}

//////////////////////////////////////////////////////////////////////////
//
// IUnknown methods
//
//////////////////////////////////////////////////////////////////////////

ULONG CKeyProvider::AddRef()
{
    return ++m_cRef;
}

ULONG CKeyProvider::Release()
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if (m_cRef == 0) {
        delete this;

        // Don't return m_cRef, because the object doesn't exist anymore.
        return((ULONG) 0);
    }

    return(m_cRef);
}


// We only support IUnknown and IServiceProvider.
HRESULT CKeyProvider::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IServiceProvider || riid == IID_IUnknown) {
        *ppv = (void *) static_cast<IServiceProvider *>(this);
        AddRef();
        return NOERROR;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP CKeyProvider::QueryService(REFIID siid, REFIID riid, void **ppv)
{
    if (siid == __uuidof(IWMReader) && riid == IID_IUnknown) {
        IUnknown *punkCert;
        
        HRESULT hr = WMCreateCertificate(&punkCert);

        if (SUCCEEDED(hr)) {
            *ppv = (void *) punkCert;
        }

        return hr;
    }
    return E_NOINTERFACE;
}
IWMProfile *GetProfile ()
{

    HRESULT hr = S_OK;
    IWMProfile *pWMProfile = NULL;
    AUDIO_PARAMS ap;
    VIDEO_PARAMS rgvp[ 5 ];
    SCRIPT_PARAMS sp;
    ZeroMemory( rgvp, sizeof( rgvp ) );

    sp.dwBitrate = DEFAULT_SCRIPT_STREAM_BANDWIDTH;

         //
    // Create audio only profile
    //
    ap.dwFormatTag = CODEC_AUDIO_MSAUDIO;
    ap.dwBitrate = 32000;
    ap.dwSampleRate = 32000;
    ap.dwChannels = 2;
    hr = GenerateProfile( &ap, NULL, 0, NULL, &pWMProfile);
	return pWMProfile;
}



int main(int argc, char* argv[])
{
    ICaptureGraphBuilder2   *pBuild = NULL;
    IGraphBuilder           *pGraph = NULL;
    IBaseFilter             *pSrc = NULL;       // Source filter
    IBaseFilter             *pMux = NULL;       // MUX filter
    IBaseFilter             *pVComp = NULL;     // Video compressor filter

    CoInitialize(NULL);

    // Create the capture graph builder.
    CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
            IID_ICaptureGraphBuilder2, (void **)&pBuild);
		
    // Make the rendering section of the graph.
    pBuild->SetOutputFileName(
            &MEDIASUBTYPE_Asf,  // File type. 
            L"C:\\Output.wma",  // File name.
            NULL,              // Receives a pointer to the multiplexer.
            NULL);      

	CKeyProvider prov;
	prov.AddRef();  

	IObjectWithSite* pObjectWithSite = NULL;

	HRESULT hr = pGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
	if (SUCCEEDED(hr))
	  {
		pObjectWithSite->SetSite((IUnknown *) (IServiceProvider *) &prov);
		pObjectWithSite->Release();
	  }

        // Receives a pointer to the file writer. 
    
    // Load the source file.
    pBuild->GetFiltergraph(&pGraph);
    pGraph->AddSourceFilter(L"C:\\Program Files\\Microsoft Money\\STDIUE2.avi", L"Source Filter", &pSrc);

    // Add the compressor filter.
    CoCreateInstance(CLSID_AVICo, NULL, CLSCTX_INPROC,  
            IID_IBaseFilter, (void **)&pVComp); 
    pGraph->AddFilter(pVComp, L"Compressor");

    // Render the video stream, through the compressor.
    pBuild->RenderStream(
            NULL,       // Output pin category
            NULL,       // Media type
            pSrc,       // Source filter
            pVComp,     // Compressor filter
           NULL);      // Sink filter (the AVI Mux)

    // Render the audio stream.
    pBuild->RenderStream(NULL, NULL, pSrc, NULL, pMux);

    // Set video compression properties.
    IAMStreamConfig         *pStreamConfig = NULL;
    IAMVideoCompression     *pCompress = NULL;

    // Compress at 100k/second data rate.
    AM_MEDIA_TYPE *pmt;
    pBuild->FindInterface(NULL, NULL, pVComp, IID_IAMStreamConfig, (void **)&pStreamConfig);                           
    pStreamConfig->GetFormat(&pmt);
    if (pmt->formattype == FORMAT_VideoInfo) 
    {
        ((VIDEOINFOHEADER *)(pmt->pbFormat))->dwBitRate = 100000;
        pStreamConfig->SetFormat(pmt);
    }
//    DeleteMediaType(pmt);

    // Request key frames every four frames.
    pStreamConfig->QueryInterface(IID_IAMVideoCompression, (void **)&pCompress);
    pCompress->put_KeyFrameRate(4);
    
    pCompress->Release();
    pStreamConfig->Release();

    // Run the graph.
    IMediaControl   *pControl = NULL;
    IMediaSeeking   *pSeek = NULL;
    IMediaEvent     *pEvent = NULL;
    pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
   
      hr = pMux->QueryInterface(IID_IMediaSeeking, (void**)&pSeek);

    pControl->Run();
    printf("Recompressing... \n");

    long evCode;
    if (SUCCEEDED(hr))  // IMediaSeeking is supported
    {
        REFERENCE_TIME rtTotal, rtNow = 0;
        pSeek->GetDuration(&rtTotal);
        while ((pEvent->WaitForCompletion(1000, &evCode)) == E_ABORT)
        {
            pSeek->GetCurrentPosition(&rtNow);
            printf("%d%%\n", (rtNow * 100)/rtTotal);
        }
        pSeek->Release();
    }
    else  // Cannot update the progress.
    {
        pEvent->WaitForCompletion(INFINITE, &evCode);
    }
    pControl->Stop();
    printf("All done\n");   

    pSrc->Release();
    pMux->Release();
    pVComp->Release();
    pControl->Release();
    pEvent->Release();
    pBuild->Release();
    pGraph->Release();
    CoUninitialize();
	return 0;
}


