// eventapp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <mmsystem.h>
#include <dshow.h>
#include "commdlg.h"
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>

#include "resource.h"

#define MAX_LOADSTRING 100

#define FILE_FILTER_TEXT \
    TEXT("Video Files (*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v)\0*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v\0")\
    TEXT("Audio files (*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd)\0*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd\0")\
    TEXT("WMT Files (*.asf; *.wma; *.wmv)\0*.asf; *.wma; *.wmv\0")\
    TEXT("MIDI Files (*.mid, *.midi, *.rmi)\0*.mid; *.midi; *.rmi\0") \
    TEXT("Image Files (*.jpg, *.bmp, *.gif, *.tga)\0*.jpg; *.bmp; *.gif; *.tga\0") \
    TEXT("All Files (*.*)\0*.*;\0\0")

#define DEFAULT_MEDIA_PATH  TEXT("\\\0")

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text


  HWND      hWnd;
  HINSTANCE ghInst;
  HMENU     ghMenu;
 
  HRESULT   hr;
  LONG      evCode;
  LONG      evParam1;
  LONG      evParam2;

  TCHAR     time[15];
  HDC       hDC; 	
	
  int       m_pos =30;
  // Collection of interfaces
  IBaseFilter   *pif   = NULL;
  IGraphBuilder *pigb  = NULL;
  IMediaControl *pimc  = NULL;
  IMediaEventEx *pimex = NULL;
  IVideoWindow  *pivw  = NULL;
// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


#define APPLICATIONNAME "PlayFile"
#define CLASSNAME "PlayFile"
#define WM_GRAPHNOTIFY  WM_USER+13
#define HELPER_RELEASE(x) { if (x) x->Release(); x = NULL; }

 
void PlayFile (LPSTR szFile)
 
