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
** $Revision: 9$
** $Date: 10/11/00 8:08:46 PM$
*/

#include <assert.h>
#include <stdlib.h>

#include <h3.h>
#include "h3sim.h"
#include "h3asm.h"

#ifdef WINSIM
extern DWORD dwHostV3Base0;
int fFake32bppOverlay = 0;
#endif // WINSIM

#define CEIL(x,y)      ( ((x)+(y)-1) / (y) )  // divide x/y, rounding up

#define GWL 198
#define GRL 199

static FxU32 last_addr = 0;		// TEX trace optimizer

void csimCommandFifoAccessErrorMessage(CsimPrivate *cp, FxU32 address, char *functionName, 
				       char *filename, int lineNumber);
FxBool csimIsCommandFifoAddress(CsimPrivate *cp, FxU32 framebufferAddress);
FxBool csimChipOwnsRawLfbPixel(SstRegs *sst, int y, int *yPrime);

FX_EXPORT void FX_CSTYLE
csimRawLfbWrite( SstRegs *sst, FxU32 addr, FxU32 data, int nbytes )
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstIORegs *sstio = &cp->io;
    SstPCIConfigRegs *pciConfigRegs = &cp->pciConfigRegs;
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr ) - SST_RAW_LFB_OFFSET;
    FxU32 base, tiled;
    FxU32 tileCompareBase;

    //Figure out where tiled memory starts
    base = SST_RAW_LFB_TILE_BEGIN_PAGE_UNMUNGE( sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_BEGIN_PAGE )*SST_TILE_SIZE;

    //Figure out where to say tiled memory starts
    if ( sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE )
        tileCompareBase = (((sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE) >>
        LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE_SHIFT) * SST_TILE_SIZE);	  
    else
        tileCompareBase = base;

    //Get pissed off if the tileCompareBase is less than the real start of tiled memory
    if ( tileCompareBase < base )
    {
        GDBG_ERROR( "csimRawLfbWrite", 
            "What the f! lfbMemoryTileCompare tile starts at 0x%08x < lfbMemoryTileCtrl tile starts at 0x%08x\n",
            tileCompareBase, base );
    }

    tiled = iaddr < tileCompareBase ? 0 : 1;

    //Check for bits that should be set if AA is enabled
    if ( (sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE) &&
        (!(pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_DISPATCH_WRITE_ENABLE) ||
        !(pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_CPU_WRITE_ENABLE) ||
        !(pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_READ_ENABLE)) )
    {
        GDBG_ERROR( "csimRawLfbWrite", "csimRawLfbWrite: You should always enable aa lfb read/writes in cfgAALfbCtrl!\n" );
    }

    if ( tiled )
    {
        //Tiled case
        //Need to figure out ownership and write to secondary buffer if AA enabled
        FxU32 stride, x, y, xbits;
        FxU32 secondaryBase;
        FxI32 yPrime;

        //Write primary sample
        iaddr -= base;
        xbits = 10 + ((sstio->lfbMemoryTileCtrl & SST_RAW_LFB_ADDR_STRIDE)>>SST_RAW_LFB_ADDR_STRIDE_SHIFT);	
        x = iaddr & SST_MASK( xbits );
        y = iaddr>>xbits;

        //For sli, check to see if we own the pixel.
        if ( !csimChipOwnsRawLfbPixel( sst, y, &yPrime ) )
            return;

        stride = (sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_STRIDE) >>  SST_RAW_LFB_TILE_STRIDE_SHIFT;
        iaddr = tiledAddress( base, stride, 1, x, yPrime );

        //Write the primary sample
        cp->environment.allowAccessesToCommandFifoRegion=FXTRUE;
        switch ( nbytes )
        {
        case 1: csimWriteMem8( cp,iaddr,data );  break;
        case 2: csimWriteMem16( cp,iaddr,data ); break;
        case 4: csimWriteMem32( cp,iaddr,data ); break;
        default:
            GDBG_ERROR( "csimRawLfbWrite","invalid nbytes of %d for tiled primary sample at 0x%08x\n",nbytes, addr );
            break;
        } 
        cp->environment.allowAccessesToCommandFifoRegion=FXFALSE;

        //If AA enabled, write the secondary sample
        if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
        {
            secondaryBase = ((pciConfigRegs->cfgAALfbCtrl & SST_SECONDARY_BUFFER_BASE) >> SST_SECONDARY_BUFFER_BASE_SHIFT) << 4;
            iaddr = tiledAddress( secondaryBase, stride, 1, x, yPrime );

            cp->environment.allowAccessesToCommandFifoRegion=FXTRUE;
            switch ( nbytes )
            {
            case 1: csimWriteMem8( cp,iaddr,data );  break;
            case 2: csimWriteMem16( cp,iaddr,data ); break;
            case 4: csimWriteMem32( cp,iaddr,data ); break;
            default:
                GDBG_ERROR( "csimRawLfbWrite","invalid nbytes of %d for tiled secondary sample at 0x%08x\n",nbytes, addr );
                break;
            }
            cp->environment.allowAccessesToCommandFifoRegion=FXFALSE;
        }
    }
    else
    {
        //Linear case
        //Only write one sample. Don't worry about ownership or secondary buffer

        cp->environment.allowAccessesToCommandFifoRegion=FXTRUE;
        switch ( nbytes )
        {
        case 1: csimWriteMem8( cp,iaddr,data );  break;
        case 2: csimWriteMem16( cp,iaddr,data ); break;
        case 4: csimWriteMem32( cp,iaddr,data ); break;
        default:
            GDBG_ERROR( "csimRawLfbWrite","invalid nbytes of %d for linear write at 0x%08x\n",nbytes, addr );
            break;
        }        
        cp->environment.allowAccessesToCommandFifoRegion=FXFALSE;
    }
}
 
FX_EXPORT FxU32 FX_CSTYLE
csimRawLfbRead( SstRegs *sst, FxU32 addr, int nbytes )
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstIORegs *sstio = &cp->io;
    SstPCIConfigRegs *pciConfigRegs = &cp->pciConfigRegs;
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr ) - SST_RAW_LFB_OFFSET;
    FxU32 base, tiled, data;
    FxU32 tileCompareBase;

    base = SST_RAW_LFB_TILE_BEGIN_PAGE_UNMUNGE( sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_BEGIN_PAGE )*SST_TILE_SIZE;

    //Figure out where to say tiled memory starts
    if ( sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE )
        tileCompareBase = (((sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE) >>
        LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE_SHIFT) * SST_TILE_SIZE);	  
    else
        tileCompareBase = base;

    //Get pissed off if the tileCompareBase is less than the real start of tiled memory
    if ( tileCompareBase < base )
    {
        GDBG_ERROR( "csimRawLfbRead", 
            "What the f! lfbMemoryTileCompare tile starts at 0x%08x < lfbMemoryTileCtrl tile starts at 0x%08x\n",
            tileCompareBase, base );
    }

    tiled = iaddr < tileCompareBase ? 0 : 1;

    if ( tiled )
    {
        //Tiled case
        //If read to parent, we need need to blend together all samples
        //If read to child, just blend together its samples
        FxU32 stride, x, y, xbits, samples[16], secondaryBase;
        FxU32 red[4], green[4], blue[4], alpha[4];
        FxI32 chipIndex, sampleIndex;
        FxI32 nSamples=0;
        FxBool inDepthBuffer;
        FxU32 depthBufferBeginPage, depthBufferEndPage;
        FxI32 yPrime;
        FxU32 memBase1Offset;

        memBase1Offset = iaddr;

        //Calculate x,y
        iaddr -= base;
        xbits = 10 + ((sstio->lfbMemoryTileCtrl & SST_RAW_LFB_ADDR_STRIDE)>>SST_RAW_LFB_ADDR_STRIDE_SHIFT);	
        x = iaddr & SST_MASK( xbits );
        y = iaddr>>xbits;
        stride = (sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_STRIDE) >>  SST_RAW_LFB_TILE_STRIDE_SHIFT;

        //Check to see if we're in the depth buffer range
        depthBufferBeginPage = (pciConfigRegs->cfgAADepthBufferAperture & SST_AA_DEPTH_BUFFER_APERTURE_BEGIN) >>
            SST_AA_DEPTH_BUFFER_APERTURE_BEGIN_SHIFT;
        depthBufferEndPage = (pciConfigRegs->cfgAADepthBufferAperture & SST_AA_DEPTH_BUFFER_APERTURE_END) >>
            SST_AA_DEPTH_BUFFER_APERTURE_END_SHIFT;
        if ( ((memBase1Offset / SST_TILE_SIZE) >= depthBufferBeginPage) && ((memBase1Offset / SST_TILE_SIZE) < depthBufferEndPage) )	 
            inDepthBuffer = FXTRUE;
        else
            inDepthBuffer = FXFALSE;

        //Collect all the samples
        for ( chipIndex=0; (FxU32)chipIndex<halInfo.boardsFound; chipIndex++ )
        {
            SstRegs *thisSST = halInfo.boardInfo[chipIndex].sstCSIM;
            CsimPrivate *thisCP = CSIM_PRIVATE( thisSST );

            //For children, skip accesses to other chips
            if ( !thisCP->environment.parentDevice && ((FxU32)chipIndex != thisCP->environment.chipIndex) )
                continue;

            // Special case is with 2 chips, non-SLI Napalm where we have 4 sample AA (thanks, Andy)
            if( !((thisSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE) && (halInfo.boardsFound==2)) )
            {
                // If SLI is disabled, make sure we count only the parent chip since the children
                // suppose to be dead (like in the Windows environment)
                if( !(sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) && !thisCP->environment.parentDevice )
                    continue;
            }

            //Check to see if we own the pixel and munge if necessary
            if ( !csimChipOwnsRawLfbPixel( thisSST, y, &yPrime ) )
                continue;

            //Get the primary sample
            iaddr = tiledAddress( base, stride, 1, x, yPrime );

            thisCP->environment.allowAccessesToCommandFifoRegion=FXTRUE;
            switch ( nbytes )
            {
            case 1: samples[nSamples++] = csimReadMem8( thisCP,iaddr );  break;
            case 2: samples[nSamples++] = csimReadMem16( thisCP,iaddr ); break;
            case 4: samples[nSamples++] = csimReadMem32( thisCP,iaddr ); break;
            default:
                GDBG_ERROR( "csimRawLfbRead","invalid nbytes of %d\n",nbytes );
                break;
            }
            thisCP->environment.allowAccessesToCommandFifoRegion=FXFALSE;	  

            //For depth buffer reads, only return first sample
            //I don't know how the hell the hardware will pick which
            //sample to return for SLIed 4 sample AA.
            if ( inDepthBuffer && nSamples > 0 )
                return( samples[0] );

            //If AA is enabled, get the secondary sample
            if ( thisSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
            {
                secondaryBase = ((pciConfigRegs->cfgAALfbCtrl & SST_SECONDARY_BUFFER_BASE) >> SST_SECONDARY_BUFFER_BASE_SHIFT) << 4;
                iaddr = tiledAddress( secondaryBase, stride, 1, x, yPrime );

                thisCP->environment.allowAccessesToCommandFifoRegion=FXTRUE;
                switch ( nbytes )
                {
                case 1: samples[nSamples++] = csimReadMem8( thisCP,iaddr );  break;
                case 2: samples[nSamples++] = csimReadMem16( thisCP,iaddr ); break;
                case 4: samples[nSamples++] = csimReadMem32( thisCP,iaddr ); break;
                default:
                    GDBG_ERROR( "csimRawLfbRead","invalid nbytes of %d\n",nbytes );
                    break;
                }
                thisCP->environment.allowAccessesToCommandFifoRegion=FXFALSE;	  	      
            }
        }
	
        // Make sure we get a reasonable number of samples back
        if ( (!(sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE) && (nSamples != 1)) ||
            ((sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE) && (nSamples != 2 && nSamples != 4)) )
        {
            GDBG_ERROR( "csimRawLfbRead", "Shizit! Got back an illegal number of samples (%d) (aaEnabled=%d)\n",
                nSamples, (sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE) ? 1 : 0);
            nSamples=1;
        }

        if(sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE)
	  {
	    FxU32 nPixelsToHandle;
	    FxU32 pixelIndex;
	    FxU32 pixelData;
	    
	    //If using a 15bpp or 16bpp, we need to deal with the second pixel
	    if(((pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_READ_FORMAT) == SST_AA_LFB_READ_FORMAT_16BPP) ||
	       ((pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_READ_FORMAT) == SST_AA_LFB_READ_FORMAT_15BPP))
	      nPixelsToHandle = 2;
	    else
	      nPixelsToHandle = 1;
	    
	    for(pixelIndex=0; pixelIndex < nPixelsToHandle; pixelIndex++)
	      { 	    
		//Convert the samples to 8888
		for ( sampleIndex=0; sampleIndex<nSamples; sampleIndex++ )
		  {
		    FxU32 sample;

		    if(nPixelsToHandle == 1)
		      sample = samples[sampleIndex];
		    else if(nPixelsToHandle == 2)
		      {
			if(pixelIndex == 0)
			  sample = samples[sampleIndex] & 0xFFFF;
			else if(pixelIndex == 1)
			  sample = (samples[sampleIndex] >> 16) & 0xFFFF;
			else
			  assert(0);
		      }
		    else
		      assert(0);
		    

		    csimConvertFramebufferFormatTo8888( sst->renderMode, sample,
							&red[sampleIndex], &green[sampleIndex], 
							&blue[sampleIndex], &alpha[sampleIndex] );
		  }
		
		//Sum the channels
		for ( sampleIndex=1; sampleIndex<nSamples; sampleIndex++ )
		  {
		    red[0] += red[sampleIndex];
		    green[0] += green[sampleIndex];
		    blue[0] += blue[sampleIndex];
		    alpha[0] += alpha[sampleIndex];
		  }      
		
		//Divide the sum
		if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
		  {
		    FxI32 rightShift;
		    
		    if ( pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_RD_DIVIDE_BY_FOUR )
		      {
			rightShift = 2; //divide by 4
			if ( nSamples != 4 )
			  GDBG_INFO( 0, "Warning! Got %d samples, but dividing by 4\n", nSamples );
		      }
		    else
		      {
			rightShift = 1; //divide by 2
			if ( nSamples != 2 )
			  GDBG_INFO( 0, "Warning! Got %d samples, but dividing by 2\n", nSamples );
		      }

		    red[0] >>= rightShift;
		    green[0] >>= rightShift;
		    blue[0] >>= rightShift;
		    alpha[0] >>= rightShift;
		  }

		//Convert to the native format
		csimConvert8888ToFramebufferFormat( (((pciConfigRegs->cfgAALfbCtrl & SST_AA_LFB_READ_FORMAT) >> SST_AA_LFB_READ_FORMAT_SHIFT) << SST_RM_3D_SHIFT),
						    red[0], green[0], blue[0], alpha[0], &pixelData );					 
		
		if(nPixelsToHandle == 1)
		  data = pixelData;
		else if(nPixelsToHandle == 2)
		  {
		    if(pixelIndex == 0)
		      data = pixelData;
		    else if(pixelIndex == 1)
		      data = data | (pixelData << 16);
		    else
		      assert(0);
		  }
		else
		  assert(0);
	      }
	  }
        else
        {
            data = samples[0];
        }
    }
    else
    {
        //Linear case
        //Just read one sample from this chip and return it

        cp->environment.allowAccessesToCommandFifoRegion=FXTRUE;
        switch ( nbytes )
        {
        case 1: data = csimReadMem8( cp,iaddr );  break;
        case 2: data = csimReadMem16( cp,iaddr ); break;
        case 4: data = csimReadMem32( cp,iaddr ); break;
        default:
            GDBG_ERROR( "csimRawLfbRead","invalid nbytes of %d\n",nbytes );
            break;
        }
        cp->environment.allowAccessesToCommandFifoRegion=FXFALSE;
    }

    return data;
}
 
//----------------------------------------------------------------------
// process a write to YUV planar space
//----------------------------------------------------------------------
// XXX GMT HACK this should be moved to lfb.c
void csimFbiYuvWrite( SstRegs *sst, FxU32 addr, FxU32 data, int nbytes )
{
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    FxU32 x,y,d;
    int i;

    iaddr -= SST_YUV_OFFSET;

    if ( cpriv->cmd.yuvBaseAddr & 0xf )
        GDBG_ERROR( "csimFbiYuvWrite","invalid (unaligned-16) yuv base address=0x%x\n",
            cpriv->cmd.yuvBaseAddr );

        x = (iaddr&SST_YUV_ADDR_X) >> SST_YUV_ADDR_X_SHIFT;
    y = (iaddr&SST_YUV_ADDR_Y) >> SST_YUV_ADDR_Y_SHIFT;

    for ( i=0; i<nbytes; i++ )
    {
        d = (data>>(i*8)) & 0xFF;
        if ( iaddr < 0x100000 )
        {			// 1st Mbyte is YYYY plane
            csimWriteMem8( cpriv,csimPixelAddress( sst,CSIM_BUF_YUV,x+i,y ),d );
        }
        else if ( iaddr < 0x200000 )
        {	        // 2nd Mbyte is UUUU plane
            csimWriteMem8( cpriv,csimPixelAddress( sst,CSIM_BUF_YUV,2*(x+i),2*y )+1,d );
            csimWriteMem8( cpriv,csimPixelAddress( sst,CSIM_BUF_YUV,2*(x+i),2*y+1 )+1,d );
        }
        else if ( iaddr < 0x300000 )
        {		// 3rd Mbyte is VVVV plane
            csimWriteMem8( cpriv,csimPixelAddress( sst,CSIM_BUF_YUV,2*(x+i)+1,2*y )+1,d );
            csimWriteMem8( cpriv,csimPixelAddress( sst,CSIM_BUF_YUV,2*(x+i)+1,2*y+1 )+1,d );
        }
        else
        {
            GDBG_ERROR( "csimFbiYuvWrite", "invalid YUV address 0x%x\n",addr );
            break;
        }
    }

}

