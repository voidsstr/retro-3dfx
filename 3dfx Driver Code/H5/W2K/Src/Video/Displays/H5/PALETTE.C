/******************************Module*Header*******************************\
* Module Name: palette.c
*
* Palette support.
*
* Copyright (c) 1992-1996 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"

#ifndef USE_DDRAW_CODE
#define SETDW(hwRegister,data)    hwRegister = (data)
#define GET(hwPtr)                hwPtr
#define ghwIO                     ((SstIORegs *)ppdev->pjBase)
#endif
BOOL HWSetPalette ( PDEV *, int, int, PVIDEO_CLUTDATA, GAMMA_STATE );

// Global Table defining the 20 Window default colours.  For 256 colour
// palettes the first 10 must be put at the beginning of the palette
// and the last 10 at the end of the palette.

PALETTEENTRY gapalBase[20] =
{
    { 0,   0,   0,   0 },       // 0
    { 0x80,0,   0,   0 },       // 1
    { 0,   0x80,0,   0 },       // 2
    { 0x80,0x80,0,   0 },       // 3
    { 0,   0,   0x80,0 },       // 4
    { 0x80,0,   0x80,0 },       // 5
    { 0,   0x80,0x80,0 },       // 6
    { 0xC0,0xC0,0xC0,0 },       // 7
    { 192, 220, 192, 0 },       // 8
    { 166, 202, 240, 0 },       // 9
    { 255, 251, 240, 0 },       // 10
    { 160, 160, 164, 0 },       // 11
    { 0x80,0x80,0x80,0 },       // 12
    { 0xFF,0,   0   ,0 },       // 13
    { 0,   0xFF,0   ,0 },       // 14
    { 0xFF,0xFF,0   ,0 },       // 15
    { 0   ,0,   0xFF,0 },       // 16
    { 0xFF,0,   0xFF,0 },       // 17
    { 0,   0xFF,0xFF,0 },       // 18
    { 0xFF,0xFF,0xFF,0 },       // 19
};

/******************************Public*Routine******************************\
* BOOL bInitializePalette
*
* Initializes default palette for PDEV.
*
\**************************************************************************/

