/* -*-c++-*- */
/* $Header: i2c.c, 2, 10/11/00 8:58:21 PM, Brent$ */
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
** File name:   i2c.c
**
** Description: i2c functions.
**
** $Revision: 2$ 
** $Date: 10/11/00 8:58:21 PM$
** 
** $History: i2c.c $
** 
** *****************  Version 1  *****************
** User: Doconnell    Date: 9/03/99    Time: 11:04a
** Created in $/devel/h5/WinNT/Src/Video/Miniport/h5
** Add TVOut support.
** 
** *****************  Version 4  *****************
** User: Doconnell    Date: 8/13/99    Time: 4:09p
** Updated in $/Releases/Voodoo3/V3_RT31/3dfx/devel/H3/WINNT/SRC/Video/Miniport/Voodoo3
** PRS 7926.  Port multiple TVOut changes from Win9X to WinNT4.  The most
** important change involves correctly storing data in EEPROM to share
** with TVOut configuration data with BIOS.  This involved changine I2C
** code to run with a slow EEPROM part.
** 
** *****************  Version 2  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 12:12p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** Clean up include of 3dfx.h
** 
** *****************  Version 1  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 10:06a
** Created in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** 
** *****************  Version 13  *****************
** User: Pratt        Date: 3/31/99    Time: 11:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** commented out debug print statments in I2CInit function
** 
** *****************  Version 12  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/08/99    Time: 4:24p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 10    12/15/98 2:13p Stuartb
** In I2C init set avenger TVOUT_RST_BIT high by default.  This won't
** affect Banshee.
** 
** 9     12/09/98 5:27p Andrew
** changed stop to back the way it was
** 
** 8     12/08/98 5:52p Andrew
** Possible fix to DDC weirdness -- changed stop to how I had it
** originally.
** 
** 7     10/08/98 4:27p Andrew
** Fixed a problem with a counter that was not being incremented.
** Believed to be the DDC monitor and switch box problem
** 
** 6     9/16/98 11:05a Stuartb
** Fixed bug that left SCL low after register write.
** 
** 5     9/10/98 2:45p Stuartb
** Added PLL and sage LCD write routines.
** 
** 4     6/23/98 10:16a Stuartb
** Added I2C multibyte writes.
** 
** 3     5/05/98 1:20p Stuartb
** Adding i2c extEscape functionality.
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:30p Andrew
** Low Level I2C routines
** 
*/

#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "3dfx.h"

#define VDDONLY
#include "tv.h"
//#include "devtable.h"
#undef  VDDONLY
//#include "tvoutdef.h"
//#include "bt868.h"
//#include "sagelcd.h"
#include "i2c.h"
#include "time.h"

/********************************************************************************
*
* The file i2c.c implements the low level I2C routines.
* void scl(PI2CMASK pI2CMask, DWORD bit);
* void sda(PI2CMASK pI2CMask, DWORD bit);
* int read_scl(PI2CMASK pI2CMask);
* void start(PI2CMASK pI2CMask);
* void stop(PI2CMASK pI2CMask);
* int send_byte(PI2CMASK pI2CMask, BYTE data);
* BYTE read_byte(PI2CMASK pI2CMask, DWORD ack);
*
* int ReadI2CRegister(PI2CMASK pI2CMask);
* int WriteI2CRegister(PI2CMASK pI2cMask);
* void I2CInit(PI2CMASK pI2CMask);
*
*********************************************************************************/

const struct i2cmask I2C_PROTO = {0, 0, 0, 1, 0, I2C_ENABLE, I2C_SCL_OUT_BIT, \
						I2C_SDA_OUT_BIT, I2C_SCL_IN_BIT, I2C_SDA_IN_BIT};
const ULONG I2C_INITVAL = (ULONG) (1 << I2C_ENABLE) | (1 << I2C_SCL_OUT_BIT) | (1 << I2C_SDA_OUT_BIT) | (1 << TVOUT_RST_BIT);

#define GETI2CDW(pReg) (*pReg)
#define SETI2CDW(pReg, dwValue) (*pReg = dwValue)


