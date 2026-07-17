/*-*-c++-*-*/
#include "vxd.h"
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
** $Revision: 7$
** $Date: 10/11/00 8:08:52 PM$
*/

#include <h3.h>
#include "h3sim.h"

#ifdef WINSIM
#include "h3asm.h"
extern unsigned int dwHostV3Base0;
extern int fFake32bppOverlay;
#endif // WINSIM

int __pixin, __pixout;	// HACK: global pixel counters for statistics
int __nopCount=1;
int __fastfillCount=1;
int __triCount=1;			// diagnostic triangle counter
int __spanCount;
static int pixCount;			// diagnostic pixel counter
static int xDir;			// current triangle X scanning direction

//----------------------------------------------------------------------
// recompute the BUSY bits in the STATUS register
//----------------------------------------------------------------------
void csimRecomputeBusy(SstRegs *sst)
{
    sst->status &= ~SST_BUSY;	// clear BUSY status and recompute
    if ( sst->status & (SST_FBI_BUSY | SST_TMU_BUSY) )
        sst->status |= SST_BUSY;
#ifdef CVG
    if ( sst->cmdFifoHoles || sst->cmdFifoDepth )
        sst->status |= SST_BUSY;
#else	// H3
    if ( sst->status & SST_GUI_BUSY )
        sst->status |= SST_BUSY;
    if ( CSIM_PRIVATE( sst )->cmd.cmdFifo0.depth )
        sst->status |= SST_BUSY;
    if ( CSIM_PRIVATE( sst )->cmd.cmdFifo1.depth )
        sst->status |= SST_BUSY;
#endif
}


//This returns 1 if the pixel is inside the guardband 
//  it returns 0 if it's outside the guardband
int sstGuardbandClip(SstRegs *sst, int x, int y)
{
    if ( sst->renderMode & SST_RM_ENGUARDBAND )
    {
        if ( !(sst->fbzMode & SST_ENRECTCLIP) )
            GDBG_ERROR( "sstRectClip","SST_RM_ENGUARDBAND set without SST_ENRECTCLIP\n" );
        if ( sst->clipLeftRight1 & 0x00010001 )
            GDBG_ERROR( "sstRectClip","clipLeftRight1 set to odd pixel boundary\n" );
        if ( x < (signed)HIWORD( sst->clipLeftRight1 ) ||
            !(x < (signed)LOWORD( sst->clipLeftRight1 )) ||
            y < (signed)HIWORD( sst->clipBottomTop1 ) ||
            !(y < (signed)LOWORD( sst->clipBottomTop1 )) )
        {
            GDBG_INFO( 160,"pixel %d,%d guardband clipped out\n",x,y );
            return 0;
        }
    }
    return( 1 );
}

//----------------------------------------------------------------------
// rectangle clip if enabled, return 1 if inside, 0 if clipped out
// also check stipple register while we are at it
// NOTE: lfb accesses call this routine if ENPIXPIPE=1
//----------------------------------------------------------------------
int
sstRectClip(SstRegs *sst, int x, int y)
{
    unsigned int stip = sst->stipple;	// load stipple reg

    sst->stats.fbiPixelsIn++;		// statistic counter
    sst->stats.fbiPixelsIn &= 0xFFFFFF;
    CSIM_PRIVATE( sst )->environment.valid = 0;	// invalidate GUI display
    GDBG_INFO( 168,"\t--------fbiPixelsIn = %d --------\n",sst->stats.fbiPixelsIn );

    if ( ! (sst->fbzMode & SST_ENSTIPPLEPATTERN) )	// rotate stipple
        sst->stipple = (stip<<1) | (stip>>31);

    if ( sst->fbzMode & SST_ENSTIPPLE )
    {	// if stipple is enabled
        if ( sst->fbzMode & SST_ENSTIPPLEPATTERN )
        {
            stip >>= (y & 3)<<3;		// isolate byte based on Y
            if ( !(stip & (0x80>>(x&7))) )
            {	// isolate bit on X
                GDBG_INFO( 160,"pixel %d,%d patterned out\n",x,y );
                return 0;
            }
        }
        else
        {
            if ( !(stip & 0x80000000) )
            {		// if MSB==0 abort pixel
                GDBG_INFO( 160,"pixel %d,%d stippled out\n",x,y );
                return 0;
            }
        }
    }
    if ( sst->fbzMode & SST_ENRECTCLIP )
    {
        if ( x < (signed)HIWORD( sst->clipLeftRight ) ||
            !(x < (signed)LOWORD( sst->clipLeftRight )) ||
            y < (signed)HIWORD( sst->clipBottomTop ) ||
            !(y < (signed)LOWORD( sst->clipBottomTop )) )
        {
            GDBG_INFO( 160,"pixel %d,%d rect-clipped out\n",x,y );
            return 0;
        }
        else if ( sst->clipLeftRight1 & SST_ENRECTCLIP1 )
        {
            // Dealing with H3, check 2nd clip rectangle
            FxU32 ex_mode = (sst->clipBottomTop1 & SST_RECTCLIP1_EX) != 0;

            if ( ( (x < (signed)(HIWORD( sst->clipLeftRight1 ) & 0x0fff))||
                !(x < (signed) LOWORD( sst->clipLeftRight1 ))	  ||
                (y < (signed)(HIWORD( sst->clipBottomTop1 ) & 0x0fff))||
                !(y < (signed) LOWORD( sst->clipBottomTop1 ))) ^ ex_mode )
            {
                GDBG_INFO( 160,"pixel %d,%d rect1-clipped out\n",x,y );
                return 0;
            }
        }
    }
    return 1;
}

//----------------------------------------------------------------------
// adjust a parameter (64-bit) by dp*dx where dx is in the range [-7,+8]
//----------------------------------------------------------------------
static void _adjust64(volatile FxI64 *p64, FxI64 dp, int dx)
{
    FxI64 s;

#if 0
        *p64 = *p64 + ((dx * dp)>>SST_XY_FRACBITS);
#else
    dp = FX_SGNEXT64( dp,SST_ST64_SIZE-1 );
    if ( dx < 0 )
    {
        dp = FX_NEG64( dp );
        dx = -dx;
    }
    if ( dx & 1 )
        s = dp;
    else
        FX_SET64( s,0,0 );
    dp = FX_SHL64( dp,1 );
    if ( dx & 2 ) s = FX_ADD64( s,dp );
    dp = FX_SHL64( dp,1 );
    if ( dx & 4 ) s= FX_ADD64( s,dp );
    dp = FX_SHL64( dp,1 );
    if ( dx & 8 ) s = FX_ADD64( s,dp );

    s = FX_SHR64( s,SST_XY_FRACBITS );
    *p64 = FX_ADD64( *p64,s );
#endif
}

