#pragma once

#include "Dialog.h"


/******************************************************************************
 *  CVideoConfig Class
 *  Helper class to configure the output format on a video capture filter.
 *
 *  This class is specifically to manage the IAMStreamConfig interface, because
 *  that interface is somewhat complicated.
 *
 *  Note that IAMStreamConfig is supported by (some) WDM devices, but not by old VfW drivers.
 *****************************************************************************/

class CVideoConfig
{
private:
	CComPtr<IAMStreamConfig> m_pConfig;  // Use this to set the output format
	int m_iCount;   // Number of formats supported
	int m_iSize;    // Size of the XXX_STREAM_CONFIG structure

protected:
	AM_MEDIA_TYPE *m_pmtConfig;      // Holds one format
	VIDEO_STREAM_CONFIG_CAPS m_scc;  // Holds possible modifications of the format

	void DumpFormat();  
	bool HasConfig() { return (m_pConfig != NULL); } 

	BITMAPINFOHEADER * const BitMapInfo();
	// Returns a pointer to the BITMAPINFOHEADER of the config format
	// (or NULL if it cannot find one) 

	// WARNING! It returns a pointer into the format block of 
	// m_pmtConfig ... don't hang onto this after the format changes.

public:
	CVideoConfig();
	virtual ~CVideoConfig();
	int Count() { return m_iCount; }  // Returns the number of formats
	int Size() { return m_iSize; }    // Returns the size of the format config structure

	HRESULT Init(IAMStreamConfig *pConfig);

	HRESULT GetCap(int iCap);
	HRESULT SetFormat();
};


/******************************************************************************
 *  CConfigDialog Class
 *  Shows a dialog where the user can set properties on the video capture filter.
 *
 *  This dialog uses the following DirectShow interfaces:
 *  IIPDVDec - Set the decoding resolution on the DV Decoder
 *  IAMVideoProcAmp - Set brightness, contrast, etc (WDM only)
 *  IAMStreamConfig - Set the output format (WDM only)
 *  IAMVfwCaptureDialogs - Show VfW dialogs (VfW only)
 *
 *****************************************************************************/

class CConfigDialog : public CBaseDialog, private CVideoConfig
{
private:
	CCaptureGraph *m_pGraph;
	CComPtr<IIPDVDec> m_pDvDec;
	CComPtr<IAMVideoProcAmp> m_pProcAmp;
	CComPtr<IAMVfwCaptureDialogs> m_pVfw;

	int m_iDisplay;  // Current DV Format

	
	HRESULT ShowVfwDialog(VfwCaptureDialogs iDialog);  // Show a VFW dialog
	void	RestoreProcAmpDefaults();    // Reset the VideoProcAmp default values

	// For video format configuration
	HRESULT	PopulateVideoTypeList();
	HRESULT	SetSpinValues(int iCap);
	void    CacheVideoFormat();  // Save the IAMStreamConfig foramt setting, for later use

public:
	CConfigDialog(CCaptureGraph *pGraph);
    HRESULT OnInitDialog();
	BOOL OnOK();
	INT_PTR OnReceiveMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT SetVideoFormat();    // Set the output format.    
	HRESULT SetDVDecoding();	 // Set the DV decoding resolution

};