//----------------------------------------------------------------------
// execute a 8 bit store to SST, only LFB access is allowed
//----------------------------------------------------------------------
void csimStore8( SstRegs *sst, FxU32 addr, FxU8 data )
{
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    //If writing to the parent, broadcast to the children.
    //This mimics the PCI snooping that the children do
    if ( cpriv->environment.parentDevice )
    {
        FxU32 counter;

        assert( cpriv->environment.chipIndex == 0 );

        for ( counter=1; counter<cpriv->environment.chipCount; counter++ )
            csimStore8( globalChildrenSST[counter-1], addr, data );
    }

    last_addr = addr;
    guiKeepAlive( 1 );

    if ( SST_IS_YUV_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET8(0x%x,%11d(0x%08x)) 0\tYUV8\n",
            addr,data,data );
        csimFbiYuvWrite( sst, addr, data, 1 );
    }
    else  if ( SST_IS_TEX_ADDR( iaddr ) )
    {
        // there can't be many of these so output at level 120
        GDBG_INFO( 120,"       SET8(0x%x,%11d(0x%08x)) 0\tTEX8\n",
            addr,data,data );

        if ( sst->tLOD & SST_TBIG )
            GDBG_INFO( 0, "Warning! (csimStore8) Don't use old texture ports for hummungo textures\n" );

        if ( SST_IS_TEX0_ADDR( iaddr ) )
        {
            iaddr -= SST_TEX0_OFFSET;    
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 1, tmu0TexturePort );
        }
        else
        {
            iaddr -= SST_TEX1_OFFSET;    
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+1, iaddr, data, 1, tmu1TexturePort );
        }
    } 
    //This is for the new 64MB texture aperture in Napalm. It's relative to texBaseAddr0
    else if ( SST_IS_TEX2_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET8(0x%x,%11d(0x%08x)) 0\tTEX8\n",
            addr,data,data );

        iaddr -= SST_TEX2_OFFSET;
        sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 1, largeTexturePort );
    }
    //------- RAW LFB territory ---------------
    else if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET8(0x%x,%11d(0x%08x)) 0\tRAW LFB\n",
            addr,data,data );
        csimRawLfbWrite( sst, addr, data, 1 );
    }
    else
    {
        GDBG_ERROR( "csimStore8","csimStore8: Not implemented yet\n" );
    }
}

//----------------------------------------------------------------------
// execute a 16 bit store to SST, only LFB access is allowed
//----------------------------------------------------------------------
void csimStore16( SstRegs *sst, FxU32 addr, FxU16 data )
{
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    //If writing to the parent, broadcast to the children.
    //This mimics the PCI snooping that the children do
    if ( cpriv->environment.parentDevice )
    {
        FxU32 counter;

        assert( cpriv->environment.chipIndex == 0 );

        for ( counter=1; counter<cpriv->environment.chipCount; counter++ )
            csimStore16( globalChildrenSST[counter-1], addr, data );
    }

    last_addr = addr;	
    guiKeepAlive( 1 );

    if ( SST_IS_LFB_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET16(0x%x,%11d(0x%08x)) 0\tLFB16\n",
            addr,data,data );
        csimFbiLfbWrite( sst, addr, data, 1 );
    }
    else if ( SST_IS_YUV_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET16(0x%x,%11d(0x%08x)) 0\tYUV16\n",
            addr,data,data );
        csimFbiYuvWrite( sst, addr, data, 2 );
    }
    else if ( SST_IS_TEX_ADDR( iaddr ) )
    {
        // there can't be many of these so output at level 120
        GDBG_INFO( 120,"       SET16(0x%x,%11d(0x%08x)) 0\tTEX16\n",
            addr,data,data );

        if ( sst->tLOD & SST_TBIG )
            GDBG_INFO( 0, "Warning! (csimStore16) Don't use old texture ports for hummungo textures\n" );

        if ( SST_IS_TEX0_ADDR( iaddr ) )
        {
            iaddr -= SST_TEX0_OFFSET;    
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 2, tmu0TexturePort );
        }
        else
        {
            iaddr -= SST_TEX1_OFFSET;    
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+1, iaddr, data, 2, tmu1TexturePort );
        }
    }
    //This is for the new 64MB texture aperture in Napalm. It's relative to texBaseAddr0
    else if ( SST_IS_TEX2_ADDR( iaddr ) )
    {
        iaddr -= SST_TEX2_OFFSET;    
        sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 2, largeTexturePort );	
    }
    //------- RAW LFB territory ---------------
    else if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET16(0x%x,%11d(0x%08x)) 0\tRAW LFB\n",
            addr,data,data );
        csimRawLfbWrite( sst, addr, data, 2 );
    }
    else
    {
        GDBG_ERROR( "SET16","SET16: bad address=0x%x, data=%d (x%x)\n",
            addr,data,data );
    }
}

#ifdef NO_FLOAT
// byte to float (stored as a dword) conversion table
#include "byt2flt.h"
#endif /* #ifdef NO_FLOAT */


//----------------------------------------------------------------------
// execute a 32 bit store to SST
//----------------------------------------------------------------------
void csimStore32( SstRegs *sst, FxU32 addr, FxU32 data )
{
    int sanityCheck;
    FxU32 chipID,chipMASK;			// chip ID
    FxU32 dataSave = data;
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    static int previousNopCount=0;

#ifdef WINSIM
    // Snooping of writes to the register space is controlled by cfgInitEnable
    // register bit 13 of a child
    if(cpriv->environment.parentDevice)
    {
        FxU32 counter;

        assert(cpriv->environment.chipIndex == 0 );

        for(counter=1; counter<cpriv->environment.chipCount; counter++)
        {
            // Each child must have snooping bit set in order to see the write
            
            CsimPrivate *cpriv2 = CSIM_PRIVATE( globalChildrenSST[counter-1] );
            SstPCIConfigRegs *pciConfigRegs = &cpriv2->pciConfigRegs;
            
            if( pciConfigRegs->cfgInitEnable_FabID & (1 << 13) )
            {
                csimStore32(globalChildrenSST[counter-1], addr, data);
            }
        }
    }
#else
    //If writing to the parent, broadcast to the children.
    //This mimics the PCI snooping that the children do
    if(cpriv->environment.parentDevice)
    {
        FxU32 counter;

        assert(cpriv->environment.chipIndex == 0 );

        for(counter=1; counter<cpriv->environment.chipCount; counter++)
	  csimStore32(globalChildrenSST[counter-1], addr, data);
    }
#endif

    guiKeepAlive( 1 );

    //------- IO territory ---------------
    if ( SST_IS_IO_ADDR( iaddr ) )
    {
        RegInfo *ri;
        SstIORegs *sstio = &cpriv->io;

        iaddr -= SST_IO_OFFSET;
        ri = csimRegisterIoInfo( iaddr );		// only register writes
        if ( ri != NULL )
        {			// if a valid register
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) IO\t%s %s\n", 
                addr,data,data,ri->name,cpriv->environment.name );
            data &= ri->mask;			// mask it

            if ( ri->special == 12345 ) //lfbMemoryConfig/lfbMemoryTileCtrl/lfbMemoryTileCompare
            {
                //Check to see if we should only update the upper bits of lfbMemoryConfig
                if ( data & SST_RAW_LFB_UPDATE_CONTROL )
                {
                    ((FxU32 *)sstio)[iaddr>>2] = (((FxU32 *)sstio)[iaddr>>2] & (~SST_RAW_LFB_READ_CONTROL)) |
                        (data & SST_RAW_LFB_READ_CONTROL);
                    GDBG_INFO( 122,"          Only setting bit 29 of lfbMemoryConfig (result=0x%0xxxxxxxx)\n",
                        ((((FxU32 *)sstio)[iaddr>>2] & LFB_MEMORY_CONFIG_MASK) >> 28) );
                }
                else
                {
                    //Write the lfbMemoryConfig stuff
                    ((FxU32 *)sstio)[iaddr>>2] = data & LFB_MEMORY_CONFIG_MASK;

                    //Determine whether to write lfbMemoryTileCtrl or lfbMemoryTileCompare
                    if ( data & SST_RAW_LFB_WRITE_CONTROL )
                    {
                        //Write to lfbMemoryTileCompare
                        sstio->lfbMemoryTileCompare = data & (~LFB_MEMORY_CONFIG_MASK);			
                        GDBG_INFO( 122,"          Routing SET to lfbMemoryTileCompare\n" );
                    }
                    else
                    {
                        //Write to lfbMemoryTileCtrl
                        sstio->lfbMemoryTileCtrl = data & (~LFB_MEMORY_CONFIG_MASK);
                        GDBG_INFO( 122,"          Routing SET to lfbMemoryTileCtrl\n" );
                    }
                }

            }
            else
                ((FxU32 *)sstio)[iaddr>>2] = data;	// and write it
#ifdef WINSIM
            // Propagate some Video register writes down to the V3 card
            if( sst==globalSST )
                if( (iaddr==VIDMAXRGBDELTA)             ||
                    (iaddr==VIDPROCCFG)                 ||
                    (iaddr==VIDPIXELBUFTHOLD)           ||
                    (iaddr==VIDSCREENSIZE)              ||
                    (iaddr==VIDOVERLAYSTARTCOORDS)      ||
                    (iaddr==VIDOVERLAYENDSCREENCOORD)   ||
                    (iaddr==VIDOVERLAYDUDX)             ||
                    (iaddr==VIDOVERLAYDUDXOFFSETSRCWIDTH) ||
                    (iaddr==VIDOVERLAYDVDY)             ||
                    (iaddr==VIDOVERLAYDVDYOFFSET)       ||
                    (iaddr==VIDDESKTOPSTARTADDR)        ||
                    (iaddr==VIDDESKTOPOVERLAYSTRIDE))
                    {
                        *(DWORD *)(dwHostV3Base0 + iaddr) = data;
                        printf("Napalm.vxd: Warning: csimStore32(0x%04X [I/O], 0x%X) passed down to V3\n", 
                                iaddr, data );
                    }
#endif // WINSIM          
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "SETIO","SETIO: bad address=0x%x, data=%d (x%x) %s\n", 
                addr,data,data,cpriv->environment.name );
        }
        if ( ri->special == 100 )
        {  // dacData
            cpriv->clut512.rgb[sstio->dacAddr] = sstio->dacData;   // store data in the clut
        }
#ifdef WINSIM
        // If we tried to activate overlays, we better NOT be rendering in 32bpp mode, since the V3
        // does overlays dont support that for simulation.  In that case we will fake using the
        // standard desktop and no overlays
        if( (sst==globalSST) && (iaddr==VIDPROCCFG) )
        {
            if( (data & 0x100) && ((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP) )
            {
                // Turn overlays off and start using desktop start address
                // QUickly reprogram the part
                *(DWORD *)(dwHostV3Base0 + VIDPROCCFG) = 0x09040080;                    // Disable processor
                
                *(DWORD *)(dwHostV3Base0 + VIDPROCCFG) =                                // Enable processor, 32bpp
                    ((*(DWORD *)(dwHostV3Base0 + VIDPROCCFG)) & ~0x100) | 0xC0081;      // fetch desktop, not overlay

                printf("Napalm.vxd: Overlays with 32bpp, faking it using desktop! (1)\n");
                fFake32bppOverlay = 1;
            }
            else
                fFake32bppOverlay = 0;
            
            // Make sure the stride is programmed
            if( fFake32bppOverlay && (iaddr==VIDDESKTOPOVERLAYSTRIDE) )
            {
                *(DWORD *)(dwHostV3Base0 + VIDDESKTOPOVERLAYSTRIDE) = (data >> 16) | (data && 0xFFFF0000);
            }
        }
#endif // WINSIM

        last_addr = addr;
        return;
    }
    //------- RAW LFB territory ---------------
    if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) 0\tRAW LFB %s\n",
            addr,data,data,cpriv->environment.name );
        csimRawLfbWrite( sst, addr, data, 4 );

        //Only let the parent device execute from the command fifo
        //This means that the childrens' command fifos don't really
        //get used.
        if ( cpriv->environment.parentDevice )
            csimSnoop32( sst, addr, data );

        return;
    }
    //------- CMD/AGP territory ---------------
    sanityCheck = SST_IS_CMDFIFO_ENABLED( sst ) && (!cpriv->inCmdFifoExecMode) &&
        (sst->status & (SST_FBI_BUSY | SST_TMU_BUSY | SST_BUSY | SST_GUI_BUSY));
    if ( SST_IS_CMDAGP_ADDR( iaddr ) )
    {
        RegInfo *ri;
        SstCRegs *sstc = &cpriv->cmd;

        iaddr -= SST_CMDAGP_OFFSET;
        ri = csimRegisterCmdInfo( iaddr );	// only register writes
        if ( ri != NULL )
        {			// if a valid register
            CmdFifo *fifo;
            CmdFifoPriv *fifopriv;

            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) AC\t%s %s\n", 
                addr,data,data,ri->name,cpriv->environment.name );
            data &= ri->mask;			// mask the data
            ((FxU32 *)sstc)[iaddr>>2] = data;	// and write it
            if ( (FxU32)sstc +iaddr > (FxU32)&(sstc->cmdFifo1) )
            {
                fifo = &sstc->cmdFifo1;
                fifopriv = &cpriv->fifo1data;
            }
            else
            {
                fifo = &sstc->cmdFifo0;
                fifopriv = &cpriv->fifo0data;
            }
            if ( ri->special == 200 )
            {		// CMD*_BUMP
                fifo->depth += data;
                csimCmdFifoExecuteN( sst, fifo, fifopriv, fifo->depth );
            }
            else if ( iaddr == MOVECMD )
                csimExecuteMoveCmd( sst );
            else if ( sanityCheck )
                GDBG_ERROR( "SET","direct reg write to 0x%x, CMD FIFO is on, chip not idle\n",addr );
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "SETAC","SETAC: bad address=0x%x, data=%d (x%x)\n %s", 
                addr,data,data,cpriv->environment.name );
        }
        last_addr = addr;
        return;
    }
    // now do a sanity check, if the CMD FIFO is enabled and we are not
    // writing a word from within the cmd fifo execute procedure, then
    // make sure that the chip is idle
    if ( sanityCheck )
        GDBG_ERROR( "SET","direct reg write to 0x%x, CMD FIFO is on, chip not idle %s\n",addr,
            cpriv->environment.name );

    //------- 2D territory ---------------
    if ( SST_IS_2D_ADDR( iaddr ) )
    {
        extern FxBool dontMessWithLaunched;
        extern FxBool InHostBlit;
        extern FxBool HostBlitComplete;
        FxU32 *paddr;
        SstGRegs *sstg = &cpriv->gui;

        iaddr -= SST_2D_OFFSET;				// get offset into 2d regs
        paddr = (FxU32 *)(iaddr + (FxU32)sstg);
        if ( paddr < sstg->launch )
        {			// register write
            RegInfo *ri = csimRegister2dInfo( iaddr );
            if ( ri == NULL ) goto badReg;
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) G\t%s %s\n", 
                addr,data,data,ri->name,cpriv->environment.name );

            if ( InHostBlit )
            {
                GDBG_ERROR( "SET", "writing non-launch register, but host blit "
                    "not complete\n" );
            }
            else if ( HostBlitComplete )
            {
                // we've received a "plain" register write, it's OK to allow
                // writes to the launch area again
                //
                HostBlitComplete = FXFALSE;
            }

            data &= ri->mask;
            if ( ri->altMap )
            {
                sstg->colorPattern[ri->altMap-1] = data;
            }
            else
            {
                ((FxU32 *)sstg)[iaddr>>2] = data;
                if ( ri->cmdCode && (data & SSTG_GO) ) sstgGo( sst, sstg );
            }
            cpriv->launched = FXFALSE;
            dontMessWithLaunched=FXFALSE;
        }
        else if ( paddr < sstg->colorPattern )
        {		// launch area
            FxU32 n = paddr - sstg->launch;
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) G\tLAUNCH[%d] %s\n", 
                addr,data,data,n,cpriv->environment.name );

            if ( HostBlitComplete )
            {
                GDBG_ERROR( "SET", "writing launch register immediately after "
                    "host blit has completed\n" );
            }

            if ( !dontMessWithLaunched )
            {
                sstgLaunch( sst, sstg, data );

                if ( !dontMessWithLaunched )  //sstgLaunch changes this dumb global
                    cpriv->launched = FXTRUE;
            }
        }
        else if ( paddr < sstg->colorPattern+64 )
        {	// color pattern
            FxU32 n = paddr - sstg->colorPattern;
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) G\tCPAT[%d] %s\n", 
                addr,data,data,n,cpriv->environment.name );

            if ( InHostBlit )
            {
                GDBG_ERROR( "SET", "writing non-launch register, but host blit "
                    "not complete\n" );
            }
            else if ( HostBlitComplete )
            {
                // we've received a "plain" register write, it's OK to allow
                // writes to the launch area again
                //
                HostBlitComplete = FXFALSE;
            }

            sstg->colorPattern[n] = data;
            cpriv->launched = FXFALSE;
            dontMessWithLaunched=FXFALSE;
        }
