//-----------------------------------------------------------------------------
//----- GenericTable.cpp : Implementation of CGenericTable
//-----------------------------------------------------------------------------

#include "StdAfx.h"

#include "GenericTable.h"
#include "Utility.h"

#include <algorithm>

Adesk::UInt32 CGenericTable::kCurrentVersionNumber = 1;

//*************************************************************************
// Code for the Class Body. 
//*************************************************************************

ACRX_DXF_DEFINE_MEMBERS(CGenericTable, AcDbEntity,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
	AcDbProxyEntity::kAllAllowedBits, GENERICTABLE,
	"OZOZRebarPos\
	|Product Desc:     Geberic Table Entity\
	|Company:          OZOZ");

//*************************************************************************
// Constructors and destructors 
//*************************************************************************

CGenericTable::CGenericTable(void) : m_BasePoint(0, 0, 0), 
	m_Direction(1, 0, 0), m_Up(0, 1, 0), m_Normal(0, 0, 1),
	m_Rows(0), m_Columns(0), m_Cells(0),
	lastTexts(), lastLines(), columnWidths(), rowHeights(), isModified(true),
	m_CellMargin(0.2)
{
}

CGenericTable::~CGenericTable(void)
{
	Clear();
	ResetDrawParams();
}

//*************************************************************************
// Properties
//*************************************************************************
const AcGeVector3d& CGenericTable::DirectionVector(void) const
{
	assertReadEnabled();
	return m_Direction;
}

const AcGeVector3d& CGenericTable::UpVector(void) const
{
	assertReadEnabled();
	return m_Up;
}

const AcGeVector3d& CGenericTable::NormalVector(void) const
{
	assertReadEnabled();
	return m_Normal;
}

const double CGenericTable::Scale(void) const
{
	assertReadEnabled();
	return m_Direction.length();
}
Acad::ErrorStatus CGenericTable::setScale(const double newVal)
{
	assertWriteEnabled();
	AcGeMatrix3d xform = AcGeMatrix3d::scaling(newVal / m_Direction.length(), m_BasePoint);
	return transformBy(xform);
}

const int CGenericTable::Columns(void) const
{
	assertReadEnabled();
	return m_Columns;
}
const int CGenericTable::Rows(void) const
{
	assertReadEnabled();
	return m_Rows;
}

const double CGenericTable::CellMargin(void) const
{
	assertReadEnabled();
	return m_CellMargin;
}
Acad::ErrorStatus CGenericTable::setCellMargin(const double newVal)
{
	assertWriteEnabled();
	m_CellMargin = newVal;
	isModified = true;
	return Acad::eOk;
}

const AcGePoint3d& CGenericTable::BasePoint(void) const
{
	assertReadEnabled();
	return m_BasePoint;
}
Acad::ErrorStatus CGenericTable::setBasePoint(const AcGePoint3d& newVal)
{
	assertWriteEnabled();
	m_BasePoint = newVal;
	return Acad::eOk;
}

const double CGenericTable::Width(void) const
{
	if(isModified)
	{
		Calculate();
	}
	double w = 0;
	for(std::vector<double>::iterator it = columnWidths.begin(); it != columnWidths.end(); it++)
	{
		w += (*it);
	}
	return w * m_Direction.length();
}

const double CGenericTable::Height(void) const
{
	if(isModified)
	{
		Calculate();
	}
	double h = 0;
	for(std::vector<double>::iterator it = rowHeights.begin(); it != rowHeights.end(); it++)
	{
		h += (*it);
	}
	return h * m_Direction.length();
}

//*************************************************************************
// Methods
//*************************************************************************

void CGenericTable::SetSize(int rows, int columns)
{
	assertWriteEnabled();

	Clear();

	m_Cells.resize(rows * columns);
	minColumnWidths.resize(columns);
	minRowHeights.resize(rows);

	// Create new cells
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < columns; j++)
		{
			m_Cells[i * columns + j] = new CTableCell();
		}
	}

	m_Rows = rows;
	m_Columns = columns;

	isModified = true;
}

void CGenericTable::Clear()
{
	assertWriteEnabled();

	for(std::vector<CTableCell*>::iterator it = m_Cells.begin(); it != m_Cells.end(); it++)
		delete (*it);

	m_Cells.clear();

	isModified = true;
}

void CGenericTable::setMinimumColumnWidth(const int j, const double newVal)
{
	assertWriteEnabled();
	minColumnWidths[j] = newVal;
}

void CGenericTable::setMinimumRowHeight(const int i, const double newVal)
{
	assertWriteEnabled();
	minRowHeights[i] = newVal;
}

