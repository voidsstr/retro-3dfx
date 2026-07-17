/* -*-c++-*- */
/* $Header: dfp.c, 20, 10/11/00 8:58:42 PM, Brent$ */
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
** File name:   dfp.c
**
** Description: Support functions for a digital flat panel.
**
** $Revision: 20$
** $Date: 10/11/00 8:58:42 PM$
**
** $Log: 
** 
*/
#define PRESENT_BIT 0x1
#define ACTIVE_BIT 0x2

#ifndef WINNT
   #define FAR
   #define WINAPI __stdcall
   typedef          char  CHAR;
   typedef   signed char  SCHAR;
   typedef unsigned char  UCHAR;

   typedef          short SHORT;
   typedef unsigned short USHORT;

   typedef          long  LONG;
   typedef unsigned long  ULONG;

   typedef unsigned char  BYTE,  FAR* LPBYTE;
   typedef unsigned short WORD,  FAR* LPWORD;
   typedef unsigned long  DWORD, FAR* LPDWORD;
   typedef          void  VOID,  FAR* LPVOID;
   typedef          int   BOOL;
   typedef int (FAR WINAPI *FARPROC)();


   #pragma pack( 1 )
       #include "gdidefs.h"
       #include "dibeng.h"
   #pragma pack()
#endif

   #include "3dfx.h"

#ifdef WINNT
   #include "miniport.h"
   #include "ntddvdeo.h"
   #include "video.h"
   #include "h3.h"
   #define DELAY(usecs)      VideoPortStallExecution(usecs)
#else
   #include "h3vdd.h"
   #include "h3.h"
   #include "shared.h"
   #define DELAY(usecs) CM_Yield(usecs, CM_YIELD_RESUME_EXEC);
#endif  // else #ifdef WINNT


#ifdef WINNT
#define DFP_DEVINFOPTR(pcontext) ((PHW_DEVICE_EXTENSION)(pcontext))
#define IS_NAPALM_X(pcontext)   (0x06 <= (DFP_DEVINFOPTR(pcontext))->PCIDeviceID)
#define IS_VOODOO3_X(pcontext)  (0x05 >= (DFP_DEVINFOPTR(pcontext))->PCIDeviceID)
#else
#define DFP_DEVINFOPTR(pcontext) ((PDEVTABLE)(pcontext))
#define IS_NAPALM_X(pcontext)   (IS_NAPALM((DFP_DEVINFOPTR(pContext))->dwVendorDeviceID))
#define IS_VOODOO3_X(pcontext)  (IS_VOODOO3((DFP_DEVINFOPTR(pContext))->dwVendorDeviceID))
#endif

#include "dfp.h"
#include "dfpdisp.h"
#include "bios.h"

#include "sliaa.h"
#ifndef WINNT
#define PCI_CFG_RD(A, B) PCI_Read_Config(DFP_DEVINFOPTR(pContext)->dwBus, DFP_DEVINFOPTR(pContext)->dwDevFunc, A)
#endif

// established timings in EDID per VESA

static DFP_MODE EstTimings[] =
   {{800, 600, 60},
	{800, 600, 56},
	{640, 480, 75},
	{640, 480, 72},
	{640, 480, 67},
	{640, 480, 60},
	{720, 400, 88},
	{720, 400, 70},
	{1280, 1024, 75},
	{1024, 768, 75},
	{1024, 768, 70},
	{1024, 768, 60},
	{1024, 768, 87},
	{832, 624, 75},
	{800, 600, 75},
	{800, 600, 72},
	{1152, 700, 75},
	{0, 0, 0}};




/*----------------------------------------------------------------------
Macro name:  READSCRATCHREGISTER3

Description:    read a byte value from BIOS scratch register 3 (location 0x1F)

Information:    
----------------------------------------------------------------------*/
#ifdef WINNT
#define READSCRATCHREGISTER3(pContext,result)  \
	{ if (DFP_DEVINFOPTR(pContext)->IsSecondaryDevice) result = DFP_DEVINFOPTR(pContext)->fakeBiosScr3; \
    else { \
    /* save index reg setting */ \
    FxU8 bSavedIndex = VideoPortReadPortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4); \
    /* return value from register shared with BIOS */ \
    VideoPortWritePortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4, 0x1f);  /* select the reg */ \
    result = VideoPortReadPortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd5);  /* read the reg */ \
    /* restore index reg setting */ \
    VideoPortWritePortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4, bSavedIndex); \
    }}
