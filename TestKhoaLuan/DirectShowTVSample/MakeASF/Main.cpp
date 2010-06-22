
#include <windows.h>
#include <dshow.h>
#include "resource.h"
#include <streams.h>
#include <strmif.h>
#include <dshowasf.h>
#include <dvdmedia.h>
#include <dv.h>
#include <atlbase.h>
#include <wmsdk.h>
#include <objbase.h>

#include "macros.h"



HINSTANCE hInst = 0;
HWND g_hwnd = 0;
BOOL g_fFrameIndexing = FALSE;
BOOL g_fMultipassEncode = FALSE;
BOOL g_fSMPTETimecodes = FALSE;
REFERENCE_TIME g_rtAvgTimePerFrame = 333333; //default to 30 fps

//Set this flag to enable additional debug status messages
BOOL g_fVerbose = TRUE;

const int g_nOutputStringBufferSize = 1024 * sizeof (_TCHAR);
int g_OutputStringLen = 0;
_TCHAR szSource[_MAX_PATH]= {'\0'};
_TCHAR szTarget[_MAX_PATH] = {'\0'};
_TCHAR szProfile[_MAX_PATH] = {'\0'};
_TCHAR szTitle[_MAX_PATH] = {'\0'};
_TCHAR szAuthor[_MAX_PATH] = {'\0'};
_TCHAR szOutputWindow[g_nOutputStringBufferSize] = {'\0'};


//
// Macros
//
#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

// Unique string name for CreateEvent() (simply a GUID)
#define WMVCOPY_INDEX_EVENT   TEXT("{78268A45-34B2-489a-838B-38833C949CBF}")

// Types used when using the Open File dialog with either 
// media file types (.avi, .wav, etc)or profile file names (.prx)
typedef enum {
    mediaFile,
    prxFile
} fileType;

typedef enum
{
    AUDIO,
    VIDEO,
    UNKNOWN
} majorType;

class CASFCallback;

// General filter graph creation functions
HRESULT CreateFilterGraph(IGraphBuilder **pGraph);
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter);
HRESULT RenderOutputPins(IBaseFilter *pFilter, IGraphBuilder* pGB);
HRESULT SetNoClock(IFilterGraph *pGraph);
LONG WaitForCompletion(HWND ghwnd, IGraphBuilder *pGraph );
HRESULT GetPinMajorType(IPin* ppPin, majorType& mType);
HRESULT GetPinByMajorType(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID majortype, IPin** ppPin);
REFERENCE_TIME GetTimePerFrameFromPin(IPin *pPin);

// ASF-specific functions
HRESULT MakeAsfFile(_TCHAR* szSource, _TCHAR* szTarget, _TCHAR* szProfile);
HRESULT LoadCustomProfile( LPCTSTR ptszProfileFileName, IWMProfile ** ppIWMProfile, BOOL& bEmptyVidRect );
HRESULT VerifyInputsConnected(IBaseFilter* pFilter);
HRESULT IndexFileByFrames(WCHAR *wszTargetFile, CASFCallback* pCallback);
HRESULT SetNativeVideoSize(IBaseFilter* pASFWriter, IWMProfile* pProfile, BOOL bUseDummyValues);
SIZE GetVideoSizeFromPin(IBaseFilter *pFilter);
HRESULT CreateProfileManager(IWMProfileManager** pPM);
HRESULT WriteProfileAsPRX( IWMProfileManager* pPM, IWMProfile* pProfile);
HRESULT AddSmpteDataUnitExtension(IWMProfile *pProfile);
HRESULT AddMetadata(IBaseFilter* pFilter, WCHAR* pwszAuthor, WCHAR* pwszTitle);


//////////////////////////////////////////////////////////////////////////////////
// This class implements the methods of the IWMStatusCallback and IAMWMBufferPassCallback interfaces
// IWMStatusCallback is used for frame indexing, and IWMAMBufferPassCallback is used to add
// data unit extensions, in this case SMPTE time codes.
//////////////////////////////////////////////////////////////////////////////////

class CASFCallback : public IWMStatusCallback, public IAMWMBufferPassCallback
{
public:
    // We create the object with a ref count of zero because we know that no one 
    // will try to use it before we call QI.
    CASFCallback(): m_refCount(0)
    {
        phr = NULL ;
        hEvent = NULL;
            
    }

    ~CASFCallback()
    {
        DbgLog((LOG_TRACE, 3, _T("Deleting CASFCallback!  refCount=0x%x\n"), m_refCount));
    }

    // IAMWMBufferPassCallback
    // This method is called by the WM ASF Writer's video input pin after it receives each sample
    // but before the sample is encoded. In this example we only set the timecode property. 
    // You can extend this to set or get any number of properties on the sample.
    virtual HRESULT STDMETHODCALLTYPE Notify(INSSBuffer3*  pNSSBuffer3,
                                             IPin*  pPin,
                                             REFERENCE_TIME*  prtStart,
                                             REFERENCE_TIME*  prtEnd
                                             )
    {

        WMT_TIMECODE_EXTENSION_DATA SMPTEExtData;
        ZeroMemory( &SMPTEExtData, sizeof( SMPTEExtData ) );

        // wRange is already zero, but we set it explicitly here to show that
        // in this example, we just have one range for the entire file
        SMPTEExtData.wRange = 0;

        DWORD carryOver = 0;
        DWORD dwStartTimeInSeconds = (DWORD)(*prtStart / 10000000 );  // convert REFERENCE_TIME to seconds for convenience
        DWORD dwSeconds = dwStartTimeInSeconds % 60;
        DWORD dwHours = dwStartTimeInSeconds / 3600;
        DWORD dwMinutes = dwStartTimeInSeconds / 60;
        //if we are at > 60 minutes then we do one additional calculation
        // so that minutes correctly starts back at zero after 59
        if (dwHours)
        {
            dwMinutes = dwMinutes % 60;
        }
        
             
        //SMPTE frames are 0-based. Also, our sample is hard-coded for 30 fps. 
        DWORD dwFrames = (DWORD) ((*prtStart % 10000000) / g_rtAvgTimePerFrame); 

        //
        // The timecode values are stored in the ANSI SMPTE format.
        //
        // BYTE  MSB               LSB
        // ----------------------------------------------
        // 1     Tens of hour      Hour
        // 2     Tens of minute  Minute
        // 3     Tens of second  Second
        // 4     Tens of frame    Frame
        //
        // For example, 01:19:30:01 would be represented as 0x01193001.

        SMPTEExtData.dwTimecode = ( ( dwFrames / 10 ) << 4 ) | ( dwFrames % 10 );
        SMPTEExtData.dwTimecode |= ( ( dwSeconds / 10 ) << 12 ) | ( ( dwSeconds % 10 ) << 8 );
        SMPTEExtData.dwTimecode |= ( ( dwMinutes / 10 ) << 20 ) | ( ( dwMinutes % 10 ) << 16 );
        SMPTEExtData.dwTimecode |= ( ( dwHours / 10 ) << 28 ) | ( ( dwHours % 10 ) << 24 );

        
        HRESULT hr = pNSSBuffer3->SetProperty(WM_SampleExtensionGUID_Timecode, (void*) &SMPTEExtData, WM_SampleExtension_Timecode_Size);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to SetProperty!  hr=0x%x\n"), hr));
        }