#if COLORTRANSLUT
        else if ( paddr < sstg->colorTransLut+256 )
        {	// color trans lut
            FxU32 n = paddr - sstg->colorTransLut;
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) G\tCLUT[%d] %s\n", 
                addr,data,data,n,cpriv->environment.name );
            sstg->colorTransLut[n] = data & 0xFFFFFF;	// mask down to 24 bits
            cpriv->launched = FXFALSE;
        }
#endif
        else
        {
            badReg:
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "SETG","SETG: bad address=0x%x, data=%d(x%x) %s\n", 
                addr,data,data,cpriv->environment.name );
            cpriv->launched = FXFALSE;
        }
        last_addr = addr;
        return;
    }
    if ( SST_IS_3D_ADDR( iaddr ) || SST_IS_3D_ALT_ADDR( iaddr ) )
        goto do3d;

    //------- TEXTURE territory ---------------
    if ( SST_IS_TEX_ADDR( iaddr ) )
    {
        if ( sst->tLOD & SST_TBIG )
            GDBG_INFO( 0, "Warning! (csimStore8) Don't use old texture ports for hummungo textures\n" );

        if ( GDBG_GET_DEBUGLEVEL( 195 ) )
        {	// optimized TEX data trace
            if ( addr == last_addr+4 )
                GDBG_PRINTF( "T+4 %x %s\n",	data, cpriv->environment.name );
            else
                GDBG_PRINTF( "TEX %x %x %s\n",	addr,data, cpriv->environment.name );
        }

        if ( SST_IS_TEX0_ADDR( iaddr ) )
        {
            iaddr -= SST_TEX0_OFFSET;    
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 4, tmu0TexturePort );
        }
        else
        {
            iaddr -= SST_TEX1_OFFSET;
            sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+1, iaddr, data, 4, tmu1TexturePort );
        }

        last_addr = addr;
        return;
    }

    //This is for the new 64MB texture aperture in Napalm. It's relative to texBaseAddr0
    if ( SST_IS_TEX2_ADDR( iaddr ) )
    {
        if ( addr == last_addr + 4 )
            GDBG_INFO( 195, "T+4 %x %s\n", data, cpriv->environment.name );
        else
            GDBG_INFO( 195, "TEX %x %x %s\n", addr, data, cpriv->environment.name );

        iaddr -= SST_TEX2_OFFSET;    
        sstTrexWriteMem( CSIM_PRIVATE( sst )->trex+0, iaddr, data, 4, largeTexturePort );

        last_addr = addr;
        return;
    }

    last_addr = addr;
    //------- RESERVED territory ---------------
    if ( SST_IS_RESERVED_ADDR( iaddr ) )
    {
        GDBG_ERROR( "SET","SET: bad address=0x%x (RESERVED not writable) %s\n", 
            addr, cpriv->environment.name );
    }
    //------- YUV territory ---------------
    else if ( SST_IS_YUV_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) 0\tYUV %s\n",
            addr,data,data, cpriv->environment.name );
        csimFbiYuvWrite( sst, addr, data, 4 );	// draw the pixel(s)
    }
    //------- LFB territory ---------------
    else if ( SST_IS_LFB_ADDR( iaddr ) )
    {
        GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) 0\tLFB %s\n",
            addr,data,data, cpriv->environment.name );
        csimFbiLfbWrite( sst, addr, data, 2 );	// draw the pixel(s)
    }
    else
    {
        GDBG_ERROR( "SET","SET: bad address=0x%x\n", addr );
    }    
    return;

    do3d:
    last_addr = addr;

    //Check our chip mask to see if we're active
    if ( (sst->chipMask & (1<<cpriv->environment.chipIndex)) == 0 )
    {
        //Make sure we always allow writes to chipMask
        if ( (addr & 0x3FFF) != 0x214 )
        {	  
            GDBG_INFO( 123,"       SET(0x%x,%11d(0x%08x)) skipped because of chip mask %s\n",
                addr,data,data, cpriv->environment.name );
            return;
        }
    }

    //------- 3D territory ---------------
    chipID = chipMASK = SST_CHIP_NUMBER( iaddr );	// get chip ID

    // turn 0,F into broadcast
    // H3 & H4 CMDFIFO packets only have MAX_NUM_TMUS+1 chip field bits.
    // Perform a broadcast if all of the packet's chip field bits are on.
    if ( chipMASK == 0 || 
        chipMASK == 0xF || 
        (CSIM_PRIVATE( sst )->inCmdFifoExecMode && chipMASK == SST_MASK( MAX_NUM_TMUS+1 )) )
        chipMASK = cpriv->chipMask;

    //If in 2 pixels per clock mode, register writes to either TMU
    //go to both TMU's
    if ( (globalSST->combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
        (cpriv->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
        (cpriv->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) )
    {
        if ( chipMASK & 0x6 )
        {
            if ( (chipMASK & 0x6) != 0x6 )
                GDBG_INFO( 125, "Forcing write to both TMU's %s(%d)\n", __FILE__, __LINE__ );

            chipMASK |= 0x6;
        }
    }

    if ( chipMASK & ~cpriv->chipMask )
    {		// if write to non-existing chip
        GDBG_ERROR( "SET","bad CHIP #%d, address=0x%x  data=%d (0x%08x)  %s\n",
            chipID,addr,data,data,cpriv->environment.name );
        return;
    }
    chipMASK &= cpriv->chipMask;		// mask with existing chips
    iaddr &= 0x3FF;				// mask off CHIP field
    {
        RegInfo *ri = csimRegister3dInfo( iaddr );
        if ( ri )
        {
            int reg = iaddr >> 2;

            // if WRAP msb is set then use alternate mapping
            if ( SST_FAKE_ADDRESS_GET_OFFSET( addr ) >= SST_3D_ALT_OFFSET )
            {
                if ( cpriv->io.miscInit1 & SST_ALT_REGMAPPING )
                {
                    reg += ri->altMap;
                    ri = csimRegister3dInfo( iaddr + (ri->altMap<<2) );
                }
            }
            GDBG_INFO( 120,"       SET(0x%x,%11d(0x%08x)) %x\t%s %s\n",
                addr,data,data, chipID,ri->name, cpriv->environment.name );
            if ( ri->special < 0 )
            {		// special < 0 implies that the reg
                chipMASK = cpriv->chipMask;	// is not-maskable, its sent to all chips
            }
            // perform conversions, either float-to-fix or int to int64
            // for float-to-fix the result is in both data(int32) and data64(int64)
            if ( ri->floatConversion )
            {
                FxI64 data64;
                if ( ri->floatConversion > 0 )
                {	// float-to-int conversion
                    //This is slow as shit, but f it.
                    //This might make it slighly more readable
                    //This has to be done for backwards compatibility
                    if ( (sst->renderMode & SST_RM_3D_MODE) != SST_RM_32BPP
                        && (!strcmp( ri->name, "FZ" ) ||
                        !strcmp( ri->name, "FDZDX" ) ||
                        !strcmp( ri->name, "FDZDY" )) )
                    {
                        //Make sure the 16 lsb fractional bits are zero 
                        data64 = _convertFloat2Fix64( data,ri->floatConversion-16 );
                        data64 = FX_SHL64( data64, 16 );
                    }
                    else
                        data64 = _convertFloat2Fix64( data,ri->floatConversion );

                        data = FX_LO64( data64 );	// copy to int32
                    reg -= 32;
                    ri -= 32;
                    GDBG_INFO( 121,"   f---SET(0x%x,   (0x%08x%08x)) %x\t%s\n",
                        addr,FX_HI64( data64 ),FX_LO64( data64 ), chipID,ri->name );
                }
                else
                {				// int to int64 conversion
                    FX_COPY32( data64,data );	// default value
                    // line up binary point, bits contains shift amount
                    data64 = FX_SHL64( data64,-ri->floatConversion );
                    // sign extend integer, no need to mask these
                    if ( ri->signExtend > 0 )
                    {		// if signExtend field
                        data64 = FX_SHL64( data64,ri->signExtend );
                        data64 = FX_SHR64( data64,ri->signExtend );
                    }
                }

                // now check for int64 stores, note that for floats "ri" is now
                // pointing to the integer register info structure
                if ( ri->floatConversion < 0 )
                {
                    // hack alert: readable holds index from sst->s64!!
                    reg = ri->readable >= 0 ? 0 : -ri->readable;
                    // for each chip in the array, write the data to it
                    if ( chipMASK & 0x1 ) (&(cpriv->fbiData.s64))[reg] = data64;
                    if ( chipMASK & 0x2 ) (&TMU_PRIVATE( cpriv->trex+0 )->s64)[reg] = data64;
#if MAX_NUM_TMUS > 1
                        if ( chipMASK & 0x4 ) (&TMU_PRIVATE( cpriv->trex+1 )->s64)[reg] = data64;
#endif
#if MAX_NUM_TMUS > 2
                        if ( chipMASK & 0x8 ) (&TMU_PRIVATE( cpriv->trex+2 )->s64)[reg] = data64;
#endif
#if MAX_NUM_TMUS > 3
                        if ( chipMASK & 0x10 ) (&TMU_PRIVATE( cpriv->trex+3 )->s64)[reg] = data64;
#endif
                    return;
                }
            }


            //EVIL: Begin
            //We need to enforce the rule that when switching between 2ppc and 1ppc,
            //there needs to be 12 NOPs issued to the TMUs before the TMU combineModes
            //are changed.
            if ( ri->special == 666187 && cpriv->environment.parentDevice &&
                ((chipMASK & 6) && !(chipMASK & 1)) ) //3d nop cmd
                previousNopCount++;

            if ( cpriv->environment.detectNopError &&
                ri->special == 187666 && cpriv->environment.parentDevice ) //combineMode
            {
                if ( (cpriv->trex[0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) &&
                    (cpriv->trex[1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) &&
                    !(data & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) &&
                    (previousNopCount < 12) )
                {
                    GDBG_ERROR( "csimStore32", "Damn sucka! You need to write 12 nops to the TMUs before switching from 2ppc to 1ppc\n" );
                }
            }
            //EVIL: End

            //EVIL: Begin
            //We need to change RegInfo.floatConversion
            //for "FZ", "FDZDX", and "FDZDY" because in 32bpp and 16bpp modes
            //The registers have different fixed point formats
            //The power of the goat lord flows forth from this
            if ( ri->special == 187 ) //This runs when renderMode register is changed
            {		
                int fs = ((data & SST_RM_3D_MODE) == SST_RM_32BPP) ?
                    SST_Z64_FRACBITS_32BPP : SST_Z64_FRACBITS_16BPP;
                GDBG_INFO( 104,"changing FZ* shift count to %d\n",fs );
                //This stuff shifts the float value to line up with
                //the 48 bit z iterator format (4.24.20 or 4.16.28)
                csimRegister3dInfo( FZ )->floatConversion = fs;
                csimRegister3dInfo( FDZDX )->floatConversion = fs;
                csimRegister3dInfo( FDZDY )->floatConversion = fs;
            }
            //EVIL: End

            if ( ri->special == 667 )  //This runs when colBufferAddr is changed
            {
                if ( data & SST_BUFFER_BASE_SELECT )		  
                    COL_BUFFER_ADDR_SECONDARY( sst ) = data & (~SST_BUFFER_BASE_SELECT);
                else
                {
                    COL_BUFFER_ADDR_PRIMARY( sst ) = data;
                    sst->colBufferAddr = data;
                }

                return;
            }

            if ( ri->special == 668 )  //This runs when auxBufferAddr is changed
            {
                if ( data & SST_BUFFER_BASE_SELECT )		  
                    AUX_BUFFER_ADDR_SECONDARY( sst ) = data & (~SST_BUFFER_BASE_SELECT);
                else
                {
                    AUX_BUFFER_ADDR_PRIMARY( sst ) = data;
                    sst->auxBufferAddr = data;
                }

                return;
            }


            if ( (ri->special == 10) && (data & 0x80000000) )
            {
                int index;

                // Loading 256 color pal entries using the NCC IQ table registers
                index = (data & 0x7f000000) >> 23;
                reg -= ((((long) &sst->nccTable0[4]) - ((long) sst)) >> 2);
                if ( reg & 1 ) index++;

                // Load palette[index] on all TREXs 
                data &= 0x00ffffff;
                if ( chipMASK & 0x2 ) TMU_PRIVATE( cpriv->trex+0 )->pal256.argb[index] = data;
#if MAX_NUM_TMUS > 1
                    if ( chipMASK & 0x4 ) TMU_PRIVATE( cpriv->trex+1 )->pal256.argb[index] = data;
#endif
#if MAX_NUM_TMUS > 2
                    if ( chipMASK & 0x8 ) TMU_PRIVATE( cpriv->trex+2 )->pal256.argb[index] = data;
#endif
#if MAX_NUM_TMUS > 3
                    if ( chipMASK & 0x10 ) TMU_PRIVATE( cpriv->trex+3 )->pal256.argb[index] = data;
#endif

                GDBG_INFO( 123,"Setting Palette Index %3d to %.08x\n", 
                    index, data );
                return;
            }

            // for each chip in the array, write the data to it
            data &= ri->mask;
            if ( ri->special )
            {		// requires special processing
                if ( (ri->special == 9) || (ri->special == 10) )
                {	
                    // NCC table invalidate
                    if ( chipMASK & 0x2 ) TMU_PRIVATE( cpriv->trex+0 )->nccValid0 = 0;
#if MAX_NUM_TMUS > 1
                        if ( chipMASK & 0x4 ) TMU_PRIVATE( cpriv->trex+1 )->nccValid0 = 0;
#endif
#if MAX_NUM_TMUS > 2
                        if ( chipMASK & 0x8 ) TMU_PRIVATE( cpriv->trex+2 )->nccValid0 = 0;
#endif
#if MAX_NUM_TMUS > 3
                        if ( chipMASK & 0x10 ) TMU_PRIVATE( cpriv->trex+3 )->nccValid0 = 0;
#endif
                }
                else if ( ri->special == 11 )
                {	// NCC table invalidate
                    if ( chipMASK & 0x2 ) TMU_PRIVATE( cpriv->trex+0 )->nccValid1 = 0;
#if MAX_NUM_TMUS > 1
                        if ( chipMASK & 0x4 ) TMU_PRIVATE( cpriv->trex+1 )->nccValid1 = 0;
#endif
#if MAX_NUM_TMUS > 2
                        if ( chipMASK & 0x8 ) TMU_PRIVATE( cpriv->trex+2 )->nccValid1 = 0;
#endif
#if MAX_NUM_TMUS > 3
                        if ( chipMASK & 0x10 ) TMU_PRIVATE( cpriv->trex+3 )->nccValid1 = 0;
#endif
                }
                else if ( ri->special == 123 )
                {	// setup ARGB packed
#ifndef NO_FLOAT	// NO_FLOAT_work
                    *(float *)&sst->sAlpha = (float)((data>>24)&0xFF);
                    *(float *)&sst->sRed = (float)((data>>16)&0xFF);
                    *(float *)&sst->sGreen = (float)((data>>8)&0xFF);
                    *(float *)&sst->sBlue = (float)((data>>0)&0xFF);
#else
                    /* NO_FLOAT
                         * this requires work.  Use table lookup to effect the
                         * translation from the given byte from int to float
                         * (stored as a dword)
                         */
                    sst->sAlpha = byteToFloat[(data >> 24) & 0xFF];
                    sst->sRed   = byteToFloat[(data >> 16) & 0xFF];
                    sst->sGreen = byteToFloat[(data >>  8) & 0xFF];
                    sst->sBlue  = byteToFloat[(data >>  0) & 0xFF];
#endif /* #ifndef NO_FLOAT */

                }
                // texture caching tracing, only works on TMU0
                else if ( ri->special == 700 )
                {
                    GDBG_INFO( 210, data != cpriv->trex[0].texBaseAddr ? 
                        "texbase change\n":"texbase-same\n");
                }
            }
            if ( chipMASK & 0x1 ) ((FxU32 *)sst)[reg] = data;
            if ( chipMASK & 0x2 ) ((FxU32 *)(cpriv->trex+0))[reg] = data;
#if MAX_NUM_TMUS > 1
                if ( chipMASK & 0x4 ) ((FxU32 *)(cpriv->trex+1))[reg] = data;
#endif
#if MAX_NUM_TMUS > 2
                if ( chipMASK & 0x8 ) ((FxU32 *)(cpriv->trex+2))[reg] = data;
#endif
#if MAX_NUM_TMUS > 3
                if ( chipMASK & 0x10 ) ((FxU32 *)(cpriv->trex+3))[reg] = data;
#endif
#ifdef WINSIM
            // Propagate some register writes down to the V3 card (but only for the
            // first chip)
            if( sst==globalSST )
            {
                if((iaddr==LEFTOVERLAYBUF) ||
                   (iaddr==SWAPBUFFERPEND))
                {
                    // We could be faking 32bpp overlays
                    if( fFake32bppOverlay==1 )
                    {
                        if( iaddr==LEFTOVERLAYBUF ) printf("Napalm.vxd: Writing to LEFTOVERLAYBUF (0x%X), with overlays faked!\n", data );
                        if( iaddr==SWAPBUFFERPEND ) printf("Napalm.vxd: Writing to SWAPBUFFERPEND (0x%X), wuth overlays faked!\n", data );
                    }
                    else
                    {
                        *(unsigned *)(dwHostV3Base0 + SST_3D_OFFSET + iaddr) = data;
                        printf("Napalm.vxd: Warning: csimStore32(0x%04X [3D], 0x%X) passed down to V3\n", 
                                iaddr, data );
                    }
                }

                // If we are trying to set 32bpp rendering mode, be sure overlays are not enabled
                if( iaddr==0x1E0 )
                {
                    SstIORegs *sstio = &cpriv->io;

                    if( ((data & SST_RM_3D_MODE) == SST_RM_32BPP) && (sstio->vidProcCfg & 0x100) )
                    {
                        // Turn overlays off and start using desktop start address
                        // QUickly reprogram the part
                        *(DWORD *)(dwHostV3Base0 + VIDPROCCFG) &= ~1;                           // Disable processor

                        *(DWORD *)(dwHostV3Base0 + VIDPROCCFG) =                                // Enable processor, 32bpp
                        ((*(DWORD *)(dwHostV3Base0 + VIDPROCCFG)) & ~0x100) | 0xC0081;          // fetch desktop, not overlay

                        printf("Napalm.vxd: Overlays with 32bpp, faking it using desktop! (2)\n");
                        fFake32bppOverlay = 1;
                    }
                    else
                        fFake32bppOverlay = 0;
                }
            
                // Make sure the stride is programmed
                if( fFake32bppOverlay && (iaddr==VIDDESKTOPOVERLAYSTRIDE) )
                {
                    *(DWORD *)(dwHostV3Base0 + VIDDESKTOPOVERLAYSTRIDE) = (data >> 16) | (data && 0xFFFF0000);
                }
            }
#endif // WINSIM

            // Only execute commands which include fbi.
            if ( (ri->cmdCode) && (chipMASK & 0x1) )
            {
                sstGo( sst,ri->cmdCode );
            }

            //Make sure that we're not enabling origin swapping with SLI
            if ( sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE )
            {
                if ( globalSST->fbzMode & SST_YORIGIN )
                    GDBG_ERROR( "csimStore32", "setting SST_YORIGIN of fbzMode with SLI!\n" );
                if ( globalSST->lfbMode & SST_LFB_YORIGIN )
                    GDBG_ERROR( "csimStore32", "setting SST_LFB_YORIGIN of lfbMode with SLI!\n" );		
            }
        }
        else
        {
            GDBG_ERROR( "SET","CHIP #%d bad address=0x%x, data=%d(x%x) %s\n",
                chipID,addr,data,data,cpriv->environment.name );
        }
#ifdef WINSIM
        // Darn hacks! We still need to update cprivs depending on the sli/aa setting

        if ( sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE )
        {
            cpriv->environment.sliEnabled = 1;

            // I dont quite understand why we set this since we never use it except
            // when comparing for dither warning in go.c [1028].  I will intentionally
            // set it to 4 assuming the driver writer will know that dither matrix should
            // be smaller than the band height
            cpriv->environment.sliBandHeight = 4;
        }
        else
        {
            cpriv->environment.sliEnabled = 0;
        }

        if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
        {
            cpriv->environment.aaEnabled = 1;
        }
        else
        {
            cpriv->environment.aaEnabled = 0;
        }
#endif // WINSIM    
    }
}

//----------------------------------------------------------------------
// snoop a 32-bit memory write to non-modal LFB space to see if it's
// a CMD FIFO write, and if so then process it
//----------------------------------------------------------------------
static void snoop32(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv,
			FxU32 addr, FxU32 offset, FxU32 data)
{
    // if CMD FIFO is enabled
    if ( fifo->baseSize & SST_EN_CMDFIFO )
    {
        // check to see if within CMD FIFO region
        if ( (offset>>12) >= fifo->baseAddrL &&
            (offset>>12) < fifo->baseAddrL + ((fifo->baseSize & SST_CMDFIFO_SIZE)+1) )
        {
            GDBG_INFO( 121,"       SET(0x%x,%11d(0x%08x)) %d\tCMDFIFO\n",
                addr,data,data,
                fifo == &CSIM_PRIVATE( sst )->cmd.cmdFifo0 ? 0:1);
            csimCmdFifoWrite( sst, fifo, fifopriv, offset, data );
        }
    }
}

void csimSnoop32( SstRegs *sst, FxU32 addr, FxU32 data )
{
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );
    FxU32 offset = SST_FAKE_ADDRESS_GET_OFFSET( addr ) - SST_RAW_LFB_OFFSET;
    snoop32( sst,&cpriv->cmd.cmdFifo0, &cpriv->fifo0data, addr, offset, data );
    snoop32( sst,&cpriv->cmd.cmdFifo1, &cpriv->fifo1data, addr, offset, data );
}


//----------------------------------------------------------------------
// execute a 8 bit read from SST
//----------------------------------------------------------------------
FxU8 csimLoad8( SstRegs *sst, FxU32 addr )
{
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    FxU8 data = 0xbf;

    //------- RAW LFB territory ---------------
    if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        data = (FxU8)csimRawLfbRead( sst, addr, 1 );
        GDBG_INFO( 120,"       GET8(0x%x,%11d(0x%08x)) 0\tRAW LFB\n",
            addr,data,data );
    }
    else
    {
        GDBG_ERROR( "GET8","GET8: bad address=0x%x\n", addr );
    }
    guiKeepAlive( 1 );
    return data;
}

//----------------------------------------------------------------------
// execute a 16 bit read from SST
//----------------------------------------------------------------------
FxU16 csimLoad16( SstRegs *sst, FxU32 addr )
{
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    FxU16 data = 0xbadf;

    //------- RAW LFB territory ---------------
    if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        data = (FxU16)csimRawLfbRead( sst, addr, 2 );
        GDBG_INFO( 120,"       GET16(0x%x,%11d(0x%08x)) 0\tRAW LFB\n",
            addr,data,data );
    }
    else
    {
        GDBG_ERROR( "GET16","GET16: bad address=0x%x\n", addr );
    }
    guiKeepAlive( 1 );
    return data;
}

