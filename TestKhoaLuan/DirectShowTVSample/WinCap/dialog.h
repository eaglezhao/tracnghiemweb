#pragma once

#include "graph.h"

/******************************************************************************
 *
 *  CBaseDialog Class
 *  Implements a Win32 dialog. Defaults to a modal dialog; for non-modal
 *  dialogs, use CNonModalDialog.
 *
 *  Examle of usage:
 *
 *  Derive a new class from CBaseDialog
 *      class CMyDialog : public CBaseDialog
 *
 *  Create an instance of the class:
 *	    CMyDialog *pDlg = new CMyDialog();
 *
 *  Call ShowDialog to show the dialog:
 *      if (pDlg)
 *	    {
 *          pDlg->ShowDialog(m_hinst, m_hwndParent); 
 *      }
 *
 *  (This is all rather MFC-ish...)
 *****************************************************************************/

class CBaseDialog
{
protected:
    HINSTANCE   m_hinst;    // application instance
    HWND        m_hwnd;     // parent window - can be NULL
    HWND        m_hDlg;     // this dialog window
    int         m_nID;      // Resource ID of the dialog window 
                            // (Set this in the constructor)

    vector<HGDIOBJ> m_GdiObjList;   // Holds a list of GDI objects that need destroying

protected:

    // Dialog proc for the dialog we manage
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Return one of our dialog controls
	HWND GetDlgItem(int nID) { return ::GetDlgItem(m_hDlg, nID); }
    
    // Redraw a control
	void RedrawControl(int nID);

    void SetWindowBitmap(int nControlID, int nBitmapID);

    // Enable or diable a menu item
    void EnableMenuItem(int nID, BOOL bEnable)
    {
        ::EnableMenuItem(GetMenu(m_hDlg), nID, (bEnable ? MF_ENABLED : MF_GRAYED));
    }

    // Override the following to handle various window messages

    // WM_INIT_DIALOG
    virtual HRESULT OnInitDialog() { return S_OK; }   

    // IDOK and IDCANCEL. Return TRUE to close the dialog or FALSE to leave it open
    virtual BOOL OnOK() { return TRUE; }
    virtual BOOL OnCancel() { return TRUE; }

    // All other messages
    virtual INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return FALSE;
    }


public:
    CBaseDialog(int nID);
    virtual ~CBaseDialog();

    virtual BOOL ShowDialog(HINSTANCE hinst, HWND hwnd);
};


/******************************************************************************
 *
 *  CNonModalDialog Class
 *  Implements a non-modal dialog. 
 *
 *  The usage is somewhat odd because your message loop gets the messages. e.g:
 *  
 *	CMyDialog dlg;
 *  dlg.ShowDialog(hInstance, hwndParant);
 *
 *	MSG msg;
 *	BOOL bRet;
 *	while (GetMessage(&msg, 0, 0, 0))
 *	{
 *		dlg.HandleMessage(&msg);  // returns TRUE if it handled the msg
 *  }
 *  
 *****************************************************************************/

class CNonModalDialog : public CBaseDialog
{

protected:

	bool m_bQuitOnClose;  // Quit the thread when we close?

    // Override OnOK and OnCancel because non-modal dialogs are closed
    // differently than modal dialogs
    virtual BOOL OnOK() 
	{
		EndDialog();
		return FALSE; 
	}
    virtual BOOL OnCancel() 
	{ 
		EndDialog();
		return FALSE; 
	}

    // OnEndDialog: Called when the dialog ends
    virtual void OnEndDialog() { };  // Do nothing, but the derived class might need to clean up.

public:

	CNonModalDialog(int nID) : CBaseDialog(nID), m_bQuitOnClose(false) { }

	BOOL ShowDialog(HINSTANCE hinst, HWND hwnd)
	{
		if (m_hDlg)
		{
			// We're already visible ... nothing to do.
			return TRUE;
		}
	    m_hinst = hinst;
		m_hwnd = hwnd;

		m_hDlg = CreateDialogParam(hinst, MAKEINTRESOURCE(m_nID), hwnd, DialogProc, (LPARAM)this);
		if (m_hDlg == 0)
		{
			return FALSE;
		}
		else
		{
			ShowWindow(m_hDlg, SW_SHOW);
			return TRUE;
		}
	}

	void EndDialog()
	{   
		if (m_hDlg)
		{
            OnEndDialog();
    
			DestroyWindow(m_hDlg);
			m_hDlg = 0;

			if (m_bQuitOnClose) PostQuitMessage(0);
		}
	}

	BOOL HandleMessage(MSG *pmsg)
	{
		if (!m_hDlg) 
		{
			return FALSE;
		}
		else
		{
			return IsDialogMessage(m_hDlg, pmsg);
		}
	}

    // Toggle whether the dialog should quit the app/thread when it closes.
    // By default this is FALSE.
	void QuitOnClose(bool bVal) { m_bQuitOnClose = bVal; }

};


