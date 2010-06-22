#pragma once

#include "dialog.h"
#include "resource.h"

#include "Il21dec.h" // Declares IAMLine21Decoder


/******************************************************************************
 *  CTunerProp Class
 *  Implements a dialog for analog TV tuner functions.
 *****************************************************************************/

class CTunerProp : public CNonModalDialog, public CDeviceHelper
{

private:
	CComPtr<IAMTVTuner> m_pTuner;
	CComPtr<IAMLine21Decoder> m_pLine21;

	long m_lChannel;            // Current TV channel
	long m_lNewChannel;         // New channel that the user is inputting.
	bool m_bStartNewChannel;    // Did the user start inputting a new channel.

	long m_lChannelMin;         // Minimum and maximum channels in the tuning space
	long m_lChannelMax;

	HRESULT SetTunerProps();

	void ScanChannel(bool bUp); // Scan up or down to the next valid channel

protected:
	HRESULT OnInitDialog();
	INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    void OnEndDialog()
    {
        SetGraph(0);
        InitDevice(0);
    }

public:
	CTunerProp() : 
	  CNonModalDialog(IDD_TV_PROP), 
      m_pTuner(0), 
	  m_lChannel(0),
	  m_lNewChannel(0),
	  m_lChannelMin(0),
	  m_lChannelMax(0),
	  m_bStartNewChannel(false)
	{ }

	void InitDevice(IBaseFilter *pFilter);

	BOOL HasTuner() { return (m_pTuner != 0); }     // Does the device support TV tuning?
	BOOL HasCC() { return (m_pLine21 != 0); }       // Are we rendering a CC stream?

};