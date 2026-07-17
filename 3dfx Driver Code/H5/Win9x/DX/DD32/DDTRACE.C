/* $Header: ddtrace.c, 2, 10/11/00 8:52:08 PM, Brent$ */
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
** File Name: 	DDTRACE.C
**
** Description: DirectDraw debugging functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:52:08 PM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* Dump_DDSCAPS                   --- Display DDSCAPS surface capabilities.
* Dump_DDCAPS                    --- Display DDCAPS driver capabilities.
* Dump_DDFXALPHACAPS             --- Display DDFXALPHACAPS alpha capabilities.
* Dump_DDFXCAPS                  --- Display DDFXCAPS effects capabilities.
* Dump_DDSD_FLAGS                --- Display DDSD surface description flags.
* Dump_DDRAWIGBL_FLAGS           --- Display DDRAWIGBL driver flags.
* Dump_DDRAWISURFGBL_FLAGS       --- Display DDRAWISURFGBL global surface flags.
* Dump_DDRAWISURFLCL_FLAGS       --- Display DDRAWISURFLCL local surface flags.
* Dump_PIXELFORMAT               --- Display PIXELFORMAT flags and data.
* Dump_RECTL                     --- Display RECTL rectangle data.
* Dump_BLTFX                     --- Display BLTFX effects flags.
* Dump_BLTROFLAGS                --- Display BLT raster operation.
* Dump_BLTFLAGS                  --- Display BLT flags.
* Dump_BLTDATA                   --- Display BLT data.
* Dump_DIRECTDRAW_GBL            --- Display DIRECTDRAW_GBL data structure.
* Dump_DDRAWSURFACE_GBL          --- Display DDRAWSURFACE_GBL global surface data.
* Dump_DDRAWSURFACE_LCL          --- Display DDRAWSURFACE_LCL local surface data.
* Dump_VIDMEM                    --- Display VIDMEM data structure.
* Dump_VIDMEMINFO                --- Display VIDMEMINFO data structure.
* Dump_SURFACEDESC               --- Display DDSURFACEDESC data structure.
* Dump_DDHAL                     --- Display DDHALINFO data structure.
* Dump_LOCK_FLAGS                --- Display DDLOCK flags.
* Dump_LOCKDATA                  --- Display DDLOCK data.
* Dump_WAITFORVERTICALBLANKDATA	 --- Display WAITFORVERTICALBLANK data.
* Dump_GETSCANLINE               --- Display GETSCANLINEDATA structure.
* Dump_DDHAL_GETBLTSTATUSDATA    --- Display GETBLTSTATUSDATA structure.
* Dump_GETFLIPSTATUSDATA         --- Display GETFLIPSTATUSDATA structure.
* Dump_DDCOLORKEY                --- Display DDCOLORKEY structure.
* Dump_DESTROYSURFACEDATA        --- Display DESTROYSURFACEDATA structure.
* Dump_DDHAL_SETMODEDATA         --- Display SETMODEDATA structure.
* Dump_SETSURFACECOLORKEY32      --- Display SEDCOLORKEYDATA structure.
* Dump_DRAWSURFACE_INT           --- Display DDRAWSURFACE_INT structure.
* Dump_DRAWPALETTE_INT           --- Display DDRAWPALETTE_INT structure.
* Dump_DRAWCLIPPER_INT           --- Display DDRAWCLIPPER_INT structure.
* Dump_CREATEPALETTEDATA         --- Display CREATEPALETTEDATA structure.
* Dump_DDRAWPALETTE_GBL          --- Display DDRAWPALETTE_GBL structure.
* Dump_DDRAWIPAL_FLAGS           --- Display DDRAWIPAL flags.
* sststat                        --- Display SST hardware state.
*
* PRIVATE FUNCTIONS:
*
* ddDebugPrint              --- Display a message to the debug terminal.
* Msg                       --- Display a debug message with driver name.
*
*******************************************************************************/

#include "precomp.h"
#include "ddglobal.h"
#include "fxpci.h"

/*----------------------------------------------------------------------
Function name: ddDebugPrint

Description:   Display a message to the debug terminal.

Return:        NONE
----------------------------------------------------------------------*/

void ddDebugPrint( LPSTR szFormat, ...)
{                     
     char buffer[256];

     wvsprintf(buffer, szFormat, (LPVOID)(&szFormat+1));
     OutputDebugString(buffer);
}// ddDebugPrint

#ifdef FXTRACE   // Don't want Dump_XXX in retail 

#ifdef H5
#  define START_STR         "3dfx32v3: "
#else
#  ifdef H4
#    define START_STR       "3dfx32v3: "
#  endif // H4
#endif // H5
#define END_STR         "\r\n"


/*----------------------------------------------------------------------
Function name: Msg

Description:   Display a debug message with driver name.

Return:        NONE
----------------------------------------------------------------------*/

void __cdecl Msg(NT9XDEVICEDATA * ppdev, int debugPrintLevel,  LPSTR szFormat, ... )
{
    char        str[256];

    if (debugPrintLevel <= _DD(DD_DebugLevel)) 
    {
      wsprintf( (LPSTR) str, START_STR );
      wvsprintf( str+lstrlen( str ), szFormat, (LPVOID)(&szFormat+1) );
      lstrcat( (LPSTR) str, END_STR );
      OutputDebugString( str );
    }

} /* Msg */

