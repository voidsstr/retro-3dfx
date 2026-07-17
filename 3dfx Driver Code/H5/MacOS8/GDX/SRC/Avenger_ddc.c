/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   avenger_ddc.c
**
**
*/

#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#include "avenger_ddc.h"
#include "gdx_debug.h"

#include <DriverServices.h>






#define I2COUT_PORT  0x78

#define MASTER_WRITE (0x00)
#define MASTER_READ (0x01)

#define SCL_BIT_ON (0x01)
#define SCL_BIT_OFF (0x00)

/*
#define I2C_ENABLE SST_SERPAR_I2C_EN
#define I2C_SCL_OUT_BIT SST_SERPAR_I2C_SCK_OUT
#define I2C_SDA_OUT_BIT SST_SERPAR_I2C_DSA_OUT
#define I2C_SCL_IN_BIT SST_SERPAR_I2C_SCK_IN
#define I2C_SDA_IN_BIT SST_SERPAR_I2C_DSA_IN
*/


#define USEC (1000)
#define DTIME (10*USEC)
#define MSEC (1000000)
#define D60HZ (166*MSEC)
#define D2MS (2*MSEC)

void DELAY(unsigned long x);

#define CLOCK_STRETCH (1000)
#define NUM_RETRY (2)

typedef struct i2cmask {
   volatile FxU32 *pReg;         /* Linear Address of the I2C Register */
   FxU32 dwData;                 /* Data to ship out only **bytes** supported */
   FxU32 port;                   /* DDC port */
   FxU16 nReg;                   /* I2C Device Register */
   FxU16 nSize;                  /* Size of Transaction only 1 supported */
   FxU8  bAddr;                  /* Device Addr  {7 Bit address << 1} */
   FxU32  bEnableBit;             /* Enable bit for I2C in register */
   FxU32  bSCLOutBit;             /* SCL Out Bit */
   FxU32  bSDAOutBit;             /* SDA Out Bit */
   FxU32  bSCLInBit;              /* SCL In Bit */
   FxU32  bSDAInBit;              /* SDA In Bit */
   FxU32  *multiWriteData;        /* ptr to data for multi-write, optional */
} I2CMASK, * PI2CMASK;



#define GETI2CDW(pReg) (__eieio(),__lwbrx((void *)pReg,0))
#define SETI2CDW(pReg, dwValue) (__eieio(),__stwbrx(dwValue,(void *)pReg,0))





void scl(PI2CMASK pI2CMask, DWORD bit);
void sda(PI2CMASK pI2CMask, DWORD bit);
int read_scl(PI2CMASK pI2CMask);
int read_sda(PI2CMASK pI2CMask);
void start(PI2CMASK pI2CMask);
void stop(PI2CMASK pI2CMask);
int send_byte(PI2CMASK pI2CMask, BYTE data);
BYTE read_byte(PI2CMASK pI2CMask, DWORD ack);

int ReadI2CRegister(PI2CMASK pI2CMask);
int WriteI2CRegister(PI2CMASK pI2cMask);
int WriteI2CRegisterMulti(PI2CMASK pI2cMask);
void I2CInit(PI2CMASK pI2CMask);

int read_sda(PI2CMASK pI2CMask);
int DoWakeUp(PI2CMASK pI2CMask);
int DDCCommand(PI2CMASK pI2CMask);







/*----------------------------------------------------------------------
Function name:  DELAY

Description:    waits for x ns.

Return:         -
----------------------------------------------------------------------*/
void DELAY(unsigned long x)
{
  union {
    unsigned long long value;
    UnsignedWide ns;
  } time;

  time.value = x;
  DelayForHardware(NanosecondsToAbsolute(time.ns));
}





