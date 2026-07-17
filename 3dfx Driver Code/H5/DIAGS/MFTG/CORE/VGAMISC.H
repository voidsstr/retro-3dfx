/*
 * vgamisc.h
 *
 */
#ifndef MDC_VGAMISC_H
#define MDC_VGAMISC_H

#include "3dfx.h"
#include "etypes.h"

// Color reference
typedef struct tagCOLORREF {
	BYTE			bpp1;				// 2 colors, monochrome
	BYTE			bpp2;				// 4 colors
	BYTE			bpp4;				// 16 colors
	BYTE			bpp8;				// 256 colors
	WORD			bpp15;			// 32K colors
	WORD			bpp16;			// 64K colors
	DWORD			bpp24;			// 16.7M colors
} COLORREF;
typedef	COLORREF							*LPCOLORREF;


extern LPBYTE      lpFont;           /* ptr to current font */
extern LPCOLORREF  lpclrTextFore;    /* ptr to current foreground color */
extern LPCOLORREF  lpclrTextBack;    /* ptr to current background color */
extern BOOL        bOpaque;          /* opaque vs transparent flag */


#ifndef BYTE
typedef	unsigned char			BYTE;
#endif

#ifndef _DATAFILE_
extern	BYTE			tblParameterTable[];
extern	BYTE			tbl16ColorDAC[];
extern	BYTE			tblCGADAC[];
extern	BYTE			tblMonoDAC[];
extern	BYTE			tbl256ColorDAC[];
extern	BYTE			tblFont8x14[];
extern	BYTE			tblFont8x16[];
extern	BYTE			tblFont8x8[];
extern	BYTE			tblFont9x14[];
extern	BYTE			tblFont9x16[];
extern	BYTE			tblSmallParameterTable[];
#endif


FxBool SetMode(FxU16 wMode);
FxU16 GetMode(void);

#endif
