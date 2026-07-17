/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name:   render.c
**
** Description: Renders triangles sent by the T&L code
**
**
*/

#define ASM_RENDERING_CODE		// comment this out to disable the asm rendering code
//#define BACKFACE_CULL

#include "precomp.h"

#if ( DX >= 7 ) && defined(TnL_HAL) 

#ifndef WINNT
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d6fvf.h"
#include "d6global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "cliprend.h"
#endif //WINNT


#include "dxins.h"

// include #defines for performance optimizations
#ifndef WINNT																	
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
#endif //WINNT


#ifdef DEBUG
static DWORD TnlStats_PrimCount = 0;
static DWORD TnlStats_NumTrisThisPrim;
static DWORD TnlStats_SpecBuffWrites = 0;

extern DWORD TnlStats_TotalTrisRendered;
extern DWORD TnlStats_SpecPartailTrisRendered;
extern DWORD TnlStats_SpecFullTrisRendered;
extern DWORD TnlStats_DiffPartailTrisRendered;
extern DWORD TnlStats_DiffFullTrisRendered;
#endif


#define TL_WRAP( A, B, C, Scale_2, Scale )									\
{																			\
    float   fTmp;															\
    INT32   dAB, dBC, dAC;													\
    fTmp = A-B;     dAB = 0x7fffffff & AS_INT32(fTmp);						\
    fTmp = B-C;     dBC = 0x7fffffff & AS_INT32(fTmp);						\
    fTmp = A-C;     dAC = 0x7fffffff & AS_INT32(fTmp);						\
    if        ((dAC > AS_INT32(Scale_2)) && (dBC > AS_INT32(Scale_2))) {	\
                if (C < B) {    C += Scale;             }					\
                else {          A += Scale; B += Scale; }					\
    } else if ((dBC > AS_INT32(Scale_2)) && (dAB > AS_INT32(Scale_2))) {	\
                if (B < A) {    B += Scale;             }					\
                else {          A += Scale; C += Scale; }					\
    } else if ((dAB > AS_INT32(Scale_2)) && (dAC > AS_INT32(Scale_2))) {	\
                if (A < C) {    A += Scale;             }					\
                else {          B += Scale; C += Scale; }					\
    }																		\
}

     
#define BIT_FAN				(0)
#define BIT_STRIP			(1)
#define BIT_STRIP_MODE      (22)
#define TRI_TYPE_FAN        (1 << BIT_FAN)
#define TRI_TYPE_STRIP      (1 << BIT_STRIP)

#define MAX_TNL_TRI_PARAMS  32	// (PH3_SIZE + 30)



/* FIFO Write macros (necessary to get the compiler to generate fast code) */
// write 1 dword to 2 destinations, reads "data" only once
#define SET1x2FDW(hwReg, memBuff, data) \
	(*(volatile float *)&(hwReg)) = (*(float *)&(memBuff)) = (data);

// write 2 dwords to 1 destination, order is read2, read1, write1, write2, inc dest ptr
// tries to get around read-after-write stalls and increment the destination ptr only once
#define SET2x1FDW(hwReg, data1, data2) \
{ \
	float ftmp = (*(float *)&(data2)); \
	((volatile float)(hwReg[0])) = (*(float *)&(data1)); \
	((volatile float)(hwReg[1])) = ftmp; \
	hwReg += 2; \
}

// write 2 dwords to 2 destinations, order is read2, read1, write1->1, write1->2, write2->1, write2->2, inc dest ptrs
#define SET2x2FDW(hwReg, memBuff, data1, data2) \
{ \
	float ftmp = (*(float *)&(data2)); \
	((volatile float)(hwReg[0])) = ((float)(memBuff[0])) = (*(float *)&(data1)); \
	((volatile float)(hwReg[1])) = ((float)(memBuff[1])) = ftmp; \
	memBuff += 2; \
	hwReg += 2; \
}



/****************************************************************************************
*   Command Fifo Functions
****************************************************************************************/
INLINE void TNL_CMDFIFO_CHECKBUMP (RC* pRc, DWORD n)
{
	DWORD dwCmdFifoBump = pRc->tl.KniRC.dwCmdFifoBump + n;
	if( dwCmdFifoBump >= 0x40000 ) {
		P6FENCE;
		dwCmdFifoBump = n;
	}
	pRc->tl.KniRC.dwCmdFifoBump = dwCmdFifoBump;
}

#ifdef WINNT
_inline void TNL_SYNC_GBL_TO_TNL(RC* pRc)									
{																			
	SETUP_PPDEV(pRc)
	FxU32	*lpCmdFifoPtr = (FxU32 *)ppdev->fifoData.fifoPtr;									
	FxI32	fifo_room = (FxI32)ppdev->fifoData.roomToEnd;														
	if (ppdev->fifoData.roomToReadPtr < ppdev->fifoData.roomToEnd)  		
		fifo_room = (FxI32)ppdev->fifoData.roomToReadPtr;					

	pRc->tl.KniRC.lpCmdFifoPtr = (DWORD *)ppdev->fifoData.fifoPtr;
	pRc->tl.KniRC.dwCmdFifoRoom = (DWORD)fifo_room * sizeof(DWORD);
	pRc->tl.KniRC.dwCmdFifoBump = (DWORD)CMDFIFOBUMP * sizeof(DWORD);
}

_inline void TNL_SYNC_TNL_TO_GBL(RC* pRc)									
{																			
	SETUP_PPDEV(pRc)
	ppdev->fifoData.lpCmdFifoPtr = (FxI32)pRc->tl.KniRC.lpCmdFifoPtr;				  
	ppdev->fifoData.roomToEnd = pRc->tl.KniRC.dwCmdFifoRoom / sizeof(DWORD);
	ppdev->fifoData.roomToReadPtr = 0;										
	CMDFIFOBUMP = pRc->tl.KniRC.dwCmdFifoBump / sizeof(DWORD);
	AGP_FLUSH;
}

void tnl_fifo_make_room (RC* pRc, FxU32 size)
{
	SETUP_PPDEV(pRc)
	TNL_SYNC_TNL_TO_GBL(pRc);
	{
	CMDFIFO_PROLOG(cmdFifo);
	CMDFIFO_CHECKROOM(ppdev, &cmdFifo,size);
	CMDFIFO_EPILOG(cmdFifo);
	}
	TNL_SYNC_GBL_TO_TNL(pRc);
}

#else // not WINNT

_inline void TNL_SYNC_GBL_TO_TNL (RC* pRc)
{										
	SETUP_PPDEV(pRc)
	pRc->tl.KniRC.lpCmdFifoPtr = (DWORD *)CMDFIFOPTR;
	pRc->tl.KniRC.dwCmdFifoRoom = (DWORD)CMDFIFOSPACE * sizeof(DWORD);
	pRc->tl.KniRC.dwCmdFifoBump = (DWORD)CMDFIFOBUMP * sizeof(DWORD);
}

_inline void TNL_SYNC_TNL_TO_GBL (RC* pRc)
{											
	SETUP_PPDEV(pRc)
	CMDFIFOPTR = (FxI32)pRc->tl.KniRC.lpCmdFifoPtr;				  
	CMDFIFOSPACE = pRc->tl.KniRC.dwCmdFifoRoom / sizeof(DWORD);
	CMDFIFOBUMP = pRc->tl.KniRC.dwCmdFifoBump / sizeof(DWORD);
	AGP_FLUSH;
}