//----------------------------------------------------------------------
// execute a 32 bit read from SST, iaddr is offset into chip
//----------------------------------------------------------------------
FxU32 csimLoad32( SstRegs *sst, FxU32 addr )
{
    FxU32 data = 0xdeadbeef;
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET( addr );
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    guiKeepAlive( 1 );
    // possibly execute some cmds from the CMD FIFO
    csimCmdFifoExecuteSome( sst );

    //------- IO territory ---------------
    if ( SST_IS_IO_ADDR( iaddr ) )
    {
        RegInfo *ri;
        SstIORegs *sstio = &cpriv->io;

        iaddr -= SST_IO_OFFSET;
        ri = csimRegisterIoInfo( iaddr );		// only register reads
        if ( ri != NULL )
        {			// if a valid register
            if ( ri->special == 100 ) // dacData
                data = cpriv->clut512.rgb[sstio->dacAddr];
            else if ( ri->special == 12345 )
            {
                if ( sstio->lfbMemoryConfig & SST_RAW_LFB_READ_CONTROL )
                {
                    //Use lfbMemoryTileCompare
                    data = sstio->lfbMemoryTileCompare;
                    GDBG_INFO( 122,"          Routing GET through lfbMemoryTileCompare\n" );
                }
                else
                { 
                    //Use lfbMemoryTileCtrl
                    data = sstio->lfbMemoryTileCtrl;
                    GDBG_INFO( 122,"          Routing GET through lfbMemoryTileCtrl\n" );
                }

                //Stick in lfbMemoryConfig stuff
                data = (data & (~LFB_MEMORY_CONFIG_MASK)) | (sstio->lfbMemoryConfig & LFB_MEMORY_CONFIG_MASK);
            }
            else
                data = ((FxU32 *)sstio)[iaddr>>2];
#ifdef WINSIM
            // Fetch some Video register off the V3 card, if not faked
            if( (iaddr==VIDMAXRGBDELTA)             ||
                (iaddr==VIDPIXELBUFTHOLD)           ||
                (iaddr==VIDSCREENSIZE)              ||
                (iaddr==VIDOVERLAYSTARTCOORDS)      ||
                (iaddr==VIDOVERLAYENDSCREENCOORD)   ||
                (iaddr==VIDOVERLAYDUDX)             ||
                (iaddr==VIDOVERLAYDUDXOFFSETSRCWIDTH) ||
                (iaddr==VIDOVERLAYDVDY)             ||
                (iaddr==VIDOVERLAYDVDYOFFSET)       ||
                (iaddr==VIDDESKTOPSTARTADDR)        ||
                (iaddr==VIDDESKTOPOVERLAYSTRIDE))
                {
                    data = *(DWORD *)(dwHostV3Base0 + iaddr);
                    printf("Napalm.vxd: Warning: csimLoad32(0x%04X [I/O], 0x%X) read from V3\n", 
                            iaddr, data );
                }

            if( iaddr==VIDPROCCFG )
            {
                if( ! fFake32bppOverlay )
                {
                    data = *(DWORD *)(dwHostV3Base0 + iaddr);
                    printf("Napalm.vxd: Warning: csimLoad32(0x%04X [I/O]) = 0x%X read from V3\n",
                            iaddr, data );
                }
            }
#endif // WINSIM          

            GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) IO\t%s\n", addr,data,data,ri->name );
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "GETIO","GETIO: bad address=0x%x, data=%d (x%x)\n", addr,data,data );
        }
    }
    //------- CMD/AGP territory ---------------
    else if ( SST_IS_CMDAGP_ADDR( iaddr ) )
    {
        RegInfo *ri;
        SstCRegs *sstc = &cpriv->cmd;

	//Because the csim is such a piece of junk, it really only
	//runs from one command fifo. Consequently, all reads to this
	//region should be redirected to the parent device
	if(!cpriv->environment.parentDevice)
	  {
	    assert(CSIM_PRIVATE(globalSST)->environment.parentDevice);
	    return(csimLoad32(globalSST, addr));
	  }

        iaddr -= SST_CMDAGP_OFFSET;
        ri = csimRegisterCmdInfo( iaddr );	// only register reads
        if ( ri != NULL )
        {			// if a valid register
            data = ((FxU32 *)sstc)[iaddr>>2];
            GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) AC\t%s\n", addr,data,data,ri->name );
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "GETAC","GETAC: bad address=0x%x, data=%d (x%x)\n", addr,data,data );
        }
    }
    //------- 2D territory ---------------
    else if ( SST_IS_2D_ADDR( iaddr ) )
    {
        FxU32 *paddr;
        SstGRegs *sstg = & CSIM_PRIVATE( sst )->gui;

        iaddr -= SST_2D_OFFSET;				// get offset into 2d regs
        paddr = (FxU32 *)(iaddr + (FxU32)sstg);
        if ( paddr < sstg->colorPattern )
        {		// register write
            RegInfo *ri = csimRegister2dInfo( iaddr );
            if ( ri )
            {
                if ( ri->readable > 0 )
                {
                    if ( ri->altMap )
                    {
                        data = sstg->colorPattern[ri->altMap-1];
                    }
                    else
                    {
                        data = ((FxU32 *)sstg)[iaddr>>2];
                    }
                    GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) G\t%s\n",
                        addr,data,data, ri->name );
                }
                else
                    GDBG_ERROR( "GETG","GETG: bad address=0x%x (not readable)\n", addr );
            }
        }
        else if ( paddr < sstg->colorPattern+64 )
        {	// color pattern
            FxU32 n = paddr - sstg->colorPattern;
            data = sstg->colorPattern[n];
            GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) G\tCPAT[%d]\n", addr,data,data,n );
        }
#if COLORTRANSLUT
        else if ( paddr < sstg->colorTransLut+256 )
        {	// color trans lut
            FxU32 n = paddr - sstg->colorTransLut;
            data = sstg->colorTransLut[n];
            GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) G\tCLUT[%d]\n", addr,data,data,n );
        }
#endif
        else
            GDBG_ERROR( "GETG","GETG: bad address=0x%x\n", addr );
    }
    //------- 3D territory ---------------
    else if ( SST_IS_3D_ADDR( iaddr ) || SST_IS_3D_ALT_ADDR( iaddr ) )
    {
        iaddr -= SST_3D_OFFSET;
        if ( SST_CHIP_NUMBER( iaddr ) )	// only allow reads from CHIP=0
            GDBG_ERROR( "GET","CHIP #%d bad address=0x%x\n",
                (iaddr >> 10) & 0xF,addr );
        else
        {
            RegInfo *ri = csimRegister3dInfo( iaddr );
            if ( ri )
            {
                if ( ri->readable > 0 )
                {
                    data = ((FxU32 *)sst)[iaddr>>2];

                    // GD: Make bit 6 of the status register toggle (V-sync) with each read
                    if ( iaddr==STATUS )
                    {
                        static int V_Sync = 0;
                        data   ^= V_Sync;
                        V_Sync ^= 1 << 6;
                    }

                    GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) 0\t%s\n",
                        addr,data,data, ri->name );
                }
                else
                    GDBG_ERROR( "GET","GET: bad address=0x%x (not readable)\n", addr );
            }
            else
                GDBG_ERROR( "GET","GET: bad address=0x%x\n", addr );
        }
    }
    //------- TEX territory ---------------
    else if ( SST_IS_TEX_ADDR( iaddr ) )
    {
        GDBG_ERROR( "GET","bad address=0x%x (TEX not readable)\n", addr );
    }
    //------- RESERVED territory ---------------
    else if ( SST_IS_RESERVED_ADDR( iaddr ) )
    {
        GDBG_ERROR( "GET","bad address=0x%x (RESERVED not readable)\n", addr );
    }
    //------- YUV territory ---------------
    else if ( SST_IS_YUV_ADDR( iaddr ) )
    {
        GDBG_ERROR( "GET","bad address=0x%x (YUV not readable)\n", addr );
    }

    //------- RAW LFB territory ---------------
    else if ( SST_IS_RAW_LFB_ADDR( iaddr ) )
    {
        data = csimRawLfbRead( sst, addr, 4 );
        GDBG_INFO( 120,"       GET(0x%x,%11d(0x%08x)) 0\tRAW LFB\n",
            addr,data,data );
    }
    else
    {
        GDBG_ERROR( "GET","GET: bad address=0x%x\n", addr );
    }

    return data;
}

////////////////////////////////////////////////////////////////////////
//
//                   PCI Configuration Register Junk
//
////////////////////////////////////////////////////////////////////////
//This initializes the PCI config registers
void csimCfgInitialize(SstRegs *sst)
{
  CsimPrivate *cpriv = CSIM_PRIVATE( sst );
  SstPCIConfigRegs *pciConfigRegs = &cpriv->pciConfigRegs;
  FxU32 deviceID;
  FxU8 *deviceIDString;

  if((deviceIDString = getenv("SST_DEVICE_ID")))
    {
      deviceID = atoi(deviceIDString);
      assert(deviceID < 128);
    }
  else
    deviceID = CSIM_DEFAULT_DEVICE_ID;

  //Zero the f'er
  memset( (void *)pciConfigRegs, 0, sizeof(cpriv->pciConfigRegs) );

  //vendorID;	        //0	15:0	3dfx Interactive Vendor Identification
  //deviceID;	        //2	15:0	Device Identification
  pciConfigRegs->deviceID_vendorID = (deviceID << 16) | _3DFX_PCI_ID;

    //status;   	        //6	15:0	PCI device status
  pciConfigRegs->status_command = ((1<<5) << 16);    //66MHz capable (see PCI 2.1 spec page 192)

  //revisionID;	        //8	7:0	Revision Identification
  //classCode[3];       //9	23:0	Generic functional description of PCI device
  pciConfigRegs->classCode_revisionID = (0x3 << 8) | 1;    //250 nanometer Napalm    
  //Display Controller device (PCI 2.1 page 189)

  //memBaseAddr1;	//20	31:0	Memory Base Address (LFB)
  pciConfigRegs->memBaseAddr1 = 0x80000000;

  //ioBaseAddr;   	//24	31:0	I/O Base Address
  pciConfigRegs->ioBaseAddr = 0xFFFFFF01;
}

FxU32 csimCfgLoad32(SstRegs *sst, FxU32 registerOffset)
{
    FxU32 data;
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );
    FxU32 *pciConfigRegs = (FxU32 *)(&cpriv->pciConfigRegs);

    //Make sure we're not reading out of bounds;
    assert( registerOffset < 256 );

    //Make sure we're aligned
    assert( (registerOffset & 0x3) == 0 );

        data = pciConfigRegs[registerOffset >> 2];

    GDBG_INFO( 118, "   csimCfgLoad32(0x%x) = 0x%x  %s\n",
        registerOffset, data, cpriv->environment.name );

