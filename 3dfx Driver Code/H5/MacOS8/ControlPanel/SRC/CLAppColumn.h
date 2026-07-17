#ifndef _H_LTextColumn
#define _H_LTextColumn

#include <LColumnView.h>


PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

class	CLAppColumn : public LColumnView {
public:
	enum				{ class_ID = FOUR_CHAR_CODE('apcl') };
	
						CLAppColumn(LStream *inStream);
	virtual				~CLAppColumn();
	
protected:
	//ResIDT			mTemplateViewID;

	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
};

PP_End_Namespace_PowerPlant

#endif