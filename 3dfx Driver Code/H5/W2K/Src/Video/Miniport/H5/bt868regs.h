/* -*-c++-*- */
/* $Header: bt868regs.h, 4, 10/11/00 8:47:13 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   bt868regs.h
**
** Description: BrookTree 868 TV Register Descriptions.
**
** $Revision: 4$
** $Date: 10/11/00 8:47:13 PM$
**
** $History: bt868regs.h $
** 
** 
*/

#ifndef _BT868REGS_H_
#define _BT868REGS_H_

/********************************************************************************
*
* The file Bt868regs.h defined the the BrookTree TV out Registers
*
*
*********************************************************************************/


// 

#define REGx6E 0x6E
#define REGx70 0x70
#define REGx7C 0x7C
#define REGx7E 0x7E
#define REGx80 0x80
#define REGx9A 0x9A
#define REGxA2 0xA2
#define REGxAE 0xAE
#define REGxB0 0xB0
#define REGxB2 0xB2
#define REGxB4 0xB4
#define REGxB8 0xB8
#define REGxBA 0xBA
#define REGxC4 0xC4
#define REGxC6 0xC6
#define REGxC8 0xC8
#define REGxCA 0xCA
#define REGxCC 0xCC
#define REGxCE 0xCE

#define BT86X_REGxBA_DACDISA   BIT(0)
#define BT86X_REGxBA_SLAVER    BIT(5)
#define BT86X_REGxBA_CHECKSTAT BIT(6)
#define BT86X_REGxBA_SRESET    BIT(7)

#define BT86X_REGxC8_DISFFILT  BIT(6)
#define BT86X_REGxC8_DISYFLPF  BIT(7)


//	auto-configuration modes
#define BT868_AC_MODE_0_RGB_NTSC_640X480		0
#define BT868_AC_MODE_1_RGB_PAL_640X480			1
#define BT868_AC_MODE_2_RGB_NTSC_800X600		2
#define BT868_AC_MODE_3_RGB_PAL_800X600			3
#define BT868_AC_MODE_4_YCRCB_NTSC_640X480		4
#define BT868_AC_MODE_5_YCRCB_PAL_640X480		5
#define BT868_AC_MODE_6_YCRCB_NTSC_800X600		6
#define BT868_AC_MODE_7_YCRCB_PAL_800X600		7


#define BT868_REG_BYTE_VAL(x)		*((PUCHAR)&x)

typedef struct tagBT868_REG_76
{
	UCHAR		ucH_CLKO		: 8;
} BT868_REG_76;

typedef struct tagBT868_REG_78
{
	UCHAR		ucH_ACTIVE	: 8;
} BT868_REG_78;

typedef struct tagBT868_REG_7A
{
	UCHAR		ucHSYNC_WIDTH	: 8;
} BT868_REG_7A;

typedef struct tagBT868_REG_7C
{
	UCHAR		ucHBURST_BEGIN	: 8;
} BT868_REG_7C;

typedef struct tagBT868_REG_7E
{
	UCHAR		ucHBURST_END	: 8;
} BT868_REG_7E;

typedef struct tagBT868_REG_80
{
	UCHAR		ucH_BLANKO	: 8;
} BT868_REG_80;

typedef struct tagBT868_REG_82
{
	UCHAR		ucV_BLANKO	: 8;
} BT868_REG_82;

typedef struct tagBT868_REG_84
{
	UCHAR		ucV_ACTIVEO	: 8;
} BT868_REG_84;

typedef struct tagBT868_REG_86
{
	UCHAR		ucH_CLKO		: 4;
	UCHAR		ucH_ACTIVE	: 2;
	UCHAR		ucReserved	: 1;
	UCHAR		ucV_ACTIVEO	: 1;
} BT868_REG_86;

typedef struct tagBT868_REG_88
{
	UCHAR		ucH_FRACT		: 8;
} BT868_REG_88;

typedef struct tagBT868_REG_8A
{
	UCHAR		ucH_CLK		: 8;
} BT868_REG_8A;

typedef struct tagBT868_REG_8C
{
	UCHAR		ucH_BLANK		: 8;
} BT868_REG_8C;

typedef struct tagBT868_REG_8E
{
	UCHAR		ucH_CLKI		: 3;
	UCHAR		ucH_BLANKI	: 1;
	UCHAR		ucVBLANKDLY	: 1;
	UCHAR		ucReserved1	: 1;
	UCHAR		ucReserved2	: 1;
	UCHAR		ucReserved3	: 1;
} BT868_REG_8E;

typedef struct tagBT868_REG_90
{
	UCHAR		ucV_LINESI	: 8;
} BT868_REG_90;

typedef struct tagBT868_REG_92
{
	UCHAR		ucV_BLANKI	: 8;
} BT868_REG_92;