/*----------------------------------------------------------------------
Function name: Dump_DDSCAPS

Description:   Display DDSCAPS surface capabilities.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDSCAPS(NT9XDEVICEDATA *ppdev, int Level, DWORD dwCaps )
{

  Msg(ppdev,  Level, " surface caps:" );
  
  if (dwCaps & DDSCAPS_3DDEVICE )
    Msg(ppdev,  Level, "DDSCAPS_3DDEVICE ");
    
  if (dwCaps & DDSCAPS_ALPHA )
    Msg(ppdev,  Level, "DDSCAPS_ALPHA ");

  if (dwCaps & DDSCAPS_BACKBUFFER )
    Msg(ppdev,  Level, "DDSCAPS_BACKBUFFER ");

  if (dwCaps & DDSCAPS_COMPLEX )
    Msg(ppdev,  Level, "DDSCAPS_COMPLEX ");

  if (dwCaps & DDSCAPS_FLIP )
    Msg(ppdev,  Level, "DDSCAPS_FLIP ");

  if (dwCaps & DDSCAPS_FRONTBUFFER )
    Msg(ppdev,  Level, "DDSCAPS_FRONTBUFFER ");

  if (dwCaps & DDSCAPS_OFFSCREENPLAIN )
    Msg(ppdev,  Level, "DDSCAPS_OFFSCREENPLAIN ");

  if (dwCaps & DDSCAPS_OVERLAY )
    Msg(ppdev,  Level, "DDSCAPS_OVERLAY ");

  if (dwCaps & DDSCAPS_PALETTE )
    Msg(ppdev,  Level, "DDSCAPS_PALETTE ");

  if (dwCaps & DDSCAPS_PRIMARYSURFACE )
    Msg(ppdev,  Level, "DDSCAPS_PRIMARYSURFACE ");

//  if (dwCaps & DDSCAPS_PRIMARYSURFACELEFT )
//    Msg(ppdev,  Level, "DDSCAPS_PRIMARYSURFACELEFT ");

  if (dwCaps & DDSCAPS_SYSTEMMEMORY )
    Msg(ppdev,  Level, "DDSCAPS_SYSTEMMEMORY ");

  if (dwCaps & DDSCAPS_TEXTURE )

    Msg(ppdev,  Level, "DDSCAPS_TEXTURE ");

  if (dwCaps & DDSCAPS_VIDEOMEMORY )
    Msg(ppdev,  Level, "DDSCAPS_VIDEOMEMORY ");

  if (dwCaps & DDSCAPS_VISIBLE )
    Msg(ppdev,  Level, "DDSCAPS_VISIBLE ");

  if (dwCaps & DDSCAPS_WRITEONLY )
    Msg(ppdev,  Level, "DDSCAPS_WRITEONLY ");

  if (dwCaps & DDSCAPS_ZBUFFER )
    Msg(ppdev,  Level, "DDSCAPS_ZBUFFER ");

  if (dwCaps & DDSCAPS_OWNDC )
    Msg(ppdev,  Level, "DDSCAPS_OWNDC ");

  if (dwCaps & DDSCAPS_LIVEVIDEO )
    Msg(ppdev,  Level, "DDSCAPS_LIVEVIDEO ");

  if (dwCaps & DDSCAPS_HWCODEC )
    Msg(ppdev,  Level, "DDSCAPS_HWCODEC ");

  if (dwCaps & DDSCAPS_MODEX )
    Msg(ppdev,  Level, "DDSCAPS_MODEX ");

  if (dwCaps & DDSCAPS_MIPMAP )
    Msg(ppdev,  Level, "DDSCAPS_MIPMAP ");

}// Dump_DDSCAPS

/*----------------------------------------------------------------------
Function name: Dump_DDCAPS

Description:   Display DDCAPS driver capabilities.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDCAPS(NT9XDEVICEDATA *ppdev, int Level, DWORD dwCaps )
{

  if (dwCaps & DDCAPS_3D )
    Msg(ppdev,  Level, "DDCAPS_3D ");

  if (dwCaps & DDCAPS_ALIGNBOUNDARYDEST )
    Msg(ppdev,  Level, "DDCAPS_ALIGNBOUNDARYDEST ");

  if (dwCaps & DDCAPS_ALIGNSIZEDEST )
    Msg(ppdev,  Level, "DDCAPS_ALIGNSIZEDEST ");

  if (dwCaps & DDCAPS_ALIGNBOUNDARYSRC )
    Msg(ppdev,  Level, "DDCAPS_ALIGNBOUNDARYSRC ");

  if (dwCaps & DDCAPS_ALIGNSIZESRC )
    Msg(ppdev,  Level, "DDCAPS_ALIGNSIZESRC ");

  if (dwCaps & DDCAPS_ALIGNSTRIDE )
    Msg(ppdev,  Level, "DDCAPS_ALIGNSTRIDE ");

  if (dwCaps & DDCAPS_ALPHA )
    Msg(ppdev,  Level, "DDCAPS_ALPHA ");

  if (dwCaps & DDCAPS_BANKSWITCHED )
    Msg(ppdev,  Level, "DDCAPS_BANKSWITHCED ");

  if (dwCaps & DDCAPS_BLT )
    Msg(ppdev,  Level, "DDCAPS_BLT ");

  if (dwCaps & DDCAPS_BLTCOLORFILL )
    Msg(ppdev,  Level, "DDCAPS_BLTCOLORFILL ");

  if (dwCaps & DDCAPS_BLTDEPTHFILL )
    Msg(ppdev,  Level, "DDCAPS_BLTDEPTHFILL ");

  if (dwCaps & DDCAPS_BLTFOURCC )
    Msg(ppdev,  Level, "DDCAPS_BLTFOURCC ");

  if (dwCaps & DDCAPS_BLTQUEUE )
    Msg(ppdev,  Level, "DDCAPS_BLTQUEUE ");

  if (dwCaps & DDCAPS_BLTSTRETCH )
    Msg(ppdev,  Level, "DDCAPS_BLTSTRETCH ");

  if (dwCaps & DDCAPS_CANBLTSYSMEM )
    Msg(ppdev,  Level, "DDCAPS_CANBLTSYSMEM ");

  if (dwCaps & DDCAPS_CANCLIP )
    Msg(ppdev,  Level, "DDCAPS_CANCLIP ");

  if (dwCaps & DDCAPS_CANCLIPSTRETCHED )
    Msg(ppdev,  Level, "DDCAPS_CANCLIPSTRECTCHED ");

  if (dwCaps & DDCAPS_COLORKEY )
    Msg(ppdev,  Level, "DDCAPS_COLORKEY ");

  if (dwCaps & DDCAPS_COLORKEYHWASSIST )
    Msg(ppdev,  Level, "DDCAPS_COLORKEYHWASSIST ");

  if (dwCaps & DDCAPS_GDI )
    Msg(ppdev,  Level, "DDCAPS_GDI ");

  if (dwCaps & DDCAPS_NOHARDWARE )
    Msg(ppdev,  Level, "DDCAPS_NOHARDWARE ");

  if (dwCaps & DDCAPS_OVERLAY )
    Msg(ppdev,  Level, "DDCAPS_OVERLAY ");

  if (dwCaps & DDCAPS_OVERLAYCANTCLIP )
    Msg(ppdev,  Level, "DDCAPS_OVERLAYCANTCLIP ");

  if (dwCaps & DDCAPS_OVERLAYFOURCC )
    Msg(ppdev,  Level, "DDCAPS_OVERLAYFOURCC ");

  if (dwCaps & DDCAPS_OVERLAYSTRETCH )
    Msg(ppdev,  Level, "DDCAPS_OVERLAYSTRETCH ");

  if (dwCaps & DDCAPS_PALETTE )
    Msg(ppdev,  Level, "DDCAPS_PALETTE ");

  if (dwCaps & DDCAPS_PALETTEVSYNC )
    Msg(ppdev,  Level, "DDCAPS_PALETTEVSYNC ");

  if (dwCaps & DDCAPS_READSCANLINE )
    Msg(ppdev,  Level, "DDCAPS_READSCANLINE ");

//  if (dwCaps & DDCAPS_STEREOVIEW )
//    Msg(ppdev,  Level, "DDCAPS_STEREOVIEW ");

  if (dwCaps & DDCAPS_VBI )
    Msg(ppdev,  Level, "DDCAPS_VBI ");

  if (dwCaps & DDCAPS_ZBLTS )
    Msg(ppdev,  Level, "DDCAPS_ZBLTS ");

  if (dwCaps & DDCAPS_ZOVERLAYS )
    Msg(ppdev,  Level, "DDCAPS_ZOVERLAYS ");

}// Dump_DDCAPS

/*----------------------------------------------------------------------
Function name: Dump_DDFXALPHACAPS

Description:   Display DDFXALPHACAPS alpha capabilities.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDFXALPHACAPS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwCaps )
{

  if (dwCaps & DDFXALPHACAPS_BLTALPHAEDGEBLEND )
    Msg(ppdev,  Level, "DDFXALPHACAPS_BLTALPHAEDGEBLEND ");

  if (dwCaps & DDFXALPHACAPS_BLTALPHAPIXELS )
    Msg(ppdev,  Level, "DDFXALPHACAPS_BLTALPHAPIXELS ");

  if (dwCaps & DDFXALPHACAPS_BLTALPHAPIXELSNEG )
    Msg(ppdev,  Level, "DDFXALPHACAPS_BLTALPHAPIXELSNEG ");

  if (dwCaps & DDFXALPHACAPS_BLTALPHASURFACES )
    Msg(ppdev,  Level, "DDFXALPHACAPS_BLTALPHASURFACES ");

  if (dwCaps & DDFXALPHACAPS_BLTALPHASURFACESNEG )
    Msg(ppdev,  Level, "DDFXALPHACAPS_BLTALPHASURFACESNEG ");

  if (dwCaps & DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND )
    Msg(ppdev,  Level, "DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND ");

  if (dwCaps & DDFXALPHACAPS_OVERLAYALPHAPIXELS )
    Msg(ppdev,  Level, "DDFXALPHACAPS_OVERLAYALPHAPIXELS ");

  if (dwCaps & DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG )
    Msg(ppdev,  Level, "DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG ");

  if (dwCaps & DDFXALPHACAPS_OVERLAYALPHASURFACES )
    Msg(ppdev,  Level, "DDFXALPHACAPS_OVERLAYALPHASURFACES ");

  if (dwCaps & DDFXALPHACAPS_OVERLAYALPHASURFACESNEG )
    Msg(ppdev,  Level, "DDFXALPHACAPS_OVERLAYALPHASURFACESNEG ");

}// Dump_DDFXALPHACAPS

/*----------------------------------------------------------------------
Function name: Dump_DDFXCAPS

Description:   Display DDFXCAPS effects capabilities.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDFXCAPS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwCaps )
{
  if (dwCaps & DDFXCAPS_BLTARITHSTRETCHY )
    Msg(ppdev,  Level, "DDFXCAPS_BLTARITHSTRETCHY ");

  if (dwCaps & DDFXCAPS_BLTARITHSTRETCHYN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTARITHSTRETCHYN ");

  if (dwCaps & DDFXCAPS_BLTMIRRORLEFTRIGHT )
    Msg(ppdev,  Level, "DDFXCAPS_BLTMIRRORLEFTRIGHT ");

  if (dwCaps & DDFXCAPS_BLTMIRRORUPDOWN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTMIRRORUPDOWN ");

  if (dwCaps & DDFXCAPS_BLTROTATION )
    Msg(ppdev,  Level, "DDFXCAPS_BLTROTATION ");

  if (dwCaps & DDFXCAPS_BLTROTATION90 )
    Msg(ppdev,  Level, "DDFXCAPS_BLTROTATION90 ");

  if (dwCaps & DDFXCAPS_BLTSHRINKX )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSHRINKX ");

  if (dwCaps & DDFXCAPS_BLTSHRINKXN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSHRINKXN ");

  if (dwCaps & DDFXCAPS_BLTSHRINKY )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSHRINKY ");

  if (dwCaps & DDFXCAPS_BLTSHRINKYN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSHRINKYN ");

  if (dwCaps & DDFXCAPS_BLTSTRETCHX )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSTRETCHX ");

  if (dwCaps & DDFXCAPS_BLTSTRETCHXN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSTRETCHXN ");

  if (dwCaps & DDFXCAPS_BLTSTRETCHY )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSTRETCHY ");

  if (dwCaps & DDFXCAPS_BLTSTRETCHYN )
    Msg(ppdev,  Level, "DDFXCAPS_BLTSTRETCHYN ");

  if (dwCaps & DDFXCAPS_OVERLAYARITHSTRETCHY )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYARITHSTRETCHY ");

  if (dwCaps & DDFXCAPS_OVERLAYARITHSTRETCHYN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYARITHSTRETCHYN ");

  if (dwCaps & DDFXCAPS_OVERLAYSHRINKX )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSHRINKX ");

  if (dwCaps & DDFXCAPS_OVERLAYSHRINKXN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSHRINKXN ");

  if (dwCaps & DDFXCAPS_OVERLAYSHRINKY )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSHRINKY ");

  if (dwCaps & DDFXCAPS_OVERLAYSHRINKYN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSHRINKYN ");

  if (dwCaps & DDFXCAPS_OVERLAYSTRETCHX )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSTRETCHX ");

  if (dwCaps & DDFXCAPS_OVERLAYSTRETCHXN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSTRETCHXN ");

  if (dwCaps & DDFXCAPS_OVERLAYSTRETCHY )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSTRETCHY ");

  if (dwCaps & DDFXCAPS_OVERLAYSTRETCHYN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYSTRETCHYN ");

  if (dwCaps & DDFXCAPS_OVERLAYMIRRORLEFTRIGHT )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYMIRRORLEFTRIGHT ");

  if (dwCaps & DDFXCAPS_OVERLAYMIRRORUPDOWN )
    Msg(ppdev,  Level, "DDFXCAPS_OVERLAYMIRRORUPDOWN ");

}// Dump_DDFXCAPS


/*----------------------------------------------------------------------
Function name: Dump_DDSD_FLAGS

Description:   Display DDSD surface description flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDSD_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags)
{

  Msg(ppdev,  Level,"DDSD flags: %x", dwFlags);

  if (dwFlags & DDSD_CAPS )
    Msg(ppdev,  Level, " DDSD_CAPS ");

  if (dwFlags & DDSD_HEIGHT )
    Msg(ppdev,  Level, " DDSD_HEIGHT ");

  if (dwFlags & DDSD_WIDTH )
    Msg(ppdev,  Level, " DDSD_WIDTH ");

  if (dwFlags & DDSD_PITCH )
    Msg(ppdev,  Level, " DDSD_PITCH ");

  if (dwFlags & DDSD_BACKBUFFERCOUNT )
    Msg(ppdev,  Level, " DDSD_BACKBUFFERCOUNT ");

  if (dwFlags & DDSD_ZBUFFERBITDEPTH )
    Msg(ppdev,  Level, " DDSD_ZBUFFERBITDEPTH ");

  if (dwFlags & DDSD_ALPHABITDEPTH )
    Msg(ppdev,  Level, " DDSD_ALPHABITDEPTH ");

//  if (dwFlags & DDSD_LPSURFACE )
//    Msg(ppdev,  Level, " DDSD_LPSURFACE ");

  if (dwFlags & DDSD_PIXELFORMAT )
    Msg(ppdev,  Level, " DDSD_PIXELFORMAT ");

  if (dwFlags & DDSD_CKDESTOVERLAY )
    Msg(ppdev,  Level, " DDSD_CKDESTOVERLAY ");

  if (dwFlags & DDSD_CKDESTBLT )
    Msg(ppdev,  Level, " DDSD_CKDESTBLT ");

  if (dwFlags & DDSD_CKSRCOVERLAY )
    Msg(ppdev,  Level, " DDSD_CKSRCOVERLAY ");

  if (dwFlags & DDSD_CKSRCBLT )
    Msg(ppdev,  Level, " DDSD_CKSRCBLT ");

  if (dwFlags & DDSD_MIPMAPCOUNT )
    Msg(ppdev,  Level, " DDSD_MIPMAPCOUNT ");

  if (dwFlags & DDSD_REFRESHRATE )
    Msg(ppdev,  Level, " DDSD_REFRESHRATE ");

  if (dwFlags == DDSD_ALL )
    Msg(ppdev,  Level, "  DDSD_ALL ");
}// Dump_DDSD_FLAGS


/*----------------------------------------------------------------------
Function name: Dump_DDRAWIGBL_FLAGS

Description:   Display DDRAWIGBL driver flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWIGBL_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags )
{

  if (dwFlags & DDRAWI_xxxxxxxxx1 )
    Msg(ppdev,  Level, " DDRAWI_xxxxxxxxx1 ");

  if (dwFlags & DDRAWI_xxxxxxxxx2 )
    Msg(ppdev,  Level, " DDRAWI_xxxxxxxxx2 ");
    
  if (dwFlags & DDRAWI_MODEX )
    Msg(ppdev,  Level, " DDRAWI_MODEX ");

  if (dwFlags & DDRAWI_DISPLAYDRV )
    Msg(ppdev,  Level, " DDRAWI_DISPLAYDRV ");

  if (dwFlags & DDRAWI_FULLSCREEN )
    Msg(ppdev,  Level, " DDRAWI_FULLSCREEN ");

  if (dwFlags & DDRAWI_MODECHANGED )
    Msg(ppdev,  Level, " DDRAWI_MODECHANGED ");

  if (dwFlags & DDRAWI_NOHARDWARE )
    Msg(ppdev,  Level, " DDRAWI_NOHARDWARE ");

  if (dwFlags & DDRAWI_PALETTEINIT )
    Msg(ppdev,  Level, " DDRAWI_PALETTEINIT ");

  if (dwFlags & DDRAWI_NOEMULATION )
    Msg(ppdev,  Level, " DDRAWI_NOEMULATION ");

  if (dwFlags & DDRAWI_HASCKEYDESTOVERLAY )
    Msg(ppdev,  Level, " DDRAWI_HASCKEYDESTOVERLAY ");

  if (dwFlags & DDRAWI_HASCKEYSRCOVERLAY )
    Msg(ppdev,  Level, " DDRAWI_HASCKEYSRCOVERLAY ");

  if (dwFlags & DDRAWI_HASGDIPALETTE )
    Msg(ppdev,  Level, " DDRAWI_HASGDIPALETTE ");

  if (dwFlags & DDRAWI_EMULATIONINITIALIZED )
    Msg(ppdev,  Level, " DDRAWI_EMULATIONINITIALIZED ");

  if (dwFlags & DDRAWI_HASGDIPALETTE_EXCLUSIVE )
    Msg(ppdev,  Level, " DDRAWI_HASGDIPALETTE_EXCLUSIVE ");

}// Dump_DDRAWIGPL_FLAGS

/*----------------------------------------------------------------------
Function name: Dump_DDRAWISURFGBL_FLAGS

Description:   Display DDRAWISURFGBL global surface flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWISURFGBL_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags )
{
  if (dwFlags & DDRAWISURFGBL_MEMFREE )
    Msg(ppdev,  Level, " DDRAWISURFGBL_MEMFREE ");
  if (dwFlags & DDRAWISURFGBL_SYSMEMREQUESTED )
    Msg(ppdev,  Level, " DDRAWISURFGBL_SYSMEMREQUESTED ");
  #if BEFOREDDK19
  if (dwFlags & DDRAWISURFGBL_INVALID )
    Msg(ppdev,  Level, " DDRAWISURFGBL_INVALID ");
  #endif
}//Dump_DDRAWISURFGBL_FLAGS

/*----------------------------------------------------------------------
Function name: Dump_DDRAWISURFLCL_FLAGS

Description:   Display DDRAWISURFLCL local surface flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWISURFLCL_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags )
{
  Msg(ppdev,  Level,"DDRAWISURFLCL flags: %x", dwFlags);

  if (dwFlags & DDRAWISURF_ATTACHED )
    Msg(ppdev,  Level, " DDRAWISURF_ATTACHED ");
  if (dwFlags & DDRAWISURF_IMPLICITCREATE )
    Msg(ppdev,  Level, " DDRAWISURF_IMPLICITCREATE ");
  if (dwFlags & DDRAWISURF_ISFREE )
    Msg(ppdev,  Level, " DDRAWISURF_ISFREE ");
  if (dwFlags & DDRAWISURF_ATTACHED_FROM )
    Msg(ppdev,  Level, " DDRAWISURF_ATTACHED_FROM ");
  if (dwFlags & DDRAWISURF_IMPLICITROOT )
    Msg(ppdev,  Level, " DDRAWISURF_IMPLICITROOT ");
  if (dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN )
    Msg(ppdev,  Level, " DDRAWISURF_PARTOFPRIMARYCHAIN ");
  if (dwFlags & DDRAWISURF_DATAISALIASED )
    Msg(ppdev,  Level, " DDRAWISURF_DATAISALIASED ");
  if (dwFlags & DDRAWISURF_HASDC )
    Msg(ppdev,  Level, " DDRAWISURF_HASDC ");
  if (dwFlags & DDRAWISURF_HASCKEYDESTOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_HASCKEYDESTOVERLAY ");
  if (dwFlags & DDRAWISURF_HASCKEYDESTBLT )
    Msg(ppdev,  Level, " DDRAWISURF_HASCKEYDESTBLT ");
  if (dwFlags & DDRAWISURF_HASCKEYSRCOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_HASCKEYSRCOVERLAY ");
  if (dwFlags & DDRAWISURF_HASCKEYSRCBLT )
    Msg(ppdev,  Level, " DDRAWISURF_HASCKEYSRCBLT ");
//  if (dwFlags & DDRAWISURF_xxxxxxxxxxx4 )
//    Msg(ppdev,  Level, " DDRAWISURF_xxxxxxxxxxx4 ");
  if (dwFlags & DDRAWISURF_HASPIXELFORMAT )       
    Msg(ppdev,  Level, " DDRAWISURF_HASPIXELFORMAT ");
  if (dwFlags & DDRAWISURF_HASOVERLAYDATA )
    Msg(ppdev,  Level, " DDRAWISURF_HASOVERLAYDATA ");
#if 0
  if (dwFlags & DDRAWISURF_xxxxxxxxxxx5 )
    Msg(ppdev,  Level, " DDRAWISURF_xxxxxxxxxxx5 ");
#endif
  if (dwFlags & DDRAWISURF_SW_CKEYDESTOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_SW_CKEYDESTOVERLAY ");
  if (dwFlags & DDRAWISURF_SW_CKEYDESTBLT )
    Msg(ppdev,  Level, " DDRAWISURF_SW_CKEYDESTBLT ");
  if (dwFlags & DDRAWISURF_SW_CKEYSRCOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_SW_CKEYSRCOVERLAY ");
  if (dwFlags & DDRAWISURF_SW_CKEYSRCBLT )
    Msg(ppdev,  Level, " DDRAWISURF_SW_CKEYSRCBLT ");
  if (dwFlags & DDRAWISURF_HW_CKEYDESTOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_HW_CKEYDESTOVERLAY ");
  if (dwFlags & DDRAWISURF_HW_CKEYDESTBLT )
    Msg(ppdev,  Level, " DDRAWISURF_HW_CKEYDESTBLT ");
  if (dwFlags & DDRAWISURF_HW_CKEYSRCOVERLAY )
    Msg(ppdev,  Level, " DDRAWISURF_HW_CKEYSRCOVERLAY ");
  if (dwFlags & DDRAWISURF_HW_CKEYSRCBLT )
    Msg(ppdev,  Level, " DDRAWISURF_HW_CKEYSRCBLT ");
#if 0
  if (dwFlags & DDRAWISURF_xxxxxxxxxxx6 )
    Msg(ppdev,  Level, " DDRAWISURF_xxxxxxxxxxx6 ");
#endif
  if (dwFlags & DDRAWISURF_HELCB )
    Msg(ppdev,  Level, " DDRAWISURF_HELCB ");
  if (dwFlags & DDRAWISURF_FRONTBUFFER )
    Msg(ppdev,  Level, " DDRAWISURF_FRONTBUFFER ");
  if (dwFlags & DDRAWISURF_BACKBUFFER )
    Msg(ppdev,  Level, " DDRAWISURF_BACKBUFFER ");
}// Dump_DDRAWISURFLCL_FLAGS

/*----------------------------------------------------------------------
Function name: Dump_PIXELFORMAT

Description:   Display PIXELFORMAT flags and data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_PIXELFORMAT(NT9XDEVICEDATA *ppdev, int Level, DDPIXELFORMAT  ddpf )
{
  Msg(ppdev,  Level,"PIXELFORMAT flags: %x", ddpf.dwFlags);

  if ( ddpf.dwFlags  & DDPF_ALPHAPIXELS )
       Msg(ppdev,  Level, "     DDPF_ALPHAPIXELS" );
  if ( ddpf.dwFlags  & DDPF_ALPHA )
       Msg(ppdev,  Level, "     DDPF_ALPHA" );
  if ( ddpf.dwFlags  & DDPF_FOURCC )
       Msg(ppdev,  Level, "     DDPF_FOURCC" );
  if ( ddpf.dwFlags  & DDPF_PALETTEINDEXED4 )
       Msg(ppdev,  Level, "     DDPF_PALETTEINDEXED4" );
  if ( ddpf.dwFlags  & DDPF_PALETTEINDEXEDTO8 )
       Msg(ppdev,  Level, "     DDPF_PALETTEINDEXEDTO8" );
  if ( ddpf.dwFlags  & DDPF_PALETTEINDEXED8 )
       Msg(ppdev,  Level, "     DDPF_PALETTEINDEXED8" );
  if ( ddpf.dwFlags  & DDPF_RGB )
       Msg(ppdev,  Level, "     DDPF_RGB" );
  if ( ddpf.dwFlags  & DDPF_COMPRESSED )
       Msg(ppdev,  Level, "     DDPF_COMPRESSED" );
  if ( ddpf.dwFlags  & DDPF_RGBTOYUV )
       Msg(ppdev,  Level, "     DDPF_RGBTOYUV" );
  if ( ddpf.dwFlags  & DDPF_YUV )
       Msg(ppdev,  Level, "     DDPF_YUV" );
  if ( ddpf.dwFlags  & DDPF_ZBUFFER )
       Msg(ppdev,  Level, "     DDPF_ZBUFFER" );
  if ( ddpf.dwFlags  & DDPF_PALETTEINDEXED1 )
       Msg(ppdev,  Level, "     DDPF_PALETTEINDEXED1" );
  if ( ddpf.dwFlags  & DDPF_PALETTEINDEXED2 )
       Msg(ppdev,  Level, "     DDPF_PALETTEINDEXED2" );

  if ( ddpf.dwFlags & DDPF_RGB ) 
  {
    Msg(ppdev,  Level, "  dwRGB / Z / Alpha BitCount     =%0lx", ddpf.dwRGBBitCount );
    Msg(ppdev,  Level, "  dwRBitMask        =%0lx", ddpf.dwRBitMask        );
    Msg(ppdev,  Level, "  dwGBitMask        =%0lx", ddpf.dwGBitMask        );
    Msg(ppdev,  Level, "  dwBBitMask        =%0lx", ddpf.dwBBitMask        );
    Msg(ppdev,  Level, "  dwRGBAlphaBitMask =%0lx", ddpf.dwRGBAlphaBitMask );
  }
}// Dump_PIXELFORMAT

/*----------------------------------------------------------------------
Function name: Dump_RECTL

Description:   Display RECTL rectangle data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_RECTL(NT9XDEVICEDATA * ppdev, int Level, LPRECTL rect )
{

    Msg(ppdev,  Level, "(top,left)=(%0lx,%0lx),(bott,right)=(%0lx,%0lx)",
        rect->top, rect->left, rect->bottom, rect->right );
}// Dump_RECTL

/*----------------------------------------------------------------------
Function name: Dump_BLTFX

Description:   Display BLTFX effects flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_BLTFX(NT9XDEVICEDATA * ppdev, int Level, LPDDBLTFX bltFX )
{
    Msg(ppdev,  Level, "bltFX:");

    Msg(ppdev,  Level, "dwSize               %0lx", bltFX->dwSize );
    Msg(ppdev,  Level, "dwDDFX               %0lx", bltFX->dwDDFX );

    if ( bltFX->dwDDFX & DDBLTFX_ARITHSTRETCHY )
      Msg(ppdev,  Level, "      DDBLTFX_ARITHSTRETCHY" );
    if ( bltFX->dwDDFX & DDBLTFX_MIRRORLEFTRIGHT )
      Msg(ppdev,  Level, "      DDBLTFX_MIRRORLEFTRIGHT" );
    if ( bltFX->dwDDFX & DDBLTFX_MIRRORUPDOWN )
      Msg(ppdev,  Level, "      DDBLTFX_MIRRORUPDOWN" );
    if ( bltFX->dwDDFX & DDBLTFX_NOTEARING )
      Msg(ppdev,  Level, "      DDBLTFX_NOTEARING" );
    if ( bltFX->dwDDFX & DDBLTFX_ROTATE180 )
      Msg(ppdev,  Level, "      DDBLTFX_ROTATE180" );
    if ( bltFX->dwDDFX & DDBLTFX_ROTATE270 )
      Msg(ppdev,  Level, "      DDBLTFX_ROTATE270" );
    if ( bltFX->dwDDFX & DDBLTFX_ROTATE90 )
      Msg(ppdev,  Level, "      DDBLTFX_ROTATE90" );
    if ( bltFX->dwDDFX & DDBLTFX_ZBUFFERRANGE )
      Msg(ppdev,  Level, "      DDBLTFX_ZBUFFERRANGE" );
    if ( bltFX->dwDDFX & DDBLTFX_ZBUFFERBASEDEST )
      Msg(ppdev,  Level, "      DDBLTFX_ZBUFFERBASEDEST" );


    Msg(ppdev,  Level, "dwROP                %0lx", bltFX->dwROP );
    if ( bltFX->dwROP  == SRCCOPY )
      Msg(ppdev,  Level, "       SRCCOPY" );
    if ( bltFX->dwROP  == SRCPAINT )
      Msg(ppdev,  Level, "       SRCPAINT" );
    if ( bltFX->dwROP  == SRCAND )
      Msg(ppdev,  Level, "       SRCAND" );
    if ( bltFX->dwROP  == SRCINVERT )
      Msg(ppdev,  Level, "       SRCINVERT" );
    if ( bltFX->dwROP  == SRCERASE )
      Msg(ppdev,  Level, "       SRCERASE" );
    if ( bltFX->dwROP  == NOTSRCCOPY )
      Msg(ppdev,  Level, "       NOTSRCCOPY" );
    if ( bltFX->dwROP  == NOTSRCERASE )
      Msg(ppdev,  Level, "       NOTSRCERASE" );
    if ( bltFX->dwROP  == MERGECOPY )
      Msg(ppdev,  Level, "       MERGECOPY" );
    if ( bltFX->dwROP  == MERGEPAINT )
      Msg(ppdev,  Level, "       MERGEPAINT" );
    if ( bltFX->dwROP  == PATCOPY )
      Msg(ppdev,  Level, "       PATCOPY" );
    if ( bltFX->dwROP  == PATPAINT )
      Msg(ppdev,  Level, "       PATPAINT" );
    if ( bltFX->dwROP  == PATINVERT )
      Msg(ppdev,  Level, "       PATINVERT" );
    if ( bltFX->dwROP  == DSTINVERT )
      Msg(ppdev,  Level, "       DSTINVERT" );
    if ( bltFX->dwROP  == BLACKNESS )
      Msg(ppdev,  Level, "       BLACKNESS" );
    if ( bltFX->dwROP  == WHITENESS )
      Msg(ppdev,  Level, "       WHITENESS" );

    Msg(ppdev,  Level, "dwDDROP              %0lx", bltFX->dwDDROP );
    if ( bltFX->dwDDROP == 0x1000 )
      Msg(ppdev,  Level, "      WHITENESS" );
    if ( bltFX->dwDDROP == 0x1 )
      Msg(ppdev,  Level, "      BLACKNESS" );
    if ( bltFX->dwDDROP == 0x80000000 )
      Msg(ppdev,  Level, "      SRCCOPY" );


    Msg(ppdev,  Level, "dwRotationAngle      %0lx", bltFX->dwRotationAngle );


    Msg(ppdev,  Level, "dwZBufferOpCode      %0lx", bltFX->dwZBufferOpCode );

    if ( bltFX->dwDDFX & DDBLTFX_ZBUFFERRANGE )
    {
      Msg(ppdev,  Level, "dwZBufferLow         %0lx", bltFX->dwZBufferLow );
      Msg(ppdev,  Level, "dwZBufferHigh        %0lx", bltFX->dwZBufferHigh );
    }

    if ( bltFX->dwDDFX & DDBLTFX_ZBUFFERBASEDEST )
      Msg(ppdev,  Level, "dwZBufferBaseDest    %0lx", bltFX->dwZBufferBaseDest );

    Msg(ppdev,  Level, "dwZDestConstBitDepth %0lx", bltFX->dwZDestConstBitDepth );
    Msg(ppdev,  Level, "dwZDestConst or lpDDSZBufferDest %0lx", 
                                       bltFX->dwZDestConstBitDepth );
    Msg(ppdev,  Level, "dwZSrcConstBitDepth   %0lx",bltFX->dwZSrcConstBitDepth );
    Msg(ppdev,  Level, "dwZSrcConst or lpDDSZBufferSrc %0lx", bltFX->dwZSrcConst );


    Msg(ppdev,  Level, "dwAlphaEdgeBlendBitDepth %0lx", bltFX->dwAlphaEdgeBlendBitDepth);
    Msg(ppdev,  Level, "dwAlphaEdgeBlend         %0lx", bltFX->dwAlphaEdgeBlend);
    Msg(ppdev,  Level, "dwReserved               %0lx", bltFX->dwReserved);
    Msg(ppdev,  Level, "dwAlphaDestConstBitDepth %0lx", bltFX->dwAlphaDestConstBitDepth);
    Msg(ppdev,  Level, "dwAlphaDestConst or lpDDSAlphaDest %0lx", bltFX->dwAlphaDestConst);
    Msg(ppdev,  Level, "dwAlphaSrcConstBitDepth  %0lx", bltFX->dwAlphaSrcConstBitDepth);
    Msg(ppdev,  Level, "dwAlphaSrcConst or lpDDSAlphaSrc %0lx", bltFX->dwAlphaSrcConst);


    Msg(ppdev,  Level, "dwFillColor or FillDepth or lpDDSPattern %0lx", 
                                       bltFX->dwFillColor);
    Msg(ppdev,  Level, "DestColorKey:");
    DUMP_DDCOLORKEY(ppdev, Level, bltFX->ddckDestColorkey );
    Msg(ppdev,  Level, "SrcColorKey:");
    DUMP_DDCOLORKEY(ppdev, Level, bltFX->ddckSrcColorkey );
}// Dump_BLTFX

/*----------------------------------------------------------------------
Function name: Dump_BLTROFLAGS

Description:   Display BLT raster operation.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_BLTROPFLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwROPFlags )
{
    Msg(ppdev,  Level, "blt rop flag: %0lx", dwROPFlags );
}// Dump_BLTROFLAGS

/*----------------------------------------------------------------------
Function name: Dump_BLTFLAGS

Description:   Display BLT flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_BLTFLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags )
{

  Msg(ppdev,  Level, "blt flags:" );
  if ( dwFlags & DDBLT_ALPHADEST  )                
     Msg(ppdev,  Level,"  DDBLT_ALPHADEST ");              
  if ( dwFlags & DDBLT_ALPHADESTCONSTOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_ALPHADESTCONSTOVERRIDE ");  
  if ( dwFlags & DDBLT_ALPHADESTNEG  ) 
     Msg(ppdev,  Level,"  DDBLT_ALPHADESTNEG ");            
  if ( dwFlags & DDBLT_ALPHADESTSURFACEOVERRIDE  )
     Msg(ppdev,  Level,"  DDBLT_ALPHADESTSURFACEOVERRIDE ");
  if ( dwFlags & DDBLT_ALPHAEDGEBLEND  )
     Msg(ppdev,  Level,"  DDBLT_ALPHAEDGEBLEND ");          
  if ( dwFlags & DDBLT_ALPHASRC  ) 
     Msg(ppdev,  Level,"  DDBLT_ALPHASRC ");                
  if ( dwFlags & DDBLT_ALPHASRCCONSTOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_ALPHASRCCONSTOVERRIDE ");   
  if ( dwFlags & DDBLT_ALPHASRCNEG  )      
     Msg(ppdev,  Level,"  DDBLT_ALPHASRCNEG ");             
  if ( dwFlags & DDBLT_ALPHASRCSURFACEOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_ALPHASRCSURFACEOVERRIDE "); 
  if ( dwFlags & DDBLT_ASYNC  )    
     Msg(ppdev,  Level,"  DDBLT_ASYNC ");                   
  if ( dwFlags & DDBLT_COLORFILL  ) 
     Msg(ppdev,  Level,"  DDBLT_COLORFILL ");               
  if ( dwFlags & DDBLT_DDFX  )                  
     Msg(ppdev,  Level,"  DDBLT_DDFX ");                    
  if ( dwFlags & DDBLT_DDROPS  ) 
     Msg(ppdev,  Level,"  DDBLT_DDROPS ");                  
  if ( dwFlags & DDBLT_KEYDEST  ) 
     Msg(ppdev,  Level,"  DDBLT_KEYDEST ");                 
  if ( dwFlags & DDBLT_KEYDESTOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_KEYDESTOVERRIDE ");         
  if ( dwFlags & DDBLT_KEYSRC  )            
     Msg(ppdev,  Level,"  DDBLT_KEYSRC ");                  
  if ( dwFlags & DDBLT_KEYSRCOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_KEYSRCOVERRIDE ");          
  if ( dwFlags & DDBLT_ROP  )             
     Msg(ppdev,  Level,"  DDBLT_ROP ");                     
  if ( dwFlags & DDBLT_ROTATIONANGLE  ) 
     Msg(ppdev,  Level,"  DDBLT_ROTATIONANGLE ");           
  if ( dwFlags & DDBLT_ZBUFFER  )              
     Msg(ppdev,  Level,"  DDBLT_ZBUFFER ");                 
  if ( dwFlags & DDBLT_ZBUFFERDESTCONSTOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_ZBUFFERDESTCONSTOVERRIDE ");
  if ( dwFlags & DDBLT_ZBUFFERDESTOVERRIDE  )   
     Msg(ppdev,  Level,"  DDBLT_ZBUFFERDESTOVERRIDE ");     
  if ( dwFlags & DDBLT_ZBUFFERSRCCONSTOVERRIDE  ) 
     Msg(ppdev,  Level,"  DDBLT_ZBUFFERSRCCONSTOVERRIDE "); 
  if ( dwFlags & DDBLT_ZBUFFERSRCOVERRIDE  )    
     Msg(ppdev,  Level,"  DDBLT_ZBUFFERSRCOVERRIDE ");      
  if ( dwFlags & DDBLT_WAIT  )         
     Msg(ppdev,  Level,"  DDBLT_WAIT ");                    
  if ( dwFlags & DDBLT_DEPTHFILL )         
     Msg(ppdev,  Level,"  DDBLT_DEPTHFILL ");                    
}// Dump_BLTFLAGS

/*----------------------------------------------------------------------
Function name: Dump_BLTDATA

Description:   Display BLT data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_BLTDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_BLTDATA pbd  )
{
  if ( pbd->lpDD != NULL )
     DUMP_DIRECTDRAW_GBL(ppdev, Level, pbd->lpDD );

  if ( pbd->lpDDDestSurface != NULL)
  {
     Msg(ppdev,  Level, "Destination Surface:" );
     DUMP_DDRAWSURFACE_LCL(ppdev, Level, pbd->lpDDDestSurface );
     DUMP_RECTL(ppdev, Level, &pbd->rDest );
  }
  if ( pbd->lpDDSrcSurface != NULL)
  {
     Msg(ppdev,  Level, "Source Surface:" );
     DUMP_DDRAWSURFACE_LCL(ppdev, Level, pbd->lpDDSrcSurface );
     DUMP_RECTL(ppdev, Level, &pbd->rSrc );
  }
  if ( pbd->dwFlags )             
     DUMP_BLTFLAGS(ppdev, Level, pbd->dwFlags );

  if ( pbd->dwFlags & DDBLT_ROP )
     DUMP_BLTROPFLAGS(ppdev, Level, pbd->dwROPFlags );

  if ( &pbd->bltFX != NULL )
     DUMP_BLTFX(ppdev, Level, &pbd->bltFX );

}// Dump_BLTDATA

/*----------------------------------------------------------------------
Function name:  Dump_DIRECTDRAW_GBL

Description:    Display DIRECTDRAW_GBL data structure.
			  
Return:         NONE
----------------------------------------------------------------------*/