#else
#define READSCRATCHREGISTER3(pContext,result)  \
    { \
    /* save index reg setting*/ \
    FxU8 bSavedIndex = inp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4)); \
    /* return value from register shared with BIOS */ \
    outp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), 0x1f);  /* select the reg */ \
    result = inp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd5));  /* read the reg */ \
    /* restore index reg setting */ \
    outp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), bSavedIndex); \
    }
#endif
/*----------------------------------------------------------------------
Macro name:  WRITESCRATCHREGISTER3

Description:    write a byte value to BIOS scratch register 3  (location 0x1F)

Information:    
----------------------------------------------------------------------*/
#ifdef WINNT
#define WRITESCRATCHREGISTER3(pContext,value) \
    { if (DFP_DEVINFOPTR(pContext)->IsSecondaryDevice) DFP_DEVINFOPTR(pContext)->fakeBiosScr3 = value; \
    else { \
    /* save index reg setting */ \
    FxU8 bSavedIndex = VideoPortReadPortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4); \
    /* write value to register shared with BIOS */ \
    VideoPortWritePortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4, 0x1f);  /* select the reg */ \
    VideoPortWritePortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd5,value);  /* write the reg */ \
    /* restore index reg setting */ \
    VideoPortWritePortUchar((PUCHAR)DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_INDEX] + 0xd4, bSavedIndex); \
    }}
#else
#define WRITESCRATCHREGISTER3(pContext,value) \
{ \
    /* save index reg setting */ \
    FxU8 bSavedIndex = inp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4)); \
    /* write value to register shared with BIOS */ \
    outp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), 0x1f);  /* select the reg */ \
    outp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd5), value);  /* write the reg */ \
    /* restore index register setting */ \
    outp ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), bSavedIndex); \
}
#endif


