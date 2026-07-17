/*******************************************************************************\
|																				|
| CURLPushButton.cp ©1997-1998 John C. Daub.  All rights reserved				|
|																				|
| See the file "CURLPushButton README" for full details, instructions, changes, |
| licensing agreement, etc.  Due to the important information included in that	|
| file, if you did not receive a copy of it, please contact the author for		|
| a copy.																		|
|																				|
| John C. Daub <mailto:hsoi@eden.com>											|
| <http://www.eden.com/~hsoi/>  <http://www.eden.com/~hsoi/prog.html>			|
|																				|
\*******************************************************************************/


#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif


#include "CURLPushButton.h"

#include <LString.h>
#include <UTextTraits.h>
#include <LStream.h>
#include <UDrawingUtils.h>
#include <PP_Constants.h>
#include <LDragAndDrop.h>
#include <LDragTask.h>
#include <UCursor.h>

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif


#if ( __PowerPlant__ < 0x01608000 )

// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CreateFromStream							[static, public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	The CreateFromStream routine, only used if compiled in conjunction
//	with versions of PowerPlant less than v1.6 (CW11).  With CW11/v1.6,
//	the RegisterClass_() macro makes these CreateFromStream routines
//	unnecessary, so we'll conditionally preprocess it out

CURLPushButton*	
CURLPushButton::CreateFromStream(
	LStream *inStream )
{
#ifdef DEBUG_NEW
	return ( NEW CURLPushButton( inStream ) );
#else
	return ( new CURLPushButton(inStream) );
#endif
}