void DUMP_DIRECTDRAW_GBL(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DIRECTDRAW_GBL lpDD )
{
    Msg(ppdev,  Level, "DDRAWI_DIRECTDRAW_GBL:");

    Msg(ppdev,  Level, "dwRefCnt :              %0lx", lpDD->dwRefCnt );
    Msg(ppdev,  Level, "dwFlags:                %0lx", lpDD->dwFlags );
      DUMP_DDRAWIGBL_FLAGS(ppdev, Level, lpDD->dwFlags );

    Msg(ppdev,  Level, "fpPrimaryOrig:          %0lx", lpDD->fpPrimaryOrig );
    Msg(ppdev,  Level, "ddCaps:                 %0lx", lpDD->ddCaps );
    Msg(ppdev,  Level, "dsList:                 %0lx", lpDD->dsList);
      DUMP_DRAWSURFACE_INT(ppdev, Level, lpDD->dsList );

    Msg(ppdev,  Level, "palList:                %0lx", lpDD->palList);
      DUMP_DRAWPALETTE_INT(ppdev, Level, lpDD->palList );

    Msg(ppdev,  Level, "clipperList:            %0lx", lpDD->clipperList);
      DUMP_DRAWCLIPPER_INT(ppdev, Level, lpDD->clipperList );

    // LPDDRAWI_DIRECTDRAW_GBL     lp16DD;         // PRIVATE: 16-bit ptr to this struct
    // DWORD                       dwMaxOverlays;  // maximum number of overlays
    // DWORD                       dwCurrOverlays; // current number of visible overlays

    Msg(ppdev,  Level, "dwMonitorFrequency:    %0lx", lpDD->dwMonitorFrequency );
    Msg(ppdev,  Level, "ddHELCaps:             %0lx", lpDD->ddHELCaps );

    // DWORD                       dwUnused2[50];  // not currently used
    // DDCOLORKEY                  ddckCKDestOverlay; // color key for destination overlay use
    // DDCOLORKEY                  ddckCKSrcOverlay; // color key for source overlay use

    Msg(ppdev,  Level, "VIDMEMINFO:");
       DUMP_VIDMEMINFO(ppdev, Level, &lpDD->vmiData );

    // LPVOID                      lpDriverHandle; // handle for use by display driver to call fns in DDRAW16.DLL
    // LPDDRAWI_DIRECTDRAW_LCL     lpExclusiveOwner;   // PRIVATE: exclusive local object

    Msg(ppdev,  Level, "dwModeIndex:            %0lx", lpDD->dwModeIndex );
    Msg(ppdev,  Level, "dwModeIndexOrig:        %0lx", lpDD->dwModeIndexOrig );
    Msg(ppdev,  Level, "dwNumfourCC:            %0lx", lpDD->dwNumFourCC );

    // DWORD                       FAR *lpdwFourCC;// PRIVATE: fourcc codes supported

    Msg(ppdev,  Level, "dwNumModes:             %0lx", lpDD->dwNumModes );

    // LPDDHALMODEINFO             lpModeInfo;     // PRIVATE: mode information
    // PROCESS_LIST                plProcessList;  // PRIVATE: list of processes using driver
    // DWORD                       dwSurfaceLockCount; // total number of outstanding locks
    // DWORD                       dwFree1;        // PRIVATE: was system color table
    // DWORD                       dwFree2;        // PRIVATE: was original palette
    // DWORD                       hDD;            // PRIVATE: NT Kernel-mode handle (was dwFree3).
    // char                        cDriverName[MAX_DRIVER_NAME]; // Driver Name
    // DWORD                       dwReserved1;    // reserved for use by display driver
    // DWORD                       dwReserved2;    // reserved for use by display driver
    // DBLNODE                     dbnOverlayRoot; // The root node of the doubly-
    //                                             // linked list of overlay z orders.
    // volatile LPWORD             lpwPDeviceFlags;// driver physical device flags
    // DWORD                       dwPDevice;      // driver physical device (16:16 pointer)
    // DWORD                       dwWin16LockCnt; // count on win16 holds
    // LPDDRAWI_DIRECTDRAW_LCL     lpWin16LockOwner;   // object owning Win16 Lock
    // DWORD                       hInstance;      // instance handle of driver
    // DWORD                       dwEvent16;      // 16-bit event
    // DWORD                       dwSaveNumModes; // saved number of modes supported
    // /*  Version 2 fields */
    // DWORD                       lpD3DGlobalDriverData;  // Global D3D Data
    // DWORD                       lpD3DHALCallbacks;      // D3D HAL Callbacks
    // DDCAPS                      ddBothCaps;     // logical AND of driver and HEL caps
}// Dump_DIRECTDRAW_GBL 