        // The calling pin actually ignores the HRESULT
        return hr;
    }

    // IWMStatusCallback
    virtual HRESULT STDMETHODCALLTYPE OnStatus(
                          /* [in] */ WMT_STATUS Status,
                          /* [in] */ HRESULT hr,
                          /* [in] */ WMT_ATTR_DATATYPE dwType,
                          /* [in] */ BYTE __RPC_FAR *pValue,
                          /* [in] */ void __RPC_FAR *pvContext)
    {
        switch ( Status )
        {
            case WMT_INDEX_PROGRESS:
                // Display the indexing progress as a percentage.
                // Use "carriage return" (\r) to reuse the status line.
                DbgLog((LOG_TRACE, 3, _T("Indexing in progress (%d%%)\r"), *pValue));
                break ;

            case WMT_CLOSED:
                *phr = hr;
                SetEvent(hEvent) ;
                DbgLog((LOG_TRACE, 3, _T("\n")));   // Move to new line (past progress line)
                break;

            case WMT_ERROR:
                *phr = hr;
                SetEvent(hEvent) ;
                DbgLog((LOG_TRACE, 3, _T("\nError during indexing operation! hr=0x%x\n"), hr));
                break;

            // Ignore these messages
            case WMT_OPENED:
            case WMT_STARTED:
            case WMT_STOPPED:
                break;
        }
        return S_OK;
    }

    //------------------------------------------------------------------------------
    // Implementation of IUnknown methods
    //------------------------------------------------------------------------------
    ULONG STDMETHODCALLTYPE AddRef( void )
    {
        
        m_refCount++;
        DbgLog((LOG_TRACE, 3, _T("CASFCallback::AddRef!  refCount=0x%x\n"), m_refCount));
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release( void )
    {
        m_refCount--;
        DbgLog((LOG_TRACE, 3, _T("CASFCallback::Release!  refCount=0x%x\n"), m_refCount));
        if(m_refCount == 0)
            delete this;
        return 1;
    }


    HRESULT STDMETHODCALLTYPE QueryInterface(
                        /* [in] */ REFIID riid,
                        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if ( riid == IID_IWMStatusCallback )
        {
            *ppvObject = ( IWMStatusCallback * ) this;
            
            
        }
        else if (riid == IID_IAMWMBufferPassCallback)
        {
            *ppvObject = ( IAMWMBufferPassCallback * ) this;
           
        }
        
        else
        {
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

public:
    HANDLE    hEvent ;
    HRESULT   *phr ;
    int       m_refCount;

};

//////////////////////////////////////////////////////////////////////////////////
// End callback class
//////////////////////////////////////////////////////////////////////////////////


// Status messages in user interface
void OutputMsg(_TCHAR* msg);


//------------------------------------------------------------------------------
// Name: MakeAsfFile()
// Desc: Play the file through a DirectShow filter graph that terminates in the WM ASF Writer.
//       We build a new filter graph manager each time inside this function. All other 
//       ASF-specific functions are called from inside MakeAsfFile. This is a long function and 
//       can be broken down into these basic steps:
//		 1) Create a Filter Graph Manager and add the WM ASF Writer, specifying a target file name.
//       2) Give the WM ASF Writer a custom profile loaded from a prx file. Alternatively, an app could
//          create a custom Windows Media 9 Series profile dynamically using the Windows Media 
//          Profile Manager. The filter must be configured with a profile
//          before we can build the rest of the graph. We need to build the graph
//          in order to determine some details about the input streams. Based on what we learn, we may need
//          to set a new profile on the WM ASF Writer before we start encoding.
//       3) Set the indexing and multipass encoding modes on the filter.
//       4) Build the complete filter graph, specifying the source file.
//       5) Verify that all inputs are connected and fail gracefully if they are not.
//       6) Determine whether the profile contains zero-sized rectangles, which indicates 
//          that it was probably created using the WM Profile Editor and the user selected the 
//          "Use Native Video Size" checkbox. If this is the case, determine the size of the
//           incoming video stream and adjust the profile to specify that rectangle size.
//       7) Set up our callback to insert Data Unit Extensions -- we have chosen to add SMPTE time codes
//          but the same technique is used to add pixel aspect ratio information, to force key frames
//          in unconstrained VBR streams, or to add custom data unit extensions that you define.
//       8) Run the graph (finally!) and wait for it to reach the end of the source file.
//       9) If performing multipass encoding, seek back to the beginning and run the graph again.
//      10) If requested, perform frame indexing on the file using the WM Format SDK directly.
//
//------------------------------------------------------------------------------
HRESULT MakeAsfFile(_TCHAR* szSource, _TCHAR* szTarget, _TCHAR* szProfile)
{
    HRESULT hr;    
    WCHAR wszTargetFile[_MAX_PATH] = {'\0'};
    WCHAR wszSourceFile[_MAX_PATH] = {'\0'};
    WCHAR wszAuthor[_MAX_PATH] = {'\0'};
    WCHAR wszTitle[_MAX_PATH] = {'\0'};
    
    CComPtr <IGraphBuilder>    pGraph;
    CComPtr <IBaseFilter>      pSourceFilter;
    CComPtr <IBaseFilter>      pASFWriter;

    CASFCallback*                     pASFCallback = NULL;
    CComPtr <IAMWMBufferPassCallback> pSMPTECallback;
    



    // Convert target filename, title and author metadata fields to a wide character string
#ifndef _UNICODE
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR) szTarget, -1, 
                        wszTargetFile, NUMELMS(wszTargetFile));
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR) szAuthor, -1, 
                        wszAuthor, NUMELMS(wszAuthor));
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) szTitle, -1, 
                        wszTitle, NUMELMS(wszTitle));
#else
    wcsncpy(wszTargetFile, szTarget, NUMELMS(wszTargetFile));
    wcsncpy(wszAuthor, szAuthor, NUMELMS(wszAuthor));
    wcsncpy(wszTitle, szTitle, NUMELMS(wszTitle));
#endif

    // Load the prx file into memory and tell us whether it is a special case profile with a zero-sized
    // video rectangle. If it is, we'll have to adjust the profile later,to specify the native video size
    // before running the graph. This is how the Windows Media Encoder works with profiles created
    // by the Profile Editor.


    CComPtr<IWMProfile> pProfile1;
    BOOL fZeroSizedVidRect = FALSE;

    // Load the data in the prx file into a WMProfile object
    hr = LoadCustomProfile((LPCSTR)szProfile, &pProfile1, /*out*/fZeroSizedVidRect);
    if(FAILED(hr)) 
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to load profile!  hr=0x%x\n"), hr));
        return hr;
    }

    if(fZeroSizedVidRect)
    {
		OutputMsg(_T("The profile has a zero-sized rectangle. This will be interpreted as an instruction to use the native video size.\r\n"));
        DbgLog((LOG_TRACE, 3, _T("Zero-sized rectangle!\n")));

        // Now we need to insert some dummy values for the output rectangle in order to avoid an
        // unhandled exception when we first configure the filter with this profile.
        // Later, after we connect the filter, we will be able to determine the upstream rectangle size, and
        // then adjust profile's rectangle values to match it.

        hr = SetNativeVideoSize(pASFWriter, pProfile1, TRUE);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to SetNativeVideoSize with dummy values!  hr=0x%x\n"), hr));
            return hr;
        }
    }

    // Create an empty DirectShow filter graph
    hr = CreateFilterGraph(&pGraph);
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Couldn't create filter graph! hr=0x%x"), hr));
        return hr;
    }

    // Add the ASF Writer filter to the graph
    hr = CreateFilter(CLSID_WMAsfWriter, &pASFWriter);
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to create WMAsfWriter filter!  hr=0x%x\n"), hr));
        return hr;
    }

    // Get a file sink filter interface from the ASF Writer filter
    // and set the output file name
    CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pFileSink (pASFWriter);
    if(!pFileSink)
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to create QI IFileSinkFilter!  hr=0x%x\n"), hr));
        return hr;
    }

    hr = pFileSink->SetFileName(wszTargetFile, NULL);
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to set target filename!  hr=0x%x\n"), hr));
        return hr;
    }

    // Add the WM ASF writer to the graph
    hr = pGraph->AddFilter(pASFWriter, L"ASF Writer");
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to add ASF Writer filter to graph!  hr=0x%x\n"), hr));
        return hr;
    }
    
 	// Obtain the interface we will use to configure the WM ASF Writer
    CComQIPtr<IConfigAsfWriter2,   &IID_IConfigAsfWriter2>   pConfigAsfWriter2(pASFWriter);
    if(!pConfigAsfWriter2)
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to QI for IConfigAsfWriter2!  hr=0x%x\n"), hr));
        return hr;
    }

    // Configure the filter with the profile
	hr = pConfigAsfWriter2->ConfigureFilterUsingProfile(pProfile1);
    if(FAILED(hr)) 
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to configure filter to use profile1!  hr=0x%08x\n"), hr));
        return hr;
    }
 
    // If frame-based indexing was requested, disable the default
    // time-based (temporal) indexing
    if (g_fFrameIndexing)
    {
        hr = pConfigAsfWriter2->SetIndexMode(FALSE);
        if(FAILED(hr)) 
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to disable time-based indexing!  hr=0x%08x\n"), hr));
            return hr;
        }
		OutputMsg(_T("Frame-based indexing has been requested.\r\n"));
    }

    // Enable multipass encoding if requested
    if (g_fMultipassEncode)
    {
        hr = pConfigAsfWriter2->SetParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS, TRUE, 0);
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to enable multipass encoding param!  hr=0x%x\n"), hr));
            return hr;
        }
    }

    // Set sync source to NULL to encode as fast as possible
    SetNoClock(pGraph);
 
	// Convert the source file into WCHARs for DirectShow
