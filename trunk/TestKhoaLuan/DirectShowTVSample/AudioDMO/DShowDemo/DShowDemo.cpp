//////////////////////////////////
//
// Link with:
// strmiids.lib - Exports DirectShow GUIDs
// dmoguids.lib - Exports DMO GUIDs

#include <dshow.h>
#include <dmodshow.h>  // Declares DMO Wrapper Filter info
#include <dmoreg.h>    // Declares DMO GUIDs
#include <commctrl.h>

#include "resource.h"


#include <initguid.h>
DEFINE_GUID(CLSID_Delay,
			0xAD90F22E, 0xC51D, 0x4F1C, 0xB4, 0x40, 0xC1, 0xF2, 0xA0, 0x1D, 0x5F, 0x1F);


#define SAFE_RELEASE(x) { if (x) { (x)->Release(); (x)=NULL; } }
#define WM_GRAPHNOTIFY  WM_APP + 1  // Windows message for graph events
#define APP_NAME		"Simple Delay Tester"


// Globals

HINSTANCE	g_hinst;
HWND		g_hwnd;
HWND		g_hwndStatus;
DWORD		g_dwRegister = 0;
BOOL		g_bPlayState = FALSE;

IGraphBuilder *g_pGraph = NULL;
IMediaControl *g_pMediaControl = NULL;
IMediaEventEx *g_pEvent = NULL;




/// Forward declares

BOOL	HandleInitDialog(HWND);
void    HandleEvent(void);
void	OpenClip(HWND);
void	EnterPlayState(void);
void	EnterStopState(void);
HRESULT LoadWavFile(HWND, LPTSTR);
void	CleanUp(void);

// Dialog procedure
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// Add and remove filter graph from running object table
HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void RemoveFromRot(DWORD pdwRegister);





/////////////////////////////////////////
/// Main entry point:

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR  lpszCmdParam,
                    int    nCmdShow )
{

    InitCommonControls();
    CoInitialize(NULL);

	g_hinst = hInstance;

    // Display the main dialog box.
    DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc );

    CoUninitialize();
    return TRUE;
}




INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg) {

    case WM_INITDIALOG:
        HandleInitDialog(hwnd);
        break;

    case WM_COMMAND:
            switch(wParam)
            { 
            case IDC_OPEN:                
                OpenClip(hwnd);
                break;
                
            case IDC_PLAY:                
                g_bPlayState ? EnterStopState() : EnterPlayState();
                break;
                
            }
        break;

    case WM_GRAPHNOTIFY:
        HandleEvent();
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
		CleanUp();
		break;
    
	
	default:
        return FALSE;  // Did not handle message
    }
    
    return TRUE;  // Handled message

}


BOOL HandleInitDialog(HWND hwnd) 
{
	g_hwnd = hwnd;

	SetWindowText(hwnd, TEXT(APP_NAME));
	
	g_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, hwnd, 0);
	SendMessage(g_hwndStatus, SB_SIMPLE, (WPARAM)TRUE, 0);


    return FALSE;
}



void OpenClip( HWND hwnd ) 
{
//    HRESULT hr;
    HMENU hMenu = GetMenu(hwnd);

    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[MAX_PATH];       // buffer for file name
        
    szFile[0] = TEXT('\0');     // Clear any garbage data from file name.

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL; 
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "WAV Files\0*.wav\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box.  
    if (GetOpenFileName(&ofn) != TRUE) // failed to open file

    {
        return;
    }

	if (SUCCEEDED(LoadWavFile(hwnd, ofn.lpstrFile)))
	{

		SendMessage(g_hwndStatus, SB_SETTEXT, SB_SIMPLE | SBT_NOBORDERS, (LPARAM)ofn.lpstrFile);
		EnableWindow(GetDlgItem(hwnd, IDC_PLAY), TRUE);
	}
	else
	{
		MessageBox(hwnd, TEXT("Cannot load file."), APP_NAME,
			MB_OK | MB_ICONERROR);
	}
}



