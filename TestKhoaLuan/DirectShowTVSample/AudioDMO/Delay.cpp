// Delay.cpp : Implementation of CDelay
#include "stdafx.h"

#include "Delay.h"

#include <uuids.h> // Media types



// Forward Declares
#ifdef _DEBUG
void DumpWaveformat(WAVEFORMATEX *pWave);
#endif

inline double RefTime2Double(REFERENCE_TIME rt) {
	return (double)rt / UNITS;
}


/////////////////////////////////////////////////////////////////////////////
// CDelay


HRESULT CDelay::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
    *pdwFlags = DMO_INPUT_STREAMF_WHOLE_SAMPLES | 
                DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE;
    return S_OK;
}


HRESULT CDelay::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
    *pdwFlags = DMO_OUTPUT_STREAMF_WHOLE_SAMPLES | 
                DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE ;
    return S_OK;
}



HRESULT CDelay::InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{    
    // If our input type is already set, reject format changes
    if (InputTypeSet(dwInputStreamIndex) && !TypesMatch(pmt, InputType(dwInputStreamIndex)))
    {
        return DMO_E_INVALIDTYPE;
    }
    
    // If our output type is already set, the input type must match
    else if (OutputTypeSet(dwInputStreamIndex) && !TypesMatch(pmt, OutputType(dwInputStreamIndex)))
    {
        return DMO_E_INVALIDTYPE;
    }
    
    // If no types are set yet, validate the format 
    else 
    {
        return CheckPcmFormat(pmt);
    }

}



HRESULT CDelay::InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
    // If our output type is already set, reject format changes
    if (OutputTypeSet(dwOutputStreamIndex) && !TypesMatch(pmt, OutputType(dwOutputStreamIndex)))
    {
        return DMO_E_INVALIDTYPE;
    }
    
    // If our input type is already set, the output type must match
    else if (InputTypeSet(dwOutputStreamIndex) && !TypesMatch(pmt, InputType(dwOutputStreamIndex)))
    {
        return DMO_E_INVALIDTYPE;
    }
    
    // If no types are set yet, validate the format 
    else 
    {
        return CheckPcmFormat(pmt);
    }
    
}



HRESULT CDelay::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, 
                                          DMO_MEDIA_TYPE *pmt)
{
    if (dwTypeIndex != 0)
    {
        return DMO_E_NO_MORE_ITEMS;
    }

    // if pmt is NULL, we just return S_OK if the type index is in range
    if (pmt == NULL)
    {
        return S_OK;
    }

    if (OutputTypeSet(0))   // If the input type is set, we prefer that one
    {
        
        return MoCopyMediaType(pmt, OutputType(0));
    }

    else {
    // if output type is not set, propose something we like
        return GetPcmType(pmt);
    }
}


HRESULT CDelay::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex,
                                           DMO_MEDIA_TYPE *pmt)
{
    if (dwTypeIndex != 0)
    {
        return DMO_E_NO_MORE_ITEMS;
    }

    // if pmt is NULL, we just return S_OK if the type index is in range
    if (pmt == NULL)
    {
        return S_OK;
    }

    if (InputTypeSet(0))   // If the input type is set, we prefer that one
    {

        return MoCopyMediaType(pmt, InputType(0));
    }
    else {
        
	    // input type is not set, propose something we like
        return GetPcmType(pmt);
    }
}



HRESULT CDelay::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize,
                                              DWORD *pcbMaxLookahead, DWORD *pcbAlignment)
{

    // IMediaObjectImpl validates this for us... 
    _ASSERTE(InputTypeSet(dwInputStreamIndex));

    // And we expect only PCM audio types.
    _ASSERTE(InputType(dwInputStreamIndex)->formattype == FORMAT_WaveFormatEx);
    
    WAVEFORMATEX *pWave = (WAVEFORMATEX*)InputType(dwInputStreamIndex)->pbFormat;
    
    *pcbSize = pWave->nBlockAlign;
    *pcbMaxLookahead = 0;
    *pcbAlignment = 1;
    
    return S_OK; 
}


HRESULT CDelay::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize,
                                               DWORD *pcbAlignment)
{
     // IMediaObjectImpl validates this for us... 
    _ASSERTE(OutputTypeSet(dwOutputStreamIndex));

    // And we expect only PCM audio types.
   _ASSERTE(OutputType(dwOutputStreamIndex)->formattype == FORMAT_WaveFormatEx);
    
    WAVEFORMATEX *pWave = (WAVEFORMATEX*)OutputType(dwOutputStreamIndex)->pbFormat;
    
    *pcbSize = pWave->nBlockAlign;
    *pcbAlignment = 1;
    
    return S_OK;  
}