//--------------------------------------
// adjust a parameter (64-bit) by dp*dx 
//--------------------------------------
static void _adjust64_z(volatile FxI64 *p64, FxI64 dp, int dx)
{
    FxI64 s;

    //FX_SGNEXT64 only sign extends negative values (i.e. replicates a 1 up to the MSB)
    //FX_SGNEXT64 doesn't sign extend positive values (i.e. replicates a 0 up to the MSB)
    dp = FX_AND64( dp, SST_MASK64( 48 ) );

    dp = FX_SGNEXT64( dp,SST_Z64_SIZE-1 );
    if ( dx < 0 )
    {
        dp = FX_NEG64( dp );
        dx = -dx;
    }
    if ( dx & 1 )
        s = dp;
    else
        FX_SET64( s,0,0 );
    dp = FX_SHL64( dp,1 );
    if ( dx & 2 ) s = FX_ADD64( s,dp );
    dp = FX_SHL64( dp,1 );
    if ( dx & 4 ) s= FX_ADD64( s,dp );
    dp = FX_SHL64( dp,1 );
    if ( dx & 8 ) s = FX_ADD64( s,dp );

    s = FX_SHR64( s,SST_XY_FRACBITS );
    *p64 = FX_ADD64( *p64,s );
}


//----------------------------------------------------------------------
// mask all registers that are < 32 or 64 bits
//----------------------------------------------------------------------
static void _maskRegs(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.edge0.e &= SST_MASK( SST_E_SIZE );
    cp->fbiData.edge1.e &= SST_MASK( SST_E_SIZE );
    cp->fbiData.spanFbi.x &= SST_MASK( SST_XY_INTBITS );
    cp->fbiData.spanFbi.y &= SST_MASK( SST_XY_INTBITS );
    cp->fbiData.spanFbi.r &= SST_MASK( SST_RGBA_SIZE );
    cp->fbiData.spanFbi.g &= SST_MASK( SST_RGBA_SIZE );
    cp->fbiData.spanFbi.b &= SST_MASK( SST_RGBA_SIZE );
    cp->fbiData.spanFbi.a &= SST_MASK( SST_RGBA_SIZE );
    cp->fbiData.spanFbi.z64 = FX_AND64( cp->fbiData.spanFbi.z64,SST_MASK64( SST_Z64_SIZE ) );

    //Make sure that we're backwards compatible in 16bpp
    if ( (sst->renderMode & SST_RM_3D_MODE) != SST_RM_32BPP )
        cp->fbiData.spanFbi.z64=FX_AND64( cp->fbiData.spanFbi.z64, 
            FX_CREATE64( 0x0000FFFF, 0xFFFF0000 ) );

    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrex.s64 = FX_AND64( td->spanTrex.s64,SST_MASK64( SST_ST64_SIZE ) );
        td->spanTrex.t64 = FX_AND64( td->spanTrex.t64,SST_MASK64( SST_ST64_SIZE ) );
        td->spanTrex.w64 = FX_AND64( td->spanTrex.w64,SST_MASK64( SST_W64_SIZE ) );
    }
}

//----------------------------------------------------------------------
// setup an edge for iteration
//----------------------------------------------------------------------
void _setupEdge(SstRegs *sst, volatile struct edgeRec *e, volatile vtxRec *v0, volatile vtxRec *v1)
{
    FXUNUSED( sst );
    // dedx, dedy 14.4 signed
    e->dedy = SIGN_EXTEND( v0->x,SST_XY_SIZE ) - SIGN_EXTEND( v1->x,SST_XY_SIZE );
    e->dedx = SIGN_EXTEND( v1->y,SST_XY_SIZE ) - SIGN_EXTEND( v0->y,SST_XY_SIZE );
    e->e = e->dedx * (0x8 - (v0->x & 0xF));		// 14.8 format
    //    e->e = SIGN_EXTEND(e->e,SST_XY_SIZE+SST_XY_FRACBITS+2);
    e->e += e->dedy * (0x8 - (v0->y & 0xF));		// 15.8 format
    e->e >>= 4;						// 15.4 format
    e->dedx &= SST_MASK( SST_E_SIZE );
    e->dedy &= SST_MASK( SST_E_SIZE );
    e->e    &= SST_MASK( SST_E_SIZE );			// 28.4
}

//----------------------------------------------------------------------
// setup a triangle for iteration by setting up the first two edges
// and loading the span parameter registers from main PCI regs
//----------------------------------------------------------------------
void sst1Setup(SstRegs *sst)
{
    int dx,dy;
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.spanFbi.x = sst->vA.x >> SST_XY_FRACBITS;	// save starting vertex
    cp->fbiData.spanFbi.y = sst->vA.y >> SST_XY_FRACBITS;	// x and y integer loc.
    dx = (0x8 - (sst->vA.x & 0xF));	// signed .4 distance from pixel center
    dy = (0x8 - (sst->vA.y & 0xF));

    cp->fbiData.spanFbi.r = sst->r;
    cp->fbiData.spanFbi.g = sst->g;
    cp->fbiData.spanFbi.b = sst->b;
    cp->fbiData.spanFbi.a = sst->a;
    cp->fbiData.spanFbi.z64 = cp->fbiData.z64;
    cp->fbiData.spanFbi.w64 = cp->fbiData.w64;
    if ( sst->fbzColorPath & SST_PARMADJUST )
    {	// subpixel parameter correction
        cp->fbiData.spanFbi.r += ((dx * SIGN_EXTEND( sst->drdx,SST_RGBA_SIZE ))>>SST_XY_FRACBITS) +
            ((dy * SIGN_EXTEND( sst->drdy,SST_RGBA_SIZE ))>>SST_XY_FRACBITS);
        cp->fbiData.spanFbi.g += ((dx * SIGN_EXTEND( sst->dgdx,SST_RGBA_SIZE ))>>SST_XY_FRACBITS) +
            ((dy * SIGN_EXTEND( sst->dgdy,SST_RGBA_SIZE ))>>SST_XY_FRACBITS);
        cp->fbiData.spanFbi.b += ((dx * SIGN_EXTEND( sst->dbdx,SST_RGBA_SIZE ))>>SST_XY_FRACBITS) +
            ((dy * SIGN_EXTEND( sst->dbdy,SST_RGBA_SIZE ))>>SST_XY_FRACBITS);
        cp->fbiData.spanFbi.a += ((dx * SIGN_EXTEND( sst->dadx,SST_RGBA_SIZE ))>>SST_XY_FRACBITS) +
            ((dy * SIGN_EXTEND( sst->dady,SST_RGBA_SIZE ))>>SST_XY_FRACBITS);

        //Make sure that we're backwards compatible in 16bpp
        if ( (sst->renderMode & SST_RM_3D_MODE) != SST_RM_32BPP )
        {
            cp->fbiData.spanFbi.z64=FX_AND64( cp->fbiData.spanFbi.z64, 
                FX_CREATE64( 0x0000FFFF, 0xFFFF0000 ) );
            cp->fbiData.dzdx64=FX_AND64( cp->fbiData.dzdx64, 
                FX_CREATE64( 0x0000FFFF, 0xFFFF0000 ) );
        }
        _adjust64_z( &cp->fbiData.spanFbi.z64, cp->fbiData.dzdx64, dx );

        //Make sure that we're backwards compatible in 16bpp
        if ( (sst->renderMode & SST_RM_3D_MODE) != SST_RM_32BPP )
        {
            cp->fbiData.spanFbi.z64=FX_AND64( cp->fbiData.spanFbi.z64, 
                FX_CREATE64( 0x0000FFFF, 0xFFFF0000 ) );
            cp->fbiData.dzdy64=FX_AND64( cp->fbiData.dzdy64, 
                FX_CREATE64( 0x0000FFFF, 0xFFFF0000 ) );
        }
        _adjust64_z( &cp->fbiData.spanFbi.z64, cp->fbiData.dzdy64, dy );

        _adjust64( &cp->fbiData.spanFbi.w64,cp->fbiData.dwdx64,dx );
        _adjust64( &cp->fbiData.spanFbi.w64,cp->fbiData.dwdy64,dy );

        sst->r = cp->fbiData.spanFbi.r;
        sst->g = cp->fbiData.spanFbi.g;
        sst->b = cp->fbiData.spanFbi.b;
        sst->a = cp->fbiData.spanFbi.a;
        sst->z = FX_LO64( FX_SHR64( cp->fbiData.z64, 16 ) ); // 4.44 to 4.28

        cp->fbiData.z64 = cp->fbiData.spanFbi.z64;
        cp->fbiData.w64 = cp->fbiData.spanFbi.w64;
    }

    if ( sst->fbzColorPath & SST_ENTEXTUREMAP )
        for ( i=0; i < cp->info->numberTmus; i++ )
        {
            TmuData *td = TMU_PRIVATE( cp->trex+i );
            td->spanTrex.s64 = td->s64;
            td->spanTrex.t64 = td->t64;
            td->spanTrex.w64 = td->w64;
            if ( sst->fbzColorPath & SST_PARMADJUST )
            {// subpixel parameter correction
                _adjust64( &td->spanTrex.s64,td->dsdx64,dx );
                _adjust64( &td->spanTrex.s64,td->dsdy64,dy );
                _adjust64( &td->spanTrex.t64,td->dtdx64,dx );
                _adjust64( &td->spanTrex.t64,td->dtdy64,dy );
                _adjust64( &td->spanTrex.w64,td->dwdx64,dx );
                _adjust64( &td->spanTrex.w64,td->dwdy64,dy );
                td->s64 = td->spanTrex.s64;
                td->t64 = td->spanTrex.t64;
                td->w64 = td->spanTrex.w64;
            }
        }
        _maskRegs( sst );
        _setupEdge( sst, &cp->fbiData.edge0, &sst->vA, &sst->vC );	// setup up 2 edges
        _setupEdge( sst, &cp->fbiData.edge1, &sst->vA, &sst->vB );
}