#ifndef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szSource, -1, wszSourceFile, NUMELMS(wszSourceFile));
#else
        wcsncpy(wszSourceFile, szSource, NUMELMS(wszSourceFile));
#endif

        DbgLog((LOG_TRACE, 3, _T("\nCopying [%ls] to [%ls]\n"), wszSourceFile, wszTargetFile));


    // Let DirectShow render the source file. We use "AddSourceFilter" and then
    // render its output pins using IFilterGraph2::RenderEx in order to force the
    // Filter Graph Manager to always use the WM ASF Writer as the renderer. If the 
    // 

    hr = pGraph->AddSourceFilter(wszSourceFile, L"Source Filter", &pSourceFilter);
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to add source filter!  hr=0x%x\n"), hr));
        return hr;
    }

    //Render all output pins on source filter using RenderEx
    hr = RenderOutputPins(pSourceFilter, pGraph);
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed RenderOutputPins!  hr=0x%x\n"), hr));
        return hr;
    }

    
     // Verify that all of our input pins are connected. If the profile specifies more streams
    // than are actually contained in the source file, the filter will not run. So here we check
    // for the condition ourselves and fail gracefully if necessary. 

    hr = VerifyInputsConnected(pASFWriter);
    if(FAILED(hr))
    {
        OutputMsg(_T("Cannot encode this file because not all input pins were connected. The profile specifies more input streams than the file contains. Aborting copy operation. \r\n"));
        DbgLog((LOG_TRACE, 3, _T("Not all inputs connected!  hr=0x%x\n"), hr));
        return hr;
    }

    // To support profiles that were created in the Windows Media Profile Editor, we need to
    // handle the case where the user selected the "Video Size Same as Input" option. This causes 
    // the video rectangle in the profile to be set to zero. The WM Encoder understands the "zero" value
    // and obtains the source video size before encoding. If we don't do the same thing, we will create a
    // valid ASF file but it will have no video frames. We have waited until now to check the input rectangle
    // size because it is most efficient to do this after the the filter graph has been built. 

    if(fZeroSizedVidRect)
    {
        hr = SetNativeVideoSize(pASFWriter, pProfile1, FALSE);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to SetNativeVideoSize!  hr=0x%x\n"), hr));
            return hr;
        }

        hr = pConfigAsfWriter2->ConfigureFilterUsingProfile(pProfile1);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed ConfigureFilterUsingProfile-2-!  hr=0x%x\n"), hr));
            return hr;
        }  
        
        OutputMsg(_T("Filter was successfully reconfigured for native video size.\r\n"));
    }
    
    // When adding Data Unit Extensions, the order of operations is very important

    if (TRUE == g_fSMPTETimecodes)
    {
        
        // (1) Set the DUE on the profile stream
        hr = AddSmpteDataUnitExtension(pProfile1);
        if(FAILED(hr)) 
        {
            DbgLog((LOG_TRACE, 3, _T("Failed AddSmpteDataUnitExtension!  hr=0x%x\n"), hr));
            return hr;
        }
    

        // (2) Update the filter with the new profile
        hr = pConfigAsfWriter2->ConfigureFilterUsingProfile(pProfile1);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed ConfigureFilterUsingProfile-3-!  hr=0x%x\n"), hr));
            return hr;
        } 
       
        // (3) Find the video pin and register our callback.
        // Note here we use the same object to handle DUE callbacks and index callbacks. So we create the object
        // on the heap , which is how COM objects should be created anyway.
        
        pASFCallback = new CASFCallback();
        if(!pASFCallback)
        {
            return E_OUTOFMEMORY;
        }
        
        hr = pASFCallback->QueryInterface( IID_IAMWMBufferPassCallback , (void**) &pSMPTECallback);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to QI for IAMWMBufferPassCallback!  hr=0x%x\n"), hr));
            return hr;
        }
        
        //find the video pin
        CComPtr<IPin> pVideoPin;
        hr = GetPinByMajorType(pASFWriter, PINDIR_INPUT, MEDIATYPE_Video, &pVideoPin);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to GetPinByMajorType(pVideoPin)!  hr=0x%x\n"), hr));
            return hr;
        }

        g_rtAvgTimePerFrame = GetTimePerFrameFromPin(pVideoPin);
        if(g_rtAvgTimePerFrame == 0)
        {
            DbgLog((LOG_TRACE, 3, _T("g_rtAvgTimePerFrame == 0\n")));
            return hr;
        }


        //Get its IAMWMBufferPass interface
        CComQIPtr<IAMWMBufferPass, &IID_IAMWMBufferPass> pBufferPass( pVideoPin ) ;

        //Give it the pointer to our object
        hr = pBufferPass->SetNotify( (IAMWMBufferPassCallback*) pSMPTECallback) ;
        if(FAILED(hr)) 
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to set callback!  hr=0x%x\n"), hr)) ;
            return hr;
        }
        
    }
     

    //Now that we have set the final profile, we can safely add the metadata
    hr = AddMetadata(pASFWriter, wszAuthor, wszTitle);
    if(FAILED(hr)) 
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to set AddMetadata!  hr=0x%x\n"), hr));
            return hr;
        }

    // Now we are ready to run the filter graph and start encoding. First we need the IMediaControl interface.
    CComQIPtr<IMediaControl, &IID_IMediaControl> pMC(pGraph);

    if(!pMC)
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to QI for IMediaControl!  hr=0x%x\n"), hr));
        return hr;
    }

    hr = pMC->Run();
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("Failed to run the graph!  hr=0x%x\nCopy aborted.\n\n"), hr));
        DbgLog((LOG_TRACE, 3, _T("Please check that you have selected the correct profile for copying.\n")
                 _T("Note that if your source ASF file is audio-only, then selecting a\n")
                 _T("video profile will cause a failure when running the graph.\n\n")));
        return hr;
    }
 
    // Wait for the event signalling that we have reached the end of the input file. We listen for the
    // EC_COMPLETE event here rather than in the app's message loop in order to keep the order of operations
    // as straightforward as possible. The downside is that we cannot stop or pause the graph once it starts. 

    int nEvent = WaitForCompletion(g_hwnd, pGraph);

    // Stop the graph. If we are doing one-pass encoding, then we are done. If doing two-pass, then we 
    // still have work to do.
    hr = pMC->Stop();
    if (FAILED(hr))
        DbgLog((LOG_TRACE, 3, _T("Failed to stop filter graph!  hr=0x%x\n"), hr));
    
    //We should never really encounter these two conditions together.
    if (g_fMultipassEncode && (nEvent != EC_PREPROCESS_COMPLETE))
    {
        DbgLog((LOG_TRACE, 3, _T("ERROR: Failed to recieve expected EC_PREPROCESSCOMPLETE.\n")));
        return E_FAIL;        
    }
            
    // If we're using multipass encode, run again
    if (g_fMultipassEncode)
    {
        DbgLog((LOG_TRACE, 3, _T("Preprocessing complete.\n")));

        // Seek to beginning of file
        CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> pMS(pMC);
        if (!pMS)
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to QI for IMediaSeeking!\n")));
            return E_FAIL;
        }
        
        LONGLONG pos=0;
        hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                               NULL, AM_SEEKING_NoPositioning);

        // Run the graph again to perform the actual encoding based on
        // the information gathered by the codec during the first pass.
        hr = pMC->Run();
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to run the graph!  hr=0x%x\n"), hr));
            return hr;
        }
        
        nEvent = WaitForCompletion(g_hwnd, pGraph);           
        hr = pMC->Stop();
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to stop filter graph after completion!  hr=0x%x\n"), hr));
            return hr;
        }

        DbgLog((LOG_TRACE, 3, _T("Copy complete.\n")));
        
        // Turn off multipass encoding
        hr = pConfigAsfWriter2->SetParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS, FALSE, 0);
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to disable multipass encoding!  hr=0x%x\n"), hr));
            return hr;
        }
    } // end if g_fMultipassEncode
 
    // Finally, if frame-based indexing was requested, this must be performed
    // manually after the file is created. Theoretically, frame based indexing
    // can be performed on any type of video data in an ASF file.

    if (g_fFrameIndexing)
    {
        if(!pASFCallback) //We didn't ask for SMPTE so we need to create the callback object here
        {
            DbgLog((LOG_TRACE, 3, _T("Creating a new callback object for Frame Indexing\n")));
            pASFCallback = new CASFCallback();
            if(!pASFCallback)
            {
                return E_OUTOFMEMORY;
            }
        }
        
        hr = IndexFileByFrames(wszTargetFile, pASFCallback);
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("IndexFileByFrames failed! hr=0x%x\n"), hr));
            return hr;
        }
        
		OutputMsg(_T("A frame-based index was added to the file.\r\n"));
    }

   // Note: Our callback object is automatically cleaned up when we exit here because we only addref'd
   // using CComPtrs in our app, which call Release automatically when they go out of scope. The
   // WM ASF Writer filter also AddRef's the object when it gets the SetNotify call, and it correctly
   // releases its pointers when its done with them.

    return hr;
}

