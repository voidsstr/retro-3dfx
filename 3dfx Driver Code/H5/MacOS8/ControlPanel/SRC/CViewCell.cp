
#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include "CViewCell.h"
#include "CViewColumn.h"
#include "CControlPanelApp.h"
#include "ControlPanelPPob.h"

#include <TArrayIterator.h>
#include <URegions.h>
#include <Sound.h>
#include <LCaption.h>
#include <UAttachments.h>
#include <UEnvironment.h>


PP_Begin_Namespace_PowerPlant


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewCell						Default Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewCell::CViewCell()
{
	mSelected = false;
	mBorder = nil;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewCell						Copy Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewCell::CViewCell(
	const CViewCell&	inOriginal)

	: LView(inOriginal)
{
}


// ---------------------------------------------------------------------------
//	¥ CViewCell						Parameterized Constructor [public]
// ---------------------------------------------------------------------------

CViewCell::CViewCell(
	const SPaneInfo&	inPaneInfo,
	const SViewInfo&	inViewInfo)
	
	: LView(inPaneInfo, inViewInfo)
{
}

// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewCell						Stream Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewCell::CViewCell(
	LStream*	inStream)
	
	: LView(inStream)
{
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ~CViewCell						Destructor				  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewCell::~CViewCell()
{
}


/*______________________________________________________________________________

	Click
*/

void
CViewCell::Click(SMouseDownEvent	&/*inMouseDown*/)
{
	BroadcastMessage( msg_Games_CellClicked, this );
}


/*______________________________________________________________________________

	HighlightCell
	
*/

void
CViewCell::HighlightCell()
{
	RGBColor hiliteColor;

	if(UEnvironment::HasFeature(env_HasAppearance11))
	{
   		GetThemeBrushAsColor(kThemeBrushDragHilite,16,true,&hiliteColor);
	}            
	else 
	{
    	hiliteColor.red     = 0xB798;
    	hiliteColor.green     = 0xB798;
    	hiliteColor.blue     = 0xB798;
	}            

	
	LBorderAttachment * theBorder = new LBorderAttachment();
	LPaintAttachment * thePaint = new LPaintAttachment(nil, &hiliteColor, nil, true);
	
	mBorder = theBorder;
	mPaint	= thePaint;
	AddAttachment(theBorder, nil, true);
	AddAttachment(thePaint, nil, true);
	
	Refresh();
}


/*______________________________________________________________________________

	UnhighlightCell
	
*/

void
CViewCell::UnhighlightCell()
{
	if (mBorder != nil)
	{
		RemoveAttachment( mBorder );
		RemoveAttachment( mPaint );
	}
	Refresh();
	
	
}


/*______________________________________________________________________________

	GetmSelected
	
*/

Boolean
CViewCell::IsCellSelected()
{
	return mSelected;
}

/*______________________________________________________________________________

	SetCellSelected
	
*/

void
CViewCell::SetCellSelected(Boolean position)
{
	mSelected = position;
}


/*______________________________________________________________________________

	SetGameInfoPtr
	
*/

void
CViewCell::SetGameInfoPtr(GameInfoP thePtr)
{
	mGameInfoPtr = thePtr;
}


/*______________________________________________________________________________

	GetGameInfoPtr
	
*/

GameInfoP
CViewCell::GetGameInfoPtr()
{
	return	mGameInfoPtr;
}

PP_End_Namespace_PowerPlant
