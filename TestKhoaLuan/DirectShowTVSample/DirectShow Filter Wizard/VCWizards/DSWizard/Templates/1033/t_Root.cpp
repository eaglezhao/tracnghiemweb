#include <windows.h>
#include <streams.h>
#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif
#include "i[!output PROJECT_NAME].h"
#include "[!output PROJECT_NAME]Prop.h"
#include "[!output PROJECT_NAME].h"
#include "resource.h"
#include <assert.h>
#include <stdio.h>

#define TRANSFORM_NAME L"[!output PROJECT_NAME] Filter"

// returns width of row rounded up to modulo 4
int RowWidth(int w) {
	if (w % 4)
		w += 4 - w % 4;
	return w;
}

// Setup information
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudpPins[] =
{
    { L"Input",             // Pins string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    },
    { L"Output",            // Pins string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER sud[!output PROJECT_NAME] =
{
    &CLSID_[!output PROJECT_NAME],	// Filter CLSID
    TRANSFORM_NAME,				// String name
    MERIT_DO_NOT_USE,			// Filter merit
    2,							// Number of pins
    sudpPins					// Pin information
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
    { TRANSFORM_NAME
    , &CLSID_[!output PROJECT_NAME]
    , C[!output PROJECT_NAME]::CreateInstance
    , NULL
    , &sud[!output PROJECT_NAME] }
  ,
    { TRANSFORM_NAME L" Properties"
    , &CLSID_[!output PROJECT_NAME]PropertyPage
    , C[!output PROJECT_NAME]Properties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//
// DllRegisterServer
//
// Handles sample registry and unregistry
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

//
// Constructor
//
C[!output PROJECT_NAME]::C[!output PROJECT_NAME](TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr) :
    CTransformFilter(tszName, punk, CLSID_[!output PROJECT_NAME]),
    CPersistStream(punk, phr)
{
	// TODO: read parameters from profile
	m_[!output PROJECT_NAME]Parameters.param1 = GetProfileInt("[!output PROJECT_NAME]", "param1", 0);
	m_[!output PROJECT_NAME]Parameters.param2 = GetProfileInt("[!output PROJECT_NAME]", "param2", 0);
} 

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
// ~C[!output PROJECT_NAME]
//
C[!output PROJECT_NAME]::~C[!output PROJECT_NAME]() 
{
	// TODO: write parameters from profile
	WriteProfileInt("[!output PROJECT_NAME]", "param1", m_[!output PROJECT_NAME]Parameters.param1);
	WriteProfileInt("[!output PROJECT_NAME]", "param2", m_[!output PROJECT_NAME]Parameters.param2);
}

//
// CreateInstance
//
// Provide the way for COM to create a [!output PROJECT_NAME] object
//
CUnknown *C[!output PROJECT_NAME]::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    C[!output PROJECT_NAME] *pNewObject = new C[!output PROJECT_NAME](NAME("[!output PROJECT_NAME]"), punk, phr);
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;
}

//
// NonDelegatingQueryInterface
//
// Reveals I[!output PROJECT_NAME] and ISpecifyPropertyPages
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
// Transform
//
// Transforms the input and saves results in the the output
//
HRESULT C[!output PROJECT_NAME]::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
	// input
    AM_MEDIA_TYPE* pTypeIn = &m_pInput->CurrentMediaType();
    VIDEOINFOHEADER *pVihIn = (VIDEOINFOHEADER *)pTypeIn->pbFormat;
	int inWidth = pVihIn->bmiHeader.biWidth;
	int inHeight = pVihIn->bmiHeader.biHeight;
	int inBytesPerPixel = pVihIn->bmiHeader.biBitCount / 8;
	unsigned char *pSrc = 0;
    pIn->GetPointer((unsigned char **)&pSrc);
	assert(pSrc);

	// output
    AM_MEDIA_TYPE *pTypeOut = &m_pOutput->CurrentMediaType();
	VIDEOINFOHEADER *pVihOut = (VIDEOINFOHEADER *)pTypeOut->pbFormat;
	int outBytesPerPixel = pVihOut->bmiHeader.biBitCount / 8;
	unsigned char *pDst = 0;
    pOut->GetPointer((unsigned char **)&pDst);
	assert(pDst);

	// TODO: insert procesing code here
	// for now, just make a copy of the input
    HRESULT hr = Copy(pIn, pOut);
    if (hr != S_OK)
        return hr;
   
    return NOERROR;
}

//
// CheckInputType
//
// Check the input type is OK - return an error otherwise
//
HRESULT C[!output PROJECT_NAME]::CheckInputType(const CMediaType *mtIn)
{
    // check this is a VIDEOINFOHEADER type
    if (*mtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    // Can we transform this type
    if (CanPerformTransform(mtIn)) {
    	return NOERROR;
    }
    return E_FAIL;
}

//
// Checktransform
//
// Check a transform can be done between these formats
//
HRESULT C[!output PROJECT_NAME]::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if (CanPerformTransform(mtIn)) {
		if (*mtOut->Subtype() == *mtIn->Subtype())
			return S_OK;
		else
			return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return VFW_E_TYPE_NOT_ACCEPTED;
}

//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT C[!output PROJECT_NAME]::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    // Is the input pin connected
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

	// get input dimensions
	CMediaType inMediaType = m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *vihIn = (VIDEOINFOHEADER *)inMediaType.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = GetBitmapSize(&vihIn->bmiHeader);
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT( Actual.cBuffers == 1 );

    if (pProperties->cBuffers > Actual.cBuffers ||
            pProperties->cbBuffer > Actual.cbBuffer) {
                return E_FAIL;
    }
    return NOERROR;
}

//
// GetMediaType
//
// Returns the supported media types in order of preferred  types (starting with iPosition=0)
//
HRESULT C[!output PROJECT_NAME]::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected
    if (m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;

    // This should never happen
    if (iPosition < 0)
        return E_INVALIDARG;

    // Do we have more items to offer
    if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

	// get input dimensions
	CMediaType inMediaType = m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *vihIn = (VIDEOINFOHEADER *)inMediaType.Format();
	int bytesPerPixel = vihIn->bmiHeader.biBitCount / 8;

	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetType(&MEDIATYPE_Video);
	if (bytesPerPixel == 3)
		pMediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
	else
		pMediaType->SetSubtype(&MEDIASUBTYPE_RGB32);
	pMediaType->SetSampleSize(RowWidth(vihIn->bmiHeader.biWidth * bytesPerPixel) * vihIn->bmiHeader.biHeight);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->ReallocFormatBuffer(sizeof(VIDEOINFOHEADER));

	// set VIDEOINFOHEADER
	VIDEOINFOHEADER *vihOut = (VIDEOINFOHEADER *)pMediaType->Format();
	vihOut->rcSource.top = 0;
	vihOut->rcSource.left = 0;
	vihOut->rcSource.bottom = 0;
	vihOut->rcSource.right = 0;
	vihOut->rcTarget.top = 0;
	vihOut->rcTarget.left = 0;
	vihOut->rcTarget.bottom = 0;
	vihOut->rcTarget.right = 0;
	double frameRate = vihIn->AvgTimePerFrame / 10000000.0;
	vihOut->dwBitRate = (int)(frameRate * vihIn->bmiHeader.biWidth * vihIn->bmiHeader.biHeight * bytesPerPixel);
	vihOut->dwBitErrorRate = 0;
	vihOut->AvgTimePerFrame = vihIn->AvgTimePerFrame;

	// set BITMAPINFOHEADER
	vihOut->bmiHeader.biBitCount = bytesPerPixel * 8;
	vihOut->bmiHeader.biClrImportant = 0;
	vihOut->bmiHeader.biClrUsed = 0;
	vihOut->bmiHeader.biCompression = BI_RGB;
	vihOut->bmiHeader.biHeight = vihIn->bmiHeader.biHeight;
	vihOut->bmiHeader.biPlanes = 1;
	vihOut->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	int lineSize = RowWidth(vihIn->bmiHeader.biWidth * bytesPerPixel);
	vihOut->bmiHeader.biSizeImage = lineSize * vihIn->bmiHeader.biHeight;
	vihOut->bmiHeader.biWidth = vihIn->bmiHeader.biWidth;
	vihOut->bmiHeader.biXPelsPerMeter = 0;
	vihOut->bmiHeader.biYPelsPerMeter = 0;

    return NOERROR;
}

//
// CanPerform[!output PROJECT_NAME]
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

	// reconnect
	CMediaType &mt = m_pOutput->CurrentMediaType();
    VIDEOINFOHEADER *pVihOut = (VIDEOINFOHEADER *)mt.pbFormat;
	if (!pVihOut)
		return NOERROR;
	// TODO: modify pVihOut if output resolution or type has changed
	HRESULT hr = ReconnectPin(m_pOutput, &mt);

    return NOERROR;
} 

//
// Copy
//
// Make destination an identical copy of source
//
HRESULT C[!output PROJECT_NAME]::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();
    long lDestSize	= pDest->GetSize();

    ASSERT(lDestSize >= lSourceSize);

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory( (PVOID) pDestBuffer,(PVOID) pSourceBuffer,lSourceSize);

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the Sync point property

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type

    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
	pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the actual data length

    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;
}
