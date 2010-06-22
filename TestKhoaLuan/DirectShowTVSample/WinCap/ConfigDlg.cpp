#include "stdafx.h"
#include "resource.h"
#include "graph.h"
#include "ConfigDlg.h"


// Static variables

// List of buttons that relate to DV Decoding resolution
static int DV_Buttons[] = { IDC_DVDEC_FULL, IDC_DVDEC_HALF, IDC_DVDEC_QUARTER, IDC_DVDEC_DC };

// List of trackbars that control VideoProcAmp settings
static int ProcAmp_Buttons[] = { IDC_BRIGHTNESS, IDC_CONTRAST, IDC_HUE, IDC_SATURATION, 
	IDC_SHARPNESS, IDC_GAMMA, IDC_WHITE_BALANCE, IDC_GAIN };

// List of VideoProcAmp properties (corresponds to the ProcAmp_Buttons entries)
static int ProcAmp_Props[] = { VideoProcAmp_Brightness, VideoProcAmp_Contrast, VideoProcAmp_Hue,
	VideoProcAmp_Saturation, VideoProcAmp_Sharpness, VideoProcAmp_Gamma, VideoProcAmp_WhiteBalance,
	VideoProcAmp_Gain };



//-----------------------------------------------------------------------------
// Name: GetDVDisplayRadioButton()
// Desc: For a given DVRESOLUTION enum value, return the resource ID of the
//       radio button for that resolution setting.
//-----------------------------------------------------------------------------

int GetDVDisplayRadioButton(int iDisplay)
{
	switch (iDisplay)
	{
	case DVRESOLUTION_FULL:
		return IDC_DVDEC_FULL;
	case DVRESOLUTION_HALF:
		return IDC_DVDEC_HALF;
	case DVRESOLUTION_QUARTER:
		return IDC_DVDEC_QUARTER;
	case DVRESOLUTION_DC:
		return IDC_DVDEC_DC;
	default:
		_ASSERT(0);
		return -1;
	}
}


/**************************   CConfigDialog methods **********************************************/


//-----------------------------------------------------------------------------
// Name: CConfigDialog()
// Desc: Constructor
//-----------------------------------------------------------------------------

CConfigDialog::CConfigDialog(CCaptureGraph *pGraph) 
: m_pGraph(pGraph), m_iDisplay(0), CBaseDialog(IDD_CONFIG_PROP)
{
	_ASSERTE(pGraph);
}


