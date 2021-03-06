
#include "StdAfx.h"

#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include "mgdinterop.h" 

#include "MgPosShape.h"
#include "MgPosGroup.h"
#include "MgRebarPos.h"
#include "MgBOQStyle.h"
#include "MgBOQTable.h"

static AcMgObjectFactoryBase **g_PEs = NULL;

extern "C" AcRx::AppRetCode __declspec(dllexport)
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
	switch(msg) 
	{
		// onload of arx
		case AcRx::kInitAppMsg: 
		{
			acrxDynamicLinker->registerAppMDIAware(pkt);
			acrxDynamicLinker->loadModule(L"NativeRebarPos.dbx", Adesk::kTrue, Adesk::kFalse);
			acrxDynamicLinker->loadModule(L"COMRebarPos.dbx", Adesk::kTrue, Adesk::kFalse);
			
			// create a new object factory array
			static AcMgObjectFactoryBase* PEs[] = 
			{
				new AcMgObjectFactory<OZOZ::RebarPosWrapper::PosShape, CPosShape>(), 
				new AcMgObjectFactory<OZOZ::RebarPosWrapper::PosGroup, CPosGroup>(), 
				new AcMgObjectFactory<OZOZ::RebarPosWrapper::RebarPos, CRebarPos>(), 
				new AcMgObjectFactory<OZOZ::RebarPosWrapper::BOQStyle, CBOQStyle>(), 
				new AcMgObjectFactory<OZOZ::RebarPosWrapper::BOQTable, CBOQTable>(), 
				// end the array with a NULL
				NULL
			};

			g_PEs = PEs;
			
		}
		break;

		case AcRx::kUnloadAppMsg:
		{
			acrxDynamicLinker->unloadModule(L"COMRebarPos.dbx", Adesk::kFalse);
			acrxDynamicLinker->unloadModule(L"NativeRebarPos.dbx", Adesk::kFalse);
		}
		break;

		case AcRx::kPreQuitMsg:
		{
			// clean up
			int i = 0;
			while(g_PEs[i] != NULL)
				delete g_PEs[i++];
		}
		break;
	}

	return AcRx::kRetOK;
}