/*----------------------------------------------------------------------
Function name:  panelEDID_help and panelEDID

Description:    Retrieves the panel EDID Info.

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/


int panelEDID_help(void * pContext, FxU8 bAddress, FxU8 *ddcData)
{
#ifdef WINNT
    H3_MEMBASE0 *sstIO = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
#else
	SstIORegs *sstIO = (void *)DFP_DEVINFOPTR(pContext)->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   I2CKEY dfpddc;
	int i, nb, sum, retstat;
	DFP_MODE *eTiming = 0;
	FxU8 retries = 2;

retry:
   
   dfpddc = i2c_getaccess(pContext, I2C_FLATPANELDDC, I2C_NORMALSPEED);
   if (dfpddc == I2C_NOTAKEY)
      goto bailout;

   retstat = 0;
   retstat |= !i2c_start(pContext, dfpddc);
   retstat |= !i2c_sendbyte(pContext, dfpddc, (FxU8)(bAddress | I2C_WRITESLAVE));
   retstat |= !i2c_sendbyte(pContext, dfpddc, 0);
   retstat |= !i2c_stop(pContext, dfpddc);
   retstat |= !i2c_start(pContext, dfpddc);
   retstat |= !i2c_sendbyte(pContext, dfpddc, (FxU8)(bAddress | I2C_READSLAVE));

   if (retstat)
      goto bailout; // it's obvious this retry won't work; try to speed things up


#define LCD_BUG_PRINCETONDFP
#ifdef LCD_BUG_PRINCETONDFP
   // @RBISSELL: The Princeton "directdigital" DFP returns a bogus EDID header.
   //            Instead of "00,FF,FF,FF,FF,FF,FF,00", it starts the EDID with the
   //            the header "00,00,FF,FF,FF,FF,FF,00".  The checksum reported within
   //            the EDID is consistent with this error.

   *(FxU32*)&ddcData[0] = 0x00000000;
   for (sum = i = 0; i < 4; i++)
	{
      i2c_readbyte(pContext, dfpddc, &ddcData[i], 1);
		sum += ddcData[i];
		DELAY(10000);
	}
   if ((*(FxU32*)&ddcData[0]) == 0xFFFF0000)  // is it a broken Princeton DFP?
   {
      // change the second byte to 0xFF, and adjust the checksum accordingly.
      sum += 0xFF;
      *(FxU32*)&ddcData[0] = 0xFFFFFF00;
   }
   for (nb = 128; i < nb; i++) //read the remainder of the edid
#else
	for (sum = i = 0, nb = 128; i < nb; i++)
#endif
   {
      i2c_readbyte(pContext, dfpddc, &ddcData[i], i != (nb - 1));
		nb = ddcData[0] >= 0x20 ? 256 : 128;
		sum += ddcData[i];
		DELAY(10000);
	}

bailout:
   i2c_stop(pContext, dfpddc);
   i2c_endaccess(pContext, dfpddc);

	if (!retstat && !(sum & 255) && ddcData[0] < 0x80 && sum)
      return 0;
	else if (--retries)
		goto retry;
	else
      return -1;
}

int panelEDID(void * pContext, FxU8 *ddcData)
{
   if (panelEDID_help(pContext, 0xA2, ddcData))
      if (panelEDID_help(pContext, 0xA0, ddcData))
         return -1;

   return 0;
}



/*----------------------------------------------------------------------
Function name:  panelDDC_help and panelDDC

Description:    Parse the panel EDID Info.

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/


int panelDDC_help(void * pContext, DFP_DISPLAY *disp, FxU8 bAddress)
{
#ifdef WINNT
    H3_MEMBASE0 *sstIO = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
#else
    SstIORegs *sstIO = (void *)DFP_DEVINFOPTR(pContext)->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
	int i, j, blk, clk, retstat;
    FxU8 ddcData[256], *dt;
    DFP_MODE *eTiming = 0;
    FxU8 retries = 2;

retry:

   if (panelEDID_help(pContext, bAddress, ddcData) != -1)
	{
		/* EDID parsing is taken from VESA EDID Standard v3 r0, 1997 */

		if (ddcData[0] >= 0x20)   // decode V2.X EDID
		{
			dt = &ddcData[128];   
			// skip over luma table if present
			if (ddcData[0x7e] & 0x20)
			{
				if (ddcData[0x80] & 0x80)
					dt += ((ddcData[0x80] & 0x1f) * 3);
				else
					dt += (ddcData[0x80] & 0x1f);
			}
			// skip frequency ranges
			dt += (8 * ((ddcData[0x7e] >> 2) & 7));
			// if timing ranges are present
			if (ddcData[0x7e] & 3)
			{
				// extract H parameters
				blk = ((dt[4] << 4) & 0xf00) | dt[2];
				blk += (((dt[13] << 4) & 0xf00) | dt[11]);
				disp->dispWidth = ((dt[23] << 4) & 0xf00) | dt[21];
				disp->htotal = disp->dispWidth + (blk / 2);
				disp->hsyncOfst = ((dt[8] << 2) & 0x300) | dt[5];
				disp->hsyncWidth = ((dt[8] << 4) & 0x300) | dt[6];
				// extract V parameters
				blk = ((dt[4] << 8) & 0xf00) | dt[3];
				blk += ((dt[13] << 8) & 0xf00) | dt[12];
				disp->dispHeight = ((dt[23] << 8) & 0xf00) | dt[22];
				disp->vtotal = disp->dispHeight + (blk / 2);
				disp->vsyncOfst = ((dt[8] << 2) & 0x030) | (dt[7] >> 4);
				disp->vsyncWidth = ((dt[8] << 4) & 0x030) | (dt[7] & 15);
				// get the average clk rate
				clk = *(FxU16 *)&dt[0];
				clk += (dt[9] + (dt[10] << 8));
				disp->dispClock = (long)(clk / 2) * (long)10;

#if WINNT && defined(DMT_ENABLED)
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.width = disp->dispWidth;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.height = disp->dispHeight;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HTotal = disp->htotal;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HSyncStart = disp->dispWidth + disp->hsyncOfst;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HSyncEnd = disp->dispWidth + disp->hsyncOfst + disp->hsyncWidth;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VTotal = disp->vtotal;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VSyncStart = disp->dispHeight + disp->vsyncOfst;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VSyncEnd = disp->dispHeight + disp->vsyncOfst + disp->vsyncWidth;
                DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.PixelClock = disp->dispClock * 1000;
#endif
			}
         // skip detailed range limits
         dt += 27 * (ddcData[0x7e] & 3);
         // if the 4-byte timing codes are present
         if (ddcData[0x7f] & 0xf8)
         {
            for(i=0; i<(ddcData[0x7f] & 0xf8); i++)
            {
               DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].width = (dt[0] * 16) + 256;
               switch(dt[2])
               {
                  case 160: // 16:10 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].width * 10) / 16;
                     break;
                  case 133: //  4: 3 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].width * 3 ) /  4;
                     break;
                  case 125: //  5: 4 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].width * 4 ) /  5;
                     break;
                  case 178: // 16: 9 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].width * 9 ) / 16;
                     break;
               }
               DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[i].refresh = dt[3];
               if (i == MAX_DFP_MODES)
                  break;

               dt += 4;
            }
         }
		}
		else    // EDID V1.X
		{
			// parse out the established timings to get the minimum we need to
			// know - display height, width and refresh.  Give preference to 
			// highest resolution and allow only 60hz refresh.

         j=0;
			for (i = 0; i < 8; i++)
			{
				if ((ddcData[0x24] & (1 << i)) &&
					EstTimings[i + 8].refresh == 60)
            {
						DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j++] = EstTimings[i + 8];
                  if (j == MAX_DFP_MODES)
                     goto mode_list_full;
            }
			}
			for (i = 0; i < 8; i++)
			{
				if ((ddcData[0x23] & (1 << i)) &&
					EstTimings[i + 0].refresh == 60)
            {
						DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j++] = EstTimings[i + 0];
                  if (j == MAX_DFP_MODES)
                     goto mode_list_full;
            }
			}
         for (i = 0; i < 16; i+=2)
         {
            if((ddcData[0x26 + i]     != 0x01) || //STI is valid
               (ddcData[0x26 + i + 1] != 0x01))
            {
               if((ddcData[0x26 + i + 1] & 0x3f) == 0) //Refresh rate == 60Hz
               {
                  DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].width = (ddcData[0x26 + i] + 31) * 8;
                  switch(ddcData[0x26 + i + 1] & 0xc0)
                  {
                  case 0x00: // 16:10 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].width * 10) / 16;
                     break;
                  case 0x40: //  4: 3 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].width * 3 ) /  4;
                     break;
                  case 0x80: //  5: 4 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].width * 4 ) /  5;
                     break;
                  case 0xc0: // 16: 9 Aspect Ratio
                     DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].height = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].width * 9 ) / 16;
                     break;
                  }
                  DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[j].refresh = 60;
                  j++;
                  if (j == MAX_DFP_MODES)
                     goto mode_list_full;
               }
            }
         }