BOOL bInitializePalette(
PDEV*    ppdev,
DEVINFO* pdi)
{
    PALETTEENTRY*   ppal;
    PALETTEENTRY*   ppalTmp;
    ULONG           ulLoop;
    BYTE            jRed;
    BYTE            jGre;
    BYTE            jBlu;
    HPALETTE        hpal;
	ULONG			ulReturnedDataLength;
	TDFX_SET_VALUE_INFO		setValueInfo;
	TDFX_QUERY_VALUE_INFO	queryValueInfo;
	PUCHAR			pulTmp;

    // initialize GammaTable
	//
	// Collect the gamma table from the miniport. If it fails,
	// then use the preset table
	//

	// initialize setValueInfo for binary data
	setValueInfo.DataLength = (256 * sizeof(ULONG));
	strcpy(setValueInfo.ValueName, "GammaTable");
	setValueInfo.ValueNameLength = strlen("GammaTable") + 1;
	setValueInfo.Type = REG_BINARY;

	// initialize queryValueInfo for binary data
	queryValueInfo.Type = REG_BINARY;
	queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;

	if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_QUERY_REGISTRY_VALUE,
			(PVOID)"GammaTable",
			strlen("GammaTable") + 1,
			&queryValueInfo,
			sizeof(TDFX_QUERY_VALUE_INFO),
			&ulReturnedDataLength))
	{
		PULONG tmp = (PULONG)queryValueInfo.Data;

		for (ulLoop = 0; ulLoop < 256; ulLoop++)
			ppdev->GammaTable[ulLoop] = tmp[ulLoop];

		/* retro3dfx: reject a degenerate/overbright persisted desktop ramp --
		   the blown out=2*in curve a crashed Glide/overbright app used to
		   leave in the registry maps mid-gray to near-white.  Fall back to
		   identity and re-persist so a box that already has a corrupt
		   GammaTable self-heals on the next boot instead of coming up washed
		   out.  A sane desktop ramp keeps mid-gray near mid-gray. */
		if ((((ppdev->GammaTable[128] >> 8) & 0xFF) >= 0xE0))
		{
			for (ulLoop = 0; ulLoop < 256; ulLoop++)
				ppdev->GammaTable[ulLoop] = (ulLoop << 16) | (ulLoop << 8) | ulLoop;

			setValueInfo.DataLength = (256 * sizeof(ULONG));
			strcpy(setValueInfo.ValueName, "GammaTable");
			setValueInfo.ValueNameLength = strlen("GammaTable") + 1;
			setValueInfo.Type = REG_BINARY;
			memcpy(setValueInfo.Data, ppdev->GammaTable, (256 * sizeof(ULONG)));
			EngDeviceIoControl(ppdev->hDriver,
				IOCTL_3DFX_SET_REGISTRY_VALUE,
				&setValueInfo, sizeof(setValueInfo), NULL, 0, &ulReturnedDataLength);
		}
	}
	else
	{
		// If we end up here, IOCTL failed...
		for (ulLoop = 0; ulLoop < 256; ulLoop++)
			ppdev->GammaTable[ulLoop] = (ulLoop << 16) | (ulLoop << 8) | ulLoop;

		memcpy(setValueInfo.Data, ppdev->GammaTable, (256 * sizeof(ULONG)));

		//
		// Since we couldn't get the table, stash it
		//
		EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_SET_REGISTRY_VALUE,
            &setValueInfo,
			sizeof(setValueInfo),
			NULL,
			0,
			&ulReturnedDataLength);
	}

	// initialize the GlideGammaTable
	//
	// Collect the gamma table from the miniport. If it fails,
	// then use the preset table
	//

	strcpy(setValueInfo.ValueName, "GlideGammaTable");
	setValueInfo.ValueNameLength = strlen("GlideGammaTable") + 1;

	if (!EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_QUERY_REGISTRY_VALUE,
			(PVOID)"GlideGammaTable",
			strlen("GlideGammaTable") + 1,
			&queryValueInfo,
			sizeof(TDFX_QUERY_VALUE_INFO),
			&ulReturnedDataLength))
	{
		PULONG tmp = (PULONG)queryValueInfo.Data;

		for (ulLoop = 0; ulLoop < 256; ulLoop++)
			ppdev->GlideGammaTable[ulLoop] = tmp[ulLoop];
	}
	else
	{
		// If we end up here, IOCTL failed...
		for (ulLoop = 0; ulLoop < 256; ulLoop++)
			ppdev->GlideGammaTable[ulLoop] = (ulLoop << 16) | (ulLoop << 8) | ulLoop;

		memcpy(setValueInfo.Data, ppdev->GlideGammaTable, (256 * sizeof(ULONG)));

		//
		// Since we couldn't get the table, stash it
		//
		EngDeviceIoControl(ppdev->hDriver,
            IOCTL_3DFX_SET_REGISTRY_VALUE,
            &setValueInfo,
			sizeof(setValueInfo),
			NULL,
			0,
			&ulReturnedDataLength);
	}


    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        // Allocate our palette:

        ppal = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(PALETTEENTRY) * 256, ALLOC_TAG, &ppdev->pPal);
        if (ppal == NULL)
            goto ReturnFalse;

        ppdev->pPal = ppal;

        // Generate 256 (8*4*4) RGB combinations to fill the palette

        jRed = 0;
        jGre = 0;
        jBlu = 0;

        ppalTmp = ppal;

        for (ulLoop = 256; ulLoop != 0; ulLoop--)
        {
            ppalTmp->peRed   = jRed;
            ppalTmp->peGreen = jGre;
            ppalTmp->peBlue  = jBlu;
            ppalTmp->peFlags = 0;

            ppalTmp++;

            if (!(jRed += 32))
                if (!(jGre += 32))
                    jBlu += 64;
        }

        // Fill in Windows reserved colours from the WIN 3.0 DDK
        // The Window Manager reserved the first and last 10 colours for
        // painting windows borders and for non-palette managed applications.

        for (ulLoop = 0; ulLoop < 10; ulLoop++)
        {
            // First 10

            ppal[ulLoop]       = gapalBase[ulLoop];

            // Last 10

            ppal[246 + ulLoop] = gapalBase[ulLoop+10];
        }

        // Create handle for palette.

        hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*) ppal, 0, 0, 0);
    }
    else
    {
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) ||
         (ppdev->iBitmapFormat == BMF_24BPP) ||
         (ppdev->iBitmapFormat == BMF_32BPP),
         "This case handles only 16, 24 or 32bpp");

        hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                                ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
    }

    ppdev->hpalDefault = hpal;
    pdi->hpalDefault   = hpal;

    if (hpal == 0)
        goto ReturnFalse;

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializePalette"));
    return(FALSE);
}