HRESULT CDelay::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
    return E_NOTIMPL;  
}

HRESULT CDelay::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
    return E_NOTIMPL;  
}


HRESULT CDelay::InternalFlush()
{
    AtlTrace("InternalFlush()\n");

	m_pBuffer = NULL;

    if (m_pbDelayBuffer)
    {
		FillBufferWithSilence();
    }

    return S_OK; 
}

HRESULT CDelay::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
	AtlTrace("InternalDiscontinuity()\n");
    return S_OK;
}

HRESULT CDelay::InternalAllocateStreamingResources()
{
	AtlTrace("InternalAllocateStreamingResources()\n");
    
	_ASSERTE(InputType(0)->formattype == FORMAT_WaveFormatEx);
    _ASSERTE(m_dwDelay > 0);

    m_pWave = (WAVEFORMATEX*)InputType(0)->pbFormat;

    // Allocate the buffer that holds the delayed samples   
	m_cbDelayBuffer = (m_dwDelay * m_pWave->nSamplesPerSec * m_pWave->nBlockAlign) / 1000;
	m_pbDelayBuffer = (BYTE*)CoTaskMemAlloc(m_cbDelayBuffer);
	
	if (m_pbDelayBuffer == NULL)
    {
		return E_OUTOFMEMORY;
    }
	
	FillBufferWithSilence();	
	
	m_pbDelayPtr = m_pbDelayBuffer;

	AtlTrace("\tAllocated %d byte buffer.\n", m_cbDelayBuffer);
	DumpWaveformat(m_pWave);	

    return S_OK;
}

HRESULT CDelay::InternalFreeStreamingResources()
{
	AtlTrace("InternalFreeStreamingResources()\n");

    if (m_pbDelayBuffer)
    {
        CoTaskMemFree(m_pbDelayBuffer);
        m_pbDelayBuffer = m_pbDelayPtr = NULL;
    }

    m_pWave = NULL;

    return S_OK;  
}


HRESULT CDelay::InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,
                                          DWORD dwFlags, REFERENCE_TIME rtTimestamp,
                                          REFERENCE_TIME rtTimelength)
{
    _ASSERTE(m_pBuffer == NULL);

    HRESULT hr = pBuffer->GetBufferAndLength(&m_pbInputData, &m_cbInputLength);
    if (FAILED(hr))
    {
        return hr;
    }


	ATLTRACE2(atlTraceGeneral, 3, "Process Input: %d bytes. Time stamp: %f/%f\n", 
		m_cbInputLength, RefTime2Double(rtTimestamp), RefTime2Double(rtTimelength));

    if (m_cbInputLength <= 0) 
        return E_FAIL;
    
    m_pBuffer = pBuffer;
    
    if (dwFlags & DMO_INPUT_DATA_BUFFERF_TIME)
    {
        m_bValidTime = true;
        m_rtTimestamp = rtTimestamp;
    }
    else
    {
        m_bValidTime = false;
    }
    
    return S_OK;
}



HRESULT CDelay::InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,
                                           DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                                           DWORD *pdwStatus)
{
    BYTE    *pbData;
    DWORD   cbData;
    DWORD   cbOutputLength;
    DWORD   cbBytesProcessed;

    CComPtr<IMediaBuffer> pOutputBuffer = pOutputBuffers[0].pBuffer;

    if (!m_pBuffer || !pOutputBuffer)
    {
        return S_FALSE;  // Did not produce output
    }


    // Get the size of our output buffer
    HRESULT hr = pOutputBuffer->GetBufferAndLength(&pbData, &cbData);
    
    hr = pOutputBuffer->GetMaxLength(&cbOutputLength);
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Skip past any valid data in the output buffer
    pbData += cbData;
    cbOutputLength -= cbData;

    if (cbOutputLength < m_pWave->nBlockAlign)
    {
        return E_FAIL;
    }
    
    // Calculate how many quanta we can process
    bool    bComplete = false;

    if (m_cbInputLength > cbOutputLength)
    {
        cbBytesProcessed = cbOutputLength;
    }
    else
    {
        cbBytesProcessed = m_cbInputLength;
        bComplete = true;
    }

    DWORD dwQuanta = cbBytesProcessed / m_pWave->nBlockAlign;

    // The actual data we write may be less than the available buffer length 
    // due to the block alignment.
    cbBytesProcessed = dwQuanta * m_pWave->nBlockAlign;
    
    hr = DoProcessOutput(pbData, m_pbInputData, dwQuanta);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = pOutputBuffer->SetLength(cbBytesProcessed + cbData);

	ATLTRACE2(atlTraceGeneral, 3, "Process Output: %d bytes.\n", cbBytesProcessed);

    if (m_bValidTime)
    {
        pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIME;
        pOutputBuffers[0].rtTimestamp = m_rtTimestamp;
        
        // Etimate how far along we are...
        pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH;
        pOutputBuffers[0].rtTimelength = (cbBytesProcessed / m_pWave->nAvgBytesPerSec) * UNITS;

		ATLTRACE2(atlTraceGeneral, 3, "\tTimestamp: %f/%f\n", cbBytesProcessed,
				RefTime2Double(pOutputBuffers[0].rtTimestamp), 
				RefTime2Double(pOutputBuffers[0].rtTimelength));

    }

    if (bComplete)
    {
        m_pBuffer = NULL;   // Release input buffer

		ATLTRACE2(atlTraceGeneral, 3, "\tProcessOutput Complete\n");
	}
    else 
    {
        pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
        m_cbInputLength -= cbBytesProcessed;
        m_pbInputData += cbBytesProcessed;
        m_rtTimestamp += pOutputBuffers[0].rtTimelength;

		ATLTRACE2(atlTraceGeneral, 3, "\tBytes remaining:\n", m_cbInputLength);
    }

    return S_OK;
}


