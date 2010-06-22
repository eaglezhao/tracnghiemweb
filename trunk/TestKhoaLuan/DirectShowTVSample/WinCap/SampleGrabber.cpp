#include "StdAfx.h"
#include "samplegrabber.h"

//-----------------------------------------------------------------------------
// Name: StillCap()
// Desc: Constructor
//-----------------------------------------------------------------------------

StillCap::StillCap(void)
: m_cRef(0)
{
	ZeroMemory(&m_MediaType, sizeof(AM_MEDIA_TYPE));
}

//-----------------------------------------------------------------------------
// Name: ~StillCap()
// Desc: Destructor
//-----------------------------------------------------------------------------

StillCap::~StillCap(void)
{
	_ASSERTE(m_cRef == 0);
}

//-----------------------------------------------------------------------------
// Name: SetMediaType()
// Desc: Store the media type that the Sample Grabber connected with
//
// mt:   Reference to the media type
//-----------------------------------------------------------------------------

HRESULT StillCap::SetMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.majortype != MEDIATYPE_Video) 
	{
		return VFW_E_INVALIDMEDIATYPE;
	}

	if ((mt.formattype != FORMAT_VideoInfo) ||
		(mt.cbFormat < sizeof(VIDEOINFOHEADER)) ||
		(mt.pbFormat == NULL))
	{
		return VFW_E_INVALIDMEDIATYPE;
	}

	return CopyMediaType(&m_MediaType, &mt);
}


//-----------------------------------------------------------------------------
// Name: QueryInterface()
// Desc: Our impementation of IUnknown::QueryInterface
//-----------------------------------------------------------------------------

STDMETHODIMP StillCap::QueryInterface(REFIID riid, void **ppvObject)
{
	if (NULL == ppvObject)
		return E_POINTER;
	if (riid == __uuidof(IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else if (riid == __uuidof(ISampleGrabberCB))
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
	else 
		return E_NOTIMPL;
	AddRef();
	return S_OK;
}

// Note: ISampleGrabber supports two callback methods: One gets the IMediaSample
// and one just gets a pointer to the buffer.

//-----------------------------------------------------------------------------
// Name: SampleCB()
// Desc: Callback that gets the media sample. (NOTIMPL - We don't use this one.)
//-----------------------------------------------------------------------------

STDMETHODIMP StillCap::SampleCB(double SampleTime, IMediaSample *pSample)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
// Name: BufferCB()
// Desc: Callback that gets the buffer. We write the bits to a bmp file.
//
// Note: In general it's a bad idea to do anything time-consuming inside the
// callback (like write a bmp file) because you can stall the graph. Also
// on Win9x you might be holding the Win16 Mutex which can cause deadlock. 
//
// Here, I know that (a) I'm not rendering the video, (b) I'm getting still
// images one at a time, not a stream, (c) I'm running XP. However there's
// probably a better way to design this.
//-----------------------------------------------------------------------------

STDMETHODIMP StillCap::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
    // Create the file.
	HANDLE hf = CreateFile(
		"C:\\Test.bmp",
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL );


	if (hf == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, TEXT("Cannot create the file!"),
			TEXT("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}


    // Size of the BITMAPINFO part of the VIDEOINFOHEADER
	long cbBitmapInfoSize = m_MediaType.cbFormat - SIZE_PREHEADER;

    // BITMAPINFO is the BITMAPINFOHEADER plus (possibly) palette entries or color masks,
    // so the size can change. We have to be careful to write the correct number of bytes.


    // In theory we validated these assumptions earlier
    _ASSERTE(m_MediaType.formattype == FORMAT_VideoInfo);
    _ASSERTE(m_MediaType.cbFormat >= sizeof(VIDEOINFOHEADER));
    _ASSERTE(m_MediaType.pbFormat != NULL);

	VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)m_MediaType.pbFormat;

	// I'm forcing the biCompression to RGB to work around a driver issue (?) in the
    // Logitech web cam. The camera sets biCompression to FOURCC 'RGB5' which doesn't
    // mean anything as far as I can determine.
	pVideoHeader->bmiHeader.biCompression = 0;

    // Prepare the bitmap file header
	BITMAPFILEHEADER bfh;
	ZeroMemory(&bfh, sizeof(bfh));

	bfh.bfType = 'MB';  // Little-endian
	bfh.bfSize = sizeof( bfh ) + BufferLen + cbBitmapInfoSize;
	bfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + cbBitmapInfoSize;

	DWORD dwWritten = 0;

	// Write the file header
	WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );
	dwWritten = 0;

    // Write the BITMAPINFO
	WriteFile(hf, HEADER(pVideoHeader), cbBitmapInfoSize, &dwWritten, NULL);		

    // Write the bits
	dwWritten = 0;
	WriteFile( hf, pBuffer, BufferLen, &dwWritten, NULL );

	CloseHandle( hf );
	return S_OK;

}


