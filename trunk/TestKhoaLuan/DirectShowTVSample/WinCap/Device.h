#pragma once

typedef CComPtr<IMoniker> MonikerPtr;

/******************************************************************************
 *  CDeviceList Class
 *  Manages a list of device monikers. Each moniker is an IMoniker pointer.
 *****************************************************************************/

class CDeviceList
{
private:
	vector<MonikerPtr> m_MonList;

public:
	HRESULT Init(const GUID& category); 
	HRESULT Release();
	HRESULT PopulateList(HWND hList);
	HRESULT GetMoniker(unsigned long index, IMoniker **ppMoniker);
	long	Count() { return (long)m_MonList.size(); }

};

