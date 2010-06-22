#include "stdafx.h"
#include "CTunerProp.h"


//-----------------------------------------------------------------------------
// Name: InitDevice()
// Desc: Initialize this object.
//
// pF:   Pointer to the capture filter
//-----------------------------------------------------------------------------

void CTunerProp::InitDevice(IBaseFilter *pF) 
{ 
	HRESULT hr;
	m_pTuner.Release();
	m_pLine21.Release();
	if (m_pGraph)
	{
        // Look UPSTREAM from the capture filter for the TV tuner
		hr = m_pGraph->CaptureGraphBuilder()->FindInterface(&LOOK_UPSTREAM_ONLY,
			0, pF, IID_IAMTVTuner, (void**)&m_pTuner);

        // Look DOWNSTREAM from the capture filter for the Line-21 Decoder
		hr = m_pGraph->CaptureGraphBuilder()->FindInterface(&LOOK_DOWNSTREAM_ONLY,
			0, pF, IID_IAMLine21Decoder, (void**)&m_pLine21);
	}
}


//-----------------------------------------------------------------------------
// Name: SetTunerProps()
// Desc: Get the current channel and the min/max channels
//-----------------------------------------------------------------------------

HRESULT CTunerProp::SetTunerProps()
{
	HRESULT hr = S_FALSE;
	if (m_pTuner)
	{
		long lVideoSubChannel, lAudioSubChannel;
		hr = m_pTuner->get_Channel(&m_lChannel, &lVideoSubChannel, &lAudioSubChannel);

		if (SUCCEEDED(hr))
		{
			m_lNewChannel = m_lChannel;
		}

		m_pTuner->ChannelMinMax(&m_lChannelMin, &m_lChannelMax);
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Called when the dialog is initialized.
//-----------------------------------------------------------------------------

HRESULT CTunerProp::OnInitDialog()
{
    SetWindowBitmap(IDC_TV_CHAN_UP, IDB_TV_CHAN_UP);
    SetWindowBitmap(IDC_TV_CHAN_DOWN, IDB_TV_CHAN_DOWN);

	SetTunerProps();

    // Check whether there is closed captioning available
	HRESULT hr = E_FAIL;
    AM_LINE21_CCSTATE CCState;
	if (HasCC())
	{
		hr = m_pLine21->GetServiceState(&CCState);
	}

    // If so, enable the CC check box and set the state to checked (CC on) or unchecked (CC off)
	if (SUCCEEDED(hr))
	{
		CheckDlgButton(m_hDlg, IDC_TV_CC, 
			(CCState == AM_L21_CCSTATE_On) ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		EnableWindow(GetDlgItem(IDC_TV_CC), FALSE);
	}

    if (HasTuner())
    {
        // Check whether the tuner supports TV, FM radio, AM radio
        long lModes = 0;
        hr = m_pTuner->GetAvailableModes(&lModes);
        if (SUCCEEDED(hr))
        {
            if (AMTUNER_MODE_TV & lModes)
            {
                EnableWindow(GetDlgItem(IDC_TUNER_MODE_TV), TRUE);
            }
            if (AMTUNER_MODE_FM_RADIO & lModes)
            {
                EnableWindow(GetDlgItem(IDC_TUNER_MODE_FM), TRUE);
            }
            if (AMTUNER_MODE_AM_RADIO & lModes)
            {
                EnableWindow(GetDlgItem(IDC_TUNER_MODE_AM), TRUE);
            }

            // Set the radio button for the current mode
            AMTunerModeType lMode;
            hr = m_pTuner->get_Mode(&lMode);
            if (SUCCEEDED(hr))
            {
                int nButton;
                switch(lMode)
                {
                case AMTUNER_MODE_TV:
                    nButton = IDC_TUNER_MODE_TV;
                    break;
                case AMTUNER_MODE_FM_RADIO:
                    nButton = IDC_TUNER_MODE_FM;
                    break;
                case AMTUNER_MODE_AM_RADIO:
                    nButton = IDC_TUNER_MODE_AM;
                    break;
                }
                CheckRadioButton(m_hDlg, IDC_TUNER_MODE_TV, IDC_TUNER_MODE_AM, nButton);
            }
        }
    }
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: OnReceiveMsg()
// Desc: Called whenever the dialog window receives a message.
//-----------------------------------------------------------------------------

INT_PTR CTunerProp::OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DRAWITEM:
		{
            // Draw the channel number inside our owner-drawn control
			TCHAR szMsg[32];
			wsprintf(szMsg, "%d", m_lNewChannel);

			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; 
			HDC hdc = lpdis->hDC;

			SetBkMode(hdc, TRANSPARENT);

			HGDIOBJ hOldFont = SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));

            // Text color changes when the user starts pushing buttons
			COLORREF oldTextColor;
			if (m_bStartNewChannel)
			{
				oldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0x0));
			}
			else
			{
				oldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
			}
			HBRUSH hbr = CreateSolidBrush(RGB(0x33, 0x66, 66));
			FillRect(hdc, &(lpdis->rcItem), hbr);

			DrawText(hdc, szMsg, -1, &(lpdis->rcItem), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			SelectObject(hdc, hOldFont);
			SetTextColor(hdc, oldTextColor);
		}
		return TRUE; 

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
            // Buttons for Channel-0 to Channel-9
			WORD wButton = LOWORD(wParam);
			if ((wButton >= IDC_TV_CHAN0) && (wButton <= IDC_TV_CHAN9))
			{
                // If we didn't start a new channel, start one now.
				if (!m_bStartNewChannel)
				{
					m_lNewChannel = 0;
					m_bStartNewChannel = true;
				}
                // Add this number to the channel we're building up.
				m_lNewChannel = m_lNewChannel * 10 + wButton - IDC_TV_CHAN0;

                // Update the channel on the UI
				RedrawControl(IDC_CHANNEL_DRAW);
				return TRUE;
			}
			switch (wButton)
			{
			case IDC_CHAN_ENTER:        // Submit new channel to the tuner.
				if (m_pTuner) 
				{ 
					HRESULT hr = m_pTuner->put_Channel(m_lNewChannel, -1, -1);
					if (SUCCEEDED(hr)) 
					{
						m_lChannel = m_lNewChannel;
					}
					else
					{
						m_lNewChannel = m_lChannel;
						hr = m_pTuner->put_Channel(m_lNewChannel, -1, -1);
					}
				}
				m_bStartNewChannel = false;
				RedrawControl(IDC_CHANNEL_DRAW);
				return TRUE;

			case IDC_TV_AUDIO_MUTE:     // Mute the audio.
				if (m_pGraph)
				{
					if (IsDlgButtonChecked(m_hDlg, (int)wParam) == BST_CHECKED)
					{
						m_pGraph->ConfigureTVAudio(FALSE);
					}
					else
					{
						m_pGraph->ConfigureTVAudio(TRUE);
					}
				}
				return TRUE;

			case IDC_TV_CC:     // Turn CC on or off.
				if (HasCC())
				{
					if (IsDlgButtonChecked(m_hDlg, (int)wParam) == BST_CHECKED)
					{
						m_pLine21->SetServiceState(AM_L21_CCSTATE_On);
					}
					else
					{
						m_pLine21->SetServiceState(AM_L21_CCSTATE_Off);
					}
				}
				return TRUE;
	
            // Scan up or down to the next channel

			case IDC_TV_CHAN_UP:
				ScanChannel(true);
				return TRUE;

			case IDC_TV_CHAN_DOWN:
				ScanChannel(false);
				return TRUE;
			}

		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// Name: ScanChannel()
// Desc: Scan up or down to the next valid channel.
//
// bUp: TRUE = go to next higher channel #, FALSE = go to next lower channel #
//-----------------------------------------------------------------------------

void CTunerProp::ScanChannel(bool bUp)
{
	if (m_pTuner)
	{
		HRESULT hr;
		long lChannel = m_lChannel; // Cache where we started so we don't loop forever
		do
		{
            // Increment or decrement the channel, but loop around if we reach min or max.
			lChannel = m_lChannel + (bUp ? 1 : -1);
			if (lChannel < m_lChannelMin)
			{
				lChannel = m_lChannelMax;
			}
			else if (lChannel > m_lChannelMax)
			{
				lChannel = m_lChannelMin;
			}

            // Tune to the channel
			hr = m_pTuner->put_Channel(lChannel, -1, -1);
			if (SUCCEEDED(hr))
			{
                // Get the signal strength. 
				long lSignal = 0;
				hr = m_pTuner->SignalPresent(&lSignal);
				if (SUCCEEDED(hr))
				{
                    // We have a signal - we're done.

                    // NOTE: You may always get a signal even if there's no picture.

					m_lNewChannel = m_lChannel = lChannel;
					m_bStartNewChannel = false;
					RedrawControl(IDC_CHANNEL_DRAW);
					break;
				}
			}
		} while (lChannel != m_lChannel);
	}
}