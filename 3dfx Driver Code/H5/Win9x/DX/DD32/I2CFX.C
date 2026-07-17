/* $Header: i2cfx.c, 2, 10/11/00 8:52:12 PM, Brent$ */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File Name: 	I2CFX.C
**
** Description: I2C support functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:52:12 PM$
**
** $History: i2cfx.c $
** 
** *****************  Version 4  *****************
** User: Edwin        Date: 5/30/99    Time: 6:33p
** Updated in $/devel/h3/Win95/dx/dd32
** Remove ifdef MM, multi-monitor is always enabled.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/31/98   Time: 7:35a
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
*/

#include "precomp.h"
#include "ddglobal.h"
#include "header.h"
#include "hw.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "h3g.h"

#include "ddvpe32.h"
#include "i2cfx.h"
#include "i2cxtrn.h"

#define I2CDev MM_DD(I2CDev)

/*----------------------------------------------------------------------
Function name: I2C_Out

Description:   

Return:        int
			   0 
			   1
----------------------------------------------------------------------*/
int I2C_OUT(NT9XDEVICEDATA *ppdev, BYTE type, BYTE data)
{
     FxU32 dwReg;
     int i;

     switch (type) 
     {
		 case SCL:                
            dwReg = GET(ghwIO->vidSerialParallelPort);
            if (data & 1) 
               dwReg |= H3_VMI_I2C_CLOCK_MASK;
            else
               dwReg &= ~H3_VMI_I2C_CLOCK_MASK;
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            
            //Set Video Serial Parallel Port register
            SETDW(ghwIO->vidSerialParallelPort, dwReg); 
            
            //Wait until the new clock state latched on the bus
            for ( ;; )
            {
               dwReg = GET(ghwIO->vidSerialParallelPort);
               if ( data & 1 )
               {
                   if ( dwReg & H3_VMI_I2C_CLOCK_STATE_MASK )
                       break;
               }
               else
               {
                   if ( !(dwReg & H3_VMI_I2C_CLOCK_STATE_MASK) )
                       break;
               }
            }
            break;
			    
		 case SDA:
            dwReg = GET(ghwIO->vidSerialParallelPort);
            if (data & 1)
            	dwReg |= H3_VMI_I2C_DATA_MASK;
            else
            	dwReg &= ~H3_VMI_I2C_DATA_MASK;
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            
            //Set Video Serial Parallel Port register
            SETDW(ghwIO->vidSerialParallelPort, dwReg); 

            //Wait until the new data state latched on the bus
            for ( i=0 ;; )
            {
               dwReg = GET(ghwIO->vidSerialParallelPort);
               if (data & 1)
               {
                   if ( dwReg & H3_VMI_I2C_DATA_STATE_MASK )
                       break;
                   else if ( ++i > 50 )
                       return(1);  //Someone is holding bus low
               }
               else
               {
                   if ( !(dwReg & H3_VMI_I2C_DATA_STATE_MASK) )
                       break;
               }
            }
            break;
     }
     return(0);
}// I2C_Out


/*----------------------------------------------------------------------
Function name: I2C_Rel

Description:   

Return:        NONE
----------------------------------------------------------------------*/
void I2C_REL(NT9XDEVICEDATA * ppdev, BYTE type)
{
     FxU32 dwReg;

     switch (type) 
     {
		 case SCL:                
            dwReg = GET(ghwIO->vidSerialParallelPort);
            dwReg |= H3_VMI_I2C_CLOCK_MASK;
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            SETDW(ghwIO->vidSerialParallelPort, dwReg);
            break;
			    
		 case SDA:
            dwReg = GET(ghwIO->vidSerialParallelPort);
            dwReg |= H3_VMI_I2C_DATA_MASK;
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            SETDW(ghwIO->vidSerialParallelPort, dwReg);
            break;
     }
}// I2C_Rel


/*----------------------------------------------------------------------
Function name: I2C_In

Description:   

Return:        BYTE
----------------------------------------------------------------------*/
BYTE I2C_IN(NT9XDEVICEDATA * ppdev, BYTE type)
{           
    FxU32 dwReg;
  
    switch (type) 
    {
		  case SCL:
            dwReg = GET(ghwIO->vidSerialParallelPort);
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            SETDW(ghwIO->vidSerialParallelPort, dwReg);
            dwReg = GET(ghwIO->vidSerialParallelPort);
            return( (dwReg & H3_VMI_I2C_CLOCK_STATE_MASK) ? 1:0 );

		  case SDA: 
            dwReg = GET(ghwIO->vidSerialParallelPort);
            dwReg |= H3_VMI_I2C_ENABLE_MASK;
            SETDW(ghwIO->vidSerialParallelPort, dwReg);
            dwReg = GET(ghwIO->vidSerialParallelPort);
            return( (dwReg & H3_VMI_I2C_DATA_STATE_MASK) ? 1:0 );

		  default:
				return(0xff);
    }              
}// I2C_In


