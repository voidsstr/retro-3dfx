//
//		GLOBAL.CPP - Variable declarations for global variables defined in vgaint.h.
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
#include "vgaint.h"

BYTE byPS2Setup;
BYTE byVGASetup;
BYTE byAdapterEnable;
BYTE byMBEnable;
BYTE byCRTCAddr;
BYTE byCRTCReg[25];
BYTE byInputStatus1;
BYTE byATCAddr;
BYTE byATCState;
BYTE byATCReg[21];
BYTE bySEQAddr;
BYTE bySEQReg[5];
BYTE byDACMask;
BYTE byDACIndex;
BYTE byDACState;
BYTE byDACColor;
BYTE byFeatControl;
BYTE byMiscReg;
BYTE byGDCAddr;
BYTE byGDCReg[9];
BYTE byGlobalCaptureMode;
BYTE byGlobalCursorBlink;
BYTE byGlobalAttrBlink;
BYTE byGlobalContinous;

WORD wBoardConfig;

DWORD dwDACAccum;
DWORD dwDACReg[256];

char chGlobalCursorSkew;
char chGlobalDispStartSkew;
char chGlobalDispEndSkew;
char chGlobalRetStartSkew;
char chGlobalRetEndSkew;
char chGlobalCbCounter;
char chGlobalLoadCounter;
char chGlobalSREmpty;
char chGlobalHBStartSkew;
char chGlobalHBEndSkew;
char chGlobalLastPixPan[2];
char chGlobalLastBytePan[2];
WORD wGlobalLastCurAddr[2];
BYTE byGlobalLastRowCount[2];
WORD wGlobalLastLineCompare; 
BYTE byGlobalFrameDelay;

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

