
#include <streams.h>     // DirectShow (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.


 //{93D83902-A730-405d-B9ED-1D63AF429884}
DEFINE_GUID(CLSID_TransformSample,
0x93D83902, 0xA730, 0x405d, 0xB9, 0xED, 0x1d, 0x63, 0xAF, 0x42, 0x98, 0x84);
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


const AMOVIESETUP_FILTER sudTransformSample =
{ &CLSID_TransformSample          // clsID
, L"Transform Filter Sample"      // strName
, MERIT_DO_NOT_USE                // dwMerit
, 2                               // nPins
, psudPins };                     // lpPin

// CNullNull
//
class CTransformSample
    : public CTransInPlaceFilter
{

public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

private:

    // Constructor - calls the base class constructor
    CTransformSample(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
        : CTransInPlaceFilter (tszName, punk, CLSID_TransformSample, phr)
    { }

    // Overrides the PURE virtual Transform of CTransInPlaceFilter base class

    HRESULT Transform(IMediaSample *pSample){  
		
      	BYTE     *pBuffer;
		int      iPos;

		// obtain a pointer to the actual buffer passed in
		pSample->GetPointer (&pBuffer);
 
            for (iPos=0; iPos < pSample->GetActualDataLength (); iPos++, pBuffer++ ) {
                 
                  *pBuffer=*pBuffer^ 0xff;
            }
	
			return NOERROR;
	
	}

    // We accept any input type.  We'd return S_FALSE for any we didn't like.
    HRESULT CheckInputType(const CMediaType* mtIn) { return S_OK; }
};



// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
    {   { L"Transform Filter Sample"
        , &CLSID_TransformSample
        , CTransformSample::CreateInstance
        , NULL
        , &sudTransformSample }
    };
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//
// CreateInstance
//
// Provide the way for COM to create a TransformSample object
CUnknown * WINAPI CTransformSample::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CTransformSample *pNewObject = new CTransformSample (NAME("Transform, filter, sample"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
} // CreateInstance


/******************************Public*Routine******************************\
* exported entry points for registration and
* unregistration (in this case they only call
* through to default implmentations).
*
*
*
* History:
*
\**************************************************************************/
STDAPI
DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI
DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

// Microsoft C Compiler will give hundreds of warnings about
// unused inline functions in header files.  Try to disable them.
#pragma warning( disable:4514)