mode_list_full:

			dt = &ddcData[54];
			if ((clk = *(FxU16 *)&dt[0]) <= 0x0101)
			{
				disp->vsyncWidth = 6;    // HACK HACK
				return (0);    // no detailed timing, just dimensions
			}
			disp->dispClock = clk * 10;
			// extract H parameters
			disp->dispWidth = ((dt[4] << 4) & 0xf00) | dt[2];
			disp->htotal = ((dt[4] << 8) & 0xf00) + dt[3] + disp->dispWidth;
			disp->hsyncOfst = ((dt[11] << 2) & 0x300) | dt[8];
			disp->hsyncWidth = ((dt[11] << 4) & 0x300) | dt[9];
			// extract V parameters
			disp->dispHeight = ((dt[7] << 4) & 0xf00) | dt[5];
			disp->vtotal = ((dt[7] << 8) & 0xf00) + dt[6] + disp->dispHeight;
			disp->vsyncOfst = ((dt[11] << 2) & 0x30) | (dt[10] >> 4);
			disp->vsyncWidth = ((dt[11] << 4) & 0x30) | (dt[10] & 15);

#if WINNT && defined(DMT_ENABLED)
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.width = disp->dispWidth;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.height = disp->dispHeight;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HTotal = disp->htotal;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HSyncStart = disp->dispWidth + disp->hsyncOfst;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.HSyncEnd = disp->dispWidth + disp->hsyncOfst + disp->hsyncWidth;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VTotal = disp->vtotal;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VSyncStart = disp->dispHeight + disp->vsyncOfst;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.VSyncEnd = disp->dispHeight + disp->vsyncOfst + disp->vsyncWidth;
            DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings.PixelClock = disp->dispClock * 1000;
#endif
		}

        retstat = 0;
	}
	else if (--retries)
		goto retry;
	else
		retstat = 1;

	return (retstat ? -1 : 0);
}

