#include "stdafx.h"
#include "device.h"

//-----------------------------------------------------------------------------
// Name: Init()
// Desc: Create a list of monikers for a given category.
//
// category: Category GUID. (See "Filter Categories" in the SDK docs)
//-----------------------------------------------------------------------------

HRESULT CDeviceList::Init(const GUID& category)
{
	m_MonList.clear();

	// Create the System Device Enumerator.
	HRESULT hr;
	CComPtr<ICreateDevEnum> pDevEnum = NULL;
	hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video capture category.
	CComPtr<IEnumMoniker> pEnum;
	hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		CComPtr<IMoniker> pMoniker;
		while(pEnum->Next(1, &pMoniker, 0) == S_OK)
		{
			m_MonList.push_back(pMoniker);
			pMoniker.Release();
		}
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: Release()
// Desc: Empty the list.
//-----------------------------------------------------------------------------

HRESULT CDeviceList::Release()
{
	m_MonList.clear();
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: PopulateList()
// Desc: Populate a Win32 list box control with a list of monikers.
//
// hList: Handle to the list box control's window
//
// Warning: List box must NOT be sorted, otherwise you cannot associate the list 
// box items with the monikers. (A better design would be to sort the monikers.)
//-----------------------------------------------------------------------------

HRESULT CDeviceList::PopulateList(HWND hList)
{
	vector<MonikerPtr>::iterator iter;
	for (iter = m_MonList.begin(); iter != m_MonList.end(); iter++)
	{
        // Bind the moniker to a property bag, which is COM speak for
        // "get the properties for this device"
	    CComPtr<IPropertyBag> pPropBag;
        HRESULT hr = (*iter)->BindToStorage(0, 0, IID_IPropertyBag, 
            (void **)&pPropBag);

		if (FAILED(hr))
		{
			return hr;
		}

		// Find the description or friendly name and add it to the list.
        // The "Description" property is probably more specific but is not
        // generally supported. (Works for some DV camcorders.)

        CComVariant varName;
		varName.vt = VT_BSTR;
		hr = pPropBag->Read(L"Description", &varName, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		}
		if (SUCCEEDED(hr))
		{
			USES_CONVERSION;
			(long)SendMessage(hList, LB_ADDSTRING, 0, 
				(LPARAM)OLE2T(varName.bstrVal));
		}

	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetMoniker()
// Desc: Get the n'th moniker from the list.
//
// index: which moniker to get
// ppMoniker: Receives an IMoniker pointer. Caller must release the interface.
//-----------------------------------------------------------------------------

HRESULT CDeviceList::GetMoniker(unsigned long index, IMoniker **ppMoniker)
{
	if (!ppMoniker) return E_POINTER;

	if (index >= m_MonList.size())
	{
		return E_INVALIDARG;
	}

	*ppMoniker = m_MonList[index];
	(*ppMoniker)->AddRef();
	return S_OK;
}