/*----------------------------------------------------------------------
Function name:  scl

Description:    Sets the Clock to bit

Information:

Return:         VOID
----------------------------------------------------------------------*/
void scl(PI2CMASK pI2CMask, DWORD bit)
{
#define FN_NAME "scl"
#define FN_LEVEL 80
  DWORD dwData;
  int nCount = 0;

  LOG_ENTRY(FN_LEVEL);

  DELAY(DTIME);

  dwData = GETI2CDW(pI2CMask->pReg);
  if ( bit )
    dwData |= pI2CMask->bSCLOutBit;  
  else
    dwData &= ~pI2CMask->bSCLOutBit;
  SETI2CDW (pI2CMask->pReg, dwData);
  LOG_PRINTF1( FN_LEVEL, "reading reg : 0x%08x\n", dwData);

  DELAY(DTIME);

  if (bit)
  {
    while (!read_scl(pI2CMask) && (nCount++ < CLOCK_STRETCH)) {
      DELAY(DTIME);
    }
  }

#if DEBUG  
  if ((bit) && (!read_scl(pI2CMask)))
  {
    LOG_PRINTF2( FN_LEVEL, "Slave did not respond %s %d\n", __FILE__, __LINE__);
  }
#endif

  LOG_EXIT(FN_LEVEL,0);
#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  sda

Description:    Sets the Data to bit.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void sda(PI2CMASK pI2CMask, DWORD bit)
{
  DWORD	dwData;

  DELAY(DTIME);

  dwData = GETI2CDW(pI2CMask->pReg);
  if ( bit )
    dwData |= pI2CMask->bSDAOutBit;  
  else
    dwData &= ~pI2CMask->bSDAOutBit;
  SETI2CDW (pI2CMask->pReg, dwData);

  DELAY(DTIME);
}





/*----------------------------------------------------------------------
Function name:  read_scl

Description:    Reads the Clock

Information:

Return:         INT
----------------------------------------------------------------------*/
int read_scl(PI2CMASK pI2CMask)
{
  return ( (GETI2CDW(pI2CMask->pReg) & pI2CMask->bSCLInBit) ? 1:0 );
}





/*----------------------------------------------------------------------
Function name:  read_sda

Description:    Reads the Data line

Information:

Return:         INT
----------------------------------------------------------------------*/
int read_sda(PI2CMASK pI2CMask)
{
  return ( (GETI2CDW(pI2CMask->pReg) & pI2CMask->bSDAInBit) ? 1:0 );
}





/*----------------------------------------------------------------------
Function name:  start

Description:    Sends a start

Information:

Return:         VOID
----------------------------------------------------------------------*/
void start(PI2CMASK pI2CMask)
{
  sda(pI2CMask, 1);
  scl(pI2CMask, 1);
  sda(pI2CMask, 0);
  scl(pI2CMask, 0);
}





/*----------------------------------------------------------------------
Function name:  stop

Description:    Sends a stop

Information:

Return:         VOID
----------------------------------------------------------------------*/
void stop(PI2CMASK pI2CMask)
{
  scl(pI2CMask, 0);
  sda(pI2CMask, 0);
  scl(pI2CMask, 1);
  sda(pI2CMask, 1);
}





/*----------------------------------------------------------------------
Function name:  send_byte

Description:    Sends the byte data

Information:

Return:         INT
----------------------------------------------------------------------*/
int send_byte(PI2CMASK pI2CMask, BYTE data)
{
#define FN_NAME "send_byte"
#define FN_LEVEL 0
  int	i;
  int   nReturn = FXTRUE;

  LOG_ENTRY(FN_LEVEL);

  for (i = 0; i < 8 && nReturn; i++)
  {
    sda(pI2CMask, (data << i) & 0x80);
    scl(pI2CMask, 1);
    if (read_scl(pI2CMask) == 0)
    {
      nReturn = FXFALSE;
      LOG_PRINTF1( FN_LEVEL,"clock line is not coming up @ line = %d\n", __LINE__);
    }

    scl(pI2CMask, 0);
    if (read_scl(pI2CMask) == 1 )
    {
      nReturn = FXFALSE;
      LOG_PRINTF1( FN_LEVEL,"clock line is not coming down @ line = %d\n", __LINE__);
    }
  }

  /* Check for ACK */
  sda(pI2CMask, 1);
  scl(pI2CMask, 1);
  if (read_sda(pI2CMask) == 1)
    nReturn = FXTRUE;
  else
    nReturn = FXFALSE;

  if (nReturn == FXTRUE)
  {
    DELAY(DTIME);
  }

ErrorExit:

  scl(pI2CMask, 0);

  LOG_EXIT(FN_LEVEL,0);
  return nReturn;

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  read_byte

Description:    Reads a byte and sends a ack if ack=1.

Information:

Return:         BYTE
----------------------------------------------------------------------*/
BYTE read_byte(PI2CMASK pI2CMask, DWORD ack)
{
#define FN_NAME "read_byte"
#define FN_LEVEL 0
  int	i, data = 0;
  int   nReturn = FXTRUE;

  LOG_ENTRY(FN_LEVEL);

  for (i = 0; i < 8 && nReturn; i++)
  {
    scl(pI2CMask, 1);
    if (read_scl(pI2CMask) == 0)
    {
      nReturn = FXFALSE;
      LOG_PRINTF1( FN_LEVEL,"clock line is not coming up @ line = %d\n", __LINE__);
    }

    data <<= 1;
    data |= read_sda(pI2CMask);

    scl(pI2CMask, 0);
    if (read_scl(pI2CMask) == 1 )
    {
      nReturn = FXFALSE;
      LOG_PRINTF1( FN_LEVEL,"clock line is not coming down @ line = %d\n", __LINE__);
    }
  }

  if (ack)
    sda(pI2CMask, 0);
  else
    sda(pI2CMask, 1);

  scl(pI2CMask, 1);
  scl(pI2CMask, 0);
  sda(pI2CMask, 1);

  LOG_EXIT(FN_LEVEL,0);
  return (BYTE)(data);

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  ReadI2CRegister

Description:    Reads a I2C register

Information:

Return:         INT
----------------------------------------------------------------------*/
int ReadI2CRegister(PI2CMASK pI2CMask)
{
#define FN_NAME "ReadI2CRegister"
#define FN_LEVEL 80
  int nReturn = 0;

  LOG_ENTRY(FN_LEVEL);

  start(pI2CMask);
  nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE));
  nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->nReg));
  start(pI2CMask);
  nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_READ));
  pI2CMask->dwData = read_byte(pI2CMask, 0x0);
  stop(pI2CMask);

  pI2CMask->nSize = 1;
  LOG_PRINTF2(FN_LEVEL, "Register %02x is %02x\n", pI2CMask->nReg, pI2CMask->dwData);

  LOG_EXIT(FN_LEVEL,0);
  return (!nReturn ? FXTRUE : FXFALSE);

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  WriteI2CRegister

