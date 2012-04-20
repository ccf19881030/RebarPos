//-----------------------------------------------------------------------------
//----- PosShape.cpp : Implementation of CPosShape
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include <windows.h>
#include <objbase.h>

#include "rxregsvc.h"

#include "assert.h"
#include "math.h"

#include "gepnt3d.h"
#include "gevec3d.h"
#include "gelnsg3d.h"
#include "gearc3d.h"

#include "dbents.h"
#include "dbsymtb.h"
#include "dbcfilrs.h"
#include "dbspline.h"
#include "dbproxy.h"
#include "dbxutil.h"
#include "acutmem.h"

#include "acdb.h"
#include "dbidmap.h"
#include "adesk.h"

#include "dbapserv.h"
#include "appinfo.h"
#include "tchar.h"

#include "PosShape.h"

//-----------------------------------------------------------------------------
Adesk::UInt32 CPosShape::kCurrentVersionNumber = 1;

ACHAR* CPosShape::Table_Name = _T("OZOZ_REBAR_SHAPES");

//-----------------------------------------------------------------------------
ACRX_DXF_DEFINE_MEMBERS(CPosShape, AcDbObject,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
	AcDbProxyEntity::kAllAllowedBits, POSSHAPE,
	"RebarPos2.0\
	|Product Desc:     PosShape Entity\
	|Company:          OZOZ");

//-----------------------------------------------------------------------------
CPosShape::CPosShape () : m_Fields(1), m_Formula(NULL), m_FormulaBending(NULL)
{ }

CPosShape::~CPosShape () 
{ 
	acutDelString(m_Formula);
	acutDelString(m_FormulaBending);

	ClearShapes();
}

//*************************************************************************
// Properties
//*************************************************************************

const Adesk::Int32 CPosShape::Fields(void) const
{
	assertReadEnabled();
	return m_Fields;
}

Acad::ErrorStatus CPosShape::setFields(const Adesk::Int32 newVal)
{
	assertWriteEnabled();
	m_Fields = newVal;
	return Acad::eOk;
}

const ACHAR* CPosShape::Formula(void) const
{
	assertReadEnabled();
	return m_Formula;
}

Acad::ErrorStatus CPosShape::setFormula(const ACHAR* newVal)
{
	assertWriteEnabled();

    acutDelString(m_Formula);
    m_Formula = NULL;
    if(newVal != NULL)
    {
        acutUpdString(newVal, m_Formula);
    }

	return Acad::eOk;
}

const ACHAR* CPosShape::FormulaBending(void) const
{
	assertReadEnabled();
	return m_FormulaBending;
}

Acad::ErrorStatus CPosShape::setFormulaBending(const ACHAR* newVal)
{
	assertWriteEnabled();

    acutDelString(m_FormulaBending);
    m_FormulaBending = NULL;
    if(newVal != NULL)
    {
        acutUpdString(newVal, m_FormulaBending);
    }

	return Acad::eOk;
}

//*************************************************************************
// Methods
//*************************************************************************
void CPosShape::AddShape(CShape* const shape)
{
	assertWriteEnabled();
	m_List.push_back(shape);
}

const CShape* CPosShape::GetShape(const ShapeSize index) const
{
	assertReadEnabled();
	return m_List.at(index);
}

void CPosShape::SetShape(const ShapeSize index, CShape* const shape)
{
	assertWriteEnabled();
	m_List[index] = shape;
}

void CPosShape::RemoveShape(const ShapeSize index)
{
	assertWriteEnabled();
	ShapeListIt it = m_List.begin();
	m_List.erase(it + index);
}

void CPosShape::ClearShapes()
{
	assertWriteEnabled();
	for(ShapeListIt it = m_List.begin(); it != m_List.end(); it++)
	{
		delete *it;
	}
	m_List.clear();
}

const ShapeSize CPosShape::GetShapeCount() const
{
	assertReadEnabled();
	return m_List.size();
}

