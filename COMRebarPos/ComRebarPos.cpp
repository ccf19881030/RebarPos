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
#include "StdAfx.h"
// ComRebarPos.cpp : Implementation of CComRebarPos
#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include "COMRebarPos_i.h"
#include "ComRebarPos.h"
#include "dbsymtb.h"
#include "../NativeRebarPos/RebarPos.h"
#include "axpnt3d.h"
#include "axpnt2d.h"
#include "dbxutil.h"
#include "dbapserv.h"

#include "axlock.h"



#define AXEntityDocLockNoDbOk(objId)                        \
    AcAxDocLock docLock(objId, AcAxDocLock::kNormal);       \
    if (docLock.lockStatus() != Acad::eNoDatabase && \
        docLock.lockStatus() != Acad::eOk)                  \
        throw docLock.lockStatus();

#define AXEntityDocLock(objId)                              \
    AcAxDocLock docLock(objId, AcAxDocLock::kNormal);       \
    if(docLock.lockStatus() != Acad::eOk)                   \
        throw docLock.lockStatus();

/////////////////////////////////////////////////////////////////////////////
// CComRebarPos
#define CHECKOUTPARAM(x) if (x==NULL) return E_POINTER;

STDMETHODIMP CComRebarPos::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IComRebarPos,
        &IID_IAcadObject,
        &IID_IAcadEntity,
        &IID_IOPMPropertyExpander
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	}
	return S_FALSE;
}

//Override to make property read-only
STDMETHODIMP CComRebarPos::Editable( 
	/* [in] */ DISPID dispID,
	/* [out] */ BOOL __RPC_FAR *bEditable)
{
	return IOPMPropertyExtensionImpl<CComRebarPos>::Editable(dispID, bEditable);
}

//Override to hide the property from display
STDMETHODIMP CComRebarPos::ShowProperty(
	/* [in] */ DISPID dispID, 
	/* [out] */ BOOL *pShow)
{
	if(dispID >= DISPID_A && dispID <= DISPID_F)
	{
		try
		{
			Acad::ErrorStatus es;
			AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead, Adesk::kTrue);
			if((es = pPoly.openStatus()) != Acad::eOk)
				throw es;

			ACHAR* len = NULL;
			switch(dispID)
			{
			case DISPID_A:
		        acutUpdString(pPoly->A(), len);
				break;
			case DISPID_B:
		        acutUpdString(pPoly->B(), len);
				break;
			case DISPID_C:
		        acutUpdString(pPoly->C(), len);
				break;
			case DISPID_D:
		        acutUpdString(pPoly->D(), len);
				break;
			case DISPID_E:
		        acutUpdString(pPoly->E(), len);
				break;
			case DISPID_F:
		        acutUpdString(pPoly->F(), len);
				break;
			default:
				;
			}

			*pShow = (len != NULL) && (len[0] != _T('\0'));
			acutDelString(len);
			return S_OK;
		}
		catch(const Acad::ErrorStatus)
		{
			return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
		}
	}

	return IOPMPropertyExtensionImpl<CComRebarPos>::ShowProperty(dispID, pShow);
}

//This is used to get the value for an element in a group.
//The element is identified by the dwCookie parameter
STDMETHODIMP CComRebarPos::GetElementValue(
	/* [in] */ DISPID dispID,
	/* [in] */ DWORD dwCookie,
	/* [out] */ VARIANT * pVarOut)
{
    CHECKOUTPARAM(pVarOut);

    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead, Adesk::kTrue);
        if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

		AcGePoint3d pt;
		switch(dispID)
		{
		case DISPID_BASEPOINT:
			if (dwCookie > 2)
				throw Acad::eInvalidInput;

		    pt = pPoly->BasePoint();
            // TODO: translate from wcs to ucs
            //acdbEcs2Ucs(asDblArray(pt), asDblArray(pt), asDblArray(pPoly->normal()), Adesk::kFalse);
            ::VariantCopy(pVarOut, &CComVariant(pt[dwCookie]));

			return S_OK;
			break;
		case DISPID_NOTEGRIP:
			if (dwCookie > 2)
				throw Acad::eInvalidInput;

			pt = pPoly->NoteGrip();
            // TODO: translate from wcs to ucs
            //acdbEcs2Ucs(asDblArray(pt), asDblArray(pt), asDblArray(pPoly->normal()), Adesk::kFalse);
            ::VariantCopy(pVarOut, &CComVariant(pt[dwCookie]));

			return S_OK;
			break;
		default:
			;
        }
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument." ,IID_IComRebarPos, hr);
    }

	return E_NOTIMPL;
}