void EnterPlayState() 
{
	SetWindowText(GetDlgItem(g_hwnd, IDC_PLAY), TEXT("Stop"));
	if (g_pMediaControl)
	{
		g_pMediaControl->Run();
	}
	g_bPlayState = TRUE;
}


void EnterStopState()
{
	if (g_pMediaControl)
	{
		IMediaSeeking *pSeek;
		
		g_pMediaControl->Stop();
		g_pGraph->QueryInterface(IID_IMediaSeeking, (void**)&pSeek);

		REFERENCE_TIME rtStart = 0;
        pSeek->SetPositions(&rtStart, AM_SEEKING_AbsolutePositioning, 
			NULL, AM_SEEKING_NoPositioning);

		pSeek->Release();
	}

	SetWindowText(GetDlgItem(g_hwnd, IDC_PLAY), TEXT("Play"));
	g_bPlayState = FALSE;

}

HRESULT LoadWavFile(HWND hwnd, LPTSTR szFileName)
{
	IBaseFilter   *pDmoFilter = NULL;
	HRESULT hr;

	CleanUp(); // Restore to original state

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&g_pGraph);
    
	if (FAILED(hr))
	{
		return hr;
	}

	hr = AddToRot(g_pGraph, &g_dwRegister);
	if (FAILED(hr))
	{
		MessageBox(hwnd, TEXT("Cannot add graph to Running Object Table"), APP_NAME,
			MB_OK | MB_ICONWARNING);
		g_dwRegister = 0;
	}

	g_pGraph->QueryInterface(IID_IMediaControl, (void **)&g_pMediaControl);
    g_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&g_pEvent);

    g_pEvent->SetNotifyWindow((OAHWND)hwnd, WM_GRAPHNOTIFY, 0);



	// Create the DMO Wrapper filter.
	hr = CoCreateInstance(CLSID_DMOWrapperFilter, NULL, 
		CLSCTX_INPROC, IID_IBaseFilter, (void **)&pDmoFilter);
	
	if (FAILED(hr))
	{
		return hr;
	}

	IDMOWrapperFilter *pWrap;
	pDmoFilter->QueryInterface(IID_IDMOWrapperFilter, (void **)&pWrap);
	
	hr = pWrap->Init(CLSID_Delay, DMOCATEGORY_AUDIO_EFFECT); 
	pWrap->Release();
	
	if (SUCCEEDED(hr))
	{
		hr = g_pGraph->AddFilter(pDmoFilter, L"SimpleDelay DMO");

#ifdef _UNICODE
		hr = g_pGraph->RenderFile(szFileName, NULL);
#else
		WCHAR wszFile[MAX_PATH];
		int result = MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wszFile, MAX_PATH);
		if (result == 0)
		{
	        hr =  E_FAIL;
		}
		else 
		{
			hr = g_pGraph->RenderFile(wszFile, NULL);
		}

#endif

	}
	
	pDmoFilter->Release();

	return hr;
}



void HandleEvent() 
{

    long evCode, param1, param2;
    HRESULT hr;

    if (!g_pEvent)
		return;

    // Spin through the events
    while (hr = g_pEvent->GetEvent(&evCode, &param1, &param2, 0), SUCCEEDED(hr))
    { 
        hr = g_pEvent->FreeEventParams(evCode, param1, param2);

        if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode) || EC_ERRORABORT == evCode)        
        {
            EnterStopState();
            break;
        }
              
    } // Spin through the events
}



void CleanUp(void)
{
	if (g_dwRegister)
	{
		RemoveFromRot(g_dwRegister);
	}
    
	if (g_pEvent)
	{
		g_pEvent->SetNotifyWindow(NULL, 0, 0);
	}

	SAFE_RELEASE(g_pMediaControl);
    SAFE_RELEASE(g_pEvent);
    SAFE_RELEASE(g_pGraph);

	EnableWindow(GetDlgItem(g_hwnd, IDC_PLAY), FALSE);

}



HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[256];

    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    
	if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}


void RemoveFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;
    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

