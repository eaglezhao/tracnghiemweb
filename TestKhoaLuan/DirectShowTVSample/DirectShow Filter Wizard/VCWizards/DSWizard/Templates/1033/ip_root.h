#pragma once

class C[!output PROJECT_NAME]
	: public CTransInPlaceFilter,
	  public I[!output PROJECT_NAME],
	  public ISpecifyPropertyPages,
	  public CPersistStream
{
public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    // Constructor
    C[!output PROJECT_NAME](TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Destructor
    ~C[!output PROJECT_NAME]();

    // Overrriden from CTransformFilter base class
    HRESULT Transform(IMediaSample *pSample);
    HRESULT CheckInputType(const CMediaType* mtIn);

    // CPersistStream stuff
    HRESULT ScribbleToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);

    // Reveals ITransformTemplate and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // These implement the custom ITransformTemplate interface
    STDMETHODIMP get_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters *irp);
    STDMETHODIMP put_[!output PROJECT_NAME]([!output PROJECT_NAME]Parameters irp);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

private:
	BOOL CanPerformTransform(const CMediaType *pMediaType) const;

	[!output PROJECT_NAME]Parameters m_[!output PROJECT_NAME]Parameters;
	CCritSec m_[!output PROJECT_NAME]Lock;
};
