
#ifndef _CVIEWCELL_H_
#define _CVIEWCELL_H_



#include <LView.h>
#include <LBroadcaster.h>
#include <UAttachments.h>
#include <LArray.h>
#include "ControlPanelPPob.h"

// ---------------------------------------------------------------------------

class CViewCell : public LView, public LBroadcaster {
public:
	enum { class_ID = FOUR_CHAR_CODE('ccel') };
	
						CViewCell();
						
						CViewCell(
							const CViewCell&	inOriginal);
							
						CViewCell(
							const SPaneInfo&	inPaneInfo,
							const SViewInfo&	inViewInfo);
						
						CViewCell(
							LStream*			inStream);
								
	virtual				~CViewCell();
	
	Boolean				IsCellSelected();
	
	void				SetCellSelected(
							Boolean				position);
	
	void				Click(
							SMouseDownEvent		&inMouseDown);
	
	void				HighlightCell();
	
	void				UnhighlightCell();
							
	void				SetGameInfoPtr(
							GameInfoP 			thePtr);
	
	GameInfoP 			GetGameInfoPtr();
	
private:
	
	Boolean				mSelected;
	LBorderAttachment*	mBorder;
	LPaintAttachment*	mPaint;
	GameInfoP			mGameInfoPtr;
	
};

#endif /* _CVIEWCELL_H_ */