//*************************************************************************
// Cell Access Methods
//*************************************************************************
void CGenericTable::setCellText(const int i, const int j, const std::wstring& newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	std::wstring scopy(newVal);
	Utility::ReplaceString(scopy, L"\r\n", L"\\P");
	Utility::ReplaceString(scopy, L"\r", L"\\P");
	Utility::ReplaceString(scopy, L"\n", L"\\P");
	cell->setText(scopy);
	isModified = true;
}
void CGenericTable::setCellTextColor(const int i, const int j, const unsigned short newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setTextColor(newVal);
	isModified = true;
}
void CGenericTable::setCellTextStyleId(const int i, const int j, const AcDbObjectId& newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setTextStyleId(newVal);
	isModified = true;
}
void CGenericTable::setCellTextHeight(const int i, const int j, const double newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setTextHeight(newVal);
	isModified = true;
}

void CGenericTable::setCellLeftBorder(const int i, const int j, const bool hasBorder, const unsigned short borderColor)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setLeftBorder(hasBorder);
	cell->setLeftBorderColor(borderColor);
	isModified = true;
}
void CGenericTable::setCellRightBorder(const int i, const int j, const bool hasBorder, const unsigned short borderColor)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setRightBorder(hasBorder);
	cell->setRightBorderColor(borderColor);
	isModified = true;
}
void CGenericTable::setCellTopBorder(const int i, const int j, const bool hasBorder, const unsigned short borderColor)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setTopBorder(hasBorder);
	cell->setTopBorderColor(borderColor);
	isModified = true;
}
void CGenericTable::setCellBottomBorder(const int i, const int j, const bool hasBorder, const unsigned short borderColor)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setBottomBorder(hasBorder);
	cell->setBottomBorderColor(borderColor);
	isModified = true;
}

void CGenericTable::setCellHorizontalAlignment(const int i, const int j, const CTableCell::Alignment newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setHorizontalAlignment(newVal);
	isModified = true;
}
void CGenericTable::setCellVerticalAlignment(const int i, const int j, const CTableCell::Alignment newVal)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setVerticalAlignment(newVal);
	isModified = true;
}

void CGenericTable::MergeAcross(const int i, const int j, const int span)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setMergeRight(span);
	
	if(span > 1)
	{
		// Clear texts
		for(int k = j + 1; k < j + span; k++)
		{
			m_Cells[i * m_Columns + k]->setText(L"");
		}
		// Clear interior borders
		for(int k = j + 1; k < j + span; k++)
		{
			m_Cells[i * m_Columns + k]->setLeftBorder(false);
		}
		for(int k = j; k < j + span - 1; k++)
		{
			m_Cells[i * m_Columns + k]->setRightBorder(false);
		}
	}
	isModified = true;
}
void CGenericTable::MergeDown(const int i, const int j, const int span)
{
	CTableCell* cell = m_Cells[i * m_Columns + j];
	cell->setMergeDown(span);
	
	if(span > 1)
	{
		// Clear texts
		for(int k = i + 1; k < i + span; k++)
		{
			m_Cells[k * m_Columns + j]->setText(L"");
		}
		// Clear interior borders
		for(int k = i + 1; k < i + span; k++)
		{
			m_Cells[k * m_Columns + j]->setTopBorder(false);
		}
		for(int k = i; k < i + span - 1; k++)
		{
			m_Cells[k * m_Columns + j]->setBottomBorder(false);
		}
	}
	isModified = true;
}

//*************************************************************************
// Helper Methods
//*************************************************************************
const void CGenericTable::ResetDrawParams(void) const
{
	for(std::vector<AcDbMText*>::iterator it = lastTexts.begin(); it != lastTexts.end(); it++)
		delete *it;
	for(std::vector<AcDbLine*>::iterator it = lastLines.begin(); it != lastLines.end(); it++)
		delete *it;
	lastTexts.clear();
	lastLines.clear();

	columnWidths.clear();
	rowHeights.clear();
}

