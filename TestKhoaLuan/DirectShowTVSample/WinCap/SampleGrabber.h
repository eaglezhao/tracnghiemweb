#pragma once

/******************************************************************************
 *  StillCap Class
 *  Callback for ISampleGrabber - this callback writes the image to a bitmap file.
 *****************************************************************************/

class StillCap : public ISampleGrabberCB
{
private:
	volatile long m_cRef;
	AM_MEDIA_TYPE m_MediaType;

public:
	StillCap(void);
	~StillCap(void);

	HRESULT SetMediaType(AM_MEDIA_TYPE& mt);

	// IUnknown methods

	STDMETHODIMP_(ULONG) AddRef() 
	{ 
		return InterlockedIncrement(&m_cRef); 
	}

	STDMETHODIMP_(ULONG) Release() 
	{ 
	    long lCount = InterlockedDecrement(&m_cRef);
		if (lCount == 0)
        {
            delete this;
		}
		// Return the temporary variable, not the member
		// variable, for thread safety.
		return (ULONG)lCount;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);

	// ISampleGrabberCB methods
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);	
	STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

};
