#include "stdafx.h"
#include "CDVProp.h"


//-----------------------------------------------------------------------------
// Name: OnReceiveMsg()
// Desc: Called whenever the dialog window receives a message.
//-----------------------------------------------------------------------------

CDVProp::OnReceiveMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MY_DEVICE_NOTIFY:
		OnDeviceNotify((LONG)wParam);
		break;

	case WM_TIMER:
		DisplayTimecode();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_VTR_PLAY:
            OnVtrPlay();
			break;
		case IDC_VTR_STOP:
            OnVtrStop();
			break;
		case IDC_VTR_TRANSMIT:
			OnTransmit();
			break;
		case IDC_VTR_REW:
			Rewind();
			break;
		case IDC_VTR_FF:
			FastForward();
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Called when the dialog is initialized.
//-----------------------------------------------------------------------------

HRESULT CDVProp::OnInitDialog()
{
    // Set the bitmaps on the buttons

    SetWindowBitmap(IDC_VTR_PLAY, IDB_PLAY);
    SetWindowBitmap(IDC_VTR_STOP, IDB_STOP);
    SetWindowBitmap(IDC_VTR_TRANSMIT, IDB_TRANSMIT);
    SetWindowBitmap(IDC_VTR_REW, IDB_VTR_REW);
    SetWindowBitmap(IDC_VTR_FF, IDB_VTR_FF);

    // Enable the VTR buttons if the device has a VTR transport.
	if (HasTransport())
	{
		EnableVtrButtons(TRUE);
	}
	else
	{
		EnableVtrButtons(FALSE);
	}

    // If the device can read timecode, start the timer that updates our UI
	if (HasTimecode())
	{
		m_Timer.Start(m_hDlg, 200);
	}
	else
	{
		m_Timer.Stop();
	}

	m_hwndNotify = m_hDlg; // This is the window that receives device notifications
	return S_OK;
};


//-----------------------------------------------------------------------------
// Name: OnOK()
// Desc: Called when the user clicks OK.
//-----------------------------------------------------------------------------

BOOL CDVProp::OnOK()
{
	m_Timer.Stop();
	return CNonModalDialog::OnOK();
}


//-----------------------------------------------------------------------------
// Name: OnVtrPlay()
// Desc: Play the VTR transport
//-----------------------------------------------------------------------------

void CDVProp::OnVtrPlay()
{
    if (m_pGraph)
    {
        // If previously we were transmitting from file to tape, it's time to rebuild the graph.
        if (m_bTransmit)  
        {
            m_pGraph->TearDownGraph();
            m_pGraph->RenderPreview();
            m_bTransmit = false;
        }
        m_pGraph->Run();
    }
    Play();

}

//-----------------------------------------------------------------------------
// Name: OnVtrStop()
// Desc: Stop the VTR transport
//-----------------------------------------------------------------------------

void CDVProp::OnVtrStop()
{
    if (m_pGraph)
    {
        m_pGraph->Stop();
    }
    Stop();
}

//-----------------------------------------------------------------------------
// Name: OnTransmit()
// Desc: Transmit from file to tape.
//-----------------------------------------------------------------------------

void CDVProp::OnTransmit()
{
	_ASSERTE(HasDevice());

    // Get a file name.

	TCHAR szFileName[MAX_PATH + 1];
    szFileName[0] = '\0';

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hDlg;
	ofn.lpstrFilter = TEXT("Avi\0*.avi\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH + 1;
	ofn.lpstrTitle = TEXT("Select a DV File to Transmit");
	ofn.Flags = OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		return;
	}

    // Try to build a transmit graph.

	HRESULT hr = m_pGraph->RenderTransmit(szFileName, (int)_tcslen(szFileName) + 1);

	if (SUCCEEDED(hr))
	{
		m_bTransmit = true;

        // Start recording to tape. There's a delay, so don't run the graph yet.
		// We'll be notified when it starts recording and start the graph then.

        hr = Record();
	}
	if (FAILED(hr))
	{
		ReportError(0, TEXT("Cannot run transmit graph"));
	}
}

//-----------------------------------------------------------------------------
// Name: OnDeviceNotify()
// Desc: Called when the transport changes state.
//
// The notification thread posts a message to our window whenever it detects
// a state change in the transport.
//-----------------------------------------------------------------------------

void CDVProp::OnDeviceNotify(LONG State)
{
	// The external device changed state.

	SetWindowText(GetDlgItem(IDC_VTR_MODE), GetModeName(State));
	if ((State == ED_MODE_RECORD) && m_pGraph)
	{
		m_pGraph->Run();
	}
}


//-----------------------------------------------------------------------------
// Name: DisplayTimecode()
// Desc: Get the timecode from the device and display it.
//-----------------------------------------------------------------------------

void CDVProp::DisplayTimecode()
{
	if (HasTimecode())
	{
		DWORD dwHour = 0, dwMin = 0, dwSec = 0, dwFrame = 0;
		if (SUCCEEDED(GetTimecode(&dwHour, &dwMin, &dwSec, &dwFrame)))
		{
			TCHAR szBuf[32];
			wsprintf(szBuf, "%.2d:%.2d:%.2d", dwHour, dwMin, dwSec);
			SetWindowText(GetDlgItem(IDC_TIMECODE), szBuf);
		}
        else
        {
            SetWindowText(GetDlgItem(IDC_TIMECODE), "???");
        }
	}
}

//-----------------------------------------------------------------------------
// Name: EnableVtrButtons()
// Desc: Enable or disable the buttons that control the VTR.
//-----------------------------------------------------------------------------

void CDVProp::EnableVtrButtons(BOOL bEnable)
{
	EnableWindow(GetDlgItem(IDC_VTR_PLAY), bEnable);
	EnableWindow(GetDlgItem(IDC_VTR_STOP), bEnable);
	EnableWindow(GetDlgItem(IDC_VTR_TRANSMIT), bEnable);
	EnableWindow(GetDlgItem(IDC_VTR_REW), bEnable);
	EnableWindow(GetDlgItem(IDC_VTR_FF), bEnable);
	
}
