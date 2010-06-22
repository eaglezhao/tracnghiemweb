//------------------------------------------------------------------------------
// File: DSRender.h
//
// Desc: Use DirectShow to render media files - header file.
//
// Copyright (c) 2002, Microsoft Press.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _DSAUDIOCAP_H_
#define _DSAUDIOCAP_H_

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
#include <dbt.h>

#define __EDEVDEFS__    //don't include edevdefs.h

#include <mmreg.h>
#include <mmstream.h>
#include <initguid.h>
#include <xprtdefs.h>   //include this instead of edevdefs
#include <dshow.h>
#include <qedit.h>		// Editing headers

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

#endif //_DSAUDIOCAP_H_
