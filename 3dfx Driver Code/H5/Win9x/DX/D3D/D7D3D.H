/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** $Revision: 13$
** $Date: 10/26/00 7:58:06 AM$
**
** $Log:
*/

#ifndef __D7D3D_H__
#define __D7D3D_H__

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)


//-----------------------------------------------------------------------------
//                       DX7 Texture management definitions
//-----------------------------------------------------------------------------

typedef struct _PALHNDL
{
  DWORD         dwFlags;
  WORD          wStartIndex;
  WORD          wNumEntries;
  PALETTEENTRY  ColorTable[256];    // array of palette entries, could be dynamic later
} PALHNDL, *PPALHNDL;

// This value will cause us to grow our arrays for surface handles, palette handles and
// state blocks in 4k blocks (or full pages on intel cpu's)
#define LISTGROWSIZE    1024

typedef struct _HNDLLIST
{
#if HL_LINKED_LIST
  struct _HNDLLIST  *pNext;           // this must be the first element in struct!
#endif

  LPVOID    pDDLcl;                   // owning ddraw pointer as a key
  struct _TXTRHNDL  **ppTxtrHndlList; // array of TXTRHNDL ptrs
                                      // the array is dynamically allocated
                                      // and the index is the surface handle
                                      // since handle=0 is not used, then
                                      // ppTxtrHndlList[0] is used to hold the
                                      // number of entries in ppTxtrHndlList
                                      // array, if allocated
  PALHNDL   **ppPalHndlList;          // array of PALHNDL ptrs
                                      // the array is dynamically allocated
                                      // and the index is the palette handle
                                      // since handle=0 is not used, then
                                      // ppPalHndlList[0] is used to hold the
                                      // number of entries in ppPalHndlList
                                      // array, if allocated
} HNDLLIST;
typedef HNDLLIST FAR* PHNDLLIST;

#define FLAGS_PER_DWORD                   32
#define IS_STATEBLOCK_RENDERSTATE_FLAG_SET(pSB,state)       pSB->uc.bStoredRS[(state)/FLAGS_PER_DWORD] & (1 << ((state) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_RENDERSTATE_FLAG(pSB,state)          pSB->uc.bStoredRS[(state)/FLAGS_PER_DWORD] |= (1 << ((state) % FLAGS_PER_DWORD))
#define IS_STATEBLOCK_TEXSTAGESTATE_FLAG_SET(pSB,tmu,state) pSB->uc.bStoredTSS[(tmu)][(state)/FLAGS_PER_DWORD] & (1 << ((state) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_TEXTSTAGESTATE_FLAG(pSB,tmu,state)   pSB->uc.bStoredTSS[(tmu)][(state)/FLAGS_PER_DWORD] |= (1 << ((state) % FLAGS_PER_DWORD))
#define IS_STATEBLOCK_VIEWPORT_FLAG_SET(pSB)                pSB->uc.bStoredVP
#define SET_STATEBLOCK_VIEWPORT_FLAG(pSB)                   pSB->uc.bStoredVP = 1
#ifdef TnL_HAL
#define IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB,light,state)  pSB->uc.bStoredLS[(light)/FLAGS_PER_DWORD][(state)] & (1 << ((light) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_LIGHTSTATE_FLAG(pSB,light,state)     pSB->uc.bStoredLS[(light)/FLAGS_PER_DWORD][(state)] |= (1 << ((light) % FLAGS_PER_DWORD))
#define IS_STATEBLOCK_MATERIAL_FLAG_SET(pSB)                pSB->uc.bStoredMAT
#define SET_STATEBLOCK_MATERIAL_FLAG(pSB)                   pSB->uc.bStoredMAT = 1
#define IS_STATEBLOCK_XFORM_FLAG_SET(pSB,xfm)               pSB->uc.bStoredXFM[(xfm)/FLAGS_PER_DWORD] & (1 << ((xfm) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_XFORM_FLAG(pSB,xfm)                  pSB->uc.bStoredXFM[(xfm)/FLAGS_PER_DWORD] |= (1 << ((xfm) % FLAGS_PER_DWORD))
#define IS_STATEBLOCK_ZRANGE_FLAG_SET(pSB)                  pSB->uc.bStoredZR
#define SET_STATEBLOCK_ZRANGE_FLAG(pSB)                     pSB->uc.bStoredZR = 1
#define IS_STATEBLOCK_CLIPPLANE_FLAG_SET(pSB,cp)            pSB->uc.bStoredCP[(cp)/FLAGS_PER_DWORD] & (1 << ((cp) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_CLIPPLANE_FLAG(pSB,cp)               pSB->uc.bStoredCP[(cp)/FLAGS_PER_DWORD] |= (1 << ((cp) % FLAGS_PER_DWORD))
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
#define IS_STATEBLOCK_STREAM_FLAG_SET(pSB,state)            pSB->uc.bStoredStream & (1 << ((state) % FLAGS_PER_DWORD))
#define SET_STATEBLOCK_STREAM_FLAG(pSB,state)               pSB->uc.bStoredStream |= (1 << ((state) % FLAGS_PER_DWORD))
#endif
// As of the bld 2082 DDK, this is the max value in the D3DRENDERSTATETYPE enum in d3dtypes.h
#define MAX_RENDERSTATES              D3DRENDERSTATE_CLIPPLANEENABLE      // = 152
// As of the bld 2072 DDK, this is the max value in the D3DTEXTURESTAGESTATETYPE enum in d3dtypes.h
//#define MAX_TEXTURESTAGESTATES        D3DTSS_TEXTURETRANSFORMFLAGS        // = 24
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
#define MAX_TEXTURESTAGESTATES        D3DTSS_RESULTARG+1                    // = 28 + 1 (wrap)
#else
#define MAX_TEXTURESTAGESTATES        D3DTSS_TEXTURETRANSFORMFLAGS+1        // = 24 + 1 (wrap)
#endif