#endif


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CURLPushButton							[public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Default constructor

CURLPushButton::CURLPushButton()
	: mURL("\p"),
	mTitleText("\p")
{
	mTextTraitsID = 0;
	mHiliteTextTraitsID = -1;
	mUseHiliteColor = false;
	mUseInvertRect = false;	// we no longer support the use of inverted rect
	mValue = Button_Off;	// no way in PP to set mValue w/o broadcasting :(

#if _CURLPushButton_DO_DND_
	mDoDragAndDrop = true;
#endif

	mCursorID = 0;
	
	SetHiliteColor( 0, 0, 0 );
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CURLPushButton							[public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Copy constructor

CURLPushButton::CURLPushButton(
	const CURLPushButton &inOriginal )
		: LControl(inOriginal),
		mURL(inOriginal.mURL),
		mTitleText(inOriginal.mTitleText)
{	
	mTextTraitsID = inOriginal.mTextTraitsID;
	mHiliteTextTraitsID = inOriginal.mHiliteTextTraitsID;
	mUseHiliteColor = inOriginal.mUseHiliteColor;
	mUseInvertRect = false; // we no longer support the use of invert rect
	
#if _CURLPushButton_DO_DND_
	mDoDragAndDrop = inOriginal.mDoDragAndDrop;
#endif

	RGBColor theColor;
	inOriginal.GetHiliteColor( theColor );
	SetHiliteColor( theColor );
	
	mCursorID = inOriginal.mCursorID;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ operator=									[public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Assignment operator

CURLPushButton&
CURLPushButton::operator=(
	const CURLPushButton &inOriginal )
{
	// check for self-assignment
	
	if ( this != &inOriginal ) {

		// pass it back up the chain
		
		LControl::operator=(inOriginal);

		// initialize
		mTextTraitsID = inOriginal.mTextTraitsID;
		mHiliteTextTraitsID = inOriginal.mHiliteTextTraitsID;
		mUseHiliteColor = inOriginal.mUseHiliteColor;
		mUseInvertRect = false; // we no longer support the use of invert rect
		
		mURL = inOriginal.mURL;
		mTitleText = inOriginal.mTitleText;
		
	#if _CURLPushButton_DO_DND_
		mDoDragAndDrop = inOriginal.mDoDragAndDrop;
	#endif

		RGBColor theColor;
		inOriginal.GetHiliteColor( theColor );
		SetHiliteColor( theColor );

		mCursorID = inOriginal.mCursorID;
	}
	
	return *this;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CURLPushButton							[public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Parameterized constructor

CURLPushButton::CURLPushButton(
	const SPaneInfo		&inPaneInfo,
	const SURLpbInfo	&inParamInfo,
	const ResIDT		inCursorID )
		: LControl(	inPaneInfo,
					inParamInfo.valueMessage,
					inParamInfo.value,
					inParamInfo.minValue,
					inParamInfo.maxValue ),
		mURL(inParamInfo.URL),
		mTitleText(inParamInfo.titleText)
{
	mTextTraitsID = inParamInfo.textTraitsID;
	mHiliteTextTraitsID = inParamInfo.hiliteTextTraitsID;
	mUseHiliteColor = inParamInfo.useHiliteColor;

	// we no longer support the use of the inverted rect, so we'll
	// just force this to false.
	mUseInvertRect = false;
	
#if _CURLPushButton_DO_DND_
	mDoDragAndDrop = true;
#endif

	SetHiliteColor( inParamInfo.hiliteColor );
	
	mCursorID = inCursorID;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CURLPushButton							[public]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	LStream constructor

CURLPushButton::CURLPushButton(
	LStream *inStream )
		: LControl( inStream )
{
	mValue = Button_Off;
	mUseInvertRect = false;
		
	inStream->ReadPString(mTitleText);
	inStream->ReadPString(mURL);
	inStream->ReadData( &mTextTraitsID, sizeof(ResIDT) );
	inStream->ReadData( &mHiliteTextTraitsID, sizeof(ResIDT) );
	inStream->ReadData( &mUseHiliteColor, sizeof(Boolean) );
	inStream->ReadData( &mHiliteColor, sizeof(RGBColor) );
	inStream->ReadData( &mCursorID, sizeof(ResIDT) );

	// we have 32 * 4 bytes of padding in the CTYP (v2.4). We need
	// to move the marker in the stream up past the padding so
	// that subclasses that add information to the CTYP will function
	// properly.
	
	inStream->SetMarker( 4 * sizeof(Int32), streamFrom_Marker );

#if _CURLPushButton_DO_DND_
	mDoDragAndDrop = true;
#endif

}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ~CURLPushButton							[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Destructor

CURLPushButton::~CURLPushButton()
{
	// nothing
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetValue									[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Technically we don't need a SetValue routine for CURLPushButton...it
//	does nothing.  However since LControl does define this method
//	(and if called, we do NOT want LControl::SetValue to be called)
//	we need to override SetValue just to hide LControl's version.

void
CURLPushButton::SetValue(
	Int32 inValue )
{
#pragma unused(inValue)

	// just for giggles....
	
	mValue = Button_Off;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetURL									[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the URL associated with the button.
//
//	GetDescriptor returns the title of the button.  Many times the URL
//	and the title of the button will be the same, but they do not have to
//	be (e.g. button title is your name and URL points to your web page).
//	
//	When you want the URL, you should call GetURL and NOT GetDescriptor.
//	GetURL is also set up so that if mURL is nil, it will automatically
//	return the button title (for added flexibility).  However, there is
//	no check made to ensure the button title is a valid URL.

StringPtr
CURLPushButton::GetURL(
	Str255 outURL ) const
{
	if ( mURL.Length() == 0 ) {

		// URL string is empty so return the button title
		return LString::CopyPStr( mTitleText, outURL );		
	
	} else {

		// URL string is not empty, so return it
		return LString::CopyPStr( mURL, outURL );
	}
}

//	This is another version of GetURL, however it does not take any
//	input parameters.

StringPtr
CURLPushButton::GetURL()
{
	if ( mURL.Length() == 0 ) {
		return mTitleText;
	} else {
		return mURL;
	}
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetURL									[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the URL to the specified input parameter.

void
CURLPushButton::SetURL(
	ConstStr255Param inURL )
{
	mURL = inURL;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDescriptor								[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Return the title text of the button
	
StringPtr
CURLPushButton::GetDescriptor(
	Str255	outDescriptor) const
{
	return LString::CopyPStr(mTitleText, outDescriptor);
}

StringPtr
CURLPushButton::GetDescriptor()
{
	return mTitleText;
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetDescriptor								[public, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Reset button title and update region

void
CURLPushButton::SetDescriptor(
	ConstStr255Param inDescriptor)
{
	mTitleText = inDescriptor;
	Refresh();
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HotSpotResult								[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Unlike LTextButton, which behaves like a radio button, CURLPushButton
//	behaves like a push button.

void
CURLPushButton::HotSpotResult(
	Int16 inHotSpot )
{
	// Undo the hilighting
	
	HotSpotAction ( inHotSpot, false, true );

	// broadcast the value message...even tho the value doesn't
	// change, send the message to inform listeners that the
	// button was clicked
	
	BroadcastValueMessage();
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HotSpotAction								[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

void
CURLPushButton::HotSpotAction(
	Int16	inHotSpot,
	Boolean	inCurrInside,
	Boolean	inPrevInside)
{
#pragma unused(inHotSpot)

	Int16 theJust = teCenter; // declare and give it some default value
	
	if (inCurrInside) {
		if (!inPrevInside) {
		
			// click is inside the button (depressed/hilited state)
			
			// get the button frame
			
			Rect	frame;
			CalcLocalFrameRect(frame);
			
			// we no longer support the invert rect technique, but
			// just as sanity...
			
			Assert_(mUseInvertRect == false);			
			
			if ( GetHiliteTextTraitsID() > -1 ) {

				// we're using a second text trait, so set it up
				
				// need to erase first to clean things up
				
				ApplyForeAndBackColors();
				::EraseRect(&frame);
				theJust = UTextTraits::SetPortTextTraits( GetHiliteTextTraitsID() );
			
			} else if ( GetUseHiliteColor() ) {

				// instead we'll reuse the main traits, but just a custom color
				
				theJust = UTextTraits::SetPortTextTraits( GetTextTraitsID() );
				RGBColor theHiliteColor;
				GetHiliteColor( theHiliteColor );
				::RGBForeColor( &theHiliteColor );

			} else {

				// nada specified, so just invert!
				
				theJust = UTextTraits::SetPortTextTraits( GetTextTraitsID() );
				::TextMode( srcXor );
			}

			// check the justification
			
			if ( theJust == teFlushDefault ) {
				theJust = ::GetSysDirection();
			}
			
			// finally, draw it
						
			UTextDrawing::DrawWithJustification((Ptr)&mTitleText[1], mTitleText.Length(),
											frame, theJust );
		}

	} else {
	
		if (inPrevInside) {
		
			// click, but moved outside the button (tracking)
			
			Rect	frame;
			CalcLocalFrameRect(frame);
			
			// we no longer support the invert rect as a technique, but just
			// for sanity
			
			Assert_(mUseInvertRect == false);

			// since we've moved outside the button, the button should
			// just look like it does normally.
			

			// need to erase first to clean things up
			ApplyForeAndBackColors();
			::EraseRect(&frame);

			theJust = UTextTraits::SetPortTextTraits(GetTextTraitsID());
			if (theJust == teFlushDefault) {
				theJust = ::GetSysDirection();
			}
						
			UTextDrawing::DrawWithJustification((Ptr)&mTitleText[1], mTitleText.Length(),
											frame, theJust );
		}
	}
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ DrawSelf									[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

void
CURLPushButton::DrawSelf()
{
	StTextState		origTextState;
	StColorPenState	origCPenState;

	// Configure the text state. If the button is selected,
	// modify the text style.

	Int16 theJust = UTextTraits::SetPortTextTraits(GetTextTraitsID());

	// get the color of the text
	
	RGBColor	textColor;
	::GetForeColor( &textColor );
	
	ApplyForeAndBackColors();
	
	if ( !IsActive() ) {
		textColor.red = textColor.red + 65535 >> 1;
		textColor.green = textColor.green + 65535 >> 1;
		textColor.blue = textColor.blue + 65535 >> 1;
	}
	
	::RGBForeColor( &textColor );
	
	// Draw the text.
	
	Rect theFrame;
	CalcLocalFrameRect(theFrame);

	if (theJust == teFlushDefault) {
		theJust = ::GetSysDirection();
	}
	
	UTextDrawing::DrawWithJustification((Ptr)&mTitleText[1], mTitleText.Length(),
										theFrame, theJust);	
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ActivateSelf								[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	We need to force a redraw when the pane becomes active so that it
//	will draw properly.

void
CURLPushButton::ActivateSelf()
{
	if ( mActive == triState_On ) {
		Refresh();
	}
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ DeactivateSelf							[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	We need to force a redraw when the pane becomes deactivated

void
CURLPushButton::DeactivateSelf()
{
	if ( mActive == triState_Off || mActive == triState_Latent ) {
		Refresh();
	}
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ AdjustCursorSelf							[protected, self]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Changes the cursor to the specified (via PPob or however) cursor. Aids
//	in giving feedback to the user about the control (that they can click on
//	it, etc.).
//
//	Thanx to Nathan Duran and Bill Modesitt for the idea.

void
CURLPushButton::AdjustCursorSelf(
	Point				/* inPortPt */,
	const EventRecord	&inMacEvent)
{
#pragma unused(inMacEvent)	// to quiet the compiler

	// if we're supporting drag and drop, we'll force to the arrow
	// if the shift key is down (that's the magic key that allows
	// the drag and drop to work). That should give some feedback
	// that something else can be done.
	
	// actually, MacOS 8's new "arrow +" cursor would really be
	// useful here, but I'll leave adding support for that as an exercise
	// for the reader ;-)
	
	ResIDT cursID = mCursorID;
	
#if _CURLPushButton_DO_DND_
	if ( (inMacEvent.modifiers & shiftKey) && GetDoDragAndDrop() ) {
		cursID = 0;
	}
#endif

	UCursor::SetTheCursor(cursID);

}


// the following routines are only utilized if we are supporting drag and
// drop, so let's process them out to save on code size if people don't
// want them.

#if _CURLPushButton_DO_DND_

// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ClickSelf									[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Provides us some drag and drop capabilities with the control. The HIG
//	of this is a bit odd since this is a control, so we'll only do this
//	if the shift key is held down. Else, just click as normal.
//
//	Code courtesy of Matt Brunk.

void
CURLPushButton::ClickSelf(
	const SMouseDownEvent &inMouseDown)
{
	// Only do a drag if we want to do it for this object, the Drag Manager is
	// present and the Shift key is pressed.

	if ( GetDoDragAndDrop() &&
			LDragAndDrop::DragAndDropIsPresent() &&
			(inMouseDown.macEvent.modifiers & shiftKey) ) {

		// If we leave the window, the drag manager will be changing thePort,
		// so we'll make sure thePort remains properly set.

		mSuperView->FocusDraw();
		CreateDragEvent(inMouseDown);
		mSuperView->OutOfFocus(nil);

	} else {
		LControl::ClickSelf(inMouseDown);
	}
}


// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CreateDragEvent							[protected, virtual]
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Establish the drag and drop event. Strip the URL of any extraneous
//	characters and do the drag task.
//
//	Code courtesy of Matt Brunk.

void
CURLPushButton::CreateDragEvent(
	const SMouseDownEvent &inMouseDown )
{
	void	*ptr;
	Int32	length;
	Str255	theURL;
	
	GetURL( theURL );
	
	// If the URL is enclosed by '<' and '>', we get the enclosing text.

	if (	(theURL[0] >= 3)	&&
			(theURL[1] == '<')		&&
			(theURL[theURL[0]] == '>') ) {

		ptr    = static_cast<void *>(&theURL[2]);	// character after '<'
		length = theURL[0] - 2;						// character before '<'

	} else {

		ptr    = static_cast<void *>(&theURL[1]);	// all of the text
		length = theURL[0];	
	}
	
	Rect	theRect;
	CalcLocalFrameRect(theRect);

	LDragTask theDragTask(	inMouseDown.macEvent,
							theRect,
							1,						// Item Reference
							'TEXT',					// flavor type
							ptr,					// pointer to text
							length,					// length of text
							0L						// flavor flags
							);

}

#endif // _CURLPushButton_DO_DND_


// the following functions are declared inline in the header file. The #pragma
// mark let's them show up in the CW IDE's function popup for ease of navigation
// and reference. :-)

#pragma mark CURLPushButton::GetTextTraitsID
#pragma mark CURLPushButton::SetTextTraitsID
#pragma mark CURLPushButton::GetHiliteTextTraitsID
#pragma mark CURLPushButton::SetHiliteTextTraitsID
#pragma mark CURLPushButton::GetUseHiliteColor
#pragma mark CURLPushButton::SetUseHiliteColor
#pragma mark CURLPushButton::GetHiliteColor
#pragma mark CURLPushButton::SetHiliteColor
#pragma mark CURLPushButton::GetDoDragAndDrop
#pragma mark CURLPushButton::SetDoDragAndDrop