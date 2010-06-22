#pragma once
#include "dialog.h"
#include "device.h"
#include "graph.h"


/******************************************************************************
 *  CAudioCapDlg Class
 *  Creates a dialog for selecting an audio capture device.
 *****************************************************************************/


class CAudioCapDlg : public CBaseDialog
{
private:
	CDeviceList m_AudioDevices;  // Holds a list of audio capture devices.
    CCaptureGraph *m_pGraph;     // Manages the graph.

    HRESULT PopulatePinList();

protected:
    INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CAudioCapDlg(CCaptureGraph *pGraph);
	~CAudioCapDlg(void);

	HRESULT OnInitDialog();
	BOOL OnOK();
};