#ifdef TnL_HAL
#define TLMAX_USER_CLIPPLANES	6
// Maximum number of ACTIVE lights - we'll support up to 64 total lights
// in the state blocks, otherwise the size is going to get out of hand
#define TLMAX_LIGHTS			64
#define TLMAX_ACTIVE_LIGHTS 	32

// These are different from the D3DTRANSFORMSTATE_xxx defines
#define TLTRANSFORMSTATE_WORLD	0
#define TLTRANSFORMSTATE_WORLD1	1
#define TLTRANSFORMSTATE_WORLD2	2
#define TLTRANSFORMSTATE_WORLD3	3
#define TLTRANSFORMSTATE_VIEW	4
#define TLTRANSFORMSTATE_PROJ	5
#define TLTRANSFORMSTATE_TEX0	6
#define TLTRANSFORMSTATE_TEX1	7
#define TLTRANSFORMSTATE_TEX2	8
#define TLTRANSFORMSTATE_TEX3	9
#define TLTRANSFORMSTATE_TEX4	10
#define TLTRANSFORMSTATE_TEX5	11
#define TLTRANSFORMSTATE_TEX6	12
#define TLTRANSFORMSTATE_TEX7	13
#define MAX_TRANSFORMATIONS		14	// 4 worlds, view, proj, 8 texture


typedef struct _TLVECTOR4
{
    D3DVALUE x;
    D3DVALUE y;
    D3DVALUE z;
    D3DVALUE w;
} TLVECTOR4, *LPTLVECTOR4;

typedef struct _TLUSERCLIPPLANE
{
  TLVECTOR4   plane;
  BOOL        bActive;
} TLUSERCLIPPLANE, *LPTLUSERCLIPPLANE;


typedef struct _LIGHTSTATE
{
	BOOL		create;		// create light has been called on this light
	BOOL		enabled;	// or disabled
	D3DLIGHT7	lData;		// all the data
} LIGHTSTATE, *LPLIGHTSTATE;
#endif

