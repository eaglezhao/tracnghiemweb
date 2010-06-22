#include "stdafx.h"
#include "dsutil.h"


HRESULT AddFilterByCLSID(
    IGraphBuilder *pGraph,  // Pointer to the Filter Graph Manager.
    const GUID& clsid,      // CLSID of the filter to create.
    LPCWSTR wszName,        // A name for the filter.
    IBaseFilter **ppF)      // Receives a pointer to the filter.
{
    if (!pGraph || ! ppF) return E_POINTER;
    *ppF = 0;
    IBaseFilter *pF = 0;
    HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&pF));
    if (SUCCEEDED(hr))
    {
        hr = pGraph->AddFilter(pF, wszName);
        if (SUCCEEDED(hr))
            *ppF = pF;
        else
            pF->Release();
    }
    return hr;
}



HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else  // Unconnected, this is the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}



HRESULT ConnectFilters(
    IGraphBuilder *pGraph, // Filter Graph Manager.
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest)    // Downstream filter.
{
    if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }
#ifdef debug
        PIN_DIRECTION PinDir;
        pOut->QueryDirection(&PinDir);
        _ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

    // Find an input pin on the downstream filter.
    IPin *pIn = 0;
    HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr))
    {
        return hr;
    }
    // Try to connect them.
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
    return hr;
}



HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc, 
    IBaseFilter *pDest)
{
    if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }

    // Find an output pin on the first filter.
    IPin *pOut = 0;
    HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) 
    {
        return hr;
    }
    hr = ConnectFilters(pGraph, pOut, pDest);
    pOut->Release();
    return hr;
}



HRESULT FindFilterInterface(
    IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
    REFGUID iid,           // IID of the interface to retrieve.
    void **ppUnk)          // Receives the interface pointer.
{
    if (!pGraph || !ppUnk) return E_POINTER;

    HRESULT hr = E_FAIL;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pF = NULL;
    if (FAILED(pGraph->EnumFilters(&pEnum)))
    {
        return E_FAIL;
    }
    // Query every filter for the interface.
    while (S_OK == pEnum->Next(1, &pF, 0))
    {
        hr = pF->QueryInterface(iid, ppUnk);
        pF->Release();
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    pEnum->Release();
    return hr;
}



HRESULT FindPinInterface(
    IBaseFilter *pFilter,  // Pointer to the filter to search.
    REFGUID iid,           // IID of the interface.
    void **ppUnk)          // Receives the interface pointer.
{
    if (!pFilter || !ppUnk) return E_POINTER;

    HRESULT hr = E_FAIL;
    IEnumPins *pEnum = 0;
    if (FAILED(pFilter->EnumPins(&pEnum)))
    {
        return E_FAIL;
    }
    // Query every pin for the interface.
    IPin *pPin = 0;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        hr = pPin->QueryInterface(iid, ppUnk);
        pPin->Release();
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    pEnum->Release();
    return hr;
}




HRESULT FindInterfaceAnywhere(
    IGraphBuilder *pGraph, 
    REFGUID iid, 
    void **ppUnk)
{
    if (!pGraph || !ppUnk) return E_POINTER;
    HRESULT hr = E_FAIL;
    IEnumFilters *pEnum = 0;
    if (FAILED(pGraph->EnumFilters(&pEnum)))
    {
        return E_FAIL;
    }
    // Loop through every filter in the graph.
    IBaseFilter *pF = 0;
    while (S_OK == pEnum->Next(1, &pF, 0))
    {
        hr = pF->QueryInterface(iid, ppUnk);
        if (FAILED(hr))
        {
            // The filter does not expose the interface, but maybe
            // one of its pins does.
            hr = FindPinInterface(pF, iid, ppUnk);
        }
        pF->Release();
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    pEnum->Release();
    return hr;
}



// Get the first upstream or downstream filter
HRESULT GetNextFilter(
    IBaseFilter *pFilter, // Pointer to the starting filter
    PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
    IBaseFilter **ppNext) // Receives a pointer to the next filter.
{
    if (!pFilter || !ppNext) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }
        if (ThisPinDir == Dir)
        {
            // Check if the pin is connected to another pin.
            IPin *pPinNext = 0;
            hr = pPin->ConnectedTo(&pPinNext);
            if (SUCCEEDED(hr))
            {
                // Get the filter that owns that pin.
                PIN_INFO PinInfo;
                hr = pPinNext->QueryPinInfo(&PinInfo);
                pPinNext->Release();
                pPin->Release();
                pEnum->Release();
                if (FAILED(hr) || (PinInfo.pFilter == NULL))
                {
                    // Something strange happened.
                    return E_UNEXPECTED;
                }
                // This is the filter we're looking for.
                *ppNext = PinInfo.pFilter; // Client must release.
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching filter.
    return E_FAIL;
}



HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    
    IStorage *pStorage = NULL;
    hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

    IStream *pStream;
    hr = pStorage->CreateStream(
        wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}



HRESULT LoadGraphFile(IGraphBuilder *pGraph, const WCHAR* wszName)
{
    IStorage *pStorage = 0;
    if (S_OK != StgIsStorageFile(wszName))
    {
        return E_FAIL;
    }
    HRESULT hr = StgOpenStorage(wszName, 0, 
        STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 
        0, 0, &pStorage);
    if (FAILED(hr))
    {
        return hr;
    }
    IPersistStream *pPersistStream = 0;
    hr = pGraph->QueryInterface(IID_IPersistStream,
             reinterpret_cast<void**>(&pPersistStream));
    if (SUCCEEDED(hr))
    {
        IStream *pStream = 0;
        hr = pStorage->OpenStream(L"ActiveMovieGraph", 0, 
            STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
        if(SUCCEEDED(hr))
        {
            hr = pPersistStream->Load(pStream);
            pStream->Release();
        }
        pPersistStream->Release();
    }
    pStorage->Release();
    return hr;
}




HRESULT CreateKernelFilter(
    const GUID &guidCategory,  // Filter category.
    LPCOLESTR szName,          // The name of the filter.
    IBaseFilter **ppFilter     // Receives a pointer to the filter.
)
{
    HRESULT hr;
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pEnum = NULL;
    if (!szName || !ppFilter) 
    {
        return E_POINTER;
    }

    // Create the system device enumerator.
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
        IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr))
    {
        return hr;
    }

    // Create a class enumerator for the specified category.
    hr = pDevEnum->CreateClassEnumerator(guidCategory, &pEnum, 0);
    pDevEnum->Release();
    if (hr != S_OK) // S_FALSE means the category is empty.
    {
        return E_FAIL;
    }

    // Enumerate devices within this category.
    bool bFound = false;
    IMoniker *pMoniker;
    while (!bFound && (S_OK == pEnum->Next(1, &pMoniker, 0)))
    {
        IPropertyBag *pBag = NULL;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue; // Maybe the next one will work.
        }
        // Check the friendly name.
        VARIANT var;
        VariantInit(&var);
        hr = pBag->Read(L"FriendlyName", &var, NULL);
        if (SUCCEEDED(hr) && (lstrcmpiW(var.bstrVal, szName) == 0))
        {
            // This is the right filter.
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter,
                (void**)ppFilter);
            bFound = true;
        }
        VariantClear(&var);
        pBag->Release();
        pMoniker->Release();
    }
    pEnum->Release();
    return (bFound ? hr : E_FAIL);
}