Description:    Writes a value dwData to a register nReg

Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegister(PI2CMASK pI2CMask)
{
#define FN_NAME "ReadI2CRegister"
#define FN_LEVEL 80
  int nCount;
  int nSuccess = FXFALSE;

  LOG_ENTRY(FN_LEVEL);
  LOG_PRINTF2(FN_LEVEL, "Register %02x set %02x\n", pI2CMask->nReg, pI2CMask->dwData);

	for (nCount=0; ((nCount < NUM_RETRY) && (nSuccess == 0)) ; nCount++)
	{
		start(pI2CMask);
		if (!send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
		{
			if (!send_byte(pI2CMask, (BYTE)pI2CMask->nReg))
			{
				if (!send_byte(pI2CMask, (BYTE)(pI2CMask->dwData & 0xFF)))
					nSuccess = FXTRUE;
				else
					LOG_PRINTF2(1,"send_byte fail %s %d\n", __FILE__, __LINE__);
			}
			else
					LOG_PRINTF2(1,"send_byte fail %s %d\n", __FILE__, __LINE__);
		}
		else
			LOG_PRINTF2(1,"send_byte failed %s %d\n", __FILE__, __LINE__);

		stop(pI2CMask);
	}

  LOG_EXIT(FN_LEVEL,nSuccess);
  return (nSuccess);

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  WriteI2CRegisterMulti

Description:    Writes multiple values to a register

Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegisterMulti(PI2CMASK pI2CMask)
{
	I2CMASK i2c;
	int retries = NUM_RETRY;

m_retry:
	if (!retries)
		return (FXFALSE);
  else if (retries != NUM_RETRY)  /* not first time through */
		stop (pI2CMask);

	retries--;
	i2c = *pI2CMask;
	start (pI2CMask);
	if (send_byte (pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
		goto m_retry;
	if (send_byte(pI2CMask, (BYTE)pI2CMask->nReg))
		goto m_retry;
	while (i2c.nSize--)
	{
		if (send_byte(pI2CMask, (BYTE)(*i2c.multiWriteData++)))
			goto m_retry;
	}
	stop (pI2CMask);
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
#define FN_NAME "I2CInit"
#define FN_LEVEL 20
  FxU32 reg;

  LOG_ENTRY(FN_LEVEL);

  __eieio();
  reg = __lwbrx((void *)pI2CMask->pReg,0);
  reg |= pI2CMask->bEnableBit;
#if H5
  if ( pI2CMask->port == k3DfxDDCVGA )
    reg &= ~SST_SERPAR_GPIO_1;     /* H5: DDC on VGA */
  else
    reg |= SST_SERPAR_GPIO_1;      /* H5: DDC on DVI */
#else
  if ( pI2CMask->port == k3DfxDDCDVI )
    reg |= SST_SERPAR_GPIO_1;      /* H5: DDC on DVI */
  else
    reg &= ~SST_SERPAR_GPIO_1;     /* H5: DDC on VGA */
#endif
  __eieio();
  __stwbrx(reg,(void *)pI2CMask->pReg,0);
  __eieio();
  reg = __lwbrx((void *)pI2CMask->pReg,0);
  LOG_PRINTF2(FN_LEVEL, "reg %08lx = %08lx\n", pI2CMask->pReg, reg);
  /*CycleTime(); */


  LOG_EXIT(FN_LEVEL,0);

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  DDCCommand

Description:    Routine to check if this is a DDC Monitor.

Information:

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DDCCommand(PI2CMASK pI2CMask)
{
#define FN_NAME "DDCCommand"
#define FN_LEVEL 0
  int nReturn = 0;

  LOG_ENTRY(FN_LEVEL);

  start(pI2CMask);
  if (!send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
  {
    if (!send_byte(pI2CMask, 0x0))
    {
         /* Does not work on Vivitron ? */
#if 1
      stop(pI2CMask);
#endif
      start(pI2CMask);
      send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_READ));
      sda(pI2CMask, 1);
      nReturn = 1;
    }
    else
    {
      stop(pI2CMask);
      LOG_PRINTF1(FN_LEVEL, "failed to send %x\n", 0x0);
    }
  }
  else
  {
    stop(pI2CMask);
    LOG_PRINTF1(FN_LEVEL, "failed to send %x\n", pI2CMask->bAddr|MASTER_WRITE);
  }

  LOG_EXIT(FN_LEVEL,nReturn);
  return nReturn;

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  DoWakeUp

Description:    Routine to query the monitor to see if it is alive.

Information:

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DoWakeUp(PI2CMASK pI2CMask)
{
#define FN_NAME "DoWakeUp"
#define FN_LEVEL 0
  int i;
  int nReturn = 0;
  BYTE bCH;

  LOG_ENTRY(FN_LEVEL);

  /* Possible Switch Box Fix? */
  sda(pI2CMask, 1);
  scl(pI2CMask, 1);
  /* 166 MSEC is approx 60 Hz */
  for (i=0; i<166; i++)
  {
    if ((1 == read_scl(pI2CMask)) && (1 == read_sda(pI2CMask)))
      break;
    DELAY(MSEC);
  }

  LOG_PRINTF2(FN_LEVEL, "scl: %d  sda: %d\n",read_scl(pI2CMask),read_sda(pI2CMask));

  if ((1 != read_scl(pI2CMask)) && (1 != read_sda(pI2CMask)))
    return nReturn;

  if (DDCCommand(pI2CMask))
  {
    bCH = read_byte(pI2CMask, 0x01);
    if (0x00 == bCH)
    {
      bCH = read_byte(pI2CMask, 0x00);
      if (0xFF == bCH)
        nReturn = 1;
      else
      {
        stop(pI2CMask);
        LOG_PRINTF1(FN_LEVEL, "Read Failed expected 0xFF got %02x\n", bCH);
      }
    }
    else
    {
      stop(pI2CMask);
      LOG_PRINTF1(FN_LEVEL, "Read Failed expected 0x00 got %02x\n", bCH);
    }
  }
  else
    LOG_PRINTF(FN_LEVEL, "DDC Command Failed\n");

  LOG_EXIT(FN_LEVEL,nReturn);
  return nReturn;

#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  AvengerReadDDC

Description:    

Information:

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int AvengerReadDDC(FxU32 port, int nAddr, Byte *pBuffer, int nSize)
{
#define FN_NAME "AvengerReadDDC"
#define FN_LEVEL 0
#if 0
  Byte theEDID[] = {
                     0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
                     0x0E, 0x11, 0x11, 0x30, 0x35, 0x35, 0x37, 0x41,
                     0x30, 0x08, 0x01, 0x02, 0x80, 0x1E, 0x17, 0x78,
                     0x2A, 0xA0, 0xFD, 0x9E, 0x56, 0x4C, 0x9C, 0x24,
                     0x19, 0x50, 0x55, 0x00, 0x08, 0x00, 0x01, 0x01, 
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x64, 0x19, 
                     0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 0x88,
                     0x36, 0x00, 0x30, 0xE4, 0x10, 0x00, 0x00, 0x1E, 
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x1D
                   };
  short theCount = 0;
  short theResult = 0;
  
  if ( port == k3DfxDDCDVI )
  {
    while ( theCount < nSize )
      pBuffer[ theCount++ ] = theEDID[ theCount ];
    theResult = 1;
  }
    
  return theResult;
#else
  I2CMASK I2CMask;
  int i;
  int j;
  int nReturn = 0;
  BYTE bChkSum;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(FN_LEVEL);
  LOG_PRINTF1(1, "Read DDC port : %s\n", port == k3DfxDDCVGA ? "VGA" : "DVI");


  I2CMask.pReg = (DWORD *)(avengerHALData->bInfo.regInfo.ioPortBase + I2COUT_PORT);
  I2CMask.bAddr = nAddr;
  I2CMask.port = port;

#if H5  
  I2CMask.bEnableBit = SST_SERPAR_DDC_EN;
  I2CMask.bSCLOutBit = SST_SERPAR_DDC_DCK_OUT;
  I2CMask.bSDAOutBit = SST_SERPAR_DDC_DDA_OUT;
  I2CMask.bSCLInBit = SST_SERPAR_DDC_DCK_IN;
  I2CMask.bSDAInBit = SST_SERPAR_DDC_DDA_IN;
#else
  if ( port == k3DfxDDCDVI || nAddr != 0xA0 )  
  {
    I2CMask.bEnableBit = SST_SERPAR_I2C_EN;
    I2CMask.bSCLOutBit = SST_SERPAR_I2C_SCK_OUT;
    I2CMask.bSDAOutBit = SST_SERPAR_I2C_DSA_OUT;
    I2CMask.bSCLInBit = SST_SERPAR_I2C_SCK_IN;
    I2CMask.bSDAInBit = SST_SERPAR_I2C_DSA_IN;
  }
  else
  {
    I2CMask.bEnableBit = SST_SERPAR_DDC_EN;
    I2CMask.bSCLOutBit = SST_SERPAR_DDC_DCK_OUT;
    I2CMask.bSDAOutBit = SST_SERPAR_DDC_DDA_OUT;
    I2CMask.bSCLInBit = SST_SERPAR_DDC_DCK_IN;
    I2CMask.bSDAInBit = SST_SERPAR_DDC_DDA_IN;
  }
#endif

  I2CInit(&I2CMask);


  for (j=0; j<NUM_RETRY; j++)
  {
    if (DoWakeUp(&I2CMask))
    {
      if (DDCCommand(&I2CMask))
      {
        /* read the first n - 1 bytes */
        for (i=0; i<nSize-1; i++)
          pBuffer[i] = read_byte(&I2CMask, 0x01);
        /* last read must terminate the access with a stop cycle */
        pBuffer[i] = read_byte(&I2CMask, 0x00);
        stop(&I2CMask);

        bChkSum=0;
        for (i=0; i<nSize; i++)
        {
          LOG_PRINTF2(FN_LEVEL, "%d %02x\n", i, pBuffer[i]);
          bChkSum += pBuffer[i];
        }

        if (0 == bChkSum)
        {
          LOG_PRINTF(FN_LEVEL, "DDC block read was successfull\n");
          nReturn = 1;
          break;
        }
        else
        {
          LOG_PRINTF1(FN_LEVEL, "Chksum failure %02x \n", bChkSum);
        }
      }
      else
      {
        LOG_PRINTF(FN_LEVEL, "DDCCommand failed\n");
      }
    }
    else
    {
      LOG_PRINTF(FN_LEVEL, "Wake Up Failed\n");
    }
  }

  LOG_EXIT(FN_LEVEL,nReturn==1?0:1);
  return nReturn;
#endif
#undef FN_NAME
#undef FN_LEVEL
}






Byte GetEDIDVersion( Byte * edid )
{
  return edid[18];
};


Byte GetEDIDRevision( Byte * edid )
{
  return edid[19];
};

void GetEDIDManufacturerName( Byte * edid, long man )
{
  char * table = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";	// A = 1, B = 2, etc...

  char * manufacturer = (char*) &man;
  manufacturer[0] = table[ (edid[8]>>2) & 31 ];
  manufacturer[1] = table[ ((edid[8] & 3)*8) | (edid[9]>>5) ];
  manufacturer[2] = table[ edid[9] & 31 ];
  manufacturer[3] = 0;

};

Byte GetEDIDVideoInputType( Byte * edid )
{
  return edid[20] & 0x80;
}

Byte GetEDIDHorImageSize( Byte * edid )
{
  return edid[21];
}

Byte GetEDIDVerImageSize( Byte * edid )
{
  return edid[22];
}

float GetEDIDGammaValue( Byte * edid )
{
  return ((float) (edid[23] + 100)) / 100;
}

Byte GetEDIDDPMS( Byte * edid )
{
  return edid[24];
}


Byte GetEDIDStdTimingHorPix( Byte * edid, short ID )
{
  return (edid[ 0x26 + ((ID - 1) * 2) ] + 31);
}

Byte GetEDIDStdTimingAspectRatio( Byte * edid, short ID )
{
  return (edid[ 0x26 + ((ID - 1) * 2) + 1 ] >> 6 & 0x3);
}

Byte GetEDIDStdTimingRefresh( Byte * edid, short ID )
{
  return ((edid[ 0x26 + ((ID - 1) * 2) + 1 ] & 0x3F) + 60);
}

short GetEDIDDetailedTimingPixelClock( Byte * edid, short ID )
{
  short freq = edid[ 0x36 + ((ID - 1) * 18) ];
  freq += edid[ 0x36 + ((ID - 1) * 18) + 1 ] * 256;
  
  return ( freq );
}


long GetEDIDDetailedTimingHActive( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 2 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 4 ] >> 4 & 0xF) * 256) );
}

long GetEDIDDetailedTimingHBlank( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 3 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 4 ]  & 0xF) * 256) );
}

