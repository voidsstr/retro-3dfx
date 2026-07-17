/* -*-c++-*- */
/* $Header: gramp.c, 8, 10/11/00 8:51:12 PM, Brent$ */
/*
** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
** File name:   gramp.c
**
** Description: Support functions for gamma ramp.
**
** $Revision: 8$
** $Date: 10/11/00 8:51:12 PM$
**
** $History: gramp.c $
** 
** *****************  Version 15  *****************
** User: Edwin        Date: 6/29/99    Time: 3:57p
** Updated in $/devel/h5/Win9x/dx/dd16
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 14  *****************
** User: Michael      Date: 6/02/99    Time: 2:56p
** Updated in $/devel/h5/Win9x/dx/dd16
** Fix for adding -WX to makefile.
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 11  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 10  *****************
** User: Stb_mimhoff  Date: 4/26/99    Time: 1:15p
** Updated in $/devel/h3/win95/dx/dd16
** More work on PRS 5097... Slower PCs were still having the problem, so
** the set_gammaramp function was optimised...
** 
** *****************  Version 9  *****************
** User: Stb_mimhoff  Date: 4/08/99    Time: 10:33a
** Updated in $/devel/h3/win95/dx/dd16
** Imhoff - Fix for PRS 5097 ( At least this works on my system ) This
** read appears to be a wasted instruction and causes sparkle in
** resolutions 1920 or higher.
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 3/13/99    Time: 7:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to check DACMode before checking VSYNC
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 1/08/99    Time: 8:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed load of palette to be in Vertical Retrace
** 
** *****************  Version 6  *****************
** User: Michael      Date: 12/29/98   Time: 11:02a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 12/14/98   Time: 1:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Swap Red/Blue on input..swap on output
** 
** *****************  Version 4  *****************
** User: Michael      Date: 12/01/98   Time: 3:53p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix gamma problems associated with Expendable Lite game.  OEMGammaRamp
** had the red and blue switched.  Also, occasional writes to the DAC data
** register were not getting through.  This is similar to a HW bug we had
** on A0/A1 but was supposably fixed.  Now if the data write is bad, it
** gets retried.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 10/16/98   Time: 3:55p
** Updated in $/devel/h3/Win95/dx/dd16
** Add code to put/check a gamma signature to fix problem with reloading
** default gamma.
** 
** *****************  Version 2  *****************
** User: Michael      Date: 9/16/98    Time: 5:35p
** Updated in $/devel/h3/Win95/dx/dd16
** Fred's (igx) assign_gammaramp() implementation and a minor gamma fix.
** 
** *****************  Version 1  *****************
** User: Michael      Date: 8/20/98    Time: 3:38p
** Created in $/devel/h3/Win95/dx/dd16
** New file for GammaRamp.
**
*/

#include "header.h"
#include "gramp.h"

extern UINT GetFlatSel(void);