typedef struct _STATEBLOCK
{
  DWORD dwHandle;
  BOOL  bCompressed;

  union
  {
    // stored state block info (uncompressed)
    struct
    {
      // we're going to waste the entry in index 0 in each of these arrays
      DWORD RenderStates[MAX_RENDERSTATES+1];
      DWORD TssStates[NUMTEXTUREUNITS+1][MAX_TEXTURESTAGESTATES+1];
      D3DHAL_DP2VIEWPORTINFO    ViewportInfoState;
#ifdef TnL_HAL
      LIGHTSTATE                LightStates[TLMAX_LIGHTS];
      D3DMATERIAL7              MaterialState;
      D3DMATRIX                 TransformationStates[MAX_TRANSFORMATIONS];
      D3DHAL_DP2ZRANGE          ZRangeState;
      TLVECTOR4                 ClipPlaneStates[TLMAX_USER_CLIPPLANES];
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      D3DHAL_DP2SETSTREAMSOURCE StreamSource[1];    // Single stream only
      D3DHAL_DP2SETINDICES      StreamIndices;
      DWORD                     dwShaderHandle;
#endif
      // store 32 flags per dword
      BOOL  bStoredRS[((MAX_RENDERSTATES+1)+(FLAGS_PER_DWORD-1))/FLAGS_PER_DWORD];
      BOOL  bStoredTSS[NUMTEXTUREUNITS+1][((MAX_TEXTURESTAGESTATES+1)+(FLAGS_PER_DWORD-1))/FLAGS_PER_DWORD];
	  BOOL  bStoredVP;
#ifdef TnL_HAL
	  BOOL  bStoredLS[(TLMAX_LIGHTS/FLAGS_PER_DWORD)+1][3];
	  BOOL  bStoredMAT;
      BOOL  bStoredXFM[(MAX_TRANSFORMATIONS/FLAGS_PER_DWORD)+1];
      BOOL  bStoredZR;
      BOOL  bStoredCP[(TLMAX_USER_CLIPPLANES/FLAGS_PER_DWORD)+1];
#endif
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      BOOL  bStoredStream;
#endif
    } uc;
    // stored state block info (compressed)
    struct
    {
      DWORD dwNumRS;
      DWORD dwNumTSS[NUMTEXTUREUNITS+1];
      DWORD dwSizeStoredViewport;
#ifdef TnL_HAL
      DWORD dwNumLightDataStates;
      DWORD dwSizeStoredMAT;
      DWORD dwNumXforms;
      DWORD dwSizeStoredZRange;
      DWORD dwNumUserClipplanes;
#endif
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      DWORD dwSizeSetStreamSource;
      DWORD dwSizeSetIndices;
      DWORD dwSizeShaderHandle;
#endif
      DWORD numPairs;   // entries in the "pair[]" array
      struct
      {
        DWORD dwType;   // either D3DRENDERSTATE_xxx or an index
        DWORD dwValue;  // the data
      } pair[1];
      // Viewport + T&L + DX 8 dynamic data will follow
    } cc;
  };
} STATEBLOCK, *PSTATEBLOCK;

extern HNDLLIST *GetHndlListPtr(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl);

typedef struct _RC  RC;
extern HRESULT SetRenderTarget(RC *,DWORD,DWORD);
extern HRESULT Clear(RC *,DWORD,DWORD,D3DVALUE,DWORD,RECT *,DWORD);
extern HRESULT FastFill(RC *, D3DHAL_DP2CLEAR *, DWORD);
extern HRESULT FastFill_2pass(RC *, D3DHAL_DP2CLEAR *, DWORD);
extern HRESULT PaletteSet(RC *,DWORD,DWORD,DWORD);
extern HRESULT PaletteUpdate(RC *,DWORD,WORD,WORD,BYTE *);
extern HRESULT BeginStateBlock(RC *, DWORD);
extern HRESULT EndStateBlock(RC *);
extern HRESULT DeleteStateBlock(RC *, DWORD);
extern HRESULT ExecuteStateBlock(RC *, DWORD);
extern HRESULT CaptureStateBlock(RC *, DWORD);

extern DWORD bufferStateSetup(RC *pRc, DWORD unconditionalSet);

extern VOID ReleaseContextStateBlocks(RC *);

#endif  // DX7

#endif // __D7D3D_H__