const void CGenericTable::Calculate(void) const
{
	if(!isModified)
	{
		return;
	}

	assertReadEnabled();

	// Reset old params
	ResetDrawParams();

	lastTexts.resize(m_Columns * m_Rows);

	// Create text styles
	for(int i = 0; i < m_Rows; i++)
	{
		for(int j = 0; j < m_Columns; j++)
		{
			CTableCell* cell = m_Cells[i * m_Columns + j];

			AcDbMText* mtext = new AcDbMText();
			mtext->setTextStyle(cell->TextStyleId());
			mtext->setTextHeight(cell->TextHeight());
			mtext->setColorIndex(cell->TextColor());
			mtext->setTextStyle(cell->TextStyleId());

			if(cell->HorizontalAlignment() == CTableCell::eNEAR)
			{
				if(cell->VerticalAlignment() == CTableCell::eNEAR)
					mtext->setAttachment(AcDbMText::kTopLeft);
				else if(cell->VerticalAlignment() == CTableCell::eCENTER)
					mtext->setAttachment(AcDbMText::kMiddleLeft);
				else if(cell->VerticalAlignment() == CTableCell::eFAR)
					mtext->setAttachment(AcDbMText::kBottomLeft);
			}
			else if(cell->HorizontalAlignment() == CTableCell::eCENTER)
			{
				if(cell->VerticalAlignment() == CTableCell::eNEAR)
					mtext->setAttachment(AcDbMText::kTopCenter);
				else if(cell->VerticalAlignment() == CTableCell::eCENTER)
					mtext->setAttachment(AcDbMText::kMiddleCenter);
				else if(cell->VerticalAlignment() == CTableCell::eFAR)
					mtext->setAttachment(AcDbMText::kBottomCenter);
			}
			else if(cell->HorizontalAlignment() == CTableCell::eFAR)
			{
				if(cell->VerticalAlignment() == CTableCell::eNEAR)
					mtext->setAttachment(AcDbMText::kTopRight);
				else if(cell->VerticalAlignment() == CTableCell::eCENTER)
					mtext->setAttachment(AcDbMText::kMiddleRight);
				else if(cell->VerticalAlignment() == CTableCell::eFAR)
					mtext->setAttachment(AcDbMText::kBottomRight);
			}

			mtext->setContents(cell->Text().c_str());

			lastTexts[i * m_Columns + j] = mtext;
		}
	}

	// Set minimum columns sizes
	columnWidths.resize(m_Columns);
	for(int j = 0; j < m_Columns; j++)
	{
		columnWidths[j] = 2.0 * m_CellMargin;
	}
	for(int j = 0; j < m_Columns; j++)
	{
		for(int i = 0; i < m_Rows; i++)
		{
			if(m_Cells[i * m_Columns + j]->MergeRight() == 0)
			{
				columnWidths[j] = max(columnWidths[j], lastTexts[i * m_Columns + j]->actualWidth() + 2.0 * m_CellMargin);
			}
			else
			{
				int span = m_Cells[i * m_Columns + j]->MergeRight();
				if(j + span - 1 > m_Columns - 1) span = m_Columns - j;
				double w = (lastTexts[i * m_Columns + j]->actualWidth() + 2.0 * m_CellMargin) / (double)span;
				for(int k = j; k < j + span; k++)
				{
					columnWidths[k] = max(columnWidths[k], w);
				}
			}
		}
		columnWidths[j] = max(columnWidths[j], minColumnWidths[j]);
	}

	// Set minimum row sizes
	rowHeights.resize(m_Rows);
	for(int i = 0; i < m_Rows; i++)
	{
		rowHeights[i] = 2.0 * m_CellMargin;
	}
	for(int i = 0; i < m_Rows; i++)
	{
		for(int j = 0; j < m_Columns; j++)
		{
			if(m_Cells[i * m_Columns + j]->MergeDown() == 0)
			{
				rowHeights[i] = max(rowHeights[i], lastTexts[i * m_Columns + j]->actualHeight() + 2.0 * m_CellMargin);
			}
			else
			{
				int span = m_Cells[i * m_Columns + j]->MergeDown();
				if(i + span - 1 > m_Rows - 1) span = m_Rows - i;
				double h = (lastTexts[i * m_Columns + j]->actualHeight() + 2.0 * m_CellMargin) / (double)span;
				for(int k = i; k < i + span; k++)
				{
					rowHeights[k] = max(rowHeights[k], h);
				}
			}
		}
		rowHeights[i] = max(rowHeights[i], minRowHeights[i]);
	}

	// Set text location
	double x = 0;
	double y = 0;
	for(int i = 0; i < m_Rows; i++)
	{
		double h = rowHeights[i];
		for(int j = 0; j < m_Columns; j++)
		{
			double w = columnWidths[j];
			if(m_Cells[i * m_Columns + j]->MergeRight() != 0)
			{
				int span = m_Cells[i * m_Columns + j]->MergeRight();
				if(j + span - 1 > m_Columns - 1) span = m_Columns - j;
				w = 0;
				for(int k = j ; k < j + span; k++)
				{
					w += columnWidths[k];
				}
			}
			if(m_Cells[i * m_Columns + j]->MergeDown() != 0)
			{
				int span = m_Cells[i * m_Columns + j]->MergeDown();
				if(i + span - 1 > m_Rows - 1) span = m_Rows - i;
				h = 0;
				for(int k = i ; k < i + span; k++)
				{
					h += rowHeights[k];
				}
			}
			double cx = m_CellMargin;
			double cy = m_CellMargin;
			if(m_Cells[i * m_Columns + j]->HorizontalAlignment() == CTableCell::eNEAR)
			{
				cx = m_CellMargin;
			}
			else if(m_Cells[i * m_Columns + j]->HorizontalAlignment() == CTableCell::eFAR)
			{
				cx = w - m_CellMargin;
			}
			else if(m_Cells[i * m_Columns + j]->HorizontalAlignment() == CTableCell::eCENTER)
			{
				cx = w / 2.0;
			}
			if(m_Cells[i * m_Columns + j]->VerticalAlignment() == CTableCell::eNEAR)
			{
				cy = m_CellMargin;
			}
			else if(m_Cells[i * m_Columns + j]->VerticalAlignment() == CTableCell::eFAR)
			{
				cy = h - m_CellMargin;
			}
			else if(m_Cells[i * m_Columns + j]->VerticalAlignment() == CTableCell::eCENTER)
			{
				cy = h / 2.0;
			}

			lastTexts[i * m_Columns + j]->setLocation(AcGePoint3d(x + cx, y - cy, 0));

			x += columnWidths[j];
		}
		x = 0;
		y -= rowHeights[i];
	}

	// Set lines
	x = 0;
	y = 0;
	for(int i = 0; i < m_Rows; i++)
	{
		double h = rowHeights[i];
		for(int j = 0; j < m_Columns; j++)
		{
			double w = columnWidths[j];

			CTableCell* cell = m_Cells[i * m_Columns + j];
			if(cell->TopBorder())
			{
				AcDbLine* line = new AcDbLine(AcGePoint3d(x, y, 0), AcGePoint3d(x + columnWidths[j], y, 0));
				line->setColorIndex(cell->TopBorderColor());
				lastLines.push_back(line);
			}
			if(cell->BottomBorder())
			{
				AcDbLine* line = new AcDbLine(AcGePoint3d(x, y - rowHeights[i], 0), AcGePoint3d(x + columnWidths[j], y - rowHeights[i], 0));
				line->setColorIndex(cell->BottomBorderColor());
				lastLines.push_back(line);
			}
			if(cell->LeftBorder())
			{
				AcDbLine* line = new AcDbLine(AcGePoint3d(x, y, 0), AcGePoint3d(x, y - rowHeights[i], 0));
				line->setColorIndex(cell->LeftBorderColor());
				lastLines.push_back(line);
			}
			if(cell->RightBorder())
			{
				AcDbLine* line = new AcDbLine(AcGePoint3d(x + columnWidths[j], y, 0), AcGePoint3d(x + columnWidths[j], y - rowHeights[i], 0));
				line->setColorIndex(cell->RightBorderColor());
				lastLines.push_back(line);
			}

			x += columnWidths[j];
		}
		x = 0;
		y -= rowHeights[i];
	}

	// Done update
	isModified = false;
}

