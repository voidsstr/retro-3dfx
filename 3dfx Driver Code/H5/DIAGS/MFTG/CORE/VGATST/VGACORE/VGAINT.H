//
//		VGAINT.H	- Register and bit definituions of the VGA hardware interface.
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
#ifndef _VGAINT_H
#define _VGAINT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "vgasim.h"

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef	__cplusplus
extern "C" {			/* Assume C declarations for C++ */
  #endif					/* __cplusplus */
#endif					/* SOLARIS */

#ifndef	SEEK_SET
#define	SEEK_SET					0		/* seek to an absolute position */
#endif										/* ifndef SEEK_SET */

//	Register definitions for adapter enable simulation
#define	PS2_SETUP				0x0094
#define	BIT_SETUP				0x20
#define	VGA_SETUP				0x0102
#define	BIT_VGAENA				0x01
#define	ADAPTER_ENABLE			0x46E8
#define	BIT_ADAPT_ENA			0x08
#define	BIT_ADAPT_SETUP		0x10
#define	MB_ENABLE				0x03C3
#define	BIT_MBENA				0x01


//	Register definitions for VGA CRT Controller
#define CRTC_MINDEX				0x03B4
#define CRTC_MDATA				0x03B5
#define CRTC_CINDEX				0x03D4
#define CRTC_CDATA				0x03D5
#define CRTC_HORIZ_TOT			0x00
#define CRTC_HDISP_ENA			0x01
#define CRTC_START_HBLANK		0x02
#define CRTC_END_HBLANK			0x03

#define BIT_RDCOMPAT				0x80
#define FLD_DES					0x60
#define SHFT_DES					5
#define FLD_EB						0x1F
#define SHFT_EB					0

#define CMP_EB 					0x3F

#define CRTC_START_HRET			0x04
#define CRTC_END_HRET			0x05

#define BIT_EB5					0x80
#define SHFT_EB5 					2
#define FLD_HRD					0x60
#define SHFT_HRD					5
#define FLD_EHR					0x1F
#define SHFT_EHR					0

#define CRTC_VERT_TOT			0x06
#define CRTC_OVERFLOW			0x07

#define BIT_VRS9					0x80
#define SHFT_VRS9 				2
#define BIT_VDE9					0x40
#define SHFT_VDE9 				3
#define BIT_VT9					0x20
#define SHFT_VT9 					4
#define BIT_LC8					0x10
#define SHFT_LC8 					4
#define BIT_VBS8					0x08
#define SHFT_VBS8 				5
#define BIT_VRS8					0x04
#define SHFT_VRS8 				6
#define BIT_VDE8					0x02
#define SHFT_VDE8 				7
#define BIT_VT8					0x01
#define SHFT_VT8 					8

#define CRTC_PRESET_ROW			0x08

#define FLD_BP						0x60
#define SHFT_BP					5
#define FLD_SRS					0x1F
#define SHFT_SRS					0

#define CRTC_MAX_SCAN_LINE		0x09

#define BIT_DSC					0x80
#define BIT_LC9					0x40
#define SHFT_LC9 					3
#define BIT_VBS9					0x20
#define SHFT_VBS9 				4
#define FLD_MSL					0x1F
#define SHFT_MSL					0

#define CRTC_CURSOR_START		0x0A

#define BIT_CO						0x20
#define FLD_RSCB					0x1F
#define SHFT_RSCB					0

#define CRTC_CURSOR_END			0x0B

#define FLD_CSK					0x60
#define SHFT_CSK					5
#define FLD_RSCE					0x1F
#define SHFT_RSCE					0

#define CRTC_START_HIGH			0x0C
#define CRTC_START_LOW			0x0D
#define CRTC_CURSOR_HIGH		0x0E
#define CRTC_CURSOR_LOW			0x0F
#define CRTC_START_VRET			0x10
#define CRTC_END_VRET			0x11

#define BIT_PR						0x80
#define BIT_S5R					0x40
#define BIT_EVI					0x20
#define BIT_CVI					0x10
#define FLD_VRE					0x0F
#define SHFT_VRE					0

#define CRTC_VDISP_ENA			0x12
#define CRTC_OFFSET				0x13
#define CRTC_UNDERLINE			0x14

#define BIT_DW						0x40
#define BIT_CB4					0x20
#define FLD_SUL					0x1F
#define SHFT_SUL					0

#define CRTC_START_VBLANK		0x15
#define CRTC_END_VBLANK			0x16
#define FLD_VBE 					0x7F
#define CRTC_MODE					0x17

#define BIT_RST					0x80
#define BIT_WB						0x40
#define BIT_ADW					0x20
#define BIT_CB2					0x08
#define BIT_HRS					0x04
#define BIT_SRC					0x02
#define BIT_CMS0					0x01

#define CRTC_LINE_CMP			0x18

// Undocumented CRTC registers
#define CRTC_LATCH            0x22
#define CRTC_TOGGLE           0x24
#define CRTC_ATC              0x26

#define BIT_ATC_TOGGLE        0x80

// Register definitions for VGA Input Status Register 1
#define INPUT_MSTATUS_1			0x03BA
#define INPUT_CSTATUS_1			0x03DA

#define BIT_VR						0x08
#define BIT_DE						0x01


// Register definitions for VGA Attribute Controller
#define ATC_INDEX					0x03C0
#define ATC_RDATA					0x03C1

#define ATC_PAL0					0x00
#define ATC_PAL1					0x01
#define ATC_PAL2					0x02
#define ATC_PAL3					0x03
#define ATC_PAL4					0x04
#define ATC_PAL5					0x05
#define ATC_PAL6					0x06
#define ATC_PAL7					0x07
#define ATC_PAL8					0x08
#define ATC_PAL9					0x09
#define ATC_PAL10					0x0A
#define ATC_PAL11					0x0B
#define ATC_PAL12					0x0C
#define ATC_PAL13					0x0D
#define ATC_PAL14					0x0E
#define ATC_PAL15					0x0F

#define ATC_MODE					0x10

#define BIT_PS						0x80
#define BIT_PW						0x40
#define BIT_PP						0x20
#define BIT_EB						0x08
#define BIT_ELG					0x04
#define BIT_ME						0x02
#define BIT_G						0x01

#define ATC_OVERSCAN				0x11

#define FLD_OVERSCAN 			0x3F

#define ATC_COLOR_PLANE			0x12

#define FLD_ECP					0x0F
#define SHFT_ECP					0

#define ATC_PEL_PAN				0x13

#define FLD_HPP					0x0F
#define SHFT_HPP					0

#define ATC_COLOR_SEL			0x14

#define FLD_SC76					0x0C
#define SHFT_SC76					2
#define FLD_SC54					0x03
#define SHFT_SC54					0

#define BIT_IPAS 					0x20
#define FLD_ATC_ADDR				0x1F

#define ATC_ADDRESS				0
#define ATC_DATA					1


//	Register definition for VGA Input Status Register 0
#define INPUT_RSTATUS_0			0x03C2

#define BIT_CI						0x80
#define BIT_SS						0x10

//	Register definition for VGA Miscellaneous Register
#define MISC_OUTPUT				0x03C2

#define BIT_VSP					0x80
#define BIT_HSP					0x40
#define BIT_PSEL              0x20
#define BIT_CS1					0x08
#define BIT_CS0					0x04
#define BIT_ERAM					0x02
#define BIT_IOS					0x01


//	Register definition for VGA Sequencer
#define SEQ_INDEX					0x03C4
#define SEQ_DATA					0x03C5

#define SEQ_RESET					0x00

#define BIT_SEQ_SR				0x02
#define BIT_ASR					0x01

#define SEQ_CLOCKING				0x01

#define BIT_SO						0x20
#define BIT_SH4					0x10
#define BIT_DC						0x08
#define BIT_SL						0x04
#define BIT_D89					0x01

#define SEQ_MAP_MASK				0x02

#define BIT_M3E					0x08
#define BIT_M2E					0x04
#define BIT_M1E					0x02
#define BIT_M0E					0x01
#define FLD_ME						0x0F
#define SHFT_ME					0

#define SEQ_MAP_SELECT			0x03

#define BIT_MA2					0x20
#define SHFT_MA2 					8		// shift up 8
#define BIT_MB2					0x10
#define SHFT_MB2 					9		// shift up 9
#define BIT_MA1					0x08
#define BIT_MA0					0x04
#define FLD_MAL 					0x0C
#define SHFT_MAL 					12		// shift up 12
#define BIT_MB1					0x02
#define BIT_MB0					0x01
#define FLD_MBL 					0x03
#define SHFT_MBL 					14		// shift up 14 */

#define SEQ_MEM_MODE				0x04

#define BIT_CH4					0x08
#define BIT_SEQ_OE				0x04
#define BIT_EM						0x02


// Register definition for VGA DAC
#define DAC_MASK					0x03C6
#define DAC_RINDEX				0x03C7
#define DAC_WINDEX				0x03C8
#define DAC_DATA					0x03C9

#define DAC_RED					0
#define DAC_GRN					1
#define DAC_BLU					2

#define DAC_READ					0x3
#define DAC_WRITE					0x0


// Register definition for VGA Feature Control
#define FEAT_CONTROLR			0x03CA
#define FEAT_CCONTROL			0x03DA
#define FEAT_MCONTROL			0x03BA

// Register definition for VGA Miscellaneous Input
#define MISC_INPUT				0x03CC

// Register definition for VGA Graphics Controller
#define GDC_INDEX					0x03CE
#define GDC_DATA					0x03CF

#define GDC_SET_RESET			0x00

#define BIT_SR3					0x08
#define BIT_SR2					0x04
#define BIT_SR1					0x02
#define BIT_SR0					0x01
#define FLD_SR						0x0F
#define SHFT_SR					0

#define GDC_ENA_SR				0x01

#define BIT_ESR3					0x08
#define BIT_ESR2					0x04
#define BIT_ESR1					0x02
#define BIT_ESR0					0x01
#define FLD_ESR					0x0F
#define SHFT_ESR					0

#define GDC_COLOR_CMP			0x02

#define BIT_CC3					0x08
#define BIT_CC2					0x04
#define BIT_CC1					0x02
#define BIT_CC0					0x01
#define FLD_CC						0x0F
#define SHFT_CC					0

#define GDC_DATA_ROT				0x03

#define BIT_FS1					0x10
#define BIT_FS0					0x08
#define FLD_FS						0x18
#define SHFT_FS					3
#define FLD_FS_NOOP				0x00
#define FLD_FS_AND				0x08
#define FLD_FS_OR					0x10
#define FLD_FS_XOR				0x18

#define BIT_RC2					0x04
#define BIT_RC1					0x02
#define BIT_RC0					0x01
#define FLD_RC						0x07
#define SHFT_RC					0

#define GDC_MAP_SEL				0x04

#define FLD_MS						0x3
#define SHFT_MS					0
#define FLD_MS_0					0x00
#define FLD_MS_1					0x01
#define FLD_MS_2					0x02
#define FLD_MS_3					0x03

#define GDC_MODE					0x05

#define BIT_C256					0x40
#define BIT_GDC_SR				0x20
#define BIT_GDC_OE				0x10
#define BIT_RM						0x08
#define BIT_WM1					0x02
#define BIT_WM0					0x01

#define FLD_WM						(BIT_WM1|BIT_WM0)
#define FLD_WM_0					0x00
#define FLD_WM_1					0x01
#define FLD_WM_2					0x02
#define FLD_WM_3					0x03

#define GDC_MISC					0x06

#define BIT_MM1					0x08
#define BIT_MM0					0x04

#define FLD_MM						(BIT_MM1|BIT_MM0)
#define FLD_MM_A0BF				0x00
#define FLD_MM_A0AF				0x04
#define FLD_MM_B0B7				0x08
#define FLD_MM_B8BF				0x0C

#define BIT_OE						0x02
#define BIT_GM						0x01

#define GDC_COLOR_DC				0x07

#define FLD_MX						0x0F
#define SHFT_MX					0

#define GDC_BIT_MASK				0x08


//	Register storage for VGA Sequencer
extern BYTE bySEQAddr;
extern BYTE bySEQReg[5];

//	Register storage for VGA Miscellaneous Register
extern BYTE byMiscReg;


// Register storage for VGA CRT Controller
extern BYTE byCRTCAddr;
extern BYTE byCRTCReg[25];
extern BYTE byPS2Setup;
extern BYTE byMBEnable;
extern BYTE byVGASetup;
extern BYTE byAdapterEnable;
extern BYTE byInputStatus1;

// Register storage for VGA Attribute Controller
extern BYTE byATCAddr;

extern BYTE byATCState;
extern BYTE byATCReg[21];

// Register storage for VGA DAC
extern BYTE byDACMask;
extern BYTE byDACIndex;

extern BYTE byDACState;
extern BYTE byDACColor;
extern DWORD dwDACAccum;
extern DWORD dwDACReg[256];

// Register  for VGA Feature Control
extern BYTE byFeatControl;

// Register storage for VGA Graphics Controller
extern BYTE byGDCAddr;
extern BYTE byGDCReg[9];

// VGA simulation global state
extern BYTE byGlobalCaptureMode;		// current capture mode for scanout
extern BYTE byGlobalCursorBlink;		// current cursor blink state for

// Scanout
extern BYTE byGlobalAttrBlink;		// current attribute blink state
extern BYTE byGlobalContinous;		// continous scan in progress

// For scanout
extern WORD wBoardConfig;				// current board type (motherboard or adapter)                   */

//   VGA simulation library global variables
extern char chGlobalCursorSkew;		// Counter for cursor skew
extern char chGlobalDispStartSkew;	// Counter for display start skew
extern char chGlobalDispEndSkew;		// Counter for display end skew
extern char chGlobalRetStartSkew;	// Counter for retrace start skew
extern char chGlobalRetEndSkew;		// Counter for retrace end skew
extern char chGlobalCbCounter;		// Counter for count by 2/4
extern char chGlobalLoadCounter;		// Counter for vload 2/4
extern char chGlobalSREmpty;       	// Clear during HR, set when SR OK
extern char chGlobalHBStartSkew;		// Counter for blank start skew
extern char chGlobalHBEndSkew;		// Counter for blank end skew
extern char chGlobalLastPixPan[2];	// Last pixel pan value for continous scan
extern char chGlobalLastBytePan[2];	// Last byte pan value for continous scan
extern WORD wGlobalLastCurAddr[2];	// Last start address for continous scan
extern BYTE byGlobalLastRowCount[2];		// Last start address for continous scan
extern WORD wGlobalLastLineCompare;		// Last line compare for continous scan
extern BYTE byGlobalFrameDelay;		// Frame delay for continous capture

// Pointers to internal plane maps
extern LPBYTE	bfpMap00;
extern LPBYTE	bfpMap01;
extern LPBYTE	bfpMap10;
extern LPBYTE	bfpMap11;
extern LPBYTE	bfpMap20;
extern LPBYTE	bfpMap21;
extern LPBYTE	bfpMap30;
extern LPBYTE	bfpMap31;

// Pointer to small frame parameter table
//extern	BYTE	tblSmallParameterTable[];

// VGA simulator prototypes for internal routines
BYTE MemLinRead (DWORD);
void MemLinWrite (DWORD, BYTE);
DWORD PtrToLin (SEGOFF);

// MAPS.CPP
int  InitMaps (void);
void FreeMaps (void);
BYTE ReadBIOS (WORD);
void WriteBIOS (WORD, BYTE);
BYTE ReadMap (BYTE, WORD);
void WriteMap (BYTE, WORD, BYTE);
LPBYTE AllocateMemory (DWORD);
void FreeMemory (LPBYTE);

// BMP.CPP
int WriteBMP8Header (FILE **, LPSTR, WORD, WORD);
int WriteBMP8Color (FILE *, BYTE);
void CloseBMP8 (FILE *);
void HandleCaptureMode (LPBYTE);
void SetupBMPHeader (LPBYTE, DWORD, WORD, WORD);
void AppendExtension (char *);

// READMODE.CPP
BYTE ReadMode0 (DWORD, BYTE);
BYTE ReadMode1 (DWORD, BYTE);


// WRITMODE.CPP
void WriteMode0 (DWORD, BYTE);
void WriteMode1 (DWORD, BYTE);
void WriteMode2 (DWORD, BYTE);
void WriteMode3 (DWORD, BYTE);

//
BYTE GDCRotate (BYTE);
void GDCSetReset (BYTE *, BYTE, BYTE, BYTE);
void GDCWrite (BYTE *, DWORD, BYTE);
WORD MemAddressMux (DWORD);
void GDC_ALU (BYTE *);

// IO.CPP
void InternalIOByteWrite (WORD, BYTE);
BYTE InternalIOByteRead (WORD);
BYTE AdapterENA (void);

// GRAPH.CPP
WORD GetVGAChar (BYTE *, WORD, BYTE);
WORD GetEGAChar (BYTE *, WORD, BYTE);
WORD GetCGAChar (BYTE *, WORD, BYTE);
WORD GetTextChar (BYTE *, WORD, BYTE);

// CAPTURE.CPP
int InternalCaptureFrame (LPSTR) ;
WORD GetVideoCharacter (BYTE *, WORD, BYTE, BYTE, char, BYTE *);
WORD ScanAddressMUX (WORD, BYTE);
BYTE PaletteMap (BYTE);
WORD NextScanAddr (WORD);
void ShiftReg(BYTE *);
int InitFrameVars (LPSTR);
void FramePrescan ();
WORD HorizWrap(WORD);

// Value returned for read when addressing memory or I/O outside of
//	that defined by the VGA
#define OPEN_BUS 			0xAA

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef __cplusplus
}							/* End of extern "C" { */
  #endif					/* __cplusplus */
#endif					/* SOLARIS */

#endif					/* End of #ifndef _VGAINT_H */

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