//This is used to set the value for an element in a group.
//The element is identified by the dwCookie parameter
STDMETHODIMP CComRebarPos::SetElementValue(
	/* [in] */ DISPID dispID,
	/* [in] */ DWORD dwCookie,
	/* [in] */ VARIANT VarIn)
{
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForWrite, Adesk::kTrue);
        if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

		AcGePoint3d pt;
		switch(dispID)
		{
		case DISPID_BASEPOINT:
			if (dwCookie > 2)
				throw Acad::eInvalidInput;

			pt = pPoly->BasePoint();

            //TODO: translate from wcs to ucs
            //acdbEcs2Ucs(asDblArray(pt),asDblArray(pt),asDblArray(pPoly->normal()),Adesk::kFalse);
	        pt[dwCookie] = V_R8(&VarIn);

		    if ((es = pPoly->setBasePoint(pt)) != Acad::eOk)
			    throw es;

			Fire_Notification(dispID);

			return S_OK;
			break;
		case DISPID_NOTEGRIP:
			if (dwCookie > 2)
				throw Acad::eInvalidInput;

			pt = pPoly->NoteGrip();

            //TODO: translate from wcs to ucs
            //acdbEcs2Ucs(asDblArray(pt),asDblArray(pt),asDblArray(pPoly->normal()),Adesk::kFalse);
	        pt[dwCookie] = V_R8(&VarIn);

		    if ((es = pPoly->setNoteGrip(pt)) != Acad::eOk)
			    throw es;

			Fire_Notification(dispID);

			return S_OK;
			break;
		default:
			;
        }
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }

	return E_NOTIMPL;
}

//This is called to get the display string for each
//element in a group.
STDMETHODIMP CComRebarPos::GetElementStrings( 
	/* [in] */ DISPID dispID,
	/* [out] */ OPMLPOLESTR __RPC_FAR *pCaStringsOut,
	/* [out] */ OPMDWORD __RPC_FAR *pCaCookiesOut)
{
    long size;

	switch(dispID)
	{
	case DISPID_BASEPOINT:
        size = 3;
        pCaStringsOut->pElems = (LPOLESTR *)::CoTaskMemAlloc(sizeof(LPOLESTR) * size);
        pCaCookiesOut->pElems = (DWORD *)::CoTaskMemAlloc(sizeof(DWORD) * size);
        for (long i = 0; i < size; i++)
            pCaCookiesOut->pElems[i] = i;
        pCaStringsOut->cElems = size;
        pCaCookiesOut->cElems = size;

        pCaStringsOut->pElems[0] = ::SysAllocString(L"Base point X");
        pCaStringsOut->pElems[1] = ::SysAllocString(L"Base point Y");
		pCaStringsOut->pElems[2] = ::SysAllocString(L"Base point Z");

		return S_OK;
		break;
	case DISPID_NOTEGRIP:
        size = 3;
        pCaStringsOut->pElems = (LPOLESTR *)::CoTaskMemAlloc(sizeof(LPOLESTR) * size);
        pCaCookiesOut->pElems = (DWORD *)::CoTaskMemAlloc(sizeof(DWORD) * size);
        for (long i = 0; i < size; i++)
            pCaCookiesOut->pElems[i] = i;
        pCaStringsOut->cElems = size;
        pCaCookiesOut->cElems = size;

        pCaStringsOut->pElems[0] = ::SysAllocString(L"Note grip X");
        pCaStringsOut->pElems[1] = ::SysAllocString(L"Note grip Y");
		pCaStringsOut->pElems[2] = ::SysAllocString(L"Note grip Z");

		return S_OK;
		break;
	default:
		;
    }

    return E_NOTIMPL;
}

//This function is called to determine the number of elements in a group
//It defaults to the number of elements in the array (3 in this case)
STDMETHODIMP CComRebarPos::GetElementGrouping(
    /* [in] */ DISPID dispID,
	/* [out] */ short *groupingNumber)
{
    return E_NOTIMPL;
}