//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Called when the dialog is initialized.
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::OnInitDialog()
{
    // Get a pointer to the capture filter.
	CComPtr<IBaseFilter> pCap;
	HRESULT hr = m_pGraph->GetDevice(&pCap);
	if (FAILED(hr))
	{
		return E_UNEXPECTED;
	}


	// Look for VFW dialogs

	pCap.QueryInterface(&m_pVfw);
	if (m_pVfw)
	{
        // If the filter supports IAMVfwCaptureDialogs, it means the device
        // is a VfW driver and may support any of three pre-defined dialogs.

        // For each one, if the driver supports that dialog, enable the 
        // "Show XXX Dialog" button in our UI.

		if (S_OK == m_pVfw->HasDialog(VfwCaptureDialog_Source))
		{
			EnableWindow(GetDlgItem(IDC_VFW_SOURCE), TRUE);
		}
		if (S_OK == m_pVfw->HasDialog(VfwCaptureDialog_Format))
		{
			EnableWindow(GetDlgItem(IDC_VFW_FORMAT), TRUE);
		}
		if (S_OK == m_pVfw->HasDialog(VfwCaptureDialog_Display))
		{
			EnableWindow(GetDlgItem(IDC_VFW_DISPLAY), TRUE);
		}
	}

	// Look for DV Decoder interface. We have to search downstream from the 
    // capture filter, because IIPDVDec is exposed by the DV Decder filter.

	hr = m_pGraph->CaptureGraphBuilder()->FindInterface(&LOOK_DOWNSTREAM_ONLY,
		0, pCap, IID_IIPDVDec, (void**)&m_pDvDec);

	if (SUCCEEDED(hr))
	{
        // Enable the radio buttons for the DV Decoding settings
		for (int i = 0; i < ARRAY_SIZE(DV_Buttons); i++)
		{
			EnableWindow(GetDlgItem(DV_Buttons[i]), TRUE);
		}
        // Get the current setting and check the corresponding radio button.
		hr = m_pDvDec->get_IPDisplay(&m_iDisplay);

		SendDlgItemMessage(m_hDlg, GetDVDisplayRadioButton(m_iDisplay),
			BM_SETCHECK, BST_CHECKED, 0);
		
	}

	// Look for VideoProcAmp stuff. Query the capture filter for the interface.

	pCap.QueryInterface(&m_pProcAmp);
	if (m_pProcAmp)
	{
        // For each VideoProcAmp property, we need to know
        // - Is it supported? (If so, enable the trackbar in our UI)
        // - The min and max range supported by the driver
        // - The current value

        for (int i = 0; i < ARRAY_SIZE(ProcAmp_Buttons); i++)
        {
            long Min, Max, Step, Default, Flags, Val;
            HWND hTrackbar = GetDlgItem(ProcAmp_Buttons[i]);

            hr = m_pProcAmp->GetRange(ProcAmp_Props[i], &Min, &Max, &Step, &Default, &Flags);

            if (SUCCEEDED(hr))
            {
                hr = m_pProcAmp->Get(ProcAmp_Props[i], &Val, &Flags);
            }

            if (SUCCEEDED(hr))
            {
                // Set the trackbar range to match the range reported by the driver.
                SendMessage(hTrackbar, TBM_SETRANGE, TRUE, MAKELONG(Min, Max));
                // Set the trackbar position to match the current value
                SendMessage(hTrackbar, TBM_SETPOS, TRUE, Val);
                EnableWindow(hTrackbar, TRUE);
            }
            else
            {
                EnableWindow(hTrackbar, FALSE);
            }
		}	

	}

	// Try IAMStreamConfig stuff

    // Skip if it's a DV device. I don't support IAMStreamConfig for DV, because
    // for some reason it screws up the DV Dec settings, and I haven't figured out
    // why yet. 

    if (!m_pDvDec)  
    {
        // IAMStreamConfig is exposed by pins on the capture filter.
        // Try to find it on the preview pin.

        CComPtr<IAMStreamConfig> pConfig;
        hr = m_pGraph->CaptureGraphBuilder()->FindInterface(&PIN_CATEGORY_PREVIEW,
            0, pCap, IID_IAMStreamConfig, (void**)&pConfig);

        // If not, look on the capture pin.
        if (FAILED(hr))
        {
            hr = m_pGraph->CaptureGraphBuilder()->FindInterface(&PIN_CATEGORY_CAPTURE,
                0, pCap, IID_IAMStreamConfig, (void**)&pConfig);
        }
        if (SUCCEEDED(hr))
        {
            hr = Init(pConfig);  // Initialize our helper object
            if (SUCCEEDED(hr))
            {
                // Initialize our UI
                PopulateVideoTypeList(); 
                SetSpinValues(0);
            }
        }
    }
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: OnOK()
// Desc: Called when the user clicks OK.
//-----------------------------------------------------------------------------

BOOL CConfigDialog::OnOK()
{
	CacheVideoFormat();
	return TRUE;
}


//-----------------------------------------------------------------------------
// Name: OnReceiveMsg()
// Desc: Called whenever the dialog window receives a message.
//-----------------------------------------------------------------------------

INT_PTR CConfigDialog::OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
    case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			switch (LOWORD(wParam))
			{
            // DV Decoding settings - just store them until we close the dialog, because
            // the app needs to stop and then re-start the graph, and update its video window
			case IDC_DVDEC_FULL:
				m_iDisplay =  DVRESOLUTION_FULL;
				break;
			case IDC_DVDEC_HALF:
				m_iDisplay =  DVRESOLUTION_HALF;
				break;
			case IDC_DVDEC_QUARTER:
				m_iDisplay =  DVRESOLUTION_QUARTER;
				break;
			case IDC_DVDEC_DC:
				m_iDisplay =  DVRESOLUTION_DC;
				break;

            // VideoProcAmp Settings
			case IDC_PROCAMP_RESTORE_DEFAULT:  // Restore the default settings
				RestoreProcAmpDefaults();
				break;

            // VFW Dialogs
            case IDC_VFW_SOURCE:
				ShowVfwDialog(VfwCaptureDialog_Source);
				break;
			case IDC_VFW_FORMAT:
				ShowVfwDialog(VfwCaptureDialog_Format);
				break;
			case IDC_VFW_DISPLAY:
				ShowVfwDialog(VfwCaptureDialog_Display);
				break;
			}
		}
		break;

		case CBN_SELCHANGE:
			if (LOWORD(wParam) == IDC_VIDEO_TYPES)
			{
                // The user selected a new format from the list provided by IAMStreamConfig.
				long sel = (long)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (sel == CB_ERR)
				{
					// Something weird just happened! Try to recover...
					sel = 0;
					SendMessage((HWND)lParam, CB_SETCURSEL, 0, 0);
				}

                // Update the height/width spinners
				SetSpinValues(sel);
			}
			break;
		} // switch WM_COMMAND
		return TRUE;

	case WM_HSCROLL:
		{
            // The user moved one of the VideoProcAmp sliders. We can update this in
            // real time. First, find which trackbar it was...
			HWND hCtl = (HWND)lParam;
			for (int i = 0; i < ARRAY_SIZE(ProcAmp_Buttons); i++)
			{
				if (hCtl == GetDlgItem(ProcAmp_Buttons[i]))
				{
                    // Set the VideoProcAmp property on the filter.
					_ASSERTE(m_pProcAmp != 0);
					DWORD dwPos = (DWORD)SendMessage(hCtl, TBM_GETPOS, 0, 0);
					m_pProcAmp->Set(ProcAmp_Props[i], dwPos, VideoProcAmp_Flags_Manual);

					break;
				}
			}
		}
		return TRUE;

    default:
        return FALSE;  // Did not handle message
	}
}


