//
// Histogram Sample
// This sample shows how to use the Sample Grabber filter for video image processing.

// Conceptual background:
// A histogram is just a frequency count of every pixel value in the image.
// There are various well-known mathematical operations that you can perform on an image
// using histograms, to enhance the image, etc. 

// Histogram stretch (aka automatic gain control):
// Stretches the image histogram to fill the entire range of values. This is a "point operation," 
// meaning each pixel is scaled to a new value, without examining the neighboring pixels. The
// histogram stretch does not actually require you to calculate the full histogram. The scaling factor
// is calculated from the minimum and maximum values in the image.

// Histogram equalization: 
// This point operation smooths the histogram curve to bring out details. This is a good way to
// process some images that have a narrow brightness range, eg very dark images. From ad hoc testing 
// on my own video files, however, the histogram stretch usually gives better results and is simpler.

// Logarithmic stretch:
// This point operation applies a logarithm operator to each pixel value, which has the effect
// of boosting the contrast in the darker regions of the image. 

// References: Handbook of Image and Video Processing (Al Bovik, ed. Academic Press)
// There are also a lot of good web sites about image processing.

// Implementation notes:
// This is a console app. It creates a filter graph like this:
//          src -> sample grabber -> avi mux -> file writer
// Video processing happens inside the sample grabber callback.
//
// More notes:
//
// - I encoded the video to a type-2 DV file.
//
// - For simplicitly, I only handle UYVY. This may cause a color conversion filter to be added
//   to the graph. The histogram operations are based on YUV luma values. 
//
// - The sample grabber is an in-place transform. This saves some memory, which is useful in
//   a file-processing scenario, especially if you are applying a series of several transforms
//   to the image. However, if you connect an in-place transform directly to the video renderer, 
//   it is very likely to kill your perf. (You have been warned!) Also, you are writing over the
//   original image, so you can't use this for non-point operations.
//
//   Basically, the sample grabber is intended for getting the bits into your app, manipulating
//   them, and possibly writing the results to a file.

// See also:
// Microsoft Research Vision SDK (http://msdn.microsoft.com/library/en-us/dnvissdk/html/vissdk.asp)
// Provides a set of template-based C++ classes for manipulating RGB and YUV images. 


/////////////////////////

// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Histogram.h"

#include "dsutil.h"


// GrabberCB: Class to implement the callback function for the sample grabber
// Note: No ref-counting, so keep the object in scope while you're using it!
class GrabberCB : public ISampleGrabberCB 
{
private:
    BITMAPINFOHEADER m_bmi;  // Holds the bitmap format 
    bool m_fFirstSample;     // true if the next sample is the first one.


public:

    GrabberCB();
    ~GrabberCB();

    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 2; }
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

    // ISampleGrabberCB methods
    STDMETHOD(SampleCB)(double SampleTime, IMediaSample *pSample);
    STDMETHODIMP BufferCB(double, BYTE *, long) { return E_NOTIMPL; }

};

HRESULT RunFile(LPTSTR pszSrcFile, LPTSTR pszDestFile);

GrabberCB grabberCallback;  // Make this global so that it stays in scope while the graph exists

// There are three image op types - uncomment the desired one.
// CContrastStretch g_stretch;  
   CEqualize g_stretch;
// CLogStretch g_stretch;

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr;

    hr = CoInitialize(0);
    if (FAILED(hr))
    {
        printf("Could not initialize the COM library.\n");
        return -1;
    }

    // For now use a hard-coded file name

    hr = RunFile("C:\\Sunset.avi", "C:\\Histogram_Output.avi");
    if (FAILED(hr))
    {
        printf("Failed to convert file.\n");
    }

    CoUninitialize();
	return 0;
}


// RunFile:
// Load a video file, connect it to the sample grabber, process the video, 
// and write the results to an AVI file.

