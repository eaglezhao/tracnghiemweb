// DESList.cpp
//

#include "DESList.h"			// This includes all the major include files needed

// Enumerate all of the effects
HRESULT EnumerateEffects(CLSID searchType)
{
	// Once again, code stolen from the DX9 SDK

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the effect category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(searchType, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					wprintf(L"Effect: %s\n", varName.bstrVal);
				}
				VariantClear(&varName);
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return hr;
}

// A very simple program to capture audio to a file using DirectShow
//
int main(int argc, char* argv[])
{

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
	     // We’ll send our error messages to the console.
        printf("ERROR - Could not initialize COM library");
        return hr;
    }

	// OK, so now we want to build the filter graph
	// Using an AudioCapture filter.
	// But there are several to choose from
	// So we need to enumerate them, then pick one.
	hr = EnumerateEffects(CLSID_VideoEffects1Category);
	hr = EnumerateEffects(CLSID_VideoEffects2Category);

    CoUninitialize();

	return 0;
}


