#pragma once

class C[!output PROJECT_NAME] : public CTransformFilter,
		 public I[!output PROJECT_NAME],
		 public ISpecifyPropertyPages,
		 public CPersistStream
{

public:

    DECLARE_IUNKNOWN;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Reveals I[!output PROJECT_NAME] and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // CPersistStream stuff
    HRESULT ScribbleToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);

    // Overrriden from CTransformFilter base class
    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    // These implement the custom I[!output PROJECT_NAME] interface
    STDMETHODIMP get_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters *irp);
    STDMETHODIMP put_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters irp);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

private:

    // Constructor
    C[!output PROJECT_NAME](TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	~C[!output PROJECT_NAME]();

    BOOL CanPerformTransform(const CMediaType *pMediaType) const;

    CCritSec m_[!output PROJECT_NAME]Lock;         // Private play critical section
	[!output PROJECT_NAME]Parameters m_[!output PROJECT_NAME]Parameters;

	HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;

};