typedef struct tagBT868_REG_94
{
	UCHAR		ucV_ACTIVEI	: 8;
} BT868_REG_94;

typedef struct tagBT868_REG_96
{
	UCHAR		ucV_LINESI	: 2;
	UCHAR		ucV_ACTIVEI	: 2;
	UCHAR		ucYLPF		: 2;
	UCHAR		ucCLPF		: 2;
} BT868_REG_96;

typedef struct tagBT868_REG_98
{
	UCHAR		ucV_SCALE		: 8;
} BT868_REG_98;

typedef struct tagBT868_REG_9A
{
	UCHAR		ucV_SCALE		: 6;
	UCHAR		ucH_BLANKO	: 2;
} BT868_REG_9A;

typedef struct tagBT868_REG_9C
{
	UCHAR		ucPLL_FRACT	: 8;
} BT868_REG_9C;

typedef struct tagBT868_REG_9E
{
	UCHAR		ucPLL_FRACT	: 8;
} BT868_REG_9E;

typedef struct tagBT868_REG_A0
{
	UCHAR		ucPLL_INT		: 6;
	UCHAR		ucBY_PLL		: 1;
	UCHAR		ucEN_XCLK		: 1;
} BT868_REG_A0;

typedef struct tagBT868_REG_A2
{
	UCHAR		ucNI_OUT		: 1;
	UCHAR		ucSETUP		: 1;
	UCHAR		uc625LINE		: 1;
	UCHAR		ucVSYNC_DUR	: 1;
	UCHAR		ucSCRESET		: 1;
	UCHAR		ucRESERVED	: 1;
	UCHAR		ucECLIP		: 1;
	UCHAR		ucEN_SCART	: 1;
} BT868_REG_A2;

typedef struct tagBT868_REG_A4
{
	UCHAR		ucSYNC_AMP	: 8;
} BT868_REG_A4;

typedef struct tagBT868_REG_A6
{
	UCHAR		ucBST_AMP		: 8;
} BT868_REG_A6;

typedef struct tagBT868_REG_A8
{
	UCHAR		ucMCR		: 8;
} BT868_REG_A8;

typedef struct tagBT868_REG_AA
{
	UCHAR		ucMCB		: 8;
} BT868_REG_AA;

typedef struct tagBT868_REG_AC
{
	UCHAR		ucMY			: 8;
} BT868_REG_AC;

typedef struct tagBT868_REG_AE
{
	UCHAR		ucMSC		: 8;
} BT868_REG_AE;

typedef struct tagBT868_REG_B0
{
	UCHAR		ucMSC		: 8;
} BT868_REG_B0;

typedef struct tagBT868_REG_B2
{
	UCHAR		ucMSC		: 8;
} BT868_REG_B2;

typedef struct tagBT868_REG_B4
{
	UCHAR		ucMSC		: 8;
} BT868_REG_B4;

typedef struct tagBT868_REG_B6
{
	UCHAR		ucPHASE_OFF	: 8;
} BT868_REG_B6;

typedef struct tagBT868_REG_B8
{
	UCHAR		ucCONFIG		: 3;
	UCHAR		ucEN_PINCFG	: 1;
	UCHAR		ucReserved1	: 1;
	UCHAR		ucReserved2	: 1;
	UCHAR		ucReserved3	: 1;
	UCHAR		ucReserved4	: 1;
} BT868_REG_B8;

typedef struct tagBT868_REG_BA
{
	UCHAR		ucDACDISA		: 1;
	UCHAR		ucDACDISB		: 1;
	UCHAR		ucDACDISC		: 1;
	UCHAR		ucDACDISD		: 1;
	UCHAR		ucDACOFF		: 1;
	UCHAR		ucReserved	: 1;
	UCHAR		ucCHECK_STAT	: 1;
	UCHAR		ucSRESET		: 1;
} BT868_REG_BA;

typedef struct tagBT868_REG_BC
{
	UCHAR		ucCCF2B1		: 8;
} BT868_REG_BC;

typedef struct tagBT868_REG_BE
{
	UCHAR		ucCCF2B2		: 8;
} BT868_REG_BE;

typedef struct tagBT868_REG_C0
{
	UCHAR		ucCCF1B1		: 8;
} BT868_REG_C0;

typedef struct tagBT868_REG_C2
{
	UCHAR		ucCCF1B2		: 8;
} BT868_REG_C2;

typedef struct tagBT868_REG_C4
{
	UCHAR		ucEN_OUT		: 1;
	UCHAR		ucDCHROMA		: 1;
	UCHAR		ucECBAR		: 1;
	UCHAR		ucECCGATE		: 1;
	UCHAR		ucECCF1		: 1;
	UCHAR		ucECCF2		: 1;
	UCHAR		ucESTATUS		: 2;
} BT868_REG_C4;