/******************************Public*Routine******************************\
* VOID vUninitializePalette
*
* Frees resources allocated by bInitializePalette.
*
* Note: In an error case, this may be called before bInitializePalette.
*
\**************************************************************************/

VOID vUninitializePalette(PDEV* ppdev)
{
    // Delete the default palette if we created one:

    if (ppdev->hpalDefault != 0)
        EngDeletePalette(ppdev->hpalDefault);

    if (ppdev->pPal != (PALETTEENTRY*) NULL)
        ENGFREEMEM(ppdev->pPal);
}

/******************************Public*Routine******************************\
* BOOL bEnablePalette
*
* Initialize the hardware's 8bpp palette registers.
*
\**************************************************************************/

BOOL bEnablePalette(PDEV* ppdev)
{
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       ulReturnedDataLength;
    ULONG       cColors;
    PVIDEO_CLUTDATA pScreenClutData;

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        // Fill in pScreenClut header info:

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

        // Copy colours in:

        cColors = 256;
        pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

        while(cColors--)
        {
            pScreenClutData[cColors].Red =    ppdev->pPal[cColors].peRed;
            pScreenClutData[cColors].Green =  ppdev->pPal[cColors].peGreen;
            pScreenClutData[cColors].Blue =   ppdev->pPal[cColors].peBlue;
            pScreenClutData[cColors].Unused = 0;
        }

        // Set palette registers:
#if 1
    if (! HWSetPalette(ppdev, 0, 256, pScreenClutData, GAMMA_DESKTOP))
#else
    if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SET_COLOR_REGISTERS,
                               pScreenClut,
                               MAX_CLUT_SIZE,
                               NULL,
                               0,
                               &ulReturnedDataLength))
#endif
        {
            DISPDBG((0, "Failed bEnablePalette"));
            return(FALSE);
        }
    }
    else
      // set gamma for 16, 24 & 32bpp modes
      HWSetPalette(ppdev, 0, 256, NULL, GAMMA_DESKTOP);

    DISPDBG((5, "Passed bEnablePalette"));

    return(TRUE);
}

/******************************Public*Routine******************************\
* VOID vDisablePalette
*
* Undoes anything done in bEnablePalette.
*
\**************************************************************************/

VOID vDisablePalette(
PDEV*   ppdev)
{
    // Nothin' to do
}

/******************************Public*Routine******************************\
* VOID vAssertModePalette
*
* Sets/resets the palette in preparation for full-screen/graphics mode.
*
\**************************************************************************/

