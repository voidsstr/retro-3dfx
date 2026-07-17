/* -*-c++-*- */
/* $Header: i2c.h, 2, 10/11/00 8:58:22 PM, Brent$ */
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
** File name:   i2c.h
**
** Description: i2c structures, macros, etc.
**
** $Revision: 2$ 
** $Date: 10/11/00 8:58:22 PM$
** 
** $History: i2c.h $
** 
** *****************  Version 1  *****************
** User: Doconnell    Date: 9/03/99    Time: 11:02a
** Created in $/devel/h5/WinNT/Src/Video/Miniport/h5
** Add TVOut support.
** 
** *****************  Version 3  *****************
** User: Doconnell    Date: 8/13/99    Time: 4:10p
** Updated in $/Releases/Voodoo3/V3_RT31/3dfx/devel/H3/WINNT/SRC/Video/Miniport/Voodoo3
** PRS 7926.  Port multiple TVOut changes from Win9X to WinNT4.  The most
** important change involves correctly storing data in EEPROM to share
** with TVOut configuration data with BIOS.  This involved changine I2C
** code to run with a slow EEPROM part.
** 
** *****************  Version 1  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 10:07a
** Created in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** 
** *****************  Version 8  *****************
** User: Michael      Date: 1/08/99    Time: 4:24p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 7     12/10/98 2:52p Stuartb
** Added TVOUT_RST_BIT define.
** 
** 6     9/10/98 2:45p Stuartb
** Added PLL and sage LCD write routines.
** 
** 5     6/23/98 10:16a Stuartb
** Added I2C multibyte writes.
** 
** 4     4/29/98 9:09p Andrew
** Removed Schematic_Error from file and added to makefile
** 
** 3     4/28/98 9:31p Andrew
** undef SCHEMATIC_ERROR so that DDC bits are swapped
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:31p Andrew
** Header file for I2c
** 
*/

#ifndef _I2C_H_
#define _I2C_H_

#define I2COUT_PORT  0x78

#define MASTER_WRITE (0x00)
#define MASTER_READ (0x01)

#define SCL_BIT_ON (0x01)
#define SCL_BIT_OFF (0x00)

// Bit defination for vidSerialParallelPort 78h
// 18 DDC Port Enable
// 19 DDC SCK
// 20 DDC SDA
// 21 DDC SCK Read
// 22 DDC SDA Read
// 23 I2C Port Enable
// 24 I2C SCK
// 25 I2C SDA
// 26 I2C SCK Read
// 27 I2C SDA Read

#define DDC_ENABLE (18)

// There is a error in the Schematic where SCL and SDA are swapped
#ifdef SCHEMATIC_ERROR
#define DDC_SCL_OUT_BIT (DDC_ENABLE + 2)
#define DDC_SDA_OUT_BIT (DDC_ENABLE + 1)
#define DDC_SCL_IN_BIT (DDC_ENABLE + 4)
#define DDC_SDA_IN_BIT (DDC_ENABLE + 3)
#else
#define DDC_SCL_OUT_BIT (DDC_ENABLE + 1)
#define DDC_SDA_OUT_BIT (DDC_ENABLE + 2)
#define DDC_SCL_IN_BIT (DDC_ENABLE + 3)
#define DDC_SDA_IN_BIT (DDC_ENABLE + 4)
#endif

#define I2C_ENABLE (23)
#define I2C_SCL_OUT_BIT (I2C_ENABLE + 1)
#define I2C_SDA_OUT_BIT (I2C_ENABLE + 2)
#define I2C_SCL_IN_BIT (I2C_ENABLE + 3)
#define I2C_SDA_IN_BIT (I2C_ENABLE + 4)

#define SST_SERPAR_GPIO_1 29

#define TVOUT_RST_BIT    31   // note, this is H4 only and VMI_MODE_TVOUT must
                              // be selected in vidInFormat!    SMB

#ifdef WINNT
#define USEC (1)
#define MSEC (1000)
#else
#define USEC (1000)
#define MSEC (1000000)
#endif
#define DTIME (5*USEC)
#define D60HZ (166*MSEC)
#define CLOCK_STRETCH (1000)
#define NUM_RETRY (5)

#define I2C_INITMASK ((ULONG) ((1 << I2C_ENABLE)      \
                           | (1 << I2C_SCL_OUT_BIT)   \
                           | (1 << I2C_SDA_OUT_BIT)   \
                           | (1 << SST_SERPAR_GPIO_1)))

typedef struct i2cmask {
   PULONG pReg;                  // Linear Address of the I2C Register
   ULONG dwData;                 // Data to ship out only **bytes** supported
   int  nReg;                   // I2C Device Register
   int  nSize;                  // Size of Transaction only 1 supported
   UCHAR  bAddr;                  // Device Addr  {7 Bit address << 1}
   UCHAR  bEnableBit;             // Enable bit for I2C in register
   UCHAR  bSCLOutBit;             // SCL Out Bit
   UCHAR  bSDAOutBit;             // SDA Out Bit
   UCHAR  bSCLInBit;              // SCL In Bit
   UCHAR  bSDAInBit;              // SDA In Bit
   UCHAR  *multiWriteData;        // ptr to data for multi-write, optional
   } I2CMASK, * PI2CMASK;

/********************************************************************************
*
* The file i2c.c implements the low level I2C routines.
* void scl(PI2CMASK pI2CMask, ULONG bit);
* void sda(PI2CMASK pI2CMask, ULONG bit);
* int read_scl(PI2CMASK pI2CMask);
* void start(PI2CMASK pI2CMask);
* void stop(PI2CMASK pI2CMask);
* int send_byte(PI2CMASK pI2CMask, UCHAR data);
* UCHAR read_byte(PI2CMASK pI2CMask, ULONG ack);
*
* int ReadI2CRegister(PI2CMASK pI2CMask);
* int WriteI2CRegister(PI2CMASK pI2cMask);
* void I2CInit(PI2CMASK pI2CMask);
*
*********************************************************************************/
void scl(PI2CMASK pI2CMask, ULONG bit, UCHAR delaymult);
void sda(PI2CMASK pI2CMask, ULONG bit, UCHAR delaymult);
int read_scl(PI2CMASK pI2CMask);
void start(PI2CMASK pI2CMask, UCHAR delaymult);
void stop(PI2CMASK pI2CMask, UCHAR delaymult);
int send_byte(PI2CMASK pI2CMask, UCHAR data, UCHAR delaymult);
UCHAR read_byte(PI2CMASK pI2CMask, ULONG ack, UCHAR delaymult);

int ReadI2CRegister(PI2CMASK pI2CMask, UCHAR delaymult);
int WriteI2CRegister(PI2CMASK pI2cMask, UCHAR delaymult);
int WriteI2CRegisterMulti(PI2CMASK pI2cMask, UCHAR delaymult);
void I2CInit(PI2CMASK pI2CMask);


//ICS9161 PLL Serial programming
void WritePLLReg(PI2CMASK pI2CMask);
void SelectPLLReg(PI2CMASK pI2CMask);

#endif
