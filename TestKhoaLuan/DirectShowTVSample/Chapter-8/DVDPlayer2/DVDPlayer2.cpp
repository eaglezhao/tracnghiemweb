// DVDPlayer2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <dshow.h>

HINSTANCE hInst;
IDvdGraphBuilder *m_pIDvdGB;
IDvdControl2 *m_pIDvdC2;
IMediaControl *m_pIMC;
	
IGraphBuilder* m_pGraph;
LRESULT CALLBACK	CmdWnd(HWND, UINT, WPARAM, LPARAM);


void InitDVD()
{
	
	CoInitialize (NULL);
		// Create an instance of the DVD Graph Builder object.
   HRESULT hr;
   hr = CoCreateInstance(CLSID_DvdGraphBuilder,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDvdGraphBuilder,
                          (void**)&m_pIDvdGB);

   // Build the DVD filter graph.
   AM_DVD_RENDERSTATUS	buildStatus;
   m_pIDvdGB->RenderDvdVideoVolume(NULL, AM_DVD_HWDEC_PREFER   , &buildStatus);
   m_pIDvdGB->GetDvdInterface(IID_IDvdControl2, (void**)&m_pIDvdC2);
   // Get a pointer to the filter graph manager.
   m_pIDvdGB->GetFiltergraph(&m_pGraph) ;
       
   m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pIMC);
   




}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  
	
	DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, NULL, (DLGPROC)CmdWnd);	
    return 0;
}



// Mesage handler for about box.
LRESULT CALLBACK CmdWnd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				InitDVD ();
				return TRUE;
		case WM_DESTROY:
			DestroyWindow (hDlg);
				//EndDialog (hDlg, NULL);
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{

			case IDC_PLAY: 
				m_pIMC->Run ();
				break;

			case IDC_STOP:
			    m_pIMC->Stop();
				break;

			case IDC_PAUSE:
				m_pIMC->Pause ();
				break;

			case IDC_RESUME:
				m_pIMC->Run ();
				break;

			case IDC_NEXT:
		 
            m_pIDvdC2->PlayNextChapter(DVD_CMD_FLAG_None, NULL);
		 
			break;
			
			case IDC_PREV:
			 
          //  m_pIDvdC2->PlayPreviousChapter(DVD_CMD_FLAG_None, NULL);
			 
			break;

			case IDC_REWIND:
			m_pIDvdC2->PlayBackwards(8.0, DVD_CMD_FLAG_None, NULL);
			break;

			case IDC_FF:
			m_pIDvdC2->PlayForwards(8.0, DVD_CMD_FLAG_None, NULL);
			break;

			case IDC_ROOTMENU:
			 
		    m_pIDvdC2->ShowMenu(DVD_MENU_Root, DVD_CMD_FLAG_Flush, NULL);
			
			break;
			}
		break;
	}
    return FALSE;
}
