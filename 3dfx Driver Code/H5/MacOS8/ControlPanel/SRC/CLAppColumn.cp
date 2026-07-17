//	TextColumn has a single TextTraits that applies to all cells
//	and uses a 'STR#' resource to specify the initial items in the column
//
//	Data is text with no length byte. To add rows at runtime, call
//	SetCellData(), passing a pointer to the first character and byte count.

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include "CLAppColumn.h"
#include <LColumnView.h>
#include <LStream.h>
#include <UDrawingUtils.h>
#include <UTextTraits.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ CLAppColumn(LStream*)
// ---------------------------------------------------------------------------

CLAppColumn::CLAppColumn(
	LStream		*inStream)
		: LColumnView(inStream)
{
	
	
	//Handle	tempViewH = ::GetResource(FOR_CHAR_CODE('view', 131);
	
	InsertRows (1, 0, nil, 0, Refresh_No);
	STableCell	cell (0, 1);
	
	cell.row = 1;
	
	SetCellData(cell, "131", 3);
	
	/*inStream->ReadData(&mTemplateViewID, sizeof(mTemplateViewID));
	
		// Initial text items in column come from a 'STR#' resource
		
	ResIDT	theTemplateViewID;
	inStream->ReadData(&theTemplateViewID, sizeof(theTemplateViewID));
	
	Handle	strxH = ::GetResource(FOUR_CHAR_CODE('STR#'), theTemplateViewID);
	if (strxH != nil) {
		SInt16	strCount = *(SInt16*)(*strxH);
		InsertRows(strCount, 0, nil, 0, Refresh_No);
		
		STableCell	cell(0, 1);
		Str255		str;
		for (SInt16 i = 1; i <= strCount; ++i) {
			::GetIndString(str, theSTRxID, i);
			cell.row = i;
			SetCellData(cell, str+1, str[0]);	// Store text without length byte
		}
		
		// Note: We don't release the STR# resource.
		// Mark it purgeable to allow the System to release it.
	}*/
}	


// ---------------------------------------------------------------------------
//	¥ ~CLAppColumn
// ---------------------------------------------------------------------------

CLAppColumn::~CLAppColumn()
{
}


// ---------------------------------------------------------------------------
//	¥ DrawCell
// ---------------------------------------------------------------------------

void
CLAppColumn::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	Rect	textRect = inLocalRect;
	::MacInsetRect(&textRect, 2, 0);
	
	// Draw the AppInfoCell view here ...
	
			
	
	
	
	char	str[255];				// Data is text with no length byte
	UInt32	len = sizeof(str);
	GetCellData(inCell, str, len);
	
	//SInt16	just = UTextTraits::SetPortTextTraits(mTxtrID);
	UTextDrawing::DrawWithJustification(str, (SInt32) len, textRect, 0);
}

PP_End_Namespace_PowerPlant