/*----------------------------------------------------------------------
Function name:  Dump_DDRAWSURFACE_GBL

Description:    Display DDRAWSURFACE_GBL global surface data.

Return:         NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWSURFACE_GBL(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWSURFACE_GBL lpDDSG )
{

    Msg(ppdev,  Level, "dwRefCnt               %0lx", lpDDSG->dwRefCnt );
    Msg(ppdev,  Level, "dwGlobalFlags          %0lx", lpDDSG->dwGlobalFlags );
      DUMP_DDRAWISURFGBL_FLAGS(ppdev, Level, lpDDSG->dwGlobalFlags );
    Msg(ppdev,  Level, "dwBlockSizeY OR lpRectList   %0lx", lpDDSG->dwBlockSizeY );
    Msg(ppdev,  Level, "dwBlockSizeX OR lpVidMemHeap %0lx", lpDDSG->dwBlockSizeX );

#if 0
    union
    {
        LPDDRAWI_DIRECTDRAW_GBL lpDD; 
        LPVOID                  lpDDHandle;
    };
#endif

    Msg(ppdev,  Level, "fpVidMem               %0lx", lpDDSG->fpVidMem );
    Msg(ppdev,  Level, "lPitch or dwLinearSize %0lx", lpDDSG->lPitch );
    Msg(ppdev,  Level, "wHeight                %0lx", lpDDSG->wHeight );
    Msg(ppdev,  Level, "wWidth                 %0lx", lpDDSG->wWidth );
    Msg(ppdev,  Level, "dwUsageCount           %0lx", lpDDSG->dwUsageCount );
    Msg(ppdev,  Level, "dwReserved1            %0lx", lpDDSG->dwReserved1 );

    //
    // NOTE: this part of the structure is ONLY allocated if the pixel
    //       format differs from that of the primary display
    //

    // if ( &lpDDSG->ddpfSurface != NULL )
    // {
    //  Msg(ppdev,  Level, "---not qualified that pixel format differs from primary---");
    //  Dump_PIXELFORMAT( Level, lpDDSG->ddpfSurface );
    // }
}// Dump_DDRAWSURFACE_GBL


/*----------------------------------------------------------------------
Function name:  Dump_DDRAWSURFACE_LCL

Description:    Display DDRAWSURFACE_LCL local surface data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWSURFACE_LCL(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWSURFACE_LCL lpDDSL )
{
    DWORD caps;

    DUMP_DDRAWSURFACE_GBL(ppdev, Level, lpDDSL->lpGbl );

    if (lpDDSL->dwFlags & DDRAWISURF_HASPIXELFORMAT)
       DUMP_PIXELFORMAT(ppdev, Level, lpDDSL->lpGbl->ddpfSurface );

#if 0
    struct _DDRAWI_DDRAWSURFACE_LCL     FAR *lpLink;
    LPATTACHLIST                        lpAttachList;
    LPATTACHLIST                        lpAttachListFrom;
    DWORD                               dwLocalRefCnt;
    DWORD                               dwProcessId;
#endif

    Msg(ppdev,  Level, "dwFlags               %0lx", lpDDSL->dwFlags );
      DUMP_DDRAWISURFLCL_FLAGS(ppdev, Level, lpDDSL->dwFlags );

    Msg(ppdev,  Level, "ddsCaps               %0lx", lpDDSL->ddsCaps );

    caps = lpDDSL->ddsCaps.dwCaps;
      DUMP_DDSCAPS(ppdev,  Level, caps );

#if 0
    union
    {
        LPDDRAWI_DDRAWPALETTE_LCL       lpDDPalette;
        LPDDRAWI_DDRAWPALETTE_LCL       lp16DDPalette;
    };
    union
    {
        LPDDRAWI_DDRAWCLIPPER_LCL       lpDDClipper;
        LPDDRAWI_DDRAWCLIPPER_LCL       lp16DDClipper;
    };
    DWORD                               dwModeCreatedIn;
    DWORD                               dwBackBufferCount;
    DDCOLORKEY                          ddckCKDestBlt;
    DDCOLORKEY                          ddckCKSrcBlt;
    DWORD                               hDC;
    DWORD                               dwReserved1; 
    DDCOLORKEY                          ddckCKSrcOverlay;
    DDCOLORKEY                          ddckCKDestOverlay;
    LPDDRAWI_DDRAWSURFACE_LCL           lpSurfaceOverlaying;
    DBLNODE                             dbnOverlayNode; 
    RECT                                rcOverlaySrc;
    RECT                                rcOverlayDest;
    DWORD                               dwClrXparent;
    DWORD                               dwAlpha;     
    LONG                                lOverlayX;   
    LONG                                lOverlayY;   
#endif
}// Dump_DDRAWSURFACE_LCL

/*----------------------------------------------------------------------
Function name:  Dump_VIDMEM

Description:    Display VIDMEM data structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_VIDMEM(NT9XDEVICEDATA *ppdev, int Level, LPVIDMEM lpVidMem )
{
    DWORD caps;

    Msg(ppdev,  Level, "dwFlags               %0lx", lpVidMem->dwFlags );
    Msg(ppdev,  Level, "fpStart               %0lx", lpVidMem->fpStart );
    Msg(ppdev,  Level, "fpEnd OR dwWidth      %0lx", lpVidMem->fpEnd );
    Msg(ppdev,  Level, "PASS 1: This memory CANNOT be used for:" );
    caps = lpVidMem->ddsCaps.dwCaps;
      DUMP_DDSCAPS(ppdev,  Level, caps );
    Msg(ppdev,  Level, "PASS 2: This memory CANNOT be used for:" );
    caps = lpVidMem->ddsCapsAlt.dwCaps;
      DUMP_DDSCAPS(ppdev,  Level, caps );
    Msg(ppdev,  Level, "lpHeap OR dwHeight    %0lx", lpVidMem->lpHeap );
}// Dump_VIDMEM 

/*----------------------------------------------------------------------
Function name:  Dump_VIDMEMINFO

Description:    Display VIDMEMOINFO data structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_VIDMEMINFO(NT9XDEVICEDATA *ppdev, int Level, LPVIDMEMINFO lpVidMemInfo )
{
    Msg(ppdev,  Level, "fpPrimary               %0lx", lpVidMemInfo->fpPrimary );
    Msg(ppdev,  Level, "dwFlags                 %0lx", lpVidMemInfo->dwFlags );
    Msg(ppdev,  Level, "dwDisplayWidth          %0lx", lpVidMemInfo->dwDisplayWidth );
    Msg(ppdev,  Level, "dwDisplayHeight         %0lx", lpVidMemInfo->dwDisplayHeight );
    Msg(ppdev,  Level, "lDisplayPitch           %0lx", lpVidMemInfo->lDisplayPitch );

    DUMP_PIXELFORMAT(ppdev, Level, lpVidMemInfo->ddpfDisplay );

    Msg(ppdev,  Level, "dwOffscreenAlign        %0lx", lpVidMemInfo->dwOffscreenAlign );
    Msg(ppdev,  Level, "dwOverlayAlign          %0lx", lpVidMemInfo->dwOverlayAlign );
    Msg(ppdev,  Level, "dwTextureAlign          %0lx", lpVidMemInfo->dwTextureAlign );
    Msg(ppdev,  Level, "dwZBufferAlign          %0lx", lpVidMemInfo->dwZBufferAlign );
    Msg(ppdev,  Level, "dwAlphaAlign            %0lx", lpVidMemInfo->dwAlphaAlign );
    Msg(ppdev,  Level, "dwNumHeaps              %0lx", lpVidMemInfo->dwNumHeaps );

#if 0
    LPVIDMEM            pvmList;                // array of heaps
#endif
}// Dump_VIDMEMINFO

/*----------------------------------------------------------------------
Function name: Dump_SURFACEDESC

Description:   Display DDSURFACEDESC data structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_SURFACEDESC(NT9XDEVICEDATA *ppdev, int Level, LPDDSURFACEDESC lpSD )
{
  DWORD  caps;

  Msg(ppdev,  Level, "dwSize                %0lx", lpSD->dwSize );
  Msg(ppdev,  Level, "dwFlags               %0lx", lpSD->dwFlags );
    DUMP_DDSD_FLAGS(ppdev, Level, lpSD->dwFlags );
  Msg(ppdev,  Level, "dwHeight              %0lx", lpSD->dwHeight );
  Msg(ppdev,  Level, "dwWidth               %0lx", lpSD->dwWidth );
  Msg(ppdev,  Level, "lPitch                %0lx", lpSD->lPitch );
  Msg(ppdev,  Level, "dwBackBufferCount or MipMapCount %0lx", lpSD->dwBackBufferCount );
  Msg(ppdev,  Level, "dwZBufferBitDepth or RefreshRate %0lx", lpSD->dwZBufferBitDepth );
  Msg(ppdev,  Level, "dwAlphaBitDepth       %0lx", lpSD->dwAlphaBitDepth );
  Msg(ppdev,  Level, "dwReserved            %0lx", lpSD->dwReserved );
  Msg(ppdev,  Level, "lpSurface             %0lx", lpSD->lpSurface );

  Msg(ppdev,  Level, "ddckCKDestOverlay:");
  DUMP_DDCOLORKEY(ppdev, Level, lpSD->ddckCKDestOverlay );
  Msg(ppdev,  Level, "ddckCKDestBlt:");
  DUMP_DDCOLORKEY(ppdev, Level, lpSD->ddckCKDestBlt );
  Msg(ppdev,  Level, "ddckCKSrcOverlay:");
  DUMP_DDCOLORKEY(ppdev, Level, lpSD->ddckCKSrcOverlay );
  Msg(ppdev,  Level, "ddckCKSrcBlt:");
  DUMP_DDCOLORKEY(ppdev, Level, lpSD->ddckCKSrcBlt );

  DUMP_PIXELFORMAT(ppdev, Level, lpSD->ddpfPixelFormat );
  caps = lpSD->ddsCaps.dwCaps;
    DUMP_DDSCAPS(ppdev,  Level, caps );
}// Dump_SURFACEDESC

/*----------------------------------------------------------------------
Function name: Dump_DDHAL

Description:   Display DDHALINFO data structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDHAL(NT9XDEVICEDATA * ppdev, int Level, LPDDHALINFO lpHI )
{
  DWORD caps;

  Msg(ppdev,  Level, "--DDHALINFO--" );
  Msg(ppdev,  Level, "dwSize                %0lx", lpHI->dwSize );
  Msg(ppdev,  Level, "ddCaps                      " );
  caps = lpHI->ddCaps.dwCaps;
    DUMP_DDCAPS(ppdev,  Level, caps );
}//Dump_DDHAL

/*----------------------------------------------------------------------
Function name: Dump_LOCK_FLAGS

Description:   Display DDLOCK flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_LOCK_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags)
{
  Msg(ppdev,  Level," lock flags: %x", dwFlags);

  if (dwFlags & DDLOCK_WAIT )
    Msg(ppdev,  Level, " DDLOCK_WAIT ");

  if (dwFlags & DDLOCK_READONLY )
    Msg(ppdev,  Level, " DDLOCK_READONLY ");

  if (dwFlags & DDLOCK_WRITEONLY )
    Msg(ppdev,  Level, " DDLOCK_WRITEONLY ");

  //?? Not sure wether these belong here

  if (dwFlags & DDLOCK_SURFACEMEMORYPTR )
    Msg(ppdev,  Level, "    ? DDLOCK_SURFACEMEMORYPTR ");

  if (dwFlags & DDLOCK_EVENT )
    Msg(ppdev,  Level, "    ? DDLOCK_EVENT ");
}// Dump_LOCK_FLAGS

/*----------------------------------------------------------------------
Function name: Dump_LOCKDATA

Description:   Display DDLOCK data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_LOCKDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_LOCKDATA lpLockData )
{
    Msg(ppdev,  Level, "--LocKData--" );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpLockData->lpDD );
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, lpLockData->lpDDSurface );
    if (lpLockData->bHasRect)
    {
      Msg(ppdev,  Level, "--rArea is valid--" );
      DUMP_RECTL(ppdev, Level, &lpLockData->rArea );
    }
    else
      Msg(ppdev,  Level, "--rArea is NOT valid--" );

    Msg(ppdev,  Level, "lpSurfData: %x", lpLockData->lpSurfData);

    // LPDDHALSURFCB_LOCK          Lock;  // PRIVATE: ptr to callback

    DUMP_LOCK_FLAGS(ppdev, Level, lpLockData->dwFlags);
}// Dump_LOCKDATA

/*----------------------------------------------------------------------
Function name: Dump_WAITFORVERTICALBLANKDATA

Description:   Display WAITFORVERTICALBLANK data.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_WAITFORVERTICALBLANKDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_WAITFORVERTICALBLANKDATA lpVB )
{
    Msg(ppdev,  Level, "--WaitForVerticalBlankData--" );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpVB->lpDD );

    Msg(ppdev,  Level," Flags: %x", lpVB->dwFlags);

    if( lpVB->dwFlags & DDWAITVB_I_TESTVB )
       Msg(ppdev,  Level, "      DDWAITVB_I_TESTVB ");

    if( lpVB->dwFlags & DDWAITVB_BLOCKBEGIN )
       Msg(ppdev,  Level, "      DDWAITVB_BLOCKBEGIN ");

    if( lpVB->dwFlags & DDWAITVB_BLOCKBEGINEVENT )
       Msg(ppdev,  Level, "      DDWAITVB_BLOCKBEGINEVENT ");

    if( lpVB->dwFlags & DDWAITVB_BLOCKEND )
       Msg(ppdev,  Level, "      DDWAITVB_BLOCKEND ");

    Msg(ppdev,  Level, "bIsInVB: %d", lpVB->bIsInVB );
    Msg(ppdev,  Level, "hEvent: %d", lpVB->hEvent );
}//  Dump_WAITFORVERTICALBLANKDATA

/*----------------------------------------------------------------------
Function name: Dump_GETSCANLINE

Description:   Display GETSCANLINEDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_GETSCANLINE(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_GETSCANLINEDATA lpGS )
{
    Msg(ppdev,  Level, "--GetScanLine--" );
    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpGS->lpDD );
    Msg(ppdev,  Level, "dwScanLine: %d", lpGS->dwScanLine);
}// Dump_GETSCANLINE

/*----------------------------------------------------------------------
Function name: Dump_DDHAL_GETBLTSTATUSDATA

Description:   Display GETBLTSTATUSDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDHAL_GETBLTSTATUSDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_GETBLTSTATUSDATA lpGBS  )
{
    Msg(ppdev,  Level, "--GetBltStatusData--" );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpGBS->lpDD );
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, lpGBS->lpDDSurface );

    if( lpGBS->dwFlags & DDGBS_CANBLT )
       Msg(ppdev,  Level, " DDGBS_CANBLT ");

    if( lpGBS->dwFlags & DDGBS_ISBLTDONE )
       Msg(ppdev,  Level, " DDGBS_ISBLTDONE ");
}// Dump_DDHAL_GETBLTSTATUSDATA

/*----------------------------------------------------------------------
Function name: Dump_GETFLIPSTATUSDATA

Description:   Display GETFLIPSTATUSDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_GETFLIPSTATUSDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_GETFLIPSTATUSDATA lpGFS )
{
    Msg(ppdev,  Level, "--FlipStatusData--" );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpGFS->lpDD );
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, lpGFS->lpDDSurface );

    if( lpGFS->dwFlags & DDGFS_CANFLIP )
       Msg(ppdev,  Level, " DDGFS_CANFLIP ");

    if( lpGFS->dwFlags & DDGFS_ISFLIPDONE )
       Msg(ppdev,  Level, " DDGFS_ISFLIPDONE ");
}// Dump_GETFLIPSTATUSDATA

/*----------------------------------------------------------------------
Function name: Dump_DDHAL_FLIPDATA

Description:   Display FLIPDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDHAL_FLIPDATA(NT9XDEVICEDATA *ppdev, int Level, LPDDHAL_FLIPDATA pfd )
{
    Msg(ppdev,  Level, "--FlipData--" );
    DUMP_DIRECTDRAW_GBL(ppdev, Level, pfd->lpDD );
    Msg(ppdev,  Level, "current surface");
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, pfd->lpSurfCurr );
    Msg(ppdev,  Level, "target surface (to flip to)");
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, pfd->lpSurfTarg );

    if( pfd->dwFlags & DDFLIP_WAIT )
      Msg(ppdev,  Level, "      DDFLIP_WAIT ");
}// Dump_DDHAL_FLIPDATA

/*----------------------------------------------------------------------
Function name: Dump_DDCOLORKEY

Description:   Display DDCOLORKEY structure.

Return:        NONE
----------------------------------------------------------------------*/
 