/*----------------------------------------------------------------------
Function name:  scl

Description:    Sets the Clock to bit
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void scl(PI2CMASK pI2CMask, ULONG bit, UCHAR delaymult)
{
	ULONG dwData;
	int nCount = 0;

	if (delaymult == 0)
      delaymult = 1;

	VideoPortStallExecution(DTIME*delaymult);

	dwData = GETI2CDW(pI2CMask->pReg);
	dwData = (dwData & ~((ULONG)1 << pI2CMask->bSCLOutBit)) | (bit << pI2CMask->bSCLOutBit);
	SETI2CDW (pI2CMask->pReg, dwData);

	VideoPortStallExecution(DTIME*delaymult);

	if (bit)
		while (!read_scl(pI2CMask) && (nCount++ < CLOCK_STRETCH))
			VideoPortStallExecution(DTIME*delaymult);

	if ((bit) && (!read_scl(pI2CMask)))
		VideoDebugPrint((1,"Slave did not respond %s %d\n", __FILE__, __LINE__));
}


/*----------------------------------------------------------------------
Function name:  sda

Description:    Sets the Data to bit.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void sda(PI2CMASK pI2CMask, ULONG bit, UCHAR delaymult)
{
	ULONG	dwData;

	if (delaymult == 0)
      delaymult = 1;

	VideoPortStallExecution(DTIME*delaymult);
		
   if (bit)
      bit = ((ULONG)1 << pI2CMask->bSDAOutBit);
	else
      bit = 0x0;

	dwData = (GETI2CDW(pI2CMask->pReg)) & ~((ULONG)1 << pI2CMask->bSDAOutBit) | bit;
	SETI2CDW (pI2CMask->pReg, dwData);
	VideoPortStallExecution(DTIME*delaymult);
}


/*----------------------------------------------------------------------
Function name:  read_scl

Description:    Reads the Clock
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int read_scl(PI2CMASK pI2CMask)
{
	return (GETI2CDW(pI2CMask->pReg) >> pI2CMask->bSCLInBit) & 0x01;
}


/*----------------------------------------------------------------------
Function name:  read_sda

Description:    Reads the Data line
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int read_sda(PI2CMASK pI2CMask)
{
	return (GETI2CDW(pI2CMask->pReg) >> pI2CMask->bSDAInBit) & 0x1;
}


/*----------------------------------------------------------------------
Function name:  start

Description:    Sends a start
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void start(PI2CMASK pI2CMask, UCHAR delaymult)
{
	if (delaymult == 0)
      delaymult = 1;

	sda(pI2CMask, 1, delaymult);
	scl(pI2CMask, 1, delaymult);
	sda(pI2CMask, 0, delaymult);
	scl(pI2CMask, 0, delaymult);
}


/*----------------------------------------------------------------------
Function name:  stop

Description:    Sends a stop
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void stop(PI2CMASK pI2CMask, UCHAR delaymult)
{
	if (delaymult == 0)
      delaymult = 1;

	scl(pI2CMask, 0, delaymult);
	sda(pI2CMask, 0, delaymult);
	scl(pI2CMask, 1, delaymult);
	sda(pI2CMask, 1, delaymult);
}


/*----------------------------------------------------------------------
Function name:  send_byte

Description:    Sends the byte data
                
Information:

Return:         INT  1 if failure
                     0 if sucess
----------------------------------------------------------------------*/
int send_byte(PI2CMASK pI2CMask, UCHAR data, UCHAR delaymult)
{
	int	i;
	int nReturn=0;

	if (delaymult == 0)
      delaymult = 1;

	for (i = 0; i < 8; i++)
	{
	  sda(pI2CMask, (data << i) & 0x80, delaymult);
	  scl(pI2CMask, 1, delaymult);
	  if (read_scl(pI2CMask) == 0)
	  {
	     VideoDebugPrint((1,"nc\n"));
	  }
	  scl(pI2CMask, 0, delaymult);
	}
	// Check for ACK
	sda(pI2CMask, 1, delaymult);
	scl(pI2CMask, 1, delaymult);
	if (read_sda(pI2CMask) == 1)
	{
		VideoPortStallExecution(2*DTIME*delaymult);
		nReturn = 1;
	}
	else
	{
		VideoPortStallExecution(2*MSEC*delaymult);    //Delay 2msec
		nReturn = 0;
	}

	scl(pI2CMask, 0, delaymult);
	return nReturn;
}


/*----------------------------------------------------------------------
Function name:  read_byte

Description:    Reads a byte and sends a ack if ack=1.
                
Information:

Return:         UCHAR
----------------------------------------------------------------------*/
UCHAR read_byte(PI2CMASK pI2CMask, ULONG ack, UCHAR delaymult)
{
	int	i, data = 0;

	if (delaymult == 0)
      delaymult = 1;

	for (i = 0; i < 8; i++)
	{
	  scl(pI2CMask, 1, delaymult);

	  if (read_scl(pI2CMask) == 0)
			VideoDebugPrint((1,"Slave not ready for read %s %d\n", __FILE__, __LINE__));

	  data <<= 1;
	  data |= read_sda(pI2CMask);

	  scl(pI2CMask, 0, delaymult);
	}
	if (ack)
	  sda(pI2CMask, 0, delaymult);
	else
	  sda(pI2CMask, 1, delaymult);

	scl(pI2CMask, 1, delaymult);
	scl(pI2CMask, 0, delaymult);
	sda(pI2CMask, 1, delaymult);

	VideoPortStallExecution(2*MSEC*delaymult);    //Delay 2msec

	return (UCHAR)(data);
}