#ifdef WINSIM
    // Print out which sst chip's configuration register are we reading
    if( sst==globalSST )
        printf("Napalm.vxd: [Parent] csimCfgLoad32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[0] )
            printf("Napalm.vxd: [Child 1] csimCfgLoad32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[1] )
            printf("Napalm.vxd: [Child 2] csimCfgLoad32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[2] )
            printf("Napalm.vxd: [Child 3] csimCfgLoad32(0x%2X, 0x%04X)\n", registerOffset, data );
    else
         printf("ERROR: Napalm.vxd: csimCfgLoad32: Invalid sst pointer of %08X", (int) sst );

#endif // WINSIM

    return( data );
}

void csimCfgStore32(SstRegs *sst, FxU32 registerOffset, FxU32 data)
{
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );
    FxU32 *pciConfigRegs = (FxU32 *)(&cpriv->pciConfigRegs);

    //Make sure we're not writing out of bounds;
    assert( registerOffset < 256 );    

    //Make sure we're aligned
    assert( (registerOffset & 0x3) == 0 );

    GDBG_INFO( 118, "   csimCfgStore32(0x%x, 0x%x)  %s\n",
        registerOffset, data, cpriv->environment.name );

    pciConfigRegs[registerOffset >> 2] = data;
    
#ifdef WINSIM
    // Set some internal variables depending on the PCI config register setting
    
    if( registerOffset==148 )   // cfgAALfbCtrl register
    {
        if( data & SST_AA_LFB_RD_DIVIDE_BY_FOUR )
            cpriv->environment.aaSampleCount = 4;
        else
            cpriv->environment.aaSampleCount = 2;
    }

    // Print out which sst chip's configuration register are we writing
    if( sst==globalSST )
        printf("Napalm.vxd: [Parent] csimCfgStore32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[0] )
            printf("Napalm.vxd: [Child 1] csimCfgStore32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[1] )
            printf("Napalm.vxd: [Child 2] csimCfgStore32(0x%2X, 0x%04X)\n", registerOffset, data );
    else 
        if( sst==globalChildrenSST[2] )
            printf("Napalm.vxd: [Child 3] csimCfgStore32(0x%2X, 0x%04X)\n", registerOffset, data );
    else
         printf("ERROR: Napalm.vxd: csimCfgStore32: Invalid sst pointer of %08X", (int) sst );

#endif // WINSIM
}



//
// Execute a 32 bit port write to SST
//
void csimOutPort32( SstRegs *sst, FxU16 port, FxU32 data )
{
#ifdef HAL_NEW_PORT_IO
    FxU32 iport = SST_FAKE_PORT_GET_OFFSET( port );
#else
    FxU32 iport = port;
#endif
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    guiKeepAlive( 1 );

    if ( iport < SIZEOF_SSTIO )
    {
        // process
        RegInfo *ri;
        SstIORegs *sstio = &cpriv->io;

            iport -= SST_IO_OFFSET;
        ri = csimRegisterIoInfo( iport );		// only register reads
        if ( ri != NULL )
        {			// if a valid register
            GDBG_INFO( 120,"       SETIO(0x%x,%11d(0x%08x)) IO\t%s\n",port,data,data,ri->name );
            data &= ri->mask;			// mask it
            ((FxU32 *)sstio)[iport>>2] = data;	// and write it
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "SETIO","bad port=0x%x, data=%d(x%x)\n",port,data,data );
        }
        if ( ri->special == 100 )
        {  // dacData
            cpriv->clut512.rgb[sstio->dacAddr] = sstio->dacData;   // store data in the clut
        }
    }
    else
    {
        // everything else is an error, but decode it to be nice
        GDBG_ERROR( "SETIO","bad port=0x%x, data=%d(x%x)\n",port,data,data );
    }

    return;
}

//
// Execute a 32 bit port read from SST
//
FxU32 csimInPort32( SstRegs *sst, FxU16 port )
{
#ifdef HAL_NEW_PORT_IO
    FxU32 iport = SST_FAKE_PORT_GET_OFFSET( port );
#else
    FxU32 iport = port;
#endif
    FxU32 data = 0xdeadbeef;
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    guiKeepAlive( 1 );

    if ( iport < SIZEOF_SSTIO )
    {
        // process
        RegInfo *ri;
        SstIORegs *sstio = &cpriv->io;

            iport -= SST_IO_OFFSET;
        ri = csimRegisterIoInfo( iport );		// only register reads
        if ( ri != NULL )
        {			// if a valid register
            if ( ri->special == 100 ) // dacData
                data = cpriv->clut512.rgb[sstio->dacAddr];
            else
                data = ((FxU32 *)sstio)[iport>>2];
            GDBG_INFO( 120,"       GETIO(0x%x,%11d(0x%08x)) IO\t%s\n", port,data,data,ri->name );
        }
        else
        {
            // everything else is an error, but decode it to be nice
            GDBG_ERROR( "GETIO","bad port=0x%x, data=%d(x%x)\n",port,data,data );
        }
    }
    else
    {
        // everything else is an error, but decode it to be nice
        GDBG_ERROR( "GETIO","bad port=0x%x, data=%d(x%x)\n",port,data,data );
    }

    return data;
}

//----------------------------------------------------------------------
// read or write data into memory[offset]
//	caller is responsible for converting (x,y) into offset
//	and accounting for surface baseAddress
//----------------------------------------------------------------------

// cause a fault - we want to instantly find out where we went wrong and debug
// it, so no use in continuing, this makes it easier to find with a debugger

// GD  This is fine when running csim with diags, but it is not desired
//     behavior when using it with the simulated environment where it
//     runs in ring-0 as a vxd.  We need a way to return to the caller as
//     well since nobody will do it for us...

#ifdef WINSIM

extern int fault_breaks;

    #define fault()    if( fault_breaks ) _asm int 1; else return

extern unsigned dwNapalmBase1;
extern unsigned dwHostV3Base1;

#else
  #define fault()    { *(int *)0 = 0; }
#endif


FX_EXPORT FxU32 FX_CSTYLE
readMem8(volatile FxU8 *memory, int offset, int memorySize, char *where)
{
    FxU32 data;
    CsimPrivate *cpriv;

    if ( offset >= memorySize )
    {
        GDBG_ERROR( "ReadMem8","ReadMem8: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

    data = memory[offset];
    if ( GDBG_GET_DEBUGLEVEL( GRL ) )
        GDBG_INFO( GRL,"ReadMem8(0x%x) => 0x%x %s\n",
            offset,data,cpriv->environment.name );

    return( data );
}

FX_EXPORT FxU32 FX_CSTYLE
readMem16(volatile FxU8 *memory, int offset, int memorySize, char *where)
{
    FxU32 data;
    CsimPrivate *cpriv;

    if ( offset >= memorySize-1 )
    {
        GDBG_ERROR( "ReadMem16","ReadMem16: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }
    if ( offset & 1 )
        GDBG_ERROR( "ReadMem16","ReadMem16: unaligned 16-bit read 0x%x\n",offset );

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

#ifdef ENDB
    data = memory[offset] | (memory[offset+1] << 8);
#else
    data = *(FxU16 *)&memory[offset];
#endif

    if ( GDBG_GET_DEBUGLEVEL( GRL ) )
        GDBG_INFO( GRL,"%sReadMem16(0x%x) => 0x%x %s  (memory=0x%x)\n",
            where,offset,(FxU16)data,cpriv->environment.name,memory );      

    return( data );
}

FX_EXPORT FxU32 FX_CSTYLE
readMem24(volatile FxU8 *memory, int tiled, int offset, int memorySize, char *where)
{
    int offset_save = offset;
    FxU32 data;
    CsimPrivate *cpriv;

    if ( offset >= memorySize-2 )
    {
        GDBG_ERROR( "ReadMem24","ReadMem24: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

    // read byte 0
    data = memory[offset];
    offset++;


    // read byte 1
    if ( tiled && (offset & SST_TILE_WIDTH_MASK) == 0 )
    {
        offset += SST_TILE_SIZE - SST_TILE_WIDTH;
        GDBG_INFO( GRL,"%sReadMem24: wrapping byte 1 to offset=0x%x\n",where,offset );
        if ( offset >= memorySize )
        {
            GDBG_ERROR( "ReadMem24","ReadMem24: memory offset 0x%x (%d) too large\n",offset,offset );
            fault( );
        }      
    }
    data |= memory[offset] << 8;
    offset++;

    // read byte 2
    if ( tiled && (offset & SST_TILE_WIDTH_MASK) == 0 )
    {
        offset += SST_TILE_SIZE - SST_TILE_WIDTH;
        GDBG_INFO( GRL,"%sReadMem24: wrapping byte 2 to offset=0x%x\n",where,offset );
        if ( offset >= memorySize-2 )
        {
            GDBG_ERROR( "ReadMem24","ReadMem24: memory offset 0x%x (%d) too large\n",offset,offset );
            fault( );
        }
    }
    data |= memory[offset] << 16;

    if ( GDBG_GET_DEBUGLEVEL( GRL ) )
        GDBG_INFO( GRL,"%sReadMem24(0x%x) => 0x%x %s\n",where,offset_save,data,tiled?"T":"L");

    return( data );
}

FX_EXPORT FxU32 FX_CSTYLE
readMem32(volatile FxU8 *memory, int offset, int memorySize, char *where)
{
    CsimPrivate *cpriv;
    FxU32 data;

    if ( offset >= memorySize-3 )
    {
        GDBG_ERROR( "ReadMem32","ReadMem32: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }
    if ( offset & 3 )
        GDBG_ERROR( "ReadMem32","ReadMem32: unaligned 32-bit read 0x%x\n",offset );

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

#ifdef ENDB
    data = memory[offset] | 
        (memory[offset+1] << 8) |
        (memory[offset+2] << 16) | 
        (memory[offset+3] << 24);
#else
    data = *(FxU32 *)&memory[offset];
#endif
    if ( GDBG_GET_DEBUGLEVEL( GRL ) )
        GDBG_INFO( GRL,"%sReadMem32(0x%x) => 0x%x\n",where,offset,data );      

    return( data );
}

FX_EXPORT FxU32 FX_CSTYLE
csimReadMem8(CsimPrivate *cp, int offset)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        return readMem8( cp->memory,offset,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem8", __FILE__, __LINE__ );

    return( 0xdefaced4 );  
}

FX_EXPORT FxU32 FX_CSTYLE
csimReadMem16(CsimPrivate *cp, int offset)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        return readMem16( cp->memory,offset,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem16", __FILE__, __LINE__ );

    return( 0xdefaced3 );
}

FX_EXPORT FxU32 FX_CSTYLE
csimReadMem24(CsimPrivate *cp, int tiled, int offset)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        return readMem24( cp->memory,tiled,offset,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem24", __FILE__, __LINE__ );

    return( 0xdefaced2 );
}

FX_EXPORT FxU32 FX_CSTYLE
csimReadMem32(CsimPrivate *cp, int offset)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        return readMem32( cp->memory,offset,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem32", __FILE__, __LINE__ );

    return( 0xdefaced1 );
}

FX_EXPORT FxU32* FX_CSTYLE
csimReadMem64(CsimPrivate *cp, int offset, FxU32 *block)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )
    {
        block[0]=readMem32( cp->memory,offset+0, cp->memorySizeInBytes,"csim" ); 
        block[1]=readMem32( cp->memory,offset+4, cp->memorySizeInBytes,"csim" ); 
    }
    else
    {
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem64", __FILE__, __LINE__ );
        block[0]=0xdefaced5;
        block[1]=0xdefaced6;
    }
    return( block );
}

FX_EXPORT FxU32* FX_CSTYLE
csimReadMem128(CsimPrivate *cp, int offset, FxU32 *block)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )
    {
        block[0]=readMem32( cp->memory,offset+0, cp->memorySizeInBytes,"csim" ); 
        block[1]=readMem32( cp->memory,offset+4, cp->memorySizeInBytes,"csim" ); 
        block[2]=readMem32( cp->memory,offset+8, cp->memorySizeInBytes,"csim" ); 
        block[3]=readMem32( cp->memory,offset+12,cp->memorySizeInBytes,"csim" );   
    }
    else
    {
        csimCommandFifoAccessErrorMessage( cp, offset, "csimReadMem128", __FILE__, __LINE__ );
        block[0]=0xdefaced7;
        block[0]=0xdefaced8;
        block[0]=0xdefaced9;
        block[0]=0xdefaceda;
    }
    return( block );
}


FX_EXPORT void FX_CSTYLE
writeMem8(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where)
{
    CsimPrivate *cpriv;

    if ( offset >= memorySize )
    {
        GDBG_ERROR( "WriteMem8","WriteMem8: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

    if ( GDBG_GET_DEBUGLEVEL( GWL ) )
        GDBG_INFO( GWL,"%sWriteMem8(0x%x,0x%x)\n",where,offset,(FxU8)data );
    memory[offset] = (FxU8)data;

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < 0x1000000) )
            *(char *)(dwHostV3Base1 + real_offset) = (char) data;
    }
#endif // WINSIM  
}

FX_EXPORT void FX_CSTYLE
writeMem16(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where)
{
    CsimPrivate *cpriv;

    if ( offset >= memorySize-1 )
    {
        GDBG_ERROR( "WriteMem16","WriteMem16: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }
    if ( offset & 1 )
        GDBG_ERROR( "WriteMem16","WriteMem16: unaligned 16-bit write 0x%x\n",offset );

    cpriv=CSIM_PRIVATE( globalSST );

    assert( cpriv->memorySizeInBytes == memorySize );

    if ( GDBG_GET_DEBUGLEVEL( GWL ) )
        GDBG_INFO( GWL,"%sWriteMem16(0x%x,0x%x) (memory=0x%x)\n",where,offset,(FxU16)data,memory );
#ifdef ENDB
    memory[offset] = (FxU8)(data & 0xFF);
    memory[offset+1] = (FxU8)((data>>8) & 0xFF);
#else    
    *(FxU16 *)&memory[offset] = (FxU16)data;
#endif

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < (0x1000000 - 1)) )
        {
            *(char *)(dwHostV3Base1 + real_offset + 0) = (char) (data & 0xFF);
            *(char *)(dwHostV3Base1 + real_offset + 1) = (char) ((data >> 8) & 0xFF);
        }
    }
#endif // WINSIM
}

FX_EXPORT void FX_CSTYLE
writeMem24(volatile FxU8 *memory, int tiled, int offset, FxU32 data, int memorySize, char *where)
{
    CsimPrivate *cpriv;

    if ( offset >= memorySize-2 )
    {
        GDBG_ERROR( "WriteMem24","WriteMem24: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }

    cpriv=CSIM_PRIVATE( globalSST );
    assert( cpriv->memorySizeInBytes == memorySize );

    if ( GDBG_GET_DEBUGLEVEL( GWL ) )
        GDBG_INFO( GWL,"%sWriteMem24(0x%x,0x%x) %s\n",where,offset,data&0xFFFFFF,tiled?"T":"L");


    // write byte 0
    memory[offset] = (FxU8)(data & 0xFF);

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < 0x1000000) )
            *(char *)(dwHostV3Base1 + real_offset) = (FxU8)(data & 0xFF);
    }
#endif // WINSIM  

    offset++;

    // write byte 1
    if ( tiled && (offset & SST_TILE_WIDTH_MASK) == 0 )
    {
        offset += SST_TILE_SIZE - SST_TILE_WIDTH;
        GDBG_INFO( GWL,"%sWriteMem24: wrapping byte 1 to offset=0x%x\n",where,offset );
        if ( offset >= memorySize )
        {
            GDBG_ERROR( "WriteMem24","WriteMem24: memory offset 0x%x (%d) too large\n",offset,offset );
            fault( );
        }
    }
    memory[offset] = (FxU8)((data >> 8) & 0xFF);

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < 0x1000000) )
            *(char *)(dwHostV3Base1 + real_offset) = (FxU8)((data >> 8) & 0xFF);
    }
#endif // WINSIM  

    offset++;

    // write byte 2
    if ( tiled && (offset & SST_TILE_WIDTH_MASK) == 0 )
    {
        offset += SST_TILE_SIZE - SST_TILE_WIDTH;
        GDBG_INFO( GWL,"%sWriteMem24: wrapping byte 2 to offset=0x%x\n",where,offset );
        if ( offset >= memorySize )
        {
            GDBG_ERROR( "WriteMem24","WriteMem24: memory offset 0x%x (%d) too large\n",offset,offset );
            fault( );
        }
    }
    memory[offset] = (FxU8)((data >> 16) & 0xFF);

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < 0x1000000) )
            *(char *)(dwHostV3Base1 + real_offset) = (FxU8)((data >> 16) & 0xFF);
    }
#endif // WINSIM  
}

