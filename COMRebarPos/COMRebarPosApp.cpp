// COMRebarPosApp.cpp : Implementation of DLL Exports.

#include "StdAfx.h"

#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include "resource.h"
#include "initguid.h"

#include "COMRebarPos_i.h"
#include "COMRebarPos_i.c"

#include "ComRebarPos.h"
#include "ComBOQTable.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ComRebarPos, CComRebarPos)
	OBJECT_ENTRY(CLSID_ComBOQTable, CComBOQTable)
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
		// Check if the arx app is loaded or not.If not, load it as UI so that we won't have proxy if this dll is unloaded by OS
		if (!::acrxAppIsLoaded(_T("NativeRebarPos.dbx")))
		{
			if (!acrxLoadModule(_T("NativeRebarPos.dbx"), false, true))
				return FALSE; // This will trigger a DLL_PROCESS_DETACH right away
		}
		//bump the reference count 
		acrxLoadModule(_T("NativeRebarPos.dbx"), false, false);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();

		// Try to decrease the refcount on the dbx. If we couldn't load it then this a no op.
		acrxUnloadModule(_T("NativeRebarPos.dbx"));
    }
    return TRUE; 
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
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
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//make this additional entry point available so when user loads
//this dll as an arx it will register itself 
extern "C" AcRx::AppRetCode __declspec(dllexport)
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt) 
{
    switch (msg) 
	{
    case AcRx::kInitAppMsg:
        //unlock the application
        acrxDynamicLinker->unlockApplication(pkt);
        acrxRegisterAppMDIAware(pkt);
		//register ourselves
		DllRegisterServer();
        break;
    case AcRx::kUnloadAppMsg:
        break;
    case AcRx::kOleUnloadAppMsg :
        //respond to this message according to our current state
        return (DllCanUnloadNow() == S_OK ? AcRx::kRetOK : AcRx::kRetError);
    }
    return AcRx::kRetOK;
}