//*************************************************************************
// Overridden methods from AcDbEntity
//*************************************************************************
Acad::ErrorStatus CGenericTable::subGetOsnapPoints(
    AcDb::OsnapMode       osnapMode,
    Adesk::GsMarker       gsSelectionMark,
    const AcGePoint3d&    pickPoint,
    const AcGePoint3d&    lastPoint,
    const AcGeMatrix3d&   viewXform,
    AcGePoint3dArray&     snapPoints,
    AcDbIntArray&         /*geomIds*/) const
{
	assertReadEnabled();

	if(osnapMode == AcDb::kOsModeIns)
	{
		snapPoints.append(m_BasePoint);
	}
	else if(osnapMode == AcDb::kOsModeEnd)
	{
		AcGeMatrix3d trans = AcGeMatrix3d::kIdentity;
		trans.setCoordSystem(m_BasePoint, m_Direction, m_Up, m_Normal);

		for(std::vector<AcDbLine*>::iterator it = lastLines.begin(); it != lastLines.end(); it++)
		{
			AcDbLine* line = (*it);
			AcGePoint3d pt1 = line->startPoint();
			AcGePoint3d pt2 = line->endPoint();
			pt1.transformBy(trans);
			pt2.transformBy(trans);
			snapPoints.append(pt1);
			snapPoints.append(pt2);
		}
	}

	return Acad::eOk;
}