//------------------------------------------------------------------------------
// Name: OpenFileDialog()
// Desc: Open the File Open dialog box in order to select a source file, profile file, 
//       and target file name.
//------------------------------------------------------------------------------

BOOL OpenFileDialog(HWND hWnd, int iFileType)
{
    OPENFILENAME ofn;
    _TCHAR        szFileName[_MAX_PATH];
    _TCHAR        szMediaFile[] = "Media Files\0*.avi;*.mpg;*.wav;*.mp3\0\0";
    _TCHAR        szPRXFile[] = "PRX Files\0*.prx";

    szFileName[0] = 0;
  
    _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;
    if (mediaFile == iFileType)
    {
        ofn.lpstrFilter = szMediaFile;
		ofn.lpstrTitle = "Select file to be converted";
    }
    else
    {
        ofn.lpstrFilter = szPRXFile;
		ofn.lpstrTitle = "Select profile";
    }
    ofn.nFilterIndex = 0 ;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) ;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir = "d:\\more movies"; 
    ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST ;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hWnd, iFileType == mediaFile ? IDC_SOURCEFILE : IDC_PROFILE, szFileName);
	return TRUE;
    } else {
	return FALSE;
    }
}



//------------------------------------------------------------------------------
// Name: LoadCustomProfile()
// Desc: Loads a custom profile from a prx file and peeks to see whether it specifies 
//       a zero-sized video rectangle that we'll have to deal with later.
//------------------------------------------------------------------------------
HRESULT LoadCustomProfile( LPCTSTR ptszProfileFileName, 
                                            IWMProfile ** ppIWMProfile, BOOL& bEmptyVidRect )
{
    HRESULT             hr = S_OK;
    DWORD               dwLength = 0;
    DWORD               dwBytesRead = 0;
    IWMProfileManager   * pProfileManager = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    LPWSTR              pwszProfile = NULL;

    if( NULL == ptszProfileFileName || NULL == ptszProfileFileName )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = CreateProfileManager( &pProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Open the profile file
        //
        hFile = CreateFile( ptszProfileFileName, 
                            GENERIC_READ, 
                            FILE_SHARE_READ, 
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL );
        if( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( hFile ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        dwLength = GetFileSize( hFile, NULL );
        if( -1 == dwLength )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Allocate memory to hold profile XML file
        //
        pwszProfile = (WCHAR *)new BYTE[ dwLength + sizeof(WCHAR) ];
        if( NULL == pwszProfile )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // The buffer must be null-terminated.
        ZeroMemory(pwszProfile, dwLength + sizeof(WCHAR) ) ;
        
        //
        // Read the profile to a buffer
        //
        if( !ReadFile( hFile, pwszProfile, dwLength, &dwBytesRead, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Load the profile from the buffer
        //
        hr = pProfileManager->LoadProfileByData( pwszProfile, 
                                                 ppIWMProfile );
        if( FAILED(hr) )
        {
            break;
        }
    }
    while( FALSE );

     // The WM Profile Editor uses empty rectangles as a signal to the Windows Media Encoder
    // to use the native video size. Our application will do the same thing.
    // Here we cheat for the sake of efficiency. In general, do not
    // get into the habit of manipulating the XML profile string directly.
    // Here we are just peeking to see if we have an empty video rectangle. 
    // If we do have one, we won't attempt to modify the XML string directly, but
    // instead will just set a flag now and modify the profile object later using the SDK methods.
        
   
    if(SUCCEEDED(hr))
    {
        if( wcsstr( pwszProfile , L"biwidth=\"0\"" ) != NULL || wcsstr( pwszProfile , L"biheight=\"0\"" ) != NULL )
        {
            bEmptyVidRect = TRUE;
        }
    }

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszProfile );
    SAFE_CLOSEHANDLE( hFile );
    SAFE_RELEASE( pProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: IndexFileByFrames()
// Desc: Index the file.
//------------------------------------------------------------------------------
HRESULT IndexFileByFrames(WCHAR *wszTargetFile, CASFCallback* pCallback)
{
    HRESULT hr;
    CComPtr<IWMIndexer> pIndexer;
    hr = WMCreateIndexer(&pIndexer);
    CASFCallback myCallback2;

    if (SUCCEEDED(hr))
    {
        // Get an IWMIndexer2 interface to configure for frame indexing
        CComQIPtr<IWMIndexer2, &IID_IWMIndexer2> pIndexer2(pIndexer);

        if(!pIndexer2) 
        {
            DbgLog((LOG_TRACE, 3, _T("CopyASF: Failed to QI for IWMIndexer2!  hr=0x%x\n"), hr));
            return hr;
        }

        // Configure for frame-based indexing
        WORD wIndexType = WMT_IT_NEAREST_CLEAN_POINT;

        hr = pIndexer2->Configure(0, WMT_IT_FRAME_NUMBERS, NULL,
                                  &wIndexType);
        if (SUCCEEDED(hr))
        {
            HANDLE hIndexEvent = CreateEvent( NULL, FALSE, FALSE, WMVCOPY_INDEX_EVENT );
            if ( NULL == hIndexEvent )
            {
                DbgLog((LOG_TRACE, 3, _T("Failed to create index event!\n")));
                return E_FAIL;
            }

            
            HRESULT hrIndex = S_OK;

            //CASFCallback 
            pCallback->hEvent = hIndexEvent;
            pCallback->phr = &hrIndex;

            CComPtr<IWMStatusCallback> pIndexCallback;
            pCallback->QueryInterface(IID_IWMStatusCallback, (void**) &pIndexCallback);

            if (g_fVerbose)
                DbgLog((LOG_TRACE, 3, _T("\nStarting the frame indexing process.\n")));

            hr = pIndexer->StartIndexing(wszTargetFile, pIndexCallback, NULL);
            if (SUCCEEDED(hr))
            {
                // Wait for indexing operation to complete
                WaitForSingleObject( hIndexEvent, INFINITE );
                if ( FAILED( hrIndex ) )
                {
                    DbgLog((LOG_TRACE, 3, _T("Indexing Failed (hr=0x%08x)!\n"), hrIndex ));
                    return hr;
                }
                //else
                    DbgLog((LOG_TRACE, 3, _T("Frame indexing completed.\n")));
            }
            else
            {
                DbgLog((LOG_TRACE, 3, _T("StartIndexing failed (hr=0x%08x)!\n"), hr));
                return hr;
            }
        }
        else
        {
            DbgLog((LOG_TRACE, 3, _T("Failed to configure frame indexer! hr=0x%x\n"), hr));
            return hr;
        }
    }

     return hr;
}



// The WM ASF Writer has no output pins, so we can simply enumerate
// all pins without QueryDirection to see if they are connected
HRESULT VerifyInputsConnected(IBaseFilter* pFilter)
{
    HRESULT         hr = S_OK;
    IEnumPins *     pEnumPin = NULL;
    IPin *          pConnectedPin = NULL;
    IPin *          ppPin = NULL;
    ULONG           ulFetched;
    DWORD           nFound = 0;
    majorType       nMediaType = AUDIO;
    int             cNumPins = 0; 

    if (!pFilter)
        return E_POINTER;
        
      // Get a pin enumerator for the filter's pins
    hr = pFilter->EnumPins( &pEnumPin );
    if(SUCCEEDED(hr))
    {
        // Explicitly check for S_OK instead of SUCCEEDED, since IEnumPins::Next()
        // will return S_FALSE if it does not retrieve as many pins as requested.
        while ( S_OK == ( hr = pEnumPin->Next( 1L, &ppPin, &ulFetched ) ) )
        {
            hr = ppPin->ConnectedTo( &pConnectedPin );
            if (pConnectedPin)
            {
                // As long as the pin is connected, we are happy.
                // Just release the connected pin and continue
                SAFE_RELEASE( pConnectedPin) ;
                
            }
            else
            {
                // It might be helpful to know which type of stream
                // you have in the profile that you don't have in the source file
               hr = GetPinMajorType(ppPin, nMediaType);
                if(AUDIO == nMediaType)
                {
                    DbgLog((LOG_TRACE, 3, _T("Unconnected audio pin!\n")));
					hr = E_FAIL;
                }
                else if (VIDEO == nMediaType)
                {
                    DbgLog((LOG_TRACE, 3, _T("Unconnected video pin!\n")));
					hr = E_FAIL;
                }
                else
                {
                    DbgLog((LOG_TRACE, 3, _T("Unconnected pin unknown type!\n")));
					hr = E_FAIL;
                }

				ppPin->Release();
				cNumPins++;
				break;
            }
            
            ppPin->Release();
            cNumPins++;
        }

		DbgLog((LOG_TRACE, 3, _T("WM ASF Writer has %d pins!\n"), cNumPins ));

        // Release enumerator
        pEnumPin->Release();
    }

    return hr;   
}

// We use this as a informational, diagnostic tool when we have an
// unconnected input pin, to see which pin it is.

HRESULT GetPinMajorType(IPin* ppPin, majorType& mType)
{
    HRESULT hr = S_OK;

    // Here for no particular we use the DirectShow IAMStreamConfig interface on the WM ASF Writer
    // to determine the _output_ format, that is the format that will be used to
    // write the file. Use the IPin interface to determine the _input_ format, that is the 
    // format of the pin connection with the upstream filter.
    CComQIPtr<IAMStreamConfig,   &IID_IAMStreamConfig>   pStreamConfig(ppPin);
    AM_MEDIA_TYPE* pMT;
    
    if(pStreamConfig)
    {
        hr = pStreamConfig->GetFormat(&pMT);
        if(FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("Failed GetFormat! (hr=0x%08x)!\n"), hr ));
            mType = UNKNOWN;
            return hr;
        }

        if(MEDIATYPE_Audio == pMT->majortype)
        {
            mType = AUDIO; //audio stream
        }

        else if(MEDIATYPE_Video == pMT->majortype)
        {
            mType = VIDEO; //video stream
        }
        DeleteMediaType(pMT);
    }
    return hr;

}

// In this function we obtain the native video size from our video input pin IAMStreamConfig interface
// and then set that size on the profile. We must then disconnect all our pins and reconfigure the 
// WM ASF Writer with the new profile, then reconnect the pins.
// We set bUseDummyValues to true to provide dummy rectangle dimensions if we know the profile
// contains a zero-sized rectangle.

HRESULT SetNativeVideoSize(IBaseFilter* pASFWriter, IWMProfile* pProfile, BOOL bUseDummyValues)
{
    HRESULT hr = S_OK;
    
    // Get the native rectangle size from the video pin
    SIZE nativeRect;
    
    if( TRUE == bUseDummyValues)
    {
        nativeRect.cx = 640;
        nativeRect.cy = 480;
    }
    else
    {
        nativeRect = GetVideoSizeFromPin(pASFWriter);
    }
    //For the profile, get the IWMStreamConfig interface for the video stream 
    DWORD dwStreams = 0;
    DWORD dwMediaTypeSize = 0;
    hr = pProfile->GetStreamCount(&dwStreams);
    if ( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 3, _T("Failed GetStreamCount (hr=0x%08x)!\n"), hr ));
        return hr;
    }

    for(WORD j = 1; j <= dwStreams ; j++)
    {
        CComPtr<IWMStreamConfig> pWMStreamConfig;
        hr = pProfile->GetStreamByNumber(j, &pWMStreamConfig);
        if ( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE, 3, _T("Failed GetStreamByNumber (hr=0x%08x)!\n"), hr ));
            return hr;
        }

        // Get the stream's major type. Note that we assume only one video stream in the file.
        GUID guidStreamType;
        hr = pWMStreamConfig->GetStreamType(&guidStreamType);
        if ( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE, 3, _T("Failed GetStreamType (hr=0x%08x)!\n"), hr ));
            return hr;
        }
        
        if(IsEqualGUID(WMMEDIATYPE_Video, guidStreamType))
        {
            CComQIPtr<IWMMediaProps, &IID_IWMMediaProps> pMediaProps (pWMStreamConfig);
            if(!pMediaProps)
            {
                DbgLog((LOG_TRACE, 3, _T("Failed to QI for IWMMediaProps (hr=0x%08x)!\n"), hr ));
                return hr;
            }

            //Retrieve the amount of memory required to hold the returned structure
            hr = pMediaProps->GetMediaType(NULL, &dwMediaTypeSize);
            if(FAILED( hr ) )
            {
                DbgLog((LOG_TRACE, 3, _T("Failed GetMediaType first call(hr=0x%08x)!\n"), hr ));
                return hr;
            }

            //Allocate the memory
            BYTE *pData = 0;
            do
            {
                pData = new BYTE[ dwMediaTypeSize ];
                    
                if ( NULL == pData )
                {
                        hr = E_OUTOFMEMORY;
                        DbgLog((LOG_TRACE, 3,_T( " Out of memory: (hr=0x%08x)\n" ), hr ));
                        break;
                }

                ZeroMemory( pData, dwMediaTypeSize );

                // Retreive the actual WM_MEDIA_TYPE structure and format block
                hr = pMediaProps->GetMediaType( ( WM_MEDIA_TYPE *) pData, &dwMediaTypeSize );
                if ( FAILED( hr ) )
                {
                    DbgLog((LOG_TRACE, 3,_T( " Get Mediatype second call failed: (hr=0x%08x)\n" ), hr ));
                    break;
                }
            
                WM_MEDIA_TYPE* pMT = ( WM_MEDIA_TYPE *) pData; // pMT is easier to read
                  
                // Set the native video rectangle size on the BITMAPINFOHEADER
                if(IsEqualGUID(pMT->formattype, WMFORMAT_VideoInfo))
                {
                    WMVIDEOINFOHEADER* pWMVih = (WMVIDEOINFOHEADER*) pMT->pbFormat;
                    pWMVih->bmiHeader.biHeight = nativeRect.cy;
                    pWMVih->bmiHeader.biWidth = nativeRect.cx;
                    
                }
                else
                {
                    //We only handle WMFORMAT_VideoInfo
                    DbgLog((LOG_TRACE, 3,_T( "Video Media Type is not WMFORMAT_VideoInfo\n" )));
                    break;
                }

                hr = pMediaProps->SetMediaType(pMT);
                if ( FAILED( hr ) )
                {
                    DbgLog((LOG_TRACE, 3,_T( " SetMediaType failed: (hr=0x%08x)\n" ), hr ));
                    break;
                }
                hr = pProfile->ReconfigStream(pWMStreamConfig);
                if ( FAILED( hr ) )
                {
                    DbgLog((LOG_TRACE, 3,_T( " ReconfigStream failed: (hr=0x%08x)\n" ), hr ));
                    break;
                }
                  
                
            }while (FALSE);
        
            SAFE_ARRAYDELETE(pData);

        }//end ifIsEqualGUID
      
    }

    return hr;

}