/*----------------------------------------------------------------------
Function name:  panelDDC

Description:    Call 'panelDDC_help()' to get panel EDID.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
int panelDDC(void * pContext, DFP_DISPLAY *disp)
{
    if (panelDDC_help(pContext, disp, 0xA2))
        if (panelDDC_help(pContext, disp, 0xA0))
            return -1;

        return 0;
}

/*----------------------------------------------------------------------
Function name:  panelOff

Description:    Turn off the panel device.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void panelOff(void * pContext)
{
    FxU8 dfpbits;
#ifdef WINNT
    H3_MEMBASE0 *sstIOregs = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
#else
    SstIORegs *sstIOregs = (void *)DFP_DEVINFOPTR(pContext)->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    sstIOregs->vidInFormat &= ~(SST_VIDEOIN_TVOUT_ENABLE);  // off
    sstIOregs->vidInFormat |= SST_VIDEOIN_G4_FOR_POSEDGE;  // Like Brooktree.

    // If its not V3 then clear the bit so we don't break TVout code.
    if(!IS_VOODOO3_X(pContext)) 
    {
       sstIOregs->vidInFormat &= ~SST_VIDEOIN_TV_DATA_SCRAMBLE_DISABLE;
    }

    // let BIOS know DFP is not active
    READSCRATCHREGISTER3(pContext, dfpbits);
    dfpbits &= ~BIOS_CRx1F_DFPACTIVE;
    WRITESCRATCHREGISTER3(pContext, dfpbits);

#ifdef WINNT
    // set the timing again because the DFP timing may be different from the monitor timing
    H3SetModeResetTiming(DFP_DEVINFOPTR(pContext));
#endif
}


/*----------------------------------------------------------------------
Function name:  panelOn

Description:    Turn on the panel.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void panelOn(void * pContext)
{
    FxU8 dfpbits;
#ifdef WINNT
    H3_MEMBASE0 *sstIOregs = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
#else
    SstIORegs *sstIOregs = (void *)DFP_DEVINFOPTR(pContext)->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    sstIOregs->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;
    sstIOregs->vidInFormat &= ~SST_VIDEOIN_G4_FOR_POSEDGE;  // Like Chrontel.

    // If its not V3 then set the bit for direct connect to SII part.
    if(!IS_VOODOO3_X(pContext)) 
    {
       sstIOregs->vidInFormat |= SST_VIDEOIN_TV_DATA_SCRAMBLE_DISABLE;
    }

    // let BIOS know DFP is active
    READSCRATCHREGISTER3(pContext, dfpbits);
    dfpbits |= BIOS_CRx1F_DFPACTIVE;
    WRITESCRATCHREGISTER3(pContext, dfpbits);

#ifdef WINNT
    // set the timing again because the DFP timing may be different from the monitor timing
    H3SetModeResetTiming(DFP_DEVINFOPTR(pContext));
#endif WINNT

}


/*----------------------------------------------------------------------
Function name:  isPanelActive

Description:    Retrieve 'PANEL IS Active' attribute from driver.

Return:         Boolean     TRUE or FALSE.
----------------------------------------------------------------------*/
FxU8 isPanelActive(void * pContext)
{
    // No released Voodoo3 boards are supported by this driver.
    if (IS_VOODOO3_X(pContext))
        return(FALSE);

#if WINNT
    return ((DFP_DEVINFOPTR(pContext)->dwDFPState & ACTIVE_BIT)==ACTIVE_BIT);
#else
    return ((((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState & ACTIVE_BIT)==ACTIVE_BIT);
#endif
}

/*----------------------------------------------------------------------
Function name:  isPanelPresent

Description:    Retrieve 'PANEL IS Present' attribute from driver.

Return:         Boolean     TRUE or FALSE.
----------------------------------------------------------------------*/
FxU8 isPanelPresent(void * pContext)
{
    // No released Voodoo3 boards are supported by this driver.
    if (IS_VOODOO3_X(pContext))
        return(FALSE);

#if WINNT
    return ((DFP_DEVINFOPTR(pContext)->dwDFPState & PRESENT_BIT)==PRESENT_BIT);
#else
    return ((((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState & PRESENT_BIT)==PRESENT_BIT);
#endif
}

/*----------------------------------------------------------------------
Function name:  isPanelScaling

Description:    Look in the parsed EDID data to see if the panel
                supports more than one mode

Information:    Checks to see if BIOS has a scaling panel connected.

Return:         Boolean  TRUE or FALSE
----------------------------------------------------------------------*/
FxU8 isPanelScaling(void * pContext)
{
    return DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[1].width != 0;
}

#ifndef	WINNT
/*----------------------------------------------------------------------
Function name:  panelFixupVGA

Description:    Called when disabling desktop to fix VGA CRTC registers FBO
                a proper full screen DOS box on the panel.

Information:    Only called if panel is active.

Return:         Returns 0
----------------------------------------------------------------------*/

int panelFixupVGA(void * pContext, FxU32 unused)
{
    outpw ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), 0x0c11);   // unlock VGA registers
    outpw ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), 0xff06);   // adjust refresh rate
    outpw ((FxU16)(DFP_DEVINFOPTR(pContext)->IoBase + 0xd4), 0x8c11);   // relock VGA registers
    return (0);
}
#endif


