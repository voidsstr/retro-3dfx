
#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include "CViewColumn.h"
#include "CViewCell.h"
#include "CControlPanelApp.h"
#include "ControlPanelPPob.h"

#include <LCaption.h>
#include <LScroller.h>
#include <LPushButton.h>

#include <LBroadcaster.h>
#include <LListener.h>
#include <PP_Messages.h>
#include <TArrayIterator.h>
#include <URegions.h>
#include <UReanimator.h>

#include <string.h>

PP_Begin_Namespace_PowerPlant

#define		CellHeight				50

// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewColumn						Default Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewColumn::CViewColumn()
{
	StartupInit();
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewColumn						Copy Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewColumn::CViewColumn(
	const CViewColumn&	inOriginal)

	: LView(inOriginal)
{
	StartupInit();
}


// ---------------------------------------------------------------------------
//	¥ CViewColumn						Parameterized Constructor [public]
// ---------------------------------------------------------------------------

CViewColumn::CViewColumn(
	const SPaneInfo&	inPaneInfo,
	const SViewInfo&	inViewInfo)
	
	: LView(inPaneInfo, inViewInfo)
{
	StartupInit();
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CViewColumn						Stream Constructor		  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewColumn::CViewColumn(
	LStream*	inStream)
	
	: LView(inStream)
{
	StartupInit();
	
}



// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ~CViewColumn						Destructor				  [public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

CViewColumn::~CViewColumn()
{
}


/*______________________________________________________________________________

	FinishCreateSelf

*/
void
CViewColumn::StartupInit()
{
	SetLastCellSelected ( nil );
}

/*______________________________________________________________________________

	FinishCreateSelf

*/

void
CViewColumn::FinishCreateSelf()
{
	LArray * 	theGamesList;
	GameInfoP 	thePtr;
	int i = 1;
	
	// Get AppList from ???
	
	theGamesList = GetCPGamesList();
	
	LArrayIterator	iterate( *theGamesList, LArrayIterator::index_BeforeStart );
	
	while( iterate.Next( &thePtr ) )
	{
		AddCell(thePtr);
	}

}



/*______________________________________________________________________________

	AddCell

*/

void
CViewColumn::AddCell(GameInfoP  thePtr)
{
	GameInfoP  currentPtr = thePtr;
	
	// create the cell from the masterCell
	
	CViewCell*	theNewCell = (CViewCell*) UReanimator::CreateView(PP_AppInfoCell, this, (LCommander*) this);
	ThrowIfNil_(theNewCell);
	
	currentPtr->GameCellPtr = theNewCell;
	theNewCell->SetGameInfoPtr ( currentPtr );
	
	ResizeImageBy(0, CellHeight, true);
	
	// Set Cell content
	SetCellContent (theNewCell, currentPtr);
	
	// and tweak a few things
	theNewCell->PutInside( this );
	theNewCell->PlaceInSuperFrameAt(0, (((currentPtr)->GameNumber)-1)*CellHeight-1, Refresh_Yes);
	//theNewCell->PlaceInSuperFrameAt(0, (nbr-1)*CellHeight, Refresh_Yes);
	
	Refresh();
	
	theNewCell->AddListener( (LListener*)this );

	
}


/*______________________________________________________________________________

	ListenToMessage

*/

void
CViewColumn::ListenToMessage(MessageT inMessage, void *ioParam)
{
	
	if ( inMessage == msg_Games_CellClicked) {
		SelectCell((CViewCell *) ioParam);
		
	}
}

/*______________________________________________________________________________

	SetCellSelected

*/

void
CViewColumn::SelectCell(CViewCell *inCell)
{
	CViewCell * lastCell;
	CViewCell *	currentCell = inCell;
	LView *		GameMainView;
	
	
	
	GameMainView = this->GetSuperView()->GetSuperView();
	
	lastCell = GetLastCellSelected();
	
	if (currentCell != lastCell)
	{
		// UnHighlight it
		if (lastCell == nil)
		{
			LPushButton * EditButton = (LPushButton*) GameMainView->FindPaneByID (game_ButtEdit);
			EditButton->Enable();
		}
		else
		{
			lastCell->UnhighlightCell();
			SetCellSelected(currentCell, false);
		}
		// Highlight currentCell
		currentCell->HighlightCell();
		
		// Set currentCell to Selected
		SetCellSelected(currentCell, true);
		
		
	}
	
}


/*______________________________________________________________________________

	SetCellSelected

*/

void
CViewColumn::SetCellSelected(CViewCell * inCell, Boolean position)
{
	
	inCell->SetCellSelected(position);
	if (position)
		SetLastCellSelected ( inCell );
}


/*______________________________________________________________________________

	GetLastCellSelected

*/

CViewCell *
CViewColumn::GetLastCellSelected()
{
	
	return mLastCellSelected;
}


/*______________________________________________________________________________

	GetLastCellSelected

*/

void
CViewColumn::SetLastCellSelected(CViewCell * inCell)
{
	mLastCellSelected = inCell;
}

/*______________________________________________________________________________

	IsCellSelected

*/

Boolean
CViewColumn::IsCellSelected(CViewCell * inCell)
{
	return inCell->IsCellSelected();
}


/*______________________________________________________________________________

	GetAppList

*/

LArray *
CViewColumn::GetCPGamesList()
{
	// Get the list of the games to add in the games tabs list ...
	
	//CNameComparator *alphaComparator = new CNameComparator;
	//ThrowIfNil_(alphaComparator);
	
	// create the variable array, with our comparator and keep it sorted
	
	LArray *theCPGamesList = new LArray( sizeof( GameInfoP ) );
	ThrowIfNil_(theCPGamesList);
	
	// Read the old list of GameInfo
	
	theCPGamesList = InsertGames2List (theCPGamesList);
	
	return theCPGamesList;
}


/*______________________________________________________________________________

	InsertGames2List

*/

LArray *
CViewColumn::InsertGames2List(LArray *theList)
{
	int i = 1;
	GameInfoP	theGIPtr;
	
	while (i < 15)
	{
		// Allocate some memory
		theGIPtr = new GameInfo;
		ThrowIfNil_(theGIPtr);
		
		// Put something inside
		strcpy(theGIPtr->GameName, "Quake III : Arena");
		strcpy(theGIPtr->GameInfos, "API: OpenGL, Res: 800x600, AA: Disabled");
		
		theGIPtr->GameNumber = i;
		theGIPtr->GameSelected = 1;
		
		
		theList->InsertItemsAt(1, LArray::index_Last, &theGIPtr);
		
		i++;
	}
	
	return theList;
}


/*______________________________________________________________________________

	SetCellContent

*/

void
CViewColumn::SetCellContent (CViewCell * inCell, GameInfoP  inGInfo)
{
	LCaption * theCaption;
	Str255	temp;
	
	// Set Game Title
	theCaption = (LCaption*) inCell->FindPaneByID(game_Title);
	strcpy((char *) &temp[1], (inGInfo)->GameName);
	temp[0] = strlen( (inGInfo)->GameName );
	theCaption->SetDescriptor(temp);
	
	// Set Game Sum up Informations
	theCaption = (LCaption*) inCell->FindPaneByID(game_Infos);
	strcpy((char *) &temp[1], (inGInfo)->GameInfos);
	temp[0] = strlen( (inGInfo)->GameInfos );
	theCaption->SetDescriptor(temp);
	
	// Set Game Number
	theCaption = (LCaption*) inCell->FindPaneByID(game_Num);
	theCaption->SetValue((inGInfo)->GameNumber);
	
}


PP_End_Namespace_PowerPlant