/*----------------------------------------------------------------------
Function name: I2C_Start

Description:   Send a start condition: HIGH to LOW transition
               on SDA line while SCL is HIGH

Return:        NONE
----------------------------------------------------------------------*/
void I2C_START(NT9XDEVICEDATA * ppdev)
{
    if ( I2C_IN(ppdev, SCL) == 0 )
    {
       I2C_OUT(ppdev, SDA, 1);
       I2C_OUT(ppdev, SCL, HICLK);
    }
    I2C_OUT(ppdev, SDA, 0);
    I2C_OUT(ppdev, SCL, LOCLK);
}// I2C_Start


/*----------------------------------------------------------------------
Function name: I2C_Stop

Description:   Send a stop condition: LOW to HIGH transition
               on SDA line while SCL is HIGH

Return:        NONE
----------------------------------------------------------------------*/
void I2C_STOP(NT9XDEVICEDATA * ppdev)
{
    I2C_OUT(ppdev, SDA, 0);
    I2C_OUT(ppdev, SCL, HICLK);
    I2C_OUT(ppdev, SDA, 1);
}// I2C_Stop


/*----------------------------------------------------------------------
Function name: I2C_SendACK

Description:   Send acknowledge

Return:        NONE
----------------------------------------------------------------------*/
void I2C_SENDACK(NT9XDEVICEDATA * ppdev)
{
    I2C_OUT(ppdev, SCL, LOCLK);        // make sure clk is lo
    I2C_OUT(ppdev, SDA, 0);            // data lo for ack
    I2C_OUT(ppdev, SCL, HICLK);        // make clk valid
    I2C_OUT(ppdev, SCL, LOCLK);        // finish off
    I2C_OUT(ppdev, SDA, 1);            // make data marking for wired and
}//  I2C_SendACK


/*----------------------------------------------------------------------
Function name: I2C_SendNACK

Description:   Send NOT acknowledge

Return:        NONE
----------------------------------------------------------------------*/
void I2C_SENDNACK(NT9XDEVICEDATA * ppdev)
{
    I2C_OUT(ppdev, SCL, LOCLK);        // make sure clk is lo
    I2C_OUT(ppdev, SDA, 1);            // data hi for nack
    I2C_OUT(ppdev, SCL, HICLK);        // make clk valid
    I2C_OUT(ppdev, SCL, LOCLK);        // finish off
    I2C_OUT(ppdev, SDA, 1);            // make data marking for wired and
}// I2C_SendNACK


/*----------------------------------------------------------------------
Function name: I2C_ACK

Description:   Get an acknowledge back from a slave device

Return:        int
----------------------------------------------------------------------*/
int I2C_ACK(NT9XDEVICEDATA * ppdev)
{   
    int i;
    
    I2C_REL(ppdev, SDA);               // Release SDA line
    I2C_OUT(ppdev, SCL, HICLK);
    i = !I2C_IN(ppdev, SDA);
    I2C_OUT(ppdev, SCL, LOCLK);
    return(i);
}// I2C_ACK


/*----------------------------------------------------------------------
Function name: I2C_Init

Description:   

Return:        NONE
----------------------------------------------------------------------*/
void I2C_INIT(NT9XDEVICEDATA * ppdev)
{        
   FxU32 dwReg;

   //Release data line and toggle SCL as long as data line is driven to 0
   dwReg = GET(ghwIO->vidSerialParallelPort) | H3_VMI_I2C_ENABLE_MASK | H3_VMI_I2C_DATA_MASK;
   dwReg &= ~H3_VMI_I2C_CLOCK_MASK;
   SETDW(ghwIO->vidSerialParallelPort, dwReg);

   for ( ; !((dwReg = GET(ghwIO->vidSerialParallelPort)) & H3_VMI_I2C_DATA_MASK);  )
   {
		I2C_OUT(ppdev, SCL, HICLK);
		I2C_OUT(ppdev, SCL, LOCLK);    
   }
   I2C_STOP(ppdev); //Reset all devices
}// I2C_Init