//*************************************************************************
// Overrides
//*************************************************************************
//- Dwg Filing protocol
Acad::ErrorStatus CPosShape::dwgOutFields(AcDbDwgFiler *pFiler) const 
{
	assertReadEnabled();

	// Save parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbObject::dwgOutFields(pFiler)) != Acad::eOk)
		return es;

	// Object version number
	pFiler->writeItem(CPosShape::kCurrentVersionNumber);

	// Properties
	pFiler->writeInt32(m_Fields);
	if (m_Formula)
		pFiler->writeString(m_Formula);
	else
		pFiler->writeString(_T(""));
	if (m_FormulaBending)
		pFiler->writeString(m_FormulaBending);
	else
		pFiler->writeString(_T(""));

    // Segments
	pFiler->writeInt32((int)m_List.size());
	for(ShapeListConstIt it = m_List.begin(); it != m_List.end(); it++)
	{
		CShape* shape = *it;
		pFiler->writeInt32(shape->type);
		pFiler->writeUInt16(shape->color);
		switch(shape->type)
		{
		case CShape::Line:
			{
				CShapeLine* line = static_cast<CShapeLine*>(shape);
				pFiler->writeDouble(line->x1);
				pFiler->writeDouble(line->y1);
				pFiler->writeDouble(line->x2);
				pFiler->writeDouble(line->y2);
			}
			break;
		case CShape::Arc:
			{
				CShapeArc* arc = (CShapeArc*)shape;
				pFiler->writeDouble(arc->x);
				pFiler->writeDouble(arc->y);
				pFiler->writeDouble(arc->r);
				pFiler->writeDouble(arc->startAngle);
				pFiler->writeDouble(arc->endAngle);
			}
			break;
		case CShape::Text:
			{
				CShapeText* text = (CShapeText*)shape;
				pFiler->writeDouble(text->x);
				pFiler->writeDouble(text->y);
				pFiler->writeDouble(text->height);
				pFiler->writeString(text->text);
			}
			break;
		}
	}

	return pFiler->filerStatus();
}

Acad::ErrorStatus CPosShape::dwgInFields(AcDbDwgFiler *pFiler) 
{
	assertWriteEnabled();

	// Read parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbObject::dwgInFields(pFiler)) != Acad::eOk)
		return es;

	// Object version number needs to be read first
	Adesk::UInt32 version = 0;
	pFiler->readItem(&version);
	if (version > CPosShape::kCurrentVersionNumber)
		return Acad::eMakeMeProxy;

	// Read params
	if (version >= 1)
	{
		acutDelString(m_Formula);
		acutDelString(m_FormulaBending);

		// Properties
        pFiler->readInt32(&m_Fields);
		pFiler->readString(&m_Formula);
		pFiler->readString(&m_FormulaBending);

		// Segments
		long count;
		pFiler->readInt32(&count);
		for(long i = 0; i < count; i++)
		{
			Adesk::Int32 type;
			pFiler->readInt32(&type);
			Adesk::UInt16 color;
			pFiler->readUInt16(&color);
			switch(type)
			{
			case CShape::Line:
				{
					CShapeLine* line = new CShapeLine();
					line->color = color;
					pFiler->readDouble(&line->x1);
					pFiler->readDouble(&line->y1);
					pFiler->readDouble(&line->x2);
					pFiler->readDouble(&line->y2);
					m_List.push_back(line);
				}
				break;
			case CShape::Arc:
				{
					CShapeArc* arc = new CShapeArc();
					arc->color = color;
					pFiler->readDouble(&arc->x);
					pFiler->readDouble(&arc->y);
					pFiler->readDouble(&arc->r);
					pFiler->readDouble(&arc->startAngle);
					pFiler->readDouble(&arc->endAngle);
					m_List.push_back(arc);
				}
				break;
			case CShape::Text:
				{
					CShapeText* text = new CShapeText();
					text->color = color;
					pFiler->readDouble(&text->x);
					pFiler->readDouble(&text->y);
					pFiler->readDouble(&text->height);
					pFiler->readString(&text->text);
					m_List.push_back(text);
				}
				break;
			}
		}
	}

	return pFiler->filerStatus();
}

