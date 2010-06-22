#include "StdAfx.h"
#include "extdevice.h"
#include <XPrtDefs.h>

//-----------------------------------------------------------------------------
// Name: GetModeName()
// Desc: Get names for all the transport modes.
//
// lMode: Which mode. (See IAMExtTransport::put_Mode) 
//-----------------------------------------------------------------------------

const TCHAR* GetModeName(long lMode)
{
	switch (lMode)
	{
	case ED_MODE_PLAY: return TEXT("Playing");
	case ED_MODE_STOP: return TEXT("Stopped");
	case ED_MODE_FREEZE: return TEXT("Paused");
	case ED_MODE_FF: return TEXT("Fast Forward");
	case ED_MODE_REW: return TEXT("Rewind");
	case ED_MODE_RECORD: return TEXT("Recording");
	default: return TEXT("");
	}
}

//-----------------------------------------------------------------------------
// Name: CExtDevice()
// Desc: Constructor
//-----------------------------------------------------------------------------

CExtDevice::CExtDevice()
: m_bVTR(FALSE), m_hThread(0), m_hNotifyEvent(0), m_hwndNotify(0), m_OldState(ED_MODE_STOP)
{
    // Create the thread that will receive device notifications or poll the device
	m_hThreadEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hThreadEndEvent == 0)
	{
		throw new bad_hresult(E_FAIL);
	}

	InitializeCriticalSection(&m_csIssueCmd);
}

//-----------------------------------------------------------------------------
// Name: ~CExtDevice()
// Desc: Destructor
//-----------------------------------------------------------------------------

CExtDevice::~CExtDevice(void)
{
	RemoveDevice();

	CloseHandle(m_hThreadEndEvent);
	DeleteCriticalSection(&m_csIssueCmd);
}


//-----------------------------------------------------------------------------
// Name: InitDevice()
// Desc: Initialize the object.
//
// pFilter: Pointer to the capture filter.
//-----------------------------------------------------------------------------

void CExtDevice::InitDevice(IBaseFilter *pFilter)
{

	m_pDevice.Release();
	m_pTransport.Release();
	m_pTimecode.Release();

    // Query for the external transport interfaces

	CComPtr<IBaseFilter> pF = pFilter;
	pF.QueryInterface(&m_pDevice);

	if (m_pDevice)
	{
		// Don't query for these unless there is an external device.
		pF.QueryInterface(&m_pTransport);
		pF.QueryInterface(&m_pTimecode);

		LONG lDeviceType = 0;
		HRESULT hr = m_pDevice->GetCapability(ED_DEVCAP_DEVICE_TYPE, &lDeviceType, 0);
		if (SUCCEEDED(hr) && (lDeviceType == ED_DEVTYPE_VCR) && (m_pTransport != 0))
		{
			m_bVTR = true;
			StartNotificationThread();  
		}
	}
}


//-----------------------------------------------------------------------------
// Name: RemoveDevice()
// Desc: Release the device from the object, and clean up.
//-----------------------------------------------------------------------------

void CExtDevice::RemoveDevice()
{
	Stop();  // Stop the transport.

    // Shut down the worker thread.
	if (m_hThread) 
	{
		// Signaling this event will cause the thread to end.    
		if (SetEvent(m_hThreadEndEvent)) {
			// Wait for it to end.
			WaitForSingleObjectEx(m_hThread, INFINITE, FALSE);
		}
		CloseHandle(m_hThread);
		m_hThread = 0;
	}

    // Release everything.
	m_pDevice.Release();
	m_pTransport.Release();
	m_pTimecode.Release();
}


//-----------------------------------------------------------------------------
// Name: SetTransportState()
// Desc: Set a new transport state, e.g. pause or rewind.
//
// lMode: Which mode (see IAMExtTransport::put_Mode)
//-----------------------------------------------------------------------------

HRESULT CExtDevice::SetTransportState(long lMode)
{
	HRESULT hr;
	if (HasTransport())
	{
        // We use a critcial section to serialize the put_Mode call, because
        // another thread is updating our UI with the state.
		EnterCriticalSection(&m_csIssueCmd); 
		hr = m_pTransport->put_Mode(lMode);
		LeaveCriticalSection(&m_csIssueCmd); 
		return hr;
	}
	return S_FALSE;
}


//-----------------------------------------------------------------------------
// Name: GetTimecode()
// Desc: Query the device for the current timecode.
//
// pdwHour:  Receives the hours
// pdwMin:   Receives the minures
// pdwSec:   Receives the seconds
// pdwFrame: Receives the frames
//
// Note: The device might report a bogus timecode (e.g. if there is no timecode
// on the tape) ... there's no way to determine this programmatically, because
// the driver only knows what the device tells it.
//-----------------------------------------------------------------------------

HRESULT CExtDevice::GetTimecode(DWORD *pdwHour, DWORD *pdwMin, DWORD *pdwSec, DWORD *pdwFrame)
{
	if (!HasTimecode())
	{
		return E_FAIL;
	}

	if (!(pdwHour && pdwMin && pdwSec && pdwFrame))
	{
		return E_POINTER;
	}

    // Initialize the struct that receives the timecode
    TIMECODE_SAMPLE TimecodeSample;
	ZeroMemory(&TimecodeSample, sizeof(TIMECODE_SAMPLE));
    TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;

	HRESULT hr;
	if (hr = m_pTimecode->GetTimecode(&TimecodeSample),  SUCCEEDED(hr)) 
    {
        // Coerce the BCD value to our own timecode struct, in order to unpack the value.

        DV_TIMECODE *pTimecode = (DV_TIMECODE*)(&(TimecodeSample.timecode.dwFrames));

        *pdwHour = pTimecode->Hours10 * 10 + pTimecode->Hours1;
        *pdwMin = pTimecode->Minutes10 * 10 + pTimecode->Minutes1;
        *pdwSec = pTimecode->Seconds10 * 10 + pTimecode->Seconds1;
        *pdwFrame = pTimecode->Frames10 * 10 + pTimecode->Frames1;

    }

	return hr;
}