long GetEDIDDetailedTimingVActive( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 5 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 7 ] >> 4 & 0xF) * 256) );
}

long GetEDIDDetailedTimingVBlank( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 6 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 7 ]  & 0xF) * 256) );
}

long GetEDIDDetailedTimingHSyncOffset( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 8 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 11 ] >> 6 & 0x3) * 256) );
}

long GetEDIDDetailedTimingHSyncWidth( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 9 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 11 ] >> 4 & 0x3) * 256) );
}

long GetEDIDDetailedTimingVSyncOffset( Byte * edid, short ID )
{
  return ( (edid[ 0x36 + ((ID - 1) * 18) + 10 ] >> 4 & 0xF) + ((edid[ 0x36 + ((ID - 1) * 18) + 11 ] >> 2 & 0x3) * 16) );
}

long GetEDIDDetailedTimingVSyncWidth( Byte * edid, short ID )
{
  return ( (edid[ 0x36 + ((ID - 1) * 18) + 10 ] & 0xF) + ((edid[ 0x36 + ((ID - 1) * 18) + 11 ] & 0x3) * 16) );
}

Byte GetEDIDDetailedTimingHImageSize( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 12 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 14 ] >> 4 & 0xF) * 256) );
}

Byte GetEDIDDetailedTimingVImageSize( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 13 ] + ((edid[ 0x36 + ((ID - 1) * 18) + 14 ] & 0xF) * 256) );
}

