#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "i[!output PROJECT_NAME].h"
#include "[!output PROJECT_NAME].h"
#include "[!output PROJECT_NAME]prop.h"

//
// CreateInstance
//
// Used by the DirectShow base classes to create instances
//
CUnknown *C[!output PROJECT_NAME]Properties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new C[!output PROJECT_NAME]Properties(lpunk, phr);
    if (punk == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return punk;

}

//
// Constructor
//
C[!output PROJECT_NAME]Properties::C[!output PROJECT_NAME]Properties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("[!output PROJECT_NAME] Property Page"),
                      pUnk,IDD_[!output PROJECT_NAME]PROP,IDS_TITLE),
    m_pI[!output PROJECT_NAME](NULL),
    m_bIsInitialized(FALSE)
{
    ASSERT(phr);
}

//
// OnReceiveMessage
//
// Handles the messages for our property window
//
BOOL C[!output PROJECT_NAME]Properties::OnReceiveMessage(HWND hwnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

}

//
// OnConnect
//
// Called when we connect to a transform filter
//
HRESULT C[!output PROJECT_NAME]Properties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pI[!output PROJECT_NAME] == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_I[!output PROJECT_NAME], (void **) &m_pI[!output PROJECT_NAME]);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pI[!output PROJECT_NAME]);

    m_pI[!output PROJECT_NAME]->get_[!output PROJECT_NAME](&m_[!output PROJECT_NAME]Parameters);
    m_bIsInitialized = FALSE ;
    return NOERROR;
}

//
// OnDisconnect
//
// Likewise called when we disconnect from a filter
//
HRESULT C[!output PROJECT_NAME]Properties::OnDisconnect()
{
    if (m_pI[!output PROJECT_NAME] == NULL) {
        return E_UNEXPECTED;
    }

    m_pI[!output PROJECT_NAME]->Release();
    m_pI[!output PROJECT_NAME] = NULL;
    return NOERROR;
}

//
// OnActivate
//
// We are being activated
//
HRESULT C[!output PROJECT_NAME]Properties::OnActivate()
{
    TCHAR   sz[60];

    _stprintf(sz, TEXT("%d"), m_[!output PROJECT_NAME]Parameters.param1);
    Edit_SetText(GetDlgItem(m_Dlg, IDC_PARAM1), sz);
    _stprintf(sz, TEXT("%d"), m_[!output PROJECT_NAME]Parameters.param2);
    Edit_SetText(GetDlgItem(m_Dlg, IDC_PARAM2), sz);

	m_bIsInitialized = TRUE;

	return NOERROR;
}

//
// OnDeactivate
//
// We are being deactivated
//
HRESULT C[!output PROJECT_NAME]Properties::OnDeactivate(void)
{
    ASSERT(m_pI[!output PROJECT_NAME]);
    m_bIsInitialized = FALSE;
    GetControlValues();
    return NOERROR;
}

//
// OnApplyChanges
//
// Apply any changes so far made
//
HRESULT C[!output PROJECT_NAME]Properties::OnApplyChanges()
{
    GetControlValues();
    m_pI[!output PROJECT_NAME]->put_[!output PROJECT_NAME](m_[!output PROJECT_NAME]Parameters);

    return NOERROR;
}

//
// GetControlValues
//
void C[!output PROJECT_NAME]Properties::GetControlValues()
{
    TCHAR sz[STR_MAX_LENGTH];

    Edit_GetText(GetDlgItem(m_Dlg, IDC_PARAM1), sz, STR_MAX_LENGTH);
	m_[!output PROJECT_NAME]Parameters.param1 = atoi(sz);

    Edit_GetText(GetDlgItem(m_Dlg, IDC_PARAM2), sz, STR_MAX_LENGTH);
	m_[!output PROJECT_NAME]Parameters.param2 = atoi(sz);
}