//-----------------------------------------------------------------------------
// Name: GetATN()
// Desc: Query the device for the current track number.
//
// pdwATN: Receives the track number
//-----------------------------------------------------------------------------

HRESULT CExtDevice::GetATN(DWORD *pdwATN)
{
	if (!HasTimecode())
	{
		return E_FAIL;
	}

	if (!pdwATN)
	{
		return E_POINTER;
	}

    TIMECODE_SAMPLE TimecodeSample;
	ZeroMemory(&TimecodeSample, sizeof(TIMECODE_SAMPLE));
    TimecodeSample.dwFlags = ED_DEVCAP_ATN_READ;

	HRESULT hr;
	if (hr = m_pTimecode->GetTimecode(&TimecodeSample),  SUCCEEDED(hr)) 
    {
        *pdwATN = TimecodeSample.timecode.dwFrames;
    }

	return hr;
}


//-----------------------------------------------------------------------------
// Name: StartNotificationThread()
// Desc: Start the thread that notifies the client of the device state.
//
// The thread itself is defined in ThreadProc
//-----------------------------------------------------------------------------

HRESULT CExtDevice::StartNotificationThread()
{
	if (m_pTransport == 0)
	{
		return E_FAIL;
	}

    // Pass a pointer to the CExtDevice object as the thread parameter.
	m_hThread = CreateThread(NULL, 0, CExtDevice::ThreadProc, this, 0, &m_ThreadId);
	return (m_hThread == NULL ? E_FAIL : S_OK);
}


//-----------------------------------------------------------------------------
// Name: ThreadProc()
// Desc: Thread that notifies the client of the device state. 
//
// The thread just calls DoNotifyLoop and waits for the loop to terminate.
//-----------------------------------------------------------------------------

DWORD WINAPI CExtDevice::ThreadProc(void *pParam)
{
	if (!pParam) return 0;

	CExtDevice* pDevice = (CExtDevice*)pParam;
	pDevice->DoNotifyLoop();


    return 1; 
}



//-----------------------------------------------------------------------------
// Name: DoNotifyLoop()
// Desc: Main loop for the device notification thread. 
//
// Some devices support asynchronous notification of device state changes. If
// so, we can wait on an event which is signaled by the driver. Otherwise, we
// have to poll the device at intervals.
//
// WARNING: I have not tested the notification code path lately, because our Sony
// camcorder does not support it. (I did test it awhile ago on another device.)
//-----------------------------------------------------------------------------

void CExtDevice::DoNotifyLoop()
{
	OutputDebugString("Entering DoNotifyLoop\n");

	_ASSERTE(m_pTransport != NULL);

    HRESULT hr;
    HANDLE  EventHandles[2];
    DWORD   WaitStatus;
    LONG    State;

    // If the device supports notification, there are two events we wait on: The notification
    // event, which is provided by the driver, and the "thread end" event, which the client
    // can signal to exit this loop.

    // Get the notification event, which is signaled when the next operation completes.   
    hr = m_pTransport->GetStatus(ED_NOTIFY_HEVENT_GET, (long *) &m_hNotifyEvent);

    while (m_hThread && m_hNotifyEvent && m_hThreadEndEvent) 
	{
        EnterCriticalSection(&m_csIssueCmd);  
        State = 0;
        hr = m_pTransport->GetStatus(ED_MODE_CHANGE_NOTIFY, &State);
        LeaveCriticalSection(&m_csIssueCmd); 

		if (hr == MAKE_HRESULT(1, FACILITY_WIN32, ERROR_NOT_SUPPORTED))
		{
			OutputDebugString(TEXT("Device does not support event notification\n"));
		}
        if (hr == E_PENDING) 
		{             
			// If the previous call to GetStatus return E_PENDING, it means the 
            // device supports notification.

            EventHandles[0] = m_hNotifyEvent;
            EventHandles[1] = m_hThreadEndEvent;
            WaitStatus = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
            if (WAIT_OBJECT_0 == WaitStatus)
			{
				OutputDebugString("Device changed state.\n");
                NotifyState(State);
            } 
            else 
			{
                break;  // End this thread.
            }
        } 
        else 
		{          
			// Otherwise, the device does not support notification, so we poll the device instead.
			PollDevice();
			break;
		} 

    } // while

    // Cancel notification. This tells the driver that we don't need m_hNotifyEvent any more.
    hr = m_pTransport->GetStatus(ED_NOTIFY_HEVENT_RELEASE, (long *) &m_hNotifyEvent);
}


//-----------------------------------------------------------------------------
// Name: PollDevice()
// Desc: Loop for polling the device.
//-----------------------------------------------------------------------------

void CExtDevice::PollDevice()
{
    // This loop polls the device for it's current state at fixed intervals.
    // We only have to wait on one event, the "thread end" event.

	while (m_hThread && m_hThreadEndEvent) 
	{
		EnterCriticalSection(&m_csIssueCmd);  
		long State = 0;
		HRESULT hr = m_pTransport->get_Mode(&State);
		LeaveCriticalSection(&m_csIssueCmd);

		if (State != m_OldState)
		{
			NotifyState(State);
			m_OldState = State;
		}

		// Wait for a while, or until the thread ends. 
		DWORD WaitStatus = WaitForSingleObjectEx(m_hThreadEndEvent, 200, FALSE); 
		if (WaitStatus == WAIT_OBJECT_0)
			break; // Exit thread now. 
		// Otherwise, the wait timed out. Time to poll again.
	}
}