Byte GetEDIDDetailedTimingHBorder( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 15 ] );
}

Byte GetEDIDDetailedTimingVBorder( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 16 ] );
}

Byte GetEDIDDetailedTimingFlags( Byte * edid, short ID )
{
  return ( edid[ 0x36 + ((ID - 1) * 18) + 17 ] );
}









/*----------------------------------------------------------------------
Function name:  AvengerReadSiI

Description:    

Information:

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int AvengerReadSiI( int nAddr, Byte * pBuffer, int nSize )
{
#define FN_NAME "AvengerReadSiI"
#define FN_LEVEL 1
  I2CMASK I2CMask;
  int i;
  int j;
  int nReturn = 0;
  BYTE bChkSum;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(FN_LEVEL);
    
  I2CMask.pReg = (DWORD *)(avengerHALData->bInfo.regInfo.ioPortBase + I2COUT_PORT);
  I2CMask.bAddr = nAddr;
  I2CMask.port = 0;
    
  I2CMask.bEnableBit = SST_SERPAR_I2C_EN;
  I2CMask.bSCLOutBit = SST_SERPAR_I2C_SCK_OUT;
  I2CMask.bSDAOutBit = SST_SERPAR_I2C_DSA_OUT;
  I2CMask.bSCLInBit = SST_SERPAR_I2C_SCK_IN;
  I2CMask.bSDAInBit = SST_SERPAR_I2C_DSA_IN;

  I2CInit(&I2CMask);


  for (j=0; j<NUM_RETRY<<1; j++)
  {
    if (DoWakeUp(&I2CMask))
    {
      if (DDCCommand(&I2CMask))
      {
        for (i=0; i<nSize-1; i++)
          pBuffer[i] = read_byte(&I2CMask, 0x01);

        pBuffer[i] = read_byte(&I2CMask, 0x00);
        stop(&I2CMask);
        
      }
      else
      {
        LOG_PRINTF(FN_LEVEL, "DDCCommand failed\n");
      }
    }
    else
    {
      LOG_PRINTF(FN_LEVEL, "Wake Up Failed\n");
    }
  }

  LOG_EXIT(FN_LEVEL,nReturn==1?0:1);
  return nReturn;

#undef FN_NAME
#undef FN_LEVEL
}

