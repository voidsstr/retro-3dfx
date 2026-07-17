#include "vxd.h"
/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Resrved.
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
** $Revision: 4$
** $Date: 10/11/00 8:08:49 PM$
*/

#include <assert.h>
#include <h3.h>
#include "h3sim.h"
#include "rgbfmt.h"

// process complete chroma test, key or range with all options
// return 1 if chroma test passes (pixel should be discarded)
int sstChromaTest(FxU32 r, FxU32 g, FxU32 b, FxU32 key, FxU32 range)
{
    if (range & SST_ENCHROMARANGE) {
        FxU32 rlo, rhi, glo, ghi, blo, bhi; // chromalimits
        FxU32 rpass, gpass, bpass;

            blo = (key        ) & 0xFF;
        glo = (key >>    8) & 0xFF;
        rlo = (key >>   16) & 0xFF;

            bhi = (range      ) & 0xFF;
        ghi = (range >>  8) & 0xFF;
        rhi = (range >> 16) & 0xFF;

        // Assume inclusive mode.
        rpass = (r >= rlo) && (r <= rhi);
        gpass = (g >= glo) && (g <= ghi);
        bpass = (b >= blo) && (b <= bhi);

        // Invert if exclusive mode.
        if (range & SST_CHROMARANGE_RED_EX  ) rpass = !rpass;
        if (range & SST_CHROMARANGE_GREEN_EX) gpass = !gpass;
        if (range & SST_CHROMARANGE_BLUE_EX ) bpass = !bpass;

        // Check for union or intersection modes.
        if (range & SST_CHROMARANGE_BLOCK_OR)
            return rpass || gpass || bpass; // UNION mode.
        else
            return rpass && gpass && bpass; // INTERSECTION mode.
    }
    else { // simple chromaKey test!!!
        return ((r<<16) | (g<<8) | (b)) == (key&0x00FFFFFF);
    } 
}

// return the clamped RGB,A iterator values
void _getAndClampIterators(SstRegs *sst, unsigned char iterators[])
{
    unsigned int ir,ig,ib,ia;   // iterated colors
    int clamp = sst->fbzColorPath & SST_RGBAZ_CLAMP;
    CsimPrivate *cp = CSIM_PRIVATE(sst);

    // load up iterated color
    ir = (cp->fbiData.spanFbi.r >> SST_RGBA_FRACBITS) & 0xFFF;
    ig = (cp->fbiData.spanFbi.g >> SST_RGBA_FRACBITS) & 0xFFF;
    ib = (cp->fbiData.spanFbi.b >> SST_RGBA_FRACBITS) & 0xFFF;
    ia = (cp->fbiData.spanFbi.a >> SST_RGBA_FRACBITS) & 0xFFF;

    // smart clamping/modulo: iteration error can only cause us to overflow or
    // underflow by 1.0 unit.  if we are off by more than that, assume modulo
    if (ir & 0xF00) {
        GDBG_INFO(165,"red overflow 0x%x %d\n",ir,ir);
        if (clamp) ir = ir&0x800 ? 0 : 0xFF;
        else if (ir == 0xFFF) ir = 0;
        else if (ir == 0x100) ir = 0xFF;
        else {
            ir &= 0xFF;
            GDBG_INFO(165,"    taking modulo of color\n");
        }
    }
    if (ig & 0xF00) {
        GDBG_INFO(165,"green overflow 0x%x %d\n",ig,ig);
        if (clamp) ig = ig&0x800 ? 0 : 0xFF;
        else if (ig == 0xFFF) ig = 0;
        else if (ig == 0x100) ig = 0xFF;
        else {
            ig &= 0xFF;
            GDBG_INFO(165,"    taking modulo of color\n");
        }
    }
    if (ib & 0xF00) {
        GDBG_INFO(165,"blue overflow 0x%x %d\n",ib,ib);
        if (clamp) ib = ib&0x800 ? 0 : 0xFF;
        else if (ib == 0xFFF) ib = 0;
        else if (ib == 0x100) ib = 0xFF;
        else {
            ib &= 0xFF;
            GDBG_INFO(165,"    taking modulo of color\n");
        }
    }
    if (ia & 0xF00) {
        GDBG_INFO(165,"alpha overflow 0x%x %d\n",ia,ia);
        if (clamp) ia = ia&0x800 ? 0 : 0xFF;
        else if (ia == 0xFFF) ia = 0;
        else if (ia == 0x100) ia = 0xFF;
        else {
            ia &= 0xFF;
            GDBG_INFO(165,"    taking modulo of color\n");
        }
    }
    iterators[0] = ir;
    iterators[1] = ig;
    iterators[2] = ib;
    iterators[3] = ia;
}

