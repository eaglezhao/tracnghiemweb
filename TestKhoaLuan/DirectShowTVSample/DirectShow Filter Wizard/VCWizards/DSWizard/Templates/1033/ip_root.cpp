#include <streams.h>     // DirectShow (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.
#include <stdio.h>
#include "i[!output PROJECT_NAME].h"
#include "[!output PROJECT_NAME].h"
#include "[!output PROJECT_NAME]Prop.h"

#define TRANSFORM_NAME L"[!output PROJECT_NAME] Filter"

// setup data - allows the self-registration to work.
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ &MEDIATYPE_NULL        // clsMajorType
, &MEDIASUBTYPE_NULL };  // clsMinorType

const AMOVIESETUP_PIN psudPins[] =
{ { L"Input"            // strName
  , FALSE               // bRendered
  , FALSE               // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
, { L"Output"           // strName
  , FALSE               // bRendered
  , TRUE                // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
};

const AMOVIESETUP_FILTER sud[!output PROJECT_NAME] =
{ &CLSID_[!output PROJECT_NAME]   // clsID
, TRANSFORM_NAME					// strName
, MERIT_DO_NOT_USE					// dwMerit
, 2									// nPins
, psudPins };						// lpPin

// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
    {   { TRANSFORM_NAME
        , &CLSID_[!output PROJECT_NAME]
        , C[!output PROJECT_NAME]::CreateInstance
        , NULL
        , &sud[!output PROJECT_NAME] },
		{ TRANSFORM_NAME L" Properties"
		, &CLSID_[!output PROJECT_NAME]PropertyPage
		, C[!output PROJECT_NAME]Properties::CreateInstance }
    };
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

//
// WriteProfileInt
//
// Writes an integer to the profile.
//
void WriteProfileInt(char *section, char *key, int i)
{
	char str[80];
	sprintf(str, "%d", i);
	WriteProfileString(section, key, str);
}

//
// C[!output PROJECT_NAME]
//
// Constructor; reads default parameters from registry
//
C[!output PROJECT_NAME]::C[!output PROJECT_NAME](TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
	: CTransInPlaceFilter (tszName, punk, CLSID_[!output PROJECT_NAME], phr), 
      CPersistStream(punk, phr)
{
	// TODO: read parameters from profile
	m_[!output PROJECT_NAME]Parameters.param1 = GetProfileInt("[!output PROJECT_NAME]", "param1", 0);
	m_[!output PROJECT_NAME]Parameters.param2 = GetProfileInt("[!output PROJECT_NAME]", "param2", 0);
}

//
// ~C[!output PROJECT_NAME]
//
// Destructor; saves parameters to registry
//
C[!output PROJECT_NAME]::~C[!output PROJECT_NAME]()
{
	// TODO: write parameters from profile
	WriteProfileInt("[!output PROJECT_NAME]", "param1", m_[!output PROJECT_NAME]Parameters.param1);
	WriteProfileInt("[!output PROJECT_NAME]", "param2", m_[!output PROJECT_NAME]Parameters.param2);
}

//
// Transform
//
// Transforms the media sample in-place
//
HRESULT C[!output PROJECT_NAME]::Transform(IMediaSample *pSample)
{
	// TODO: insert transform code here

	return NOERROR; 
}

//
// NonDelegatingQueryInterface
//
// Reveals ITransformTemplate and ISpecifyPropertyPages
//
STDMETHODIMP C[!output PROJECT_NAME]::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_I[!output PROJECT_NAME]) {
        return GetInterface((I[!output PROJECT_NAME] *) this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

//
// CreateInstance
//
// Provide the way for COM to create a C[!output PROJECT_NAME] object
CUnknown * WINAPI C[!output PROJECT_NAME]::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    C[!output PROJECT_NAME] *pNewObject = new C[!output PROJECT_NAME](NAME("[!output PROJECT_NAME]"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//
// CheckInputType
//
// Check a transform can be done.
//
HRESULT C[!output PROJECT_NAME]::CheckInputType(const CMediaType *mtIn)
{
    if (CanPerformTransform(mtIn))
		return S_OK;
	else
	    return VFW_E_TYPE_NOT_ACCEPTED;
}

//
// CanPerformTransform
//
// We support RGB24 and RGB32 input
//
BOOL C[!output PROJECT_NAME]::CanPerformTransform(const CMediaType *pMediaType) const
{
    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) {
        if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24)) {
            VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
            return (pvi->bmiHeader.biBitCount == 24);
        }
        if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32)) {
            VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
            return (pvi->bmiHeader.biBitCount == 32);
        }
    }
    return FALSE;
} 

#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;

//
// GetClassID
//
// This is the only method of IPersist
//
STDMETHODIMP C[!output PROJECT_NAME]::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}

//
// ScribbleToStream
//
// Overriden to write our state into a stream
//
HRESULT C[!output PROJECT_NAME]::ScribbleToStream(IStream *pStream)
{
	// TODO: write transform parameters to stream
    HRESULT hr;
    WRITEOUT(m_[!output PROJECT_NAME]Parameters.param1);
    WRITEOUT(m_[!output PROJECT_NAME]Parameters.param2);
    return NOERROR;

}

//
// ReadFromStream
//
// Likewise overriden to restore our state from a stream
//
HRESULT C[!output PROJECT_NAME]::ReadFromStream(IStream *pStream)
{
	// TODO: read transform parameters from stream
    HRESULT hr;
    READIN(m_[!output PROJECT_NAME]Parameters.param1);
    READIN(m_[!output PROJECT_NAME]Parameters.param2);
    return NOERROR;
}

//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP C[!output PROJECT_NAME]::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_[!output PROJECT_NAME]PropertyPage;
    return NOERROR;
}

//
// get_[!output PROJECT_NAME]
//
// Copies the transform parameters to the given destination.
//
STDMETHODIMP C[!output PROJECT_NAME]::get_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters *irp)
{
    CAutoLock cAutolock(&m_[!output PROJECT_NAME]Lock);
    CheckPointer(irp,E_POINTER);

	*irp = m_[!output PROJECT_NAME]Parameters;

    return NOERROR;
}

//
// put_[!output PROJECT_NAME]
//
// Copies the transform parameters from the given source.
//
STDMETHODIMP C[!output PROJECT_NAME]::put_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters irp)
{
    CAutoLock cAutolock(&m_[!output PROJECT_NAME]Lock);

	m_[!output PROJECT_NAME]Parameters = irp;
    SetDirty(TRUE);

    return NOERROR;
} 

//
// DllRegisterServer
//
STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
}