// All the WM ASF Writer's pins are input pins, so we don't need to test for PIN_DIRECTION
// Note that we don't use IAMStreamConfig::GetFormat because that returns the format that will be used
// to create the file, in other words the format based on the profile. We want the input format, so we call IPin::ConnectionMediaType.

SIZE GetVideoSizeFromPin(IBaseFilter *pFilter)
{
    SIZE       vidRect;
    vidRect.cx = 0;
    vidRect.cy = 0;

    BOOL       bFound = FALSE;
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return vidRect;
    }
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        
        AM_MEDIA_TYPE mt;
        pPin->ConnectionMediaType(&mt);
        if (IsEqualGUID(FORMAT_VideoInfo, mt.formattype))
        {
           DisplayType("GetVideoSizeFromPin: Video MT= \n", &mt);
           VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*) mt.pbFormat;
           BITMAPINFOHEADER bmi = vih->bmiHeader;
           vidRect.cx = bmi.biWidth;
           vidRect.cy = bmi.biHeight;
           break;
        }
        else if (IsEqualGUID(FORMAT_VideoInfo2, mt.formattype))
        {
           VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*) mt.pbFormat;
           BITMAPINFOHEADER bmi = vih->bmiHeader;
           vidRect.cx = bmi.biWidth;
           vidRect.cy = bmi.biHeight;
           break;
        }
        CoTaskMemFree((VIDEOINFOHEADER*)mt.pbFormat);
        pPin.Release();
    }

    return vidRect;  
}