void UpdateAttributes(void * pContext)
{
    FxU8 dfpbits;

    // panel attributes can change because of hotplug event.  The hotplug "unplug" interrupt clears the
    // dfpDisp.dispWidth variable which will force us to read the EDID again here.  Since all outside requests
    // eventually pass though this routine the "DFP_scaling" attribute will be updated before it is used.
    if (isPanelPresent(pContext) &&
        (DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth == 0))    // if dfpDisp.dispWidth != 0, then we must have EDID.
    {
        // setup default values if the read EDID fails.
        DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth = 1024;
        DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth = 768;
        DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[1].width = 0;  // key value for "is DFP scaling";
        
#if WINNT && defined(DMT_ENABLED)
        // initialize centered mode base timings, hardcode to 1024x768@60hz
        memcpy( &DFP_DEVINFOPTR(pContext)->centeredDfpCurrentTimings, 
                &DFP_DEVINFOPTR(pContext)->centeredDfpBaseTimings, 
                            sizeof(TIMING_PARAMS) );
#endif

        // try to read the EDID, ignore errors
        panelDDC (pContext, &DFP_DEVINFOPTR(pContext)->dfpDisp);

        // inform BIOS of DFP configuration using NAPALM style bit pattern    

        READSCRATCHREGISTER3(pContext, dfpbits);

        //Set up the res bit field
        if(DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth >= 1600)
        {
            dfpbits &= ~BIOS_CRx1F_DFPCONNECTED;
            dfpbits |= BIOS_CRx1F_1600x1200;
        }
        else if(DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth >= 1280)
        {
            dfpbits &= ~BIOS_CRx1F_DFPCONNECTED;
            dfpbits |= BIOS_CRx1F_1280x1024;
        }
        else
        {
            dfpbits &= ~BIOS_CRx1F_DFPCONNECTED;
            dfpbits |= BIOS_CRx1F_1024x768;
        }

        //Set up the scaling bit
        if (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[1].width != 0)
            dfpbits |= BIOS_CRx1F_PANELSCALING;
        else
            dfpbits &= ~BIOS_CRx1F_PANELSCALING;

        WRITESCRATCHREGISTER3(pContext, dfpbits);
    }
}


#ifdef WINNT
int DFP_ProcessRequest(void * pContext, PVIDEO_REQUEST_PACKET RequestPacket)
{
    int status = 0;
//???    ULONG dfpStatus;

    UpdateAttributes(pContext);

    if (sizeof(ULONG) >  RequestPacket->InputBufferLength)
    {
        VideoDebugPrint((0,"DFPPACKET input buffer too small\n"));
        return (0xffffffff);
    }

    switch (((PDFPDEVICEPACKET)RequestPacket->InputBuffer)->dfpPacketFunc)
    {

    case dfpGetFlatPanelStatus:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(DFPSTATUS)))
        {
            VideoDebugPrint((0,"dfpGetFlatPanelStatus result buffer size mismatch\n"));
            status = 0xffffffff;
            break;
        }

        ((PDFPSTATUS)RequestPacket->OutputBuffer)->panelPresent = isPanelPresent(pContext);
        ((PDFPSTATUS)RequestPacket->OutputBuffer)->panelActiveAtBoot = DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot;
        ((PDFPSTATUS)RequestPacket->OutputBuffer)->panelActive = isPanelActive(pContext);
        break;

    case dfpTurnOnFlatPanel:
        panelOn( pContext );
        break;

    case dfpTurnOffFlatPanel:
        panelOff( pContext );
        break;
    }

    return (status);

}
#else
int DFP_ProcessRequest(void * pContext, DFPDEVICEPACKET * RequestPacket)
{
    int status = 0;

    UpdateAttributes(pContext);

    switch (RequestPacket->dfpPacketFunc)
    {
#if 0
    case dfpGetPanelAttributes:
        ((PDFPATTRIBS)OutputBuffer)->panelWidth = pInstance->priv.dfpDisp.dispWidth;
        ((PDFPATTRIBS)OutputBuffer)->panelHeight = pInstance->priv.dfpDisp.dispHeight;
        ((PDFPATTRIBS)OutputBuffer)->panelRefreshRate = 60;
        ((PDFPATTRIBS)OutputBuffer)->panelIsScaling = (DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[1].width != 0);
        ((PDFPATTRIBS)OutputBuffer)->panelIsActiveAtBoot = (pInstance->priv.bDfpActiveAtBoot);
        *resultSize = sizeof(DFPATTRIBS);
        break;
#endif

    case dfpGetFlatPanelStatus:
        RequestPacket->dfpOptData.dfpStatusResponse.panelPresent = isPanelPresent(pContext);
        RequestPacket->dfpOptData.dfpStatusResponse.panelActiveAtBoot = DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot;
        RequestPacket->dfpOptData.dfpStatusResponse.panelActive = isPanelActive(pContext);
        break;

    case dfpTurnOnFlatPanel:
        panelOn( pContext );
        break;

    case dfpTurnOffFlatPanel:
        panelOff( pContext );
        break;
    }

    return (status);

}
#endif