FX_EXPORT void FX_CSTYLE
writeMem32(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where)
{
    CsimPrivate *cpriv;

    if ( offset >= memorySize-3 )
    {
        GDBG_ERROR( "WriteMem32","WriteMem32: memory offset 0x%x (%d) too large\n",offset,offset );
        fault( );
    }
    if ( offset & 3 )
        GDBG_ERROR( "WriteMem32","WriteMem32: unaligned 32-bit write 0x%x\n",offset );

    cpriv=CSIM_PRIVATE( globalSST );  
    assert( cpriv->memorySizeInBytes == memorySize );

    if ( GDBG_GET_DEBUGLEVEL( GWL ) )
        GDBG_INFO( GWL,"%sWriteMem32(0x%x,0x%x)\n",where,offset,data );
#ifdef ENDB
    memory[offset] = (FxU8)(data & 0xFF);
    memory[offset+1] = (FxU8)((data>>8) & 0xFF);
    memory[offset+2] = (FxU8)((data>>16) & 0xFF);
    memory[offset+3] = (FxU8)((data>>24) & 0xFF);
#else    
    *(FxU32 *)&memory[offset] = data;
#endif

#ifdef WINSIM
    // If there is no AA or SLI, write the pixel in the composite buffer.
    // Otherwise wait, since it will be written down in csimWritePixel()
    //
    if( !((globalSST->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) || ((globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE))) )
    {
        int real_offset = (unsigned) memory + offset - dwNapalmBase1;

        if ( (real_offset >= 0) && (real_offset < (0x1000000 - 3)) )
            *(unsigned  *)(dwHostV3Base1 + real_offset) = data;
    }
#endif // WINSIM  
}


FX_EXPORT void FX_CSTYLE
csimWriteMem8(CsimPrivate *cp, int offset, FxU32 data)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        writeMem8( cp->memory,offset,data,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimWriteMem8", __FILE__, __LINE__ );
}

FX_EXPORT void FX_CSTYLE
csimWriteMem16(CsimPrivate *cp, int offset, FxU32 data)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        writeMem16( cp->memory,offset,data,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimWriteMem16", __FILE__, __LINE__ );
}

FX_EXPORT void FX_CSTYLE
csimWriteMem24(CsimPrivate *cp, int tiled, int offset, FxU32 data)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        writeMem24( cp->memory,tiled,offset,data,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimWriteMem24", __FILE__, __LINE__ );
}

FX_EXPORT void FX_CSTYLE
csimWriteMem32(CsimPrivate *cp, int offset, FxU32 data)
{ 
    if ( !csimIsCommandFifoAddress( cp, offset ) ||
        cp->environment.allowAccessesToCommandFifoRegion )          
        writeMem32( cp->memory,offset,data,cp->memorySizeInBytes,"csim" ); 
    else
        csimCommandFifoAccessErrorMessage( cp, offset, "csimWriteMem32", __FILE__, __LINE__ );
}

//
// compute the byte address of a pixel in tiled memory space
//
// NOTE: input and output address will be wrapped between [0,max memory)
FX_EXPORT FxU32 FX_CSTYLE tiledAddress(FxU32 base, FxU32 stride, FxU32 depth, int x, int y)
{
    FxI32 addr;
    FxU32 tBase, xBase, yBase;
    FxI32 xLoc, yLoc;
    FxU32 xTile, yTile;
    FxU32 xOffset, yOffset;
    FxU32 nt;

    assert( depth > 0 );

        base &= SST_BUFFER_BASE_ADDR;  // mask undefined bits

    tBase = (base & SST_BUFFER_BASE_T) >> SST_BUFFER_BASE_T_SHIFT; // base page
    xBase = (base & SST_BUFFER_BASE_X) >> SST_BUFFER_BASE_X_SHIFT; // base x
    yBase = (base & SST_BUFFER_BASE_Y) >> SST_BUFFER_BASE_Y_SHIFT; // base y

    xLoc = xBase+x*depth;                        // x locn in bytes  
    yLoc = yBase+y;                              // y locn in bytes

    if ( xLoc < 0 )
    {                            // adjust base so xlocn is positive
        nt = (-xLoc + SST_TILE_WIDTH - 1) / SST_TILE_WIDTH;
        tBase -= nt;
        xLoc += nt*SST_TILE_WIDTH;
    }

    if ( yLoc < 0 )
    {			       // adjust base so ylocn is positive
        nt = (-yLoc + SST_TILE_HEIGHT - 1) / SST_TILE_HEIGHT;
        tBase -= nt*stride;
        yLoc += nt*SST_TILE_HEIGHT;
    }

    xTile = xLoc >> SST_TILE_WIDTH_BITS;         // x locn in tiles
    xOffset = xLoc & SST_TILE_WIDTH_MASK;        // x byte offset within tile
    yTile = yLoc >> SST_TILE_HEIGHT_BITS;        // y locn in tiles 
    yOffset = yLoc & SST_TILE_HEIGHT_MASK;       // y byte offset within tile

    GDBG_INFO( 206, "tiledAddress: xTile=%d xOffset=%d yTile=%d yOffset=%d\n",
        xTile, xOffset, yTile, yOffset );

    addr = ((tBase + yTile * stride + xTile) << SST_BUFFER_BASE_T_SHIFT)
        + (yOffset << SST_BUFFER_BASE_Y_SHIFT) + xOffset;

    GDBG_INFO( 206, "tiledAddress:   addr(unmasked)=%d\n", addr );

    addr &= SST_BUFFER_BASE_ADDR;

    GDBG_INFO( 206, "tiledAddress:   addr(masked)=%d\n", addr );

    return addr;
}

