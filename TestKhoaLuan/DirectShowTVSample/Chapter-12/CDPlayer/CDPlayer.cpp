// CDPlayer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <dshow.h>
#include "initguid.h"
 
// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IBasicAudio   *pBA = NULL;
IMediaSeeking *pMS = NULL;

IBaseFilter *pWMP =       NULL;
IFileSourceFilter *pFSF = NULL;
AM_MEDIA_TYPE *pmt =      NULL;
 
//  CA9067FF-777D-4B65-AA5F-C0B27E3EC75D

    DEFINE_GUID (CLSID_WMPCD, 
	0xCA9067FF, 0x777D, 0x4B65, 0xAA, 0x5F, 0xC0, 0xB2, 0x7E, 0x3E, 0xC7, 0x5D);

// Foward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE m_hDlg;

IPin *GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
    BOOL       bFound = FALSE;
    IEnumPins  *pEnum;
    IPin       *pPin;
	

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (bFound = (PinDir == PinDirThis))
            break;
        pPin->Release();
    }
    pEnum->Release();
    return (bFound ? pPin : 0);  
}

void InitDirectShow (void)
{
	
	
	CoInitialize (NULL);

	// Get the interface for DirectShow's GraphBuilder
    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB);

    // Have the graph construct its the appropriate graph automatically
       
    // QueryInterface for DirectShow interfaces
    pGB->QueryInterface(IID_IMediaControl, (void **)&pMC);
    pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME);
    pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS);

	CoCreateInstance(CLSID_WMPCD, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pWMP);

	pGB->AddFilter (pWMP, L"WMP CD Filter");
	pWMP->QueryInterface (IID_IFileSourceFilter, (void **) &pFSF);
	pGB->Render (GetPin (pWMP, PINDIR_OUTPUT));
}

BOOL FileFind (void)
{
	WIN32_FIND_DATA wFindData;
	HANDLE          hFind       = NULL;
	BOOL            fFinished   = FALSE;

	memset (&wFindData, 0, sizeof (WIN32_FIND_DATA));
	
	hFind = FindFirstFile ("D:\\*.*", &wFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	while (!fFinished){
		
		SendMessage (GetDlgItem ((HWND) m_hDlg, IDC_LIST), LB_ADDSTRING,  0, (LPARAM) (LPCTSTR) wFindData.cFileName);
		memset (&wFindData, 0, sizeof (WIN32_FIND_DATA));
		if (!FindNextFile (hFind, &wFindData)){
			if (GetLastError () == ERROR_NO_MORE_FILES)
				fFinished = TRUE;
		}

	}
	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
    DialogBox(NULL, (LPCTSTR)IDD_MAIN,NULL, (DLGPROC)WndProc);

	return TRUE;
}


// Mesage handler for about box.
LRESULT CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_hDlg = hDlg;
	
	switch (message)
	{
		case WM_INITDIALOG:
				InitDirectShow ();
				FileFind ();
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_LIST && HIWORD(wParam) == LBN_SELCHANGE) 
			{

			 	TCHAR *pVarName=NULL;
				WCHAR    wPath[MAX_PATH];
				int iIndex = SendMessage (GetDlgItem ((HWND) m_hDlg, IDC_LIST),
					LB_GETCURSEL,  0, 0);
				int iLength = SendMessage (GetDlgItem ((HWND) m_hDlg, IDC_LIST),
					LB_GETTEXTLEN, iIndex, 0)+1;
			 	
				pVarName = (char * ) calloc (iLength, sizeof (TCHAR)); 

				SendMessage (GetDlgItem ((HWND) m_hDlg, IDC_LIST), 
				LB_GETTEXT, iIndex, (LPARAM) pVarName);
		    	
			   // MultiByteToWideChar(CP_ACP, 0, pVarName, -1, wPath, MAX_PATH);     
        
				pFSF->Load ( (LPCOLESTR) wPath, pmt);	
				pMC->Run ();	
			    return TRUE;
			}
			
			else if (LOWORD(wParam) == IDC_PLAY)
				 pMC->Run ();
			else if (LOWORD(wParam) == IDC_STOP)
				 pMC->Stop ();
			else if (LOWORD(wParam) == IDC_PAUSE)
				 pMC->Pause ();
	
	}
    return FALSE;
}


/*EndDialog(hDlg, LOWORD(wParam));
				return TRUE;)*/
	
 