//This function is called to determine how many groups there are in
//the array.
//For example in case of the polyline this is the number of vertices.
//We are not implementing this because it defaults to nGroupCnt = 0
STDMETHODIMP CComRebarPos::GetGroupCount(
    /* [in] */ DISPID dispID,
	/* [out] */ long *nGroupCnt)
{
    return E_NOTIMPL;
}

//OPM calls this function for each property to obtain a list of strings and cookies if they are available.
//For our textstyle property we would like to display all the textstyles currently available in the database.
//This function is declared on the IPerPropertyBrowsing interface. Our IOPMPropertyExtensionImpl
//class implements this member by reading the values in the OPM property map. (You set this up in your
//head file when you use BEGIN_OPMPROP_MAP, OPMPROP_ENTRY, END_OPMPROP_MAP macros.)
//Since we need a dynamic list of entries in this drop down list and a static map cannot implement this, 
//we need to override this function a provide dynamic list of text styles to OPM.
STDMETHODIMP CComRebarPos::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pCaStringsOut, CADWORD *pCaCookiesOut)
{
    return  IOPMPropertyExtensionImpl<CComRebarPos>::GetPredefinedStrings(dispID, pCaStringsOut, pCaCookiesOut);

	/*
    if (dispID != DISPID_TEXTSTYLENAME)
        return  IOPMPropertyExtensionImpl<CComRebarPos>::GetPredefinedStrings(dispID,pCaStringsOut,pCaCookiesOut);

    USES_CONVERSION;
    AcDbTextStyleTable* pTT;
    
    AcDbDatabase *pDb = m_objRef.objectId().database();
    if (NULL == pDb)
        pDb = acdbHostApplicationServices()->workingDatabase();
    
    if (pDb->getTextStyleTable(pTT,AcDb::kForRead)==Acad::eOk)
    {
        AcDbTextStyleTableIterator* pIter;
        if (pTT->newIterator(pIter)==Acad::eOk)
        {
            long size = 0;

            // Clear the array.
            mObjectIdArray.removeAll();

            for (pIter->start();!pIter->done();pIter->step())
                size++;
            pCaStringsOut->pElems = (LPOLESTR *)::CoTaskMemAlloc(sizeof(LPOLESTR) * size);
            pCaCookiesOut->pElems = (DWORD *)::CoTaskMemAlloc(sizeof(DWORD) * size);
            long i=0;
            for (pIter->start();!pIter->done();pIter->step())
            {
                AcDbTextStyleTableRecord* pTTR;
                if (pIter->getRecord(pTTR,AcDb::kForRead)!=Acad::eOk)
                    continue;
                const TCHAR* pName = NULL;
                if (pTTR->getName(pName)==Acad::eOk){
                    //we want to show the name of the textstyle as 
                    //it appears in the database
                    pCaStringsOut->pElems[i] = ::SysAllocString(CT2W(pName));
                    pCaCookiesOut->pElems[i] = mObjectIdArray.append(pTTR->objectId());
                }
                pTTR->close();
                i++;
            }
            pCaStringsOut->cElems = i;
            pCaCookiesOut->cElems = i;
        }
        if (pIter)
            delete pIter;
        pTT->close();
    }
	*/
    return S_OK;
}

//OPM calls this function when the user selects an element from a drop down list. OPM provides
//the cookie that we associated with the element in the GetPredefinedStrings function. We are
//responsible for mapping this cookie back to a value that the properties corresponding put_ function
//can understand. 
//In this particular case all we need to do is to provide the name of the text style as
//the put_TextStyle method needs that.
STDMETHODIMP CComRebarPos::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
{
    return  IOPMPropertyExtensionImpl<CComRebarPos>::GetPredefinedValue(dispID,dwCookie, pVarOut);

	/*
    if (dispID != DISPID_TEXTSTYLENAME)
        return  IOPMPropertyExtensionImpl<CComRebarPos>::GetPredefinedValue(dispID,dwCookie, pVarOut);

    USES_CONVERSION;

    assert((INT_PTR)dwCookie >= 0);
    assert((INT_PTR)dwCookie < mObjectIdArray.length());
    AcDbObjectId id = mObjectIdArray[dwCookie];
    assert(!id.isNull());

    AcDbTextStyleTableRecord* pTTR;
    HRESULT hr = S_OK;
    if (acdbOpenObject(pTTR,id,AcDb::kForRead)!=Acad::eOk)
        return E_FAIL;
    const TCHAR* pName = NULL;
    if (pTTR->getName(pName)==Acad::eOk)
        ::VariantCopy(pVarOut,&CComVariant(CT2W(pName)));
    else
        hr = E_FAIL;
    pTTR->close();
    return hr;
	*/
	return S_OK;
}