/*----------------------------------------------------------------------
Function name:  assign_gammaramp

Description:    Assign r,g,b values to global gamma ramp location i.
                Incoming is 8-bit.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void assign_gammaramp(int i, int r, int g, int b, BOOL fOVL)
{
    r &= 0xff;
    g &= 0xff;
    b &= 0xff;
    if(fOVL)
    {
      _FF(ovlgamma_ramp).red[i] = r << 8;
      _FF(ovlgamma_ramp).green[i] = g << 8;
      _FF(ovlgamma_ramp).blue[i] = b << 8;
    }
    else
    {
      _FF(gamma_ramp).red[i] = r << 8;
      _FF(gamma_ramp).green[i] = g << 8;
      _FF(gamma_ramp).blue[i] = b << 8;
    }

}


/*----------------------------------------------------------------------
Function name:  set_gammaramp

Description:    Program the DAC with the saved values in the global
                gamma ramp array.

Information:
    The DAC is actually TWO DACs - the first is index 0..255,
    and the second is index 256..511. The display uses DAC 0,
    so we only program that part (indexes 0..255).

    In 16bpp+ modes, the DAC sets the hardware gamma ramp function.
    Instead of the usual 8bpp DAC operation, where each index
    is translated to R/G/B, each of the R/G/B entries in high color
    modes represents a gamma ramp.

    To rephrase - index 0 red is the red shade that a red value of
    0 should translate to. Index 1 blue is the blue shade that a
    blue value of 1 should translate to.

    For example, if all R entries in the DAC where set to 0
    no red would appear on the screen!.

    Note that we can't program an individual entry; instead we have
    to program an entire triplet.

    There is also DAC setting code in the minivxd, which I believe
    could be eliminated. But we currently make use of that code when
    switching INTO a full screen mode, so leave it in. (And, it makes
    things safer).

	In modes 1920 or higher, the time required to set the DAC during the 
	vertical blank becomes shorter, so we have optimized this function to
	reduce sparkle on slower machines. 

Return:         VOID
----------------------------------------------------------------------*/
void LoadPalette(DWORD DacAddr, FxU32 FAR *  pColor, WORD FlatSel, WORD Start, WORD End);
#define CHECK_PALETTE
#ifdef CHECK_PALETTE
extern WORD CheckPalette;
#endif
#pragma warning (disable: 4047 4704)
void set_gammaramp(BOOL foverlay)
{
#ifdef WIN_CSIM
   SstIORegs FAR *lph3IORegs = (SstIORegs *)_FF(regRealBase);
#endif
	FxU32 color[256];
   FxU32 r;
   FxU32 g;
   FxU32 b;
   FxU32 base;
   FxU32 data;
   int index;
   int j;
   #define MAX_RETRY (10)

	UINT FlatSel = GetFlatSel();
   DDGAMMARAMP *  this_gamma;

   

   if (foverlay)
      {
      this_gamma = &(_FF(ovlgamma_ramp));
      base = 256;
      }
   else
      {
      /* If desktop is not 16bpp or more, return */
      if ( _FF(bpp) <= 8 )
         return;
      this_gamma = &(_FF(gamma_ramp));
      base = 0;
      }

   for (index = 0; index < 256; ++index )
  	   {
      r = this_gamma->red[index];
    	g = this_gamma->green[index];
	   b = this_gamma->blue[index];

    	r = (r >> 8) & 0xff;
	   g = (g >> 8) & 0xff;
    	b = (b >> 8) & 0xff;

	   color[index] = r | (g << 8) | (b << 16);
      }

   /* Wait for vblank before updating dac entries to prevent sparkle */
   // Make sure Hsync and Vsync are toggling before we check them
   if (!(GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
      {
      // if in vtrace wait until out
      while ((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK))
         ;
   
      // if in active time then wait for start of vtrace
      while (!((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
         ;

      // should be start of vtrace
      }

   LoadPalette((DWORD)&lph3IORegs->dacAddr, color, FlatSel, (WORD)base, (WORD)base+256);
#define CHECK_PALETTE
#ifdef CHECK_PALETTE
   if (CheckPalette)
      {
      for (index = (int)base; index < (int)(base+256); index++)
         {
         // Write DAC address.
         SETDW(lph3IORegs->dacAddr, index);

         // Read DAC address, to avoid bursting.
         data = GET(lph3IORegs->dacAddr);

         // Read DAC data
         // On V3 there is a hand shaking problem between the
         // PCI clocks and the Video Clocks 
         for (j=0; j<MAX_RETRY; j++)
            {
            data = GET(lph3IORegs->dacData);
            if (data == color[index - base])
               break;
            }

         if (data != color[index - base])
            {
            DPF(DBGLVL_ALL, "dac index %x, got data = 0x%08lx expected = 0x%08lx", index, data, color[index - base]);
            _asm {int 03};
            }
         }
      }
#endif

}
#pragma warning (default: 4047 4704)

/* Gamma ramp signature - MUST BE 16 CHARACTERS LONG */

static unsigned char GAMMA_SIG[16] = "3Dfx Gamma Ramp ";


/* Expected gamma ramp default value */

#define DEFAULT_GAMMA_RAMP(i) (((unsigned)i << 8) | GAMMA_SIG[i & 0x0f])


/*----------------------------------------------------------------------
Function name:  init_gammaramp

Description:    Initialize saved gammaramp to linear.
                
Information:
    Initialize saved gammaramp to linear. Note that the gamma ramp
    will most likely be reprogrammed to be the desktop gamma before
    dx6 has a chance to read it! This is not a problem. If dx6
    requests reading the gamma ramp, then we will look at
    default_gamma_flag. If that is set, then we know the default
    gamma should be in effect (even if its not -- the desktop gamma
    may be in effect), and we will return that!

Return:         VOID
----------------------------------------------------------------------*/
void init_gammaramp(void)
{
    int i;

    for ( i = 0; i < 256; ++i )
    {
        _FF(gamma_ramp).red[i] =
        _FF(gamma_ramp).green[i] =
        _FF(gamma_ramp).blue[i] = DEFAULT_GAMMA_RAMP(i);
        _FF(ovlgamma_ramp).red[i] =
        _FF(ovlgamma_ramp).green[i] =
        _FF(ovlgamma_ramp).blue[i] = DEFAULT_GAMMA_RAMP(i);
    }

    _FF(default_gamma_flag) = 1;
}


/*----------------------------------------------------------------------
Function name:  test_gammasig

Description:    Test the gamma ramp signature.
                
Information:

Return:         INT     1 = The system default gamma ramp.
                        0 = Not system defualt gamma ramp.
----------------------------------------------------------------------*/
/* Test gamma ramp signature. Returns 1 if gamma ramp is the system
 * default gamma ramp
 */

int test_gammasig(LPDDGAMMARAMP lpGammaRamp)
{
    int i;
    unsigned int expected_value;

    for ( i = 0; i < 256; ++i )
    {
        expected_value = DEFAULT_GAMMA_RAMP(i);

        if ( (lpGammaRamp->red[i] != expected_value) ||
             (lpGammaRamp->green[i] != expected_value) ||
             (lpGammaRamp->blue[i] != expected_value) )
            return 0;
    }

    return 1;
}


/*----------------------------------------------------------------------
Function name:  OEMGammaRamp

Description:    Read or write hardware gamma ramp.
                
Information:
    Read or write hardware gamma ramp. This entry is ordinal 32
    in display.def.  Due to the nature of this function, I am not
    too sure WHY read is required.  We just send Windows what we
    wrote.  However, we DO program the hardware gamma ramp.

Return:         BOOL    1 = HW successfully programmed.
                        0 = HW not programmed.
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds OEMGammaRamp(
    LPDIBENGINE lpPDevice,
    WORD wGetSet,
    LPDDGAMMARAMP lpGammaRamp)
{
    int i;

    /* If not 16bpp or more, fail (gamma handled differently at 8bpp) */

    if ( _FF(bpp) <= 8 )
        return 0;

    if ( wGetSet ) /* 0 == read, 1 == write */
    {
        /* Write gamma ramp to hardware */

        /* If its putting default gamma ramp back, set flag; if its
         * customizing, clear it.
         */
        _FF(default_gamma_flag) = test_gammasig(lpGammaRamp);

        /* Copy to local gamma ramp.
         * We need a local copy of this so we can correctly set the
         * gamma ramp after full screen sessions and the like. We
         * also keep it around to feed back to Windows when asked to
         * read the gamma ramp. Note that the gamma ramp is initially
         * set to linear (init_gammaramp)
         */

        for ( i = 0; i < 256; ++i )
        {
//          It appears that we need to switch the ordering of
//          red and blue.  The HW expects to write the DAC as
//          00rrggbb.  The lpGammaRamp appears to be coming
//          down as 00bbggrr.  So, do the swap here.
            _FF(gamma_ramp).red[i] = lpGammaRamp->blue[i];
            _FF(gamma_ramp).green[i] = lpGammaRamp->green[i];
            _FF(gamma_ramp).blue[i] = lpGammaRamp->red[i];
        }

        /* And send it to the hardware */

        set_gammaramp(0);
    }
    else
    {
        /* Read gamma ramp */

        /* Read from local gamma ramp. I initially tried reading from
         * hardware, but may have done it incorrectly. This problem
         * is corrected by simply passing back what Windows wants!
         */

        for ( i = 0; i < 256; ++i )
        {
            // Since we swap on input...swap RRBB on output
            lpGammaRamp->red[i] = _FF(gamma_ramp).blue[i];
            lpGammaRamp->green[i] = _FF(gamma_ramp).green[i];
            lpGammaRamp->blue[i] = _FF(gamma_ramp).red[i];
        }
        
    }

    return 1;
}


