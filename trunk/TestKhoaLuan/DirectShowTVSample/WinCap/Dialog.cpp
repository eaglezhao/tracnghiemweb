#include "stdafx.h"
#include "dialog.h"

//-----------------------------------------------------------------------------
// Name: CBaseDialog()
// Desc: Constructor
// 
// nID: Resource ID of the dialog
//-----------------------------------------------------------------------------

CBaseDialog::CBaseDialog(int nID)
: m_nID(nID), m_hDlg(0), m_hinst(0), m_hwnd(0)
{
}


CBaseDialog::~CBaseDialog()
{
    vector<HGDIOBJ>::iterator iter;
	for (iter = m_GdiObjList.begin(); iter != m_GdiObjList.end(); iter++)
    {
        DeleteObject(*iter);  // Delete any GDI objects we loaded
    }

}


//-----------------------------------------------------------------------------
// Name: ShowDialog()
// Desc: Displays the dialog
//
// hinst: Application instance
// hwnd:  Handle to the parent window. Use NULL if no parent.
//
// Returns TRUE if successful or FALSE otherwise
//-----------------------------------------------------------------------------
BOOL CBaseDialog::ShowDialog(HINSTANCE hinst, HWND hwnd)
{
    // Cache these...
    m_hinst = hinst;
    m_hwnd = hwnd;

    // Show the dialog. Pass a pointer to ourselves as the LPARAM
	INT_PTR ret = DialogBoxParam(hinst, MAKEINTRESOURCE(m_nID), 
		hwnd, DialogProc, (LPARAM)this);

	if (ret == 0)
	{
		ReportError(0, TEXT("Bad hwnd"));
		return FALSE;
	}
	if (ret == -1)
	{
		TCHAR szErr[32];
		wsprintf(szErr, TEXT("Error Code: %d"), GetLastError());
		ReportError(0, szErr);
		return FALSE;
	}
	return (IDOK == ret);
}


//-----------------------------------------------------------------------------
// Name: DialogProc()
// Desc: DialogProc for the dialog. This is a static class method.
//
// lParam: Pointer to the CBaseDialog object. 
//
// The CBaseDialog class specifies lParam when it calls DialogBoxParam. We store the 
// pointer as user data in the window. 
//
// (Note: The DirectShow CBasePropertyPage class uses the same technique.)
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CBaseDialog::DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CBaseDialog *pDlg = 0;  // Pointer to the dialog class that manages the dialog 

    if (msg == WM_INITDIALOG)
    {
        // Get the pointer to the dialog object and store it in 
        // the window's user data

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG)lParam);
        pDlg = (CBaseDialog*)lParam;
        if (pDlg)
        {
            pDlg->m_hDlg = hDlg;
            pDlg->OnInitDialog();
        }
        return FALSE;
    }

    // Get the dialog object from the window's user data
    pDlg = (CBaseDialog*)(DWORD_PTR) GetWindowLongPtr(hDlg, DWLP_USER);

    if (msg == WM_COMMAND)
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (!pDlg || pDlg->OnOK())
			{
		        EndDialog(hDlg, LOWORD(wParam));
			}
			return TRUE;

        case IDCANCEL:
            if (!pDlg || pDlg->OnCancel())
			{
		        EndDialog(hDlg, LOWORD(wParam));
			}
            return TRUE;
        }
    }

    if (pDlg)
    {
        // Let the object handle the message
        return pDlg->OnReceiveMsg(hDlg, msg, wParam, lParam);
    }
    else
    {
        return FALSE;
    }
}


//-----------------------------------------------------------------------------
// Name: RedrawControl()
// Desc: Repaints a control
//
// nID: Resource ID of the control
//-----------------------------------------------------------------------------

void CBaseDialog::RedrawControl(int nID)
{
    // Find the dialog rect and the control rect, both relative to the display
	RECT rcDlg, rcControl;
	GetWindowRect(m_hDlg, &rcDlg);
	GetWindowRect(GetDlgItem(nID), &rcControl);

    // Adjust the dialog rect by the size of the border and caption 
	rcDlg.top += SystemBorderHeight();
	rcDlg.left += SystemBorderWidth();
	rcDlg.top += SystemCaptionHeight();

    // Find the dialog rect relative to the dialog position
	OffsetRect(&rcControl, - rcDlg.left, - rcDlg.top);

	InvalidateRect(m_hDlg, &rcControl, TRUE);
	UpdateWindow(m_hDlg);
}


//-----------------------------------------------------------------------------
// Name: SetWindowBitmap()
// Desc: Load a bitmap resource and set it on a control window
//
// nControlID: resource ID of the control
// nBitmapID:  resource ID of the bitmap
//-----------------------------------------------------------------------------

void CBaseDialog::SetWindowBitmap(int nControlID, int nBitmapID)
{
    HBITMAP hBitmap = SetBitmapImg(m_hinst, nBitmapID, GetDlgItem(nControlID));

    if (hBitmap)
    {
        m_GdiObjList.push_back(hBitmap);  // Store in our list for later deletion.
    }
}