//----------------------------------------------------------------------
// push the state of the _span registers
//----------------------------------------------------------------------
void _push(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    GDBG_INFO( 136,"---push state: x,y=%d,%d\n",
        SIGN_EXTEND( cp->fbiData.spanFbi.x,SST_XY_INTBITS ),
        SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) );
    cp->fbiData.edge0.eSave = cp->fbiData.edge0.e;
    cp->fbiData.edge1.eSave = cp->fbiData.edge1.e;
    cp->fbiData.spanFbiSave = cp->fbiData.spanFbi;
    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrexSave = td->spanTrex;
    }
}

//----------------------------------------------------------------------
// pop the state of the _span registers
//----------------------------------------------------------------------
void _pop(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.edge0.e = cp->fbiData.edge0.eSave;
    cp->fbiData.edge1.e = cp->fbiData.edge1.eSave;
    cp->fbiData.spanFbi = cp->fbiData.spanFbiSave;
    GDBG_INFO( 136,"---pop  state: x,y=%d,%d\n",
        SIGN_EXTEND( cp->fbiData.spanFbi.x,SST_XY_INTBITS ),
        SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) );
    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrex = td->spanTrexSave;
    }
}

//----------------------------------------------------------------------
// iterate the current span up (in Y) one pixel
//----------------------------------------------------------------------
void _up(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.spanFbi.y++;			// iterate Y and error terms
    cp->fbiData.edge0.e += cp->fbiData.edge0.dedy;
    cp->fbiData.edge1.e += cp->fbiData.edge1.dedy;
    cp->fbiData.spanFbi.r += sst->drdy;	// iterate parameters up (in Y) 1 pixel
    cp->fbiData.spanFbi.g += sst->dgdy;
    cp->fbiData.spanFbi.b += sst->dbdy;
    cp->fbiData.spanFbi.a += sst->dady;
    cp->fbiData.spanFbi.z64 = FX_ADD64( cp->fbiData.spanFbi.z64,cp->fbiData.dzdy64 );
    cp->fbiData.spanFbi.w64 = FX_ADD64( cp->fbiData.spanFbi.w64,cp->fbiData.dwdy64 );
    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrex.s64 = FX_ADD64( td->spanTrex.s64,td->dsdy64 );
        td->spanTrex.t64 = FX_ADD64( td->spanTrex.t64,td->dtdy64 );
        td->spanTrex.w64 = FX_ADD64( td->spanTrex.w64,td->dwdy64 );
    }
    _maskRegs( sst );
}

//----------------------------------------------------------------------
// iterate the current span right one pixel
//----------------------------------------------------------------------
void _right(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.spanFbi.x += 1;
    cp->fbiData.edge0.e += cp->fbiData.edge0.dedx;
    cp->fbiData.edge1.e += cp->fbiData.edge1.dedx;
    cp->fbiData.spanFbi.r += sst->drdx;
    cp->fbiData.spanFbi.g += sst->dgdx;
    cp->fbiData.spanFbi.b += sst->dbdx;
    cp->fbiData.spanFbi.a += sst->dadx;
    cp->fbiData.spanFbi.z64 = FX_ADD64( cp->fbiData.spanFbi.z64,cp->fbiData.dzdx64 );
    cp->fbiData.spanFbi.w64 = FX_ADD64( cp->fbiData.spanFbi.w64,cp->fbiData.dwdx64 );
    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrex.s64 = FX_ADD64( td->spanTrex.s64,td->dsdx64 );
        td->spanTrex.t64 = FX_ADD64( td->spanTrex.t64,td->dtdx64 );
        td->spanTrex.w64 = FX_ADD64( td->spanTrex.w64,td->dwdx64 );
    }
    _maskRegs( sst );
}

//----------------------------------------------------------------------
// iterate the current span left one pixel
//----------------------------------------------------------------------
void _left(SstRegs *sst)
{
    FxU32 i;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    cp->fbiData.spanFbi.x -= 1;
    cp->fbiData.edge0.e -= cp->fbiData.edge0.dedx;
    cp->fbiData.edge1.e -= cp->fbiData.edge1.dedx;
    cp->fbiData.spanFbi.r -= sst->drdx;
    cp->fbiData.spanFbi.g -= sst->dgdx;
    cp->fbiData.spanFbi.b -= sst->dbdx;
    cp->fbiData.spanFbi.a -= sst->dadx;
    cp->fbiData.spanFbi.z64 = FX_SUB64( cp->fbiData.spanFbi.z64,cp->fbiData.dzdx64 );
    cp->fbiData.spanFbi.w64 = FX_SUB64( cp->fbiData.spanFbi.w64,cp->fbiData.dwdx64 );
    for ( i=0; i < cp->info->numberTmus; i++ )
    {
        TmuData *td = TMU_PRIVATE( cp->trex+i );
        td->spanTrex.s64 = FX_SUB64( td->spanTrex.s64,td->dsdx64 );
        td->spanTrex.t64 = FX_SUB64( td->spanTrex.t64,td->dtdx64 );
        td->spanTrex.w64 = FX_SUB64( td->spanTrex.w64,td->dwdx64 );
    }
    _maskRegs( sst );
}