//-----------------------------------------------------------------------------
// Name: SetDVDecoding()
// Desc: Set the DV Decoding resolution based on the last state of the UI.
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::SetDVDecoding()
{
	if (m_pDvDec && (m_iDisplay))
	{
		int iNow = 0;
		if (FAILED(m_pDvDec->get_IPDisplay(&iNow)))
		{
			return E_FAIL;
		}
		if (iNow == m_iDisplay)
		{
			return S_FALSE;
		}
		m_pGraph->Stop();  // Can't switch resolutions on the fly.
		return m_pDvDec->put_IPDisplay(m_iDisplay);
	}

	return S_FALSE;
}


//-----------------------------------------------------------------------------
// Name: ShowVfwDialog()
// Desc: Show a VfW dialog.
//
// iDialog: Which dialog to show. (See IAMVfwCaptureDialogs ref)
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::ShowVfwDialog(VfwCaptureDialogs iDialog)
{
	if (m_pVfw && m_pGraph)
	{
		m_pGraph->Stop();
		if (FAILED(m_pVfw->ShowDialog(iDialog, m_hDlg)))
        {
            ReportError(m_hDlg, TEXT("Cannot display this dialog."));
        }
		m_pGraph->Run();
	}
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RestoreProcAmpDefaults()
// Desc: Reset all the VideoProcAmp settings to their default values.
//-----------------------------------------------------------------------------

void CConfigDialog::RestoreProcAmpDefaults()
{
	HRESULT hr;
	if (m_pProcAmp)
	{
		for (int i = 0; i < ARRAY_SIZE(ProcAmp_Buttons); i++)
		{
            // For each property, get the default. 
			long Min, Max, Step, Default, Flags;
			hr = m_pProcAmp->GetRange(ProcAmp_Props[i], &Min, &Max, &Step, 
				&Default, &Flags);

			if (SUCCEEDED(hr))
			{
                // Set the default value on the device
				hr = m_pProcAmp->Set(ProcAmp_Props[i], Default, VideoProcAmp_Flags_Manual);
				if (SUCCEEDED(hr))
				{
                    // Update the trackbar position
					SendMessage(GetDlgItem(ProcAmp_Buttons[i]), TBM_SETPOS, TRUE, Default);
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Name: CacheVideoFormat()
// Desc: Save the output format settings - but don't change the output format yet.
//
// For IAMStramConfig, our UI has the output format in two places:
// - A list box with the list of formats.
// - Spinners that can be used to change the width and height of the format.
//
// Here, we need to modify the format to reflect the spinner values. This requires
// us to dig into the format block of the media type and find the BITMAPINFOHEADER,
// which contains the width and height
//-----------------------------------------------------------------------------

void CConfigDialog::CacheVideoFormat()
{
	if (HasConfig() && (Count() > 0))
	{
		BOOL bTranslate = FALSE;
		BITMAPINFOHEADER *pBmi = BitMapInfo();  // Find the BITMAPINFOHEADER

		if (pBmi)
		{
            // Get the new width and height from the spinners.
			int iWidth = (int)GetDlgItemInt(m_hDlg, IDC_WIDTH, &bTranslate, TRUE);

			if (bTranslate)
			{
				pBmi->biWidth = iWidth;
			}

            // Spinner always lists + height, but some formats want - biHeight. Adjust accordingly.
			int iHeightSign = pBmi->biHeight >= 0 ? 1 : -1;
			int iHeight = (int)GetDlgItemInt(m_hDlg, IDC_HEIGHT, &bTranslate, TRUE);

			if (bTranslate)
			{
				pBmi->biHeight = abs(iHeight) * iHeightSign;
			}

			// pBmi->biSizeImage = DIBSIZE(*pBmi);
		}
	}
}


//-----------------------------------------------------------------------------
// Name: SetVideoFormat()
// Desc: Set the output format.
//
// We tear down the graph before setting the format. You might only have to
// re-connect the output pins on the capture filter, but this is safest.
// Caller should rebuild the graph.
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::SetVideoFormat()
{
	if (HasConfig() && (Count() > 0))
	{
		m_pGraph->Stop();
		m_pGraph->TearDownGraph();
		return SetFormat();
	}
	return S_FALSE;
}


//-----------------------------------------------------------------------------
// Name: SetSpinValues()
// Desc: Set the spinners to reflect the IAMStreamConfig info
//
// iCap: Index of the format capability.
//
// For each video format returned by IAMStreamConfig::GetStreamCaps, there is
// a corresponding VIDEO_STREAM_CONFIG_CAPS that tells the minimum and maximum
// output sizes for that format, plus the X and Y "step" values. This function
// sets up our spinners to reflect this information.
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::SetSpinValues(int iCap)
{
    if (Count() < iCap + 1)
    {
        return E_INVALIDARG;
    }

	HWND hHeightSpin = GetDlgItem(IDC_HEIGHT_SPIN);
	HWND hWidthSpin = GetDlgItem(IDC_WIDTH_SPIN);

    // Enable the spinners plus the buddy edit windows
	EnableWindow(hHeightSpin, TRUE);
	EnableWindow(hWidthSpin, TRUE);
	EnableWindow(GetDlgItem(IDC_WIDTH), TRUE);
	EnableWindow(GetDlgItem(IDC_HEIGHT), TRUE);

    // Find the format
	HRESULT hr = GetCap(iCap);

	if (FAILED(hr))
	{
		// Now what??
	}
	else
	{
        // Set the spinner ranges to the min and max output sizes
		SendMessage(hWidthSpin, UDM_SETRANGE32, 
			(WPARAM)m_scc.MinOutputSize.cx,
			(LPARAM)m_scc.MaxOutputSize.cx);

		SendMessage(hHeightSpin, UDM_SETRANGE32, 
			(WPARAM)m_scc.MinOutputSize.cy,
			(LPARAM)m_scc.MaxOutputSize.cy);

        // Set the spinner values to the default output size for that format (found
        // in the BITMAPINFOHEADER.)
		BITMAPINFOHEADER * const pBmi = BitMapInfo();
		if (pBmi)
		{
			SendMessage(hWidthSpin, UDM_SETPOS32, 0, (LPARAM)(pBmi->biWidth));
			SendMessage(hHeightSpin, UDM_SETPOS32, 0, (LPARAM)(abs(pBmi->biHeight)));
		}
		// or else?

        // Set the spinner "acceleration" to match the output "step" sizes. 
        // In other words, if the device supports output sizes in increments of N
        // pixels, then clicking on the spinner will advance the spin control by N.

        UDACCEL acc = { 0, m_scc.OutputGranularityY };

		SendMessage(hHeightSpin, UDM_SETACCEL, 1, (LPARAM)&acc);

		acc.nInc = m_scc.OutputGranularityX;

		SendMessage(hWidthSpin, UDM_SETACCEL, 1, (LPARAM)&acc);
	}

	return hr;
}


//-----------------------------------------------------------------------------
// Name: PopulateVideoTypeList()
// Desc: Fill a list with the names of all the formats reported by IAMStreamConfig 
//       (We do our best to come up with a name for each format...)
//-----------------------------------------------------------------------------

HRESULT CConfigDialog::PopulateVideoTypeList()
{
	HRESULT hr;
	HWND hList = GetDlgItem(IDC_VIDEO_TYPES);
	_ASSERTE(hList);

	int iCount = Count(), iSize = Size();

    if (iCount > 0)
    {
    	EnableWindow(hList, TRUE);
    }

	if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// This is not the struct you're looking for.
		return E_FAIL;
	}
	for (int i = 0; i < iCount; i++)
	{
		hr = GetCap(i);
		if (SUCCEEDED(hr))
		{
			SendMessage(hList, CB_ADDSTRING, 0, (LPARAM)VideoFormatInfo(m_pmtConfig));
		}
	}

	if (iCount > 0)
	{
		SendMessage(hList, CB_SETCURSEL, 0, 0);
	}
	return S_OK;
}




/**************************   CVideoConfig methods **********************************************/

//-----------------------------------------------------------------------------
// Name: CVideoConfig()
// Desc: Constructor
//-----------------------------------------------------------------------------


CVideoConfig::CVideoConfig()
: m_pmtConfig(0), m_iCount(0), m_iSize(0)
{
	ZeroMemory(&m_scc, sizeof(VIDEO_STREAM_CONFIG_CAPS));
}

//-----------------------------------------------------------------------------
// Name: ~CVideoConfig()
// Desc: Destructor
//-----------------------------------------------------------------------------


CVideoConfig::~CVideoConfig()
{
	DeleteMediaType(m_pmtConfig);
}


//-----------------------------------------------------------------------------
// Name: Init()
// Desc: Initialize the object.
//
// pConfig: Pointer to the IAMStreamConfig interface. It's up to the client
// to query the filter for this.
//-----------------------------------------------------------------------------

HRESULT CVideoConfig::Init(IAMStreamConfig *pConfig)
{
	m_pConfig = pConfig;  // Implicit AddRef
	if (m_pConfig)
	{
        // Find the number and struct size
		HRESULT hr = pConfig->GetNumberOfCapabilities(&m_iCount, &m_iSize);
		if (FAILED(hr))
		{
			m_iCount = 0;
			m_iSize = 0;
			return hr;
		}
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetCap()
// Desc: Get the n'th format from the device.
//
// iCap: Index of the format, must be 0 ... Count() - 1
//-----------------------------------------------------------------------------

HRESULT CVideoConfig::GetCap(int iCap)
{
	if (!m_pConfig) return E_FAIL;


    // We use GetStreamCaps to find the format. For video, it fills an 
    // AUDIO_STREAM_CONFIG_CAPS struct. (Audio uses something else but
    // the CVideoConfig class is only for video.) The value of m_iSize 
    // is the size of the struct. So, we have to make sure this is
    // the size we're expecting.
	if (m_iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		return E_FAIL;
	}
    // Index in range?
	if (iCap < 0 || iCap > m_iCount)
	{
		return E_INVALIDARG;
	}

	DeleteMediaType(m_pmtConfig);  // Delete the old format.

	return m_pConfig->GetStreamCaps(iCap, &m_pmtConfig, (BYTE*)&m_scc);
}



//-----------------------------------------------------------------------------
// Name: SetFormat()
// Desc: Try setting the device to use the current format, obtained from GetCap().
//-----------------------------------------------------------------------------

HRESULT CVideoConfig::SetFormat()
{
	if (m_pmtConfig == 0)
	{
		return E_FAIL;
	}

	if (!m_pConfig)
	{
		return E_FAIL;
	}

	// Set the format on every pin. (I'm not sure if you're allowed to set different 
    // formats on different pins? Presumably.)
	
    // Get the pin info, which contains the owning filter. Use this to enumerate
    // all the pins and look for IAMStreamConfig.
	PIN_INFO PinInfo;
	CComQIPtr<IPin> pPin(m_pConfig);
	pPin->QueryPinInfo(&PinInfo);

	CComPtr<IEnumPins> pEnum;
	PinInfo.pFilter->EnumPins(&pEnum);
	PinInfo.pFilter->Release();  

	pPin.Release();
	HRESULT hr;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		CComQIPtr<IAMStreamConfig> pConfig(pPin);
		if (pConfig)
		{
			hr = pConfig->SetFormat(m_pmtConfig);
		}
		pPin.Release();
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DumpFormat()
// Desc: Get the device's current format and write it to the debug window.
//-----------------------------------------------------------------------------

void CVideoConfig::DumpFormat()
{
	AM_MEDIA_TYPE *pmt_actual = NULL;
	if (m_pConfig)
	{
		HRESULT hr = m_pConfig->GetFormat(&pmt_actual);
		if (SUCCEEDED(hr))
		{
			OutputDebugString(TEXT("Output Video Format:\n\t"));
			OutputDebugString(VideoFormatInfo(pmt_actual));
			OutputDebugString(TEXT("\n"));
			DeleteMediaType(pmt_actual);
		}
	}
}




//-----------------------------------------------------------------------------
// Name: BitMapInfo()
// Desc: Looks for the BITMAPINFOHEADER structure in the current format caps.
//
// If successful, it returns a pointer into the format block. (Note that the 
// format block goes away when we delete the media type!)
//
// If it fails, it returns NULL.
//-----------------------------------------------------------------------------
BITMAPINFOHEADER * const CVideoConfig::BitMapInfo()
{
	if (m_pmtConfig == 0)
	{
		return 0;
	}

	if (m_pmtConfig->majortype != MEDIATYPE_Video)
	{
        // Not a video type.
		return 0;
	}

    // Examine the format type.

	if (m_pmtConfig->formattype == FORMAT_VideoInfo)
	{
		if ((m_pmtConfig->pbFormat != 0) &&
            (m_pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)))
		{
			VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)(m_pmtConfig->pbFormat);
			return &(pVIH->bmiHeader);
		}
	}
	else if (m_pmtConfig->formattype == FORMAT_VideoInfo2)
	{
		if ((m_pmtConfig->pbFormat != 0) &&
            (m_pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER2)))
		{
			VIDEOINFOHEADER2 *pVIH = (VIDEOINFOHEADER2*)(m_pmtConfig->pbFormat);
			return &(pVIH->bmiHeader);
		}
	}

    // Note: It could be MPEG1VIDEOINFO or MPEG2VIDEOINFO, but for now I don't handle 
    // those types. It could also be DVINFO, in which case there's no BITMAPINFOHEADER.

	return 0;
}