/*----------------------------------------------------------------------
Function name: I2C_WByte

Description:   

Return:        int
----------------------------------------------------------------------*/
int I2C_WBYTE(NT9XDEVICEDATA * ppdev, BYTE data)
{
    BYTE i;

	 // Send out data stream MSbit goes out first
    for (i = 0x80; i; i >>= 1)  
    {
		if ( data & i )
       {
			if ( I2C_OUT(ppdev, SDA, 1) )
           {
                I2C_INIT(ppdev);
                return(0);
           }
       }
		else
       {
			I2C_OUT(ppdev, SDA, 0);
       }
		I2C_OUT(ppdev, SCL, HICLK);
		I2C_OUT(ppdev, SCL, LOCLK);    
    }
    return( I2C_ACK(ppdev) );    // Wait for acknowledge
}// I2C_WByte


/*----------------------------------------------------------------------
Function name: GetRegPtr

Description:   

Return:        
----------------------------------------------------------------------*/
PSTR GETREGPTR(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE index)
{
   int i, j;
	I2C_DEV *dptr;

	switch (addr)				//Get ptr to device struct
	{
		case DEV_SAA7110:
		case DEV_BT829:
		default:
			dptr = I2CDev;		
			for ( i=0; i < MAX_I2C_DEVICES; i++ )
			{
				if ( dptr->wDevAddr == addr )
				{
				    if ( 0 <= dptr->wRegCnt && dptr->wRegCnt <= I2C_REGSPERDEV ) 
				    {
						 for ( j=0; j < (int)dptr->wRegCnt; j++) 
						 {
						 	if (dptr->Regs[j].Index == index)
								return(&dptr->Regs[j].Data);
						 }            
				    }
					break;
				}
				else
				{
					dptr++;
				}
			}
			break;
	}
   return(NULL);
}//  GetRegPtr


/*----------------------------------------------------------------------
Function name: WriteI2CReg

Description:   Write an I3C device register
               Argument format: DevID, RegisterIndex, Data

Return:        NONE
----------------------------------------------------------------------*/
void WRITEI2CREG(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr, BYTE data)
{
    PSTR rptr;
    int i;

    for ( i = 0 ; i < 1000; i++ )
    {	       
       I2C_START(ppdev);
       if ( !I2C_WBYTE(ppdev, (BYTE)(addr & I2C_WRITE)) ) //Addr LSB is 0 for write
          I2C_STOP(ppdev);
       else if ( !I2C_WBYTE(ppdev, subaddr) )
          I2C_STOP(ppdev);
       else if ( !I2C_WBYTE(ppdev, data) )
          I2C_STOP(ppdev);
       else 
          break;
    }
       
    I2C_STOP(ppdev);

    //If device has been initialized record the new register value for future reads 
    if ( (rptr = GETREGPTR(ppdev, addr, subaddr)) )
		*rptr = data;
}// WriteI2CReg


/*----------------------------------------------------------------------
Function name: ReadI3CRegCache

Description:   Read an I2C device cache register

Return:        BYTE
----------------------------------------------------------------------*/
BYTE READI2CREGCACHE(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr)
{
    PSTR rptr;

    if ( (rptr = GETREGPTR(ppdev, addr, subaddr)) )
		return *rptr;
    else
		return 0xFF;
}// ReadI3CRegCache


/*----------------------------------------------------------------------
Function name: ReadI2CStatus

Description:   Read an I2C device status

Return:        BYTE
----------------------------------------------------------------------*/
BYTE READI2CSTATUS(NT9XDEVICEDATA * ppdev, BYTE addr)
{
    int j;
    BYTE data;
	
    I2C_START(ppdev);
    I2C_WBYTE(ppdev, (BYTE)(addr|I2C_READ));	//Addr LSB is 1 for read

    for (data = j = 0; j < 8 ; j++) 
    {
      I2C_OUT(ppdev, SCL, LOCLK);      
      I2C_OUT(ppdev, SCL, HICLK);
      data = (data << 1) | I2C_IN(ppdev, SDA); // read when clk is high
    }
    I2C_OUT(ppdev, SCL, LOCLK);
    I2C_SENDACK(ppdev);						// send an ACK (or NACK for finish?)
    I2C_STOP(ppdev);
    return(data);
}// ReadI2CStatus


