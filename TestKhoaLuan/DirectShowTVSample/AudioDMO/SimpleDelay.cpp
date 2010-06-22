// SimpleDelay.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"

#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmoimpl.h>

#include <uuids.h> // Media types

#include <initguid.h>
#include "Delay.h"



CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Delay, CDelay)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	DMO_PARTIAL_MEDIATYPE mt;
	mt.type = MEDIATYPE_Audio;
	mt.subtype = MEDIASUBTYPE_PCM;

	DMORegister(L"Simple Delay",
				CLSID_Delay,
				DMOCATEGORY_AUDIO_EFFECT,
				0,		// Flags
				1,		// Number of input types to register
				&mt,	// Input types
				1,		// Number of output types to register
				&mt);	// Output types


    // registers object (no typelib)
    return _Module.RegisterServer();
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	DMOUnregister(CLSID_Delay, DMOCATEGORY_AUDIO_EFFECT);

    return _Module.UnregisterServer();
}