//----------------------------------------------------------------------
// draw a pixel, process in each TREX and then FBI(sst)
//----------------------------------------------------------------------
void sstPixel(SstRegs *sst)
{
    unsigned long col0=0,col1=0;	// init both to zero just in case
    int x,y;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    //Make sure that everyone is consistent about the the number
    //of pixels per clock to run in
    if ( sst->fbzColorPath & SST_ENTEXTUREMAP )
    {
        if ( sst->combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK )
        {
            if ( ((cp->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) == 0) ||
                ((cp->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) == 0) )
                GDBG_ERROR( "sstPixel", "Inconsistent pixels per clock fbi=%d tmu0=%d tmu1=%d\n",
                    sst->combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1,
                cp->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1,
                cp->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1);
        }
        else
        {
            if ( ((cp->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) != 0) ||
                ((cp->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) != 0) )
                GDBG_ERROR( "sstPixel", "Inconsistent pixels per clock fbi=%d tmu0=%d tmu1=%d\n",
                    sst->combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1,
                cp->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1,
                cp->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ? 2 : 1);
        }
    }    

    x = SIGN_EXTEND( cp->fbiData.spanFbi.x,SST_XY_INTBITS );
    y = SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS );
    if ( sst->fbzColorPath & SST_ENTEXTUREMAP )
    {
#if MAX_NUM_TMUS > 3
            if ( cp->info->numberTmus > 3 )
            sstTrexColor( cp->trex+3,(unsigned char *)&col1,(unsigned char *)&col0 );
#endif
#if MAX_NUM_TMUS > 2
            if ( cp->info->numberTmus > 2 )
            sstTrexColor( cp->trex+2,(unsigned char *)&col0,(unsigned char *)&col1 );
#endif
        if ( cp->info->numberTmus > 1 )
            sstTrexColor( cp->trex+1,(unsigned char *)&col1,(unsigned char *)&col0 );
        sstTrexColor( cp->trex+0,cp->fbiData.trexIn,(unsigned char *)&col1 );
#if HAL_HSIM
        // if we are simulating hardware in TREX standalone mode, then there
        // is no framebuffer to store pixels, so instead we check all the
        // RGBA values as they come out of TREX0
        // BUT: to simulate real-world flow control we buffer up the texels
        // and check them later on
        if ( halInfo.hsim )
        {
            trexAddToCheckFifo( (cp->fbiData.trexIn[3]<<24) |
                (cp->fbiData.trexIn[0]<<16) |
                (cp->fbiData.trexIn[1]<<8) |
                cp->fbiData.trexIn[2],
                x,y,__triCount,pixCount );
        }
#endif
    }
    if ( sstGuardbandClip( sst, x, y ) )  // first perform guardband clipping
    {
        if ( sstRectClip( sst, x, y ) )   // Then perform the rectclip
        {		
            sstFbiPixel( sst,x,y,FXFALSE );		// light,fog,aa,blend,dither, write ...
        }
    }
}

//----------------------------------------------------------------------
// draw a span, search from seed point, its too hard to explain...
//----------------------------------------------------------------------
static void sstSpan(SstRegs *sst)
{
    unsigned int in0, in1;
    unsigned int major,minor;
    int wasOut0=0, wasOut1=0;	// sticky bits
    int filledOne=0;		// TRUE if we have filled a pixel
    int pushed=0;		// TRUE if we have pushed _span state
    // moved the lookahead stuff outside the check for whether we're inside
    // the triangle in an effort to match the hardware each cycle
    int look_ahead_flag = 0;
    int lookOut0=0;
    int lookOut1=0;
    int suppress_lookahead = 1;		// HACK: global for first time in new triangle
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    FxI32 sliY;

    //Check to see if we own this span, if not skip it.
    if ( !csimChipOwnsPixel( sst, cp->fbiData.spanFbi.y, &sliY ) )
    {
        GDBG_INFO( 133, "SPAN:  y=%d  skipped because not owned by %s\n",
            cp->fbiData.spanFbi.y, cp->environment.name );
        return;
    }


    GDBG_INFO( 133,"SPAN: ------------------- y = %d ---------------------------\n",
        SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) );
    __spanCount++;
    _push( sst );			// push state right off the bat
    minor = sst->triangleCMD & 0x80000000;
    major = minor ^ 0x80000000;	// compute constants

    while ( 1 )
    {
#ifdef GDBG_INFO_ON
        if ( GDBG_GET_DEBUGLEVEL( 135 ) )
            sstPrintSpanRegs( sst,"+",135 );
#endif
        in0 = major ^ (cp->fbiData.edge0.e & 0x80000000);
        in1 = minor ^ (cp->fbiData.edge1.e & 0x80000000);

#if 1

            // don't do lookahead for the first pixel in a triangle scanline
            // hardware lookahead terms are used only for predicting whats up next cycle
            // also need to suppress if we push to a new scanline, change directions
            // if the pixel is out.  Then the first lookahead is looking the wrong
            // direction (lookahead direction is precomputed), so suppress here as well.

            if ( suppress_lookahead )
        {
            suppress_lookahead = 0;
        }
        else
        {
            // X look-ahead optimization, the look ahead error terms can be
            // computed before loop but wasOut* cannot be modified until after
            // we test for the pixel being inside the triangle
            // Scott: note we can both PUSH and POP in same cycle!
            if ( xDir )
            {				// look left
                if ( minor )
                {			// edge1 is on the left side
                    if ( (cp->fbiData.edge1.e - cp->fbiData.edge1.dedx) & 0x80000000 )
                    {
                        GDBG_INFO( 138,"---lookahead left: edge1 is out\n" );
                        lookOut1 |= 0x80000000;
                        look_ahead_flag = 1;
                        // goto look_ahead;
                    }
                }
                else
                {
                    if ( (cp->fbiData.edge0.e - cp->fbiData.edge0.dedx) & 0x80000000 )
                    {
                        GDBG_INFO( 138,"---lookahead left: edge0 is out\n" );
                        lookOut0 |= 0x80000000;
                        look_ahead_flag = 1;
                        // goto look_ahead;
                    }
                }
            }
            else
            {				// look right
                if ( minor )
                {			// edge0 is on the right side
                    if ( !((cp->fbiData.edge0.e + cp->fbiData.edge0.dedx) & 0x80000000) )
                    {
                        GDBG_INFO( 138,"---lookahead right : edge0 is out\n" );
                        lookOut0 |= 0x80000000;
                        look_ahead_flag = 1;
                        // goto look_ahead;
                    }
                }
                else
                {
                    if ( !((cp->fbiData.edge1.e + cp->fbiData.edge1.dedx) & 0x80000000) )
                    {
                        GDBG_INFO( 138,"---lookahead right : edge1 is out\n" );
                        lookOut1 |= 0x80000000;
                        look_ahead_flag = 1;
                        // goto look_ahead;
                    }
                }
            }
        }
#endif
        if ( in0 & in1 )
        {			// inside the triangle
            __pixin++;
            pixCount++;
            GDBG_INFO( 134,"===both edges in: x,y=%d,%d  pixel #%d (0x%x) ===\n",
                SIGN_EXTEND( cp->fbiData.spanFbi.x,SST_XY_INTBITS ),
                SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ),
                pixCount,pixCount );
            sstPixel( sst );
            filledOne = 1;
            if ( look_ahead_flag )
            {
                look_ahead_flag = 0;
                wasOut0 |= 0x80000000 & lookOut0;
                wasOut1 |= 0x80000000 & lookOut1;
                goto look_ahead;
            }
            // GMT: SST-1 does not perform Y look-ahead optimization, so disable it
#if YLOOKAHEAD
            // Y look-ahead optimization, check the next scanline's inside status
            if ( wasOut0 | wasOut1 )
            {		// if already was outside once
                in0 = (cp->fbiData.edge0.e + cp->fbiData.edge0.dedy) & 0x80000000;
                in0 ^= major;
                in1 = (cp->fbiData.edge1.e + cp->fbiData.edge1.dedy) & 0x80000000;
                in1 ^= minor;
                GDBG_INFO( 139,"---checking next scan: in0:%x in1:%x\n",in0,in1 );
                if ( in0 & in1 )
                {		// if next scanline in
                    _push( sst );			// push this location
                    pushed = 1;
                }
            }
#endif
        }
        else
        {	// we know that the pixel is outside of at least one edge
            int last_xdir;
            __pixout++;
            wasOut0 |= 0x80000000 & ~in0;
            wasOut1 |= 0x80000000 & ~in1;

            if ( look_ahead_flag )
            {
                look_ahead_flag = 0;
                wasOut0 |= 0x80000000 & lookOut0;
                wasOut1 |= 0x80000000 & lookOut1;
                goto look_ahead;
            }
            look_ahead:
            if ( wasOut0 & wasOut1 )
            {		// if both edges have been OUT
                GDBG_INFO( 136,"---both edges OUT, returning\n" );
                if ( pushed )			// but first pop, if we pushed
                    _pop( sst );			// due to next scanline begin IN
                return;
            }

            GDBG_INFO( 136,"---not in, heading for IN edge\n" );
            if ( filledOne )			// if filled at least one pixel
                _pop( sst );			// pop and step one unit
            last_xdir = xDir;
            xDir = wasOut0 ? minor : major;	// head for the INSIDE edge

            // stepped down a line and are outside triangle, and we just
            // changed directions.  Hardware can't correctly lookahead
            if ( !filledOne && (last_xdir != xDir) )
                suppress_lookahead = 1;
        }

        if ( xDir )				// step in the new direction
            _left( sst );
        else
            _right( sst );
    }
}

