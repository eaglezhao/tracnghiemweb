#include "StdAfx.h"
#include "audiocapdlg.h"
#include "resource.h"


//-----------------------------------------------------------------------------
// Name: CAudioCapDlg()
// Desc: Constructor
//-----------------------------------------------------------------------------

CAudioCapDlg::CAudioCapDlg(CCaptureGraph *pGraph)
: CBaseDialog(IDD_AUDIO_DEVICE) 
{
    _ASSERTE(pGraph);
    m_pGraph = pGraph;
}

//-----------------------------------------------------------------------------
// Name: ~CAudioCapDlg()
// Desc: Destructor
//-----------------------------------------------------------------------------

CAudioCapDlg::~CAudioCapDlg(void)
{
}


//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Called when the dialog is initialized.
//-----------------------------------------------------------------------------

HRESULT CAudioCapDlg::OnInitDialog()
{
    // Create the list of audio capture devices
	HRESULT hr = m_AudioDevices.Init(CLSID_AudioInputDeviceCategory);
	if (FAILED(hr))
	{
		return hr;
	}

    // Populate our list box with their names.
	HWND hList = GetDlgItem(IDC_AUDIO_DEVICE_LIST);
	hr = m_AudioDevices.PopulateList(hList);

	if (m_AudioDevices.Count() == 0)
	{
		// Disable the OK button if there are no items
		EnableWindow(GetDlgItem(IDOK), FALSE);
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: OnOK()
// Desc: Called when the user clicks OK.
//-----------------------------------------------------------------------------

BOOL CAudioCapDlg::OnOK()
{
    // Get the current selection, if any

	ULONG ulSel = (ULONG)SendMessage(GetDlgItem(IDC_AUDIO_DEVICE_LIST), LB_GETCURSEL, 0, 0);

	if (ulSel != LB_ERR)
	{
        HRESULT hr = E_FAIL;

        // The user selected something, so try to create this audio device.
        // Start by getting the device moniker.
        IMoniker *pMoniker;
		m_AudioDevices.GetMoniker(ulSel, &pMoniker);

        // Now find the name of the input pin that the user selected.
        ulSel = (ULONG)SendMessage(GetDlgItem(IDC_AUDIO_PIN_LIST), LB_GETCURSEL, 0, 0);
        if (ulSel != LB_ERR)
        {
            USES_CONVERSION;
            TCHAR szAudioInput[MAX_PIN_NAME]; 
            SendMessage(GetDlgItem(IDC_AUDIO_PIN_LIST), LB_GETTEXT, 
                (WPARAM)ulSel, (LPARAM)szAudioInput);

           	hr = m_pGraph->AddAudioCaptureDevice(pMoniker, T2W(szAudioInput));
        }

        if (FAILED(hr))
        {
            ReportError(m_hDlg, TEXT("Cannot create the audio capture device"));
        }
    }

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: OnReceiveMsg()
// Desc: Called whenever the dialog window receives a message.
//-----------------------------------------------------------------------------

INT_PTR CAudioCapDlg::OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case LBN_SELCHANGE:
            // The user selected an audio device. Populate the pin list with the
            // names of the input pins.
            if (LOWORD(wParam) == IDC_AUDIO_DEVICE_LIST)
            {
                PopulatePinList();
            }
        }
        return TRUE;

    default:
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: PopulatePinList()
// Desc: Populate the pin list with the names of all the input pins on the
//       audio capture device. (Unfortunately you have to actually create the
//       audio capture filter to do this.)
//-----------------------------------------------------------------------------

HRESULT CAudioCapDlg::PopulatePinList()
{
    HRESULT hr;

    HWND hPinList = GetDlgItem(IDC_AUDIO_PIN_LIST);

    // Clear the list
    SendMessage(hPinList, LB_RESETCONTENT, 0, 0);

    // Get the device moniker.
    ULONG ulSel = (ULONG)SendMessage(GetDlgItem(IDC_AUDIO_DEVICE_LIST), LB_GETCURSEL, 0, 0);

    if (ulSel == LB_ERR)
    {
        return E_FAIL;
    }

    IMoniker *pMoniker;
    m_AudioDevices.GetMoniker(ulSel, &pMoniker);

    // Create the filter ... note that we're not adding it to the graph at this point.
    CComPtr<IBaseFilter> pF;
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pF);

    if (FAILED(hr))
    {
        return hr;
    }
    else
    {
        
        USES_CONVERSION;
        CComPtr<IEnumPins> pEnum;
        CComPtr<IPin> pPin;
        hr = pF->EnumPins(&pEnum);
        if (FAILED(hr))
        {
            return hr;
        }

        // Enumerate the pins. For every input pin, add the pin's name to the list.
        while (S_OK == pEnum->Next(1, &pPin, 0))
        {
            PIN_INFO PinInfo;
            hr = pPin->QueryPinInfo(&PinInfo);
            if (PinInfo.pFilter)
            {
                PinInfo.pFilter->Release();
            }
            if (FAILED(hr))
            {
                return hr;
            }
            if (PinInfo.dir == PINDIR_INPUT)  // input pin?  (ignore the output pin)
            {
                SendMessage(hPinList, LB_ADDSTRING,
                    0, (LPARAM)W2T(PinInfo.achName));
            }
            pPin.Release();
        }
    }

    SendMessage(hPinList, LB_SETCURSEL, 0, 0);
    return S_OK;
}