Acad::ErrorStatus CGenericTable::subGetGripPoints(
    AcGePoint3dArray& gripPoints,
    AcDbIntArray& osnapModes,
    AcDbIntArray& geomIds) const
{
	assertReadEnabled();
	gripPoints.append(m_BasePoint);
	return Acad::eOk;
}

Acad::ErrorStatus CGenericTable::subMoveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
    if(indices.length() == 0 || offset.isZeroLength())
        return Acad::eOk;

	assertWriteEnabled();

	transformBy(AcGeMatrix3d::translation(offset));

	return Acad::eOk;
}

Acad::ErrorStatus CGenericTable::subTransformBy(const AcGeMatrix3d& xform)
{
	assertWriteEnabled();
	
	m_BasePoint.transformBy(xform);
	m_Direction.transformBy(xform);
	m_Up.transformBy(xform);
	m_Normal = m_Direction.crossProduct(m_Up);

	// Correct direction vectors
	double scale = m_Direction.length();
	m_Direction = m_Direction.normal() * scale;
	m_Up = m_Up.normal() * scale;
	m_Normal = m_Normal.normal() * scale;

	return Acad::eOk;
}

Acad::ErrorStatus CGenericTable::subExplode(AcDbVoidPtrArray& entitySet) const
{
    assertReadEnabled();

	if(isModified)
	{
		Calculate();
	}

	if(lastTexts.size() == 0 && lastLines.size() == 0)
	{
		return Acad::eInvalidInput;
	}

	Acad::ErrorStatus es;

	// Transformations
	AcGeMatrix3d trans = AcGeMatrix3d::kIdentity;
	trans.setCoordSystem(m_BasePoint, m_Direction, m_Up, m_Normal);

	// Texts
	for(std::vector<AcDbMText*>::iterator it = lastTexts.begin(); it != lastTexts.end(); it++)
	{
		AcRxObject* obj = (*it)->clone();
		AcDbMText* text = static_cast<AcDbMText*>(obj);
		if((es = text->transformBy(trans)) != Acad::eOk)
		{
			return es;
		}
		entitySet.append(text);
	}

	// Lines
	for(std::vector<AcDbLine*>::iterator it = lastLines.begin(); it != lastLines.end(); it++)
	{
		AcRxObject* obj = (*it)->clone();
		AcDbLine* line = static_cast<AcDbLine*>(obj);
		if((es = line->transformBy(trans)) != Acad::eOk)
		{
			return es;
		}
		entitySet.append(line);
	}

    return Acad::eOk;
}

Adesk::Boolean CGenericTable::subWorldDraw(AcGiWorldDraw* worldDraw)
{
    assertReadEnabled();

    if(worldDraw->regenAbort())
	{
        return Adesk::kTrue;
    }

	// Update if modified
	if(isModified)
	{
		Calculate();
	}

	// Transform to match orientation
	AcGeMatrix3d trans = AcGeMatrix3d::kIdentity;
	trans.setCoordSystem(m_BasePoint, m_Direction, m_Up, m_Normal);
	worldDraw->geometry().pushModelTransform(trans);

	// Draw texts
	for(std::vector<AcDbMText*>::iterator it = lastTexts.begin(); it != lastTexts.end(); it++)
	{
		AcDbMText* text = (*it);
		worldDraw->geometry().draw(text);
	}

	// Draw lines
	for(std::vector<AcDbLine*>::iterator it = lastLines.begin(); it != lastLines.end(); it++)
	{
		AcDbLine* line = (*it);
		worldDraw->geometry().draw(line);
	}

	// Reset transform
	worldDraw->geometry().popModelTransform();

    return Adesk::kTrue; // Don't call viewportDraw().
}

Acad::ErrorStatus CGenericTable::subGetGeomExtents(AcDbExtents& extents) const
{
    assertReadEnabled();

	if(isModified)
	{
		Calculate();
	}

	// Get ECS extents
	double w = Width();
	double h = Height();
	AcGePoint3d pt1(0, 0, 0);
	AcGePoint3d pt2(w, 0, 0);
	AcGePoint3d pt3(w, h, 0);
	AcGePoint3d pt4(0, h, 0);

	// Transform to WCS
	AcGeMatrix3d trans = AcGeMatrix3d::kIdentity;
	trans.setCoordSystem(m_BasePoint, m_Direction, m_Up, m_Normal);
	pt1.transformBy(trans);
	pt2.transformBy(trans);
	pt3.transformBy(trans);
	pt4.transformBy(trans);

	extents.addPoint(pt1);
	extents.addPoint(pt2);
	extents.addPoint(pt3);
	extents.addPoint(pt4);
	
	return Acad::eOk;
}

//*************************************************************************
// Overridden methods from AcDbObject
//*************************************************************************