void tnl_fifo_make_room(RC* pRc, FxU32 size)
{
	SETUP_PPDEV(pRc)
	TNL_SYNC_TNL_TO_GBL(pRc);
	{
	CMDFIFO_PROLOG(cmdFifo);
	fifo_MakeRoom(ppdev, &cmdFifo, size);
	CMDFIFO_EPILOG(cmdFifo);
	}
	TNL_SYNC_GBL_TO_TNL(pRc);
}
#endif // not WINNT



/****************************************************************************************
*   Local Function Definitions
****************************************************************************************/
void TL_RENDER_DumpSpecularBuffer ( RC* pRc );


/****************************************************************************************
*   Externs
****************************************************************************************/
extern DWORD setDX6state( RC *pRc, DWORD primitiveType, DWORD count );
extern DWORD TL_RenderFunctionLookupTable;

// Global Specular Buffer
#define SPECULAR_BUFFER_ENTRIES     1024        // don't go overboard here or we'll thresh the L1   
#define SPECULAR_BUFFER_THRESHOLD   SPECULAR_BUFFER_ENTRIES-32
FxU32 SpecBuffer[SPECULAR_BUFFER_ENTRIES];

// This switch turns on "no aliasing across function calls".  The was compiler cacheing the 
// command fifo pointers/space/etc after tnl_fifo_make_room() instead of reloading them.  I
// even tried making the values volatile but MSVC ignored this it.  (BAD!!!) I've checked 
// that this switch works on the release version of service pack 4.
#pragma optimize( "w", on )

/****************************************************************************************
*   Rendering Setup Function
*
****************************************************************************************/
void TL_RENDER_Setup ( RC* pRc )
{
    SETUP_PPDEV ( pRc )         // initializes ppdev
    DWORD RfBits = 0;       	// local copy
	DWORD AnyTx;
    if( HW_STATE_CHANGED )
        setDX6state( pRc, 0, 0);    // pRc, primitiveType, #primitives


    /*
    * Intialize the bitfield
    */
    TestBitAndSet( &RfBits, BIT_RC_SPECULAR, &pRc->specular, 1 );												//BIT_RC_SPECULAR
//	RfBits |= (pRc->specular) ? BIT_RC_SPECULAR : 0;															//BIT_RC_SPECULAR
    RfBits |= (pRc->shadeMode == D3DSHADE_FLAT) ? 
    				(BIT_RC_FLAT_SHADING | BIT_RC_NEVER_AUTOSTRIP) : 0;											//BIT_RC_FLAT_SHADING
    RfBits |= (pRc->sst.sSetupMode & (SST_SETUP_RGB | SST_SETUP_A)) ? BIT_RC_COLOR : BIT_RC_BAD_FVF_TYPE;		//BIT_RC_COLOR
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_WBUFFER, &pRc->state, STATE_REQUIRES_WBUFFER );						//BIT_RC_SPECULAR
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_Z, &pRc->sst.sSetupMode, SST_SETUP_Z );								//BIT_RC_REQUIRES_Z
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_WFBI, &pRc->sst.sSetupMode, SST_SETUP_Wfbi );						//BIT_RC_REQUIRES_WFBI
    TestBitAndSet( &RfBits, BIT_RC_VERTEX_FOG, &pRc->state, STATE_REQUIRES_VERTEXFOG );							//BIT_RC_VERTEX_FOG
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_W, &pRc->sst.sSetupMode, SST_SETUP_W0 );							//BIT_RC_REQUIRES_W
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_PERSPECTIVE, &pRc->state, STATE_REQUIRES_PERSPECTIVE );				//BIT_RC_REQUIRES_PERSPECTIVE
    TestBitAndSet( &RfBits, BIT_RC_ALPHA, &pRc->alphaBlendEnable, 1 );											//BIT_RC_ALPHA
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_TX0, &pRc->state, STATE_REQUIRES_ST_TMU0 );							//BIT_RC_REQUIRES_TX0
    TestBitAndSet( &RfBits, BIT_RC_REQUIRES_TX1, &pRc->state, STATE_REQUIRES_ST_TMU1 );							//STATE_REQUIRES_ST_TMU1
    TestBitAndSet( &RfBits, BIT_RC_WRAP_TX0_S, &pRc->wrapT0, D3DWRAP_U );										//BIT_RC_WRAP_TX0_S
    TestBitAndSet( &RfBits, BIT_RC_WRAP_TX0_T, &pRc->wrapT0, D3DWRAP_V );										//BIT_RC_WRAP_TX0_T
    TestBitAndSet( &RfBits, BIT_RC_WRAP_TX1_S, &pRc->wrapT1, D3DWRAP_U );										//BIT_RC_WRAP_TX1_S
    TestBitAndSet( &RfBits, BIT_RC_WRAP_TX1_T, &pRc->wrapT1, D3DWRAP_V );										//BIT_RC_WRAP_TX1_T
    //RfBits |= (pRc->state & STATE_REQUIRES_WBUFFER) ? BIT_RC_REQUIRES_WBUFFER : 0;							//BIT_RC_REQUIRES_WBUFFER
    //RfBits |= (pRc->sst.sSetupMode & SST_SETUP_Z) ? BIT_RC_REQUIRES_Z : 0;									//BIT_RC_REQUIRES_Z
    //RfBits |= (pRc->sst.sSetupMode & SST_SETUP_Wfbi) ? BIT_RC_REQUIRES_WFBI : 0;								//BIT_RC_REQUIRES_WFBI
    //RfBits |= (pRc->state & STATE_REQUIRES_VERTEXFOG) ? BIT_RC_VERTEX_FOG : 0;								//BIT_RC_VERTEX_FOG
    //RfBits |= (pRc->sst.sSetupMode & SST_SETUP_W0) ? BIT_RC_REQUIRES_W : 0;									//BIT_RC_REQUIRES_W
    //RfBits |= (pRc->state & STATE_REQUIRES_PERSPECTIVE) ? BIT_RC_REQUIRES_PERSPECTIVE : 0;					//BIT_RC_REQUIRES_PERSPECTIVE
    //RfBits |= (pRc->alphaBlendEnable) ? BIT_RC_ALPHA : 0;														//BIT_RC_ALPHA
    //RfBits |= (pRc->wrapT0 & D3DWRAP_U) ? BIT_RC_WRAP_TX0_S : 0;												//BIT_RC_WRAP_TX0_S
    //RfBits |= (pRc->wrapT0 & D3DWRAP_V) ? BIT_RC_WRAP_TX0_T : 0;												//BIT_RC_WRAP_TX0_T
    //RfBits |= (pRc->wrapT1 & D3DWRAP_U) ? BIT_RC_WRAP_TX1_S : 0;												//BIT_RC_WRAP_TX1_S
    //RfBits |= (pRc->wrapT1 & D3DWRAP_V) ? BIT_RC_WRAP_TX1_T : 0;												//BIT_RC_WRAP_TX1_T
    RfBits |= ( ((RfBits & BIT_RC_REQUIRES_TX0) && (RfBits & (BIT_RC_WRAP_TX0_S | BIT_RC_WRAP_TX0_T)))
              ||((RfBits & BIT_RC_REQUIRES_TX1) && (RfBits & (BIT_RC_WRAP_TX1_S | BIT_RC_WRAP_TX1_T)))
              ) ? (BIT_RC_WRAP_VALID | BIT_RC_NEVER_AUTOSTRIP) : 0;												//BIT_RC_WRAP_VALID
	AnyTx = (RfBits & (BIT_RC_REQUIRES_TX0 | BIT_RC_REQUIRES_TX1)) ? 1 : 0;