void DUMP_DDCOLORKEY(NT9XDEVICEDATA *ppdev, int Level, DDCOLORKEY ck )
{
    Msg(ppdev,  Level, "--ColorKey--" );
    Msg(ppdev,  Level,"dwColorSpaceLowValue:  %0lx", ck.dwColorSpaceLowValue);
    Msg(ppdev,  Level,"dwColorSpaceHighValue: %0lx", ck.dwColorSpaceHighValue);
} // Dump_DDCOLORKEY

/*----------------------------------------------------------------------
Function name: Dump_DESTROYSURFACEDATA

Description:   Display DESTROYSURFACEDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDHAL_DESTROYSURFACEDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_DESTROYSURFACEDATA pdsd )
{
    Msg(ppdev,  Level, "--DestroySurfaceData--" );
    DUMP_DIRECTDRAW_GBL(ppdev, Level, pdsd->lpDD );
    Msg(ppdev,  Level, "destroy surface struct");
    DUMP_DDRAWSURFACE_LCL(ppdev, Level, pdsd->lpDDSurface );
}// Dump_DESTROYSURFACEDATA

/*----------------------------------------------------------------------
Function name: Dump_DDHAL_SETMODEDATA

Description:   Display SETMODEDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDHAL_SETMODEDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_SETMODEDATA psmd )
{
    Msg(ppdev,  Level, "--SetModeData--" );
    Msg(ppdev,  Level, "  dwModeIndex: %0lx", psmd->dwModeIndex );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, psmd->lpDD );
}// Dump_DDHAL_SETMODEDATA

/*----------------------------------------------------------------------
Function name: Dump_SETSURFACECOLORKEY32

Description:   Display SEDCOLORKEYDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_SETSURFACECOLORKEY32(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_SETCOLORKEYDATA pssck )
{
    Msg(ppdev,  Level,"--SetSurfaceColorKey--" );
    Msg(ppdev,  Level,"dwFlags: %0lx", pssck->dwFlags );
    Msg(ppdev,  Level,"dwColorSpaceLowValue:  %0lx", pssck->ckNew.dwColorSpaceLowValue);
    Msg(ppdev,  Level,"dwColorSpaceHighValue: %0lx", pssck->ckNew.dwColorSpaceHighValue);
} // Dump_SETSURFACECOLORKEY32

/*----------------------------------------------------------------------
Function name: Dump_DRAWSURFACE_INT

Description:   Display DDRAWSURFACE_INT structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DRAWSURFACE_INT(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWSURFACE_INT lpSI )
{
    if( lpSI )
    {
      Msg(ppdev,  Level, "DRAWI_DRAWSURFACE_INT:");
      Msg(ppdev,  Level, "lpVtbl                %0lx", lpSI->lpVtbl);
      Msg(ppdev,  Level, "lpLcl                 %0lx", lpSI->lpLcl);
      Msg(ppdev,  Level, "lpLink                %0lx", lpSI->lpLink);
      Msg(ppdev,  Level, "dwIntRefCnt           %0lx", lpSI->dwIntRefCnt);
    }
    else
    {
      Msg(ppdev,  Level, "DRAWI_DRAWSURFACE_INT: NULL");
    }
}// Dump_DRAWSURFACE_INT

/*----------------------------------------------------------------------
Function name: Dump_DRAWPALETTE_INT

Description:   Display DDRAWPALETTE_INT structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DRAWPALETTE_INT(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWPALETTE_INT lpPI )
{
    if( lpPI )
    {
      Msg(ppdev,  Level, "DRAWI_DRAWPALETTE_INT:");
      Msg(ppdev,  Level, "lpVtbl                %0lx", lpPI->lpVtbl);
      Msg(ppdev,  Level, "lpLcl                 %0lx", lpPI->lpLcl);
      Msg(ppdev,  Level, "lpLink                %0lx", lpPI->lpLink);
      Msg(ppdev,  Level, "dwIntRefCnt           %0lx", lpPI->dwIntRefCnt);
    }
    else
    {
      Msg(ppdev,  Level, "DRAWI_DRAWPALETTE_INT: NULL");
    }
}// Dump_DRAWPALETTE_INT

/*----------------------------------------------------------------------
Function name: Dump_DRAWCLIPPER_INT

Description:   Display DDRAWCLIPPER_INT structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DRAWCLIPPER_INT(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWCLIPPER_INT lpCI )
{
    if( lpCI )
    {
      Msg(ppdev,  Level, "DRAWI_DRAWCLIPPER_INT:");
      Msg(ppdev,  Level, "lpVtbl                %0lx", lpCI->lpVtbl);
      Msg(ppdev,  Level, "lpLcl                 %0lx", lpCI->lpLcl);
      Msg(ppdev,  Level, "lpLink                %0lx", lpCI->lpLink);
      Msg(ppdev,  Level, "dwIntRefCnt           %0lx", lpCI->dwIntRefCnt);
    }
    else
    {
      Msg(ppdev,  Level, "DRAWI_DRAWCLIPPER_INT: NULL");
    }
}// Dump_DRAWCLIPPER_INT

/*----------------------------------------------------------------------
Function name: Dump_CREATEPALETTEDATA

Description:   Display CREATEPALETTEDATA structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_CREATEPALETTEDATA(NT9XDEVICEDATA * ppdev, int Level, LPDDHAL_CREATEPALETTEDATA lpCpData )
{
    Msg(ppdev,  Level, "--Create Palette Data--" );

    DUMP_DIRECTDRAW_GBL(ppdev, Level, lpCpData->lpDD );

    DUMP_DDRAWPALETTE_GBL(ppdev, Level, lpCpData->lpDDPalette );

    Msg(ppdev,  Level, "lpColorTable           %0lx", lpCpData->lpColorTable);

    if( lpCpData->is_excl )
       Msg(ppdev,  Level, " Process HAS exclusive mode");
    else
       Msg(ppdev,  Level, " Process HAS NOT exclusive mode");
}// Dump_CREATEPALETTEDATA

/*----------------------------------------------------------------------
Function name: Dump_DDRAWPALETTE_GBL

Description:   Display DDRAWPALETTE_GBL structure.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWPALETTE_GBL(NT9XDEVICEDATA * ppdev, int Level, LPDDRAWI_DDRAWPALETTE_GBL lpDpGbl )
{
    Msg(ppdev,  Level, "--DDrawPalette Gbl Data--" );
    Msg(ppdev,  Level, "dwRefCnt           %0lx", lpDpGbl->dwRefCnt);

    DUMP_DDRAWIPAL_FLAGS(ppdev, Level, lpDpGbl->dwFlags );

    // LPDDRAWI_DIRECTDRAW_LCL lpDD_lcl

    Msg(ppdev,  Level, "dwProcessId        %0lx", lpDpGbl->dwProcessId);
    Msg(ppdev,  Level, "lpColorTable       %0lx", lpDpGbl->lpColorTable);
}// Dump_DDRAWPALETTE_GBL

/*----------------------------------------------------------------------
Function name: Dump_DDRAWIPAL_FLAGS

Description:   Display DDRAWIPAL flags.

Return:        NONE
----------------------------------------------------------------------*/