VOID vAssertModePalette(
PDEV*   ppdev,
BOOL    bEnable)
{
    // USER immediately calls DrvSetPalette after switching out of
    // full-screen, so we don't have to worry about resetting the
    // 8bpp palette here.
    //
    // retro3dfx: BUT for a truecolor (16/24/32bpp) desktop USER does NOT
    // call DrvSetPalette (there is no indexed palette), so nothing used to
    // re-program the DAC gamma bank when returning from a full-screen Glide/
    // OpenGL app -- the desktop was left on the app's gamma ramp (e.g. Quake3
    // overbright) and stayed washed out.  Re-apply the persistent desktop
    // gamma here.  This runs from DrvAssertMode(ENABLE), which the OS calls
    // whenever the GDI desktop is reprogrammed -- including after a game
    // CRASHES -- so the desktop gamma is always restored.
    if (bEnable && (ppdev->iBitmapFormat != BMF_8BPP))
        HWSetPalette(ppdev, 0, 256, NULL, GAMMA_DESKTOP);
}

/******************************Public*Routine******************************\
* BOOL DrvSetPalette
*
* DDI entry point for manipulating the palette.
*
\**************************************************************************/

BOOL DrvSetPalette(
DHPDEV  dhpdev,
PALOBJ* ppalo,
FLONG   fl,
ULONG   iStart,
ULONG   cColors)
{
    BYTE            ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT     pScreenClut;
    PVIDEO_CLUTDATA pScreenClutData;
    PDEV*           ppdev;

    UNREFERENCED_PARAMETER(fl);

	GLIDE_EXCLUSION(glideState[ 0 ]);

    ppdev = (PDEV*) dhpdev;

    // Fill in pScreenClut header info:

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = (USHORT) cColors;
    pScreenClut->FirstEntry = (USHORT) iStart;

    pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

    if (cColors != PALOBJ_cGetColors(ppalo, iStart, cColors,
                                     (ULONG*) pScreenClutData))
    {
        DISPDBG((0, "DrvSetPalette failed PALOBJ_cGetColors"));
        return (FALSE);
    }

#if defined(USE_DDRAW_CODE) && !(defined(H3_A0) || defined(H3_A1))
    HWSetPalette(ppdev, iStart, cColors, pScreenClutData, GAMMA_DESKTOP);
#else
    // Set the high reserved byte in each palette entry to 0.
    // Do the appropriate palette shifting to fit in the DAC.

    if (ppdev->cPaletteShift)
    {
        while(cColors--)
        {
            pScreenClutData[cColors].Red >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Green >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Blue >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Unused = 0;
        }
    }
    else
    {
        while(cColors--)
        {
            pScreenClutData[cColors].Unused = 0;
        }
    }

    // Set palette registers

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_SET_COLOR_REGISTERS,
                           pScreenClut,
                           MAX_CLUT_SIZE,
                           NULL,
                           0,
                           &cColors))
    {
        DISPDBG((0, "DrvSetPalette failed SET_COLOR_REGISTERS"));
        return (FALSE);
    }
#endif

    return(TRUE);

}

#if (_WIN32_WINNT >= 0x0500)
/******************************Public*Routine******************************\
* BOOL DrvIcmSetDeviceGammaRamp
*
* DDI entry point for manipulating the device gamma ramp.
*
* Note that GCAPS2_CHANGEGAMMARAMP has to be set for this to be called.
* Don't set GCAPS2_CHANGEGAMMARAMP when running 8bpp, though!  Bah Humbug!
*
\**************************************************************************/