/*----------------------------------------------------------------------
Function name: WriteI2CData

Description:   Write an I2C device register
			   Argument format: DevID, NumOfData, DataArray
			   Comment: The data stream sent to the slave device
                        is not cached.


Return:        NONE
----------------------------------------------------------------------*/
void WRITEI2CDATA(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE numbytes, BYTE *sptr)
{
	int i;

	if (numbytes > 0 && sptr != NULL)
	{		       
	    I2C_START(ppdev);
	    I2C_WBYTE(ppdev, (BYTE)(addr & I2C_WRITE));
		for ( i = 0; i < numbytes; i++ )
	    	I2C_WBYTE(ppdev, *sptr++);
	    I2C_STOP(ppdev);
	}
}// WriteI2CData


/*----------------------------------------------------------------------
Function name: ReadI2CData

Description:   Read I2C data stream
			   Argument format: DevID, NumOfData, DataArray
			   Return: Number of data received
			   Comment: No attempt is made to check the number of
                        valid data actually sent by the slave device.
			   
Return:        BYTE
----------------------------------------------------------------------*/
BYTE READI2CDATA(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr, BYTE numbytes, BYTE *dptr)
{  
   int i, j;
   BYTE data;
	
	if (numbytes > 0 && dptr != NULL)
	{	
       for ( ;; )
       {
	        I2C_START(ppdev);
           
           if ( !I2C_WBYTE(ppdev, (BYTE)(addr & I2C_WRITE)) ) //Addr LSB is 0 for write
               I2C_STOP(ppdev);
           else if ( !I2C_WBYTE(ppdev, subaddr) )
               I2C_STOP(ppdev);
           else
               break;
       }
       
       for ( ;; )
       {
	        I2C_START(ppdev);
	        if ( !I2C_WBYTE(ppdev, (BYTE)(addr | I2C_READ)) ) //Addr LSB is 1 for read
               I2C_STOP(ppdev);
           else
               break;
       }

		for (i = 0; i < numbytes; )
		{
		    for (data = j = 0; j < 8 ; j++) 
		    {
			  I2C_OUT(ppdev, SCL, LOCLK);      
			  I2C_OUT(ppdev, SCL, HICLK);
			  data = (data << 1) | I2C_IN(ppdev, SDA);   // read when clk is high
		    }
		    I2C_OUT(ppdev, SCL, LOCLK);
			dptr[i++] = data;
	    	I2C_SENDACK(ppdev);
		}
	    I2C_SENDACK(ppdev);
	    I2C_STOP(ppdev);
	}
   return(i);
}// ReadI2CData


/*----------------------------------------------------------------------
Function name: I2CDev_Init

Description:   Enable I2C communication channel registers.
			   Initialize I2C device datastructure and the
               device registers.

Return:        int
----------------------------------------------------------------------*/
int I2CDEV_INIT(NT9XDEVICEDATA * ppdev, I2C_DEV *devptr)
{
	unsigned int i, j;

	//Need to check if hardware has been enabled for I2C communication?

	if ( devptr->wDevAddr != 0 && devptr->wRegCnt > 0 )
	{
		for ( i = 0; i < MAX_I2C_DEVICES; i++ )
		{
			if ( I2CDev[i].wDevAddr == 0 && I2CDev[i].wRegCnt == 0 )
			{
				I2CDev[i] = *devptr;						//Save device's ID and configuration

				for ( j = 0; j < I2CDev[i].wRegCnt; j++ )	//Initialize I2C device
               {
                   if ( I2CDev[i].Regs[j].Index != 0xff )
			    	    WRITEI2CREG(ppdev, (BYTE)I2CDev[i].wDevAddr, I2CDev[i].Regs[j].Index, I2CDev[i].Regs[j].Data);
               }
				return(0); 
			}
			else if ( devptr->wDevAddr == I2CDev[i].wDevAddr )
			{
				break; //Device already exist
			}
		}		
	}
	return(-1);
}// I2CDev_Init


/*----------------------------------------------------------------------
Function name: I2CDev_Close

Description:   Close I2C device data structure

Return:        NONE
----------------------------------------------------------------------*/
void I2CDEV_CLOSE(NT9XDEVICEDATA * ppdev, BYTE addr)
{
	int i;

	if ( addr != 0 )
	{
		for ( i = 0; i < MAX_I2C_DEVICES; i++ )
		{
			if ( I2CDev[i].wDevAddr != 0 && I2CDev[i].wDevAddr == addr )
			{
				I2CDev[i].wDevAddr = 0;
				I2CDev[i].wRegCnt  = 0;
				break;				
			}
		}
	}
}// I2CDev_Close
