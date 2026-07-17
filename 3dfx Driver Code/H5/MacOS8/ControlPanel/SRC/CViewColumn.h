
#ifndef _CVIEWCOLUMN_H_
#define _CVIEWCOLUMN_H_


#include <LView.h>
#include <LListener.h>
#include "CViewCell.h"
#include "ControlPanelPPob.h"


// ---------------------------------------------------------------------------

class CViewColumn : public LView, public LListener {
public:
	enum { class_ID = FOUR_CHAR_CODE('vcol') };
	
						CViewColumn();
						
						CViewColumn(
							const CViewColumn&	inOriginal);
							
						CViewColumn(
							const SPaneInfo&	inPaneInfo,
							const SViewInfo&	inViewInfo);
						
						CViewColumn(
							LStream*			inStream);
								
	virtual				~CViewColumn();
	
	void				StartupInit();

	void				FinishCreateSelf();
	
	void				AddCell(
							GameInfoP  		thePtr);

	void				ListenToMessage(
							MessageT 			inMessage, 
							void *				ioParam);
							
		
	void				SelectCell(
							CViewCell *			inCell);
							
	void				SetCellSelected(
							CViewCell * 		inCell, 
							Boolean 			position);
								
	Boolean				IsCellSelected(
							CViewCell *			inCell);
	
	CViewCell *			GetLastCellSelected();
	
	void				SetLastCellSelected(
							CViewCell *			inCell);
							
	LArray *			GetCPGamesList();
	
	LArray *			InsertGames2List(
							LArray *			theList);
							
	void				SetCellContent (
							CViewCell * 		inCell, 
							GameInfoP 			inGInfo);
							
private:

	CViewCell * 		mLastCellSelected;

};


#endif /* _CVIEWCOLUMN_H_ */