typedef struct tagBT868_REG_C6
{
	UCHAR		ucIN_MODE		: 3;
	UCHAR		ucHSYNCI		: 1;
	UCHAR		ucVSYNCI		: 1;
	UCHAR		ucFIELDI		: 1;
	UCHAR		ucEN_DOT		: 1;
	UCHAR		ucEN_BLANKO	: 1;
} BT868_REG_C6;

typedef struct tagBT868_REG_C8
{
	UCHAR		ucF_SELY		: 3;
	UCHAR		ucF_SELC		: 3;
	UCHAR		ucDIS_FFILT	: 1;
	UCHAR		ucDIS_YFLPF	: 1;
} BT868_REG_C8;

typedef struct tagBT868_REG_CA
{
	UCHAR		ucYATTENUATE	: 3;
	UCHAR		ucYCORING		: 3;
	UCHAR		ucDIS_GMSHY	: 1;
	UCHAR		ucDIS_GMUSHY	: 1;
} BT868_REG_CA;

typedef struct tagBT868_REG_CC
{
	UCHAR		ucCATTENUATE	: 3;
	UCHAR		ucCCORING		: 3;
	UCHAR		ucDIS_GMSHC	: 1;
	UCHAR		ucDIS_GMUSHC	: 1;
} BT868_REG_CC;

typedef struct tagBT868_REG_CE
{
	UCHAR		ucOUT_MUXA	: 2;
	UCHAR		ucOUT_MUXB	: 2;
	UCHAR		ucOUT_MUXC	: 2;
	UCHAR		ucOUT_MUXD	: 2;
} BT868_REG_CE;

typedef struct tagBT868_REG_D0
{
	UCHAR		ucCCR_START	: 8;
} BT868_REG_D0;

typedef struct tagBT868_REG_D2
{
	UCHAR		ucCC_ADD		: 8;
} BT868_REG_D2;

typedef struct tagBT868_REG_D4
{
	UCHAR		ucCC_ADD		: 4;
	UCHAR		ucCCR_START	: 1;
	UCHAR		ucEN_ASYNC	: 1;
	UCHAR		ucDIV2		: 1;
	UCHAR		ucMODE2X		: 1;
} BT868_REG_D4;

typedef struct tagBT868_REG_D6
{
	UCHAR		ucLUMADLY		: 2;
	UCHAR		ucOUT_MODE	: 2;
	UCHAR		ucReserved1	: 1;
	UCHAR		ucReserved2	: 1;
	UCHAR		ucReserved3	: 1;
	UCHAR		ucReserved4	: 1;
} BT868_REG_D6;

typedef struct tagBT868_REG_D8
{
	UCHAR		ucReserved	: 8;
} BT868_REG_D8;

typedef struct
{
	BT868_REG_76	reg76;
	BT868_REG_78	reg78;
	BT868_REG_7A	reg7A;
	BT868_REG_7C	reg7C;
	BT868_REG_7E	reg7E;
	BT868_REG_80	reg80;
	BT868_REG_82	reg82;
	BT868_REG_84	reg84;
	BT868_REG_86	reg86;
	BT868_REG_88	reg88;
	BT868_REG_8A	reg8A;
	BT868_REG_8C	reg8C;
	BT868_REG_8E	reg8E;
	BT868_REG_90	reg90;
	BT868_REG_92	reg92;
	BT868_REG_94	reg94;
	BT868_REG_96	reg96;
	BT868_REG_98	reg98;
	BT868_REG_9A	reg9A;
	BT868_REG_9C	reg9C;
	BT868_REG_9E	reg9E;
	BT868_REG_A0	regA0;
	BT868_REG_A2	regA2;
	BT868_REG_A4	regA4;
	BT868_REG_A6	regA6;
	BT868_REG_A8	regA8;
	BT868_REG_AA	regAA;
	BT868_REG_AC	regAC;
	BT868_REG_AE	regAE;
	BT868_REG_B0	regB0;
	BT868_REG_B2	regB2;
	BT868_REG_B4	regB4;
	BT868_REG_B6	regB6;
	BT868_REG_B8	regB8;
	BT868_REG_BA	regBA;
	BT868_REG_BC	regBC;
	BT868_REG_BE	regBE;
	BT868_REG_C0	regC0;
	BT868_REG_C2	regC2;
	BT868_REG_C4	regC4;
	BT868_REG_C6	regC6;
	BT868_REG_C8	regC8;
	BT868_REG_CA	regCA;
	BT868_REG_CC	regCC;
	BT868_REG_CE	regCE;
	BT868_REG_D0	regD0;
	BT868_REG_D2	regD2;
	BT868_REG_D4	regD4;
	BT868_REG_D6	regD6;
	BT868_REG_D8	regD8;
} BT868_Regs;

#endif