//    RfBits |= ((RfBits & BIT_RC_SPECULAR) && AnyTx) ? BIT_RC_SPEC_AND_TEXTURES : 0;								//BIT_RC_SPEC_AND_TEXTURES

	// check for remaining bad fvf types
	RfBits |= (AnyTx != ((DWORD)((RfBits & BIT_RC_REQUIRES_W) ? 1 : 0))) ? BIT_RC_BAD_FVF_TYPE : 0;				// textures require w, no textures require no w

	// these four are used for setting up the specular pass only
    RfBits |= IS_NAPALM ? BIT_RC_IS_NAPALM : 0;																	//BIT_RC_IS_NAPALM
    TestBitAndSet( &RfBits, BIT_RC_FOG_ENABLE, &pRc->fogEnable, 1 );											//BIT_RC_FOG_ENABLE
    TestBitAndSet( &RfBits, BIT_RC_Z_ENABLE, &pRc->zEnable, 1 );												//BIT_RC_Z_ENABLE
    TestBitAndSet( &RfBits, BIT_RC_Z_WRITE_ENABLE, &pRc->zWriteEnable, 1 );										//BIT_RC_Z_WRITE_ENABLE
    //RfBits |= pRc->fogEnable ? BIT_RC_FOG_ENABLE : 0;															//BIT_RC_FOG_ENABLE
    //RfBits |= pRc->zEnable ? BIT_RC_Z_ENABLE : 0;																//BIT_RC_Z_ENABLE
    //RfBits |= pRc->zWriteEnable ? BIT_RC_Z_WRITE_ENABLE : 0;													//BIT_RC_Z_WRITE_ENABLE

//	RfBits |= BIT_RC_NEVER_AUTOSTRIP;	// disable to debug without autostripping
    pRc->tl.KniRC.RfBits = RfBits;     // write local copy back out


	// Check if we should we pre-calculate the prospective divide
    if( (!(RfBits & BIT_RC_REQUIRES_PERSPECTIVE)) || (RfBits & BIT_RC_WRAP_VALID) )
      pRc->tl.dwTLState &= ~TLPV_DO_PROSPECTIVE_DIVIDE;
	else
      pRc->tl.dwTLState |= TLPV_DO_PROSPECTIVE_DIVIDE;


    /*
    * Intialize autostripping
    */
	pRc->tl.KniRC.prevC = 0;			// disables autostripping on next tri

    /* 
    * Build wrap variables
    */
    if ( RfBits & BIT_RC_WRAP_VALID ) {
		asmKniTxScaleHalf.f[0] = asmKniTxScaleHalf.f[2] = (pRc->sst.scaleS * 0.5f);
		asmKniTxScaleHalf.f[1] = asmKniTxScaleHalf.f[3] = (pRc->sst.scaleT * 0.5f);
		asmKniTxScaleOne.f[0] = (pRc->wrapT0 & D3DWRAP_U) ? (pRc->sst.scaleS * 1.0f) : 0;
		asmKniTxScaleOne.f[1] = (pRc->wrapT0 & D3DWRAP_V) ? (pRc->sst.scaleT * 1.0f) : 0;
		asmKniTxScaleOne.f[2] = (pRc->wrapT1 & D3DWRAP_U) ? (pRc->sst.scaleS * 1.0f) : 0;
		asmKniTxScaleOne.f[3] = (pRc->wrapT1 & D3DWRAP_V) ? (pRc->sst.scaleT * 1.0f) : 0;
    }

	/*
	* Intialize command fifo
	*/
	TNL_SYNC_GBL_TO_TNL(pRc);
#ifdef DEBUG
	TnlStats_NumTrisThisPrim=0;
	++TnlStats_PrimCount;
#endif

    /*
   	* Build start commands
    */
   	pRc->tl.KniRC.CmdStartDiff = CMDFIFO_BUILD_PK3( CMD_START, 3, pRc->sst.sSetupMode, 1 ) | (1<<BIT_STRIP_MODE);
    pRc->tl.KniRC.CmdContDiff  = CMDFIFO_BUILD_PK3( CMD_CONT , 1, pRc->sst.sSetupMode, 1 );
//   	if ( RfBits & BIT_RC_SPEC_AND_TEXTURES ) {
	if ( RfBits & BIT_RC_SPECULAR ) {	// SPECULARFIX!!!
       	if ( RfBits & BIT_RC_ALPHA ) {
           	pRc->tl.KniRC.CmdStartSpec = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | (SST_SETUP_RGB | SST_SETUP_A)), 1 ) | (1<<BIT_STRIP_MODE);
   	        pRc->tl.KniRC.CmdContSpec  = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | (SST_SETUP_RGB | SST_SETUP_A)), 1 );
        } else {
            pRc->tl.KniRC.CmdStartSpec = CMDFIFO_BUILD_PK3( CMD_START, 3, ((pRc->sst.sSetupMode & ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)) | (SST_SETUP_RGB | SST_SETUP_A)), 1 ) | (1<<BIT_STRIP_MODE);
           	pRc->tl.KniRC.CmdContSpec  = CMDFIFO_BUILD_PK3( CMD_CONT , 1, ((pRc->sst.sSetupMode & ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)) | (SST_SETUP_RGB | SST_SETUP_A)), 1 );
       	}

        /* intialize specular buffer */
   	    pRc->tl.KniRC.lpSpecBuffBase = &SpecBuffer[1];		// start at an odd 8-byte boundary, this only needs done once - at initialization
       	pRc->tl.KniRC.lpSpecBuff = pRc->tl.KniRC.lpSpecBuffBase;

        // Set the the point at which to dump the specular buffer
   	    // check for the special case of specular and no Z (send down a tri at a time)
       	if ( (RfBits & (BIT_RC_SPECULAR | BIT_RC_REQUIRES_Z)) == (BIT_RC_SPECULAR | BIT_RC_REQUIRES_Z) )
           	pRc->tl.KniRC.SpecBufferThreshold = &(pRc->tl.KniRC.lpSpecBuff[SPECULAR_BUFFER_THRESHOLD]);
   	    else
            pRc->tl.KniRC.SpecBufferThreshold = pRc->tl.KniRC.lpSpecBuff;     // dumps the specular buffer each tri
    }

   	/*
    * Figure out which function to use for rendering
   	*/
#if defined(VCPP) && defined(ASM_RENDERING_CODE)
	if((pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE) && (! (RfBits & BIT_RC_BAD_FVF_TYPE))) {
		DWORD *RfLut = (DWORD*)&TL_RenderFunctionLookupTable;
		pRc->tl.KniRC.RenderTriFunct_NoClip = (TL_RENDER_RenderTri_Type_NoClip*)RfLut[RfBits & BITS_RC_RFUNCT_MASK];
		if ( pRc->tl.KniRC.RenderTriFunct_NoClip == 0 ) {
			// wierd types default to the C rendering code
			pRc->tl.KniRC.RenderTriFunct_NoClip = TL_RENDER_Default_NoClip;
		} else {
			// align the cmd fifo to an odd 8-byte boundary  ... I really hate 
			// doing this but it's needed to 8-byte align the qword writes
			if (! ((DWORD)pRc->tl.KniRC.lpCmdFifoPtr & 0x4)) {
				if ( sizeof(DWORD) > pRc->tl.KniRC.dwCmdFifoRoom )
					tnl_fifo_make_room(pRc, MAX_TNL_TRI_PARAMS);
   		    SETDW( *(pRc->tl.KniRC.lpCmdFifoPtr++), 0 );	// send a NOP
				pRc->tl.KniRC.dwCmdFifoRoom -= sizeof(DWORD);
			}
		}
	} 
	else 