void DFP_Initialize(void * pContext)

{
    FxU8 ucTemp;
#ifdef WINNT
    ULONG 	       dwReg;
#if REDUCED_MEMORY_MAPPINGS
    PH3_3D_REGISTERS sstRegs = (PH3_3D_REGISTERS) ((UCHAR *) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_3D_REGS_INDEX]);
#else
    PH3_3D_REGISTERS sstRegs = (PH3_3D_REGISTERS) ((UCHAR *) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX] + SST_3D_OFFSET);
#endif  
    PHW_DEVICE_EXTENSION HwDeviceExtension = DFP_DEVINFOPTR(pContext);  // needed for PCI_CFG_RD macro
#endif  
    DWORD cfgSliAaMisc;

#ifdef WINNT
    DFP_DEVINFOPTR(pContext)->dwDFPState = 0;     // clear "present" bit
#else
    ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState = 0;     // clear "present" bit
#endif
    DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot  = FALSE;

    // No Voodoo3 boards are supported by this driver.
    if (IS_VOODOO3_X(pContext))
    {
        return;
    }

    // test if BIOS supports DFP
    if ((DFP_DEVINFOPTR(pContext)->biosBoardConfigInfo & BIOS_BOARDCONFIG_DFPSUPPORT)!=0)
    {
	    cfgSliAaMisc = PCI_CFG_RD(CFG_SLI_AA_MISC, 0);
	    if(cfgSliAaMisc & CFG_HOTPLUG_PIN)
#ifdef WINNT
	        DFP_DEVINFOPTR(pContext)->dwDFPState = PRESENT_BIT;
#else
            ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState = PRESENT_BIT;
#endif

	    // if Bios says panel is active the first time we check then we believe it was present at boot.
        READSCRATCHREGISTER3(pContext, ucTemp);
        DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot  = (!!(ucTemp & BIOS_CRx1F_DFPACTIVE));

        if (DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot)
#ifdef WINNT
            DFP_DEVINFOPTR(pContext)->dwDFPState |= ACTIVE_BIT; // set "active" bit
#else
            ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState |= ACTIVE_BIT; // set "active" bit
#endif

	    // force dfpDisp.dispWidth, and dfpDisp.dispHeight to be filled in with values read from EDID
	    DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth = 0;     // insure it's zero so EDID is read.

	    //??? should update be called here???

// Win9x HotPlug interrupt is set up elsewhere.
// hotplug is supported by Win2K but not WinNT4
#if (_WIN32_WINNT >= 0x500)
	    // test if BIOS supports DFP
	    if ((DFP_DEVINFOPTR(pContext)->biosBoardConfigInfo & BIOS_BOARDCONFIG_DFPSUPPORT)!=0)
	    {
	        // enable hotplug interrupt
	        dwReg = sstRegs->intrCtrl & 0x7FFFFFFF;
	        dwReg |= H5_HP_INT_ENABLE;
	        sstRegs->intrCtrl = dwReg;
	    }
#endif
	}
}
#if 0
void DFP_Initialize(void * pContext)

