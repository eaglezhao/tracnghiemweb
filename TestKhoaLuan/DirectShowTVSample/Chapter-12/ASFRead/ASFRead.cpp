// ASFRead.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <streams.h>
#include "wmsdk.h"
#include <dshowasf.h>
 
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


int main(int argc, char* argv[])
{

    IGraphBuilder     *pGraph=NULL;
	IMediaControl     *pControl=NULL;
	IBaseFilter       *pFilter=NULL;
	IPin              *pPin=NULL;
	IPin              *pPin2=NULL;
	IFileSourceFilter *pSource=NULL;
    IWMHeaderInfo     *pWMHeaderInfo =NULL;

	AM_MEDIA_TYPE     pMediaType;
	static OAFilterState state;
    
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
 	
	CoCreateInstance (CLSID_WMAsfReader, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **) &pFilter);
	
	pGraph->AddFilter (pFilter, L"ASF Reader");

	hr = pFilter->QueryInterface (IID_IFileSourceFilter, (void **) &pSource);

	hr = pSource->Load (L"C:\\WMSDK\\WMSSDK\\Samples\\Plugin\\demo.asf", &pMediaType);
	if (FAILED (hr))
		printf("Failed to load source file: hr=0x%x\n", hr);
 
	pFilter->QueryInterface (IID_IWMHeaderInfo, (void **) &pWMHeaderInfo);
	
    hr = GetPin (pFilter, PINDIR_OUTPUT, 0,  &pPin);
	if (FAILED (hr))
		printf ("Could not find 1st pin");

    pGraph->Render (pPin);
     
	hr = GetPin (pFilter, PINDIR_OUTPUT, 1,  &pPin2);
	if (FAILED(hr))
		printf ("Could not find 2nd pin");

	pGraph->Render (pPin2); 
	pGraph->QueryInterface (IID_IMediaControl,(void**) &pControl);

	pControl->Run ();
 
	WORD  pcAttributes;
	pWMHeaderInfo->GetAttributeCount (0, &pcAttributes);
	printf ("Attribute Count: %d\n", pcAttributes);
	

	WORD  wIndex;
	WORD  wStreamNum;
	WCHAR *pwszName;
	WORD  cchNameLen;
	WMT_ATTR_DATATYPE  Type;
	BYTE * pValue;
	WORD  cbLength;

	for (wIndex=0; wIndex<=pcAttributes;wIndex++)
	{
		hr= pWMHeaderInfo->GetAttributeByIndex(
			wIndex,
			&wStreamNum,
			NULL,
			&cchNameLen,
			&Type,
			NULL,
			&cbLength
		);
		
		if ( FAILED( hr ) )
		{
 			printf(  "GetAttributeByIndex failed for Script no %d (hr=0x%08x).\n" , wIndex, hr) ;
			break ;
		}


		pwszName = new WCHAR[cchNameLen] ;
		pValue = new BYTE[cbLength] ;

		pWMHeaderInfo->GetAttributeByIndex(
			wIndex,
			&wStreamNum,
			pwszName,
			&cchNameLen,
			&Type,
			pValue,
			&cbLength
		);
		
		if ( FAILED( hr ) )
		{
 			printf(  "GetAttributeByIndex failed for Script no %d (hr=0x%08x).\n" , wIndex, hr) ;
		    break ;
		}

		printf ("Index: %d\n", 	wIndex);
		printf ("Stream No.: %d\n", wStreamNum);
		printf ("Name: %S\n", pwszName);
		printf ("Type: %i\n", Type);
		printf ("Value: %S\n", pValue);
        
	}





	WCHAR	*pwszType = NULL ;
	WORD	cchTypeLen = 0 ;
	WCHAR	*pwszCommand = NULL ;
	WORD	cchCommandLen = 0 ;
	QWORD	cnsScriptTime = 0 ;
	 hr = S_OK ;
	
	WORD    cScript;
	hr = pWMHeaderInfo->GetScriptCount( &cScript ) ;
	if ( FAILED( hr ) )
	{
	//	_tprintf( _T("GetScriptCount failed (hr=0x%08x).\n" ), hr) ;
		return hr;
	}
	
	for( int i = 0 ; i < cScript; ++i)
	{
		//
		// Get the memory reqd for this script
		//

		hr = pWMHeaderInfo->GetScript( i ,
										   NULL ,
										   &cchTypeLen ,
										   NULL ,
										   &cchCommandLen ,
										   &cnsScriptTime ) ;
		if ( FAILED( hr ) )
		{
 			printf(  "GetScript failed for Script no %d (hr=0x%08x).\n" , i, hr) ;
		//	break ;
		}
		
		pwszType = new WCHAR[cchTypeLen] ;
		pwszCommand = new WCHAR[cchCommandLen] ;
		if( pwszType == NULL || pwszCommand == NULL)
		{
			hr = E_OUTOFMEMORY ;
			break ;
		}

		//
		// Now, get the script
		//

		hr = pWMHeaderInfo->GetScript( i ,
										   pwszType ,
										   &cchTypeLen ,
										   pwszCommand ,
										   &cchCommandLen ,
										   &cnsScriptTime ) ;
		if ( FAILED( hr ) )
		{
		printf(  "GetScript failed for Script no %d (hr=0x%08x).\n" , i, hr) ;
    //	break ;
		}
		
		printf ("%i\n", i);
		printf ("type: %S\n", pwszType);
		printf ("command: %S\n\n", pwszCommand);
	MessageBox(NULL,"continue", NULL, NULL);
	}
	
	state=State_Running;
	while (state!= State_Stopped)  // block until stream is stopped 
 		pControl->GetState (0, &state); 

	CoUninitialize  ();
	return 0;
}