#endif
	{
		pRc->tl.KniRC.RenderTriFunct_NoClip = TL_RENDER_Default_NoClip;
	}

	pRc->tl.KniRC.pRc = pRc;
	pRc->tl.KniRC.LastRfBits = RfBits;

}   // end of TL_RENDER_Setup()

/****************************************************************************************
*   Default C triangle rendering code - clipping
*
****************************************************************************************/
void TL_RENDER_Default_Clip ( RC* pRc, lpTLBN pA, lpTLBN pB, lpTLBN pC, D3DVERTEX *pAin, D3DVERTEX *pBin, D3DVERTEX *pCin )
{
#define SWTNL_DBG 1
  // Set the default count to 1
  FxU32 count = 1;
  DWORD         ClipContinue = 0;       // Tells us if we need to continue the 
                                        // clipped triangle already in progress
  DWORD         dwNumClippedVertices;
  LPBYTE pTLV = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;
  DWORD dwUnion;
  DWORD dwMask;
  DWORD TLV_idx;
  LPDWORD       pTA = NULL;
  LPDWORD       pTB = NULL;
  LPDWORD       pTC = NULL;
  DWORD         TempNumTexCoords = pRc->tl.InFVF.dwNumTexCoords;
  DWORD         TempStage0CoordSize = pRc->tl.dwTexCoordSize[0];
  DWORD         TempStage1CoordSize = pRc->tl.dwTexCoordSize[1];

_asm emms
  if ( !pA || !pB || !pC )
  {
      D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: ERROR!!! TLBN Pointers are invalid" );
	  return;
  }
  if ( !pTLV )
  {
      D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: ERROR!!! PTLV Pointer is invalid" );
	  return;
  }	  

  for( ; count > 0; count-- )
  {
      // Check to see if we had started a multi triangle clip
      // in the last loop.
      if(ClipContinue)
      {
        // Yes, we have a cliped triangle in progress
        // Don't bother pulling out new vertex data 
        // from the command stream. Just re-arrange the
        // vertex pointers from the new output clip
        // FVF buffer for the next blade of the fan to clip

                    // pA is the center of the Fan, it does NOT change
        pTB = pTC;    // pB is assigned to the Previous blade's pC vertex
        pTC = (LPDWORD) (pTLV + TLBN_SIZE*TLV_idx);//pRc->tl.TLFVF.dwStride*TLV_idx); // pC is the next vertex in the clipped FVF buf

        ClipContinue--;     // Decrement the ClipContine value
        TLV_idx++;          // Increment the new FVF buf index

		D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: We are in the middle of a fan: TLV_idx = %8lXh", TLV_idx );
		D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: We are in the middle of a fan: ClipContinue = %8lXh", ClipContinue );
		D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: We are in the middle of a fan: pTA = %8lXh pTB = %8lXh pTC = %8lXh", pTA, pTB, pTC );
      }
      else
      {
        // !!! QUICK TRIVIAL REJECT TEST !!!
        // Clipping test for this triangle
        if ((pA->clip_code & pB->clip_code & pC->clip_code))
        {
          // This triangle is totaly clipped
          // skip it and go on to teh next one
          // in the command stream
          pRc->tl.KniRC.prevC = 0;			// disables autostripping
          D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: Trivial Reject, Clip codes say this triangle is not even worth processing" );

		  return;
        }
		
        // This Triangle is not TR clipped 
        // Get the OR flags and clip mask to see if we can trivial 
        // accept it
        dwUnion = (pA->clip_code | pB->clip_code | pC->clip_code);
        dwMask = TLCLIP_LEFT  | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  |         
                 TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
   
        if (pRc->tl.dwTLState & TLPV_GUARDBAND) 
        {
          dwMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM |      
                   TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
        }

		D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: dwUnion = %8lXh", dwUnion );

        // If all the vertices are in, 
        // No clipping is needed! 
        // Let it pass down to the FIFO stuffing
        if ((dwUnion & dwMask) != 0)
        {
		  // Fake out the texture coord index for clipping just two textures
		  // because that is the max that will ever be in TLBN for napalm
          pRc->tl.InFVF.dwNumTexCoords = 2;
		  pRc->tl.dwTexCoordSize[0] = 8;
		  pRc->tl.dwTexCoordSize[1] = 8;

          if((dwNumClippedVertices = ClipTLBNTriangle( pRc, pA, pB, pC, pAin, pBin, pCin, dwMask)))
          {
            ClipContinue = dwNumClippedVertices - 3; 

            pTA = (LPDWORD) (pTLV); 
            pTB = (LPDWORD) (pTLV + TLBN_SIZE);//pRc->tl.TLFVF.dwStride); 
            pTC = (LPDWORD) (pTLV + TLBN_SIZE*2);//pRc->tl.TLFVF.dwStride*2); 
            TLV_idx = 3;  // Next vertex for the fan is at postion 3

            // Let the master loop know that we ne need to process more 
            // triangles if the clippeing calls for it.
            count += ClipContinue;

			D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: dwNumClippedVertices = %8lXh", dwNumClippedVertices );
			D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: count = %8lXh", count );
			D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: pTA = %8lXh pTB = %8lXh pTC = %8lXh", pTA, pTB, pTC );

            // Restore faking out the texture coordinate index
			pRc->tl.InFVF.dwNumTexCoords = TempNumTexCoords;
            pRc->tl.dwTexCoordSize[0] = TempStage0CoordSize;
		    pRc->tl.dwTexCoordSize[1] = TempStage1CoordSize;
          }
          else
          {
			// Clip routine said that there was nothing to
            // clip, skip it and go on to the next one
            // in the command stream
			pRc->tl.KniRC.prevC = 0;			// disables autostripping on next tri

            // Restore faking out the texture coordinate index
			pRc->tl.InFVF.dwNumTexCoords = TempNumTexCoords;
            pRc->tl.dwTexCoordSize[0] = TempStage0CoordSize;
		    pRc->tl.dwTexCoordSize[1] = TempStage1CoordSize;
             
            D3DPRINT( SWTNL_DBG, "TL_RENDER_Default_Clip: Trivial Reject, Clip routine said there was nothing to clip" );

			return;
          }

        } // if clipping on this triangle
		else // We trivially accepted this triangle!
		{
		    // All vertices are inside the viewing frustrum, lets draw em!
    		pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pA, pB, pC );
		}

	  } // not a clip continuation
      pRc->tl.KniRC.prevC = 0;			// disables autostripping on next tri

      if ( pTA && pTB && pTC )	
		pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pTA, pTB, pTC );
  }
  
  pRc->tl.KniRC.prevC = 0;			// disables autostripping on next tri
}

