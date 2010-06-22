#pragma once

#include "dialog.h"
#include "ExtDevice.h"
#include "resource.h"

/******************************************************************************
 *  CTimer Class
 *  Implements a simple Win32 timer.
 *****************************************************************************/

class CTimer
{
private:
	UINT_PTR m_ID;   // Timer id
	HWND m_hwnd;     // Window to receive the messages

public:

	CTimer() : m_ID(0) {}
	~CTimer() { Stop(); }

    // Start the timer
	BOOL Start(HWND hwnd, UINT msTimeout)
	{
        Stop();
		m_hwnd = hwnd;
		m_ID = SetTimer(hwnd, 0, msTimeout, 0);
		return (m_ID == 0 ? FALSE : TRUE);
	}

    // Stop the timer
	void Stop()
	{
		if (m_ID)
		{
			KillTimer(m_hwnd, m_ID);
		}
	}
};


/******************************************************************************
 *  CDVProp Class
 *  Shows a non-modal dialog for controlling a DV camcorder
 *****************************************************************************/

class CDVProp : public CNonModalDialog, public CExtDevice
{

private:
	void SetInitialButtonState();
	void EnableVtrButtons(BOOL bEnable);

	void OnTransmit();
	void OnDeviceNotify(LONG State);
	void DisplayTimecode();
    void OnVtrPlay();
    void OnVtrStop();

	CTimer	m_Timer;  // Timer for updating the timecode

	bool m_bTransmit;

protected:
	HRESULT OnInitDialog();
	INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    void OnEndDialog() 
    {
        SetGraph(0);
        RemoveDevice();
    }



public:
	CDVProp() : 
	  CNonModalDialog(IDD_DV_PROP),
	  m_bTransmit(false)
	{ }

	BOOL OnOK();
};