void DUMP_DDRAWIPAL_FLAGS(NT9XDEVICEDATA * ppdev, int Level, DWORD dwFlags)
{
  Msg(ppdev,  Level," ddrawipal flags: %x", dwFlags);

  if (dwFlags & DDRAWIPAL_256 )
    Msg(ppdev,  Level, " DDRAWIPAL_256 ");

  if (dwFlags & DDRAWIPAL_16 )
    Msg(ppdev,  Level, " DDRAWIPAL_16 ");

  if (dwFlags & DDRAWIPAL_GDI )
    Msg(ppdev,  Level, " DDRAWIPAL_GDI ");

  if (dwFlags & DDRAWIPAL_STORED_8 )
    Msg(ppdev,  Level, " DDRAWIPAL_STORED_8 ");

  if (dwFlags & DDRAWIPAL_STORED_16 )
    Msg(ppdev,  Level, " DDRAWIPAL_STORED_16 ");

  if (dwFlags & DDRAWIPAL_STORED_24 )
    Msg(ppdev,  Level, " DDRAWIPAL_STORED_24 ");

  if (dwFlags & DDRAWIPAL_EXCLUSIVE )
    Msg(ppdev,  Level, " DDRAWIPAL_EXCLUSIVE ");

  if (dwFlags & DDRAWIPAL_INHEL )
    Msg(ppdev,  Level, " DDRAWIPAL_INHEL ");

  if (dwFlags & DDRAWIPAL_DIRTY )
    Msg(ppdev,  Level, " DDRAWIPAL_DIRTY ");

  if (dwFlags & DDRAWIPAL_ALLOW256 )
    Msg(ppdev,  Level, " DDRAWIPAL_ALLOW256 ");

  if (dwFlags & DDRAWIPAL_4 )
    Msg(ppdev,  Level, " DDRAWIPAL_4 ");

  if (dwFlags & DDRAWIPAL_2 )
    Msg(ppdev,  Level, " DDRAWIPAL_2 ");

  if (dwFlags & DDRAWIPAL_STORED_8INDEX )
    Msg(ppdev,  Level, " DDRAWIPAL_STORED_8INDEX ");
}// Dump_DDRAWIPAL_FLAGS



