// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#include <dshow.h>
#include <qedit.h>
#include <Dvdmedia.h.>  // Defines VIDEOINFOHEADER2

#include <atlbase.h>    // Smart pointers
#include <Commdlg.h>    // Common dialogs
#include <commctrl.h>   // Common controls

#include <vector>
#include <iterator>

using namespace std;


struct bad_hresult
{
	HRESULT hr;
	bad_hresult(HRESULT hr) { this->hr = hr; }
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#include "utils.h"