Acad::ErrorStatus CGenericTable::dwgOutFields(AcDbDwgFiler* pFiler) const
{
	assertReadEnabled();

	// Save parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbEntity::dwgOutFields(pFiler)) != Acad::eOk)
		return es;

	// Object version number
	pFiler->writeItem(CGenericTable::kCurrentVersionNumber);

	pFiler->writePoint3d(m_BasePoint);
	pFiler->writeVector3d(m_Direction);
	pFiler->writeVector3d(m_Up);

	pFiler->writeDouble(m_CellMargin);

	pFiler->writeInt32(m_Rows);
	pFiler->writeInt32(m_Columns);

	for(std::vector<CTableCell*>::const_iterator it = m_Cells.begin(); it != m_Cells.end(); it++)
	{
		CTableCell* cell = (*it);

		pFiler->writeString(cell->Text().c_str());

		pFiler->writeUInt16(cell->TextColor());
		pFiler->writeUInt16(cell->TopBorderColor());
		pFiler->writeUInt16(cell->LeftBorderColor());
		pFiler->writeUInt16(cell->BottomBorderColor());
		pFiler->writeUInt16(cell->RightBorderColor());

		pFiler->writeBool(cell->TopBorder());
		pFiler->writeBool(cell->LeftBorder());
		pFiler->writeBool(cell->BottomBorder());
		pFiler->writeBool(cell->RightBorder());

		pFiler->writeInt32(cell->MergeRight());
		pFiler->writeInt32(cell->MergeDown());

		pFiler->writeHardPointerId(cell->TextStyleId());

		pFiler->writeDouble(cell->TextHeight());

		pFiler->writeInt32(cell->HorizontalAlignment());
		pFiler->writeInt32(cell->VerticalAlignment());
	}

	return pFiler->filerStatus();
}

Acad::ErrorStatus CGenericTable::dwgInFields(AcDbDwgFiler* pFiler)
{
	assertWriteEnabled();

	// Read parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbEntity::dwgInFields(pFiler)) != Acad::eOk)
		return es;

	// Object version number needs to be read first
	Adesk::UInt32 version = 0;
	pFiler->readItem(&version);
	if (version > CGenericTable::kCurrentVersionNumber)
		return Acad::eMakeMeProxy;


	pFiler->readPoint3d(&m_BasePoint);
	pFiler->readVector3d(&m_Direction);
	pFiler->readVector3d(&m_Up);
	m_Normal = m_Direction.crossProduct(m_Up);

	pFiler->readDouble(&m_CellMargin);

	Adesk::Int32 rows;
	Adesk::Int32 columns;
	pFiler->readInt32(&rows);
	pFiler->readInt32(&columns);
	SetSize(rows, columns);

	for(int i = 0; i < m_Rows * m_Columns; i++)
	{
		CTableCell* cell = m_Cells[i];

		ACHAR* t_Text = NULL;
		
		Adesk::UInt16 t_TextColor;
		Adesk::UInt16 t_TopBorderColor;
		Adesk::UInt16 t_LeftBorderColor;
		Adesk::UInt16 t_BottomBorderColor;
		Adesk::UInt16 t_RightBorderColor;

		bool t_TopBorder;
		bool t_LeftBorder;
		bool t_BottomBorder;
		bool t_RightBorder;

		Adesk::Int32 t_MergeRight;
		Adesk::Int32 t_MergeDown;

		AcDbHardPointerId t_TextStyleId;

		double t_TextHeight;

		Adesk::Int32 t_HorizontalAlignment;
		Adesk::Int32 t_VerticalAlignment;

		pFiler->readString(&t_Text);

		pFiler->readUInt16(&t_TextColor);
		pFiler->readUInt16(&t_TopBorderColor);
		pFiler->readUInt16(&t_LeftBorderColor);
		pFiler->readUInt16(&t_BottomBorderColor);
		pFiler->readUInt16(&t_RightBorderColor);

		pFiler->readBool(&t_TopBorder);
		pFiler->readBool(&t_LeftBorder);
		pFiler->readBool(&t_BottomBorder);
		pFiler->readBool(&t_RightBorder);

		pFiler->readInt32(&t_MergeRight);
		pFiler->readInt32(&t_MergeDown);

		pFiler->readHardPointerId(&t_TextStyleId);

		pFiler->readDouble(&t_TextHeight);

		pFiler->readInt32(&t_HorizontalAlignment);
		pFiler->readInt32(&t_VerticalAlignment);

		cell->setText(t_Text);
		cell->setTextColor(t_TextColor);
		cell->setTopBorderColor(t_TopBorderColor);
		cell->setLeftBorderColor(t_LeftBorderColor);
		cell->setBottomBorderColor(t_BottomBorderColor);
		cell->setRightBorderColor(t_RightBorderColor);

		cell->setTopBorder(t_TopBorder);
		cell->setLeftBorder(t_LeftBorder);
		cell->setBottomBorder(t_BottomBorder);
		cell->setRightBorder(t_RightBorder);

		cell->setMergeRight(t_MergeRight);
		cell->setMergeDown(t_MergeDown);

		cell->setTextStyleId(t_TextStyleId);

		cell->setTextHeight(t_TextHeight);

		cell->setHorizontalAlignment((CTableCell::Alignment)t_HorizontalAlignment);
		cell->setVerticalAlignment((CTableCell::Alignment)t_VerticalAlignment);

		acutDelString(t_Text);
	}


	isModified = true;

	return pFiler->filerStatus();
}

