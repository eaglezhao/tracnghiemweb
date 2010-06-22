#pragma once

#include "graph.h"

#define WM_MY_DEVICE_NOTIFY (WM_APP + 0x10)  // Private message for device state changes

const TCHAR* GetModeName(long lMode);  // Find the name of a transport mode (stop, pause, etc)

/******************************************************************************
 *  CExtDevice Class
 *  Manages camcorder (device control) interfaces
 *****************************************************************************/

class CExtDevice : public CDeviceHelper
{
private:
	CComPtr<IAMExtDevice> m_pDevice;
	CComPtr<IAMExtTransport> m_pTransport;
	CComPtr<IAMTimecodeReader> m_pTimecode;

	BOOL m_bVTR;  // Is the device in VTR mode?

	CRITICAL_SECTION m_csIssueCmd;  // Protects the device state during state transitions
	HANDLE           m_hThreadEndEvent;  // Signal this event to end the notification thread
	HANDLE			 m_hNotifyEvent;     // This event is signalled when the state changes
	HANDLE           m_hThread;          // Thread to get device notifications, or to poll the device
	DWORD			 m_ThreadId;         // Thread ID.

    long             m_OldState;         // Caches the old transport state when we poll the device

	static DWORD WINAPI ThreadProc(void *pParam);  // Thread window proc

	HRESULT CExtDevice::StartNotificationThread();
	void    DoNotifyLoop();
	void	PollDevice();

    // DV_TIMECODE struct - used to unpack BCD timecode. Each BCD digit is 4 bytes from
    // the DWORD timecode value. Windows is Little Endian so the digits are declared in
    // "reverse" order.
    typedef struct {
        ULONG Frames1   :4; 
        ULONG Frames10  :4; 
        ULONG Seconds1  :4; 
        ULONG Seconds10 :4; 
        ULONG Minutes1  :4; 
        ULONG Minutes10 :4; 
        ULONG Hours1    :4; 
        ULONG Hours10   :4; 
    } DV_TIMECODE;


protected:

	HRESULT SetTransportState(long lMode);

	HWND m_hwndNotify;  // Window to receive device notifications

	void NotifyState(long lState)
	{
		if (m_hwndNotify)
		{
			PostMessage(m_hwndNotify, WM_MY_DEVICE_NOTIFY, (WPARAM)lState, 0);
		}
	}


public:
	CExtDevice();
	virtual ~CExtDevice(void);

	void InitDevice(IBaseFilter *pFilter);
	void RemoveDevice();

	BOOL HasDevice() { return (m_pDevice != NULL); }
	BOOL HasTransport() { return m_pTransport && m_bVTR; }
	BOOL HasTimecode() { return (m_pTimecode != NULL); }

	HRESULT GetTimecode(DWORD *pdwHour, DWORD *pdwMin, DWORD *pdwSec, DWORD *pdwFrame);
	HRESULT GetATN(DWORD *pdwATN);

	// Device control (transport state)
	HRESULT Play() { return SetTransportState(ED_MODE_PLAY); }
	HRESULT Stop() { return SetTransportState(ED_MODE_STOP); }
	HRESULT Record() { return SetTransportState(ED_MODE_RECORD); }
	HRESULT Rewind() { return SetTransportState(ED_MODE_REW ); }
	HRESULT FastForward() { return SetTransportState(ED_MODE_FF); }

};