HRESULT CComRebarPos::CreateNewObject(AcDbObjectId& objId, AcDbObjectId& ownerId, TCHAR* keyName)
{
    try 
    {
        HRESULT hr;
        
        if (FAILED(hr = CreateObject(ownerId, keyName)))
            throw hr;
        if (FAILED(hr = AddToDb(objId, ownerId, keyName)))
            throw hr;
    }
    catch(HRESULT hr)
    {
        //we can become more sophisticated 
        return hr;
    }

    return S_OK;
}

// IAcadBaseObject2Impl
STDMETHODIMP CComRebarPos::ForceDbResident(VARIANT_BOOL *forceDbResident)
{
	if (NULL == forceDbResident)
		return E_POINTER;

	*forceDbResident = ACAX_VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CComRebarPos::CreateObject(AcDbObjectId ownerId, TCHAR *keyName) 
{
        try 
        {
            Acad::ErrorStatus es;
            AcDbObjectPointer<CRebarPos> pPoly;
            if((es = pPoly.create()) != Acad::eOk)
                throw es;

            pPoly->setDatabaseDefaults(ownerId.database());
            CRebarPos *pTmp = NULL;
            pPoly.release(pTmp);
            
            SetObject((AcDbObject*&)pTmp);
        }
        catch(const Acad::ErrorStatus)
        {
            return Error(L"Failed to create object", IID_IComRebarPos, E_FAIL);
        }
        
        return S_OK;
}

STDMETHODIMP CComRebarPos::AddToDb(AcDbObjectId& objId, AcDbObjectId ownerId, TCHAR* keyName)
{
    try 
    {
        AXEntityDocLock(ownerId);

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForRead);

        AcDbBlockTableRecordPointer pBlockTableRecord(ownerId, AcDb::kForWrite);
        if((es = pBlockTableRecord.openStatus()) != Acad::eOk)
            throw es;

        if((es = pBlockTableRecord->appendAcDbEntity(objId, pPoly.object())) != Acad::eOk)
            throw es;

    }
    catch(const Acad::ErrorStatus)
    {
        //we can became more sophisticated 
        return Error(L"Failed to add object to database", IID_IComRebarPos, E_FAIL);
    }

    return SetObjectId(objId);
}