Acad::ErrorStatus CGenericTable::dxfOutFields(AcDbDxfFiler* pFiler) const
{
	assertReadEnabled();

	// Save parent class information first.
	Acad::ErrorStatus es;
	if((es = AcDbEntity::dxfOutFields(pFiler)) != Acad::eOk)
		return es;

	// Subclass
	pFiler->writeItem(AcDb::kDxfSubclass, _T("GenericTable"));

	pFiler->writePoint3d(AcDb::kDxfXCoord, m_BasePoint);
	// Use max precision when writing out direction vectors
	pFiler->writeVector3d(AcDb::kDxfXCoord + 1, m_Direction, 16);
	pFiler->writeVector3d(AcDb::kDxfXCoord + 2, m_Up, 16);;

	pFiler->writeDouble(AcDb::kDxfReal, m_CellMargin);

	pFiler->writeInt32(AcDb::kDxfInt32, m_Rows);
	pFiler->writeInt32(AcDb::kDxfInt32, m_Columns);

	for(std::vector<CTableCell*>::const_iterator it = m_Cells.begin(); it != m_Cells.end(); it++)
	{
		CTableCell* cell = (*it);

		pFiler->writeString(AcDb::kDxfText, cell->Text().c_str());

		pFiler->writeUInt16(AcDb::kDxfXInt16, cell->TextColor());
		pFiler->writeUInt16(AcDb::kDxfXInt16, cell->TopBorderColor());
		pFiler->writeUInt16(AcDb::kDxfXInt16, cell->LeftBorderColor());
		pFiler->writeUInt16(AcDb::kDxfXInt16, cell->BottomBorderColor());
		pFiler->writeUInt16(AcDb::kDxfXInt16, cell->RightBorderColor());

		pFiler->writeBool(AcDb::kDxfBool, cell->TopBorder());
		pFiler->writeBool(AcDb::kDxfBool, cell->LeftBorder());
		pFiler->writeBool(AcDb::kDxfBool, cell->BottomBorder());
		pFiler->writeBool(AcDb::kDxfBool, cell->RightBorder());

		pFiler->writeInt32(AcDb::kDxfInt32, cell->MergeRight());
		pFiler->writeInt32(AcDb::kDxfInt32, cell->MergeDown());

		pFiler->writeObjectId(AcDb::kDxfHardPointerId, cell->TextStyleId());

		pFiler->writeDouble(AcDb::kDxfReal, cell->TextHeight());

		pFiler->writeInt32(AcDb::kDxfInt32, cell->HorizontalAlignment());
		pFiler->writeInt32(AcDb::kDxfInt32, cell->VerticalAlignment());
	}

	return pFiler->filerStatus();
}