HRESULT CreateProfileManager(IWMProfileManager** pPM)
{
   
    // Create profile manager
    //
    HRESULT hr = S_OK;
    
    hr = WMCreateProfileManager( pPM );
    if( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 3,_T( " WMCreateProfileManager failed: (hr=0x%08x)\n" ), hr ));
        return hr;
    }
    
    return hr;

}



//------------------------------------------------------------------------------
// Name: WriteProfileAsPRX()
// Desc: Writes a profile to a PRX file.
//------------------------------------------------------------------------------
HRESULT WriteProfileAsPRX( IWMProfileManager* pProfileManager, IWMProfile* pProfile)
{
    HANDLE hFile = NULL;
    HRESULT hr = S_OK;
    DWORD dwBytesWritten, dwProfileDataLength;
    BOOL bResult;
    WCHAR* wszProfileData = NULL;
     
    // Convert the profile to XML
    // First get the number of WCHARs required for the buffer
    dwProfileDataLength = 0;
    hr = pProfileManager->SaveProfile( pProfile, NULL, &dwProfileDataLength );
    
	if ( FAILED( hr ) )
    {
		DbgLog((LOG_TRACE, 3,_T( " SaveProfile with NULL failed: (hr=0x%08x)\n" ), hr ));
        return hr;
    }
    
	// Allocate the buffer
	dwProfileDataLength = dwProfileDataLength * sizeof( WCHAR );
    wszProfileData = new WCHAR[ dwProfileDataLength + 1 ];
    if ( !wszProfileData )
    {
        hr = E_OUTOFMEMORY;
        DbgLog((LOG_TRACE, 3,_T( " could not allocate profile buffer: (hr=0x%08x)\n" ), hr ));
        return hr;
    }
    
	// Write the profile into the buffer
	hr = pProfileManager->SaveProfile( pProfile, wszProfileData, &dwProfileDataLength );
    if ( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 3,_T( " SaveProfile failed: (hr=0x%08x)\n" ), hr ));
        return hr;
    }


    do
    {
        //
        // Create the file, overwriting any existing file
        //
        hFile = CreateFile( "test4.prx", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
        if ( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Write the profile data from the buffer to the file
        //
        bResult = WriteFile( hFile, wszProfileData, dwProfileDataLength, &dwBytesWritten, NULL );
        if ( !bResult )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }
    }
    while ( FALSE );

    //
    // Close the file, if it was opened successfully
    //
    SAFE_CLOSEHANDLE( hFile );
    SAFE_ARRAYDELETE( wszProfileData ) ;

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateFilterGraph()
// Desc: Create a DirectShow filter graph.
//------------------------------------------------------------------------------
HRESULT CreateFilterGraph(IGraphBuilder **pGraph)
{
    HRESULT hr;

    if (!pGraph)
        return E_POINTER;

    hr = CoCreateInstance(CLSID_FilterGraph, // get the graph object
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder,
                          (void **) pGraph);

    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("CreateFilterGraph: Failed to create graph!  hr=0x%x\n"), hr));
        *pGraph = NULL;
        return hr;
    }

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: CreateFilter()
// Desc: Create a DirectShow filter.
//------------------------------------------------------------------------------
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

    if (!ppFilter)
        return E_POINTER;

    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) ppFilter);

    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("CreateFilter: Failed to create filter!  hr=0x%x\n"), hr));
        *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: SetNoClock()