/************************************************
  OPM Properties
*************************************************/
STDMETHODIMP CComRebarPos::get_BasePoint(VARIANT * pVal)
{
	CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForRead, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        AcAxPoint3d pt;
		pt = pPoly->BasePoint();
        pt.setVariant(pVal);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_BasePoint(VARIANT newVal)
{
    try
    {
        AcAxPoint3d pt = newVal;
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForWrite, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        if ((es = pPoly->setBasePoint(pt)) != Acad::eOk)
            throw es;
        else
            Fire_Notification(DISPID_BASEPOINT);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_NoteGrip(VARIANT * pVal)
{
	CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForRead, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        AcAxPoint3d pt;
		pt = pPoly->NoteGrip();
        pt.setVariant(pVal);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_NoteGrip(VARIANT newVal)
{
    try
    {
        AcAxPoint3d pt = newVal;
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForWrite, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        if ((es = pPoly->setNoteGrip(pt)) != Acad::eOk)
            throw es;
        else
            Fire_Notification(DISPID_NOTEGRIP);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Pos(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Pos()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Pos(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setPos(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_POS);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Count(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Count()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Count(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setCount(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_COUNT);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Diameter(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Diameter()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Diameter(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setDiameter(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_DIAMETER);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Spacing(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Spacing()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Spacing(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setSpacing(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_SPACING);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Multiplier(long * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

        *pVal = pPoly->Multiplier();
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Multiplier(long newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setMultiplier(newVal)) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_MULTIPLIER);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_ShowLength(VARIANT_BOOL * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

		*pVal = (pPoly->ShowLength() == Adesk::kTrue ? VARIANT_TRUE : VARIANT_FALSE);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_ShowLength(VARIANT_BOOL newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
		if ((es = pPoly->setShowLength(newVal == VARIANT_TRUE ? Adesk::kTrue : Adesk::kFalse)) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_SHOWLENGTH);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_ShowMarkerOnly(VARIANT_BOOL * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

		*pVal = (pPoly->ShowMarkerOnly() == Adesk::kTrue ? VARIANT_TRUE : VARIANT_FALSE);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_ShowMarkerOnly(VARIANT_BOOL newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
		if ((es = pPoly->setShowMarkerOnly(newVal == VARIANT_TRUE ? Adesk::kTrue : Adesk::kFalse)) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_SHOWMARKERONLY);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Note(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Note()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Note(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setNote(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_NOTE);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_A(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->A()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_A(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setA(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_A);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_B(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->B()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_B(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setB(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_B);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_C(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->C()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_C(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setC(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_C);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_D(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->D()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_D(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setD(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_D);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_E(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->E()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_E(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setE(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_E);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_F(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->F()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_F(BSTR newVal)
{
    try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForWrite,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        if ((es = pPoly->setF(W2T(newVal))) != Acad::eOk)
            throw es;
        else 
            Fire_Notification(DISPID_F);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Length(BSTR * pVal)
{
    CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef,AcDb::kForRead,Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;
        
        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pPoly->Length()));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object", IID_IComRebarPos, E_FAIL);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Shape(BSTR * pVal)
{
	CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForRead, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

        AcDbTextStyleTableRecordPointer pTextStyleRecord(pPoly->ShapeId(), AcDb::kForRead);
        if((es = pTextStyleRecord.openStatus()) != Acad::eOk)
            throw es;

        const TCHAR* pName;
        if ((es = pTextStyleRecord->getName(pName)) != Acad::eOk)
            throw es;

        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pName));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Shape(BSTR newVal)
{
	try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForWrite, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

        USES_CONVERSION;
        AcDbDatabase* pDb = m_objRef.objectId().database();
        if (NULL == pDb)
            pDb = acdbHostApplicationServices()->workingDatabase();
        
        AcDbTextStyleTableRecordPointer pTextStyleRecord(W2T(newVal), pDb, AcDb::kForRead);
        if((es = pTextStyleRecord.openStatus()) != Acad::eOk)
            throw es;

        if ((es = pPoly->setShapeId(pTextStyleRecord->objectId())) != Acad::eOk)
            throw es;
        else
            Fire_Notification(DISPID_SHAPE);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::get_Group(BSTR * pVal)
{
	CHECKOUTPARAM(pVal);
    try
    {
        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForRead, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

        AcDbTextStyleTableRecordPointer pTextStyleRecord(pPoly->GroupId(), AcDb::kForRead);
        if((es = pTextStyleRecord.openStatus()) != Acad::eOk)
            throw es;

        const TCHAR* pName;
        if ((es = pTextStyleRecord->getName(pName)) != Acad::eOk)
            throw es;

        USES_CONVERSION;
        *pVal = SysAllocString(CT2W(pName));
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to open object.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

STDMETHODIMP CComRebarPos::put_Group(BSTR newVal)
{
	try
    {
        AXEntityDocLockNoDbOk(m_objRef.objectId());

        Acad::ErrorStatus es;
        AcAxObjectRefPtr<CRebarPos> pPoly(&m_objRef, AcDb::kForWrite, Adesk::kTrue);
	    if((es = pPoly.openStatus()) != Acad::eOk)
            throw es;

        USES_CONVERSION;
        AcDbDatabase* pDb = m_objRef.objectId().database();
        if (NULL == pDb)
            pDb = acdbHostApplicationServices()->workingDatabase();
        
        AcDbTextStyleTableRecordPointer pTextStyleRecord(W2T(newVal), pDb, AcDb::kForRead);
        if((es = pTextStyleRecord.openStatus()) != Acad::eOk)
            throw es;

        if ((es = pPoly->setGroupId(pTextStyleRecord->objectId())) != Acad::eOk)
            throw es;
        else
            Fire_Notification(DISPID_GROUP);
    }
    catch(const Acad::ErrorStatus)
    {
        return Error(L"Failed to set property.", IID_IComRebarPos, E_FAIL);
    }
    catch(const HRESULT hr)
    {
        return Error(L"Invalid argument.", IID_IComRebarPos, hr);
    }
	return S_OK;
}