#endif   // FXTRACE

char *_rgblanes_str[] = {"ARGB", "ABGR", "RGBA", "BGRA"};

char *_lfbformat_str[] = { "565",  "555", "1555", "**3*",
			   "888", "8888", "**6*", "**7*",
			  "**8*", "**9*", "*10*", "*11*",
			  "Z565", "Z555", "Z1555","ZZ"};
/*----------------------------------------------------------------------
Function name: sststat

Description:   Display SST hardware state.

Return:        NONE
----------------------------------------------------------------------*/

void SSTSTAT (NT9XDEVICEDATA * ppdev, DWORD *sstbase)
{

    int verbose = 0;
    int Level = _DD(DD_DebugLevel);

    FxU32 x; //deviceNumber;

    SstRegs *sst;

    verbose = 1;

    sst = (SstRegs *) sstbase;

    if (sst == NULL) {
	   Msg(ppdev,  Level,"error: could not find 3Dfx card.");
	   return;
    }

#define OFFSET(a,b) (((int)(&a->b)) - (int)a)
                                                      
    Msg(ppdev,  Level,"  Register Name      Data  Address");
    Msg(ppdev,  Level,"---------------  -------- --------");

    //--------------------------------------------------
    x = sst->status;
   Msg(ppdev,  Level,"    status: %08x      %3x",x,OFFSET(sst,status));
    if (verbose) {
	Msg(ppdev,  Level,"\t\t       %02x : pci fifo free space (%d)",
				x&SST_FIFOLEVEL,x&SST_FIFOLEVEL);
	Msg(ppdev,  Level,"\t\t\t%d : vertical retrace",(x&SST_VRETRACE)!=0);
	Msg(ppdev,  Level,"\t\t\t%d : fbi busy",(x&SST_FBI_BUSY)!=0);
	Msg(ppdev,  Level,"\t\t\t%d : tmu busy",(x&SST_TREX_BUSY)!=0);
	Msg(ppdev,  Level,"\t\t\t%d : sst busy",(x&SST_BUSY)!=0);
  #if 0
	Msg(ppdev,  Level,"\t\t\t%d : displayed buffer",
				(x&SST_DISPLAYED_BUFFER)>>SST_DISPLAYED_BUFFER_SHIFT);
	Msg(ppdev,  Level,"\t\t     %Level4x : mem fifo free space (%d)",
				(x&SST_MEMFIFOLEVEL)>>SST_MEMFIFOLEVEL_SHIFT,
				(x&SST_MEMFIFOLEVEL)>>SST_MEMFIFOLEVEL_SHIFT);
  #endif
	Msg(ppdev,  Level,"\t\t\t%d : swap buffers pending",
				(x&SST_SWAPBUFPENDING)>>SST_SWAPBUFPENDING_SHIFT);
    }
    //--------------------------------------------------
    x = sst->fbzColorPath;
    Msg(ppdev,  Level,"   fbzColorPath: %08x   %3x",x,OFFSET(sst,fbzColorPath));
    //--------------------------------------------------
    x = sst->fogMode;
    Msg(ppdev,  Level,"   fogMode:      %08x   %3x",x,OFFSET(sst,fogMode));
    if (verbose) {
	if (x & SST_ENFOGGING	)	Msg(ppdev,  Level,"\t\t\t  : ENFOGGING");
	if (x & SST_FOGADD	)	Msg(ppdev,  Level,"\t\t\t  : FOGADD");
	if (x & SST_FOGMULT	)	Msg(ppdev,  Level,"\t\t\t  : FOGMULT");
	if (x & SST_FOG_ALPHA	)	Msg(ppdev,  Level,"\t\t\t  : FOG_ALPHA");
	if (x & SST_FOG_Z	)	Msg(ppdev,  Level,"\t\t\t  : FOG_Z");
	if (x & SST_FOG_CONSTANT)	Msg(ppdev,  Level,"\t\t\t  : FOG_CONSTANT");
    }
    //--------------------------------------------------
    x = sst->alphaMode;
    Msg(ppdev,  Level," alphaMode:      %08x   %3x",x,OFFSET(sst,alphaMode));
    //--------------------------------------------------
    x = sst->fbzMode;
    Msg(ppdev,  Level,"   fbzMode:      %08x   %3x",x,OFFSET(sst,fbzMode));
    if (verbose) {
	if (x & SST_ENRECTCLIP	)	Msg(ppdev,  Level,"\t\t\t  : ENRECTCLIP");
	if (x & SST_ENCHROMAKEY	)	Msg(ppdev,  Level,"\t\t\t  : ENCHROMAKEY");
	if (x & SST_ENSTIPPLE	)	Msg(ppdev,  Level,"\t\t\t  : ENSTIPPLE");
	if (x & SST_WBUFFER	)	Msg(ppdev,  Level,"\t\t\t  : WBUFFER");
	if (x & SST_ENDEPTHBUFFER)	Msg(ppdev,  Level,"\t\t\t  : ENDEPTHBUFFER");
	Msg(ppdev,  Level,"\t\t      %s%s%s : zfunction",
		x & SST_ZFUNC_LT ? "<" : " ",
		x & SST_ZFUNC_GT ? ">" : " ",
		x & SST_ZFUNC_EQ ? "=" : " ");
	if (x & SST_ENDITHER	)	Msg(ppdev,  Level,"\t\t\t  : ENDITHER");
	if (x & SST_RGBWRMASK	)	Msg(ppdev,  Level,"\t\t\t  : RGBWRMASK");
	if (x & SST_ZAWRMASK	)	Msg(ppdev,  Level,"\t\t\t  : ZAWRMASK");
	if (x & SST_DITHER2x2	)	Msg(ppdev,  Level,"\t\t\t  : DITHER2x2");
	if (x & SST_ENSTIPPLEPATTERN)	Msg(ppdev,  Level,"\t\t\t  : ENSTIPPLEPATTERN");
	if (x & SST_ENALPHAMASK	)	Msg(ppdev,  Level,"\t\t\t  : ENALPHAMASK");
  #if 0
	Msg(ppdev,  Level,"\t\t\t%d : drawbuffer (Level=front, 1=back)",
		(x & SST_DRAWBUFFER)>>SST_DRAWBUFFER_SHIFT);
  #endif
	if (x & SST_ENZBIAS	)	Msg(ppdev,  Level,"\t\t\t  : ENZBIAS");
	if (x & SST_YORIGIN	)	Msg(ppdev,  Level,"\t\t\t  : YORIGIN");
	if (x & SST_ENALPHABUFFER)	Msg(ppdev,  Level,"\t\t\t  : ENALPHABUFFER");
	if (x & SST_ENDITHERSUBTRACT)	Msg(ppdev,  Level,"\t\t\t  : ENDITHERSUBTRACT");
    }
    //--------------------------------------------------
    x = sst->lfbMode;
    Msg(ppdev,  Level,"   lfbMode:     %08x  %3x",x,OFFSET(sst,lfbMode));
    if (verbose) {
	Msg(ppdev,  Level,"\t\t    %5s : lfb format",
		_lfbformat_str[(x & SST_LFB_FORMAT)>>SST_LFB_FORMAT_SHIFT]);
  #if 0
	Msg(ppdev,  Level,"\t\t\t%d : writebuffer (Level=front, 1=back, 2=aux)",
		(x & SST_LFB_WRITEBUFSELECT)>>SST_LFB_WRITEBUFSELECT_SHIFT);
  #endif
	Msg(ppdev,  Level,"\t\t\t%d : readbuffer (Level=front, 1=back, 2=aux)",
		(x & SST_LFB_READBUFSELECT)>>SST_LFB_READBUFSELECT_SHIFT);
	Msg(ppdev,  Level,"\t\t     %4s : rgba lanes",
		_rgblanes_str[(x & SST_LFB_RGBALANES)>>SST_LFB_RGBALANES_SHIFT]);
	if (x & SST_LFB_ENPIXPIPE	)    Msg(ppdev,  Level,"\t\t\t  : ENPIXPIPE");
	if (x & SST_LFB_WRITE_SWAP16	) Msg(ppdev,  Level,"\t\t\t  : WRITE_SWAP16");
	if (x & SST_LFB_WRITE_BYTESWAP) Msg(ppdev,  Level,"\t\t\t  : WRITE_BYTESWAP");
	if (x & SST_LFB_YORIGIN		   ) Msg(ppdev,  Level,"\t\t\t  : YORIGIN");
	if (x & SST_LFB_WSELECT		   ) Msg(ppdev,  Level,"\t\t\t  : WSELECT");
	if (x & SST_LFB_READ_SWAP16	) Msg(ppdev,  Level,"\t\t\t  : READ_SWAP16	");
	if (x & SST_LFB_READ_BYTESWAP	) Msg(ppdev,  Level,"\t\t\t  : READ_BYTESWAP");
    }
    //--------------------------------------------------
    x = sst->clipLeftRight;
    Msg(ppdev,  Level,"  clipLeftRight: %08x %3x",x,OFFSET(sst,clipLeftRight));
    x = sst->clipBottomTop;
    Msg(ppdev,  Level,"  clipBottomTop: %08x %3x",x,OFFSET(sst,clipBottomTop));
    x = sst->stipple;
    Msg(ppdev,  Level,"   stipple:      %08x %3x",x,OFFSET(sst,stipple));
    x = sst->c0;
    Msg(ppdev,  Level,"        c0:      %08x %3x",x,OFFSET(sst,c0));
    x = sst->c1;
    Msg(ppdev,  Level,"        c1:      %08x %3x",x,OFFSET(sst,c1));
    x = sst->stats.fbiPixelsIn;
    Msg(ppdev,  Level,"    fbiPixelsIn: %08x %3x",x,OFFSET(sst,stats.fbiPixelsIn));
    x = sst->stats.fbiChromaFail;
    Msg(ppdev,  Level,"  fbiChromaFail: %08x %3x",x,OFFSET(sst,stats.fbiChromaFail));
    x = sst->stats.fbiZfuncFail;
    Msg(ppdev,  Level,"   fbiZfuncFail: %08x %3x",x,OFFSET(sst,stats.fbiZfuncFail));
    x = sst->stats.fbiAfuncFail;
    Msg(ppdev,  Level,"   fbiAfuncFail: %08x %3x",x,OFFSET(sst,stats.fbiAfuncFail));
    x = sst->stats.fbiPixelsOut;
    Msg(ppdev,  Level,"   fbiPixelsOut: %08x %3x",x,OFFSET(sst,stats.fbiPixelsOut));
    #if 0
    x = sst->fbiInit4;
    Msg(ppdev,  Level,"  fbiInit4:      %08x %3x",x,OFFSET(sst,fbiInit4));
    x = sst->vRetrace;
    Msg(ppdev,  Level,"  vRetrace:      %08x %3x",x,OFFSET(sst,vRetrace));
    x = sst->backPorch;
    Msg(ppdev,  Level," backPorch:      %08x %3x",x,OFFSET(sst,backPorch));
    x = sst->videoDimensions;
    Msg(ppdev,  Level,"videoDimensions: %08x %3x",x,OFFSET(sst,videoDimensions));
    x = sst->fbiInit0;
    Msg(ppdev,  Level,"  fbiInit0: %08x      %3x",x,OFFSET(sst,fbiInit0));
    x = sst->fbiInit1;
    Msg(ppdev,  Level,"  fbiInit1: %08x      %3x",x,OFFSET(sst,fbiInit1));
    x = sst->fbiInit2;
    Msg(ppdev,  Level,"  fbiInit2: %08x      %3x",x,OFFSET(sst,fbiInit2));
    x = sst->fbiInit3;
    Msg(ppdev,  Level,"  fbiInit3: %08x      %3x",x,OFFSET(sst,fbiInit3));
#endif
}// sststat