// Desc: Prevents an unnecessary clock from being created.
// This speeds up the copying process, since the renderer won't wait
// for the proper time to render a sample; instead, the data will
// be processed as fast as possible.
//------------------------------------------------------------------------------
HRESULT SetNoClock(IFilterGraph *pGraph)
{
    if (!pGraph)
        return E_POINTER;

    CComPtr<IMediaFilter> pFilter;
    HRESULT hr = pGraph->QueryInterface(IID_IMediaFilter, (void **) &pFilter);

    if(SUCCEEDED(hr))
    {
        // Set to "no clock"
        hr = pFilter->SetSyncSource(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("SetNoClock: Failed to set sync source!  hr=0x%x\n"), hr));
        }
    }
    else
    {
        DbgLog((LOG_TRACE, 3, _T("SetNoClock: Failed to QI for media filter!  hr=0x%x\n"), hr));
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: OutputMsg()
// Desc: Displays status messages in the user interface output window
//------------------------------------------------------------------------------

void OutputMsg(_TCHAR* msg)
{

	int nInputLen = (int) _tcslen(msg);
	g_OutputStringLen += nInputLen;

    if ( g_nOutputStringBufferSize > g_OutputStringLen + 1)
    {
       _tcsncat(szOutputWindow, msg, (size_t) nInputLen);
    }
    
 
	HWND hDesc = GetDlgItem(g_hwnd, IDC_DESC);
    SetWindowText(hDesc, szOutputWindow);
    
}


//------------------------------------------------------------------------------
// Name: WaitForCompletion()
// Desc: Waits for a media event that signifies completion or cancellation
//       of a task.
//------------------------------------------------------------------------------
LONG WaitForCompletion(HWND g_hwnd, IGraphBuilder *pGraph )
{
    HRESULT hr;
    LONG levCode = 0;
    CComPtr <IMediaEvent> pME;

    if (!pGraph)
        return -1;
        
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME);
    if (SUCCEEDED(hr))
    {
        OutputMsg(_T("Waiting for completion. This could take several minutes, depending on file size and selected profile.\r\n"));
        DbgLog((LOG_TRACE, 3, _T("Waiting for completion...\n  This could take several minutes, ")
                 _T("depending on file size and selected profile.\n")));
        HANDLE hEvent;
        
        hr = pME->GetEventHandle((OAEVENT *)&hEvent);
        if(SUCCEEDED(hr)) 
        {
            // Wait for completion and dispatch messages for any windows
            // created on our thread.
            for(;;)
            {
                while(MsgWaitForMultipleObjects(
                    1,
                    &hEvent,
                    FALSE,
                    INFINITE,
                    QS_ALLINPUT) != WAIT_OBJECT_0)
                {
                    MSG Message;

                    while (PeekMessage(&Message, NULL, 0, 0, TRUE))
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }

                // Event signaled. See if we're done.
                LONG_PTR lp1, lp2;
                
                if(pME->GetEvent(&levCode, &lp1, &lp2, 0) == S_OK)
                {
                    pME->FreeEventParams(levCode, lp1, lp2);
                
                    if(EC_COMPLETE == levCode)
                    {
                        OutputMsg(_T("Encoding complete!\r\n"));
                        // Display received event information
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_COMPLETE.\n")));
                        }                            
                        break;
                    }
                    else if(EC_ERRORABORT == levCode)
                    {
                        OutputMsg(_T("Error abort!\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_ERRORABORT.\n")));
                        }                            
                        break;
                    }
                    else if(EC_USERABORT == levCode)
                    {
                        OutputMsg(_T("User Abort\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_USERABORT.\n")));
                        }
                        break;
                    }
                    else if( EC_PREPROCESS_COMPLETE == levCode)
                    {        
                        OutputMsg(_T("Preprocess pass complete!\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_PREPROCESS_COMPLETE.\n")));
                        }
                        break;
                    }
                    else
                    {   
                        OutputMsg(_T("Received some unknown event!\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received event %d.\n"), levCode));
                        }
                    }
                }
            }
        }
        else
        {
            OutputMsg(_T("Unexpected Failure!\r\n"));
            DbgLog((LOG_TRACE, 3, _T("Unexpected failure (GetEventHandle failed)...\n")));
        }
    }        
    else
        DbgLog((LOG_TRACE, 3, _T("QI failed for IMediaEvent interface!\n")));

    return levCode;
}

BOOL CALLBACK Dlg_Main( HWND hDlg,  UINT msg, WPARAM wParam, LPARAM lParam)
 
{
    g_hwnd = hDlg;
    HWND hCheckBox = 0;
    HRESULT hr = S_OK;
	int nSourceLength = 0;

	switch(msg)
	{
		case WM_INITDIALOG:
				                    
				break;

		case WM_COMMAND:
			
			switch(wParam)
			{
            
			
                case IDC_BTN_SOURCEFILE:
                    OpenFileDialog(hDlg, mediaFile);
                    GetDlgItemText(hDlg, IDC_SOURCEFILE, szSource, _MAX_PATH);
                    nSourceLength = (int) strlen(szSource);
                    ZeroMemory(szTarget, _MAX_PATH);
                    if(nSourceLength > 5)
                    {
                        strncpy(szTarget, szSource, nSourceLength - 4);
                        strcat(szTarget, ".wmv"); //TODO this is lame need to adjust for wma or asf
                        SetDlgItemText(hDlg, IDC_TARGETFILE, szTarget);
                    }
                    break;

                case IDC_BTN_PRXFILE:
                    OpenFileDialog(hDlg, prxFile);
                    break;

				case ID_GO:
                    ZeroMemory(szOutputWindow, g_nOutputStringBufferSize);
                    GetDlgItemText(hDlg, IDC_SOURCEFILE, szSource, _MAX_PATH);
                    GetDlgItemText(hDlg, IDC_TARGETFILE, szTarget, _MAX_PATH);
                    GetDlgItemText(hDlg, IDC_PROFILE, szProfile, _MAX_PATH);
                    GetDlgItemText(hDlg, IDC_AUTHOR, szAuthor, _MAX_PATH);
                    GetDlgItemText(hDlg, IDC_TITLE, szTitle, _MAX_PATH);


                    if(szSource[0] == '\0')
                    {
                        MessageBox(g_hwnd, "Please enter a source file name", NULL, MB_OK);
                        break;
                    }
                    if(szTarget[0] == '\0')
                    {
                        MessageBox(g_hwnd, "Please enter a target file name", NULL, MB_OK);
                        break;
                    }

                    if(szProfile[0] == '\0')
                    {
                        MessageBox(g_hwnd, "Please enter a profile file name", NULL, MB_OK);
                        break;
                    }

                    //does the user want to do multipass encoding?
                    hCheckBox = GetDlgItem(hDlg, IDC_MULTIPASS);
                    if(SendMessage(hCheckBox, BM_GETCHECK, NULL, NULL))
                    {
                        g_fMultipassEncode = TRUE;
                    }
                    else
                    {
                        g_fMultipassEncode = FALSE;
                    }

                    //does the user want frame indexes?
                    hCheckBox = GetDlgItem(hDlg, IDC_FRAMEINDEX);
                    if(SendMessage(hCheckBox, BM_GETCHECK, NULL, NULL))
                    {
                        g_fFrameIndexing = TRUE;
                    }
                    else
                    {
                        //reset the global variable for next time
                        g_fFrameIndexing = FALSE;
                    }

                    //does the user want SMPTE time codes?
                    hCheckBox = GetDlgItem(hDlg, IDC_SMPTE);
                    if(SendMessage(hCheckBox, BM_GETCHECK, NULL, NULL))
                    {
                        g_fSMPTETimecodes = TRUE;
                    }
                    else
                    {
                        g_fSMPTETimecodes = FALSE;
                    }
                    
                    hr = MakeAsfFile(szSource, szTarget, szProfile);
                    if(FAILED(hr))
                    {
                        OutputMsg(_T("Encoding session failed.\r\n"));
                    }
                    
                    DbgLog((LOG_TRACE, 3, _T("Completed MakeAsfFile\n")));
                     

					break;
                    
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
                    DbgLog((LOG_TRACE, 3, _T("End dialog\n")));
                    
					break;
			
			}
		break;

		default:
		break;
	
	}

    return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	hInst = hInstance;
	CoInitialize(NULL);

	// Dshow debug initialization
	DbgInitialise(hInst);
	DbgSetModuleLevel(LOG_TRACE | LOG_ERROR, 3);

   

	// This will loop until the Exit button is hit
	DialogBox(hInst, MAKEINTRESOURCE(IDD_MAKEASF), NULL, (DLGPROC)Dlg_Main);

    // Clean up
	CoUninitialize();
 	DbgTerminate(); //causes access violation
	
    return 0;
}



