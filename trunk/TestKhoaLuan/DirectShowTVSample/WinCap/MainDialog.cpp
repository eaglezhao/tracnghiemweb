#include "stdafx.h"
#include "resource.h"

#include "MainDialog.h"
#include "ConfigDlg.h"
#include "AudioCapDlg.h"

extern TCHAR szAppName[];

//-----------------------------------------------------------------------------
// Name: CMainDialog()
// Desc: Constructor
//-----------------------------------------------------------------------------

CMainDialog::CMainDialog()
 : CNonModalDialog(IDD_MAIN), 
   m_bCapturing(false),
   m_pGraph(0)
{ 
}


//-----------------------------------------------------------------------------
// Name: OnReceiveMsg()
// Desc: Called whenever the dialog window receives a message.
//-----------------------------------------------------------------------------

INT_PTR CMainDialog::OnReceiveMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case CGraph::WM_GRAPH_MESSAGE:
		if (m_pGraph)
		{
			m_pGraph->HandleEvent(*this);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PREVIEW:
			OnSelectDevice();
			break;
		case IDC_CAPTURE:
			OnCapture(GetDlgItem(IDC_CAPTURE));
			break;
		case IDC_CONFIGURE:
			OnConfig();
			break;
		case IDC_SNAPSHOT:
			if (m_pGraph)
			{
                if (SUCCEEDED(m_pGraph->TakeSnapShot()))
                {
                    MessageBox(m_hDlg, TEXT("Click!"), szAppName, MB_OK);
                }
                else
                {
                    ReportError(m_hDlg, "Snapshot failed.");
                }
			}
			break;

		// These buttons show the device dialogs for TV, VTR
		case IDC_SHOW_TV_PROP:
			TVProp.ShowDialog(m_hinst, m_hDlg);
			break;
		case IDC_SHOW_DV_PROP:
			DVProp.ShowDialog(m_hinst, m_hDlg);
			break;

		// Menu items
		case IDM_ABOUT:
			MessageBox(m_hDlg, TEXT("Windows video capture sample."), szAppName, MB_OK);
			break;
		case IDM_FILE_SET_CAPTURE:
			SetCaptureFileName();
			break;
		case IDM_OPTIONS_AUDIOCAPTUREDEVICE:
			OnSelectAudioDevice();
			break;

		default:
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: OnSelectDevice()
// Desc: The user selected a device. Build a preview graph.
//-----------------------------------------------------------------------------

void CMainDialog::OnSelectDevice()
{	
	HRESULT hr;
	SetInitialDialogState();  // Reset the UI state

	LRESULT index = (long)SendDlgItemMessage(m_hDlg, IDC_DEVICE_LIST, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR)
	{
        MessageBox(m_hDlg, TEXT("Select a device first."), szAppName, MB_OK);
		return;
	}

    // Get rid of the old graph and make a new one. This should guarantee a fresh start.
    hr = InitGraph();  

	if (FAILED(hr))
	{
		ReportError(m_hDlg, TEXT("Could not create the filter graph."));
		return;
	}

	// Select the device and add it to the graph.
	CComPtr<IMoniker> pMoniker;
	hr = m_DevList.GetMoniker((unsigned long)index, &pMoniker);
	if (SUCCEEDED(hr))
	{
		hr = m_pGraph->AddCaptureDevice(pMoniker);

		if (SUCCEEDED(hr))
		{
            // Get a pointer to the capture filter.
			CComPtr<IBaseFilter> pCap;
			if (SUCCEEDED(m_pGraph->GetDevice(&pCap)))
			{
                EnableMenuItem(IDM_OPTIONS_AUDIOCAPTUREDEVICE, TRUE);

                // Build the preview portion of the graph.
				hr = StartPreview();

				if (SUCCEEDED(hr))
				{
					// Write the graph to a file
					SaveGraphFile(L"C:\\MyGraph.GRF");
					hr = m_pGraph->Run();
				}

                // Look for DV or TV functionality, using our helper objects.

				// Show DV/VTR Dialog?
				DVProp.InitDevice(pCap);
				if (DVProp.HasDevice())
				{
					DVProp.ShowDialog(m_hinst, m_hwnd);
					EnableWindow(GetDlgItem(IDC_SHOW_DV_PROP), TRUE);
				}

				// Show TV dialog?
				TVProp.InitDevice(pCap);
				if (TVProp.HasTuner())
				{
					TVProp.ShowDialog(m_hinst, m_hwnd);
					EnableWindow(GetDlgItem(IDC_SHOW_TV_PROP), TRUE);
				}
			}
		}
	}

	if (FAILED(hr))
	{
        RefreshDeviceList(); // Maybe a device was removed?
		SetInitialDialogState();
		ReportError(m_hDlg, TEXT("Could not start preview."));
	}
}


//-----------------------------------------------------------------------------
// Name: StartPreview()
// Desc: Build a preview graph.
//-----------------------------------------------------------------------------

HRESULT CMainDialog::StartPreview()
{
	HRESULT hr = E_UNEXPECTED;
	if (m_pGraph)
	{
		hr = m_pGraph->RenderPreview();
		if (SUCCEEDED(hr))
		{
			// Mute the audio to avoid feedback from the speakers
			m_pGraph->SetAudio(-10000);

			EnableWindow(GetDlgItem(IDC_CONFIGURE), TRUE);
			EnableWindow(GetDlgItem(IDC_CAPTURE), TRUE);
			
			// Try to render the still pin. 
			
			// !!!
			// On some cameras you need to render the capture pin first. 
            // I'm just lucky the Logitech camera does not have a preview pin, 
            // so the capture pin is already hooked up.
			HRESULT hrStill = m_pGraph->RenderStillPin(0, 0);
			if (SUCCEEDED(hrStill))
			{
				EnableWindow(GetDlgItem(IDC_SNAPSHOT), TRUE);
			}
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: InitGraph()
// Desc: Create a new filter graph.
//-----------------------------------------------------------------------------

HRESULT CMainDialog::InitGraph()
{	
	DVProp.SetGraph(0);
	TVProp.SetGraph(0);
	TVProp.InitDevice(0);
	DVProp.RemoveDevice();

	if (m_pGraph)
	{
		delete m_pGraph;
	}

	try
	{
		m_pGraph = new CCaptureGraph();

        // Initialize our helper objects
        DVProp.SetGraph(m_pGraph);
		TVProp.SetGraph(m_pGraph);

        // Set up the video window and the event-sink window
		m_pGraph->SetVideoWindow(GetDlgItem(IDC_VIDWIN));
		m_pGraph->SetEventWindow(m_hDlg);
	}
	catch (bad_hresult& bad_hr)
	{
		return bad_hr.hr;
	}

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Called when the dialog is initialized.
//-----------------------------------------------------------------------------

HRESULT CMainDialog::OnInitDialog()
{
    SetWindowBitmap(IDC_SHOW_TV_PROP, IDB_TV);
    SetWindowBitmap(IDC_SHOW_DV_PROP, IDB_DV);

    SetInitialDialogState();

	RefreshDeviceList();

	wsprintf(m_szCaptureFile, TEXT("C:\\CapFile.avi"));
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RefreshDeviceList()
// Desc: Refresh the list of video capture devices.
//-----------------------------------------------------------------------------

HRESULT CMainDialog::RefreshDeviceList()
{
	HWND hList = GetDlgItem(IDC_DEVICE_LIST);
	SendMessage(hList, LB_RESETCONTENT, 0, 0);  // Clear the list.

	HRESULT hr = m_DevList.Init(CLSID_VideoInputDeviceCategory);
	if (SUCCEEDED(hr))
	{
		hr = m_DevList.PopulateList(hList);
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: OnConfig()
// Desc: Show the dialog to configure the device.
//-----------------------------------------------------------------------------

void CMainDialog::OnConfig()
{
	CConfigDialog *pConfigDlg = new CConfigDialog(m_pGraph);
	if (pConfigDlg)
	{
		if (pConfigDlg->ShowDialog(m_hinst, m_hDlg))
		{
            // Set the output size (IAMStreamConfig)
			HRESULT hr = pConfigDlg->SetVideoFormat();
			if (hr == S_OK)
			{
				// Rebuild the graph
				hr = StartPreview();
			}
			if (FAILED(hr))
			{
				ReportError(m_hDlg, TEXT("Could not set the requested video format."));
			}
            
            // Set the DV decoding resolution. 
			hr = pConfigDlg->SetDVDecoding();

            // Invalidate the video rectangle in case the 
			// video size has changed.
			InvalidateRect(m_hDlg, 0, TRUE);
			hr = m_pGraph->Run();
		}		
		delete pConfigDlg;
	}
}



//-----------------------------------------------------------------------------
// Name: OnGraphEvent()
// Desc: Handle a DirectShow graph event.
//-----------------------------------------------------------------------------

void CMainDialog::OnGraphEvent(long lEvent, long lParam1, long lParam2)
{
	switch (lEvent)
	{
	case EC_DEVICE_LOST:
        // Lost the device.
		OutputDebugString(TEXT(">>> EC_DEVICE_LOST\n"));
        if (lParam2 == 0)
        {
            // ?? This causes an assert which appears to be harmless, and is
            // possibly a bug in KSProxy?
            m_pGraph->ReleaseDevice();
            m_pGraph->TearDownGraph();
        }
		RefreshDeviceList();
		SetInitialDialogState();
		break;
	case EC_COMPLETE:
		// End of a file - sent when we are transmitting from file to tape.
		m_pGraph->Stop();
		DVProp.Stop();
		break;
	}
}


//-----------------------------------------------------------------------------
// Name: OnCapture()
// Desc: Start or stop capture to file. (We use the same button for both)
//-----------------------------------------------------------------------------

void CMainDialog::OnCapture(HWND hCaptureButton)
{
	HRESULT hr;
	if (m_bCapturing)
	{
        // We're capturing now, so stop.
		hr = m_pGraph->StopCapture();

        // Toggle the button caption.
		EnableWindow(GetDlgItem(IDC_CONFIGURE), TRUE);
		SetWindowText(hCaptureButton, TEXT("Start Capture"));
		m_bCapturing = false;
	}
	else
	{
        // We're not capturing now, so start. First build the graph...
		hr = m_pGraph->RenderAviCapture(m_szCaptureFile, MAX_PATH);

		if (SUCCEEDED(hr))
		{
			// Write a copy of the filter graph to disk.
			SaveGraphFile(L"C:\\MyGraph.GRF");

            // Then start the capture stream.
			hr = m_pGraph->StartCapture();

            // Toggle the button caption
			if (SUCCEEDED(hr))
			{
				EnableWindow(GetDlgItem(IDC_CONFIGURE), FALSE);
				SetWindowText(hCaptureButton, TEXT("Stop Capture"));
				m_bCapturing = true;
			}
		}
		if (FAILED(hr))
		{
			ReportError(m_hDlg, TEXT("Error starting capture"));
		}
	}
}


//-----------------------------------------------------------------------------
// Name: OnOK()
// Desc: Called when the user clicks OK (really "quit" on this dialog)
//-----------------------------------------------------------------------------

BOOL CMainDialog::OnOK()
{
	if (MessageBox(m_hDlg, TEXT("Quit application?"), szAppName, MB_OKCANCEL) == IDOK)
	{
		m_DevList.Release();

		TVProp.EndDialog();
		DVProp.EndDialog();

		delete m_pGraph;

		return CNonModalDialog::OnOK();
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: SetInitialDialogState()
// Desc: Set all the UI controls to their initial state
//-----------------------------------------------------------------------------

void CMainDialog::SetInitialDialogState()
{
	TVProp.EndDialog();
	DVProp.EndDialog();

	// Disable the control buttons
	EnableWindow(GetDlgItem(IDC_CAPTURE), FALSE);
	EnableWindow(GetDlgItem(IDC_CONFIGURE), FALSE);
	EnableWindow(GetDlgItem(IDC_SNAPSHOT), FALSE);

	// Disable the buttons to show the device props
	EnableWindow(GetDlgItem(IDC_SHOW_TV_PROP), FALSE);
	EnableWindow(GetDlgItem(IDC_SHOW_DV_PROP), FALSE);

    // Disable the "Selct Audio Capture Device" menu item
    EnableMenuItem(IDM_OPTIONS_AUDIOCAPTUREDEVICE, FALSE);

}


//-----------------------------------------------------------------------------
// Name: SetCaptureFileName()
// Desc: Select a name for the capture file. 
//-----------------------------------------------------------------------------

void CMainDialog::SetCaptureFileName()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hDlg;
	ofn.lpstrFilter = TEXT("Avi\0*.avi\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = m_szCaptureFile;
	ofn.nMaxFile = MAX_PATH + 1;
	ofn.lpstrTitle = TEXT("Select a target file for video capture.");
	ofn.Flags = OFN_OVERWRITEPROMPT;

	GetSaveFileName(&ofn);
}


//-----------------------------------------------------------------------------
// Name: OnSelectAudioDevice()
// Desc: Select an audio capture device.
//-----------------------------------------------------------------------------

void CMainDialog::OnSelectAudioDevice()
{

	CAudioCapDlg *pDlg = new CAudioCapDlg(m_pGraph);
	if (pDlg)
	{
		pDlg->ShowDialog(m_hinst, m_hDlg);  // The dialog does everything.
		delete pDlg;
	}
}

// Pass filename in wszPath, saves filter graph to that file
HRESULT CMainDialog::SaveGraphFile(WCHAR* wszPath)
{
	const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    IStorage *pStorage = NULL;
	IPersistStream *pPersist = NULL;
	IGraphBuilder* pGraph = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

	// Next, create a stream to store.
    IStream *pStream;
    hr = pStorage->CreateStream(
		wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

	// The IPersistStream converts a stream into a persistent object.

	m_pGraph->CaptureGraphBuilder()->GetFiltergraph(&pGraph);
	pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
	pGraph->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}
