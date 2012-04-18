//
// (C) Copyright 1998-2007 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// ComPolygon.h : Declaration of the CComPolygon

#pragma once

#include "resource.h"       // main symbols
#include "dynprops.h"
#include "axtempl.h"
#include "tchar.h"

// DISPIDs for resources
#define	DISPID_BASEPOINT		0x00000001
#define	DISPID_NOTEGRIP		0x00000002
#define	DISPID_POS		0x00000003
#define	DISPID_COUNT		0x00000004
#define	DISPID_DIAMETER		0x00000005
#define	DISPID_SPACING		0x00000006
#define	DISPID_MULTIPLIER		0x00000007
#define	DISPID_SHOWLENGTH		0x00000008
#define	DISPID_NOTE		0x00000009
#define	DISPID_A		0x0000000A
#define	DISPID_B		0x0000000B
#define	DISPID_C		0x0000000C
#define	DISPID_D		0x0000000D
#define	DISPID_E		0x0000000E
#define	DISPID_F		0x0000000F
#define	DISPID_LENGTH		0x00000010
#define	DISPID_SHAPE		0x00000011
#define	DISPID_GROUP		0x00000012

/////////////////////////////////////////////////////////////////////////////
// CComPolygon
class ATL_NO_VTABLE CComPolygon : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComPolygon, &CLSID_ComPolygon>,
	public ISupportErrorInfo,
	public IAcadEntityDispatchImpl<CComPolygon, &CLSID_ComPolygon, 
                                   IComPolygon, &IID_IComPolygon, 
                                   &LIBID_COMPOLYLib>,
    //These are the OPM related interfaces. These interfaces
    //are usually not used by any other ObjectDBX host than 
    //AutoCAD, although they could be. AutoCAD uses them
    //to customize the display of this object's properties
    //in the Object Property Manager pane.
    public IOPMPropertyExtensionImpl<CComPolygon>,
    //If a property is VT_ARRAY then IOPMPropertyExpander is QueryInterface-d for
    public IOPMPropertyExpander
{
public:
	CComPolygon()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMPOLYGON)
DECLARE_NOT_AGGREGATABLE(CComPolygon)

BEGIN_COM_MAP(CComPolygon)
	COM_INTERFACE_ENTRY(IComPolygon)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IAcadBaseObject)
    COM_INTERFACE_ENTRY(IAcadBaseObject2)
	COM_INTERFACE_ENTRY(IAcadObject)
	COM_INTERFACE_ENTRY(IAcadEntity)
    COM_INTERFACE_ENTRY(IRetrieveApplication)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IOPMPropertyExtension)
    COM_INTERFACE_ENTRY(ICategorizeProperties)
    COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
    COM_INTERFACE_ENTRY(IOPMPropertyExpander)
END_COM_MAP()


// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IAcadBaseObjectImpl
    virtual HRESULT CreateNewObject(AcDbObjectId& objId, AcDbObjectId& ownerId, TCHAR* keyName);

// IAcadBaseObject2Impl
    STDMETHOD(ForceDbResident)(VARIANT_BOOL *forceDbResident);
    STDMETHOD(AddToDb)(AcDbObjectId& objId, AcDbObjectId ownerId = AcDbObjectId::kNull, TCHAR* keyName = NULL);
    STDMETHOD(CreateObject)(AcDbObjectId ownerId = AcDbObjectId::kNull, TCHAR *keyName = NULL);

// IComPolygon
public:
	STDMETHOD(get_BasePoint)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_BasePoint)(/*[in]*/ VARIANT newVal);
	STDMETHOD(get_NoteGrip)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_NoteGrip)(/*[in]*/ VARIANT newVal);
	STDMETHOD(get_Pos)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Pos)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Count)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Count)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Diameter)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Diameter)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Spacing)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Spacing)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Multiplier)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Multiplier)(/*[in]*/ long newVal);
	STDMETHOD(get_ShowLength)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_ShowLength)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Note)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Note)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_A)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_A)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_B)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_B)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_C)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_C)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_D)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_D)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_E)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_E)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_F)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_F)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Length)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Shape)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Shape)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Group)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Group)(/*[in]*/ BSTR newVal);
//
// OPM
//

// IOPMPropertyExtension
BEGIN_OPMPROP_MAP()
    OPMPROP_ENTRY(0, DISPID_BASEPOINT, PROPCAT_Position, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_NOTEGRIP, PROPCAT_Position, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_POS, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_COUNT, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_DIAMETER, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_SPACING, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_MULTIPLIER, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_SHOWLENGTH, PROPCAT_Appearance, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_NOTE, PROPCAT_Text, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_A, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_B, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_C, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_D, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_E, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_F, PROPCAT_Data, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_LENGTH, PROPCAT_Data, 0, 0, 0, _T(""), 0, 0, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_SHAPE, PROPCAT_Misc, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
    OPMPROP_ENTRY(0, DISPID_GROUP, PROPCAT_Misc, 0, 0, 0, _T(""), 0, 1, IID_NULL, IID_NULL, "")
END_OPMPROP_MAP()


    STDMETHOD(GetCategoryName)(THIS_
                           /* [in]  */ PROPCAT propcat, 
                           /* [in]  */ LCID lcid,
                           /* [out] */ BSTR* pbstrName) { return S_FALSE;}

    //Override to make property read-only
	STDMETHOD(Editable)( 
		/* [in] */ DISPID dispID,
		/* [out] */ BOOL __RPC_FAR *bEditable);

    //Override to hide the property from display
	STDMETHOD(ShowProperty)(
		/* [in] */ DISPID dispID, 
		/* [out] */ BOOL *pShow);

    virtual HINSTANCE GetResourceInstance()
    {
        return _Module.GetResourceInstance();
    }
    //Used for property expansion (currently variant types)
	STDMETHOD(GetElementValue)(
		/* [in] */ DISPID dispID,
		/* [in] */ DWORD dwCookie,
		/* [out] */ VARIANT * pVarOut);

    //Used for property expansion (currently variant types)
	STDMETHOD(SetElementValue)(
		/* [in] */ DISPID dispID,
		/* [in] */ DWORD dwCookie,
		/* [in] */ VARIANT VarIn);       

    //Used for property expansion (currently variant types)
	STDMETHOD(GetElementStrings)( 
		/* [in] */ DISPID dispID,
		/* [out] */ OPMLPOLESTR __RPC_FAR *pCaStringsOut,
		/* [out] */ OPMDWORD __RPC_FAR *pCaCookiesOut);

    //Used for property expansion (currently variant types)
    STDMETHOD(GetElementGrouping)(
        /* [in] */ DISPID dispID,
		/* [out] */ short *groupingNumber);

    //Used for property expansion (currently variant types)
    STDMETHOD(GetGroupCount)(
        /* [in] */ DISPID dispID,
		/* [out] */ long *nGroupCnt);
    STDMETHOD(GetPredefinedStrings)(
        /* [in] */ DISPID dispID,
        /* [out] */ CALPOLESTR *pCaStringsOut,
        /* [out] */ CADWORD *pCaCookiesOut);
    STDMETHOD(GetPredefinedValue)(
        /* [in] */ DISPID dispID, 
        /* [out] */ DWORD dwCookie, 
        /* [out] */ VARIANT *pVarOut);

    protected:
        AcDbObjectIdArray mObjectIdArray;
};