Acad::ErrorStatus CGenericTable::dxfInFields(AcDbDxfFiler* pFiler)
{
	assertWriteEnabled();

	// Read parent class information first.
	Acad::ErrorStatus es;
	if(((es = AcDbEntity::dxfInFields(pFiler)) != Acad::eOk) || !pFiler->atSubclassData(_T("GenericTable")))
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
	if (version > CGenericTable::kCurrentVersionNumber)
		return Acad::eMakeMeProxy;

	// Read params
	AcGeVector3d t_Direction, t_Up;
	AcGePoint3d t_BasePoint;
	double t_CellMargin;
	int t_Rows;
	int t_Columns;
	std::vector<CTableCell*> t_Cells;

	if((es = Utility::ReadDXFPoint(pFiler, AcDb::kDxfXCoord, _T("base point"), t_BasePoint)) != Acad::eOk) return es;
	if((es = Utility::ReadDXFVector(pFiler, AcDb::kDxfXCoord + 1, _T("direction vector"), t_Direction)) != Acad::eOk) return es;
	if((es = Utility::ReadDXFVector(pFiler, AcDb::kDxfXCoord + 2, _T("up vector"), t_Up)) != Acad::eOk) return es;

	if((es = Utility::ReadDXFReal(pFiler, AcDb::kDxfReal, _T("cell margin"), t_CellMargin)) != Acad::eOk) return es;

	if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("row count"), t_Rows)) != Acad::eOk) return es;
	if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("column count"), t_Columns)) != Acad::eOk) return es;

	t_Cells.resize(t_Rows * t_Columns);

	for(int i = 0; i < t_Rows * t_Columns; i++)
    {
		CTableCell* cell = new CTableCell();

		ACHAR* t_Text = NULL;
		
		unsigned short t_TextColor;
		unsigned short t_TopBorderColor;
		unsigned short t_LeftBorderColor;
		unsigned short t_BottomBorderColor;
		unsigned short t_RightBorderColor;

		bool t_TopBorder;
		bool t_LeftBorder;
		bool t_BottomBorder;
		bool t_RightBorder;

		int t_MergeRight;
		int t_MergeDown;

		AcDbHardPointerId t_TextStyleId;

		double t_TextHeight;

		int t_HorizontalAlignment;
		int t_VerticalAlignment;

		if((es = Utility::ReadDXFString(pFiler, AcDb::kDxfText, _T("cell text pos"), t_Text)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFUInt(pFiler, AcDb::kDxfXInt16, _T("cell text color"), t_TextColor)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFUInt(pFiler, AcDb::kDxfXInt16, _T("cell top border color"), t_TopBorderColor)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFUInt(pFiler, AcDb::kDxfXInt16, _T("cell left border color"), t_LeftBorderColor)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFUInt(pFiler, AcDb::kDxfXInt16, _T("cell bottom border color"), t_BottomBorderColor)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFUInt(pFiler, AcDb::kDxfXInt16, _T("cell right border color"), t_RightBorderColor)) != Acad::eOk) return es;

		if((es = Utility::ReadDXFBool(pFiler, AcDb::kDxfBool, _T("cell top border"), t_TopBorder)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFBool(pFiler, AcDb::kDxfBool, _T("cell left border"), t_LeftBorder)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFBool(pFiler, AcDb::kDxfBool, _T("cell bottom border"), t_BottomBorder)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFBool(pFiler, AcDb::kDxfBool, _T("cell right border"), t_RightBorder)) != Acad::eOk) return es;

		if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("cell merge right"), t_MergeRight)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("cell merge down"), t_MergeDown)) != Acad::eOk) return es;

		if((es = Utility::ReadDXFObjectId(pFiler, AcDb::kDxfHardPointerId, _T("cell text style"), t_TextStyleId)) != Acad::eOk) return es;

		if((es = Utility::ReadDXFReal(pFiler, AcDb::kDxfReal, _T("cell text height"), t_TextHeight)) != Acad::eOk) return es;

		if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("cell horizontal alignment"), t_HorizontalAlignment)) != Acad::eOk) return es;
		if((es = Utility::ReadDXFLong(pFiler, AcDb::kDxfInt32, _T("cell vertical alignment"), t_VerticalAlignment)) != Acad::eOk) return es;

		cell->setText(t_Text);
		cell->setTextColor(t_TextColor);
		cell->setTopBorderColor(t_TopBorderColor);
		cell->setLeftBorderColor(t_LeftBorderColor);
		cell->setBottomBorderColor(t_BottomBorderColor);
		cell->setRightBorderColor(t_RightBorderColor);

		cell->setTopBorder(t_TopBorder);
		cell->setLeftBorder(t_LeftBorder);
		cell->setBottomBorder(t_BottomBorder);
		cell->setRightBorder(t_RightBorder);

		cell->setMergeRight(t_MergeRight);
		cell->setMergeDown(t_MergeDown);

		cell->setTextStyleId(t_TextStyleId);

		cell->setTextHeight(t_TextHeight);

		cell->setHorizontalAlignment((CTableCell::Alignment)t_HorizontalAlignment);
		cell->setVerticalAlignment((CTableCell::Alignment)t_VerticalAlignment);

		acutDelString(t_Text);

		t_Cells.push_back(cell);
    }

	// Successfully read DXF codes; set object properties.
	m_BasePoint = t_BasePoint;
	m_Direction = t_Direction;
	m_Up = t_Up;
	m_Normal = m_Direction.crossProduct(m_Up);
	m_CellMargin = t_CellMargin;
	SetSize(t_Rows, t_Columns);
	m_Cells = t_Cells;

	isModified = true;

    return es;
}