{
    FxU8 ucTemp;

    ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState = 0;     // clear "present" bit

    // No Voodoo3 boards are supported by this driver.
    if (IS_VOODOO3_X(pContext))
    {
        DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot  = FALSE;
        return;
    }

    // test if BIOS supports DFP
    if ((DFP_DEVINFOPTR(pContext)->biosBoardConfigInfo & BIOS_BOARDCONFIG_DFPSUPPORT)!=0)
    {
        // check if the hardware thinks a dfp is attached
//???        cfgSliAaMisc = PCI_CFG_RD(CFG_SLI_AA_MISC, 0);
//???        if(cfgSliAaMisc & CFG_HOTPLUG_PIN)
            ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState = PRESENT_BIT;     // set "present" bit

        // if Bios says panel is active the first time we check then we believe we booted to the DFP
        READSCRATCHREGISTER3(pContext, ucTemp);
        DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot  = (!!(ucTemp & BIOS_CRx1F_DFPACTIVE));

        if (DFP_DEVINFOPTR(pContext)->bDfpActiveAtBoot)
            ((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState |= ACTIVE_BIT; // set "active" bit

        // force dfpDisp.dispWidth, and dfpDisp.dispHeight to be filled in with values read from EDID
		DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth = 0;     // insure it's zero so EDID is read.

    }

}
#endif

#ifdef WINNT

/*----------------------------------------------------------------------
Function name:  DFP_SetActiveState

Description:    Enable or Disable the DFP device as selected by value of alternate
                display device mask passed in.

Information:

Return:         FxU32     STB_FUNCTION_DFP if success,
                          0 if the current mode does not allow completion.
----------------------------------------------------------------------*/
FxU32 DFP_SetActiveState (void * pContext, FxU32 activeStateMask)
{
    PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
    ULONG xres = RegisterMap->vidScreenSize & 0xfff;
    ULONG result = 0;


    if (isPanelPresent(pContext))
    {
        // the card is DFP Capable, check if DFP is supposed to be on.
        if ((activeStateMask & STB_FUNCTION_DFP) == STB_FUNCTION_DFP)
        { // DFP should be turned on
            // check if DFP is off and needs to be turned on
            if (!isPanelActive(pContext))
            {
                // DFP is off and needs to be turned on, Check if the current mode is valid for DFP
                if((DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth >= xres) /*??? &&
                   (current_refresh_rate == 60)???*/)
                    // The current mode is valid for DFP.
                    result = STB_FUNCTION_DFP;

                //  turn on DFP if valid mode or indicate that DFP should be on if invalid mode.
                panelOn( pContext );
            }
        }
        else
        {// DFP should be turned off
            // check if DFP is on and needs to be turned off
            if (isPanelActive(pContext))
                 panelOff( pContext );

            result = STB_FUNCTION_DFP;
        }
    }
    else
        result = STB_FUNCTION_DFP;  // if DFP is not present just say everything is ok.

    return(result);
}
#else
/*----------------------------------------------------------------------
Function name:  panelGbl

Description:    Returns a ptr to the globaldata panel status dword

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
PDWORD panelGbl(void * pContext)
{
   if(DFP_DEVINFOPTR(pContext)->lpDriverData)
      return &(((GLOBALDATA *)DFP_DEVINFOPTR(pContext)->lpDriverData)->dwDFPState);
   else
      return NULL;
}

#endif //else def WINNT

// Win9x HotPlug interrupt is handled differently
// hotplug is supported by Win2K but not WinNT4
#if (_WIN32_WINNT >= 0x500)
void ProcessHotPlugEvent(void * pContext,
                         void * pParams)
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = DFP_DEVINFOPTR(pContext);  // needed for PCI_CFG_RD macro
    DWORD cfgSliAaMisc;

    cfgSliAaMisc = PCI_CFG_RD(CFG_SLI_AA_MISC, 0);

    if(cfgSliAaMisc & CFG_HOTPLUG_PIN)
    {
        //Plug In

        //Set the connected bit for the control panel
        DFP_DEVINFOPTR(pContext)->dwDFPState = PRESENT_BIT;

    }
    else
    {
        //UnPlug

        //Deactivate the panel output
        if(isPanelActive(pContext))
            panelOff(pContext);

        //Invalidate the current edid info
        DFP_DEVINFOPTR(pContext)->dfpDisp.dispWidth = 0;
        DFP_DEVINFOPTR(pContext)->dfpDisp.stdModes[0].width = 0;

        //Reset the connected bit for the control panel
        DFP_DEVINFOPTR(pContext)->dwDFPState = 0;   // reset all PRESENT, and ACTIVE BITS.

    }

    VideoPortEnumerateChildren(HwDeviceExtension,NULL);
}

void DfpSetPowerOff(void * pContext)
{
    H3_MEMBASE0 *sstIOregs = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
    if (isPanelActive(pContext))
        sstIOregs->vidInFormat &= ~(SST_VIDEOIN_TVOUT_ENABLE);  // turn off signal to DFP
}

void DfpSetPowerOn(void * pContext)
{
    H3_MEMBASE0 *sstIOregs = (PH3_MEMBASE0) DFP_DEVINFOPTR(pContext)->MappedAddress[SST_IO_REGS_INDEX];
    if (isPanelActive(pContext))
        sstIOregs->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;  // turn on signal to DFP
}

#endif

//EOF
