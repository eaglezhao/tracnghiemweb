// ASF Writer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <streams.h>
#include "wmsdk.h"
#include <dshowasf.h>
#define __AFXWIN_H__
#include "C:\WMSDK\WMFSDK9\samples\wmgenprofile\exe\genprofile.h"
 
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

HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION PinDir,
						DWORD dwPin,  IPin ** ppPin )
{
	HRESULT			hr = S_OK;
	IEnumPins *		pEnumPin = NULL;
	IPin *			pConnectedPin = NULL;
	PIN_DIRECTION	PinDirection;
	ULONG			ulFetched;
	DWORD			nFound = 0;

	ASSERT( pFilter != NULL );
	*ppPin = NULL;

	hr = pFilter->EnumPins( &pEnumPin );
	if(SUCCEEDED(hr))
	{
		while ( S_OK == ( hr = pEnumPin->Next( 1L, ppPin, &ulFetched ) ) )
		{
				hr = (*ppPin)->QueryDirection( &PinDirection );
				if ( ( S_OK == hr ) && ( PinDirection == PinDir ) )
				{
					if ( nFound == dwPin )
					{
						break;
					}
					nFound++;
				}
		 
			(*ppPin)->Release();
		}
	}
	pEnumPin->Release();
	return hr;
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

	IGraphBuilder     *pGraph=NULL;
	IMediaControl     *pControl=NULL;
	IBaseFilter       *pFilter=NULL;
	IConfigAsfWriter  *pConfig = NULL;
	IFileSinkFilter2  *pSink = NULL;
	AM_MEDIA_TYPE     pMediaType;
		static OAFilterState state;
	IMediaEvent       *pEvent;
	LONG lEvCode = 0;

	CoInitialize (NULL);
 
	CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **) &pGraph);

	CKeyProvider prov;
	prov.AddRef();  

	IObjectWithSite* pObjectWithSite = NULL;

	HRESULT hr = pGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
	if (SUCCEEDED(hr))
	  {
		pObjectWithSite->SetSite((IUnknown *) (IServiceProvider *) &prov);
		pObjectWithSite->Release();
	  }
 	
	CoCreateInstance (CLSID_WMAsfWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **) &pFilter);
	
	pGraph->AddFilter (pFilter, L"ASF Writer");
	

	pGraph->QueryInterface (IID_IMediaControl, (void **) &pControl);

	hr = pFilter->QueryInterface (IID_IFileSinkFilter2, (void **) &pSink);
 
	pSink->SetFileName (L"C:\\Bocelli.wma", &pMediaType);

	pFilter->QueryInterface (IID_IConfigAsfWriter, (void **) &pConfig);
	pConfig->ConfigureFilterUsingProfile (GetProfile());
	hr = pGraph->RenderFile (L"C:\\Program Files\\Napster\\My Files\\Andrea Bocelli & Sarah Brightman - Time To Say Good Bye.mp3", NULL);
	if(FAILED(hr))
    {
        printf("Failed to render the file!  hr=0x%x\nCopy aborted.\n\n", hr);
    }
	hr = pControl->Run ();
 	if(FAILED(hr))
    {
        printf("Failed to run the graph!  hr=0x%x\nCopy aborted.\n\n", hr);
    }
    else
	{
    pGraph->QueryInterface(IID_IMediaEvent, (void **) &pEvent);

 	hr = pEvent->WaitForCompletion(-1, &lEvCode);
    printf("Copy completed.\n");

    // Stop the graph
    hr = pControl->Stop();
	}

	CoUninitialize  ();

	return 0;
}