/****************************************************************************************
*   Default C triangle rendering code - non-clipping
*
*   This should handle any triangle type.  It isn't meant to be incredibly fast but 
*   should be a good starting place to convert on assembly/SSE.
*
****************************************************************************************/
void TL_RENDER_Default_NoClip ( TL_RENDER_DATA* KniRC, lpTLBN pA, lpTLBN pB, lpTLBN pC )
{
	RC*		pRc = (RC*)KniRC->pRc;
    SETUP_PPDEV ( pRc )         // initializes ppdev
    BOOL	autostripTri, spec2ndPassReq;
    DWORD	CmdContDiff, CmdContSpec;
	float	fTmp;

    DWORD   RfBits = KniRC->RfBits;			// hopefully this will stay in a cpu register
    DWORD   *lpCmdfifo, *lpSpecBuff;    	// local pointers
    TextureType texA[2], texB[2], texC[2];  // copy the textures if wrap is true
	DWORD	diffuseA, diffuseB, diffuseC;	// have to copy these because of flat shading
	DWORD	specularA, specularB, specularC;	// have to copy these because of flat shading

_asm emms
    /*******************************************************************
    *   Backface Cull Check
    *   if (sign ((pA->x-pB->x)*(pB->y-pC->y) - (pA->y-pB->y)*(pB->x-pC->x)))
    *       cull the triangle
    *******************************************************************/
#ifdef BACKFACE_CULL
    float area = (((pA->x-pB->x)*(pB->y-pC->y)) - ((pA->y-pB->y)*(pB->x-pC->x)));
    DWORD sign = AS_INT32(area) & 0x80000000L;
    if ((sign ^ pRc->cullMask) == 0x80000000L) {
		KniRC->prevC = 0;			// disables autostripping on next tri
		return;
    }
#endif
#ifdef DEBUG
	++TnlStats_TotalTrisRendered;
	++TnlStats_NumTrisThisPrim;
#endif

    /*******************************************************************
    *   Specular 2nd Pass Check: 2nd pass for specular highlights on 
    *       textures only if the specular color is not black
	*	Also load the colors and handle the flat shading
    *******************************************************************/
//	if ( RfBits & BIT_RC_SPEC_AND_TEXTURES ) {
	if ( RfBits & BIT_RC_SPECULAR ) {	// SPECULARFIX!!!
		spec2ndPassReq = (pA->specular | pB->specular | pC->specular) & 0x00ffffff;
		if (! (RfBits & BIT_RC_FLAT_SHADING)) {
			diffuseA = pA->diffuse;		specularA = pA->specular;
			diffuseB = pB->diffuse;		specularB = pB->specular;
			diffuseC = pC->diffuse;		specularC = pC->specular;
		} else {
			diffuseA  = diffuseB  = diffuseC  = pA->diffuse;
			specularA = specularB = specularC = pA->specular;
		}
    } else {
#if 0
		spec2ndPassReq = 0;
		if (! (RfBits & BIT_RC_FLAT_SHADING)) {
			if (! (RfBits & BIT_RC_SPECULAR)) {
				diffuseA = pA->diffuse;		
				diffuseB = pB->diffuse;		
				diffuseC = pC->diffuse;
			} else {
	            specularA = pA->specular;	CLAMP888( diffuseA, pA->diffuse, specularA );
	            specularB = pB->specular;	CLAMP888( diffuseB, pB->diffuse, specularB );
	            specularC = pC->specular;	CLAMP888( diffuseC, pC->diffuse, specularC );
			}
		} else {
			if (! (RfBits & BIT_RC_SPECULAR)) {
				diffuseA = diffuseB = diffuseC = pA->diffuse;
			} else {
				specularA = specularB = specularC = pA->specular;
	            CLAMP888( diffuseA, pA->diffuse, specularA );
				diffuseB = diffuseC = diffuseA;
			}
		}
#else	// SPECULARFIX!!!
		spec2ndPassReq = 0;
		if (! (RfBits & BIT_RC_FLAT_SHADING)) {
			diffuseA = pA->diffuse;		
			diffuseB = pB->diffuse;		
			diffuseC = pC->diffuse;
		} else {
			diffuseA = diffuseB = diffuseC = pA->diffuse;
		}
#endif
	}


    /*******************************************************************
    *   Autostrip Check 
    *   if ((prevC == currA) && (prevB == currB)    STRIP
    *   if ((prevA == currA) && (prevC == currB)    FAN
	*   if (currTriType == lastTriType) CmdCont |= STRIP_MODE
    *******************************************************************/
    if ( KniRC->prevC ) {
		DWORD triStrip = ((((DWORD)KniRC->prevC - (DWORD)pA) | ((DWORD)KniRC->prevB - (DWORD)pB)) == 0) ? 1 : 0;	// strip
		DWORD triFan = ((((DWORD)KniRC->prevA - (DWORD)pA) | ((DWORD)KniRC->prevC - (DWORD)pB)) == 0) ? 1 : 0;	// fan
		DWORD currTriType = (triStrip<<BIT_STRIP) | (triFan<<BIT_FAN);

		if ( (currTriType == 0) || (spec2ndPassReq && !KniRC->lastSpecRendered) ) {
			KniRC->lastTriType = TRI_TYPE_FAN; // Prime next strip/fan
			autostripTri = 0;
		} else {
			DWORD stripMode = ((currTriType - KniRC->lastTriType) == 0) ? 1 : 0;
			stripMode <<= BIT_STRIP_MODE;
			CmdContDiff = KniRC->CmdContDiff | stripMode;
			CmdContSpec = KniRC->CmdContSpec | stripMode;
			KniRC->lastTriType = currTriType;
			autostripTri = 1;
		}
    } else {
		KniRC->lastTriType = TRI_TYPE_FAN; // Prime next strip/fan
        autostripTri = 0;
    }
	KniRC->prevA = pA;      
	KniRC->prevB = pB;      
	KniRC->prevC = pC;


    /*******************************************************************
    *   WRAP
    *
    *   if wrap the T&L hands us: 
    *       tex = tex * scale + offset
    *   else we get:
    *       tex = tex * scale + offset * w
    *******************************************************************/
    if ( RfBits & BIT_RC_WRAP_VALID ) {
        // copy the textures
        if ( RfBits & BIT_RC_REQUIRES_TX0 ) {
            texA[0].s = pA->tex[0].s;       texA[0].t = pA->tex[0].t;
            texB[0].s = pB->tex[0].s;       texB[0].t = pB->tex[0].t;
            texC[0].s = pC->tex[0].s;       texC[0].t = pC->tex[0].t;
        }
        if ( RfBits & BIT_RC_REQUIRES_TX1 ) {
            texA[1].s = pA->tex[1].s;       texA[1].t = pA->tex[1].t;
            texB[1].s = pB->tex[1].s;       texB[1].t = pB->tex[1].t;
            texC[1].s = pC->tex[1].s;       texC[1].t = pC->tex[1].t;
        }

        // do the wrap
        if ( RfBits & BIT_RC_WRAP_TX0_S )   TL_WRAP ( texA[0].s, texB[0].s, texC[0].s, asmKniTxScaleHalf.f[0], asmKniTxScaleOne.f[0] );
        if ( RfBits & BIT_RC_WRAP_TX0_T )   TL_WRAP ( texA[0].t, texB[0].t, texC[0].t, asmKniTxScaleHalf.f[1], asmKniTxScaleOne.f[1] );
        if ( RfBits & BIT_RC_WRAP_TX1_S )   TL_WRAP ( texA[1].s, texB[1].s, texC[1].s, asmKniTxScaleHalf.f[2], asmKniTxScaleOne.f[2] );
        if ( RfBits & BIT_RC_WRAP_TX1_T )   TL_WRAP ( texA[1].t, texB[1].t, texC[1].t, asmKniTxScaleHalf.f[3], asmKniTxScaleOne.f[3] );

        // perspective correct divide is not done by the T&L code if wrap is true
        if ( RfBits & BIT_RC_REQUIRES_PERSPECTIVE ) {
			if ( RfBits & BIT_RC_REQUIRES_TX0 ) {
				texA[0].s *= pA->w;			texA[0].t *= pA->w;
				texB[0].s *= pB->w;			texB[0].t *= pB->w;
				texC[0].s *= pC->w;			texC[0].t *= pC->w;
			}
			if ( RfBits & BIT_RC_REQUIRES_TX1 ) {
				texA[1].s *= pA->w;			texA[1].t *= pA->w;
				texB[1].s *= pB->w;			texB[1].t *= pB->w;
				texC[1].s *= pC->w;			texC[1].t *= pC->w;
			}
        }
    }

    /************************************************************************************
    *   Start sending everything to the hardware 
    ************************************************************************************/
    /*
    * Command FIFO setup
    */
	TNL_CMDFIFO_CHECKBUMP(pRc, MAX_TNL_TRI_PARAMS*sizeof(DWORD));
	if ( (MAX_TNL_TRI_PARAMS * sizeof(DWORD)) > KniRC->dwCmdFifoRoom )
		tnl_fifo_make_room(pRc, MAX_TNL_TRI_PARAMS);
	lpCmdfifo = KniRC->lpCmdFifoPtr;

    if ( !spec2ndPassReq ) {
	    /*******************************************************************
    	*  non-specular only
	    *******************************************************************/
    	if ( autostripTri ) {
#ifdef DEBUG
			++TnlStats_DiffPartailTrisRendered;
#endif
	        /* Partial Triangle */
    	    SETDW( lpCmdfifo[0], CmdContDiff );                                         // continue triangle start command
        	/* Vertex C */
			fTmp = pC->y;
    	    SETFDW( lpCmdfifo[1], pC->x );												// x
        	SETFDW( lpCmdfifo[2], fTmp );												// y
			lpCmdfifo += 3;
			if ( RfBits & BIT_RC_COLOR )			SETDW( *lpCmdfifo++, diffuseC );	// diffuse
	        if ( RfBits & BIT_RC_REQUIRES_Z )       SETFDW( *lpCmdfifo++, pC->z );      // z
	        if ( RfBits & BIT_RC_REQUIRES_WFBI )    SETFDW( *lpCmdfifo++, pC->wfbi );   // wfbi
    	    if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pC->w );      // w
       	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pC->tex[0].s, pC->tex[0].t );	// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pC->tex[1].s, pC->tex[1].t );	// tx1
            } else {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texC[0].s, texC[0].t );		// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texC[1].s, texC[1].t );		// tx1
	        }
	    } else {
#ifdef DEBUG
			++TnlStats_DiffFullTrisRendered;
#endif
    	    /* Full Triangle */
        	SETDW( lpCmdfifo[0], KniRC->CmdStartDiff );									// full triangle start command
	        /* Vertex A */
			fTmp = pA->y;
    	    SETFDW( lpCmdfifo[1], pA->x );												// x
        	SETFDW( lpCmdfifo[2], fTmp );												// y
			lpCmdfifo += 3;
	        if ( RfBits & BIT_RC_COLOR )            SETDW( *lpCmdfifo++, diffuseA );	// diffuse
    	    if ( RfBits & BIT_RC_REQUIRES_Z )       SETFDW( *lpCmdfifo++, pA->z );      // z
        	if ( RfBits & BIT_RC_REQUIRES_WFBI )    SETFDW( *lpCmdfifo++, pA->wfbi );   // wfbi
	        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pA->w );      // w
       	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pA->tex[0].s, pA->tex[0].t );	// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pA->tex[1].s, pA->tex[1].t );	// tx1
            } else {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texA[0].s, texA[0].t );		// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texA[1].s, texA[1].t );		// tx1
	        }
    	    /* Vertex B */
			fTmp = pB->y;
    	    SETFDW( lpCmdfifo[0], pB->x );												// x
        	SETFDW( lpCmdfifo[1], fTmp );												// y
			lpCmdfifo += 2;
			if ( RfBits & BIT_RC_COLOR )			SETDW( *lpCmdfifo++, diffuseB );	// diffuse
        	if ( RfBits & BIT_RC_REQUIRES_Z )       SETFDW( *lpCmdfifo++, pB->z );      // z
	        if ( RfBits & BIT_RC_REQUIRES_WFBI )    SETFDW( *lpCmdfifo++, pB->wfbi );   // wfbi
    	    if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pB->w );      // w
       	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pB->tex[0].s, pB->tex[0].t );	// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pB->tex[1].s, pB->tex[1].t );	// tx1
            } else {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texB[0].s, texB[0].t );		// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texB[1].s, texB[1].t );		// tx1
	        }
	        /* Vertex C */
			fTmp = pC->y;
    	    SETFDW( lpCmdfifo[0], pC->x );												// x
        	SETFDW( lpCmdfifo[1], fTmp );												// y
			lpCmdfifo += 2;
			if ( RfBits & BIT_RC_COLOR )			SETDW( *lpCmdfifo++, diffuseC );	// diffuse
    	    if ( RfBits & BIT_RC_REQUIRES_Z )       SETFDW( *lpCmdfifo++, pC->z );      // z
        	if ( RfBits & BIT_RC_REQUIRES_WFBI )    SETFDW( *lpCmdfifo++, pC->wfbi );   // wfbi
	        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pC->w );      // w
       	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pC->tex[0].s, pC->tex[0].t );	// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pC->tex[1].s, pC->tex[1].t );	// tx1
            } else {
	    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texC[0].s, texC[0].t );		// tx0
	    	    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texC[1].s, texC[1].t );		// tx1
	        }
    	}

		KniRC->lastSpecRendered = 0;
		KniRC->prevC = (RfBits & BIT_RC_NEVER_AUTOSTRIP) ? 0 : pC;

		/*
		* Command FIFO cleanup
		*/
		KniRC->dwCmdFifoRoom -= (DWORD)lpCmdfifo - (DWORD)KniRC->lpCmdFifoPtr;
		KniRC->lpCmdFifoPtr = lpCmdfifo;
   	} // end of one pass tri's
   	else 
   	{	/*******************************************************************
    	*   two pass
	    *******************************************************************/
        lpSpecBuff = KniRC->lpSpecBuff;
    	if ( autostripTri ) {
#ifdef DEBUG
			++TnlStats_SpecPartailTrisRendered;
#endif
			/* Partial Triangle */
    	    SETDW( lpCmdfifo[0], CmdContDiff );		// diffuse triangle continue command
    	    SETDW( lpSpecBuff[0], CmdContSpec );	// specular triangle continue command
        	/* Vertex C */
			fTmp = pC->y;
	        SET1x2FDW( lpCmdfifo[1], lpSpecBuff[1], pC->x );   	// x
	        SET1x2FDW( lpCmdfifo[2], lpSpecBuff[2], fTmp );   	// y
			lpCmdfifo += 3;		lpSpecBuff += 3;
			if ( RfBits & BIT_RC_COLOR ) {
				SETDW( *lpCmdfifo++, diffuseC );				// diffuse
				SETDW( *lpSpecBuff++, specularC );		   		// specular
			}
			if ( RfBits & BIT_RC_REQUIRES_Z )			SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->z );	// z
			if ( RfBits & BIT_RC_REQUIRES_WFBI )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->wfbi );	// wfbi
			if ( ! (RfBits & BIT_RC_ALPHA) ) {
		        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pC->w );						// w
    	   	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    		    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pC->tex[0].s, pC->tex[0].t );	// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pC->tex[1].s, pC->tex[1].t );	// tx1
	            } else {
		    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texC[0].s, texC[0].t );		// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texC[1].s, texC[1].t );		// tx1
	        	}
   		    } else {
				if ( RfBits & BIT_RC_REQUIRES_W )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->w );				// w
				if (! (RfBits & BIT_RC_WRAP_VALID)) {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pC->tex[0].s, pC->tex[0].t );	// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pC->tex[1].s, pC->tex[1].t );	// tx1
				} else {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texC[0].s, texC[0].t );		// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texC[1].s, texC[1].t );		// tx1
    		    }
			}
	    } // end of partial 2-pass tri
	    else 
	    {
#ifdef DEBUG
			++TnlStats_SpecFullTrisRendered;
#endif
    	    /* Full Triangle */
    	    SETDW( lpCmdfifo[0], KniRC->CmdStartDiff );	// diffuse triangle start command
    	    SETDW( lpSpecBuff[0], KniRC->CmdStartSpec );	// specular triangle start command
        	/* Vertex A */
			fTmp = pA->y;
	        SET1x2FDW( lpCmdfifo[1], lpSpecBuff[1], pA->x );	// x
	        SET1x2FDW( lpCmdfifo[2], lpSpecBuff[2], fTmp );		// y
			lpCmdfifo += 3;		lpSpecBuff += 3;
			if ( RfBits & BIT_RC_COLOR ) {
				SETDW( *lpCmdfifo++, diffuseA );				// diffuse
				SETDW( *lpSpecBuff++, specularA );				// specular
			}
			if ( RfBits & BIT_RC_REQUIRES_Z )			SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pA->z );	// z
			if ( RfBits & BIT_RC_REQUIRES_WFBI )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pA->wfbi );	// wfbi
			if ( ! (RfBits & BIT_RC_ALPHA) ) {
		        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pA->w );						// w
    	   	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    		    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pA->tex[0].s, pA->tex[0].t );	// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pA->tex[1].s, pA->tex[1].t );	// tx1
	            } else {
		    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texA[0].s, texA[0].t );		// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texA[1].s, texA[1].t );		// tx1
	        	}
   		    } else {
				if ( RfBits & BIT_RC_REQUIRES_W )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pA->w );				// w
				if (! (RfBits & BIT_RC_WRAP_VALID)) {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pA->tex[0].s, pA->tex[0].t );	// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pA->tex[1].s, pA->tex[1].t );	// tx1
				} else {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texA[0].s, texA[0].t );		// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texA[1].s, texA[1].t );		// tx1
    		    }
			}
        	/* Vertex B */
			fTmp = pB->y;
	        SET1x2FDW( lpCmdfifo[0], lpSpecBuff[0], pB->x );	// x
	        SET1x2FDW( lpCmdfifo[1], lpSpecBuff[1], fTmp );		// y
			lpCmdfifo += 2;		lpSpecBuff += 2;
			if ( RfBits & BIT_RC_COLOR ) {
				SETDW( *lpCmdfifo++, diffuseB );				// diffuse
				SETDW( *lpSpecBuff++, specularB );		 	  	// specular
			}
			if ( RfBits & BIT_RC_REQUIRES_Z )			SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pB->z );	// z
			if ( RfBits & BIT_RC_REQUIRES_WFBI )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pB->wfbi );	// wfbi
			if ( ! (RfBits & BIT_RC_ALPHA) ) {
		        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pB->w );						// w
    	   	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    		    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pB->tex[0].s, pB->tex[0].t );	// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pB->tex[1].s, pB->tex[1].t );	// tx1
	            } else {
		    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texB[0].s, texB[0].t );		// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texB[1].s, texB[1].t );		// tx1
	        	}
   		    } else {
				if ( RfBits & BIT_RC_REQUIRES_W )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pB->w );				// w
				if (! (RfBits & BIT_RC_WRAP_VALID)) {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pB->tex[0].s, pB->tex[0].t );	// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pB->tex[1].s, pB->tex[1].t );	// tx1
				} else {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texB[0].s, texB[0].t );		// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texB[1].s, texB[1].t );		// tx1
    		    }
			}
        	/* Vertex C */
			fTmp = pC->y;
	        SET1x2FDW( lpCmdfifo[0], lpSpecBuff[0], pC->x );	// x
	        SET1x2FDW( lpCmdfifo[1], lpSpecBuff[1], fTmp );		// y
			lpCmdfifo += 2;		lpSpecBuff += 2;
			if ( RfBits & BIT_RC_COLOR ) {
				SETDW( *lpCmdfifo++, diffuseC );				// diffuse
				SETDW( *lpSpecBuff++, specularC );			   	// specular
			}
			if ( RfBits & BIT_RC_REQUIRES_Z )			SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->z );	// z
			if ( RfBits & BIT_RC_REQUIRES_WFBI )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->wfbi );	// wfbi
			if ( ! (RfBits & BIT_RC_ALPHA) ) {
		        if ( RfBits & BIT_RC_REQUIRES_W )       SETFDW( *lpCmdfifo++, pC->w );						// w
    	   	    if (! (RfBits & BIT_RC_WRAP_VALID)) {
	    		    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, pC->tex[0].s, pC->tex[0].t );	// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, pC->tex[1].s, pC->tex[1].t );	// tx1
	            } else {
		    	    if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x1FDW( lpCmdfifo, texC[0].s, texC[0].t );		// tx0
	    		    if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x1FDW( lpCmdfifo, texC[1].s, texC[1].t );		// tx1
	        	}
   		    } else {
				if ( RfBits & BIT_RC_REQUIRES_W )		SET1x2FDW( *lpCmdfifo++, *lpSpecBuff++, pC->w );				// w
				if (! (RfBits & BIT_RC_WRAP_VALID)) {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pC->tex[0].s, pC->tex[0].t );	// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, pC->tex[1].s, pC->tex[1].t );	// tx1
				} else {
					if ( RfBits & BIT_RC_REQUIRES_TX0 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texC[0].s, texC[0].t );		// tx0
					if ( RfBits & BIT_RC_REQUIRES_TX1 )	SET2x2FDW( lpCmdfifo, lpSpecBuff, texC[1].s, texC[1].t );		// tx1
    		    }
			}
    	} // end of full 2-pass tri

		/*
		* Command FIFO cleanup
		*/
		KniRC->dwCmdFifoRoom -= (DWORD)lpCmdfifo - (DWORD)KniRC->lpCmdFifoPtr;
		KniRC->lpCmdFifoPtr = lpCmdfifo;

		/*******************************************************************
		*   Dump the specular buffer if needed
		*******************************************************************/
		/* Check if we need to dump the specular buffer */
		KniRC->lpSpecBuff = lpSpecBuff;
		if ( lpSpecBuff >= KniRC->SpecBufferThreshold ) {
#ifdef ASM_RENDERING_CODE
			TL_RENDER_DumpSpecBuffer_Asm (&pRc->tl.KniRC);
#else
			TL_RENDER_DumpSpecularBuffer (pRc);
#endif
			KniRC->prevC = 0;       // forces the next tri to be a full tri
        } else {
			KniRC->lastSpecRendered = -1;
			KniRC->prevC = (RfBits & BIT_RC_NEVER_AUTOSTRIP) ? 0 : pC;
        }
	} // end of 2-pass tri


}   // end of TL_RENDER_Default_NoClip()