HRESULT FindPinByName(IBaseFilter *pFilter, const WCHAR *wszName, IPin **ppPin)
{
    if (!pFilter || !wszName || !ppPin)
    {
        return E_POINTER;
    }
    if (wcslen(wszName) >= MAX_PIN_NAME)
    {
        return E_INVALIDARG;
    }
    
    IEnumPins *pEnum;
    HRESULT hr;
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    IPin *pPin;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        PIN_INFO PinInfo;
        pPin->QueryPinInfo(&PinInfo);
        PinInfo.pFilter->Release();
        if (wcscmp(wszName, PinInfo.achName) == 0)
        {
            *ppPin = pPin;
            pEnum->Release();
            return hr;
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;
}


//----------------------------------------------------------------------------
// GetVideoInfoParameters
//
// Helper function to get the important information out of a VIDEOINFOHEADER
// Author: Robin Speed
//-----------------------------------------------------------------------------

void GetVideoInfoParameters(
    const VIDEOINFOHEADER *pvih, // Pointer to the format header.
    BYTE  * const pbData,        // Pointer to the first address in the buffer.
    DWORD *pdwWidth,         // Returns the width in pixels.
    DWORD *pdwHeight,        // Returns the height in pixels.
    LONG  *plStrideInBytes,  // Add this to a row to get the new row down
    BYTE **ppbTop,           // Returns pointer to the first byte in the top row of pixels.
    bool bYuv                // Is this a YUV format? (true = YUV, false = RGB)
    )
{
    LONG lStride;


    //  For 'normal' formats, biWidth is in pixels. 
    //  Expand to bytes and round up to a multiple of 4.
    if (pvih->bmiHeader.biBitCount != 0 &&
        0 == (7 & pvih->bmiHeader.biBitCount)) 
    {
        lStride = (pvih->bmiHeader.biWidth * (pvih->bmiHeader.biBitCount / 8) + 3) & ~3;
    } 
    else   // Otherwise, biWidth is in bytes.
    {
        lStride = pvih->bmiHeader.biWidth;
    }

    //  If rcTarget is empty, use the whole image.
    if (IsRectEmpty(&pvih->rcTarget)) 
    {
        *pdwWidth = (DWORD)pvih->bmiHeader.biWidth;
        *pdwHeight = (DWORD)(abs(pvih->bmiHeader.biHeight));
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap. 
        {
            *plStrideInBytes = lStride; // Stride goes "down"
            *ppbTop           = pbData; // Top row is first.
        } 
        else        // Bottom-up bitmap
        {
            *plStrideInBytes = -lStride;    // Stride goes "up"
            *ppbTop = pbData + lStride * (*pdwHeight - 1);  // Bottom row is first.
        }
    } 
    else   // rcTarget is NOT empty. Use a sub-rectangle in the image.
    {
        *pdwWidth = (DWORD)(pvih->rcTarget.right - pvih->rcTarget.left);
        *pdwHeight = (DWORD)(pvih->rcTarget.bottom - pvih->rcTarget.top);
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap.
        {
            // Same stride as above, but first pixel is modified down
            // and and over by the target rectangle.
            *plStrideInBytes = lStride;     
            *ppbTop = pbData +
                     lStride * pvih->rcTarget.top +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        } 
        else  // Bottom-up bitmap.
        {
            *plStrideInBytes = -lStride;
            *ppbTop = pbData +
                     lStride * (pvih->bmiHeader.biHeight - pvih->rcTarget.top - 1) +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        }
    }
}


