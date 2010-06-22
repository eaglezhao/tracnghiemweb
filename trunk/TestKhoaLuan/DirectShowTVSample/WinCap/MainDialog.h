#pragma once

#include "dialog.h"
#include "graph.h"
#include "Device.h"
#include "ExtDevice.h"
#include "CTunerProp.h"
#include "CDVProp.h"

/******************************************************************************
 *  CMainDialog Class
 *  This is the main dialog for the app.
 *****************************************************************************/

class CMainDialog : public CNonModalDialog, public CGraphEventHandler
{
private:
	CDeviceList     m_DevList;		// Manages the device list
	CCaptureGraph   *m_pGraph;		// Manages the graph

	TCHAR m_szCaptureFile[MAX_PATH + 1];
	bool  m_bCapturing;

	void SetInitialDialogState();
	void SetCaptureFileName();

	HRESULT RefreshDeviceList();
	HRESULT StartPreview();
	void OnSelectDevice();
	void OnSelectAudioDevice();
	void OnConfig();
	void OnCapture(HWND hCaptureButton);

public:
	CMainDialog();

	HRESULT InitGraph();

	// message handlers
    HRESULT OnInitDialog();
	BOOL OnOK();
	INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    // Used by HandleEvent
	void OnGraphEvent(long lEvent, long lParam1, long lParam2);

	// Non-modal dialogs for device-specific function
	CTunerProp TVProp;
	CDVProp    DVProp;

	// Pass filename in wszPath, saves filter graph to that file
	HRESULT SaveGraphFile(WCHAR* wszPath);
};