/****************************************************************************************
*   Rendering Cleanup Function
*
****************************************************************************************/
void TL_RENDER_Cleanup ( RC* pRc )
{
    // dump the specular buffer if needed
//    if ((pRc->tl.KniRC.RfBits & BIT_RC_SPEC_AND_TEXTURES) && (pRc->tl.KniRC.lpSpecBuff != pRc->tl.KniRC.lpSpecBuffBase)) {
    if ((pRc->tl.KniRC.RfBits & BIT_RC_SPECULAR) && (pRc->tl.KniRC.lpSpecBuff != pRc->tl.KniRC.lpSpecBuffBase)) {	// SPECULARFIX!!!
#ifdef ASM_RENDERING_CODE
		TL_RENDER_DumpSpecBuffer_Asm (&pRc->tl.KniRC);
#else
		TL_RENDER_DumpSpecularBuffer (pRc);
#endif
    }

	TNL_SYNC_TNL_TO_GBL(pRc);
}


/****************************************************************************************
*   DumpSpecularBuffer Function
****************************************************************************************/
#define SPECULAR_SWITCH_ENTRIES (1+10+1 + 1+9+1)	// need these for switching back and forth
void TL_RENDER_DumpSpecularBuffer ( RC* pRc )
{
    SETUP_PPDEV ( pRc )
    DWORD   RfBits = pRc->tl.KniRC.RfBits;
    DWORD   *lpCmdfifo;                                     // local command fifo pointer
    DWORD   *lpSpecBuffStart = pRc->tl.KniRC.lpSpecBuffBase;   // start of the specular buffer
    DWORD   *lpSpecBuffEnd = pRc->tl.KniRC.lpSpecBuff;         // current pointer
    DWORD   fbzMode, combineModeFBI, fbzColorPath;
	DWORD	requiredRoom = ((DWORD)lpSpecBuffEnd - (DWORD)lpSpecBuffStart) + (SPECULAR_SWITCH_ENTRIES * sizeof(DWORD));

#ifdef DEBUG
  static DWORD lastEndPtr, lastRoom;
#endif

    /*
    * Command FIFO setup
    */
#ifdef DEBUG
	TnlStats_SpecBuffWrites += requiredRoom/sizeof(DWORD);
#endif
	TNL_CMDFIFO_CHECKBUMP(pRc, requiredRoom);
	if ( (requiredRoom) > pRc->tl.KniRC.dwCmdFifoRoom )
	{
		tnl_fifo_make_room( pRc, requiredRoom/sizeof(DWORD) );
	}
	lpCmdfifo = pRc->tl.KniRC.lpCmdFifoPtr;


    /*******************************************************************
    * setup for specular tri's
    *******************************************************************/
    // Where FOG() = fog function
    // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]
    //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1
    //    Pass 2    =                       (1 - AlphaFog)T2
    if ( RfBits & BIT_RC_FOG_ENABLE ) {
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
        SETDW( *lpCmdfifo++, ((pRc->sst.fogMode & ~SST_FOGMULT) | SST_FOGADD) );
    }

    // let's just add in the color ignoring the effect on alpha blending for the time being
    // (Src * 1 + Dst * 1)
    SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, alphaMode, 0xF ) );
    if ( RfBits & BIT_RC_ALPHA ) {
        SETDW( *lpCmdfifo++, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
    } else {
        SETDW( *lpCmdfifo++, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
    }

    if ( (RfBits & (BIT_RC_Z_ENABLE | BIT_RC_Z_WRITE_ENABLE)) == (BIT_RC_Z_ENABLE | BIT_RC_Z_WRITE_ENABLE) ) {
//    if ( (RfBits & BIT_RC_Z_ENABLE) && (RfBits & BIT_RC_Z_WRITE_ENABLE) ) {	// fixed this
        // if z-buffering then this triangle z values equals the values written on pass 1
        fbzMode = (pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_GT)) | SST_ZFUNC_EQ;
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
        SETDW( *lpCmdfifo++, fbzMode );
    }

    // texture mapping off and texture blending off but keep alpha the same
    if ( RfBits & BIT_RC_IS_NAPALM ) {
#ifdef NEW_CCU
        combineModeFBI = (  (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE |
                            ~(SST_CM_CC_OTHERSELECT | SST_CM_CC_LOCALSELECT | SST_CM_CC_MSELECT_7 | SST_CM_CC_INVERT_OTHER |
                              SST_CM_CC_INVERT_LOCAL | SST_CM_CC_OUTSHIFT | SST_CM_CC_INVERT_ADD_LOCAL)) 
                            & pRc->sst.combineModeFBI );
        combineModeFBI |= SST_CM_CC_OTHERSELECT_IRGB; //Use iterated color (specular color)
        //fbzcolorpath: pass iterated color and leave alpha the same.
        fbzColorPath = pRc->sst.fbzColorPath & (~SST_CCOMBINE);
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
        SETDW( *lpCmdfifo++, combineModeFBI );
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath, 0x1 ) );
        SETDW( *lpCmdfifo++, fbzColorPath );