//----------------------------------------------------------------------
// return the byte offset (from 0) of a pixel within a 2D/3D buffer
//----------------------------------------------------------------------
FX_EXPORT FxU32 FX_CSTYLE
csimPixelAddress(SstRegs *sst, FxI32 buffer, int x, int y)
{
    int base,stride;
    int tiled = 0;
    int bpp, align;
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstGRegs *sstg = &cp->gui;
    SstIORegs *sstio = &cp->io;
    SstCRegs *sstc = &cp->cmd;
    int pdepth;          // bytes per pixel
    FxU32 format;
    FxU32 addr;
    int is_yuv = 0;

    switch ( buffer )
    {
    case CSIM_BUF_2D_SRC:
    case CSIM_BUF_2D_STRETCH_SRC:
        base = (sstg->srcBaseAddr & SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT;
        switch ( sstg->srcFormat & SSTG_SRC_PACK )
        {
        case SSTG_SRC_PACK_SRC:	align = 0; break;
        case SSTG_SRC_PACK_8:	align = 1; break;
        case SSTG_SRC_PACK_16:	align = 2; break;
        case SSTG_SRC_PACK_32:	align = 4; break;
        default:
            GDBG_ERROR( "csimPixelAddress", "bad 2d source packing %d\n",
                (sstg->srcFormat & SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT );
            return 0;
        }
        switch ( sstg->srcFormat & SSTG_SRC_FORMAT )
        {
        case SSTG_PIXFMT_1BPP: 	bpp = 1; break;
        case SSTG_PIXFMT_8BPP: 	bpp = 8; break;
        case SSTG_PIXFMT_15BPP:
        case SSTG_PIXFMT_16BPP:	bpp = 16; break;
        case SSTG_PIXFMT_24BPP:	bpp = 24; break;
        case SSTG_PIXFMT_32BPP:	bpp = 32; break;
        case SSTG_PIXFMT_422YUV:
        case SSTG_PIXFMT_422UYV:
            bpp = 16;
            is_yuv = 1;
            break;

        default:
            GDBG_ERROR( "csimPixelAddress", "bad 2d source format %d\n",
                (sstg->srcFormat & SSTG_SRC_FORMAT)>>SSTG_SRC_FORMAT_SHIFT );
            return 0;
        }
        // determine stride
        if ( ((sstg->srcBaseAddr & SSTG_IS_TILED) != 0) && (align != 0 ) )
            GDBG_INFO( 0,"csimPixelAddress: tile bit overridden by packing (x=0x%x,y=0x%x,base=0x%x,pack=0d)\n",
                x,y,sstg->srcBaseAddr,(sstg->srcFormat & SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT );
        if ( align == 0 )
        {
            tiled = sstg->srcBaseAddr & SSTG_IS_TILED;
            if ( tiled )
                stride = (sstg->srcFormat & SSTG_SRC_TILE_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
            else
                stride = (sstg->srcFormat & SSTG_SRC_LINEAR_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
        }
        else
        {
            int xsize;
            if ( buffer == CSIM_BUF_2D_SRC )
                xsize = sstg->dstSize & 0xFFFF;
            else
            {
                xsize = sstg->srcSize & 0xFFFF;
                // for stretch blit only.  a zero source width is weird,
                // we actually treat is as a source width of one, for no
                // particularly good reason other than we decided we should
                // draw something when xsize == 0, and move on to other data
                // when source height != 0
                if ( xsize == 0 )
                    xsize = 1;
            }

            stride = CEIL( xsize*bpp,8*align )*align;
            if ( bpp == 16 || bpp == 32 )
            {
                if ( align < bpp/8 )
                    GDBG_ERROR( "csimPixelAddress","invalid packing (%d byte) for %d bpp format\n",align,bpp );
                if ( align == 2 || align == 4 )
                    if ( stride & (align-1) )
                        GDBG_ERROR( "csimPixelAddress","stride is not a multiple of %d bytes\n",align );
            }
        }
        // determine bytes/pixel
        if ( bpp == 1 )
        {
            x = x/8;           // convert x from bits to bytes
            pdepth = 1;
        }
        else
        {
            pdepth = bpp/8;
        }
        if ( is_yuv )
        {
            // always read the entire 32-bit word that contains the two
            // pixels, regardless of whether x is even or odd.
            // other functions will pick out the correct Y,U, and V
            x &= ~1;
        }

        break ;
    case CSIM_BUF_2D_DST:
        tiled = sstg->dstBaseAddr & SSTG_IS_TILED;
        base = (sstg->dstBaseAddr & SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT;
        if ( tiled )
            stride = (sstg->dstFormat & SSTG_DST_TILE_STRIDE) >> SSTG_DST_STRIDE_SHIFT;
        else
            stride = (sstg->dstFormat & SSTG_DST_LINEAR_STRIDE) >> SSTG_DST_STRIDE_SHIFT;
        switch ( sstg->dstFormat & SSTG_DST_FORMAT )
        {
        case SSTG_PIXFMT_8BPP: 	pdepth = 1; break;
        case SSTG_PIXFMT_15BPP:
        case SSTG_PIXFMT_16BPP:	pdepth = 2; break;
        case SSTG_PIXFMT_24BPP:	pdepth = 3; break;
        case SSTG_PIXFMT_32BPP:	pdepth = 4; break;
        default: 
            GDBG_ERROR( "csimPixelAddress", "bad 2d destination format %d\n",
                (sstg->dstFormat & SSTG_DST_FORMAT)>>SSTG_DST_FORMAT_SHIFT );
            return 0;
        }
        break ;
    case CSIM_BUF_3D_COLOR:
        // are we drawing into the front or back buffer?
        tiled = sst->colBufferStride & SST_BUFFER_MEMORY_TYPE;
        base = (sst->colBufferAddr & SST_BUFFER_BASE_ADDR)>>SST_BUFFER_BASE_SHIFT;
        if ( tiled )
            stride = (sst->colBufferStride & SST_BUFFER_TILE_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
        else
            stride = (sst->colBufferStride & SST_BUFFER_LINEAR_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;

        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP )
            pdepth = 4;
        else
            pdepth = 2;
        break;
    case CSIM_BUF_3D_FRONT:		// must be defined as 0
        GDBG_ERROR( "csimPixelAddress","illegal access to CSIM_BUF_3D_FRONT\n" );
        return 0;
        break;
    case CSIM_BUF_3D_BACK:		// must be defined as 1
        GDBG_ERROR( "csimPixelAddress","illegal access to CSIM_BUF_3D_BACK\n" );
        return 0;
        break;
    case CSIM_BUF_3D_TRIPLE:
        GDBG_ERROR( "csimPixelAddress","illegal access to CSIM_BUF_3D_TRIPLE\n" );
        return 0;
        break;
    case CSIM_BUF_3D_AUX1:
        tiled = sst->auxBufferStride & SST_BUFFER_MEMORY_TYPE;
        base = (sst->auxBufferAddr & SST_BUFFER_BASE_ADDR)>>SST_BUFFER_BASE_SHIFT;
        if ( tiled )
            stride = (sst->auxBufferStride & SST_BUFFER_TILE_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
        else
            stride = (sst->auxBufferStride & SST_BUFFER_LINEAR_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP ) 
            pdepth = 4;
        else if ( sst->fbzMode & SST_ENALPHABUFFER ) 
            pdepth = 1;
        else
            pdepth = 2;
        break;
    case CSIM_BUF_OVERLAY:
        tiled = sstio->vidProcCfg & SST_OVERLAY_TILED_EN;
        base = (sstio->vidCurrOverlayStartAddr & SST_VIDEO_START_ADDR)
            >> SST_VIDEO_START_ADDR_SHIFT;
        if ( tiled )
            stride = (sstio->vidDesktopOverlayStride & SST_OVERLAY_TILE_STRIDE) 
            >> SST_OVERLAY_STRIDE_SHIFT;
        else
            stride = (sstio->vidDesktopOverlayStride & SST_OVERLAY_LINEAR_STRIDE) 
            >> SST_OVERLAY_STRIDE_SHIFT;
        format = sstio->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT;
        switch ( format )
        {
        case SST_OVERLAY_PIXEL_YUV411:  
            x &= 0xfffffffe;
            pdepth = 2; break;
        case SST_OVERLAY_PIXEL_RGB565D: 
        case SST_OVERLAY_PIXEL_RGB565U: 
        case SST_OVERLAY_PIXEL_RGB1555D: 
        case SST_OVERLAY_PIXEL_RGB1555U: 
            pdepth = 2; break;
        case SST_OVERLAY_PIXEL_RGB32U: 
            pdepth = 4; break;
        case SST_OVERLAY_PIXEL_YUYV422: 
        case SST_OVERLAY_PIXEL_UYVY422: 
            pdepth = 2; 
            x &= 0xfffffffe;
            break;
        default: 
            GDBG_ERROR( "csimPixelAddress", "bad overlay pixel format %d\n",
                format>>SST_OVERLAY_PIXEL_FORMAT_SHIFT );
            return 0;
        }
        break ;
    case CSIM_BUF_DESKTOP:
        tiled = sstio->vidProcCfg & SST_DESKTOP_TILED_EN;
        base = (sstio->vidDesktopStartAddr & SST_VIDEO_START_ADDR)
            >> SST_VIDEO_START_ADDR_SHIFT;
        if ( tiled )
            stride = (sstio->vidDesktopOverlayStride & SST_DESKTOP_TILE_STRIDE) 
            >> SST_DESKTOP_STRIDE_SHIFT;
        else
            stride = (sstio->vidDesktopOverlayStride & SST_DESKTOP_LINEAR_STRIDE) 
            >> SST_DESKTOP_STRIDE_SHIFT;
        switch ( sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT )
        {
        case SST_DESKTOP_PIXEL_PAL8:   pdepth = 1; break;
        case SST_DESKTOP_PIXEL_RGB565: pdepth = 2; break;
        case SST_DESKTOP_PIXEL_RGB1555U: pdepth = 2; break;
        case SST_DESKTOP_PIXEL_RGB24:  pdepth = 3; break;
        case SST_DESKTOP_PIXEL_RGB32:  pdepth = 4; break;
        default:
            GDBG_ERROR( "csimPixelAddress","bad desktop pixel format %d\n",
                (sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT)>>SST_DESKTOP_PIXEL_FORMAT_SHIFT );
            return 0;
        }
        break ;
    case CSIM_BUF_CURSOR:
        tiled = 0;                    // cursor must be in linear memory
        base = sstio->hwCurPatAddr;
        stride = 16;                  // 16 bytes per row
        pdepth = 4;
        if ( x<0 || x>=64 || y<0 || y>=64 )
        {
            GDBG_ERROR( "csimPixelAddress","bad cursor address (x,y)=%d,%d(0x%x,0x%x)\n",x,y,x,y );
            return 0;
        }
        x /= 32;
        break;
    case CSIM_BUF_YUV:
        tiled = sstc->yuvStride & SST_YUV_MEMORY_TILED;
        base = (sstc->yuvBaseAddr & SST_YUV_BASE_ADDR) >> SST_YUV_BASE_ADDR_SHIFT;
        if ( tiled )
            stride = (sstc->yuvStride & SST_YUV_TILE_STRIDE) >> SST_YUV_STRIDE_SHIFT;
        else
            stride = (sstc->yuvStride & SST_YUV_LINEAR_STRIDE) >> SST_YUV_STRIDE_SHIFT;
        pdepth = 2;
        break;
    default:
        GDBG_ERROR( "csimPixelAddress","invalid CSIM buffer %d\n",buffer );
        return 0;
    }

    // we end up here if buffer is 3D
    //if (cp->numBuffers <= buffer) {	// sanity check buffer existence
    //	GDBG_ERROR("csimPixelAddress","buffer %d does not exist\n",buffer);
    //	return 0;
    //}

    if ( pdepth == 2 && (base & 1) )
        GDBG_ERROR( "csimPixelAddress","unaligned 16-bit access (buffer=%d, base=0x%x)\n",buffer,base );
    if ( pdepth == 4 && (base & 3) )
        GDBG_ERROR( "csimPixelAddress","unaligned 32-bit access (buffer=%d, base=0x%x)\n",buffer,base );

    if ( tiled )
    {
        addr = tiledAddress( base,stride,pdepth,x,y );
    }
    else
    {
        addr = base + y*stride + x*pdepth;  // byte stride
    }

    if ( pdepth == 2 && (addr & 1) )
    {
        GDBG_ERROR( "csimPixelAddress","unaligned 16-bit access (buffer=%d, addr=0x%x)\n",buffer,addr );
        GDBG_ERROR( "csimPixelAddress","base=0x%x, stride=0x%x, pdepth=%d, x=0x%x, y=0x%x, %s\n",
            base,stride,pdepth,x,y,tiled ? "Tiled" : "Linear");
    }
    if ( pdepth == 4 && (addr & 3) )
    {
        GDBG_ERROR( "csimPixelAddress","unaligned 32-bit access (buffer=%d, addr=0x%x)\n",buffer,addr );
        GDBG_ERROR( "csimPixelAddress","base=0x%x, stride=0x%x, pdepth=%d, x=0x%x, y=0x%x, %s\n",
            base,stride,pdepth,x,y,tiled ? "Tiled" : "Linear");
    }

    if ( addr >= (FxU32)cp->memorySizeInBytes )
        GDBG_INFO( 200,"csimPixelAddress: addr=0x%x, buffer=%d, base=0x%x, stride=0x%x, pdepth=%d, x=0x%x, y=0x%x, %s\n",
            addr,buffer,base,stride,pdepth,x,y,tiled ? "Tiled" : "Linear");

    return addr;
}

//----------------------------------------------------------------------
// read and write a pixel from a 2D/3D buffer (in native format)
//----------------------------------------------------------------------


FX_EXPORT FxU32 FX_CSTYLE
csimReadPixel(SstRegs *sst, FxI32 buffer, int x, int y)
{
    FxU32 data=0x0defaced;
    FxI32 sliY;
    CsimPrivate *cp;

    assert( x >= 0 );
    assert( y >= 0 );

    if(currentRenderingChip == NULL)
      currentRenderingChip = globalSST;

// This was added without any comment on why and it seems like messes up WinSim, so
// I am commenting this out until I get convinced that this was actually useful
#ifndef WINSIM
    if(csimChipOwnsPixel(currentRenderingChip, y, &sliY))
      sst = currentRenderingChip;
    else
#endif    
      sst=csimFindPixelOwner( y, &sliY );

    if ( sst==NULL )
        return( data );

    cp = CSIM_PRIVATE( sst );
    data=readPixel( sst, buffer, x, y, cp->memory, "csim" );

    return( data );
}

FX_EXPORT FxU32 FX_CSTYLE
readPixel(SstRegs *sst, FxI32 buffer, int x, int y, volatile FxU8 *mem, char *where)
{
    FxU32 addr, data, pat0, pat1, bit, _y, u, v;
    FxI32 sliY;
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstIORegs *sstio = &cp->io;
    SstGRegs *sstg = &cp->gui;
    int memSize = cp->memorySizeInBytes;
    int tiled;

    //For SLI, check to see if we own the pixel. Also do munging if pixel is owned
    if ( sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE )
    {
        if ( !csimChipOwnsPixel( sst, y, &sliY ) )
        {
            GDBG_INFO( 200,"%sReadPixel(%d,%d) not owned by %s\n",
                where,x,y,cp->environment.name );
            return( 0xDEADDEAD );
        }

        addr = csimPixelAddress( sst,buffer,x,sliY );
        GDBG_INFO( 200,"%sReadPixel(%d,%d)@0x%x (%s sliY=%d)\n",
            where,x,y,addr,cp->environment.name,sliY );
    }
    else
    {
        addr = csimPixelAddress( sst,buffer,x,y );
        GDBG_INFO( 200,"%sReadPixel(%d,%d)@0x%x\n",where,x,y,addr );
    }



    switch ( buffer )
    {

    case CSIM_BUF_2D_SRC:
    case CSIM_BUF_2D_STRETCH_SRC:
        tiled = sstg->srcBaseAddr & SSTG_IS_TILED;
        if ( tiled && (sstg->srcFormat & SSTG_SRC_PACK) != SSTG_SRC_PACK_SRC )
        {
            GDBG_INFO( 0,"%sReadPixel: tile bit overridden by packing (x=0x%x,y=0x%x,base=0x%x,pack=0d)\n",
                where,x,y,sstg->srcBaseAddr,(sstg->srcFormat & SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT );
            tiled = 0;
        }
        switch ( sstg->srcFormat & SSTG_SRC_FORMAT )
        {
        case SSTG_PIXFMT_1BPP: 	data = readMem8( mem,addr,memSize,where )>>(7-(x&7));	break;
        //  case SSTG_PIXFMT_4BPP: 	data = readMem8 (mem,addr,memSize,where)>>((x&1)*4);	break;
        case SSTG_PIXFMT_8BPP: 	data = readMem8( mem,addr,memSize,where );		break;
        case SSTG_PIXFMT_15BPP:
        case SSTG_PIXFMT_16BPP:	data = readMem16( mem,addr,memSize,where );		break;
        case SSTG_PIXFMT_24BPP:	data = readMem24( mem,tiled,addr,memSize,where );		break;
        case SSTG_PIXFMT_32BPP:	data = readMem32( mem,addr,memSize,where );		break;

        case SSTG_PIXFMT_422YUV:
            data = readMem32( mem,addr,memSize,where );
            _y = (data >> ((x & 1) ? 16 : 0)) & 0xFF;
            u = (data >> 8) & 0xFF;
            v = (data >> 24) & 0xFF;
            data = (_y << 16) | (u << 8) | v;
            break;

        case SSTG_PIXFMT_422UYV:
            data = readMem32( mem,addr,memSize,where );
            _y = (data >> ((x & 1) ? 24 : 8)) & 0xFF;
            u = (data >> 0) & 0xFF;
            v = (data >> 16) & 0xFF;
            data = (_y << 16) | (u << 8) | v;
            break;

        default:
            GDBG_ERROR( "ReadPixel","invalid 2d source format %d\n",
                (sstg->srcFormat & SSTG_SRC_FORMAT)>>SSTG_SRC_FORMAT_SHIFT );
            return 0;
        }
        break ;

    case CSIM_BUF_2D_DST:
        tiled = sstg->dstBaseAddr & SSTG_IS_TILED;
        switch ( sstg->dstFormat & SSTG_DST_FORMAT )
        {
        case SSTG_PIXFMT_8BPP: 	data = readMem8( mem,addr,memSize,where );		break;
        case SSTG_PIXFMT_15BPP:
        case SSTG_PIXFMT_16BPP:	data = readMem16( mem,addr,memSize,where );		break;
        case SSTG_PIXFMT_24BPP:	data = readMem24( mem,tiled,addr,memSize,where );		break;
        case SSTG_PIXFMT_32BPP:	data = readMem32( mem,addr,memSize,where );		break;
        default:
            GDBG_ERROR( "ReadPixel","invalid 2d destination format %d\n",
                (sstg->dstFormat & SSTG_DST_FORMAT)>>SSTG_DST_FORMAT_SHIFT );
            return 0;
        }
        break ;

    case CSIM_BUF_3D_COLOR:
        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP )
            data = readMem32( mem,addr,memSize,where );
        else
            data = readMem16( mem,addr,memSize,where );
        break;

    case CSIM_BUF_3D_AUX1:
        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP )
            data = readMem32( mem,addr,memSize,where );
        else if ( sst->fbzMode & SST_ENALPHABUFFER )
            data = readMem8( mem,addr,memSize,where );
        else
            data = readMem16( mem,addr,memSize,where );
        break;

    case CSIM_BUF_DESKTOP:
        tiled = sstio->vidProcCfg & SST_DESKTOP_TILED_EN;
        switch ( sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT )
        {
        case SST_DESKTOP_PIXEL_PAL8:	data = readMem8( mem,addr,memSize,where );	break;
        case SST_DESKTOP_PIXEL_RGB565:	data = readMem16( mem,addr,memSize,where );	break;
        case SST_DESKTOP_PIXEL_RGB1555U:	data = readMem16( mem,addr,memSize,where );	break;
        case SST_DESKTOP_PIXEL_RGB24:	data = readMem24( mem,tiled,addr,memSize,where );	break;
        case SST_DESKTOP_PIXEL_RGB32:	data = readMem32( mem,addr,memSize,where );	break;
        default:
            GDBG_ERROR( "ReadPixel","invalid desktop format %d\n",
                (sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT)
                >>SST_DESKTOP_PIXEL_FORMAT_SHIFT );
            return 0;
        }
        break ;

    case CSIM_BUF_OVERLAY:
        switch ( sstio->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT )
        {
        case SST_OVERLAY_PIXEL_YUV411:	data = readMem32( mem,addr,memSize,where );	break;
        case SST_OVERLAY_PIXEL_RGB1555D: 	
        case SST_OVERLAY_PIXEL_RGB1555U: 
        case SST_OVERLAY_PIXEL_RGB565D: 	
        case SST_OVERLAY_PIXEL_RGB565U: 	data = readMem16( mem,addr,memSize,where );	break;
        case SST_OVERLAY_PIXEL_RGB32U: 	data = readMem32( mem,addr,memSize,where );	break;
        case SST_OVERLAY_PIXEL_YUYV422: 
        case SST_OVERLAY_PIXEL_UYVY422: 	data = readMem32( mem,addr,memSize,where );	break;
        default:
            GDBG_ERROR( "ReadPixel","invalid overlay format %d\n",
                (sstio->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT)
                >>SST_OVERLAY_PIXEL_FORMAT_SHIFT );
            return 0;
        }
        break ;

    case CSIM_BUF_CURSOR:
        bit = x % 32;
        bit = (bit & 0xf8) | (7 - (bit & 7)); // change lsbs to match little byte endianness

        pat0 = (readMem32( mem,addr,memSize,where ) >> bit) & 0x1;
        pat1 = (readMem32( mem,addr+8,memSize,where ) >> bit) & 0x1;
        data = 2*pat1+pat0;
        break;

    default:
        GDBG_ERROR( "ReadPixel","invalid buffer %d\n",buffer );
        return 0;
    }

    GDBG_INFO( 200,"%sReadPixel(%d,%d)@0x%x => 0x%x\n",where,x,y,addr,data );
    return data;
}

FX_EXPORT void FX_CSTYLE
csimWritePixel(SstRegs *sst, FxI32 buffer, int x, int y, FxU32 col)
{
    int sliY;
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    CsimPrivate *cpParent = CSIM_PRIVATE( globalSST );
    FxU32 addr;
    SstGRegs *sstg; &cp->gui;

    writePixel( sst, buffer, x, y, col, cp->memory, "csim" );

    sstg = &cp->gui;

    //For SLI, check to see if we own the pixel.
    if ( !csimChipOwnsPixel( sst, y, &sliY ) )
        return;

    if ( sst->aaCtrl & SST_AA_CONTROL_AA_ENABLE )
    {
        //Always use the primary buffers to calculate the pixel address.
        //Otherwise, guiDrawPixel gets f'ed up.
        if ( !cp->environment.aaPrimaryBuffers )
        {
            sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
            sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );
            addr = csimPixelAddress( sst,buffer,x,y );
            sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
            sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
        }
        else
            addr = csimPixelAddress( sst,buffer,x,y );
    }
    else
        addr = csimPixelAddress( sst,buffer,x,y );

    switch ( buffer )
    {
    case CSIM_BUF_2D_DST:
        switch ( sstg->dstFormat & SSTG_DST_FORMAT )
        {
        case SSTG_PIXFMT_8BPP:
            // need to do CLUT palette translation here
            if ( halInfo.video && cpParent->windows ) 
                guiDrawPixel( cpParent,x,y,addr,col,col,col );
            break;
        case SSTG_PIXFMT_15BPP:
            // NOTE: for now do NOT do color expansion
            if ( halInfo.video && cpParent->windows ) 
                guiDrawPixel( cpParent,x,y,addr,
                    (col & 0x7C00) >> 7,
                    (col & 0x03E0) >> 2,
                    (col & 0x001F) << 3 );
            break;
        case SSTG_PIXFMT_16BPP:
            // NOTE: for now do NOT do color expansion
            if ( halInfo.video && cpParent->windows ) 
                guiDrawPixel( cpParent,x,y,addr,
                    (col & 0xF800) >> 8,
                    (col & 0x07E0) >> 3,
                    (col & 0x001F) << 3 );
            break;
        case SSTG_PIXFMT_24BPP:
        case SSTG_PIXFMT_32BPP:
            if ( halInfo.video && cpParent->windows ) 
                guiDrawPixel( cpParent,x,y,addr,col>>16,col>>8,col );
            break;
        }
        guiKeepAlive( 1 );
        break;

    case CSIM_BUF_3D_COLOR:
        {
            FxU32 color[4], finalColor;
            FxU32 r[4], g[4], b[4], a[4];

                color[0] = col;

            csimConvertFramebufferFormatTo8888( sst->renderMode, color[0], 
                &r[0], &g[0], &b[0], &a[0] );

            //If AA enabled, read the other sample(s) and blend
            if ( sst->aaCtrl && SST_AA_CONTROL_AA_ENABLE )
            {	
                if ( cp->environment.aaPrimaryBuffers )
                {	    
                    sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
                    sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
                }
                else
                {
                    sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
                    sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );
                }

                color[1] = readPixel( sst, buffer, x, y, cp->memory, "csim" );
                csimConvertFramebufferFormatTo8888( sst->renderMode, color[1], 
                    &r[1], &g[1], &b[1], &a[1] );

                if ( cp->environment.aaPrimaryBuffers )
                {	    
                    sst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( sst );
                    sst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( sst );
                }
                else
                {
                    sst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( sst );
                    sst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( sst );
                }

                //Blend the two samples together
                r[0] = (r[0] + r[1]) >> 1;
                g[0] = (g[0] + g[1]) >> 1;
                b[0] = (b[0] + b[1]) >> 1;

                //If appropriate get the other subsamples
                if ( cp->environment.aaSampleCount == 4 )
                {
                    FxBool foundOwner=FXFALSE;
                    SstRegs *otherSst;
                    CsimPrivate *otherCp;
                    FxU32 chipIndex;
                    FxU32 sliY;

                    //Find the owner of the other samples
                    for ( chipIndex=0; chipIndex<cp->environment.chipCount; chipIndex++ )
                    {
                        if ( chipIndex == 0 )
                            otherSst = globalSST;
                        else
                            otherSst = globalChildrenSST[chipIndex-1];

                        if ( (otherSst != sst) && csimChipOwnsPixel( otherSst, y, &sliY ) )
                        {
                            foundOwner = FXTRUE;
                            break;
                        }
                    }
                    assert( foundOwner );

                        otherCp = CSIM_PRIVATE( otherSst );
                    assert( otherCp->environment.aaSampleCount == 4 );

                    //Read the samples
                    color[2] = readPixel( otherSst, buffer, x, y, otherCp->memory, "csim" );
                    csimConvertFramebufferFormatTo8888( otherSst->renderMode, color[2],
                        &r[2], &g[2], &b[2], &a[2] );

                    if ( otherCp->environment.aaPrimaryBuffers )
                    {	    
                        otherSst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( otherSst );
                        otherSst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( otherSst );
                    }
                    else
                    {
                        otherSst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( otherSst );
                        otherSst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( otherSst );
                    }

                    color[3] = readPixel( otherSst, buffer, x, y, otherCp->memory, "csim" );
                    csimConvertFramebufferFormatTo8888( otherSst->renderMode, color[3], 
                        &r[3], &g[3], &b[3], &a[3] );

                    if ( otherCp->environment.aaPrimaryBuffers )
                    {	    
                        otherSst->colBufferAddr = COL_BUFFER_ADDR_PRIMARY( otherSst );
                        otherSst->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY( otherSst );
                    }
                    else
                    {
                        otherSst->colBufferAddr = COL_BUFFER_ADDR_SECONDARY( otherSst );
                        otherSst->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY( otherSst );
                    }	      

                    //Blend the two samples together
                    r[2] = (r[2] + r[3]) >> 1;
                    g[2] = (g[2] + g[3]) >> 1;
                    b[2] = (b[2] + b[3]) >> 1;

                    //Blend the blended samples together
                    r[0] = (r[0] + r[2]) >> 1;
                    g[0] = (g[0] + g[2]) >> 1;
                    b[0] = (b[0] + b[2]) >> 1;	     
                }
            }

#ifdef WINSIM
            // Write the averaged pixel into the composite buffer (V3 fb) ONLY
            // ---------------------------------------------------------------

            csimConvert8888ToFramebufferFormat( sst->renderMode, r[0], g[0], b[0], a[0], &finalColor );

            switch ( sst->renderMode & SST_RM_3D_MODE )
            {
            case SST_RM_32BPP:
                //            writeMem32Composite(cp->compositeMemory, addr, finalColor, cp->memorySizeInBytes, "csimComposite");

                if ( addr < (0x1000000 - 3) )
                    *(unsigned  *)(dwHostV3Base1 + addr) = finalColor;

                break;
            case SST_RM_15BPP:
            case SST_RM_16BPP:
                //            writeMem16(cp->compositeMemory, addr, finalColor, cp->memorySizeInBytes, "csimComposite");

                if ( addr < (0x1000000 - 1) )
                {
                    *(char *)(dwHostV3Base1 + addr + 0) = (char) (finalColor & 0xFF);
                    *(char *)(dwHostV3Base1 + addr + 1) = (char) ((finalColor >> 8) & 0xFF);
                }

                break ;
            }
#else      
            //Write the pixel into the composite buffer

            csimConvert8888ToFramebufferFormat( sst->renderMode, r[0], g[0], b[0], a[0], &finalColor );
            switch ( sst->renderMode & SST_RM_3D_MODE )
            {
            case SST_RM_32BPP:
                writeMem32( cp->compositeMemory, addr, finalColor, cp->memorySizeInBytes, "csimComposite" );		     
                break;
            case SST_RM_15BPP:
            case SST_RM_16BPP:
                writeMem16( cp->compositeMemory, addr, finalColor, cp->memorySizeInBytes, "csimComposite" );
                break;
            default:
                assert( 0 );
            }	

            if ( halInfo.video && cpParent->windows ) 
                guiDrawPixel( cpParent,x,y,addr, r[0], g[0], b[0] );       
#endif // WINSIM
        }
        break ;      
    }

    if ( halInfo.video && cpParent->windows ) 
        guiKeepAlive( 1 );   
}

FX_EXPORT void FX_CSTYLE
writePixel(SstRegs *sst, FxI32 buffer, int x, int y, FxU32 col, volatile FxU8 *mem, char *where)
{
    FxU32 addr, pat0, pat1, bit;
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstIORegs *sstio = &cp->io;
    SstGRegs *sstg = &cp->gui;
    int memSize = cp->memorySizeInBytes;
    int tiled;
    FxI32 sliY;

    //For SLI, check to see if we own the pixel. Also do munging if pixel is owned
    if ( sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE )
    {
        if ( !csimChipOwnsPixel( sst, y, &sliY ) )
        {
            GDBG_INFO( 200,"%sWritePixel(%d,%d) <= 0x%x not owned by %s\n",
		       where,x,y,col,cp->environment.name );
            return;
        }

        addr = csimPixelAddress( sst,buffer,x,sliY );
        GDBG_INFO( 200,"%sWritePixel(%d,%d)@0x%x <= 0x%x (%s sliY=%d)\n",
            where,x,y,addr,col,cp->environment.name,sliY );
    }
    else
    {
        addr = csimPixelAddress( sst,buffer,x,y );
        GDBG_INFO( 200,"%sWritePixel(%d,%d)@0x%x <= 0x%x\n",where,x,y,addr,col );
    }

    switch ( buffer )
    {
    case CSIM_BUF_2D_DST:
        tiled = (sstg->dstBaseAddr & SSTG_IS_TILED);
        switch ( sstg->dstFormat & SSTG_DST_FORMAT )
        {
        case SSTG_PIXFMT_8BPP: 	writeMem8( mem,addr,col,memSize,where ); 	break;
        case SSTG_PIXFMT_15BPP:	
        case SSTG_PIXFMT_16BPP:	writeMem16( mem,addr,col,memSize,where ); 	break;
        case SSTG_PIXFMT_24BPP:	writeMem24( mem,tiled,addr,col,memSize,where ); 	break;
        case SSTG_PIXFMT_32BPP:	writeMem32( mem,addr,col,memSize,where ); 	break;
        default:
            GDBG_ERROR( "WritePixel","invalid 2d destination format 0x%x\n",
                sstg->dstFormat );
            return;
        }
        break ;

    case CSIM_BUF_3D_COLOR:
        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP )
            writeMem32( mem,addr,col,memSize,where ); 
        else
            writeMem16( mem,addr,col,memSize,where ); 
        break;

    case CSIM_BUF_3D_AUX1:
        if ( (sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP )
            writeMem32( mem,addr,col,memSize,where );
        else if ( sst->fbzMode & SST_ENALPHABUFFER )
            writeMem8( mem,addr,col,memSize,where );
        else
            writeMem16( mem,addr,col,memSize,where );
        break;

    case CSIM_BUF_DESKTOP:
        tiled = sstio->vidProcCfg & SST_DESKTOP_TILED_EN;
        switch ( sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT )
        {
        case SST_DESKTOP_PIXEL_PAL8:       writeMem8( mem,addr,col,memSize,where );  	break;
        case SST_DESKTOP_PIXEL_RGB1555U:   writeMem16( mem,addr,col,memSize,where ); 	break;
        case SST_DESKTOP_PIXEL_RGB565:     writeMem16( mem,addr,col,memSize,where ); 	break;
        case SST_DESKTOP_PIXEL_RGB24:      writeMem24( mem,tiled,addr,col,memSize,where ); 	break;
        case SST_DESKTOP_PIXEL_RGB32:      writeMem32( mem,addr,col,memSize,where ); 	break;
        default:
            GDBG_ERROR( "WritePixel","invalid desktop format %d\n",
                (sstio->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT)
                >>SST_DESKTOP_PIXEL_FORMAT_SHIFT );
            return;
        }
        break ;

    case CSIM_BUF_OVERLAY:
        switch ( sstio->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT )
        {
        case SST_OVERLAY_PIXEL_YUV411:      writeMem32( mem,addr,col,memSize,where ); 	break;
        case SST_OVERLAY_PIXEL_RGB1555D: 
        case SST_OVERLAY_PIXEL_RGB1555U: 
        case SST_OVERLAY_PIXEL_RGB565D: 
        case SST_OVERLAY_PIXEL_RGB565U: writeMem16( mem,addr,col,memSize,where ); 	break;
        case SST_OVERLAY_PIXEL_RGB32U:     writeMem32( mem,addr,col,memSize,where ); 	break;
        case SST_OVERLAY_PIXEL_YUYV422: 
        case SST_OVERLAY_PIXEL_UYVY422:     writeMem32( mem,addr,col,memSize,where ); 	break;
        default:
            GDBG_ERROR( "WritePixel","invalid overlay format %d\n",
                (sstio->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT)
                >>SST_OVERLAY_PIXEL_FORMAT_SHIFT );
            return;
        }
        break ;

    case CSIM_BUF_CURSOR:
        bit = x % 32;
        bit = (bit & 0xf8) | (7 - (bit & 7)); // change lsbs to match little byte endianness
        pat0 = readMem32( mem,addr,memSize,where );
        pat0 = col & BIT( 0 ) ? (pat0 | BIT( bit )) : (pat0 & ~BIT( bit ));
        writeMem32( mem,addr,pat0,memSize,where );
        addr += 8;
        pat1 = readMem32( mem,addr,memSize,where );
        pat1 = col & BIT( 1 ) ? (pat1 | BIT( bit )) : (pat1 & ~BIT( bit ));
        writeMem32( mem,addr,pat1,memSize,where );
        break;

    default:
        GDBG_ERROR( "WritePixel","invalid buffer %d\n",buffer );
        return;
    }
}

//----------------------------------------------------------------------
// disable/enable the video display, called when changing lots of pixels
// like when init the screen for diags
FX_EXPORT void FX_CSTYLE
csimVideo(CsimPrivate *cp, FxBool enable)
{
    static int vsave = -1;

    if ( enable )
    {
        halInfo.video = vsave;
        vsave = -1;
        if ( halInfo.csim && halInfo.video )
            guiRefresh( cp );		// redisplay all the windows
#ifdef HAL_HSIM
        if ( halInfo.hsim )
            FBI_UPDATE_DISPLAY( );	// update HW screen display;
#endif
    }
    else
    {
        if ( vsave < 0 )
            vsave = halInfo.video;
        halInfo.video = FXFALSE;
    }
}

FX_EXPORT FxBool FX_CSTYLE csimChipOwnsPixel(SstRegs *sst, int y, int *yPrime)
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    FxU32 renderMask, compareMask, scanMask, log2Chips;  

    if ( !(sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) )
    {
        *yPrime = y;
        return( FXTRUE );
    }

    renderMask = (sst->sliCtrl & SST_SLI_CONTROL_RENDER_MASK) >> SST_SLI_CONTROL_RENDER_MASK_SHIFT;
    compareMask = (sst->sliCtrl & SST_SLI_CONTROL_COMPARE_MASK) >> SST_SLI_CONTROL_COMPARE_MASK_SHIFT;
    scanMask = (sst->sliCtrl & SST_SLI_CONTROL_SCAN_MASK) >> SST_SLI_CONTROL_SCAN_MASK_SHIFT;
    log2Chips = (sst->sliCtrl & SST_SLI_CONTROL_LOG2_CHIP_COUNT) >> SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT;

    *yPrime = ((y>>log2Chips) & ~scanMask) + (y & scanMask);

    if ( (y & renderMask) == compareMask )
        return( FXTRUE );
    else
        return( FXFALSE );  
}

FxBool csimChipOwnsRawLfbPixel(SstRegs *sst, int y, int *yPrime)
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );
    SstPCIConfigRegs *pciConfigRegs = &cp->pciConfigRegs;
    FxU32 renderMask, compareMask, scanMask, log2Chips;  

    if ( !(sst->sliCtrl & SST_SLI_CONTROL_SLI_ENABLE) )
    {
        *yPrime = y;
        return( FXTRUE );
    }

    //Make sure that the read/write enable stuff is always enabled
    if ( !(pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_CPU_WRITE_ENABLE) ||
        !(pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_DISPATCH_WRITE_ENABLE) ||
        !(pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_READ_ENABLE) )
    {
        GDBG_ERROR( "csimChipOwnsRawLfbPixel", "csimChipOwnsRawLfbPixel: You should always enable raw lfb read/writes in cfgSliLfbCtrl!\n" );
    }

    renderMask =  (pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_RENDERMASK)    >> SST_SLI_LFB_RENDERMASK_SHIFT;
    compareMask = (pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_COMPAREMASK)   >> SST_SLI_LFB_COMPAREMASK_SHIFT;
    scanMask =    (pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_SCANMASK)      >> SST_SLI_LFB_SCANMASK_SHIFT;
    log2Chips =   (pciConfigRegs->cfgSliLfbCtrl & SST_SLI_LFB_NUMCHIPS_LOG2) >> SST_SLI_LFB_NUMCHIPS_LOG2_SHIFT;

    *yPrime = ((y>>log2Chips) & ~scanMask) + (y & scanMask);

    if ( (y & renderMask) == compareMask )
        return( FXTRUE );
    else
        return( FXFALSE );  
}

FX_EXPORT SstRegs* FX_CSTYLE csimFindPixelOwner(FxI32 y, FxI32 *sliY)
{
    SstRegs *sst = globalSST;
    FxBool foundOwner = FXFALSE;

    //If SLI is enabled go through all chips to find owner
    if ( csimChipOwnsPixel( globalSST, y, sliY ) )
    {
        sst=globalSST;
        foundOwner=FXTRUE;
    }
    else
    {
        FxU32 i;

        for ( i=1; i<CSIM_PRIVATE( globalSST )->environment.chipCount; i++ )
        {
            if ( csimChipOwnsPixel( globalChildrenSST[i-1], y, sliY ) )
            {
                sst=globalChildrenSST[i-1];
                foundOwner=FXTRUE;
                break;
            }
        }
    }
    assert( foundOwner );

    return( sst );
}

FX_EXPORT SstRegs* FX_CSTYLE csimFindSecondaryPixelOwner(FxI32 y, FxI32 *sliY)
{
    SstRegs *sst;
    FxI32 ownerCount=0;
    FxU32 i;

    assert( CSIM_PRIVATE( globalSST )->environment.chipCount > 1 );
    assert( globalSST->aaCtrl & SST_AA_CONTROL_AA_ENABLE );

    if ( csimChipOwnsPixel( globalSST, y, sliY ) )
    {
        sst=globalSST;
        ownerCount++;
    }

    for ( i=1; i<CSIM_PRIVATE( globalSST )->environment.chipCount; i++ )
    {
        if ( csimChipOwnsPixel( globalChildrenSST[i-1], y, sliY ) )
        {
            sst=globalChildrenSST[i-1];
            ownerCount++;
        }
    }
    assert( ownerCount == 2 );

    return( sst );
}

FX_EXPORT FxBool FX_CSTYLE csimConvert8888ToFramebufferFormat(FxU32 renderMode, FxU32 r, FxU32 g, FxU32 b, FxU32 a, FxU32 *result)
{
    FxU32 red, green, blue, alpha;

    red = r & 0xFF;
    green = g & 0xFF;
    blue = b & 0xFF;
    alpha = a & 0xFF;

    switch ( renderMode & SST_RM_3D_MODE )
    {
    case SST_RM_32BPP:
        *result = (red << 16) | (green << 8) | (blue) | (alpha << 24);
        break;
    case SST_RM_16BPP:
        *result = ((red >> 3) << 11) | ((green >> 2) << 5) | ((blue >> 3));
        break;
    case SST_RM_15BPP:
        *result = ((red >> 3) << 10) | ((green >> 3) << 5) | ((blue >> 3)) | ((alpha >> 7) << 15);
        break;
    default:
        GDBG_ERROR( "csimConvert8888ToFramebufferFormat", "Illegal format!\n" );
    }	  

    return( FXTRUE );
}

FX_EXPORT FxBool FX_CSTYLE csimConvert8888ToFramebufferFormat_2(FxU32 renderMode, FxU32 ARGB, FxU32 *frameBufferColor)
{
    FxU32 r, g, b, a;

    a=(ARGB>>24) & 0xFF;
    r=(ARGB>>16) & 0xFF;
    g=(ARGB>> 8) & 0xFF;
    b=(ARGB>> 0) & 0xFF;

    return( csimConvert8888ToFramebufferFormat( renderMode, r, g, b, a, frameBufferColor ) );
}

FX_EXPORT FxBool FX_CSTYLE csimConvertFramebufferFormatTo8888(FxU32 renderMode, FxU32 color, FxU32 *r, FxU32 *g, FxU32 *b, FxU32 *a)
{
    switch ( renderMode & SST_RM_3D_MODE )
    {
    case SST_RM_32BPP:
        *r = ((color >> 16) & 0xFF);
        *g = ((color >> 8) & 0xFF);
        *b = ((color >> 0) & 0xFF);
        *a = ((color >> 24) & 0xFF);
        break;
    case SST_RM_16BPP:
        *r = (((color >> 11) & 0x1F) << 3);
        *g = (((color >> 5) & 0x3F) << 2);
        *b = (((color >> 0) & 0x1F) << 3);
        *a = 0x0;
        break;
    case SST_RM_15BPP:
        *r = (((color >> 10) & 0x1F) << 3);
        *g = (((color >> 5) & 0x1F) << 3);
        *b = (((color >> 0) & 0x1F) << 3);
        *a = ((color & 0x8000) ? 0xFF : 0x00);
        break;
    default:
        GDBG_ERROR( "csimConvert8888ToFramebufferFormat", "Illegal format!\n" );
    }	  

    return( FXTRUE );
}

FX_EXPORT FxBool FX_CSTYLE csimConvertFramebufferFormatTo8888_2(FxU32 renderMode, FxU32 frameBufferColor, FxU32 *ARGB)
{
    FxU32 r, g, b, a;
    FxBool returnValue;

    returnValue = csimConvertFramebufferFormatTo8888( renderMode, frameBufferColor, &r, &g, &b, &a );

        *ARGB = ((a & 0xFF) << 24) | ((r & 0xFF)<<16) | ((g & 0xFF)<<8) | ((b & 0xFF)<<0);

    return( returnValue );
}


void csimCommandFifoAccessErrorMessage(CsimPrivate *cp, FxU32 address, char *functionName, 
				       char *filename, int lineNumber)
{
    GDBG_ERROR( functionName, "Illegal access to command fifo! %s(%d)\n", filename, lineNumber );  
    GDBG_INFO_MORE( 0, "  address=0x%x   tmu0 texBaseAddr=0x%x  tmu1 texBaseAddr=0x%x\n",
        address, cp->trex[0].texBaseAddr, cp->trex[1].texBaseAddr );
}

FxBool csimIsCommandFifoAddress(CsimPrivate *cp, FxU32 framebufferAddress)
{
    SstCRegs *sstc = &cp->cmd;
    CmdFifo *cmdFifo;
    int fifoIndex;
    FxU32 minAddress, maxAddress;

    for ( fifoIndex=0; fifoIndex<2; fifoIndex++ )
    {
        if ( fifoIndex==0 )
            cmdFifo = &sstc->cmdFifo0;
        else
            cmdFifo = &sstc->cmdFifo1;

        //Check to see if this is an enabled command fifo in the framebuffer
        if ( (cmdFifo->baseSize & SST_CMDFIFOEN) &&
            ((cmdFifo->baseSize & SST_CMDFIFOAGP) == 0) )
        {
            minAddress = (cmdFifo->baseAddrL & SST_BASEADDRL) * 4096;
            maxAddress = minAddress + (((cmdFifo->baseSize & SST_BASESIZE) + 1) * 4096);

            if ( framebufferAddress >= minAddress && framebufferAddress < maxAddress )
                return( FXTRUE );
        }
    }

    return( FXFALSE );
}