//-----------------------------------------------------------------------------
//----- AcDbObject protocols
//- Dxf Filing protocol
Acad::ErrorStatus CPosShape::dxfOutFields(AcDbDxfFiler *pFiler) const 
{
	assertReadEnabled();

	// Save parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbObject::dxfOutFields(pFiler)) != Acad::eOk)
		return es;

	// Subclass
	pFiler->writeItem(AcDb::kDxfSubclass, _T("PosShape"));

	// Object version number
	pFiler->writeItem(AcDb::kDxfInt32, CPosShape::kCurrentVersionNumber);

	// Properties
	pFiler->writeInt32(AcDb::kDxfInt32 + 1, m_Fields);
	if(m_Formula)
		pFiler->writeString(AcDb::kDxfXTextString, m_Formula);
	else
		pFiler->writeString(AcDb::kDxfXTextString, _T(""));
	if(m_FormulaBending)
		pFiler->writeString(AcDb::kDxfXTextString, m_FormulaBending);
	else
		pFiler->writeString(AcDb::kDxfXTextString, _T(""));

    // Segments
	pFiler->writeInt32(AcDb::kDxfInt32, (int)m_List.size());
	for(ShapeListConstIt it = m_List.begin(); it != m_List.end(); it++)
	{
		CShape* shape = *it;
		pFiler->writeInt32(AcDb::kDxfInt32, shape->type);
		pFiler->writeUInt16(AcDb::kDxfColor, shape->color);
		switch(shape->type)
		{
		case CShape::Line:
			{
				CShapeLine* line = static_cast<CShapeLine*>(shape);
				pFiler->writeDouble(AcDb::kDxfXCoord, line->x1);
				pFiler->writeDouble(AcDb::kDxfYCoord, line->y1);
				pFiler->writeDouble(AcDb::kDxfXCoord + 1, line->x2);
				pFiler->writeDouble(AcDb::kDxfYCoord + 1, line->y2);
			}
			break;
		case CShape::Arc:
			{
				CShapeArc* arc = (CShapeArc*)shape;
				pFiler->writeDouble(AcDb::kDxfXCoord, arc->x);
				pFiler->writeDouble(AcDb::kDxfYCoord, arc->y);
				pFiler->writeDouble(AcDb::kDxfReal, arc->r);
				pFiler->writeDouble(AcDb::kDxfAngle, arc->startAngle);
				pFiler->writeDouble(AcDb::kDxfAngle + 1, arc->endAngle);
			}
			break;
		case CShape::Text:
			{
				CShapeText* text = (CShapeText*)shape;
				pFiler->writeDouble(AcDb::kDxfXCoord, text->x);
				pFiler->writeDouble(AcDb::kDxfYCoord, text->y);
				pFiler->writeDouble(AcDb::kDxfTxtSize, text->height);
				pFiler->writeString(AcDb::kDxfText, text->text);
			}
			break;
		}
	}

	return pFiler->filerStatus();
}