{ // PlayFile //
 
      WCHAR wFile[MAX_PATH];

#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, szFile, -1, wFile, MAX_PATH);
#else
    lstrcpy(wFile, szFile);
#endif

	  CoInitialize(NULL);
      hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pigb);
      if (SUCCEEDED(hr)) 
      { // Graphbuilder instance

        // QueryInterface for some basic interfaces
        pigb->QueryInterface(IID_IMediaControl, (void **)&pimc);
        pigb->QueryInterface(IID_IMediaEventEx, (void **)&pimex);
        pigb->QueryInterface(IID_IVideoWindow, (void **)&pivw);
		
		pivw->put_Owner ((OAHWND)hWnd);
		pivw->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
 
        // Have the graph construct its the appropriate graph automatically
        hr = pigb->RenderFile(wFile, NULL);
		
        // Have the graph signal event via window callbacks for performance
        pimex->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);
 
          // Let's get ready to rumble!
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
        SetForegroundWindow(hWnd);
        SetFocus(hWnd);
        // Run the graph if RenderFile succeeded
      
		
		if (SUCCEEDED(hr))
          pimc->Run();
         pimex->CancelDefaultHandling (EC_NOTIFY_WINDOW);
		 MessageBox (NULL, "pause", NULL,NULL);
		 
		 pimc->Pause();
      } // Graphbuilder instance
 
    } // PlayFile //
 
  // ---------------------------------------------------------------------------
  // End of multimedia specific code.
  // ---------------------------------------------------------------------------
 
  BOOL GetClipFileName(LPSTR szName)
 
    { // GetClipFileName //
 
      static OPENFILENAME ofn={0};
    static BOOL bSetInitialDir = FALSE;

    *szName = 0;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hWnd;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrFilter       = FILE_FILTER_TEXT;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrTitle        = TEXT("Open Media File...\0");
    ofn.lpstrFileTitle    = NULL;
    ofn.lpstrDefExt       = TEXT("*\0");
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;
    
    // Remember the path of the first selected file
    if (bSetInitialDir == FALSE)
    {
        ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH;
        bSetInitialDir = TRUE;
    }
    else 
        ofn.lpstrInitialDir = NULL;

    return GetOpenFileName((LPOPENFILENAME)&ofn);
    } // GetClipFileName //
 


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_EVENTAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_EVENTAPP);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_EVENTAPP);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_EVENTAPP;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_OPEN:
				     TCHAR szFilename[MAX_PATH];
 
                     GetClipFileName(szFilename); 
					 PlayFile(szFilename);
                   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			 
			break;
		case WM_GRAPHNOTIFY:
 
            while (SUCCEEDED(pimex->GetEvent(&evCode, &evParam1, &evParam2, 0)))
 
              { 
					// Spin through the events
              hr = pimex->FreeEventParams(evCode, evParam1, evParam2);		   

              switch (evCode)  
				{
				 
			     case EC_COMPLETE:
				  					
					MessageBox (NULL, "EC_COMPLETE", NULL, NULL);	
					 
					 // Finished
                    pivw->put_Visible(OAFALSE);
 
                    HELPER_RELEASE(pivw);
                    HELPER_RELEASE(pif);
                    HELPER_RELEASE(pigb);
                    HELPER_RELEASE(pimc);
                    HELPER_RELEASE(pimex);
                    break;
 
				  case EC_BUFFERING_DATA:
					
				     MessageBox (NULL, "EC_BUFFERING_DATA", NULL, NULL);
					 break;
					 
				  case EC_CLOCK_CHANGED:
					 MessageBox (NULL, "EC_CLOCK_CHANGED", NULL, NULL);
					 break;
				  
				  case EC_END_OF_SEGMENT:
					 MessageBox (NULL, "EC_END_OF_SEGMENT", NULL, NULL);
					 break;
				  
				  case EC_ERROR_STILLPLAYING:

					 MessageBox (NULL, "EC_ERROR_STILLPLAYING", NULL, NULL);
					 break;

				  case EC_ERRORABORT:

					 MessageBox (NULL, "EC_ERROR_STILLPLAYING", NULL, NULL);
					 break;
				
				  case EC_FULLSCREEN_LOST:

					 MessageBox (NULL, "EC_ERROR_STILLPLAYING", NULL, NULL);
					 break;
				  
				  case EC_NEED_RESTART:

				     MessageBox (NULL, "EC_NEED_RESTART", NULL, NULL);
					 break;
				  
				  case EC_NOTIFY_WINDOW:
					  
					 MessageBox (NULL, "EC_NOTIFY_WINDOW", NULL, NULL);
					 break;
				   
				  case EC_OLE_EVENT:

					 MessageBox (NULL, "EC_OLE_EVENT", NULL, NULL);
					 break;
				
				  case EC_OPENING_FILE:

					 MessageBox (NULL, "EC_OPENING_FILE", NULL, NULL);
					 break;
				  
				  case EC_PALETTE_CHANGED:

					 MessageBox (NULL, "EC_PALETTE_CHANGED", NULL, NULL);
					 break;
				  
				  case EC_QUALITY_CHANGE:

					 MessageBox (NULL, "EC_QUALITY_CHANGE", NULL, NULL);
					 break;
				  
				  case EC_REPAINT:

					  MessageBox (NULL, "EC_REPAINT", NULL, NULL);
					  break;
				  
				  case EC_SEGMENT_STARTED:
					  MessageBox (NULL, "EC_SEGMENT_STARTED", NULL, NULL);
					  break;		
					
				  case EC_SHUTTING_DOWN:
					  MessageBox (NULL, "EC_SHUTTING_DOWN", NULL, NULL);
					  break;

				  case EC_STARVATION:
					  MessageBox (NULL, "EC_STARVATION", NULL, NULL);
					  break;
			  
				  case EC_STREAM_CONTROL_STARTED:
					  MessageBox (NULL, "EC_STREAM_CONTROL_STARTED", NULL, NULL);
					  break;
				
				  case EC_STREAM_CONTROL_STOPPED:
					  MessageBox (NULL, "EC_STREAM_CONTROL_STARTED", NULL, NULL);
					  break;

				  case EC_STREAM_ERROR_STILLPLAYING:
					  MessageBox (NULL, "EC_STREAM_ERROR_STILLPLAYING", NULL, NULL);
					  break;	
					  
				  case EC_STREAM_ERROR_STOPPED:
					  MessageBox (NULL, "EC_STREAM_ERROR_STOPPED", NULL, NULL);
					  break;	
				  
				  case EC_TIME:
					  MessageBox (NULL, "EC_TIME", NULL, NULL);
					  break;

				  case EC_USERABORT:
					  
					  MessageBox (NULL, "EC_USERABORT", NULL, NULL);
					  break;

				  case EC_VIDEO_SIZE_CHANGED:
					  MessageBox (NULL, "EC_VIDEO_SIZE_CHANGED", NULL, NULL);
					  break;
				  
				 /* case EC_VIDEO_SIZE_AR_CHANGED:

					  MessageBox (NULL, "EC_VIDEO_SIZE_AR_CHANGED", NULL, NULL);
					  break;*/
				  
				  case EC_WINDOW_DESTROYED:
					  MessageBox (NULL, "EC_WINDOW_DESTROYED", NULL, NULL);
					  break;


			  }   


              }  
            break;

		
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
