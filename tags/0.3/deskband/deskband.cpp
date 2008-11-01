// deskband.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f deskbandps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"

#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#pragma data_seg()

#include "sample.h"

// 8e0d7e18-8018-4050-980d-36d3f20dd7c7
DEFINE_GUID(CLSID_SampleDeskBand,0x8e0d7e18,0x8018,0x4050,0x98,0x0d,0x36,0xd3,0xf2,0x0d,0xd7,0xc7);

// 89796427-6c3d-4710-951f-9dfb0d702da8
//DEFINE_GUID(CLSID_SampleVerticalBar,0x89796427,0x6c3d,0x4710,0x95,0x1f,0x9d,0xfb,0x0d,0x70,0x2d,0xa8);

// 8858d47b-8864-4737-83d9-cd101436d639
//DEFINE_GUID(CLSID_SampleHorizontalBar,0x8858d47b,0x8864,0x4737,0x83,0xd9,0xcd,0x10,0x14,0x36,0xd6,0x39);

CShellModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SampleDeskBand, CSampleDeskBand)
//OBJECT_ENTRY(CLSID_SampleVerticalBar, CSampleVerticalBar)
//OBJECT_ENTRY(CLSID_SampleHorizontalBar, CSampleHorizontalBar)
END_OBJECT_MAP()

BEGIN_TYPE_MAP(TypeMap)
TYPE_ENTRY(CLSID_SampleDeskBand, DeskBand)
//TYPE_ENTRY(CLSID_SampleVerticalBar, VerticalExplorerBar)
//TYPE_ENTRY(CLSID_SampleHorizontalBar, HorizontalExplorerBar)
END_TYPE_MAP()

BEGIN_MENU_MAP(DeskBandMenu)
//MENU_ENTRY(IDM_DESKBAND_CMD,0,TEXT("&Sample Desk Band Cmd"),TEXT("Help text"),TEXT("sample"))
END_MENU_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(TypeMap, ObjectMap, hInstance);
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
    // registers desk band object
    return _Module.RegisterServer();
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer();
}