#else
        combineModeFBI = (  SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | SST_CM_CCA_OTHERSELECT_TA | 
                            SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE ) 
                            & pRc->sst.combineModeFBI; 
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
        SETDW( *lpCmdfifo++, combineModeFBI );
#endif
    } else {
        fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
        SETDW( *lpCmdfifo++, fbzColorPath );
    }


    /*******************************************************************
    * dump the specular buffer 
    *******************************************************************/
    do {
		SETDW( *lpCmdfifo++, *lpSpecBuffStart++ );
    } while ( lpSpecBuffStart < lpSpecBuffEnd );
/* assembly is much faster but it turns off optimizations for the rest of the function
	_asm {
		mov		esi, dword ptr [lpSpecBuffStart]
		mov		edi, dword ptr [lpCmdfifo]
		mov		ecx, dword ptr [numSpecBuffEntries]
		rep		movsd
		mov		dword ptr [lpCmdfifo], edi
	}
*/
    // reset the specular buffer pointer back to the base
    pRc->tl.KniRC.lpSpecBuff = pRc->tl.KniRC.lpSpecBuffBase;


    /*******************************************************************
    * restore everything back to the way it was
    *******************************************************************/
    if ( RfBits & BIT_RC_IS_NAPALM ) {
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
        SETDW( *lpCmdfifo++, pRc->sst.combineModeFBI );
#ifdef NEW_CCU
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath,1 ) );
        SETDW( *lpCmdfifo++, pRc->sst.fbzColorPath );
#endif
    } else {
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
        SETDW( *lpCmdfifo++, pRc->sst.fbzColorPath );
    }

    if ( RfBits & BIT_RC_FOG_ENABLE ) {
        SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
        SETDW( *lpCmdfifo++, pRc->sst.fogMode );
    }

    SETDW( *lpCmdfifo++, CMDFIFO_BUILD_PK1( 2, 1, alphaMode, 0xF ) );
    SETDW( *lpCmdfifo++, pRc->sst.alphaMode );
    SETDW( *lpCmdfifo++, pRc->sst.fbzMode );

    /*
    * Command FIFO cleanup
    */
#ifdef DEBUG
  lastEndPtr = (DWORD)lpCmdfifo;
  lastRoom = ((DWORD)lpCmdfifo - (DWORD)CMDFIFOPTR) / 4;
#endif
	pRc->tl.KniRC.dwCmdFifoRoom -= (DWORD)lpCmdfifo - (DWORD)pRc->tl.KniRC.lpCmdFifoPtr;
	pRc->tl.KniRC.lpCmdFifoPtr = lpCmdfifo;

} // end of TL_RENDER_DumpSpecularBuffer()



#endif	//#if ( DX >= 7 ) && defined(TnL_HAL)