//----------------------------------------------------------------------
// draw a triangle by drawing a span and then moving up one pixel
//----------------------------------------------------------------------
static void sstTriangle(SstRegs *sst)
{
    int ystop;
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    xDir = 0;				// always start in same direction (as HW)
    pixCount = 0;			// reset diagnostic pixel counter
    sst->fbiTrianglesOut++;
    sst->fbiTrianglesOut &= 0xFFFFFF;

    //Make sure that if guard-band clipping is enabled that the clip rect is
    //on even coordinates
    if ( sst->renderMode & SST_RM_ENGUARDBAND )
    {
        if ( sst->clipLeftRight1 & (1 << SST_CLIPLEFT_SHIFT) )
            GDBG_INFO( 0, "Warning! clipLeftRight left is odd with guard-banding on!\n" );
        if ( sst->clipLeftRight1 & (1 << SST_CLIPRIGHT_SHIFT) )
            GDBG_INFO( 0, "Warning! clipLeftRight right is odd with guard-banding on!!\n" );
    }

    GDBG_INFO( 191,"triangle #%d\n",sst->fbiTrianglesOut );
    guiKeepAlive( 100 );
    guiChange( cp,1 );		// update triangle stats gui stuff
    if ( cp->environment.skipRendering )
        return;

    // first subTriangle
    ystop = SIGN_EXTEND( sst->vA.y,SST_XY_SIZE );
    ystop += (1<<(SST_XY_FRACBITS-1))-1;	// ALMOST_HALF
    ystop >>= SST_XY_FRACBITS;
    if ( SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) < ystop )
    {
        _up( sst );			// move up (in Y) one scanline
    }

    ystop = SIGN_EXTEND( sst->vB.y,SST_XY_SIZE );
    ystop += (1<<(SST_XY_FRACBITS-1))-1;	// ALMOST_HALF
    ystop >>= SST_XY_FRACBITS;
    GDBG_INFO( 126,"===1st subTriangle: y %d to %d === area:0x%x\n",
        SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ),
        ystop,
        sst->triangleCMD );

    // process spans until y >= yStop, use signed comparison
    while ( SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) < ystop )
    {
        sstSpan( sst );		// draw the span
        if ( cp->fbiData.spanFbi.y == 4095 ) return;
        _up( sst );			// move up (in Y) one scanline
    }

    // setup new minor edge, adjust E to pixel center
    _setupEdge( sst, &cp->fbiData.edge1, &sst->vB, &sst->vC );
    {
        int dx,dy;
        dx = SIGN_EXTEND( cp->fbiData.spanFbi.x,SST_XY_INTBITS ) - 
            SIGN_EXTEND( (sst->vB.x>>SST_XY_FRACBITS),SST_XY_INTBITS );
        dy = SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) -
            SIGN_EXTEND( (sst->vB.y>>SST_XY_FRACBITS),SST_XY_INTBITS );
        cp->fbiData.edge1.e += dx * cp->fbiData.edge1.dedx;
        if ( dy == 0 ) ;
        else if ( dy == 1 )
            cp->fbiData.edge1.e += cp->fbiData.edge1.dedy;
        else
            GDBG_ERROR( "sstTriangle.setup","dy = %d  span.y=%d vB.y=%d\n",
                dy,cp->fbiData.spanFbi.y,
                SIGN_EXTEND( (sst->vB.y>>SST_XY_FRACBITS),SST_XY_INTBITS ) );
        cp->fbiData.edge1.e &= SST_MASK( SST_E_SIZE );
    }

    // second subTriangle
    ystop = SIGN_EXTEND( sst->vC.y,SST_XY_SIZE );
    ystop += (1<<(SST_XY_FRACBITS-1))-1;	// ALMOST_HALF
    ystop >>= SST_XY_FRACBITS;
    GDBG_INFO( 126,"===2nd subTriangle: y %d to %d === area:0x%x\n",
        SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ),
        ystop,
        sst->triangleCMD );

    // process spans until y >= yStop, use signed comparison
    while ( SIGN_EXTEND( cp->fbiData.spanFbi.y,SST_XY_INTBITS ) < ystop )
    {
        sstSpan( sst );		// draw the span
        // special case for when y=4095 and is about to wrap
        if ( cp->fbiData.spanFbi.y == 4095 ) return;
        _up( sst );			// move up (in Y) one scanline
    }
}

