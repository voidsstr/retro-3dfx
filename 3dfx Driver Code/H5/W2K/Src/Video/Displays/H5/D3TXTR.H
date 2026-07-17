/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** File name: d3txtr.h
**
** Description: Dynamic and Static Texture allocation/freeing macros
**              Functions for Dynamic and Static Texture alloc/freeing.
**
** $Revision: 12$
** $Date: 10/25/00 4:59:19 AM$
**
**
** $Log: 
**  12   3dfx      1.7.1.1.1.1 10/25/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  11   3dfx      1.7.1.1.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  10   3dfx      1.7.1.1     06/02/00 Russ Lind       modified dynamic memory
**       allocation code so MEMCHECK tracking can be enabled
**  9    3dfx      1.7.1.0     05/09/00 Russ Lind       added PALHNDL_INRANGE macro
**  8    3dfx      1.7         03/22/00 Bob Seitsinger  Added debug statements in
**       FREETEXTUREDESC (w98 only).
**  7    3dfx      1.6         12/15/99 Tim Little      Some stuff for the FXT1 >
**       8:1 aspect texture stuff.  It is disabled for now.
**  6    3dfx      1.5         10/15/99 Russ Lind       removed static array
**       implementation code
**  5    3dfx      1.4         10/04/99 Edwin Wong      Add TXTRHNDL_INRANGE()
**       macro to validate texture handle. Change TXTRHNDL_INUSE() to bitwise AND
**       test.
**  4    3dfx      1.3         09/17/99 Tim Little      Initial changes for support
**       for FXT1 textures, nothing done to handle > 8:1 aspect textures currently.
**  3    3dfx      1.2         09/16/99 Christopher Wilcox Fixed link errors in
**       DirectX 7 build.
** 
**  2    3dfx      1.1         09/14/99 Tim Little      Support for > 8:1 aspect
**       ratio DXTn textures and fix for PRS 8669
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     8/10/99 10:40a Russ
**  for DX7, redefine TXTRHNDL_PTR
**                disable TXTRHNDL functions
**
** 2     6/23/99 9:15p Russ
** DX7 palettized texture changes (#ifdef'd for DX >= 7)
** modified PALETTEGBL macro
**
** 1     6/02/99 6:44a Michael
** Branch from H3
**
** 14    5/30/99 5:56p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 13    5/26/99 11:02a Cwilcox
** Removed D3DSTATIC_ARRAY code.
**
** 12    3/18/99 12:46p Stb_bburton
** Added deferred texture download support.
**
** 11    1/29/99 10:49a Cshaw
** Added unified headers.
**
** 10    12/09/98 6:43a Russ
** NT5 D3D changes for Banshee
**
** 9     12/04/98 9:32a Martin
** new macro to check if a texture is in use.  Fixes bug where they use
** handles for textures that have been deleted.
**
** 8     11/22/98 9:08p Andrew
** Changes to support multi-monitor
**
** 7     10/21/98 8:51p Russ
** make function prototypes portable between win9x and NT5
** change HeapAlloc to DXMALLOCZ
**
** 6     7/24/98 1:37p Hohn
**
** 5     5/06/98 6:10p Adrians
** Changes for DX6 into DX5 driver.
**
** 1     4/29/98 6:31p Adrians
** Created
 *
 * 4     11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#ifndef __D3TXTR_H__
#define __D3TXTR_H__

#include "d3global.h"


//-----------------------------
// dynamic array implementation
//-----------------------------
#define TXTRDESC_INVALID  -1
#define TXTRDESC_ALLOC_ENTRIES_SIZE 1280
#define TXTRDESC_PTR(txtrID) (&_D3(txtrDesc[txtrID]) )
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#define TXTRHNDL_PTR(txtrHN) (pRc->pHndlList->ppTxtrHndlList[(txtrHN)])
#define TXTRHNDL_INRANGE(txtrHN) \
  ((txtrHN) && ((txtrHN) <= ((DWORD)TXTRHNDL_PTR(0))))
#else
#define TXTRHNDL_PTR(txtrHN) (&_D3(txtrHndl[txtrHN]) )
#define TXTRHNDL_INRANGE(txtrHN) ((txtrHN) && ((txtrHN) < MAXTEXTUREHANDLE))
#endif
#define TXTRHNDL_INUSE(txtrHN)  ((TXTRHNDL_PTR(txtrHN)->flags) & HandleInUse)
#define TXTRDESC_FROM_HNDL(txtrHN) (TXTRDESC_PTR((TXTRHNDL_PTR(txtrHN)->txtrID)))

#define MAXTEXTURECOUNT  (_D3(numTextureDescs) )
#define MAXTEXTUREHANDLE (_D3(numTextureHndls) )

// palletized texture tests
#define PALETTIZED(txtrID) (TXTRDESC_PTR(txtrID)->formatFlags & TEXFMTFLG_PALETTIZED)
#define PALETTIZEDHANDLE(txtrHN) (TXTRDESC_FROM_HNDL(txtrHN)->formatFlags & TEXFMTFLG_PALETTIZED)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#define PALETTEGBL(txtrHN)      pRc->pHndlList->ppPalHndlList[TXTRHNDL_PTR(txtrHN)->dwPaletteHandle]
#define PALHNDL_INRANGE(txtrHN) ((TXTRHNDL_PTR(txtrHN)->dwPaletteHandle) && (TXTRHNDL_PTR(txtrHN)->dwPaletteHandle <= (DWORD)pRc->pHndlList->ppPalHndlList[0]))
#else
#define PALETTEGBL(txtrHN) (((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(txtrHN)->surfLcl))->lpDDPalette->lpLcl->lpGbl)
#endif

// external declaration in d3init.c
extern  BOOL DESC_INIT_ARRAY(NT9XDEVICEDATA *ppdev);
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
extern  BOOL HNDL_INIT_ARRAY(NT9XDEVICEDATA *ppdev);
#endif

// external declaration in d3txtr.c
extern void stretchDXTn(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev);
extern void shrinkDXTn(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev);
extern void stretchFXT1(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev);
extern void shrinkFXT1(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev);

extern void decode4bpp_block(int *code, int *pp0, int *pp1, int *pp2, int *pp3);
extern void encode4bpp_block(int *pp0, int *pp1, int *pp2, int *pp3, int *code);


#ifdef FX_DEFINE_MACROS

/*-------------------------------------------------------------------
Function Name:  ALLOCTEXTUREDESC
Description:    allocate texture descriptor. Dynamic array implementation.
Information:    signed int  __stdcall ALLOCTEXTUREDESC(NT9XDEVICEDATA * ppdev,
                                                       TXTRDESC *txtrList)

Return:         signed int - index of allocated texture upon success
                or TXTRDESC_INVALID upon failure
-------------------------------------------------------------------*/
signed int  __stdcall ALLOCTEXTUREDESC(NT9XDEVICEDATA * ppdev, TXTRDESC *txtrList)
{
  DWORD cnt;

  //don't ever use the first entry cause nobody likes a zero handle
  cnt=1;

  while ( (cnt < MAXTEXTURECOUNT) && (txtrList[cnt].flags & InUse))
   ++cnt;

  // out of textures - try and alloc a larger array
  if (cnt == MAXTEXTURECOUNT)
  {
    TXTRDESC *newTxtrList;
    int       size;

    // allocate a larger array. The theory is that there will be an upper limit on
    // the maximum and eventually the array will reach the max and allocation will
    // never occur again.
    size = (MAXTEXTURECOUNT + TXTRDESC_ALLOC_ENTRIES_SIZE) * sizeof(TXTRDESC);
    newTxtrList = (TXTRDESC *) D3DMALLOCZ(size, 0);
    if (!newTxtrList)
      return((signed int)TXTRDESC_INVALID);
    else
    {
        // copy the old data into the new arrays & new entries initialized to zero
        for (cnt=0; cnt < MAXTEXTURECOUNT; ++cnt)
        {
          newTxtrList[cnt] = _D3(txtrDesc[cnt]) ;
        }

        MAXTEXTURECOUNT += TXTRDESC_ALLOC_ENTRIES_SIZE;
        for (; cnt < MAXTEXTURECOUNT; ++cnt)
        {
          newTxtrList[cnt].flags = 0;
        }

        // free old array
        if (_D3(txtrDesc) != NULL)
        {
          D3DFREE(_D3(txtrDesc));
        }

        // new arrays
        _D3(txtrDesc)   = newTxtrList;
        UPDATE_BLOCK_DATA(newTxtrList, &_D3(txtrDesc));

        // has to be a free one now
        cnt = 1;
        while ( (cnt < MAXTEXTURECOUNT) && (newTxtrList[cnt].flags & InUse))
          ++cnt;
      }
  }

  //cnt was incremented at the bottom of last loop, clear all flags
  TXTRDESC_PTR(cnt)->flags = InUse;
#ifdef DEFER_TXTR_DNLD
  TXTRDESC_PTR(cnt)->reload = 0;
  memset (TXTRDESC_PTR(cnt)->checksum, 0, 9 * sizeof(DWORD));
#endif // DEFER_TXTR_DNLD
  return (signed int)cnt;

}


/*-------------------------------------------------------------------
Function Name:  FREETEXTUREDESC
Description:    free texture descriptor. Dynamic array implementation.
Information:    void __stdcall FREETEXTUREDESC(NT9XDEVICEDATA * ppdev, int handle)
Return:         void
            
            
            
-------------------------------------------------------------------*/
void __stdcall FREETEXTUREDESC(NT9XDEVICEDATA * ppdev, int handle)
{
#if defined(DEBUG) && !defined(WINNT)
	if (handle == 0)
	{
		D3DPRINT(0, "FREETEXTUREDESC: handle==0!! flags", TXTRDESC_PTR(handle)->flags);
	}
	else
	{
		D3DPRINT(DLSURFACE, "FREETEXTUREDESC: handle %d, flags %08x",
							handle, TXTRDESC_PTR(handle)->flags);
	}
#endif

  if (handle < (int)MAXTEXTURECOUNT)
    TXTRDESC_PTR(handle)->flags = 0;
}

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
/*-------------------------------------------------------------------
Function Name:  ALLOCTEXTUREHANDLE
Description:    Allocate a texture handle. Dynamic array implementation.
Information:    signed int  __stdcall ALLOCTEXTUREHANDLE(NT9XDEVICEDATA * ppdev,
                                                         TXTRHNDL *txtrList)
Return:         signed int - index of texture handle.
                      or TXTRDESC_INVALID upon failure
            
-------------------------------------------------------------------*/
signed int  __stdcall ALLOCTEXTUREHANDLE(NT9XDEVICEDATA * ppdev, TXTRHNDL *txtrList)
{
  DWORD     cnt;
  TXTRHNDL *txtr;

  //don't ever use the first entry cause nobody likes a zero handle
  cnt=1;

  while ( (cnt < MAXTEXTUREHANDLE) && (txtrList[cnt].flags & HandleInUse))
   ++cnt;

  // out of textures - try and alloc a larger array
  if (cnt == MAXTEXTUREHANDLE)
  {
    TXTRHNDL *newTxtrList;
    int       size;

    // allocate a larger array. The theory is that there will be an upper limit on
    // the maximum and eventually the array will reach the max and allocation will
    // never occur again.
    size = (MAXTEXTUREHANDLE + TXTRDESC_ALLOC_ENTRIES_SIZE) * sizeof(TXTRHNDL);
    newTxtrList = (TXTRHNDL *) D3DMALLOCZ(size, 0);
    if (!newTxtrList)
      return((signed int)TXTRDESC_INVALID);
    else
    {
        // copy the old data into the new arrays & new entries initialized to zero
        for (cnt=0; cnt < MAXTEXTUREHANDLE; ++cnt)
        {
          newTxtrList[cnt] = _D3(txtrHndl[cnt]) ;
        }

        MAXTEXTUREHANDLE += TXTRDESC_ALLOC_ENTRIES_SIZE;
        for (; cnt < MAXTEXTUREHANDLE; ++cnt)
        {
          newTxtrList[cnt].flags = 0;
        }

        // free old array
        if (_D3(txtrHndl) != NULL)
        {
          D3DFREE(_D3(txtrHndl));
        }

        // new arrays
        _D3(txtrHndl) = newTxtrList;
        UPDATE_BLOCK_DATA(newTxtrList, &_D3(txtrHndl));

        // has to be a free one now
        cnt = 1;
        while ( (cnt < MAXTEXTUREHANDLE) && (newTxtrList[cnt].flags & HandleInUse))
          ++cnt;
      }
  }

  //cnt was incremented at the bottom of last loop, clear all flags
  txtr = TXTRHNDL_PTR(cnt);
  txtr->flags = HandleInUse ;
#ifndef WINNT
  txtr->processId = 0 ;
#endif
  txtr->contextId = 0 ;
  return (signed int)cnt;

}

//
/*-------------------------------------------------------------------
Function Name:  FREETEXTUREHANDLE
Description:    free texture handle. Dynamic array implementation.
Information:    void __stdcall FREETEXTUREHANDLE(NT9XDEVICEDATA * ppdev, int handle)
Return:         void
            
            
            
-------------------------------------------------------------------*/
void __stdcall FREETEXTUREHANDLE(NT9XDEVICEDATA * ppdev, int handle)
{
  if (handle < (int)MAXTEXTUREHANDLE)
    TXTRHNDL_PTR(handle)->flags = 0;
}

#ifndef WINNT
//
/*-------------------------------------------------------------------
Function Name:  TXTRFREEHANDLESFORPROCESS
Description:    free texture associated with a specific ProcessID. Dynamic array implementation.
Information:    void  __stdcall TXTRFREEHANDLESFORPROCESS(NT9XDEVICEDATA *ppdev, DWORD pid)
Return:         void
-------------------------------------------------------------------*/
void  __stdcall TXTRFREEHANDLESFORPROCESS(NT9XDEVICEDATA *ppdev, DWORD pid)
{
  int cnt;

  cnt=1;
  while (cnt < (int)MAXTEXTUREHANDLE)
  {
   if (TXTRHNDL_PTR(cnt)->flags & HandleInUse)
   {
      if (TXTRHNDL_PTR(cnt)->processId == pid)
      {
        D3DPRINT( 255, "DestroyHandlesForProcess handle=%d", cnt );
        FREETEXTUREHANDLE(ppdev, cnt);
      }
   }
   ++cnt;
  }
}
#endif

/*-------------------------------------------------------------------
Function Name:  TXTRFREEHANDLESFORCONTEXT
Description:    free texture associated with a specific context. Dynamic array implementation.
Information:    void  __stdcall TXTRFREEHANDLESFORCONTEXT(NT9XDEVICEDATA * ppdev, DWORD context)
Return:         void
-------------------------------------------------------------------*/
void  __stdcall TXTRFREEHANDLESFORCONTEXT(NT9XDEVICEDATA * ppdev, DWORD context)
{
  int cnt;

  cnt=1;
  while (cnt < (int)MAXTEXTUREHANDLE)
  {
   if (TXTRHNDL_PTR(cnt)->flags & HandleInUse)
   {
      if (TXTRHNDL_PTR(cnt)->contextId == context)
      {
        D3DPRINT( 255, "DestroyHandlesForContext handle=%d", cnt );
        FREETEXTUREHANDLE(ppdev, cnt);

      }
   }
   ++cnt;
  }
}
#endif  // DX7

/*-------------------------------------------------------------------
Function Name:  DESC_INIT_ARRAY
Description:    Initialize the texture descriptor array. Dynamic array implementation.
Information:    BOOL DESC_INIT_ARRAY(NT9XDEVICEDATA *ppdev)
Return:         BOOL TRUE - upon success
                     FALSE- upon failure
            
            
-------------------------------------------------------------------*/
BOOL DESC_INIT_ARRAY(NT9XDEVICEDATA *ppdev)
{
    _D3(txtrDesc)
               = (TXTRDESC *) D3DMALLOCZ(TXTRDESC_ALLOC_ENTRIES_SIZE * sizeof(TXTRDESC), &_D3(txtrDesc));
    if (!_D3(txtrDesc) )
      return(FALSE);
    else
    {
      MAXTEXTURECOUNT = TXTRDESC_ALLOC_ENTRIES_SIZE;
      return(TRUE);
    }
}

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
/*-------------------------------------------------------------------
Function Name:  HNDL_INIT_ARRAY
Description:    Initialise texture handle array. Dynamic array implementation.
Information:    BOOL HNDL_INIT_ARRAY(NT9XDEVICEDATA *ppdev)
Return:         BOOL TRUE - upon success
                FALSE- upon failure
-------------------------------------------------------------------*/
BOOL HNDL_INIT_ARRAY(NT9XDEVICEDATA *ppdev)
{
    _D3(txtrHndl)
               = (TXTRHNDL *) D3DMALLOCZ(TXTRDESC_ALLOC_ENTRIES_SIZE * sizeof(TXTRHNDL), &_D3(txtrHndl));
    if (!_D3(txtrHndl))
      return(FALSE);
    else
    {
      MAXTEXTUREHANDLE = TXTRDESC_ALLOC_ENTRIES_SIZE;
      return(TRUE);
    }
}
#endif  // DX7
#endif  // FX_DEFINE_MACROS

#endif //__D3TXTR_H__
