// Recompress.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>
#include <stdio.h>
#include "initguid.h"

DEFINE_GUID (CLSID_GSM,
			 0x33d9a761, 0x90c8, 0x11D0,0xBD, 0x43, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }
    WCHAR wsz[128];
    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}
int main(int argc, char* argv[])
{
    ICaptureGraphBuilder2   *pCaptureGraphBuilder = NULL;
    IGraphBuilder           *pGraphBuilder = NULL;
    IBaseFilter             *pSource = NULL;       
    IBaseFilter             *pMux = NULL;       
    IBaseFilter             *pVideoCompressor = NULL;  
    IBaseFilter             *pAudioCompressor = NULL;

    IAMStreamConfig         *pAMStreamConfig = NULL;
    IAMVideoCompression     *pAMVideoCompression = NULL;

	IMediaControl           *pControl = NULL;
    IMediaSeeking           *pSeek = NULL;
    IMediaEvent             *pEvent = NULL;

	HRESULT hr;

	DWORD pdwRegister=0;
    CoInitialize(NULL);

    // Create the capture graph builder.
    CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
            IID_ICaptureGraphBuilder2, (void **)&pCaptureGraphBuilder);

    // Make the rendering section of the graph.
    pCaptureGraphBuilder->SetOutputFileName(
            &MEDIASUBTYPE_Avi,  // File type. 
            L"C:\\STDIUE1.avi",  // File name.
            &pMux,              // pointer to the multiplexer.
            NULL);              // pointer to the file writer. 
    
    // Load the source file.
    pCaptureGraphBuilder->GetFiltergraph(&pGraphBuilder);
    pGraphBuilder->AddSourceFilter(L"C:\\Program Files\\Microsoft Money\\Media\\STDIUE1.avi", L"Source Filter", &pSource);

    // Add the compressor filter.
    CoCreateInstance(CLSID_AVICo, NULL, CLSCTX_INPROC,  
            IID_IBaseFilter, (void **)&pVideoCompressor); 
    pGraphBuilder->AddFilter(pVideoCompressor, L"Video Compressor");

    // Render the video stream, through the compressor.
    pCaptureGraphBuilder->RenderStream(
            NULL,       // Output pin category
            NULL,       // Media type
            pSource,       // Source filter
            pVideoCompressor,     // Compressor filter
            pMux);      // Sink filter (the AVI Mux)

   /* CoCreateInstance(CLSID_GSM, NULL, CLSCTX_INPROC,  
            IID_IBaseFilter, (void **)&pAudioCompressor); 
    pGraphBuilder->AddFilter(pAudioCompressor, L"Audio Compressor");*/
	
	// Render the audio stream.
    pCaptureGraphBuilder->RenderStream(
		     NULL,
			 NULL, 
			 pSource, 
			 pAudioCompressor, 
			 pMux);
 
    // Compress at 100k/second data rate.
    AM_MEDIA_TYPE *pmt;
    pCaptureGraphBuilder->FindInterface(NULL, NULL, pVideoCompressor, IID_IAMStreamConfig, (void **)&pAMStreamConfig);                           
    
	pAMStreamConfig->GetFormat(&pmt);
	 
      if (pmt->formattype == FORMAT_VideoInfo) 
    {
		
        ((VIDEOINFOHEADER *)(pmt->pbFormat))->dwBitRate = 100000;
		
		pAMStreamConfig->SetFormat(pmt);
	  }  
 

    // Request key frames every four frames.
    pAMStreamConfig->QueryInterface(IID_IAMVideoCompression, (void **)&pAMVideoCompression);
    pAMVideoCompression->put_KeyFrameRate(4);
    pAMVideoCompression->Release();
    pAMStreamConfig->Release();

    // Run the graph.
    
    pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&pControl);
    pGraphBuilder->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
   
     hr = pMux->QueryInterface(IID_IMediaSeeking, (void**)&pSeek);
	
   
    pControl->Run();
    printf("Recompressing... \n");

    long evCode;
    if (SUCCEEDED(hr))  
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

    pSource->Release();
    pMux->Release();
    pVideoCompressor->Release();
	pAudioCompressor->Release ();
    pControl->Release();
    pEvent->Release();
    pCaptureGraphBuilder->Release();
    pGraphBuilder->Release();
    CoUninitialize();

	return 0;
}

 
 
