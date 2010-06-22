// WinCap.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WinCap.h"
#define MAX_LOADSTRING 100

#include "MainDialog.h"

// Globals

const long cchAppName = 128;
TCHAR szAppName[cchAppName];

// Add additional flags as needed to support more Common Control classes.
const DWORD CommonControlInitFlags = ICC_WIN95_CLASSES | ICC_UPDOWN_CLASS;

/**** FORWARD DECLARES *****/

// dlg procs
LRESULT CALLBACK ConfigDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL InitApp(HINSTANCE hInstance);





/**** FUNCTION DEFINITIONS *****/


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	if (!InitApp(hInstance))
	{
		return 1;
	}

	{
		// Scope for CMainDialog object

		CMainDialog MainDlg;

        MainDlg.QuitOnClose(true);
		MainDlg.ShowDialog(hInstance, 0);

		MSG msg;
		BOOL bRet;
		while (bRet = GetMessage(&msg, 0, 0, 0), bRet != 0)
		{
			if (bRet == -1)
			{
				ReportError(0, TEXT("Error in message loop"));
				break;
			}

			if (MainDlg.HandleMessage(&msg))
			{
			}
			else if (MainDlg.TVProp.HandleMessage(&msg))
			{
			}
			else if (MainDlg.DVProp.HandleMessage(&msg))
			{
			}
		}
	}
	CoUninitialize();
	return 0;
}



void ReportError(HWND hwnd, const TCHAR* msg)
{
	MessageBox(hwnd, msg, szAppName, MB_OK | MB_ICONERROR);
}


BOOL InitApp(HINSTANCE hInstance)
{
	if (!LoadString(hInstance, IDS_APP_TITLE, szAppName, cchAppName))
	{
		MessageBox(0, TEXT("Cannot load string!"), 0, MB_OK | MB_ICONERROR);
		return FALSE;
	}


	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = CommonControlInitFlags;
	if (!InitCommonControlsEx(&icc))
	{
		ReportError(0, TEXT("Cannot initialize the Windows Common Controls library"));
		return FALSE;
	}

	HRESULT hr = CoInitialize(0);
	if (FAILED(hr))
	{
		ReportError(0, TEXT("Cannot initialize COM libary"));
		return FALSE;
	}

	return TRUE;
}