HRESULT RunFile(LPTSTR pszSrcFile, LPTSTR pszDestFile)
{
    HRESULT hr;
    
    USES_CONVERSION;  // For TCHAR -> WCHAR conversion macros

    CComPtr<IGraphBuilder> pGraph;  // Filter Graph Manager
    CComPtr<IBaseFilter> pGrabF;    // Sample grabber
    CComPtr<IBaseFilter> pMux;      // AVI Mux
    CComPtr<IBaseFilter> pSrc;      // Source filter
    CComPtr<IBaseFilter> pDVEnc;    // DV Encoder

    CComPtr<ICaptureGraphBuilder2> pBuild;

    // Create the filter graph manager.
    hr = pGraph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr))
    {
        printf("Could not create the Filter Graph Manager (hr = 0x%X.)\n", hr);
        return hr;
    }

    // Create the capture graph builder.
    hr = pBuild.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (FAILED(hr))
    {
        printf("Could not create the Capture Graph Builder (hr = 0x%X.)\n", hr);
        return hr;
    }
    pBuild->SetFiltergraph(pGraph);

    // Build the file-writing portion of the graph.
    hr = pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, T2W(pszDestFile), &pMux, NULL);
    if (FAILED(hr))
    {
        printf("Could not hook up the AVI Mux / File Writer (hr = 0x%X.)\n", hr);
        return hr;
    }

    // Add the source filter for the input file
    hr = pGraph->AddSourceFilter(T2W(pszSrcFile), L"Source", &pSrc);
    if (FAILED(hr))
    {
        printf("Could not add the source filter (hr = 0x%X.)\n", hr);
        return hr;
    }


    // Create some filters and add them to the graph

    // DV Video Encoder
    hr = AddFilterByCLSID(pGraph, CLSID_DVVideoEnc, L"DV Encoder", &pDVEnc);
    if (FAILED(hr))
    {
        printf("Could not add the DV video encoder filter (hr = 0x%X.)\n", hr);
        return hr;
    }
    

    // Sample Grabber
    hr = AddFilterByCLSID(pGraph, CLSID_SampleGrabber, L"Grabber", &pGrabF);
    if (FAILED(hr))
    {
        printf("Could not add the sample grabber filter (hr = 0x%X.)\n", hr);
        return hr;
    }
    CComQIPtr<ISampleGrabber> pGrabber(pGrabF);
    if (!pGrabF)
    {
        return E_NOINTERFACE;
    }

    // Configure the sample grabber
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_UYVY;
	mt.formattype = FORMAT_VideoInfo; 

    // Note: I don't expect the next few methods to fail ....

    hr = pGrabber->SetMediaType(&mt);  // Set the media type we want for the connection.
    _ASSERTE(SUCCEEDED(hr));

    hr = pGrabber->SetOneShot(FALSE);  // Disable "one-shot" mode
    _ASSERTE(SUCCEEDED(hr));

    hr = pGrabber->SetBufferSamples(FALSE); // Disable sample buffering
    _ASSERTE(SUCCEEDED(hr));

    hr = pGrabber->SetCallback(&grabberCallback, 0); // Set our callback. '0' means 'use the SampleCB callback'
    _ASSERTE(SUCCEEDED(hr));


    // Build the graph. First connect the source to the DV Encoder, through the sample grabber.
    // This should connect the video stream.
    hr = pBuild->RenderStream(0, 0, pSrc, pGrabF, pDVEnc);
    if (SUCCEEDED(hr))
    {
        // Next, connect the DV Encoder to the AVI Mux
        hr = pBuild->RenderStream(0, 0, pDVEnc, 0, pMux);

        if (SUCCEEDED(hr))
        {
            // Maybe we have an audio stream. If so, connect it the AVI Mux.
            // But don't fail if we don't...
            HRESULT hrTmp = pBuild->RenderStream(0, 0, pSrc, 0, pMux);

            SaveGraphFile(pGraph, L"C:\\Grabber.grf");
        }
    }

    if (FAILED(hr))
    {
       printf("Error building the graph (hr = 0x%X.)\n", hr);
       return hr;
    }


    // Find out the exact video format.
    hr = pGrabber->GetConnectedMediaType(&mt);
    if (FAILED(hr)) 
    {
        printf("Could not get the video format. (hr = 0x%X.)\n", hr);
        return hr;  
    }
    
    VIDEOINFOHEADER *pVih;
    if ((mt.subtype == MEDIASUBTYPE_UYVY) && (mt.formattype == FORMAT_VideoInfo))
    {
        pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
    }
    else 
    {
        // This is not the format we expected!
   		CoTaskMemFree(mt.pbFormat);
        return VFW_E_INVALIDMEDIATYPE;
    }

    g_stretch.SetFormat(*pVih);
    CoTaskMemFree(mt.pbFormat);

    // Turn off the graph clock
    CComQIPtr<IMediaFilter> pMF(pGraph);
    pMF->SetSyncSource(NULL);

    // Run the graph to completion.
    CComQIPtr<IMediaControl> pControl(pGraph);
    CComQIPtr<IMediaEvent> pEvent(pGraph);
    long evCode = 0;

    printf("Processing the video file... ");


    pControl->Run();
    pEvent->WaitForCompletion(INFINITE, &evCode);
    pControl->Stop();

    printf("Done.\n");

    return hr;
}


GrabberCB::GrabberCB() : m_fFirstSample(true)
{
}

GrabberCB::~GrabberCB()
{
}

// Support querying for ISampleGrabberCB interface
HRESULT GrabberCB::QueryInterface(REFIID iid, void**ppv)
{
    if (!ppv) { return E_POINTER; }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_ISampleGrabberCB)
    {
        *ppv = static_cast<ISampleGrabberCB*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }
    AddRef();  // We don't actually ref count, but in case we change the implementation later.
    return S_OK;
}

// SampleCB: This is where we process each sample
HRESULT GrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{

    HRESULT hr;

    // Get the pointer to the buffer
    BYTE *pBuffer;
    hr = pSample->GetPointer(&pBuffer);

    // Tell the image processing class about it
    g_stretch.SetImage(pBuffer);

    if (FAILED(hr))
    {
        OutputDebugString(TEXT("SampleCB: GetPointer FAILED\n"));
        return hr;
    }

    // Scan the image on the first sample. Re-scan is there is a discontinuity.
    // (This will produce horrible results if there are big scene changes in the
    // video that are not associated with discontinuities. Might be safer to re-scan
    // each image, at a higher perf cost.)

    if (m_fFirstSample)
    {
        hr = g_stretch.ScanImage();
        m_fFirstSample = false;
    }
    else if (S_OK == pSample->IsDiscontinuity())
    {
        hr = g_stretch.ScanImage();
    }

    // Convert the image
    hr = g_stretch.ConvertImage();


    return S_OK;
}


