	
// Delay.h : Declaration of the CDelay class.


// Link with the following lib files:
//
//		msdmo.lib     -  DMO helper functions
//		dmoguids.lib  -  IIDs and DMO category GUIDs
//		strmiids.lib  -  Media type GUIDs

#ifndef __DELAY_H_
#define __DELAY_H_

#include "resource.h"       // main symbols

#define FIX_LOCK_NAME
#include <dmo.h>

#include <dmoimpl.h>



DEFINE_GUID(CLSID_Delay,
			0xAD90F22E, 0xC51D, 0x4F1C, 0xB4, 0x40, 0xC1, 0xF2, 0xA0, 0x1D, 0x5F, 0x1F);


const DWORD UNITS = 10000000;  // 1 sec = 1 * UNITS
const long DEFAULT_WET_DRY_MIX = 25;
const long DEFAULT_DELAY = 2000; // in ms


/////////////////////////////////////////////////////////////////////////////
// CDelay
class ATL_NO_VTABLE CDelay : 
	public IMediaObjectImpl<CDelay, 1, 1>,
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDelay, &CLSID_Delay>, // DMO Template (1 input stream & 1 output stream)
	public IMediaObjectInPlace
{
public:
	CDelay() :
  	  m_nWet(DEFAULT_WET_DRY_MIX),
	  m_dwDelay(DEFAULT_DELAY)
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DELAY)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDelay)
	COM_INTERFACE_ENTRY(IMediaObject)
	COM_INTERFACE_ENTRY(IMediaObjectInPlace)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		FreeStreamingResources();  // In case client does not call this.
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

public:
	
	// Declare internal methods required by IMediaObjectImpl
	HRESULT InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
	HRESULT InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	HRESULT InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt);
	HRESULT InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pcbMaxLookahead, DWORD *pcbAlignment);
	HRESULT InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pcbAlignment);
	HRESULT InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	HRESULT InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
	HRESULT InternalFlush();
	HRESULT InternalDiscontinuity(DWORD dwInputStreamIndex);
	HRESULT InternalAllocateStreamingResources();
	HRESULT InternalFreeStreamingResources();

	HRESULT InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,
		DWORD dwFlags, REFERENCE_TIME rtTimestamp,
		REFERENCE_TIME rtTimelength);
	
	HRESULT InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,
		DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
		DWORD *pdwStatus);
	
	HRESULT InternalAcceptingInput(DWORD dwInputStreamIndex);

	/// IMediaObjectInPlace methods
	STDMETHOD(Process)(ULONG ulSize, BYTE *pData, REFERENCE_TIME refTimeStart, DWORD dwFlags);
	STDMETHOD(Clone)(IMediaObjectInPlace **ppMediaObject);
	STDMETHOD(GetLatency)(REFERENCE_TIME *pLatencyTime);
	
	
private:


	// TypesMatch: Return true if all the required fields match
	bool	TypesMatch(const DMO_MEDIA_TYPE *pmt1, const DMO_MEDIA_TYPE *pmt2);
	
	// CheckPcmFormat: Return S_OK if pmt is a valid PCM audio type.
	HRESULT CheckPcmFormat(const DMO_MEDIA_TYPE *pmt);

	// DoProcessOutput: Process data
    HRESULT DoProcessOutput(
                BYTE *pbData,            // Pointer to the output buffer
                const BYTE *pbInputData, // Pointer to the input buffer
                DWORD dwQuantaToProcess  // Number of quanta to process
                );

	void FillBufferWithSilence(void);

	// GetPcmType: Get our only preferred PCM type
	//             
	HRESULT GetPcmType(DMO_MEDIA_TYPE *pmt);


	// Members 

	CComPtr<IMediaBuffer>	m_pBuffer;			// Pointer to the current input buffer

	BYTE					*m_pbInputData;		// Pointer to the data in the input buffer
	DWORD					m_cbInputLength;	// Length of the data

	REFERENCE_TIME			m_rtTimestamp;		// Most recent timestamp
	bool					m_bValidTime;		// Is timestamp valid?

	WAVEFORMATEX			*m_pWave;			// Pointer to the WAVEFORMATEX struct


	BYTE					*m_pbDelayBuffer;	// circular buffer for delay samples
	DWORD					m_cbDelayBuffer;	// size of the delay buffer
	BYTE					*m_pbDelayPtr;		// ptr to next delay sample

	long					m_nWet;			// Wet portion of wet/dry mix
	DWORD					m_dwDelay;		// Delay in ms


	bool  Is8Bit()			{ return (m_pWave->wBitsPerSample == 8); }

	// Moves the delay pointer around the circular buffer
	void IncrementDelayPtr(size_t size) 
	{
		m_pbDelayPtr += size;
		if (m_pbDelayPtr + size > m_pbDelayBuffer + m_cbDelayBuffer)
		{
			m_pbDelayPtr = m_pbDelayBuffer;
		}
	}
	
	

};

#endif //__DELAY_H_