Acad::ErrorStatus CPosShape::dxfInFields(AcDbDxfFiler *pFiler) 
{
	assertWriteEnabled();

	// Read parent class information first.
	Acad::ErrorStatus es;
	if(((es = AcDbObject::dxfInFields(pFiler)) != Acad::eOk) || !pFiler->atSubclassData(_T("PosShape")))
		return es;

	resbuf rb;
	// Object version number
    Adesk::UInt32 version;
    pFiler->readItem(&rb);
    if (rb.restype != AcDb::kDxfInt32) 
    {
        pFiler->pushBackItem();
        pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (version)"), AcDb::kDxfInt32);
        return pFiler->filerStatus();
    }
    version = rb.resval.rlong;
	if (version > CPosShape::kCurrentVersionNumber)
		return Acad::eMakeMeProxy;

	// Read params
	// Properties
	Adesk::Int32 t_Fields;
	ACHAR* t_Formula = NULL;
	ACHAR* t_FormulaBending = NULL;
	ShapeList t_List;

	if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfInt32 + 1) 
	{
		t_Fields = rb.resval.rlong;
	}
	else
	{
        pFiler->pushBackItem();
        pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (fields)"), AcDb::kDxfXInt16);
        return pFiler->filerStatus();
	}
	if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXTextString) 
	{
		acutUpdString(rb.resval.rstring, t_Formula);
	}
	else
	{
        pFiler->pushBackItem();
        pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (formula)"), AcDb::kDxfXTextString);
        return pFiler->filerStatus();
	}
	if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXTextString + 1) 
	{
		acutUpdString(rb.resval.rstring, t_FormulaBending);
	}
	else
	{
        pFiler->pushBackItem();
        pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (formula bending)"), AcDb::kDxfXTextString + 1);
        return pFiler->filerStatus();
	}

	// Segments
	long count;
	if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfInt32) 
	{
		count = rb.resval.rlong;
	}
	else
	{
        pFiler->pushBackItem();
        pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (count)"), AcDb::kDxfInt32);
        return pFiler->filerStatus();
	}
	
	for(long i = 0; i < count; i++)
	{
		Adesk::Int32 type;
		Adesk::UInt16 color;
		if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfInt32) 
		{
			type = rb.resval.rlong;
		}
		else
		{
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment type code)"), AcDb::kDxfInt32);
			return pFiler->filerStatus();
		}
		if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfColor) 
		{
			color = rb.resval.rint;
		}
		else
		{
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment color)"), AcDb::kDxfColor);
			return pFiler->filerStatus();
		}

		switch(type)
		{
		case CShape::Line:
			{
				CShapeLine* line = new CShapeLine();
				line->color = color;
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXCoord) 
				{
					line->x1 = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment x1)"), AcDb::kDxfXCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfYCoord) 
				{
					line->y1 = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment x1)"), AcDb::kDxfYCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXCoord + 1) 
				{
					line->x2 = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment x1)"), AcDb::kDxfXCoord + 1);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfYCoord + 1) 
				{
					line->y2 = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (segment x1)"), AcDb::kDxfYCoord + 1);
					return pFiler->filerStatus();
				}
				t_List.push_back(line);
			}
			break;
		case CShape::Arc:
			{
				CShapeArc* arc = new CShapeArc();
				arc->color = color;
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXCoord) 
				{
					arc->x = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (arc x)"), AcDb::kDxfXCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfYCoord) 
				{
					arc->y = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (arc y)"), AcDb::kDxfYCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfReal) 
				{
					arc->r = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (arc r)"), AcDb::kDxfReal);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfAngle) 
				{
					arc->startAngle = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (arc start angle)"), AcDb::kDxfAngle);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfAngle + 1) 
				{
					arc->endAngle = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (arc end angle)"), AcDb::kDxfAngle + 1);
					return pFiler->filerStatus();
				}
				t_List.push_back(arc);
			}
			break;
		case CShape::Text:
			{
				CShapeText* text = new CShapeText();
				text->color = color;
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfXCoord) 
				{
					text->x = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (text x)"), AcDb::kDxfXCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfYCoord) 
				{
					text->y = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (text y)"), AcDb::kDxfYCoord);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfTxtSize) 
				{
					text->height = rb.resval.rreal;
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (text height)"), AcDb::kDxfTxtSize);
					return pFiler->filerStatus();
				}
				if ((es = pFiler->readItem(&rb)) == Acad::eOk && rb.restype == AcDb::kDxfText) 
				{
					acutUpdString(rb.resval.rstring, text->text);
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode, _T("\nError: expected group code %d (text contents)"), AcDb::kDxfText);
					return pFiler->filerStatus();
				}
				
				t_List.push_back(text);
			}
			break;
		}
	}

	m_Fields = t_Fields;
	setFormula(t_Formula);
	setFormulaBending(t_FormulaBending);
	m_List = t_List;

	acutDelString(t_Formula);
	acutDelString(t_FormulaBending);

	return pFiler->filerStatus();
}

//*************************************************************************
// Common static dictionary methods
//*************************************************************************

ACHAR* CPosShape::GetTableName()
{
	return Table_Name;
}