HRESULT RenderOutputPins(IBaseFilter *pFilter, IGraphBuilder* pGB)
{
 
    CComPtr <IEnumPins>     pEnum;
    CComPtr <IPin>          pPin;
    CComQIPtr<IFilterGraph2, &IID_IFilterGraph2> pFilterGraph2(pGB);

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }

    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PINDIR_OUTPUT == PinDirThis)
        {
            pFilterGraph2->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE, 3, _T("Failed to render source filter pin (hr=0x%08x)!\n"), hr));
                return hr;
            }
        }
        pPin.Release();
    }
    
    return hr;  
}

HRESULT AddSmpteDataUnitExtension(IWMProfile *pProfile)
{
    HRESULT hr;
     
    DWORD dwStreams = 0;
    DWORD dwMediaTypeSize = 0;
    hr = pProfile->GetStreamCount(&dwStreams);
    if ( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 3, _T("Failed GetStreamCount (hr=0x%08x)!\n"), hr ));
        return hr;
    }

    //First, find the profile's video stream
    for(WORD j = 1; j <= dwStreams ; j++)
    {
        CComPtr<IWMStreamConfig> pWMStreamConfig;
        hr = pProfile->GetStreamByNumber(j, &pWMStreamConfig);
        if ( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE, 3, _T("Failed GetStreamByNumber (hr=0x%08x)!\n"), hr ));
            return hr;
        }

        // Get the stream's major type. Note that in this example we assume 
        // that there is only one video stream in the file.
        GUID guidStreamType;
        hr = pWMStreamConfig->GetStreamType(&guidStreamType);
        if ( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE, 3, _T("Failed GetStreamType (hr=0x%08x)!\n"), hr ));
            return hr;
        }
        
        // If this is the video stream, then set the DUE on it
        if(IsEqualGUID(WMMEDIATYPE_Video, guidStreamType))
        {
            CComQIPtr<IWMStreamConfig2, &IID_IWMStreamConfig2> pWMStreamConfig2 (pWMStreamConfig);

            hr = pWMStreamConfig2->AddDataUnitExtension(  WM_SampleExtensionGUID_Timecode,
                                                    WM_SampleExtension_Timecode_Size,
                                                    NULL,
                                                    0
                                                    );

            if ( FAILED( hr ) )
            {
                DbgLog((LOG_TRACE, 3, _T("Failed to set SMPTE DUE (hr=0x%08x)!\n"), hr ));
                return hr;
            }
            
            
            // Don't forget to call this, or else none of the changes will go into effect!
            hr = pProfile->ReconfigStream(pWMStreamConfig);
            if ( FAILED( hr ) )
            {
                DbgLog((LOG_TRACE, 3, _T("Failed to reconfig stream (hr=0x%08x)!\n"), hr ));
                return hr;
            }

            return S_OK;
            
        }

    }
   
    // We didn't find a video stream in the profile so just fail without trying 
    // anything heroic.
    return E_FAIL; 

}

HRESULT GetPinByMajorType(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID majortype, IPin** ppPin)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        AM_MEDIA_TYPE mt;
		
        // Try this first as it is easy
        hr = pPin->ConnectionMediaType(&mt);
        if(SUCCEEDED(hr))
        {
            if (PinDir == PinDirThis && IsEqualGUID(majortype, mt.majortype))
            {
                *ppPin = pPin.Detach();
                CoTaskMemFree(mt.pbFormat);
                return S_OK;
            }

		    CoTaskMemFree(mt.pbFormat); //can COnnectionMediaType fail and still fill in pbFormat?
        }
        else
        {
            if(hr == VFW_E_NOT_CONNECTED)
            {
                CComPtr<IEnumMediaTypes> pEnumMediaTypes;
                AM_MEDIA_TYPE* pMT;
                pPin->EnumMediaTypes(&pEnumMediaTypes);
                while(pEnumMediaTypes->Next(1, &pMT, 0) == S_OK)
                {
                    if(pMT->majortype == majortype)
                    {
                        *ppPin = pPin.Detach();
                        FreeMediaType(*pMT);
                        return S_OK;

                    }
                    FreeMediaType(*pMT);
                } // end while

            } // end if VFW_E_NOT_CONNECTED
        } //end else
        pPin.Release();
    } // while while pPin
    
    return E_FAIL;  
}

// Metadata should be added-modified using the IWMHeaderInfo3 interface
// but the WM ASF Writer only implements IWMHeaderInfo. To get the IWMHeaderInfo3
// we need a pointer to the underlying WM Writer object itself, which we get through
// a QueryService call. Once we have the IWMWriterAdvanced2 interface we can use it to
// query for any other interface on the writer. A word to the wise -- don't try to
// modify any streaming properties on the writer directly, except what is available through IWMWriterAdvanced2.
// Modifying metadata is safe as long as you do it after the filter has been configured
// with the profile that will actually be used to write the file.
HRESULT AddMetadata(IBaseFilter* pFilter, WCHAR* pwszAuthor, WCHAR* pwszTitle)
{
    HRESULT hr = S_OK;
    CComQIPtr<IServiceProvider> pServiceProvider(pFilter);
    if(!pServiceProvider)
    {
        return E_FAIL;
    }

    CComPtr<IWMWriterAdvanced2> pWriterAdvanced2;
    hr = pServiceProvider->QueryService(IID_IWMWriterAdvanced2, &pWriterAdvanced2);

    if(FAILED(hr))
    {
        return hr;
    }

    CComQIPtr<IWMHeaderInfo3, &IID_IWMHeaderInfo3> pWMHeaderInfo3 (pWriterAdvanced2);
    if (!pWMHeaderInfo3)
    {
        return E_FAIL;
    }

    // If we wanted the ability to modify these attributes later in our session,
    // we would store this value. 
    WORD pwIndex = 0;
    DWORD length = (DWORD) (wcslen(pwszAuthor)* sizeof(WCHAR)) ;

    //Set the author that the user entered
    hr = pWMHeaderInfo3->AddAttribute(0, g_wszWMAuthor, &pwIndex, WMT_TYPE_STRING, 0, (BYTE*) pwszAuthor, length);
    if(FAILED(hr))
    {
        return hr;
    }
       
    //Set the title that the user entered
    length = (DWORD)(wcslen(pwszTitle)* sizeof(WCHAR)) ;
    hr = pWMHeaderInfo3->AddAttribute(0, g_wszWMTitle, &pwIndex, WMT_TYPE_STRING, 0, (BYTE*) pwszTitle, length);
    
    return hr;
}

//Get the average time per frame from the video pin
// This is calculated as 10,000,000 / frames per second
// 333333 = 30 fps , 333667 = 29.97 fps, etc.

REFERENCE_TIME GetTimePerFrameFromPin(IPin *pPin)
{
    
    REFERENCE_TIME        rtTimePerFrame = 0;
    AM_MEDIA_TYPE mt;
    pPin->ConnectionMediaType(&mt);
    if (IsEqualGUID(FORMAT_VideoInfo, mt.formattype))
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*) mt.pbFormat;
        rtTimePerFrame = vih->AvgTimePerFrame;
        
    }
    else if (IsEqualGUID(FORMAT_VideoInfo2, mt.formattype))
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*) mt.pbFormat;
        rtTimePerFrame = vih->AvgTimePerFrame;
       
    }

    return rtTimePerFrame;  
}