BOOL DrvIcmSetDeviceGammaRamp(
DHPDEV  dhpdev,
ULONG   iFormat,
PVOID   lpRamp)
{
  PDEV*					ppdev;
  PGAMMARAMP			pGammaRamp;
  ULONG					i;
  TDFX_SET_VALUE_INFO	setValueInfo;
  TDFX_QUERY_VALUE_INFO	queryValueInfo;


  ppdev = (PDEV*) dhpdev;

  pGammaRamp = lpRamp;

  if (iFormat == IGRF_RGB_256WORDS)
  {
    /* retro3dfx gamma-persist guard.  A full-screen Glide/OpenGL app sets its
       gamma through this GDI entry point too (Quake3 with r_overBrightBits
       doubles the ramp -> out = min(255,2*in), a blown 2x-clip curve).  The
       vintage code wrote EVERY such ramp into the persistent desktop
       GammaTable AND saved it to the registry, so once a game exited (or
       crashed) the Windows desktop was left washed out -- permanently, across
       reboots.  Three cases: */

    /* (A) A full-screen app owns the hardware (!bEnabled): apply its ramp live
       as TRANSIENT so in-game gamma works, but never touch the persistent
       desktop GammaTable and never save it.  vAssertModePalette re-asserts
       GAMMA_DESKTOP when the desktop returns (even after a crash). */
    if (! ppdev->bEnabled)
    {
      for (i = 0; i < 256; i++)
      {
        ppdev->TransientGammaTable[i] = ((pGammaRamp->Red[i]   & 0xFF00) << 8) |
                                         (pGammaRamp->Green[i] & 0xFF00)       |
                                        ((pGammaRamp->Blue[i]  & 0xFF00) >> 8);
      }
      return HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_TRANSIENT);
    }

    /* (B) On the GDI desktop, but a blown/overbright ramp (mid-gray maps to
       near-white) is being pushed -- e.g. the OS re-applying a crashed game's
       cached global gamma ramp.  REJECT it: re-assert the clean persistent
       desktop gamma so the desktop can never be washed out, and do not save
       the bad ramp.  A sane desktop ramp keeps mid-gray near mid-gray. */
    if (((ULONG)(pGammaRamp->Green[128] >> 8) & 0xFF) >= 0xE0)
    {
      return HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_DESKTOP);
    }

    /* (C) A sane desktop gamma change: apply it and persist it (normal). */
    for (i = 0; i < 256; i++)
    {
      ppdev->GammaTable[i] = ((pGammaRamp->Red[i]   & 0xFF00) << 8) |
                              (pGammaRamp->Green[i] & 0xFF00)       |
                             ((pGammaRamp->Blue[i]  & 0xFF00) >> 8);
    }

	// initialize setValueInfo for binary data
	setValueInfo.DataLength = (256 * sizeof(ULONG));
	strcpy(setValueInfo.ValueName, "GammaTable");
	setValueInfo.ValueNameLength = strlen("GammaTable") + 1;
	setValueInfo.Type = REG_BINARY;
	memcpy(setValueInfo.Data, ppdev->GammaTable, (256 * sizeof(ULONG)));
	//
	// Save the gamma table in the registry
	//
	if (!EngDeviceIoControl(ppdev->hDriver,
		IOCTL_3DFX_SET_REGISTRY_VALUE,
		&setValueInfo,
		sizeof(setValueInfo),
		NULL,
		0,
		&i))
    {
      DISPDBG((0, "DrvIcmSetDeviceGammaRamp: IOCTL_3DFX_SET_REGISTRY_VALUE okay"));
    }

    return HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_DESKTOP);
  }

  return TRUE;
}
#endif

/******************************Public*Routine******************************\
* BOOL HWSetPalette
*
*
*
\**************************************************************************/