//----------------------------------------------------------------------
// draw a pixel, do the following steps
//  0) select color input data from possible sources
//  1) chroma-key/transparency
//  2) apply RGBA modulation
//  3) fog
//  4) alpha function
//      5) stencil test/stencil fail operation
//  6) zfunction/zbuffer / stencil pass-depth (pass|fail) operation
//  7) alphablend
//  8) dither (always do this last)
//  9) put the pixel to the framebuffer through the writemasks
//----------------------------------------------------------------------
void
sstFbiPixel(SstRegs *sst, int x, int y, FxBool useLfbData)
{
    int alp,bpp,fbz,fbzCP,combineMode,flip,alphaMaskFailed;
    unsigned int renderMode, stencilMode;
    unsigned char local[4], other[4], iterators[4], mselect_7[3];
    unsigned int r,g,b,a;       // "other" color split out
    unsigned int z,fogZ,fogA,fogW;
    unsigned int r_before_fog, g_before_fog, b_before_fog;
    unsigned int destColor;
    unsigned int stencilRef, stencilWriteMask, newStencil, stencilAndZ;
    FxI64 z64, w64;
    FxBool otherColorIsLFB=FXFALSE, otherAlphaIsLFB=FXFALSE;
    CsimPrivate *cp = CSIM_PRIVATE(sst);
    static int dithmat[4][4][4] = {
        {{0,8,2,10}, {12,4,14,6}, {3,11,1,9}, {15,7,13,5}},
        {{12,0,14,2}, {4,8,6,10}, {15,3,13,1}, {7,11,5,9}},
        {{4,12,6,14}, {8,0,10,2}, {7,15,5,13}, {11,3,9,1}},
        {{8,4,10,6}, {0,12,2,14}, {11,7,9,5}, {3,15,1,13}}
        };
    int (*pdithmat)[4];

    alp = sst->alphaMode;
    fbz = sst->fbzMode;
    renderMode = sst->renderMode;
    stencilMode = sst->stencilMode;
    fbzCP = sst->fbzColorPath;
    combineMode = sst->combineMode;
    flip = fbz & SST_YORIGIN;
    alphaMaskFailed = 0;
    bpp = renderMode & SST_RM_3D_MODE;
    stencilWriteMask = (stencilMode & SST_STENCIL_WMASK) >> SST_STENCIL_WMASK_SHIFT;
    _getAndClampIterators(sst,iterators);

    GDBG_INFO(140,"\t-RGBA Iterator FBI=%3x%3x%3x%3x\n",iterators[0],
        iterators[1], iterators[2], iterators[3]);

    if((bpp & SST_RM_32BPP) & (fbz & SST_ENALPHAMASK))
        GDBG_ERROR("sstFbiPixel", "Cannot have SST_ENALPHAMASK set with 32 bit framebuffer\n");

    //Check for illegal inputs
    if(!(fbzCP & SST_ENTEXTUREMAP))
    {
        FxBool failed=FXFALSE;

        if((fbzCP & (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL)) == (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL))
        {
            GDBG_ERROR("sstFbiPixel", "Damn! Can't use fbzColorPath[15:14]=2'b11 with texturing disabled\n");
            failed=FXTRUE;
        }

        if((fbzCP & (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL)) == (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL))
        {
            GDBG_ERROR("sstFbiPixel", "Damn! Can't use fbzColorPath[24:23]=2'b11 with texturing disabled\n");
            failed=FXTRUE;
        }

        if(failed)
            GDBG_INFO(0, "fbzColorPath = 0x%08x\n", fbzCP);
    }

    //Make sure we don't confuse lfb writes and rendering
    if(useLfbData)
    {
        fbzCP = (fbzCP & (~(SST_RGBSELECT | SST_ASELECT))) | SST_RGBSEL_LFB | SST_ASEL_LFB;
        combineMode = (combineMode & (~(SST_CM_CC_OTHERSELECT | SST_CM_CCA_OTHERSELECT))) |
            SST_CM_CC_OTHERSELECT_LFB_RGB | SST_CM_CCA_OTHERSELECT_LFB_A;
    }
    else
    {
        if((fbzCP & SST_RGBSELECT) == SST_RGBSEL_LFB)
            fbzCP = (fbzCP & (~SST_RGBSELECT)) | SST_RGBSEL_RGBA;
        if((fbzCP & SST_ASELECT) == SST_ASEL_LFB)
            fbzCP = (fbzCP & (~SST_ASELECT)) | SST_ASEL_RGBA;

        if((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_LFB_RGB)
            combineMode = (combineMode & (~SST_CM_CC_OTHERSELECT)) | SST_CM_CC_OTHERSELECT_IRGB;
        if((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_LFB_A)
            combineMode = (combineMode & (~SST_CM_CCA_OTHERSELECT)) | SST_CM_CCA_OTHERSELECT_IA;
    }

    //------------------------------------------------------------------
    // perform RGB color selection for the "other" color
    //------------------------------------------------------------------
    if(!(combineMode & SST_CM_USE_COMBINE_MODE))
    {
        switch (fbzCP & SST_RGBSELECT) {
        case SST_RGBSEL_TREXOUT:   // fetch the TREX0 output color
            r = cp->fbiData.trexIn[0];
            g = cp->fbiData.trexIn[1];
            b = cp->fbiData.trexIn[2];

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_RGBSEL_RGBA:      // fetch RGB iterators
            r = iterators[0];
            g = iterators[1];
            b = iterators[2];
            break;
        case SST_RGBSEL_C1:        // fetch C1 color
            r = (sst->c1>>16) & 0xFF;
            g = (sst->c1>>8) & 0xFF;
            b = sst->c1 & 0xFF;
            break;
        case SST_RGBSEL_LFB:       // fetch LFB color, already 8888 format
            goto rgb_other_lfb;       // avoid duplicate code
        }
    }
    else  //Using CombineMode Register
    {
        switch (combineMode & SST_CM_CC_OTHERSELECT)
        {
        case SST_CM_CC_OTHERSELECT_IRGB:  // fetch RGB iterators
            r = iterators[0];
            g = iterators[1];
            b = iterators[2];
            break;
        case SST_CM_CC_OTHERSELECT_TRGB:  // fetch the TREX0 output color
            r = cp->fbiData.trexIn[0];
            g = cp->fbiData.trexIn[1];
            b = cp->fbiData.trexIn[2];

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_CM_CC_OTHERSELECT_C1_RGB:  // fetch C1 color
            r = (sst->c1>>16) & 0xFF;
            g = (sst->c1>>8) & 0xFF;
            b = sst->c1 & 0xFF;
            break;
        case SST_CM_CC_OTHERSELECT_LFB_RGB:
            rgb_other_lfb:
            // NOTE: we only get here upon LFB access
            otherColorIsLFB = FXTRUE;
            if ( !(sst->lfbMode & SST_LFB_ENPIXPIPE ) ) // Use lfbmode if ! using pixpipe
            {
                flip = sst->lfbMode & SST_LFB_YORIGIN;
            }

            if(useLfbData)
            {
                r = cp->fbiData.lfbRGBdata & 0xFF;
                g = (cp->fbiData.lfbRGBdata>>8) & 0xFF;
                b = (cp->fbiData.lfbRGBdata>>16) & 0xFF;
                iterators[0] = r;
                iterators[1] = g;
                iterators[2] = b;
            }
            else
            {
                r = iterators[0];
                g = iterators[1];
                b = iterators[2];
            }
            if ((sst->fbzColorPath & SST_ASELECT) != SST_ASEL_LFB)
                GDBG_ERROR("sstFbiPixel",
                    "SST_RGBSEL_LFB is set but SST_ASEL_LFB is not\n");
            break;
        case SST_CM_CC_OTHERSELECT_IA:  // fetch Alpha iterator
            r = iterators[3];
            g = iterators[3];
            b = iterators[3];
            break;
        case SST_CM_CC_OTHERSELECT_TA:  // fetch the TREX0 output alpha
            r = cp->fbiData.trexIn[3];
            g = cp->fbiData.trexIn[3];
            b = cp->fbiData.trexIn[3];      

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_CM_CC_OTHERSELECT_C1_A: // fetch C1 color's alpha
            r = (sst->c1>>24) & 0xFF;
            g = (sst->c1>>24) & 0xFF;
            b = (sst->c1>>24) & 0xFF;
            break;
        case SST_CM_CC_OTHERSELECT_ZERO:
            r = 0;
            g = 0;
            b = 0;
            break;
        }
    }

    // check where to get yorigin subtraction value from, put result into z
    if (renderMode & SST_RM_YORIGIN_SELECT) { //use renderMode
        z = (renderMode & SST_RM_YORIGIN_TOP) >> SST_RM_YORIGIN_TOP_SHIFT;
    }
    else {  // use miscInit0
        SstIORegs *sstio = &(CSIM_PRIVATE(sst))->io; 
        z = (sstio->miscInit0 & SST_YORIGIN_TOP) >> SST_YORIGIN_TOP_SHIFT;
    }

    if (flip) {             // perform YORIGIN flip
        y = z - y;
    }
    // perform sanity check for off-screen drawing
    if (x < 0) GDBG_ERROR("sstFbiPixel", "(x=%d) < 0\n",x);
    if (x > 2047) GDBG_ERROR("sstFbiPixel", "(x=%d) > 2047\n",x);
    if (y < 0) GDBG_ERROR("sstFbiPixel", "(y=%d) < 0\n",y);

    if (flip && ((unsigned)y > z))
    {
        GDBG_ERROR("sstFbiPixel", "flip=1 and (y=%d) > %d\n",y,z);
    }

    //------------------------------------------------------------------
    // get the appropriate W,Z values, also alpha iterator for fog
    //------------------------------------------------------------------
    w64 = cp->fbiData.spanFbi.w64;
    z64 = cp->fbiData.spanFbi.z64;
    z64 = FX_SHR64(z64, SST_Z64_FRACBITS_32BPP);
    {
        FxU32 zmax = bpp==SST_RM_32BPP ? 0xFFFFFF : 0xFFFF00;
        z = FX_LO64(z64) & (0x0F000000 | zmax);  //4 bits overflow, 24 bits integer
        if (z & 0xF000000) {
            GDBG_INFO(165,"z overflow 0x%x %d\n",z,z);
            if (sst->fbzColorPath & SST_RGBAZ_CLAMP) 
                z = z&0x8000000 ? 0 : zmax;
            else if (z == (0x0F000000 | zmax)) z = 0;
            else if (z == 0x1000000) z = zmax;
            else {
                z &= zmax;
                GDBG_INFO(9,"    taking modulo of z\n");
            }
        }
    }    
    //Note: At this point, the z value is 24 bits wide independent  
    //      of the framebuffer depth. If the framebuffer depth is not 32bpp,
    //      the 8 LSBs will have to be truncated later
    fogA = a = iterators[3];
    fogW = FX_HI64(w64) & 0xFFFF;
    if (fogW & 0xFF00) {
        GDBG_INFO(165,"fogW overflow 0x%x %d\n",fogW,fogW);
        if (sst->fbzColorPath & SST_RGBAZ_CLAMP) fogW = fogW&0x8000 ? 0 : 0xFF;
        else if (fogW == 0xFFFF) fogW = 0;
        else if (fogW == 0x0100) fogW = 0xFF;
        else {
            fogW &= 0xFF;
            GDBG_INFO(9,"    taking modulo of fogW\n");
        }
    }
    // NOTE: z,a might get overwritten later by an LFB access

    //------------------------------------------------------------------
    // perform Alpha color selection for the "other" color
    //------------------------------------------------------------------
    if(!(combineMode & SST_CM_USE_COMBINE_MODE))
    {
        switch (fbzCP & SST_ASELECT) {
        case SST_ASEL_TREXOUT:     // fetch the TREX0 output color
            a = cp->fbiData.trexIn[3];

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_ASEL_RGBA:        // fetch RGBA iterators (already done)
            break;
        case SST_ASEL_C1:      // fetch C1 color
            goto asel_c1;         // avoid duplicate code;
        case SST_ASEL_LFB:
            goto asel_lfb;        // avoid duplicate code;
        }
    }
    else  //Using CombineMode Register
    {
        switch (combineMode & SST_CM_CCA_OTHERSELECT)     
        {
        case SST_CM_CCA_OTHERSELECT_IA:  // fetch RGBA iterators (already done)
            break;
        case SST_CM_CCA_OTHERSELECT_TA:  //Get TREX0 Alpha
            a = cp->fbiData.trexIn[3];

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_CM_CCA_OTHERSELECT_C1_A: 
            asel_c1:
            a = (sst->c1>>24) & 0xFF;
            if (cp->cmdCode == SST_FASTFILLCMD) {
                if(bpp & SST_RM_32BPP) {
                    a = (sst->c1>>24) & 0xFF;   // alpha comes from C1 !!!
                    z = sst->zaColor & 0x00FFFFFF;// z comes from here
                    z |= (stencilMode & SST_STENCIL_REF) << (24-SST_STENCIL_REF_SHIFT);
                    if((fbz & SST_ZAWRMASK) || (stencilMode & SST_STENCIL_WMASK)) {
                        if ((fbz & SST_ZAWRMASK)==0) {
                            z &= 0xFF000000;
                            z |= 0x00FFFFFF & csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
                        }
                        if ((stencilMode & SST_STENCIL_WMASK)==0) {
                            z &= 0x00FFFFFF;
                            z |= 0xFF000000 & csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
                        }
                    }
                    stencilAndZ = z;
                }
                else {
                    a = (sst->zaColor>>24) & 0xFF;
                    z = (sst->zaColor & 0xFFFF)<<8; //Shifted up 8 to make it 24 bits
                }
                goto lfb_bypass;        
            }
            break;
        case SST_CM_CCA_OTHERSELECT_LFB_A:
            asel_lfb:
            otherAlphaIsLFB = FXTRUE;

            if(useLfbData)
            {
                fogA = a = cp->fbiData.lfbAdata;        // get LFB Z/A values (and W)
                iterators[3] = a;
            }
            else
            {
                a = iterators[3];
                break;
            }

            z = cp->fbiData.lfbZdata >> 8;      // lfbZdata is .32 format
            GDBG_INFO(223,"\t-a,z from LFB =%3x %x\n",a,z);

            if ((sst->lfbMode & SST_LFB_ENPIXPIPE) == 0) {  // BYPASS mode
                fbz = cp->fbiData.lfbFBZmode;     // override modes
                renderMode = cp->fbiData.lfbRenderMode;
                alp = 0;

                //If doing a 3D lfb write, don't write to the stencil buffer if
                //the pixel pipeline is diabled
                stencilMode = stencilMode & ~(SST_STENCIL_WMASK);

                if (bpp == SST_RM_32BPP)
                {
                    //Need to get old stencil value, because the stencil value be written
                    //later if fbz & SST_ZAWRMASK; therefore, the old stencil value
                    //has to be in stencilAndZ
                    stencilAndZ = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
                    stencilAndZ = (0xFF000000 & stencilAndZ) | (0x00FFFFFF & z);
                }

                goto lfb_bypass;          // and bypass the pipe
            }
            if (sst->lfbMode & SST_LFB_WSELECT) {   // w comes from zaColor
                if(bpp & SST_RM_32BPP)
                    FX_SET64(w64,0,(sst->zaColor & 0xFFFFFF)<<(SST_W64_FRACBITS-24));
                else
                    FX_SET64(w64,0,(sst->zaColor & 0xFFFF)<<(SST_W64_FRACBITS-16));
            }
            // else data comes from LFB data
            else {
                FX_SET64(w64,0,cp->fbiData.lfbZdata<<(SST_W64_FRACBITS-32));
            }
            GDBG_INFO(223,"\t-w64 from LFB = %08x_%08x\n",
                FX_LO64(FX_SHR64(w64,32)),FX_LO64(w64));      
            break;        
        }
    }

    if(useLfbData && (otherAlphaIsLFB ^ otherColorIsLFB))
        GDBG_ERROR("sstFbiPixel", "shit! rgbLFB=%d aLFB=%d x,y=%d,%d %s(%d)\n",
            otherColorIsLFB, otherAlphaIsLFB, x, y, __FILE__, __LINE__);

    //------------------------------------------------------------------
    // NOTE: this is really the beginning of the pixel pipeline
    // FASTFILL and lfb_bypass mode jump around this code
    GDBG_INFO(140,"\t-RGBA into FBI    =%3x%3x%3x%3x\n",r,g,b,a);

    if (fbz & SST_ENALPHAMASK) {    // ALPHAMASK culling
        if ((a&1) == 0) {       // test LSB of alpha
            GDBG_INFO(163,"pixel %d,%d alphamask failed\n",x,y);
            sst->stats.fbiAfuncFail++;
            sst->stats.fbiAfuncFail &= 0xFFFFFF;
            alphaMaskFailed = 1;
        }
    }

    //------------------------------------------------------------------
    // chroma-key pixel if enabled, return 0 if transparent
    //------------------------------------------------------------------
    if (fbz & SST_ENCHROMAKEY) {
        if (sstChromaTest(r,g,b,sst->chromaKey,sst->chromaRange)) {
            GDBG_INFO(161,"pixel %d,%d chromaTest failed\n", x,y);
            sst->stats.fbiChromaFail++;
            sst->stats.fbiChromaFail &= 0xFFFFFF;
            return;         // don't draw the pixel
        }
    }

    // Don't return due to alphaMask failure until here, so chromakey
    // check can be done. This is the same way hw parallelizes things.
    if (alphaMaskFailed) {
        return;             // don't draw the pixel
    }


    //------------------------------------------------------------------
    // perform RGB selection for the "mselect_7" color
    //------------------------------------------------------------------    
    if(combineMode & SST_CM_USE_COMBINE_MODE)
    {
        switch(combineMode & SST_CM_CC_MSELECT_7)
        {
        case SST_CM_CC_MSELECT_7_IRGB:
            mselect_7[0] = iterators[0];
            mselect_7[1] = iterators[1];
            mselect_7[2] = iterators[2];
            break;
        case SST_CM_CC_MSELECT_7_C1_RGB:  
            mselect_7[0] = (unsigned char)((sst->c1>>16) & 0xFF);  //red
            mselect_7[1] = (unsigned char)((sst->c1>>8) & 0xFF);   //green
            mselect_7[2] = (unsigned char)(sst->c1 & 0xFF);        //blue
            break;
        case SST_CM_CC_MSELECT_7_IA:
            mselect_7[0] = iterators[3];
            mselect_7[1] = iterators[3];
            mselect_7[2] = iterators[3];
            break;
        case SST_CM_CC_MSELECT_7_C1_A:
            mselect_7[0] = (unsigned char)((sst->c1>>24) & 0xFF);
            mselect_7[1] = (unsigned char)((sst->c1>>24) & 0xFF);
            mselect_7[2] = (unsigned char)((sst->c1>>24) & 0xFF);
            break;
        default:
            assert("Invalid MSELECT_7 Case!" && 0);
        }
    }
    else
    {
        mselect_7[0] = iterators[0];
        mselect_7[1] = iterators[1];
        mselect_7[2] = iterators[2];
    }
    GDBG_INFO(174, "\t-RGB mselect_7 = %3x%3x%3x\n", mselect_7[0],
        mselect_7[2], mselect_7[2]);


    //------------------------------------------------------------------
    // color combine/composite, first do quick PASS-thru check
    //------------------------------------------------------------------
    if(!(combineMode & SST_CM_USE_COMBINE_MODE))
    {
        // select the proper "local" color now
        switch(fbzCP & SST_ALOCALSELECT) {    // first select local Alpha
        case SST_ALOCAL_ITERATOR:
            local[3] = iterators[3];
            break;
        case SST_ALOCAL_C0:
            local[3] = (unsigned char)(sst->c0 >> 24);
            break;
        case SST_ALOCAL_Z:
            local[3] = ( z >> (SST_Z64_INTBITS_32BPP-8) ) & 0xFF;
            GDBG_INFO(142,"\t-RGBA combine in =%3x%3x%3x%3x alocal=z=%3x\n",r,g,b,a,local[3]);
            break;
        case SST_ALOCAL_W:
            if ((fbzCP & SST_ASELECT) == SST_ASEL_LFB) {
                local[3] = ( z >> (SST_Z64_INTBITS_32BPP-8) ) & 0xFF;
                GDBG_INFO(148,"NOTE: XXX Alocal_W case hit with LFBs %x\n",local[3]);
            }
            else local[3] = fogW;
            break;
        }

        // now select local RGB
        // if override bit is set, we use the Msb of Alpha from trex to select
        // else we use the SST_LOCALSELECT bit to select
        // this allows the texture alpha to modify lighting/shading!!!


        if((fbzCP & SST_LOCALSELECT_OVERRIDE_WITH_ATEX) && !(fbzCP & SST_ENTEXTUREMAP))
            GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);

        if (fbzCP & SST_LOCALSELECT_OVERRIDE_WITH_ATEX ?
            (cp->fbiData.trexIn[3] & 0x80) : (fbzCP & SST_LOCALSELECT))
        {
            local[0] = (unsigned char)(sst->c0 >> 16);    // use C0: ARGB format
            local[1] = (unsigned char)(sst->c0 >> 8);
            local[2] = (unsigned char)(sst->c0 >> 0);
        }
        else {                // else use RGB iterators
            local[0] = iterators[0];
            local[1] = iterators[1];
            local[2] = iterators[02];
        }

        other[0] = r;
        other[1] = g;
        other[2] = b;
        other[3] = a;
        sstCompositeRGB(sst, FXTRUE, local, other, iterators, mselect_7, 
            NULL, NULL, //fbi ccu doesn't use localTextureColor or otherTextureColor
            (fbzCP>>SST_CCOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT,
            fbzCP & SST_CC_REVERSE_BLEND ? 0x00 : 0xFF);

        r = local[0];     // replace with "local" color
        g = local[1];
        b = local[2];

        // NOTE: only local[3] and other[3] are used
        other[3] = a;
        sstCompositeA(sst, FXTRUE, local, other, iterators,
            0, 0, //fbi cca doesn't use localTextureAlphe or otherTextureAlpha
            (fbzCP>>SST_CCOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT,
            fbzCP & SST_CCA_REVERSE_BLEND ? 0x00 : 0xFF);
        a = local[3];     // replace with "local" color   
    }
    else //Using CombineMode Register
    { 
        // select the proper "local" alpha now
        switch(combineMode & SST_CM_CCA_LOCALSELECT)
        { // first select local Alpha
        case SST_CM_CCA_LOCALSELECT_IA:
            local[3] = iterators[3];
            break;
        case SST_CM_CCA_LOCALSELECT_C0_A:
            local[3] = (unsigned char)(sst->c0 >> 24);
            break;
        case SST_CM_CCA_LOCALSELECT_IZ:
            local[3] = ( z >> (SST_Z64_INTBITS_32BPP-8) ) & 0xFF;
            GDBG_INFO(142,"\t-RGBA combine in =%3x%3x%3x%3x alocal=z=%3x\n",r,g,b,a,local[3]);
            break;
        case SST_CM_CCA_LOCALSELECT_IW:
            if ((fbzCP & SST_ASELECT) == SST_ASEL_LFB) {
                local[3] = ( z >> (SST_Z64_INTBITS_32BPP-8) ) & 0xFF;
                GDBG_INFO(148,"NOTE: XXX Alocal_W case hit with LFBs %x\n",local[3]);
            }
            else local[3] = fogW;
            break;
        case SST_CM_CCA_LOCALSELECT_TA:
            local[3] = cp->fbiData.trexIn[3];

#ifndef WINSIM
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                
            break;
        case SST_CM_CCA_LOCALSELECT_ZERO:
            local[3]=0;
            break;
        default:
            assert("Illegal f'in SST_CM_CCA_LOCALSELECT!" && 0);
        }

        // select the proper "local" RGB now
        if (fbzCP & SST_LOCALSELECT_OVERRIDE_WITH_ATEX)
        {
#ifndef WINSIM        
            if(!(fbzCP & SST_ENTEXTUREMAP))
                GDBG_ERROR("sstFbiPixel", "using texture data with texturing disabled (%d)\n", __LINE__);
#endif
            if(cp->fbiData.trexIn[3] & 0x80)          
            {
                local[0] = (unsigned char)(sst->c0 >> 16);  // use C0: ARGB format
                local[1] = (unsigned char)(sst->c0 >> 8);
                local[2] = (unsigned char)(sst->c0 >> 0);       
            }
            else {
                local[0] = iterators[0];
                local[1] = iterators[1];
                local[2] = iterators[2];
            }
        }
        else
        {
            switch(combineMode & SST_CM_CC_LOCALSELECT)
            {
            case SST_CM_CC_LOCALSELECT_IRGB:
                local[0] = iterators[0];
                local[1] = iterators[1];
                local[2] = iterators[2];
                break;
            case SST_CM_CC_LOCALSELECT_C0_RGB:
                local[0] = (unsigned char)(sst->c0 >> 16);  // use C0: ARGB format
                local[1] = (unsigned char)(sst->c0 >> 8);
                local[2] = (unsigned char)(sst->c0 >> 0);       
                break;
            case SST_CM_CC_LOCALSELECT_TRGB:
                local[0] = cp->fbiData.trexIn[0];
                local[1] = cp->fbiData.trexIn[1];
                local[2] = cp->fbiData.trexIn[2];

#ifndef WINSIM
                if(!(fbzCP & SST_ENTEXTUREMAP))
                    GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                    
                break;
            case SST_CM_CC_LOCALSELECT_IA:
                local[0] = iterators[3];
                local[1] = iterators[3];
                local[2] = iterators[3];
                break;
            case SST_CM_CC_LOCALSELECT_C0_A:
                local[0] = (unsigned char)(sst->c0 >> 24);
                local[1] = (unsigned char)(sst->c0 >> 24);
                local[2] = (unsigned char)(sst->c0 >> 24);
                break;
            case SST_CM_CC_LOCALSELECT_TA:
                local[0] = cp->fbiData.trexIn[3];
                local[1] = cp->fbiData.trexIn[3];
                local[2] = cp->fbiData.trexIn[3];

#ifndef WINSIM
                if(!(fbzCP & SST_ENTEXTUREMAP))
                    GDBG_ERROR("sstFbiPixel", "Using texture data with texturing disabled (%d)\n", __LINE__);
#endif                    
                break;
            case SST_CM_CC_LOCALSELECT_ZERO:
            case SST_CM_CC_LOCALSELECT_ZERO7:
                local[0] = 0;
                local[1] = 0;
                local[2] = 0;
                break;
            default:
                assert("Oh shit!" && 0);
                break;
            }
        }

        other[0] = r;
        other[1] = g;
        other[2] = b;
        other[3] = a;

        sstCompositeRGB(sst, FXTRUE, local, other, iterators, mselect_7,
            NULL, NULL, //fbi ccu doesn't use localTextureColor or otherTextureColor
            (fbzCP>>SST_CCOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT,
            fbzCP & SST_CC_REVERSE_BLEND ? 0x00 : 0xFF);
        r = local[0];       // replace with "local" color
        g = local[1];
        b = local[2];

        sstCompositeA(sst, FXTRUE, local, other, iterators,
            0, 0, //fbi ccu doesn't use localTextureAlpha or otherTextureAlpha
            (fbzCP>>SST_CCOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT,
            fbzCP & SST_CCA_REVERSE_BLEND ? 0x00 : 0xFF);
        a = local[3];       // replace with "local" color
    }
    GDBG_INFO(142,"\t-RGBA combine out =%3x%3x%3x%3x\n",r,g,b,a);

    //------------------------------------------------------------------
    // perform alpha function if enabled
    //------------------------------------------------------------------
    if (alp & SST_ENALPHAFUNC) {
        unsigned int aref;
        aref = (sst->alphaMode & SST_ALPHAREF)>>SST_ALPHAREF_SHIFT;
        if (alp & SST_ALPHAFUNC_LT)
            if (a < aref) goto afunc_pass;
            if (alp & SST_ALPHAFUNC_EQ)
                if (a == aref) goto afunc_pass;
                if (alp & SST_ALPHAFUNC_GT)
                    if (a > aref) goto afunc_pass;
                    GDBG_INFO(163,"pixel %d,%d alphafunc failed\n",x,y);
                    sst->stats.fbiAfuncFail++;
                    sst->stats.fbiAfuncFail &= 0xFFFFFF;
                    return;         // don't draw the pixel
    }
    afunc_pass:

    //------------------------------------------------------------------
    // perform stencil junk if in 32bpp and if depth buffering/stencil buffering is on
    //------------------------------------------------------------------
    if(stencilMode & SST_STENCIL_ENABLE)
    {
        int stencilPass=0;
        unsigned int stencilDest, stencilMask, stencilFunction;

        if((stencilMode & SST_STENCIL_ENABLE) && !(bpp & SST_RM_32BPP))
            GDBG_ERROR("sstFbiPixel", "stencil enabled and not in 32bpp mode\n");

        //Record the original state of the stencil and z buffer
        stencilAndZ = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);

        stencilRef = (stencilMode & SST_STENCIL_REF) >> SST_STENCIL_REF_SHIFT;
        stencilDest = (stencilAndZ & 0xFF000000) >> 24;
        stencilMask = (stencilMode & SST_STENCIL_MASK) >> SST_STENCIL_MASK_SHIFT;
        stencilFunction = (stencilMode & SST_STENCIL_FUNC) >> SST_STENCIL_FUNC_SHIFT;

        switch(stencilFunction)
        {
        case 0: //Never
            break;

        case 1: //Less Than
            if((stencilRef & stencilMask) < (stencilDest & stencilMask))
                stencilPass = 1;
            break;

        case 2: //Equal
            if((stencilRef & stencilMask) == (stencilDest & stencilMask))
                stencilPass = 1;
            break;

        case 3: //Less than or equal
            if((stencilRef & stencilMask) <= (stencilDest & stencilMask))
                stencilPass = 1;
            break;

        case 4: //Greater than
            if((stencilRef & stencilMask) > (stencilDest & stencilMask))
                stencilPass = 1;
            break;      

        case 5: //Not Equal
            if((stencilRef & stencilMask) != (stencilDest & stencilMask))
                stencilPass = 1;
            break;

        case 6: //Greater than or equal
            if((stencilRef & stencilMask) >= (stencilDest & stencilMask))
                stencilPass = 1;
            break;

        case 7: //Always
            stencilPass=1;
            break;      
        }

        if(!stencilPass)
        {    
            /*This is the stencil fail operation*/
            FxU32 stencil;

            GDBG_INFO(164,"pixel %d,%d stencil function(0x%x) failed, stRef=0x%x, stDest=0x%x\n",
                x,y, stencilFunction, stencilRef & stencilMask, stencilDest & stencilMask);

            sst->fbiStencilFail++;
            sst->fbiStencilFail &= 0xFFFFFF;

                stencil=(stencilAndZ>>24)&0xFF;
            switch((sst->stencilOp & SST_STENCIL_SFAIL_OP) >> SST_STENCIL_SFAIL_OP_SHIFT)
            {
            case SST_SOP_KEEP:
                newStencil=stencil;
                break;
            case SST_SOP_ZERO:
                newStencil=0;
                break;
            case SST_SOP_REPLACE:
                newStencil=stencilRef;
                break;
            case SST_SOP_INC:
                newStencil=(stencil+1)&0xFF;
                break;
            case SST_SOP_DEC:
                newStencil=(stencil-1)&0xFF;
                break;
            case SST_SOP_NEG:
                newStencil=(~stencil)&0xFF;
                break;
            case SST_SOP_INCSAT:
                if(stencil < 0xFF)
                    newStencil=stencil + 1;
                else
                    newStencil=stencil;
                break;
            case SST_SOP_DECSAT:
                if(stencil > 0x00)
                    newStencil=stencil - 1;
                else
                    newStencil=stencil;
                break;
            default:
                GDBG_ERROR("sstFbiPixel", "You bastard-o! Illegal Stencil Fail Operation\n");
                break;
            }
            goto stencil_write_and_return;
        }
    }


    //------------------------------------------------------------------
    // perform zfunction if enabled and determine stencil operation
    //------------------------------------------------------------------
    fogZ = z;               // save unbiased original Z
    if (fbz & SST_WBUFFER) {        // if wbuffering, z=float rep. of 1/w
        // use the Z iterator as a float if SST_DEPTH_FLOAT_SEL and NOT an lfb
        if ((fbz & SST_DEPTH_FLOAT_SEL) && ((fbzCP & SST_ASELECT) != SST_ASEL_LFB))
        {
            z64 = cp->fbiData.spanFbi.z64;      // get the unclamped iterator
            z = sstWfloat64(sst, FX_SHR64(z64, 12));
        }
        else
            z = sstWfloat64(sst,w64);     // use actual W value (or LFB)

        GDBG_INFO(223,"float z = %x\n",z);
    }
    GDBG_INFO(223,"unbiased z = %x\n",z);
    if (fbz & SST_ENZBIAS) {            // optionally apply signed Zbias to z
        if(bpp & SST_RM_32BPP)
            z += SIGN_EXTEND(sst->zaColor,24);  
        else
            z += SIGN_EXTEND(sst->zaColor,16)<<8;   // line up with 4.12(8)

        if (z & 0x80000000) z = 0;        // clamp negative values to 0
    }
    GDBG_INFO(223,"biased z = %x\n",z);
    if (z > 0xFFFFFF) z = 0xFFFFFF;     // clamp high
    GDBG_INFO(223,"clamped z = %x\n",z);

    if (fbz & SST_ENDEPTHBUFFER) {
        unsigned int ztest,zref;

        if(bpp & SST_RM_32BPP)
        {
            ztest = fbz & SST_ZCOMPARE_TO_ZACOLOR ? (sst->zaColor & 0xFFFFFF) : z;
            if(stencilMode & SST_STENCIL_ENABLE)
                zref = stencilAndZ;     // use already fetched value
            else
                zref = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
            zref = zref & 0x00FFFFFF;

            if (fbz & SST_ZFUNC_LT)
                if (ztest < zref) goto zfunc_pass;
                if (fbz & SST_ZFUNC_EQ)
                    if (ztest == zref) goto zfunc_pass;
                    if (fbz & SST_ZFUNC_GT)
                        if (ztest > zref) goto zfunc_pass;
        }
        else
        {
            ztest = fbz & SST_ZCOMPARE_TO_ZACOLOR ? (sst->zaColor & 0xFFFF) : (z>>8);
            zref = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);

            if (fbz & SST_ZFUNC_LT)
                if (ztest < zref) goto zfunc_pass;
                if (fbz & SST_ZFUNC_EQ)
                    if (ztest == zref) goto zfunc_pass;
                    if (fbz & SST_ZFUNC_GT)
                        if (ztest > zref) goto zfunc_pass;
        }

        GDBG_INFO(162,"pixel %d,%d zfunction failed, ztest=0x%x  ref=0x%x\n",
            x,y,ztest,zref);
        sst->stats.fbiZfuncFail++;
        sst->stats.fbiZfuncFail &= 0xFFFFFF;

        /*The depthbuffer failed*/
        if(stencilMode & SST_STENCIL_ENABLE) {
            /*This is the stencil pass/depth buffer fail operation*/
            FxU32 stencil;

                stencil=(stencilAndZ>>24)&0xFF;
            switch((sst->stencilOp & SST_STENCIL_ZFAIL_OP) >> SST_STENCIL_ZFAIL_OP_SHIFT)
            {
            case SST_SOP_KEEP:
                newStencil=stencil;
                break;
            case SST_SOP_ZERO:
                newStencil=0;
                break;
            case SST_SOP_REPLACE:
                newStencil=stencilRef;
                break;
            case SST_SOP_INC:
                newStencil=(stencil+1)&0xFF;
                break;
            case SST_SOP_DEC:
                newStencil=(stencil-1)&0xFF;
                break;
            case SST_SOP_NEG:
                newStencil=(~stencil)&0xFF;
                break;
            case SST_SOP_INCSAT:
                if(stencil < 0xFF)
                    newStencil=stencil + 1;
                else
                    newStencil=stencil;
                break;
            case SST_SOP_DECSAT:
                if(stencil > 0x00)
                    newStencil=stencil - 1;
                else
                    newStencil=stencil;
                break;
            default:
                GDBG_ERROR("sstFbiPixel", 
                    "You bastard-o! Illegal Stencil Pass/Depth Fail Operation\n");
                break;
            }
            goto stencil_write_and_return;
        }
        goto check_stencil_write;   // don't draw the pixel
    }

    zfunc_pass:
    {
        unsigned int stencil;
        if(stencilMode & SST_STENCIL_ENABLE)
        {
            /*This is the stencil pass/depth buffer pass operation*/
            stencil=(stencilAndZ>>24)&0xFF;
            GDBG_INFO(164,"pixel %d,%d stencil function passed, stDest=0x%x\n",
                x,y, stencil);

            switch((sst->stencilOp & SST_STENCIL_ZPASS_OP) >> SST_STENCIL_ZPASS_OP_SHIFT)
            {
            case SST_SOP_KEEP:
                newStencil=stencil;
                break;
            case SST_SOP_ZERO:
                newStencil=0;
                break;
            case SST_SOP_REPLACE:
                newStencil=stencilRef;
                break;
            case SST_SOP_INC:
                newStencil=(stencil+1)&0xFF;
                break;
            case SST_SOP_DEC:
                newStencil=(stencil-1)&0xFF;
                break;
            case SST_SOP_NEG:
                newStencil=(~stencil)&0xFF;
                break;
            case SST_SOP_INCSAT:
                if(stencil < 0xFF)
                    newStencil=stencil + 1;
                else
                    newStencil=stencil;
                break;
            case SST_SOP_DECSAT:
                if(stencil > 0x00)
                    newStencil=stencil - 1;
                else
                    newStencil=stencil;
                break;
            default:
                GDBG_ERROR("sstFbiPixel", 
                    "You bastard-o! Illegal Stencil Pass/Depth Pass Operation\n");
                break;
            }
        }
        // z pass and stencil disabled
        else {
            // get the original state of the stencil and z buffer
            stencilAndZ = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
            stencil = stencilAndZ >> 24;
            newStencil = (stencilMode & SST_STENCIL_REF) >> SST_STENCIL_REF_SHIFT;
        }

        // Update stencil value and depth value
        // Don't write now, wait until end of pixel pipeline
        newStencil = ((stencil & (~stencilWriteMask)) | 
            (newStencil & stencilWriteMask)) & 0xFF;
        if (fbz & SST_ZAWRMASK) stencilAndZ = z;
        stencilAndZ &= 0x00FFFFFF;
        stencilAndZ |= (newStencil<<24);
    }

    //------------------------------------------------------------------
    // fog, get 5 MSBs of W (after recip) after shift right
    // NOTE: in 15bpp/16bpp w is in 4.12 floating point format (4 exponent, 12 mantissa)
    // NOTE: in 32bpp w is in 5.19 floating point format (5 exponent, 19 mantissa)
    //------------------------------------------------------------------
    r_before_fog = r;           // save away the source color before fog
    g_before_fog = g;
    b_before_fog = b;
    if (sst->fogMode & SST_ENFOGGING) {
        int w,alphaFog,fogColor;

        // if multiplying textures into framebuffer, then compute Afog*Cfog
        if (sst->fogMode & SST_FOGMULT) r=g=b=0;
        if (sst->fogMode & SST_FOG_CONSTANT) {
            fogColor = sst->fogColor;
            r += (fogColor>>16) & 0xFF; // get RED channel
            g += (fogColor>>8) & 0xFF;  // get GREEN channel
            b += (fogColor>>0) & 0xFF;  // get BLUE channel
            goto fogged;
        }
        switch (sst->fogMode & (SST_FOG_Z|SST_FOG_ALPHA)) {
        case SST_FOG_Z|SST_FOG_ALPHA:
            if ((fbzCP & SST_ASELECT) == SST_ASEL_LFB) {
                alphaFog = w = ( fogZ >> (SST_Z64_INTBITS_32BPP-8) ) & 0xFF;
                //          GDBG_INFO(148,"NOTE: XXX Fog_linear_W case hit with LFB\n");
            }
            else alphaFog = w = fogW;
            break;
        case SST_FOG_Z:
            alphaFog = w = fogZ >> (SST_Z64_INTBITS_32BPP-8);
            break;
        case SST_FOG_ALPHA:
            alphaFog = w = fogA;
            break;
        case 0:
            {   
                int del;
                static int lasty= -1;     // debug helper

                w = sstWfloat64(sst,w64);
                if (fbz & SST_ENZBIAS) // optionally apply signed Zbias
                {   
                    if(bpp & SST_RM_32BPP)
                        w += SIGN_EXTEND(sst->zaColor,24);
                    else
                        w += SIGN_EXTEND(sst->zaColor,16)<<8;  //Always have 24 bits integer

                    if (w & 0x80000000) w = 0;
                }
                if (w > 0xFFFFFF) w = 0xFFFFFF;

                //Convert from 5.19 w to 4.12 bit w
                //This way the old code can be used unmodified
                if(bpp & SST_RM_32BPP)
                    w <<= 1;
                w >>= 8;              // data is left-justified in 24-bits

                w >>= 2;              // 6.8 format for 16bpp, 
                if (w > 0x3FFF) w = 0x3FFF;   // clamp to 1.0 (6.8 format)
                alphaFog = sst->fogTable[w>>9];   // get 32-bit word, 2 entries in it
                alphaFog >>= (w&0x100)>>4;    // isolate one 16-bit entry
                del = alphaFog & 0xFF;        // isolate delta value within the entry
                alphaFog = (alphaFog>>8) & 0xFF;  // isolate the base value

                if ((sst->fogMode & SST_FOG_ZONES) && (del&2)) {
                    del &= 0xFC;
                    del = -del;
                }
                else
                    del &= 0xFC;

                if (sst->fogMode & SST_FOG_DITHER) {
                    // delta*fraction, or 6.2 * 0.8, result is 8.10
                    // shift right 6 to get 8.4 then add dither fraction
                    int inc = ((del * (w&0xFF))>>6) + dithmat[0][y & 3][x & 3];
#ifdef GDBG_INFO_ON
                    if (y != lasty) {   // only print out once per scanline
                        if (GDBG_GET_DEBUGLEVEL(143))
                            GDBG_INFO(143,"y=%d fog=0x%x= %x %c 0x%x(del) * .%02x(frac) +dit=%d wfloat=%x\n",
                                y,alphaFog + (inc>>4),alphaFog,
                                del<0?'-':'+',del<0?(-del>>2):(del>>2),w&0xFF,
                        dithmat[0][y & 3][x & 3],w<<2);
                        //            lasty = y;
                    }
#endif
                    alphaFog = alphaFog + (inc>>4);
                }
                else {
#ifdef GDBG_INFO_ON
                    if (y != lasty) {   // only print out once per scanline
                        if (GDBG_GET_DEBUGLEVEL(143))
                            GDBG_INFO(143,"y=%d fog beta=0x%x: %x(base) %c %x(del) * .%02x(frac) wfloat=%x\n",
                                y,alphaFog + ((del*(w&0xFF))>>10),alphaFog,
                                del<0?'-':'+',del<0?(-del>>2):(del>>2),
                        w&0xFF,w<<2);
                        //            lasty = y;
                    }
#endif
                    // base + delta*fraction, or 8.0 + 6.2 * 0.8, result is 8.10
                    alphaFog = alphaFog + ((del * (w&0xFF))>>10);
                }
                break;
            }
        }
        if (alphaFog < 0) GDBG_ERROR("sstFbiPixel", "fog underflow: %d\n",alphaFog);
        if (alphaFog > 255) GDBG_ERROR("sstFbiPixel", "fog overflow: %d\n",alphaFog);

        alphaFog += 1;          // add in +1 bias for 256/255 scale
        // get fog color(888), use 0 if adding textures into framebuffer
        fogColor = (sst->fogMode & SST_FOGADD) ? 0 : sst->fogColor;
        // for now use high-precision multiply
        {
            int t;
            t = (fogColor>>16) & 0xFF;  // get RED channel
            r = (r<<8) + alphaFog * (t - r);
            r >>= 8;
            t = (fogColor>>8) & 0xFF;   // get GREEN channel
            g = (g<<8) + alphaFog * (t - g);
            g >>= 8;
            t = (fogColor>>0) & 0xFF;   // get BLUE channel
            b = (b<<8) + alphaFog * (t - b);
            b >>= 8;
        }
        fogged:
        if (r > 255) r = 255;       // safety clamp
        if (g > 255) g = 255;
        if (b > 255) b = 255;
        GDBG_INFO(144,"\t-RGBA after fog   =%3x%3x%3x%3x    w = %d  fog = 0x%x(%d)\n",
            r,g,b,a, w,alphaFog,alphaFog);
    }

    //------------------------------------------------------------------
    // blend pixel if enabled
    //------------------------------------------------------------------
    if (alp & SST_ENALPHABLEND) {
        int dr,dg,db,da;            // destination rgba
        int tmp;
        AFUNC srcFuncRGB, dstFuncRGB;
        AFUNC srcFuncA, dstFuncA;

        destColor = csimReadPixel(sst,CSIM_BUF_3D_COLOR,x,y);

	//Figure out which dither matrix to use
	if(sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE)
	  {
	    if (renderMode & SST_RM_DITHER_ROTATION)
	      pdithmat = dithmat[(sst->fogMode & SST_DITHER_ROTATE_BLEND_AA)>>SST_DITHER_ROTATE_BLEND_AA_SHIFT];
	    else
	      pdithmat = dithmat[0];  
	  }
	else
	  {
	    if (renderMode & SST_RM_DITHER_ROTATION)
	      pdithmat = dithmat[(sst->fogMode & SST_DITHER_ROTATE_BLEND)>>SST_DITHER_ROTATE_BLEND_SHIFT];
	    else
	      pdithmat = dithmat[0];  
	  }

        //Do Blending depending of the framebuffer mode 
        switch(bpp)
        {
        /*************************************************************
        *                     16 bpp                                *
        *************************************************************/
        case SST_RM_16BPP:
            dr = (destColor>>8) & 0xF8;
            dg = (destColor>>3) & 0xFC;
            db = (destColor<<3) & 0xF8;

            // alpha comes from zbuffer memory (when its used)
            if (cp->numBuffers > CSIM_BUF_3D_AUX1)
                da = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y) & 0xFF;
            else if (fbz & SST_ENALPHABUFFER)
                GDBG_ERROR("sstFbiPixel","SST_ENALPHABUFFER set but no alpha present\n");

            if (fbz & SST_ENDITHERSUBTRACT) 
            {
                if (fbz & SST_DITHER2x2)
                    tmp = 8-pdithmat[y & 1][x & 1];
                else
                    tmp = 8-pdithmat[y & 3][x & 3];  // subtract out dither matrix
                dr += tmp>>1;
                dg += tmp>>2;
                db += tmp>>1;
                if (dr<0) dr = 0;           // clamp low
                if (dg<0) dg = 0;
                if (db<0) db = 0;
            }

            dr += dr >> 5;              // add in MSBS, expand to 0xFF
            dg += dg >> 6;
            db += db >> 5;

            if (fbz & SST_ENDITHERSUBTRACT) 
            {
                if (dr > 0xFF) dr = 0xFF;       // and clamp high
                if (dg > 0xFF) dg = 0xFF;
                if (db > 0xFF) db = 0xFF;
            }

            tmp = (alp & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT;
            if (tmp==SST_A_DSTALPHA || tmp==SST_AOM_DSTALPHA || tmp==SST_A_SATURATE) 
            {
                if (fbz & SST_ENDEPTHBUFFER)
                    GDBG_ERROR("sstFbiPixel", "dest. alpha referenced while depthbuffering\n");
                if (!(fbz & SST_ENALPHABUFFER))
                    GDBG_ERROR("sstFbiPixel", "dest. alpha referenced without SST_ENALPHABUFFER\n");
            }
            srcFuncRGB = _srcFactRGB[tmp];

                tmp = (alp & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT;
            if (tmp==SST_A_DSTALPHA || tmp==SST_AOM_DSTALPHA) 
            {
                if (fbz & SST_ENDEPTHBUFFER)
                    GDBG_ERROR("sstFbiPixel", "dest. alpha referenced while depthbuffering\n");
                if (!(fbz & SST_ENALPHABUFFER))
                    GDBG_ERROR("sstFbiPixel", "dest. alpha referenced without SST_ENALPHABUFFER\n");
            }
            dstFuncRGB = _dstFactRGB[tmp];

            if (tmp != SST_A_COLORBEFOREFOG) {      // if not color before fog
                r_before_fog = r;           // then just use r,g,b
                g_before_fog = g;
                b_before_fog = b;
            }
            // GMT: old limited alpha factors
            srcFuncA = _srcFactA[(alp & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT];
            dstFuncA = _dstFactA[(alp & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT];
            break;

        /*************************************************************
        *                     15 bpp                                *
        *************************************************************/
        case SST_RM_15BPP:
            dr = (destColor>>7) & 0xF8;
            dg = (destColor>>2) & 0xF8;
            db = (destColor<<3) & 0xF8;
            da = (destColor & 0x8000) ? 0xFF : 0x00;

            if (fbz & SST_ENALPHABUFFER)
                GDBG_ERROR("sstFbiPixel","SST_ENALPHABUFFER set in 1555 ARGB mode\n");

            if (fbz & SST_ENDITHERSUBTRACT) 
            {
                if (fbz & SST_DITHER2x2)
                    tmp = 8-pdithmat[y & 1][x & 1];
                else
                    tmp = 8-pdithmat[y & 3][x & 3];  // subtract out dither matrix
                dr += tmp>>1;
                dg += tmp>>1;
                db += tmp>>1;
                if (dr<0) dr = 0;           // clamp low
                if (dg<0) dg = 0;
                if (db<0) db = 0;
            }

            dr += dr >> 5;              // add in MSBS, expand to 0xFF
            dg += dg >> 5;
            db += db >> 5;

            if (fbz & SST_ENDITHERSUBTRACT) 
            {
                if (dr > 0xFF) dr = 0xFF;       // and clamp high
                if (dg > 0xFF) dg = 0xFF;
                if (db > 0xFF) db = 0xFF;
            }

            srcFuncRGB = _srcFactRGB[(alp & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT];
            dstFuncRGB = _dstFactRGB[(alp & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT];

            if (((alp & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT) != SST_A_COLORBEFOREFOG) { // if not color before fog
                r_before_fog = r;           // then just use r,g,b
                g_before_fog = g;
                b_before_fog = b;
            }

            //15bpp only supports the old alpha channel alpha factors 
            srcFuncA = _srcFactA[(alp & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT];
            dstFuncA = _dstFactA[(alp & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT];
            break;

        /*************************************************************
        *                     32 bpp                                *
        *************************************************************/
        case SST_RM_32BPP:
            dr = (destColor>>16) & 0xFF;
            dg = (destColor>>8)  & 0xFF;
            db = (destColor>>0)  & 0xFF;
            da = (destColor>>24) & 0xFF;

            //      if (fbz & SST_ENDITHERSUBTRACT) 
            //        GDBG_ERROR("sstFbiPixel", "You've gone bonkers! Why dither in 32bpp?\n");

            srcFuncRGB = _srcFactRGB[(alp & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT];
            dstFuncRGB = _dstFactRGB[(alp & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT];

            if (((alp & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT) != SST_A_COLORBEFOREFOG) { // if not color before fog
                r_before_fog = r;           // then just use r,g,b
                g_before_fog = g;
                b_before_fog = b;
            }

            // GMT: new expanded alpha factors for 8888 ARGB modes
            srcFuncA = _srcFactA32[(alp & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT];
            dstFuncA = _dstFactA32[(alp & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT];
            break;

        default:
            GDBG_ERROR("sstFbiPixel", 
                "Holy Jesus! We're f'ed! Invalid framebuffer format in RenderMode register\n");
        }
        GDBG_INFO(146,"\t-RGBA  dst->ablend=%3x%3x%3x%3x\n",dr,dg,db,da);

        // now do the actual blend computation
        if (sst->fogMode & SST_RGB_BLEND_SUB) {
            if (sst->fogMode & SST_RGB_BLEND_REVERSE) {
                r = ((dstFuncRGB(r_before_fog,a,dr,da)*dr)>>8) - ((srcFuncRGB(r,a,dr,da)*r)>>8);
                g = ((dstFuncRGB(g_before_fog,a,dg,da)*dg)>>8) - ((srcFuncRGB(g,a,dg,da)*g)>>8);
                b = ((dstFuncRGB(b_before_fog,a,db,da)*db)>>8) - ((srcFuncRGB(b,a,db,da)*b)>>8);
            }
            else {
                r = ((srcFuncRGB(r,a,dr,da)*r)>>8) - ((dstFuncRGB(r_before_fog,a,dr,da)*dr)>>8);
                g = ((srcFuncRGB(g,a,dg,da)*g)>>8) - ((dstFuncRGB(g_before_fog,a,dg,da)*dg)>>8);
                b = ((srcFuncRGB(b,a,db,da)*b)>>8) - ((dstFuncRGB(b_before_fog,a,db,da)*db)>>8);
            }
        }
        else {
            r = ((srcFuncRGB(r,a,dr,da)*r)>>8) + ((dstFuncRGB(r_before_fog,a,dr,da)*dr)>>8);
            g = ((srcFuncRGB(g,a,dg,da)*g)>>8) + ((dstFuncRGB(g_before_fog,a,dg,da)*dg)>>8);
            b = ((srcFuncRGB(b,a,db,da)*b)>>8) + ((dstFuncRGB(b_before_fog,a,db,da)*db)>>8);
        }

        if (sst->fogMode & SST_A_BLEND_SUB) {
            if (sst->fogMode & SST_A_BLEND_REVERSE)
                a = (((dstFuncA(a,a,da,da)*da)>>8)&0xff) - (((srcFuncA(a,a,da,da)*a)>>8)&0xff);
            else
                a = (((srcFuncA(a,a,da,da)*a)>>8)&0xff) - (((dstFuncA(a,a,da,da)*da)>>8)&0xff);
        }
        else    // add is commutative, so order doesn't matter
            a = (((srcFuncA(a,a,da,da)*a)>>8)&0xff) + (((dstFuncA(a,a,da,da)*da)>>8)&0xff);

        // r,g,b,a are unsigned
        if ((signed)r < 0) r = 0;
        if ((signed)g < 0) g = 0;
        if ((signed)b < 0) b = 0;
        if ((signed)a < 0) a = 0;

        if (r > 0xff) r = 0xff;
        if (g > 0xff) g = 0xff;
        if (b > 0xff) b = 0xff;
        if (a > 0xff) a = 0xff;

        GDBG_INFO(146,"\t-RGBA after ablend=%3x%3x%3x%3x\n",r,g,b,a);
    }
    lfb_bypass:
    if(useLfbData && (otherAlphaIsLFB ^ otherColorIsLFB))
        GDBG_ERROR("sstFbiPixel", "shit! rgbLFB=%d aLFB=%d x,y=%d,%d %s(%d)\n",
            otherColorIsLFB, otherAlphaIsLFB, x, y, __FILE__, __LINE__);

    if (bpp != SST_RM_32BPP) {    // if 32bpp just skip all this crap!
        //------------------------------------------------------------------
        // dither pixel if enabled, y coordinate is already flipped
        //------------------------------------------------------------------
        if (fbz & SST_ENDITHER) {
            int dm,mask=~0;

	    //Figure out which dither matrix to use
	    if(sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE)
	      {
		if (renderMode & SST_RM_DITHER_ROTATION)
		  pdithmat = dithmat[(sst->fogMode & SST_DITHER_ROTATE_AA)>>SST_DITHER_ROTATE_AA_SHIFT];
		else
		  pdithmat = dithmat[0];  
	      }
	    else
	      {
		if (renderMode & SST_RM_DITHER_ROTATION)
		  pdithmat = dithmat[(sst->fogMode & SST_DITHER_ROTATE)>>SST_DITHER_ROTATE_SHIFT];
		else
		  pdithmat = dithmat[0];  
	      }

            if (fbz & SST_DITHER2x2) {
                dm = pdithmat[y & 1][x & 1];
                mask = ~3;
            }
            else {
                dm = pdithmat[y & 3][x & 3];
            }
            r = _sstDit5(r,dm,mask>>1)<<3;  // return to 8.0 format
            if (bpp == SST_RM_16BPP)
                g = _sstDit6(g,dm,mask)<<2;
            else
                g = _sstDit5(g,dm,mask>>1)<<3;
            b = _sstDit5(b,dm,mask>>1)<<3;
        }
        else {          // truncate to 5,6,5
            r = r & 0xF8;       // but leave in 8.0 format
            g = g & (bpp == SST_RM_15BPP ? 0xF8 : 0xFC);
            b = b & 0xF8;
        }
        if (bpp == SST_RM_15BPP) {
            if ((renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO) a=0x00;
            if ((renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE)  a=0xFF;
            a = a & 0x80;
        }
        GDBG_INFO(149,"\t-RGBA after dither=%3x%3x%3x%3x\n",r,g,b,a);
    }

    //------------------------------------------------------------------
    // and finally write the pixel into the framebuffer
    // NOTE: LFB accesses replace fbz with the appropriate bits set
    //------------------------------------------------------------------
    switch(bpp)
    {
    case SST_RM_32BPP: // 8888 ARGB   
        {
            unsigned int color;
            if ((cp->cmdCode == SST_FASTFILLCMD) || !(alp & SST_ENALPHABLEND))
                destColor = csimReadPixel(sst,CSIM_BUF_3D_COLOR,x,y);
            color = destColor;
            if (fbz & SST_ENALPHABUFFER)
                GDBG_ERROR("sstFbiPixel", "SST_ENALPHABUFFER set in 32bpp mode\n");

            if((fbz & SST_RGBWRMASK))
            {
                if(renderMode & SST_RM_RED_WMASK) 
                    color = (color & 0xFF00FFFF) | (r<<16);
                if(renderMode & SST_RM_GREEN_WMASK) 
                    color = (color & 0xFFFF00FF) | (g<<8);
                if(renderMode & SST_RM_BLUE_WMASK) 
                    color = (color & 0xFFFFFF00) | (b<<0);
                if((renderMode & SST_RM_ALPHA_WMASK))
                    color = (color & 0x00FFFFFF) | (a<<24);

                GDBG_INFO(192,"32bpp write masks: alpha=%d red=%d green=%d blue=%d\n",
                    (renderMode & SST_RM_ALPHA_WMASK) ? 1 : 0,
                (renderMode & SST_RM_RED_WMASK) ? 1 : 0,
                (renderMode & SST_RM_GREEN_WMASK) ? 1 : 0,
                (renderMode & SST_RM_BLUE_WMASK) ? 1 : 0);

                GDBG_INFO(191,"(%d,%d): 8888 color=%08x\n",x,y, color);
                csimWritePixel(sst, CSIM_BUF_3D_COLOR, x, y, color);
            }

            // now write to depthbuffer/stencilbuffer if either writemask is set
            // note the bits are merged together up above
            if((fbz & SST_ZAWRMASK) || (stencilMode & SST_STENCIL_WMASK)) {
                GDBG_INFO(191,"(%d,%d): stencil+z=%8x\n",x,y,stencilAndZ);
                csimWritePixel(sst, CSIM_BUF_3D_AUX1, x, y, stencilAndZ);
            }
            else
            {
                GDBG_INFO(191,"(%d,%d): stencil+z not written\n", x, y);
                GDBG_INFO(191,"stencilMode = 0x%x   fbz = 0x%x\n", stencilMode, fbz);
            }
        }
        break;

    case SST_RM_16BPP: // 565 RGB
        if (fbz & SST_RGBWRMASK) 
        {
            GDBG_INFO(191,"(%d,%d): 565 color=%08x\n",x,y,(r<<8) | (g<<3) | (b>>3));
            csimWritePixel(sst, CSIM_BUF_3D_COLOR, x, y, (r<<8) | (g<<3) | (b>>3));
        }

        if (fbz & SST_ZAWRMASK) 
        {     // alpha takes precedence
            GDBG_INFO(191,"(%d,%d): z=%8x\n",x,y,z);
            csimWritePixel(sst, CSIM_BUF_3D_AUX1, x, y, fbz & SST_ENALPHABUFFER ? a : 
            z>>8);  //Need to convert z from 24 bits to 16 bits
        }
        break;

    case SST_RM_15BPP: // 1555 ARGB
        if(fbz & SST_RGBWRMASK)
        {
            GDBG_INFO(191,"(%d,%d): 1555 color=%08x\n",x,y,(a<<8) | (r<<7) | (g<<2) | (b>>3));
            csimWritePixel(sst, CSIM_BUF_3D_COLOR, x, y, (a<<8) | (r<<7) | (g<<2) | (b>>3));
        } 

        if (fbz & SST_ZAWRMASK)  
        {     
            GDBG_INFO(191,"(%d,%d): z=%8x\n",x,y,z);
            csimWritePixel(sst, CSIM_BUF_3D_AUX1, x, y, fbz & SST_ENALPHABUFFER ? a : 
            z>>8);  //Need to convert z from 24 bits to 16 bits
        }
        break;


    default:
        GDBG_ERROR("sstFbiPixel", "Illegal f'n framebuffer format!\n");
    }



    sst->stats.fbiPixelsOut++;
    sst->stats.fbiPixelsOut &= 0xFFFFFF;
    GDBG_INFO(169,"\t--------fbiPixelsOut = %d --------\n",sst->stats.fbiPixelsOut);
    guiKeepAlive(1);
    return;

    //------------------------------------------------------------
    // come here if the pixel is aborted but stencils may need writing
    // if 32bpp mode and the stencil writemask is set, 
    // then we still need to write it
    check_stencil_write:
        if((bpp == SST_RM_32BPP) && (stencilMode & SST_STENCIL_WMASK)) {
        FxU32 stencil;

        // get the original state of the stencil and z buffer
        stencilAndZ = csimReadPixel(sst, CSIM_BUF_3D_AUX1, x, y);
        newStencil = (stencilMode & SST_STENCIL_REF) >> SST_STENCIL_REF_SHIFT;

        stencil_write_and_return:
        stencil = stencilAndZ >> 24;

        // merge in newStencil with old stencil thru the writemask
        newStencil = ((stencil & (~stencilWriteMask)) | 
            (newStencil & stencilWriteMask)) & 0xFF;
        // keep old Z, merge in newStencil
        stencilAndZ = (newStencil<<24) | (stencilAndZ & 0x00FFFFFF);

        //Update stencil value; maintain present depth value
        GDBG_INFO(191,"(%d,%d): stencil+z=%08x\n",x,y,stencilAndZ);
        csimWritePixel(sst, CSIM_BUF_3D_AUX1, x, y, stencilAndZ);
    }
    return;     
}