HRESULT CDelay::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
    return (m_pBuffer ? S_FALSE : S_OK);
}


//////////////////////////////////////
//   In-place methods
/////////////////////////////////////



STDMETHODIMP CDelay::Process(ULONG ulSize, BYTE *pData, REFERENCE_TIME refTimeStart, DWORD dwFlags)
{
    if (dwFlags &= ~DMO_INPLACE_ZERO)
        return E_INVALIDARG;

    if (!pData) 
    {
        return E_POINTER;
    }


	ATLTRACE2(atlTraceGeneral, 3, "Process: %d bytes. Time stamp: %f\n", 
		ulSize, RefTime2Double(refTimeStart));



    LockIt lock(this);
    
    if (!InputTypeSet(0) || !OutputTypeSet(0)) 
    {
        return DMO_E_TYPE_NOT_SET;
    }
    
    //  Make sure all streams have media types set and resources are allocated
    HRESULT hr = AllocateStreamingResources();
    
	if (SUCCEEDED(hr))
		hr = DoProcessOutput(pData, pData, ulSize / m_pWave->nBlockAlign);

	return hr;

    // If this DMO supported an effect tail, it would return S_FALSE until 
    // the tail was processed. See IMediaObjectInPlace::Process documentation.
}


STDMETHODIMP CDelay::Clone(IMediaObjectInPlace **ppMediaObject)
{
    HRESULT hr;


    if (!ppMediaObject)
    {
        return E_POINTER;
    }

    *ppMediaObject = NULL;

    // Make a new one
    CDelay *pTemp = new CComObject<CDelay>;

    if (!pTemp)
    {
        return E_OUTOFMEMORY;
    }
    
    // Set the media types
    CComQIPtr<IMediaObject, &IID_IMediaObject> pMediaObj(pTemp);
    _ASSERTE(pMediaObj != NULL);

    if (InputTypeSet(0))
    {
        hr = pMediaObj->SetInputType(0, InputType(0), 0);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (OutputTypeSet(0))
    {
        hr = pMediaObj->SetOutputType(0, OutputType(0), 0);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Everything is OK, return the AddRef'd pointer.
    return pTemp->QueryInterface(IID_IMediaObjectInPlace, (void**)ppMediaObject);
}


STDMETHODIMP CDelay::GetLatency(REFERENCE_TIME *pLatencyTime)
{
    if (pLatencyTime == NULL)
        return E_POINTER;

    *pLatencyTime = 0;
    return S_OK;
}


// Our private methods
HRESULT CDelay::DoProcessOutput(BYTE *pbData, const BYTE *pbInputData, DWORD dwQuanta)
{
    _ASSERTE(m_pbDelayBuffer);

    DWORD sample, channel, num_channels;
	
	num_channels = m_pWave->nChannels;

	if (Is8Bit())
	{
		for (sample = 0; sample < dwQuanta; ++sample)
		{
			for (channel = 0; channel < num_channels; ++channel)
			{
                // 8-bit sound is 0..255 with 128 == silence
				
                // Get the input sample and normalize to -128 .. 127
                int i = pbInputData[sample * num_channels + channel] - 128;
				
                // Get the delay sample and normalize to -128 .. 127
                int delay = m_pbDelayPtr[0] - 128;
				
                m_pbDelayPtr[0] = i + 128;
                IncrementDelayPtr(sizeof(unsigned char));
				
                i = (i * (100 - m_nWet)) / 100 + (delay * m_nWet) / 100;
				
                // Truncate
                if (i > 127)
                    i = 127;
                if (i < -128)
                    i = -128;
				
                pbData[sample * num_channels + channel] = (unsigned char)(i+128);
				
            }
		}
	}
	else  // 16-bit
	{
		for (sample = 0; sample < dwQuanta; ++sample)
		{
			for (channel = 0; channel < num_channels; ++channel)
			{
                int i = ((short*)pbInputData)[sample * num_channels + channel];
				
                int delay = ((short*)m_pbDelayPtr)[0];
				
                ((short*)m_pbDelayPtr)[0] = i;
                IncrementDelayPtr(sizeof(short));
				
                i = (i * (100 - m_nWet)) / 100 + (delay * m_nWet) / 100;
				
                // Truncate
                if (i > 32767)
                    i = 32767;
                if (i < -32768)
                    i = -32768;
				
                ((short*)pbData)[sample * num_channels + channel] = (short)i;
				
            }
        }
    }
    return S_OK;
}




// TypesMatch: Return true if all the required fields match

bool CDelay::TypesMatch(const DMO_MEDIA_TYPE *pmt1, const DMO_MEDIA_TYPE *pmt2)
{
    if (pmt1->majortype     == pmt2->majortype      &&
        pmt1->subtype       == pmt2->subtype        &&
        pmt1->lSampleSize   == pmt2->lSampleSize    &&
        pmt1->formattype    == pmt2->formattype     &&
        pmt1->cbFormat      == pmt1->cbFormat       &&
        memcmp(pmt1->pbFormat, pmt2->pbFormat, pmt1->cbFormat) == 0)
    {
        return true;
    }
    else
    {   
        return false;
    }
}


// CheckPcmFormat: Return S_OK if pmt is a valid PCM audio type.

HRESULT CDelay::CheckPcmFormat(const DMO_MEDIA_TYPE *pmt)
{
    if (pmt->majortype      == MEDIATYPE_Audio      &&
        pmt->subtype        == MEDIASUBTYPE_PCM     &&
        pmt->formattype     == FORMAT_WaveFormatEx  &&
        pmt->cbFormat       == sizeof(WAVEFORMATEX) &&
        pmt->pbFormat != NULL)
    {
        // Check the format block
        
        WAVEFORMATEX *pWave = (WAVEFORMATEX*)pmt->pbFormat;
        
        if ((pWave->wFormatTag == WAVE_FORMAT_PCM) &&
            (pWave->wBitsPerSample == 8 || pWave->wBitsPerSample == 16) &&
            (pWave->nBlockAlign == pWave->nChannels * pWave->wBitsPerSample / 8) &&
            (pWave->nAvgBytesPerSec == pWave->nSamplesPerSec * pWave->nBlockAlign))
        {
            return S_OK;
        }
    }
    return DMO_E_INVALIDTYPE;
}



// Return our only preferred type.
HRESULT CDelay::GetPcmType(DMO_MEDIA_TYPE *pmt)
{
    // Accepts anything, but proposes 44.1 kHz, 16-bit, 2-channel
    HRESULT hr = MoInitMediaType(pmt, sizeof(WAVEFORMATEX));

    // Can fail to allocate the format block...
    if (FAILED(hr))
    {
        return hr;
    }

    pmt->majortype = MEDIATYPE_Audio;
    pmt->subtype = MEDIASUBTYPE_PCM;
    pmt->formattype = FORMAT_WaveFormatEx;

    
    WAVEFORMATEX *pWave = (WAVEFORMATEX*)(pmt->pbFormat);
    
    pWave->wFormatTag = WAVE_FORMAT_PCM;
    pWave->nChannels = 2;
    pWave->nSamplesPerSec = 44100;
    pWave->wBitsPerSample = 16;
    pWave->nBlockAlign = (pWave->nChannels * pWave->wBitsPerSample) / 8;
    pWave->nAvgBytesPerSec = pWave->nSamplesPerSec * pWave->nBlockAlign;
    pWave->cbSize = 0;

    return S_OK;
}


// Fill delay buffer with silence. (definition of 'silence' depends on the format)
void CDelay::FillBufferWithSilence()
{
	if (Is8Bit())
		FillMemory(m_pbDelayBuffer, m_cbDelayBuffer, 0x80);
	else
		ZeroMemory(m_pbDelayBuffer, m_cbDelayBuffer);
}


#ifdef _DEBUG
void DumpWaveformat(WAVEFORMATEX *pWave)
{
	AtlTrace("WAVEFORMATEX Structure:\n");
	AtlTrace("\tChannels: %d\n", pWave->nChannels);
	AtlTrace("\tSamples/Sec: %f kHz\n", (double)pWave->nSamplesPerSec / 1000);
	AtlTrace("\tBits/Sample: %d\n", pWave->wBitsPerSample);
}
#else
#define DumpWaveformat(x) 
#endif