BOOL
HWSetPalette(
	PDEV *ppdev,
	int start,
	int count,
	PVIDEO_CLUTDATA pClutData,
	GAMMA_STATE gammaState
	)
{
	int   i;
#if defined(CSIM)
  SstIORegs *pIORegs = ((SstIORegs *)ppdev->CSIM_pjBase);
#endif

  // we should probably wait for vsync to prevent sparkle
#if defined(CSIM)
#define WAIT_FOR_VSYNC    0
#else
#define WAIT_FOR_VSYNC    1
#endif

	GLIDE_EXCLUSION(glideState[ 0 ]);

#if WAIT_FOR_VSYNC
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
  // Make sure Hsync and Vsync are toggling before we check them.  If monitor is off and TvOut is on they won't be toggling.
  if  (!(GET(ghwIO->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
#endif //def TVOUT_SUPPORTED
  {
    // wait until we're out of vsync
    while (((GET(ghwIO->status) & SST_VRETRACE) ^
          (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
      ;
    // now wait until we're in vsync
    while (! ((GET(ghwIO->status) & SST_VRETRACE) ^
            (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
      ;
  }
  // now the for loop has a full vsync to update all entries
#endif

	for (i = start; i < count; i++)
	{
		DWORD temp;
		DWORD dwRed, dwGreen, dwBlue;


		if (BMF_8BPP == ppdev->iBitmapFormat)
		{
			dwRed   = (DWORD)pClutData[i-start].Red;
			dwGreen = (DWORD)pClutData[i-start].Green;
			dwBlue  = (DWORD)pClutData[i-start].Blue;
			ppdev->pPal[i-start].peRed   = (BYTE)dwRed;
			ppdev->pPal[i-start].peGreen = (BYTE)dwGreen;
			ppdev->pPal[i-start].peBlue  = (BYTE)dwBlue;
		}
		else
		{
			dwRed   = (DWORD)i;
			dwGreen = (DWORD)i;
			dwBlue  = (DWORD)i;
		}

#if WAIT_FOR_VSYNC && 0
    // use the bWaitForVsync arg to decide if we check for vsync
    //if (bWaitForVsync)
#ifdef TVOUT_SUPPORTED
    // Make sure Hsync and Vsync are toggling before we check them.  If monitor is off and TvOut is on they won't be toggling.
    if  (!(GET(ghwIO->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
#endif //def TVOUT_SUPPORTED
    {
      while (! ((GET(ghwIO->status) & SST_VRETRACE) ^
           (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
        ;
    }
#endif

		SETDW(ghwIO->dacAddr, i);
		temp = GET(ghwIO->dacAddr);     // read it back -- hw quirk

#if defined(CSIM)
    pIORegs->dacAddr = i;
    temp = pIORegs->dacAddr;
#endif

		switch (gammaState)	{
			case GAMMA_DESKTOP:
				temp = (ppdev->GammaTable[dwRed]   & 0x00FF0000)
					 | (ppdev->GammaTable[dwGreen] & 0x0000FF00)
					 | (ppdev->GammaTable[dwBlue]  & 0x000000FF);
			break;
			case GAMMA_GLIDE:
				temp = (ppdev->GlideGammaTable[dwRed]   & 0x00FF0000)
					 | (ppdev->GlideGammaTable[dwGreen] & 0x0000FF00)
					 | (ppdev->GlideGammaTable[dwBlue]  & 0x000000FF);
			break;
			case GAMMA_TRANSIENT:
			default:
				temp = (ppdev->TransientGammaTable[dwRed]   & 0x00FF0000)
					 | (ppdev->TransientGammaTable[dwGreen] & 0x0000FF00)
					 | (ppdev->TransientGammaTable[dwBlue]  & 0x000000FF);
			break;
		}

#if WAIT_FOR_VSYNC && 0
    // use the bWaitForVsync arg to decide if we check for vsync
    //if (bWaitForVsync)
#ifdef TVOUT_SUPPORTED
    // Make sure Hsync and Vsync are toggling before we check them.  If monitor is off and TvOut is on they won't be toggling.
    if  (!(GET(ghwIO->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
#endif //def TVOUT_SUPPORTED
    {
      while (! ((GET(ghwIO->status) & SST_VRETRACE) ^
                (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
        ;
    }
#endif

		SETDW(ghwIO->dacData, temp);
#if defined(CSIM)
    pIORegs->dacData = temp;
#endif
	    temp = GET(ghwIO->dacData);     // read it back -- hw quirk
#if defined(CSIM)
      temp = pIORegs->dacData;
#endif
	}

#if (_WIN32_WINNT >= 0x0500)
  // select the lower 256 entries for the desktop clut
  if (GAMMA_DESKTOP == gammaState)
  {
    SETDW(ghwIO->vidProcCfg, (GET(ghwIO->vidProcCfg) & ~SST_DESKTOP_CLUT_SELECT));
#if defined(CSIM)
    pIORegs->vidProcCfg &= ~SST_DESKTOP_CLUT_SELECT;
#endif
  }
#endif

	return TRUE;
}