//----------------------------------------------------------------------
// FASTFILL command: fill the rectangle defined by the cliprect with color1
// only the writemask and dither features apply
//----------------------------------------------------------------------
static void sstFastFill(SstRegs *sst)
{
#ifdef GDBG_INFO_ON
    unsigned char gdSave[GDBG_MAX_LEVELS];
#endif
    int x,y,xstop,ystop;
    int fbzColorPathSave = sst->fbzColorPath;	// save away fbzColorPath
    unsigned long m = 0x7fff;			// mask for SST96
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    int fbzModeSave = sst->fbzMode;	        // save away fbzMode
    int combineModeSave = sst->combineMode;     // save away combineMode

    currentRenderingChip = sst;

    //Check for fastfills to tiled surfaces in SLI mode
    if((sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) &&
       (sst->colBufferStride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED)
      {
        SstIORegs *sstio = &(CSIM_PRIVATE( sst )->io);
        if(!(sstio->dramInit1 & SST_MCTL_TYPE_SDRAM))
	  {
	    GDBG_ERROR("sstFastFill", "The hw is f'ed and doesn't support fastfills to tiled buffers with SLI enabled\n");
	  }
      }

    //Check for fastfills with dither rotation enabled
    if(sst->renderMode & SST_RM_DITHER_ROTATION)
      GDBG_ERROR("sstFastFill", "The hw is f'ed and doesn't supports fastfills with dither rotation enabled\n");
		 
       
    xstop = LOWORD( sst->clipLeftRight ) & m;
    ystop = LOWORD( sst->clipBottomTop ) & m;
    GDBG_INFO( 126,"===FASTFILL: %d,%d to %d,%d\n",
        HIWORD( sst->clipLeftRight ) & m,HIWORD( sst->clipBottomTop ) & m,
        xstop,ystop );
    if ( cp->environment.skipRendering )
        return;

    sst->fbzColorPath &= ~(SST_RGBSELECT|SST_ASELECT);	// clear out SELECTs
    sst->fbzColorPath |= SST_RGBSEL_C1|SST_ASEL_C1;	// force it to C1

    //Make sure that if we're using the combineMode register that we select
    //C1 for RGB and A
    sst->combineMode = SST_CM_CC_OTHERSELECT_C1_RGB | SST_CM_CCA_OTHERSELECT_C1_A;

#ifdef GDBG_INFO_ON
    // for practical reasons we disable debug info during this loop
    for ( x=127; x<GDBG_MAX_LEVELS; x++ )
    {		// and debug levels
        gdSave[x] = GDBG_GET_DEBUGLEVEL( x );
        GDBG_SET_DEBUGLEVEL( x,0 );
    }
#endif
    csimVideo( cp,FXFALSE );

    if(sst->renderMode & SST_RM_DITHER_ROTATION)
      GDBG_ERROR("sstFastFill", "Can't use dither rotation with fastfills!\n");

    // disable dithering if SST_FASTFILL_DISABLE_DITHER is specified
    // *unless* SDRAM is being used
    if ( sst->fastfillCMD & SST_FASTFILL_DISABLE_DITHER )
    {
        SstIORegs *sstio = &(CSIM_PRIVATE( sst )->io);
        if ( ! (sstio->dramInit1 & SST_MCTL_TYPE_SDRAM) )
        {
            sst->fbzMode &= ~SST_ENDITHER;
        }
    }

    for ( y = HIWORD( sst->clipBottomTop ) & m; y < ystop; y++ )
        for ( x = HIWORD( sst->clipLeftRight ) & m; x < xstop; x++ )
        {
            sstFbiPixel( sst,x,y,FXFALSE );			// process the pixel
            
            if( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
	      {
		cp->environment.aaPrimaryBuffers = FXFALSE;
		sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
		sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
                sstFbiPixel( sst,x,y,FXFALSE );			// process the pixel
		sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
		sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );		
		cp->environment.aaPrimaryBuffers = FXTRUE;
            }
        }
        
        csimVideo( cp,FXTRUE );
        sst->fbzColorPath = fbzColorPathSave;	// restore the fbzColorPath
        sst->fbzMode = fbzModeSave; 		// restore the fbzMode
        sst->combineMode = combineModeSave;

#ifdef GDBG_INFO_ON
        for ( x=127; x<GDBG_MAX_LEVELS; x++ )		// and debug levels
            GDBG_SET_DEBUGLEVEL( x,gdSave[x] );
#endif
}

//----------------------------------------------------------------------
// convenient print routine
//	NOTE: for maximum performance compile without GDBG_INFO_ON
//----------------------------------------------------------------------
void sstPrintOut(SstRegs *sst, int cmdCode)
{
#ifdef GDBG_INFO_ON
    if ( GDBG_GET_DEBUGLEVEL( 125 ) )
    {	// delay this printout until after setup
        sstPrintModes( sst,"sst.GO:", cmdCode );
        sstPrintRegs( sst,"< GO:" );
    }
#endif
}

//----------------------------------------------------------------------
// EXECution procedure
//	NOTE: for maximum performance compile without GDBG_INFO_ON
//----------------------------------------------------------------------
void sstGo(SstRegs *sst, int cmdCode)
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    // GMT: don't bother with this now 
    //    if (!gdbg_get_debuglevel(0)) return;	// if benchmarking then exit now

    cp->cmdCode = cmdCode;
    sst->status |= SST_FBI_BUSY;		// set BUSY status
    // to accurately emulate the real hardware, we do not clear the BUSY bit
    // after a triangle, the hardware likes a NOP command to execute first

    switch ( cmdCode )
    {			// decode command
    case SST_NOPCMD:
        GDBG_INFO( 105,"SST_NOP(%d) #%d (0x%x)\n",
            sst->nopCMD,__nopCount,__nopCount );
        sstPrintOut( sst,cmdCode );
        // reset the pixel counters
        if ( sst->nopCMD & SST_NOP_RESET_PIXEL_STATS )
        {
            sst->stats.fbiPixelsIn = 0;
            sst->stats.fbiChromaFail = 0;
            sst->stats.fbiZfuncFail = 0;
            sst->stats.fbiAfuncFail = 0;
            sst->stats.fbiPixelsOut = 0;
            sst->fbiStencilFail = 0;
        }
        // reset triangle counters
        if ( sst->nopCMD & SST_NOP_RESET_TRIANGLE_STATS )
        {
            sst->fbiTrianglesOut = 0;
        }
        __nopCount++;
        sst->status &= ~SST_FBI_BUSY;	// clear BUSY status
        break ;
    case SST_FASTFILLCMD:
        GDBG_INFO( 105,"SST_FASTFILL: #%d (0x%x)\n",
            __fastfillCount,__fastfillCount );
        sstPrintOut( sst,cmdCode );
        sstFastFill( sst );
        __fastfillCount++;
        sst->status &= ~SST_FBI_BUSY;	// clear BUSY status
        break ;
    case SST_SWAPBUFCMD:		// swap and then show the front buffer
        {
            int was,next, swapint;

            swapint = sst->swapbufferCMD & SST_SWAP_BUFFER_INTERVAL;
            swapint >>= SST_SWAP_BUFFER_INTERVAL_SHIFT;
#ifdef CVG
            if ( !(sst->swapbufferCMD & SST_SWAP_DONT_SWAP) )
            {
                was = sst->status & SST_DISPLAYED_BUFFER;
                was >>= SST_DISPLAYED_BUFFER_SHIFT;
                next = was + 1;
                if ( next >= cp->numColorBuffers )
                    next = 0;
                sst->status &= ~SST_DISPLAYED_BUFFER;
                sst->status |= next<<SST_DISPLAYED_BUFFER_SHIFT;
            }
            // save this code for CVG
            GDBG_INFO( 105,"SST_SWAPBUFFERS: #%d (0x%x)  vSync=%d int=%d display was=%d next=%d\n",
                cp->environment.curBufferSwapCount+1,
                cp->environment.curBufferSwapCount+1,
                sst->swapbufferCMD & SST_SWAP_EN_WAIT_ON_VSYNC,
                swapint,was,next );
#else
            FXUNUSED( was );
            FXUNUSED( next );
            if ( !(sst->swapbufferCMD & SST_SWAP_DONT_SWAP) )
            {
                if ( cp->io.vidProcCfg&SST_OVERLAY_STEREO_EN )
                    GDBG_ERROR( "go","stereo video nyi (PS)\n" );
                cp->io.vidCurrOverlayStartAddr = 
                    (sst->leftOverlayBuf&SST_VIDEO_START_ADDR) << SST_VIDEO_START_ADDR_SHIFT;
            }
            GDBG_INFO( 105,"SST_SWAPBUFFERS: #%d (0x%x)  vSync=%d int=%d\n",
                cp->environment.curBufferSwapCount+1,
                cp->environment.curBufferSwapCount+1,
                sst->swapbufferCMD & SST_SWAP_EN_WAIT_ON_VSYNC,
                swapint );
#endif

#ifdef WINSIM
            // In the sim server environment, we can do the same functionality on a V3
            // We could be also faking overlays...
            if( fFake32bppOverlay )
            {
                // Use desktop address instead
                *(unsigned *)(dwHostV3Base0 + VIDDESKTOPSTARTADDR) = sst->leftOverlayBuf;
            }
            else
            {
                *(unsigned *)(dwHostV3Base0 + SST_3D_OFFSET + SWAPBUFFERCMD) = cmdCode;
            }
#endif // WINSIM

            sstPrintOut( sst,cmdCode );

            sstPrintStats( sst );
            sst->status &= ~SST_FBI_BUSY;	// clear BUSY status
            cp->environment.curBufferSwapCount++;

            // redisplay all the windows    
            if ( halInfo.csim && halInfo.video ) guiRefresh( cp );
            guiChange( cp,0 );		// update GUI displays
            break ;
        }
    case SST_SDRAWTRICMD:		// setup and draw a triangle
        cp->tsuData.vertexCount++;		// inc vertex count
        if ( cp->tsuData.vertexCount > 2 )
        {
            if ( !(sst->sSetupMode & SST_SETUP_FAN) )
            {
                GDBG_INFO( 126,"setup: v[0] = v[1]\n" );
                cp->tsuData.vArray[0] = cp->tsuData.vArray[1];
                cp->tsuData.pingpong ^= 0x80000000;
            }
            GDBG_INFO( 126,"setup: v[1] = v[2]\n" );
            cp->tsuData.vArray[1] = cp->tsuData.vArray[2];
            cp->tsuData.vertexCount = 2;
        }
        GDBG_INFO( 105,"SST_SDRAWTRICMD: [%d] pp=%d %s %s\n",
            cp->tsuData.vertexCount,cp->tsuData.pingpong!=0,
            sst->sSetupMode & SST_SETUP_FAN ? "FAN" : "STRIP",
        sst->sSetupMode & SST_SETUP_DIS_PINGPONG ? "DISpingpong":"");

        sstCopyVertex( sst );
        if ( cp->tsuData.vertexCount < 2 )
            return;		// skip register printouts

        // setup and then fall thru to draw a triangle
        if ( !sstTriangleSetup( sst ) ) return;

    case SST_TRIANGLECMD:		// fill a triangle
        {
            vtxRec originalVa, originalVb, originalVc;
            FxI32 xOffset, yOffset;
	    FxU32 originalR, originalG, originalB, originalA, originalZ;
	    FxI64 originalZ64, originalW64;

            TmuData tmuData[MAX_NUM_TMUS];

            GDBG_INFO( 105,"SST_TRIANGLE(%d) #%d (0x%x)\n",
                sst->triangleCMD,__triCount,__triCount );

            //Record the original vertices
            originalVa = sst->vA;
            originalVb = sst->vB;
            originalVc = sst->vC;

            //Add in the primary x,y perturbation in the aaCtrl register
            xOffset = (sst->aaCtrl & SST_AA_CONTROL_PRIMARY_X_OFFSET) >> SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT;
            yOffset = (sst->aaCtrl & SST_AA_CONTROL_PRIMARY_Y_OFFSET) >> SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT;
            xOffset = SIGN_EXTEND( xOffset, 7 );
            yOffset = SIGN_EXTEND( yOffset, 7 );
            xOffset += SIGN_EXTEND( cp->environment.triangleOffsetX, 7 );
            yOffset += SIGN_EXTEND( cp->environment.triangleOffsetY, 7 );
            sst->vA.x += xOffset;
            sst->vA.y += yOffset;
            sst->vB.x += xOffset;
            sst->vB.y += yOffset;
            sst->vC.x += xOffset;
            sst->vC.y += yOffset;

            /*GDBG_INFO(0, "go Primary: xOffset=0x%x yOffset=0x%x\n", xOffset, yOffset);
                GDBG_INFO(0, "Triangle: 0x%x,0x%x  0x%x,0x%x  0x%x,0x%x\n",
                  sst->vA.x, sst->vA.y, sst->vB.x, sst->vB.y, sst->vC.x, sst->vC.y);*/


            //Make sure that we're not chaining the tmus together if running
            //in 2 ppc
            if ( (globalSST->combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
                (cp->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
                (cp->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) )
            {
                if ( csimActiveTMUs( globalSST ) == 2 )
                {		    
                    csimActiveTMUs( globalSST );
                    GDBG_ERROR( "sstGo", "TMUs chained while running in 2ppc!\n" );		    
                    GDBG_INFO( 0, "tmu0: combineMode=0x%08x  textureMode=0x%08x\n",
                        cp->trex[0].combineMode, cp->trex[0].textureMode );		
                }
            }

            //Make sure that if we're running in SLI with dithering enabled that
            //the bandHeight is >= the dither matrix height
            if ( (sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) && (sst->fbzMode & SST_ENDITHER) )
            {
                if ( sst->fbzMode & SST_DITHER2x2 )
                {
                    if ( cp->environment.sliBandHeight < 2 )
                        GDBG_ERROR( "sstGo", "sliBandHeight of %d is less than height of 2x2 dither matrix\n",
                            cp->environment.sliBandHeight );
                }
                else
                {
                    if ( cp->environment.sliBandHeight < 4 )
                        GDBG_ERROR( "sstGo", "sliBandHeight of %d is less than height of 4x4 dither matrix\n",
                            cp->environment.sliBandHeight );
                }
            }

            //Make sure that we're not trying to use compressed, split textures.
            if ( ((cp->trex[0].tLOD & SST_LOD_TSPLIT) && (cp->trex[0].textureMode & SST_COMPRESSED_TEXTURES)) ||
                ((cp->trex[1].tLOD & SST_LOD_TSPLIT) && (cp->trex[1].textureMode & SST_COMPRESSED_TEXTURES)) )
                GDBG_ERROR( "sstGo", "Trying to use split compressed textures!\n" );

            //Make sure that the FBI isn't using TMU 0's data when
            //texturing is disabled
            if ( !(globalSST->fbzColorPath & SST_ENTEXTUREMAP) )
            {
                if ( ((globalSST->combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TRGB) ||
                    ((globalSST->combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TA) )
                {
                    GDBG_ERROR( "sstGo", "Illegal fbi CC_OTHERSELECT with texturing disabled\n" );
                    GDBG_INFO( 0, "combineMode=0x%08x   fbzColorPath=0x%08x\n",
                        globalSST->combineMode, globalSST->fbzColorPath );
                }

                if ( ((globalSST->combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TRGB) ||
                    ((globalSST->combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TA) )
                {
                    GDBG_ERROR( "sstGo", "Illegal fbi CC_LOCALSELECT with texturing disabled\n" );
                    GDBG_INFO( 0, "combineMode=0x%08x   fbzColorPath=0x%08x\n",
                        globalSST->combineMode, globalSST->fbzColorPath );
                }

                if ( (globalSST->combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_TA )
                {
                    GDBG_ERROR( "sstGo", "Illegal fbi CCA_OTHERSELECT with texturing disabled\n" );
                    GDBG_INFO( 0, "combineMode=0x%08x   fbzColorPath=0x%08x\n",
                        globalSST->combineMode, globalSST->fbzColorPath );
                }

                if ( (globalSST->combineMode & SST_CM_CCA_LOCALSELECT) == SST_CM_CCA_LOCALSELECT_TA )
                {
                    GDBG_ERROR( "sstGo", "Illegal fbi CCA_LOCALSELECT with texturing disabled\n" );
                    GDBG_INFO( 0, "combineMode=0x%08x   fbzColorPath=0x%08x\n",
                        globalSST->combineMode, globalSST->fbzColorPath );
                }
            }

            //Specify that we're drawing the primary triangle
	    currentRenderingChip = sst;
            cp->environment.aaPrimaryBuffers = FXTRUE;

            if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
            {
                FxU32 i;

                GDBG_INFO( 127, "AA primary triangle\n" );
                GDBG_INFO( 128, "go Primary: xOffset=0x%x yOffset=0x%x\n", xOffset, yOffset );
                GDBG_INFO( 128, "Triangle: 0x%x,0x%x  0x%x,0x%x  0x%x,0x%x\n",
                    sst->vA.x, sst->vA.y, sst->vB.x, sst->vB.y, sst->vC.x, sst->vC.y );

                //Back up copies of TMU data for both TMUs. Otherwise, wacky shit like
                //double sub-pixel correction will occur for the second triangle
                for ( i=0; i<cp->info->numberTmus; i++ )
                    memcpy( &tmuData[i], TMU_PRIVATE( cp->trex+i ), sizeof(TmuData) );
            }
	    
	    //Store original, pre sub-pixel adjusted values in case we're rendering
	    //to the secondary buffer as well
	    originalR = sst->r;
	    originalG = sst->g;
	    originalB = sst->b;
	    originalA = sst->a;
	    originalZ = sst->z;
	    originalZ64 = cp->fbiData.z64;
	    originalW64 = cp->fbiData.w64;
	    
            sst1Setup( sst );	        // load params and setup
            sstPrintOut( sst,cmdCode );
            sstTriangle( sst );	// and fill it

            //Render the triangle again if AA is enabled
            if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
            {	
                FxU32 i;

                GDBG_INFO( 127, "AA secondary triangle\n" );


		//Restore original, pre sub-pixel adjusted values
		sst->r = originalR;
		sst->g = originalG;
		sst->b = originalB;
		sst->a = originalA;
		sst->z = originalZ;
		cp->fbiData.z64 = originalZ64;
		cp->fbiData.w64 = originalW64;

                //Restore original TMU data
                for ( i=0; i<cp->info->numberTmus; i++ )
                    memcpy( TMU_PRIVATE( cp->trex+i ), &tmuData[i], sizeof(TmuData) );		

                //Make sure that chip 0's secondary colBufferAddr, and auxBufferAddr
                //are stored in sst
                sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
                sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );

                //Setup the secondary vertices
                xOffset = (sst->aaCtrl & SST_AA_CONTROL_SECONDARY_X_OFFSET) >> SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT;
                yOffset = (sst->aaCtrl & SST_AA_CONTROL_SECONDARY_Y_OFFSET) >> SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT;
                xOffset = SIGN_EXTEND( xOffset, 7 );
                yOffset = SIGN_EXTEND( yOffset, 7 );
                xOffset += SIGN_EXTEND( cp->environment.triangleOffsetX, 7 );
                yOffset += SIGN_EXTEND( cp->environment.triangleOffsetY, 7 );

                sst->vA.x = originalVa.x + xOffset;
                sst->vA.y = originalVa.y + yOffset;
                sst->vB.x = originalVb.x + xOffset;
                sst->vB.y = originalVb.y + yOffset;
                sst->vC.x = originalVc.x + xOffset;
                sst->vC.y = originalVc.y + yOffset;

                GDBG_INFO( 128, "go Secondary: xOffset=0x%x yOffset=0x%x\n", xOffset, yOffset );
                GDBG_INFO( 128, "Triangle: 0x%x,0x%x  0x%x,0x%x  0x%x,0x%x\n",
                    sst->vA.x, sst->vA.y, sst->vB.x, sst->vB.y, sst->vC.x, sst->vC.y );			  

                //Specify that we're drawing the secondary triangle
                cp->environment.aaPrimaryBuffers = FXFALSE;

                //Render the second triangle
                sst1Setup( sst );	        // load params and setup
                sstPrintOut( sst,cmdCode );
                sstTriangle( sst );	// and fill it

                //Restore the primary buffers
                sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
                sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );
            }

            __triCount++;

            //Restore the original vertices
            sst->vA = originalVa;
            sst->vB = originalVb;
            sst->vC = originalVc;	    

            //Restore to default to primary buffers
            cp->environment.aaPrimaryBuffers = FXTRUE;

            // NOTE: we leave the status with FBI_BUSY
            break ;
        }
    case SST_SBEGINTRICMD:		// begin a new tri strip/fan
        cp->tsuData.vertexCount = 0;
        cp->tsuData.pingpong = 0;
        GDBG_INFO( 105,"SST_SBEGINTRICMD: [0]\n" );
        sstCopyVertex( sst );
        return;			// skip register printouts
    default:
        GDBG_ERROR( "sstGo","bad cmdCode = %x\n",cmdCode );
        break;
    }

    //------------------------------------------------------------------
    // all done
    //------------------------------------------------------------------
#ifdef GDBG_INFO_ON
    if ( GDBG_GET_DEBUGLEVEL( 130 ) ) sstPrintRegs( sst,"> GO:" );
    if ( GDBG_GET_DEBUGLEVEL( 125 ) ) gdbg_printf( "\n" );
#endif
    csimRecomputeBusy( sst );
}

