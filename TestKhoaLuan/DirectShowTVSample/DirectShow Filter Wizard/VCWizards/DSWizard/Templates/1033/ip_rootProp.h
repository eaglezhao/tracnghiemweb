#pragma once

class C[!output PROJECT_NAME]Properties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();

    C[!output PROJECT_NAME]Properties(LPUNKNOWN lpunk, HRESULT *phr);

    BOOL m_bIsInitialized;													// Used to ignore startup messages
    I[!output PROJECT_NAME] *m_pI[!output PROJECT_NAME];				// The custom interface on the filter
	[!output PROJECT_NAME]Parameters [!output PROJECT_NAME]Parameters;	// transform parameters
};

