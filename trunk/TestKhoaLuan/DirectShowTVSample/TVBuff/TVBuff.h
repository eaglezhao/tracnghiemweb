//------------------------------------------------------------------------------
// File: DSWebcamCap.h
//
// Desc: Use DirectShow to render media files - header file.
//
// Copyright (c) 2002, Microsoft Press.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _DSWEBCAMCAP_H_
#define _DSWEBCAMCAP_H_

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
#include <dbt.h>
#include <atlbase.h>

#define __EDEVDEFS__    //don't include edevdefs.h

#include <mmreg.h>
#include <mmstream.h>
#include <initguid.h>
#include <xprtdefs.h>   //include this instead of edevdefs
#include <dshow.h>
#include <sbe.h>		// Stream Buffer Engine includes - MUST HAVE THIS FILE INCLUDED!
#include <dshowasf.h>	// We're configuring ASF files

#ifdef DEBUG
#define SAFE_RELEASE(pUnk) if (pUnk) \
{ \
    pUnk->Release(); \
    pUnk = NULL; \
} \
else \
{ \
}
#else
#define SAFE_RELEASE(pUnk) if (pUnk)\
{\
    pUnk->Release();\
    pUnk = NULL;\
}
#endif //DEBUG

#endif //_DSWEBCAMCAP_H_