/*----------------------------------------------------------------------
Function name:  ReadI2CRegister

Description:    Reads a I2C register
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int ReadI2CRegister(PI2CMASK pI2CMask, UCHAR delaymult)
{
	int nReturn = 0;

	if (delaymult == 0)
      delaymult = 1;

	start(pI2CMask, delaymult);
 	nReturn |= send_byte(pI2CMask, (UCHAR)(pI2CMask->bAddr|MASTER_WRITE), delaymult);
	nReturn |= send_byte(pI2CMask, (UCHAR)(pI2CMask->nReg), delaymult);
	start(pI2CMask, delaymult);
	nReturn |= send_byte(pI2CMask, (UCHAR)(pI2CMask->bAddr|MASTER_READ), delaymult);
	pI2CMask->dwData = read_byte(pI2CMask, 0x0, delaymult);
	stop(pI2CMask, delaymult);

    pI2CMask->nSize = 1;
	
	VideoDebugPrint((1,"Register %02x is %02x\n", pI2CMask->nReg, pI2CMask->dwData));
	
	return (!nReturn ? FXTRUE : FXFALSE);
}


/*----------------------------------------------------------------------
Function name:  WriteI2CRegister  

Description:    Writes a value dwData to a register nReg
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegister(PI2CMASK pI2CMask, UCHAR delaymult)
{
	int nCount;
	int nSuccess = FXFALSE;

	if (delaymult == 0)
      delaymult = 1;
   
	VideoDebugPrint((1,"Register %02x set %02x\n", pI2CMask->nReg, pI2CMask->dwData));
   

	for (nCount=0; ((nCount < NUM_RETRY) && (nSuccess == 0)) ; nCount++)
	{
		start(pI2CMask, delaymult);
		if (!send_byte(pI2CMask, (UCHAR)(pI2CMask->bAddr|MASTER_WRITE), delaymult))
		{
			if (!send_byte(pI2CMask, (UCHAR)pI2CMask->nReg, delaymult))
			{
				if (!send_byte(pI2CMask, (UCHAR)(pI2CMask->dwData & 0xFF), delaymult))
					nSuccess = FXTRUE;
				else
					VideoDebugPrint((1,"send_byte fail %s %d\n", __FILE__, __LINE__));
			}
			else
					VideoDebugPrint((1,"send_byte fail %s %d\n", __FILE__, __LINE__));
		}
		else
			VideoDebugPrint((1,"send_byte failed %s %d\n", __FILE__, __LINE__));

		stop(pI2CMask, delaymult);
	}

	return (nSuccess);
}


/*----------------------------------------------------------------------
Function name:  WriteI2CRegisterMulti

Description:    Writes multiple values to a register 
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegisterMulti(PI2CMASK pI2CMask, UCHAR delaymult)
{
	I2CMASK i2c;
	int retries = NUM_RETRY;

	if (delaymult == 0)
      delaymult = 1;

m_retry:
	if (!retries)
		return (FXFALSE);
	else if (retries != NUM_RETRY)  // not first time through
		stop (pI2CMask, delaymult);

	retries--;
	i2c = *pI2CMask;
	start (pI2CMask, delaymult);
	if (send_byte (pI2CMask, (UCHAR)(pI2CMask->bAddr|MASTER_WRITE), delaymult))
		goto m_retry;
	if (send_byte(pI2CMask, (UCHAR)pI2CMask->nReg, delaymult))
		goto m_retry;
	while (i2c.nSize--)
	{
		if (send_byte(pI2CMask, (UCHAR)(*i2c.multiWriteData++), delaymult))
			goto m_retry;
	}
	stop (pI2CMask, delaymult);
	return (FXTRUE);
}


/*----------------------------------------------------------------------
Function name:  I2CInit

Description:    Used to init a I2C Port
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void I2CInit(PI2CMASK pI2CMask)
{
   VideoDebugPrint((1,"%08lx %08lx\n", pI2CMask->pReg, *pI2CMask->pReg));
   *pI2CMask->pReg |= (1 << pI2CMask->bEnableBit);
   VideoDebugPrint((1,"%08lx %08lx\n", pI2CMask->pReg, *pI2CMask->pReg));
}


/////////////////////////////////////////////////////////////////////////////////
//
//SECTION FOR 9161PLL SERIAL PROGRAMMING
//
////////////////////////////////////////////////////////////////////

void unlockPLL(PI2CMASK pI2CMask);
int send_dataPLL(PI2CMASK pI2CMask, ULONG data, ULONG reg);


/*----------------------------------------------------------------------
Function name:  unlockPLL

Description:    Unlock sequence
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void unlockPLL(PI2CMASK pI2CMask)
{
	sda(pI2CMask, 0, 0); //Reset data
	scl(pI2CMask, 0, 0); //Reset clk
   
	sda(pI2CMask, 1, 0); //Set data high, toggle clk 5 times
	scl(pI2CMask, 1, 0); //1
	scl(pI2CMask, 0, 0);
	scl(pI2CMask, 1, 0); //2
	scl(pI2CMask, 0, 0);
	scl(pI2CMask, 1, 0); //3
	scl(pI2CMask, 0, 0);
	scl(pI2CMask, 1, 0); //4
	scl(pI2CMask, 0, 0);
	scl(pI2CMask, 1, 0); //5
	scl(pI2CMask, 0, 0);
   
	sda(pI2CMask, 0, 0); //Set data low
	scl(pI2CMask, 1, 0); //Clk high
}


/*----------------------------------------------------------------------
Function name:  send_dataPLL

Description:    Sends the byte data
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int send_dataPLL(PI2CMASK pI2CMask, ULONG data, ULONG reg)
{
	int	i;
	int nReturn=0;

   //Send START
	sda(pI2CMask, 0, 0); //Data low
   //scl(pI2CMask, 1); //Toggle clk
	scl(pI2CMask, 0, 0);
   scl(pI2CMask, 1, 0);
   
   
   //Send 21 data bits:
	for (i = 0; i < 21; i++)
	{
	  sda(pI2CMask, !((data >> i) & 0x01), 0 );
	  scl(pI2CMask, 0, 0);
	  sda(pI2CMask, (data >> i) & 0x01, 0);
	  scl(pI2CMask, 1, 0);
	  if (read_scl(pI2CMask) == 0)
	  {
	     VideoDebugPrint((1,"nc\n"));
	  }
	}
   
   //Send 3 address bits:
	for (i = 0; i < 3; i++)
	{
	  sda(pI2CMask, !((reg >> i) & 0x01), 0 );
	  scl(pI2CMask, 0, 0);
	  sda(pI2CMask, (reg >> i) & 0x01, 0);
	  scl(pI2CMask, 1, 0);
	  if (read_scl(pI2CMask) == 0)
	  {
	     VideoDebugPrint((1,"nc\n"));
	  }
	}
   
   //Send STOP
	sda(pI2CMask, 1, 0); //Data high
	scl(pI2CMask, 0, 0);
	scl(pI2CMask, 1, 0);
   
	VideoPortStallExecution(2*MSEC);    //Delay 2msec
   
	return nReturn;
}


/*----------------------------------------------------------------------
Function name:  WritePLLReg

Description:    Writes a value dwData to a register nReg
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void WritePLLReg(PI2CMASK pI2CMask)
{
	//int nSuccess = FXTRUE;
   
   VideoDebugPrint((1,"Register %02x set %02x\n", pI2CMask->nReg, pI2CMask->dwData));
   

	//for (nCount=0; ((nCount < NUM_RETRY) && (nSuccess == 0)) ; nCount++)
	//{
		unlockPLL(pI2CMask);
      
	   if ( send_dataPLL(pI2CMask, (pI2CMask->dwData & 0x1FFFFF), (ULONG)(pI2CMask->nReg & 0x07)) )
		{
         //nSuccess = FXFALSE;
         VideoDebugPrint((1,"send_byte fail %s %d\n", __FILE__, __LINE__));
		}

	//}
}


/*----------------------------------------------------------------------
Function name:  SelectPLLReg

Description:    Select register for output
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void SelectPLLReg(PI2CMASK pI2CMask)
{
   switch ( pI2CMask->nReg )
   {
      case 0:
	      scl(pI2CMask, 0, 0);
	      sda(pI2CMask, 0, 0);
         break;
      case 1:
	      scl(pI2CMask, 1, 0);
	      sda(pI2CMask, 0, 0);
         break;
      case 2:
	      scl(pI2CMask, 0, 0);
	      sda(pI2CMask, 1, 0);
         break;
      case 3:
	      scl(pI2CMask, 1, 0);
	      sda(pI2CMask, 1, 0);
         break;
   }
}
