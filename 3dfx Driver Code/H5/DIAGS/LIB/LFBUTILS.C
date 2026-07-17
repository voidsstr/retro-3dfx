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
** Last Edited: Thu May 15 18:08:43 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$
** $Date: 10/11/00 8:11:55 PM$
**
*/

#include "udiag.h"
#include "sstdiag.h"
#include "fbi.h"
#include "lfbutils.h"
#include "hsimio.h"

/* signExtend
**
*/
FxI32 signExtend( FxI32 value, FxU32 sigBits )
{
    if ( sigBits > 31 )
    {
	GDBG_ERROR("signExtend", "sigBits out of range, result is meaningless.\n" );
    }
    value <<= (32 - sigBits);
    value >>= (32 - sigBits);
    return value;
}

/* currentDrawBuffer
**
*/
int currentDrawBuffer( SstRegs *shadowRegs )
{
#ifdef CVG
    // Determine Current Draw Buffer
    if ( shadowRegs->lfbMode & SST_LFB_ENPIXPIPE  ) // Write Through PixPipe???
    {   // Go by fbzMode bits
        switch( shadowRegs->fbzMode & SST_DRAWBUFFER )
        {
          case SST_DRAWBUFFER_FRONT:
            return LFB_FRONT_BUFFER;
            break;
          case SST_DRAWBUFFER_BACK:
            return LFB_BACK_BUFFER;
            break;
          default:
            GDBG_ERROR( "testPixelInFB", "detected an invalid write buffer.\n" );
            break;
        }
    }
    else 
    {   // Go by lfbMode bits
        switch( shadowRegs->lfbMode & SST_LFB_WRITEBUFSELECT )
        {
          case SST_LFB_WRITEFRONTBUFFER:
            return LFB_FRONT_BUFFER;
            break;
          case SST_LFB_WRITEBACKBUFFER:
            return LFB_BACK_BUFFER;
            break;
          default:
            GDBG_ERROR( "testPixelInFB", "detected an invalid write buffer.\n" );
            break;
        }
    }
    return LFB_FRONT_BUFFER;
#else // H3
    return diago.curdrawbuffer; 
#endif
}

/* flipYOrigin
**
*/
FxBool flipYOrigin( SstRegs *shadowRegs )
{
    if ( ( ( shadowRegs->lfbMode & SST_LFB_YORIGIN ) && !( shadowRegs->lfbMode & SST_LFB_ENPIXPIPE ) ) ||
	 ( ( shadowRegs->fbzMode & SST_YORIGIN ) && ( shadowRegs->lfbMode & SST_LFB_ENPIXPIPE ) ) )
    {
	return FXTRUE;
    }
    return FXFALSE;
}

/* testPixelInFB
**
** Summary: Test a pixel in the frame buffer given an fbiColors struct
**          Looks at shadowRegs to figure out which is the current
**          draw buffer, front/back.
**
** Arguments: x, y - coordinates of pixel
**            pixel - a pointer to an fbiColors structure
**            shadowRegs - pointer to current shadow registers
**
*/
int testPixelInFB( int x, int y, SstRegs *shadowRegs,  fbiColors *pixel )
{
    FxU32 testColor;
    testColor = (pixel->alpha<<24) | (pixel->red<<16) | (pixel->green<<8) |
		(pixel->blue);
    if (diago.rgb == 16) testColor &= 0x00F8FCF8;
    if (diago.rgb == 15) testColor &= 0x80F8F8F8;
    return DIAG_TEST_PIXEL(currentDrawBuffer(shadowRegs), x,y,testColor);
}

/* lfbWrite
**
** Summary: Write a pixel to the hardware in the correct way based on the
**          number of pixels you are to write.
**
** Argumenets: x, y - screen coordinates from the current y-origin
**             value - 32 bit value to write( if 16-bit write, 16 LSBs are written )
**             pixelsToWrite - number of pixels to write, 1 or 2, corrected to show
**                             number of pixels actually written
**             shadowRegs - pointer to shadow register structure
**             sst - a pointer to the SstRegs structure that the hardware setup code
**                   provides
*/
void lfbWrite( FxU32 x, FxU32 y, FxU32 value,
               FxU32 pixelsToWrite, SstRegs *shadowRegs,
               SstRegs *sst )
{
    // Create LFB Base Address ( both short and long formats )
    FxU32 *lfbBaseL = (FxU32 *) SST_LFB_ADDRESS( sst );
    FxU16 *lfbBaseS = (FxU16 *) SST_LFB_ADDRESS( sst );
    
    // Set lfbBase* to the correct scan line

    gdbg_info( 20, "lfbWrite x: %d y: %d value: 0x%.8x\n", x, y, value );
    
    switch( shadowRegs->lfbMode & SST_LFB_FORMAT )
    {
      case SST_LFB_565:
      case SST_LFB_555:
      case SST_LFB_1555:
      case SST_LFB_ZZ:   
        if ( pixelsToWrite == 2 )
        {
            if ( x % 2 )
            {
                GDBG_ERROR( "lfbWrite", "Misaligned 2 pixel write - ( x=%d, y=%d )\n", x, y );
            }
            SET( lfbBaseS[lfbOffset(x,y)], value );
        }
        else
        {
            FxU32 address = (FxU32)&(lfbBaseS[lfbOffset(x,y)]);
            
            // Fix Address Based on Word/Byte Swap
            if ( shadowRegs->lfbMode & SST_LFB_WRITE_BYTESWAP )
	      address ^= 2;
            if ( shadowRegs->lfbMode & SST_LFB_WRITE_SWAP16 )
	      address ^= 2;
            
            // Blech!!!
            SET16( *(FxU16*)address, (unsigned short)value );
        }
        break;
      case SST_LFB_888:
      case SST_LFB_8888:
      case SST_LFB_Z565:
      case SST_LFB_Z555:
      case SST_LFB_Z1555:
      case SST_LFB_Z32:
        SET( lfbBaseL[lfbOffset(x,y)], value );
        break;
      default:
        GDBG_ERROR( "lfbWrite", "Illegal Write Format %d\n",
                   (shadowRegs->lfbMode & SST_LFB_FORMAT) >>
                   SST_LFB_FORMAT_SHIFT ); 
        break;
    }
    
    return;
}

/* packBits
**
** Summary: Pack a 32-bit value with a series of bit fields specified in the format
**          Value, Width, from MSB to LSB
**
** Arguments: This function takes a variable number of arguments
**            n - number of fields to create in the 32 bit pattern
**            val<x>  - bit pattern to fill field <x> where x [0,n)
**            width<x> - width of field <x>
**
** Return Value: The packed 32 bit pattern
*/
FxU32 packBits( FxU32 n, ... /*val1, width1, ..., val<n>, width<n>*/ )
{
    va_list argumentPointer;
    FxU32   bitField = 0;
    FxU32   bitIndex = 0;
    
    va_start( argumentPointer, n );
    
    for ( ; n; n-- )
    {
        FxU32   bitMask;
        FxU32   fieldValue;
        FxU32   fieldWidth;
        
        fieldValue = va_arg( argumentPointer, FxU32 );
        fieldWidth = va_arg( argumentPointer, FxU32 );
        
        if ( bitIndex + fieldWidth > 32 )
        {
            gdbg_printf( "Error: packBits() field width overflow.\n" );
            DIAG_INCERROR();
        }
    
	if (fieldWidth == 32)
	    bitMask = 0xFFFFFFFF;
	else {    
            bitMask = ~0u;
            bitMask <<= fieldWidth;
            bitMask = ~bitMask;
	}
        
        fieldValue &= bitMask;
        fieldValue <<= ( 32 - bitIndex - fieldWidth );
        bitField |= fieldValue;
        
        bitIndex += fieldWidth;
    }
    
    va_end( argumentPointer );
    return bitField;
}


/* mangleColor
** 
** Summary: Re-Order the Color Bits into a single 32-bit word according
**          to the lfbWriteMode
**
** Parameters: writeValue - pointer to a 32-bit storage where the mangled color will
**                          be stored
**             shadowRegs - register set expressing the current state of the fbi
**             pixelsToWrite - number of pixels to be written
**             a1,r1,g1,b1,depth1 - data values for first pixel
**             a2,r2,g2,b2,depth2 - data values for first pixel
**             
*/
void mangleColor( FxU32 *writeValue, SstRegs *shadowRegs, FxU32 pixelsToWrite,
                  FxU32 a1, FxU32 r1, FxU32 g1, FxU32 b1, FxU32 depth1,
                  FxU32 a2, FxU32 r2, FxU32 g2, FxU32 b2, FxU32 depth2 )
{
    FxU32 z1Width, z2Width;
    FxU32 a1Width, a2Width;
    FxU32 r1Width, r2Width;
    FxU32 g1Width, g2Width;
    FxU32 b1Width, b2Width;
    FxBool swapWords;
    FxU32 wordsPerPixel;
    FxU32 words;
    
    // Get WordSwap Info
    swapWords = ( shadowRegs->lfbMode & SST_LFB_WRITE_SWAP16 ) ? FXTRUE : FXFALSE;
    
    // Do color lane swap and setup Colors
    switch ( shadowRegs->lfbMode & SST_LFB_FORMAT )
    {
      case SST_LFB_565:
        z1Width = z2Width = 0;
        depth1 = depth2 = 0;
        a1Width = a2Width = 0;
        a1 = a2 = 0;
        r1Width = r2Width = 5;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = g2Width = 6;
        g1 >>= 2;
        g2 >>= 2;
        b1Width = b2Width = 5;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 1;
        break;
      case SST_LFB_555:
        z1Width = z2Width = 0;
        depth1 = depth2 = 0;
        a1Width = a2Width = 1;
        a1 = a2 = 0;
        r1Width = r2Width = 5;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = 5;
        g2Width = 5;
        g1 >>= 3;
        g2 >>= 3;
        b1Width = 5;
        b2Width = 5;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 1;
        break;
      case SST_LFB_1555:
        z1Width = z2Width = 0;
        depth1 = depth2 = 0;
        a1Width = a2Width = 1;
        a1 = a1 ? 1 : 0;
        a2 = a2 ? 1 : 0;
        r1Width = r2Width = 5;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = g2Width = 5;
        g1 >>= 3;
        g2 >>= 3;
        b1Width = b2Width = 5;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 1;
        break;
      case SST_LFB_888: 
        z1Width = z2Width = 0;
        depth1 = depth2 = 0;
        a1Width = 8;
        a2Width = 0;
        a1 = a2 = 0;
        r1Width = 8;
        r2Width = 0;
        g1Width = 8;
        g2Width = 0;
        b1Width = 8;
        b2Width = 0;
        swapWords = FXFALSE;
        wordsPerPixel = 2;
        break;
      case SST_LFB_8888:        
        z1Width = z2Width = 0;
        depth1 = depth2 = 0;
        a1Width = 8;
        a2Width = 0;
        r1Width = 8;
        r2Width = 0;
        g1Width = 8;
        g2Width = 0;
        b1Width = 8;
        b2Width = 0;
        swapWords = FXFALSE;
        wordsPerPixel = 2;
        break;
      case SST_LFB_Z565:        
        z1Width = 16;
        z2Width = 0;
        a1Width = a2Width = 0;
        a1 = a2 = 0;
        r1Width = 5;
        r2Width = 0;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = 6;
        g2Width = 0;
        g1 >>= 2;
        g2 >>= 2;
        b1Width = 5;
        b2Width = 0;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 2;
        break;
      case SST_LFB_Z555:        
        z1Width = 16;
        z2Width = 0;
        a1Width = 1;
        a2Width = 0;
        a1 = a2 = 0;
        r1Width = 5;
        r2Width = 0;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = 5;
        g2Width = 0;
        g1 >>= 3;
        g2 >>= 3;
        b1Width = 5;
        b2Width = 0;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 2;
        break;
      case SST_LFB_Z1555:       
        z1Width = 16;
        z2Width = 0;
        a1Width = 1;
        a2Width = 0;
        a1 = a1 ? 1 : 0;
        a2 = a2 ? 1 : 0;
        r1Width = 5;
        r2Width = 0;
        r1 >>= 3;
        r2 >>= 3;
        g1Width = 5;
        g2Width = 0;
        g1 >>= 3;
        g2 >>= 3;
        b1Width = 5;
        b2Width = 0;
        b1 >>= 3;
        b2 >>= 3;
        wordsPerPixel = 2;
        break; 
      case SST_LFB_ZZ:  
        z1Width = z2Width = 16;
        a1Width = a2Width = 0;
        a1 = a2 = 0;
        r1Width = r2Width = 0;
        r1 = r2 = 0;
        g1Width = g2Width = 0;
        g1 = g2 = 0;
        b1Width = b2Width = 0;
        b1 = b2 = 0;
        wordsPerPixel = 1;
        break;
      case SST_LFB_Z32:       
        z1Width = 32;
        z2Width = 0;
        a1Width = a2Width = 0;
        a1 = a2 = 0;
        r1Width = r2Width = 0;
        r1 = r2 = 0;
        g1Width = g2Width = 0;
        g1 = g2 = 0;
        b1Width = b2Width = 0;
        b1 = b2 = 0;
        swapWords = FXFALSE;
        wordsPerPixel = 2;
        break;
      case SST_LFB_u1:          
      case SST_LFB_u2:  
      case SST_LFB_u3:  
      default:
        gdbg_printf( "Error: mangleColor() unrecognized/reserved lfb write format\n" );
        DIAG_INCERROR();
        break;
    }
    
    // This will be removed when I feel better about this code
    if ( z1Width + z2Width +
         a1Width + a2Width +
         r1Width + r2Width +
         g1Width + g2Width +
         b1Width + b2Width != 32 )
    {
        gdbg_printf( "Error: mangleColor() has an internal consistency error\n" );
        DIAG_INCERROR();
    }
    
    words = pixelsToWrite * wordsPerPixel;
    
    switch( shadowRegs->lfbMode & SST_LFB_RGBALANES )
    {
      case SST_LFB_RGBALANES_ARGB:
        *writeValue = packBits(10,
                               depth2, z2Width, a2, a2Width,
                               r2, r2Width, g2, g2Width,
                               b2, b2Width, depth1, z1Width,
                               a1, a1Width, r1, r1Width,
                               g1, g1Width, b1, b1Width );
        break;
      case SST_LFB_RGBALANES_ABGR:
        *writeValue = packBits(10,
                               depth2, z2Width, a2, a2Width,
                               b2, b2Width, g2, g2Width,
                               r2, r2Width, depth1, z1Width,
                               a1, a1Width, b1, b1Width,
                               g1, g1Width, r1, r1Width );
        break;
      case SST_LFB_RGBALANES_RGBA:
        *writeValue = packBits(10,
                               depth2, z2Width, r2, r2Width,
                               g2, g2Width, b2, b2Width,
                               a2, a2Width, depth1, z1Width,
                               r1, r1Width, g1, g1Width,
                               b1, b1Width, a1, a1Width );
        break;
      case SST_LFB_RGBALANES_BGRA:
        *writeValue = packBits(10,
                               depth2, z2Width, b2, b2Width,
                               g2, g2Width, r2, r2Width,
                               a2, a2Width, depth1, z1Width,
                               b1, b1Width, g1, g1Width,
                               r1, r1Width, a1, a1Width );
        break;
    }
    
    if ( swapWords )
    {
        if ( words == 2 )
          *writeValue = ( *writeValue << 16 ) | (*writeValue >> 16 );
    }
    
    if ( shadowRegs->lfbMode & SST_LFB_WRITE_BYTESWAP )
    {
        *writeValue = packBits( 4,
                                (*writeValue),
                                8,
                                (*writeValue >> 8 ),
                                8,
                                (*writeValue >> 16 ),
                                8,
                                (*writeValue >> 24 ),
                                8 );
        
        if ( words == 1 ) *writeValue >>= 16;
    }
    
    return;
}



/* rndlfbMode
**
** Summary - generates an acceptable random lfbMode and fixes number
**           of pixels to write
**
** Return Value - a set of register bits that is OK for lfb reads and writes
*/
FxU32 rndlfbMode( enum TestOpt mode, FxU32 *pixelsToWrite )
{
    FxU32  modeBits;
    FxBool passFlag = FXTRUE;
    
    do
    {
        passFlag = FXTRUE;
        
        // Generate Random Mode
        modeBits = iRandom( ~0u );
	if (CSIM_PRIVATE(diago.sstCSIM)->environment.chipCount > 1)
	  modeBits &= ~SST_LFB_YORIGIN;

        // Mask off Write Mode
        modeBits &= ~SST_LFB_FORMAT;
        
        // Set Write Mode
        switch( mode )
        {
          case TEST565:
            modeBits |= SST_LFB_565;
            break;
          case TESTX555:
            modeBits |= SST_LFB_555;
            break;
          case TEST1555:
            modeBits |= SST_LFB_1555;
            break;
          case TESTX888:
            modeBits |= SST_LFB_888;
            *pixelsToWrite = 1;
            break;
          case TEST8888:
            modeBits |= SST_LFB_8888;
            *pixelsToWrite = 1;
            break;
          case TEST16N565:
            modeBits |= SST_LFB_Z565;
            *pixelsToWrite = 1;
            break;
          case TEST16NX555:
            modeBits |= SST_LFB_Z555;
            *pixelsToWrite = 1;
            break;
          case TEST16N1555:
            modeBits |= SST_LFB_Z1555;
            *pixelsToWrite = 1;
            break;
          case TEST16N16:
            modeBits |= SST_LFB_ZZ;
            break;
          case TESTZ32:
            modeBits |= SST_LFB_Z32;
            *pixelsToWrite = 1;
            break;
          default:
            gdbg_printf( "Error: rndLfbMode Unknown TestOpt\n" );
            DIAG_INCERROR();
            break;
        }
        
        // Check for Reserved Frame Buffer Select Bits
#ifdef CVG
        switch( modeBits & SST_LFB_WRITEBUFSELECT )
        {
          case 2 << SST_LFB_WRITEBUFSELECT_SHIFT:
          case 3 << SST_LFB_WRITEBUFSELECT_SHIFT:
            passFlag = FXFALSE;
            continue;
            break;
          default:
            break;
        }
#endif
        
        // Check for Read Reserved Read Buffer Select Bits
        switch( modeBits & SST_LFB_READBUFSELECT )
        {
#ifdef CVG
	  case 3 << SST_LFB_READBUFSELECT_SHIFT:
#else // H3
	  case 0 << SST_LFB_READBUFSELECT_SHIFT:
	  case 3 << SST_LFB_READBUFSELECT_SHIFT:
#endif
            passFlag = FXFALSE;
            continue;
            break;
          default:
            break;
        }
        
    } while( !passFlag );
    
    return modeBits;
}

/* rndfbzMode
**
** Summary - generates an acceptable random fbzMode
**
** Return Value - a set of register bits that is OK for lfb reads and writes
*/

FxU32 rndfbzMode( void )
{
    FxU32  modeBits;
    FxBool passFlag = FXTRUE;
    
    do
    {
        modeBits = iRandom( ~0u );
        passFlag = FXTRUE;
        
        // Generate Random Mode
        modeBits = iRandom( ~0u );

        if(CSIM_PRIVATE(diago.sstCSIM)->environment.chipCount > 1)
          modeBits &= ~SST_YORIGIN;

    } while ( !passFlag );
    
    // Mask off Reserved Bit
    modeBits &= ~SST_ENSTIPPLEPATTERN;
    
    // Mask off Dither 
    modeBits &= ~(SST_ENDITHER | SST_ENDITHERSUBTRACT);
    
    // Mask off Stipple
    modeBits &= ~SST_ENSTIPPLE;

    // Mask off Special Z-Compare
    modeBits &= ~SST_ZCOMPARE_TO_ZACOLOR;
    
    // Only Turn off Depth/Alpha writes 10% of the time
    modeBits &= ~SST_RGBWRMASK;
    modeBits |= iRandom( 10 ) ? SST_RGBWRMASK : 0;
    
    modeBits &= ~SST_ZAWRMASK;
    modeBits |= iRandom( 10 ) ? SST_ZAWRMASK : 0;

    // SST_ENALPHABUFFER and SST_ENDEPTHBUFFER are mutually exclusive
    // Arbitrarily give depth buffering precedence
    if ( modeBits & SST_ENDEPTHBUFFER )
    {
	modeBits &= ~SST_ENALPHABUFFER;
    }
    // in 15bpp and 32bpp mode, this bit is illegal
    if (diago.rgb != 16)
	modeBits &= ~SST_ENALPHABUFFER;
    
    // Mask off Special Floating point Z
// GMT BUG:this should be OK to allow now
//    modeBits &= ~SST_DEPTH_FLOAT_SEL;
    
    return modeBits;
}

/* rndfbzColorPlath
**
** Summary - generates an acceptable random fbzColorPath for lfb writes
**
** Return Value - a set of register bits that is OK for lfb writes
*/

FxU32 rndfbzColorPath( void )
{
    FxU32  modeBits;
    FxBool passFlag = FXTRUE;
    
    do
    {
        modeBits = iRandom( ~0u );
        passFlag = FXTRUE;
        
        // RGBA Select and Alpha Select Are Irrelevant
        
        // Catch reserved bit in cca_localselect
        switch( modeBits & SST_ALOCALSELECT )
        {
          case 3 << SST_ALOCALSELECT_SHIFT:
            passFlag = FXFALSE;
            continue;
            break;
          default:
            break;
        }
        
        // Catch texture value and reserved bits in mselect
        switch( modeBits & SST_CC_MSELECT )
        {
          case SST_CC_MATREX:
          case SST_CC_MRGBTMU:
          case 6 << SST_CC_MSELECT_SHIFT:
          case 7 << SST_CC_MSELECT_SHIFT:
            passFlag = FXFALSE;
            continue;
            break;
          default:
            break;
        }
        
        // Catch texture value and reserved bits in mselect
        switch( modeBits & SST_CCA_MSELECT) 
        {
          case SST_CCA_MATREX:
          case 5 << SST_CCA_MSELECT_SHIFT:
          case 6 << SST_CCA_MSELECT_SHIFT:
          case 7 << SST_CCA_MSELECT_SHIFT:
            passFlag = FXFALSE;
            continue;
            break;
          default:
            break;
        }

    // !! I am making the assumption that SST_PARMADJUST and SST_ENTEXTUREMAP do !!
    // !! nothing when in LFB mode, which is likely not the case for SST_ENTEXTUREMAP !!
	// !! need to ask about this one... !! 
        
    } while( !passFlag );

	// Mask of ENTEXTUREMAP and PARMADJUST
	modeBits &= ~SST_ENTEXTUREMAP;
	modeBits &= ~SST_PARMADJUST;
    
    // Mask off reserved bit 7
    modeBits &= ~SST_LOCALSELECT_OVERRIDE_WITH_ATEX;

    // CC_ADD_CLOCAL and  CC_ADD_ALOCAL can't both be selected.
    if ( modeBits & SST_CC_ADD_CLOCAL )
    {
	modeBits &= ~SST_CC_ADD_ALOCAL;
    }

    // CCA_ADD_CLOCAL and  CCA_ADD_ALOCAL can't both be selected.
    if ( modeBits & SST_CCA_ADD_CLOCAL )
    {
	modeBits &= ~SST_CCA_ADD_ALOCAL;
    }
    
    return modeBits;
}

/* rndfogMode
**
** Summary - generates an acceptable random fogMode for lfb testing
**
** Return Value - a set of register bits that is OK for lfb writes
*/

FxU32 rndfogMode( void )
{
    FxU32  modeBits;
    FxBool passFlag = FXTRUE;
    
    do
    {
        modeBits = iRandom( ~0u );
        passFlag = FXTRUE;
        
        // Everything Passes this register
        
    } while( !passFlag );
    
    return modeBits;
}

// PIXPIPE EMULATION CODE


/*
** I really hate globals but this seems to be
** the cleanest solution given this is an aferthought. :(
*/
fbiColors srcColorPreFog;
FxU32     workingDepthValue;
FxU32     bufferDepthValue;
fbiColors bufferColorValue;
FxBool    pixelValid;

/* Function: EmulateChromaKey
**
** Summary: Emulate the functionality of the Chroma Key Unit
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE if pixel passes chroma key test
**               FXFALSE if pixel is invalidated
*/
FxBool EmulateChromaKey( fbiColors *pixel,
                         FxU32 x, FxU32 y, FxU32 depth,
                         SstRegs *shadowRegs,
                         SstRegs *realRegs )
{
    FxU32 r;
    FxU32 g;
    FxU32 b;
    
    // ChromaKey is XRGB
    r = shadowRegs->chromaKey & 0x00FF0000;
    r >>= 16;
    
    g = shadowRegs->chromaKey & 0x0000FF00;
    g >>= 8;
    
    b = shadowRegs->chromaKey & 0x000000FF;
    
    if ( (pixel->red == (short)r) && (pixel->green == (short)g) && (pixel->blue == (short)b) )
    {
        return FXFALSE;
    }
    
    return FXTRUE;
}


/* Function: EmulateCCU
**
** Summary: Emulate the functionality of the CCU.  There are certain register
**          combinations which must be considered invalid for the purposes of
**          this diagnositic, therefore it is the responsibility of the calling
**          code to check the validity of the CCU Mode bits before sending it
**          down the pixel pipe. 
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE in all cases, pixel contains new color value
*/
FxBool EmulateCCU( fbiColors *pixel,
                   FxU32 x, FxU32 y, FxU32 depth,
                   SstRegs *shadowRegs,
                   SstRegs *realRegs )
{
    fbiColors iterColor;
    fbiColors texColor; 
    fbiColors color0;
    fbiColors color1;
    
    // Init Colors
    color0.red = (short)((shadowRegs->c0 & 0x00FF0000) >> 16);
    color0.green = (short)((shadowRegs->c0 & 0x0000FF00) >> 8);
    color0.blue = (short)(shadowRegs->c0 & 0x000000FF);
    color0.alpha = (short)((shadowRegs->c0 & 0xFF000000) >> 24);
    
    color1.red = (short)((shadowRegs->c1 & 0x00FF0000) >> 16);
    color1.green = (short)((shadowRegs->c1 & 0x0000FF00) >> 8);
    color1.blue = (short)(shadowRegs->c1 & 0x000000FF);
    color1.alpha = (short)((shadowRegs->c1 & 0xFF000000) >> 24);
    
    // These will have an undefined state in the hardware, and should never be selected, but if they
    // are, this should give sufficiently wrong results to be noticed.
    texColor.red = iRandom( 0xFF );
    texColor.green = iRandom( 0xFF );
    texColor.blue = iRandom( 0xFF );
    texColor.alpha = iRandom( 0xFF );
    
    iterColor.red = pixel->red;
    iterColor.green = pixel->green; 
    iterColor.blue = pixel->blue; 
    iterColor.alpha = pixel->alpha;
    
    colorcombine( shadowRegs->fbzColorPath,   
		  0,
                  &iterColor,     
                  &texColor,      
                  pixel,         
                  &color1, 
                  &color0, 
                  (FxU16)((depth >> 24) & 0xFF),	// depth is .32 always
		  0,
                  pixel );
    
    return FXTRUE;
}

/* Function: EmulateAtest
**
** Summary: Emulate the functionality of the Alpha test unit
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE If Pixel Passes
**               FXFALSE If Pixel Fails
*/

FxBool EmulateATest( fbiColors *pixel,
                         FxU32 x, FxU32 y, FxU32 depth,
                         SstRegs *shadowRegs,
                         SstRegs *realRegs )
{
    // Alpha Test
      FxBool ltLine = (pixel->alpha < (short)
                       ((shadowRegs->alphaMode & SST_ALPHAREF) >> SST_ALPHAREF_SHIFT));
      FxBool eqLine = (pixel->alpha == (short)
                       ((shadowRegs->alphaMode & SST_ALPHAREF) >> SST_ALPHAREF_SHIFT));
      
      if ( !(!(shadowRegs->alphaMode & SST_ENALPHAFUNC) ||
	     ((shadowRegs->alphaMode & SST_ALPHAFUNC_LT) && ltLine) ||
	     ((shadowRegs->alphaMode & SST_ALPHAFUNC_GT) && !(ltLine || eqLine)) ||
	     ((shadowRegs->alphaMode & SST_ALPHAFUNC_EQ) && eqLine)) )
      {
          return FXFALSE;
      }

      return(FXTRUE);
}
	 
/* Function: EmulateZCompare
**
** Summary: Emulate the functionality of the Z Compare Unit
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE If Pixel Passes
**               FXFALSE If Pixel Fails
*/

FxBool EmulateZCompare( fbiColors *pixel,
                         FxU32 x, FxU32 y, FxU32 depth,
                         SstRegs *shadowRegs,
                         SstRegs *realRegs )
{
	FxU32  wFloat;
	FxI32  pixelDepthValue; 
	FxI32  zmax = diago.rgb == 32 ? 0xFFFFFF : 0xFFFF;

	// wfloat_select
	if ( shadowRegs->fbzMode & SST_WBUFFER )
	{
	  // Calculate W Value
	  if (shadowRegs->lfbMode & SST_LFB_WSELECT) 
	    {
	      depth = shadowRegs->zaColor & zmax;
	      depth <<= (diago.rgb==32) ? 6 : 14;	// get it into 2.30 format
	      wFloat = wBufferValue(depth);	// 2.30 format
	    }
	  else 
	    {
	      wFloat = wFloat64(FX_CREATE64(0, depth));
	      depth >>= 2;	      
	    }
	  pixelDepthValue = wFloat;
	}
	else
	{
	    pixelDepthValue = depth >> (diago.rgb==32 ? 8 : 16);
	}
GDBG_INFO(223,"pixel depth = %x,   %x %x\n",pixelDepthValue, depth, wFloat);
    
	// zbias_enable
	if ( shadowRegs->fbzMode & SST_ENZBIAS )
	{
	    if (diago.rgb == 32)
		pixelDepthValue += signExtend( shadowRegs->zaColor & zmax, 24 );
	    else
		pixelDepthValue += signExtend( shadowRegs->zaColor & zmax, 16 );
	}
GDBG_INFO(223,"biased pixel depth = %x,   %x %x\n",pixelDepthValue, depth, wFloat);
    
	// Clamp
	if ( pixelDepthValue < 0 )
	{
		pixelDepthValue = 0;
	}
	else if ( pixelDepthValue > zmax )
	{
		pixelDepthValue = zmax;
	}
    
	// Compare ( bufferDepthValue is global )
	if ( shadowRegs->fbzMode & SST_ENDEPTHBUFFER )
	{
		FxBool ltLine = pixelDepthValue < (FxI32) bufferDepthValue;
		FxBool eqLine = pixelDepthValue == (FxI32) bufferDepthValue;
        
		if ( !(((shadowRegs->fbzMode & SST_ZFUNC_LT) && ltLine) ||
			 ((shadowRegs->fbzMode & SST_ZFUNC_GT) && !(ltLine || eqLine)) ||
			 ((shadowRegs->fbzMode & SST_ZFUNC_EQ) && eqLine)) )
		{
			return FXFALSE;
		}
		else
		{
	            workingDepthValue = pixelDepthValue;
        	}
	}
	else if ( shadowRegs->fbzMode & SST_ENALPHABUFFER )
	{
		workingDepthValue = pixel->alpha;
	}
	else // Unconditionally write depth value down to depth buffer
	{
		workingDepthValue = pixelDepthValue;
	}
GDBG_INFO(223,"workding depth = %x\n",workingDepthValue);
    
    return FXTRUE;
}

/* Function: EmulateFog
**
** Summary: Emulate the functionality of the Fog Unit
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE in all cases, pixel contains new color value
*/
FxBool EmulateFog( fbiColors *pixel,
                   FxU32 x, FxU32 y, FxU32 depth,
                   SstRegs *shadowRegs,
                   SstRegs *realRegs )
{
    fogMode      fogMode;
    fbiColors    fogColor;
    fbiColors    srcPixel;
    FxI16        iterAlpha;
    FxI16        iterZ;
    FxU32        wFloat;
    FxI16        wExp;
    FxI16        wMant;
    FxI32        fogIndex;
    
    // Fog Table Stuff
    static FxU32 fogTable[64];
    
    // Store SRC Color Before Fogging
    srcColorPreFog.red = pixel->red;
    srcColorPreFog.green = pixel->green;
    srcColorPreFog.blue = pixel->blue;
    
    for ( fogIndex = 0; fogIndex < 32; fogIndex++ )
    {
	fogTable[fogIndex * 2] = shadowRegs->fogTable[fogIndex] & 0xFFFF;
	fogTable[fogIndex * 2 + 1] = (shadowRegs->fogTable[fogIndex] >> 16) & 0xFFFF;
    }
    
    // Setup arguments to fog
    fogMode.fog_enable = shadowRegs->fogMode & SST_ENFOGGING ? 1 : 0;
    fogMode.fog_mult = shadowRegs->fogMode & SST_FOGMULT ? 1 : 0;
    fogMode.fog_add = shadowRegs->fogMode & SST_FOGADD ? 1 : 0;
    fogMode.fog_constant = shadowRegs->fogMode & SST_FOG_CONSTANT ? 1 : 0;
    fogMode.fog_alpha = shadowRegs->fogMode & SST_FOG_ALPHA ? 1 : 0;
    fogMode.fog_z = shadowRegs->fogMode & SST_FOG_Z ? 1 : 0;
    fogMode.fog_dither = 0;
    fogMode.fog_zones = 0;

    fogColor.red = (unsigned short)((shadowRegs->fogColor & 0x00FF0000) >> 16);
    fogColor.green =(unsigned short)((shadowRegs->fogColor & 0x0000FF00) >> 8);
    fogColor.blue = (unsigned short)(shadowRegs->fogColor & 0x000000FF);
    
    // Calculate W Value
    wFloat = ( shadowRegs->lfbMode & SST_LFB_WSELECT ) ?
      wBufferValue( (shadowRegs->zaColor & 0xFFFF) << 14 ) :
      wBufferValue( depth >> 2 );
    // GMT BUG: this needs to be fixed to handle 24-bit 5.19 floats
    // if fogmode is ever enabled.... which it isn't in lfbwrite.c
    // we need to do the conversion to 5.19 and then clamp
    wFloat = wBufferValue( wFloat );
    wExp = (FxI16) (wFloat>>12);
    wMant = (FxI16) (wFloat & 0x0FFF);
    
    iterAlpha = pixel->alpha;
    iterZ = (short) ((depth >> 24) & 0xFF);
    
    srcPixel.red = pixel->red;
    srcPixel.green = pixel->green;
    srcPixel.blue = pixel->blue;
    srcPixel.alpha = pixel->alpha;
    
    fog( &fogMode, 0,0, &srcPixel, &fogColor, fogTable, iterAlpha, iterZ, wExp, wMant, pixel );
    
    return FXTRUE;  
}


/* Function: EmulateAlphaBlend
**
** Summary: Emulate the Functionality of the Alpha Blend Unit
**
** Parameters: See EmulatePixPipe
**
** Return Value: FXTRUE in all cases, pixel contains new color value
*/
FxBool EmulateAlphaBlend( fbiColors *pixel,
                          FxU32 x, FxU32 y, FxU32 depth,
                          SstRegs *shadowRegs,
                          SstRegs *realRegs )
{
    ablendMode blendMode;
    fbiColors  srcColor;
    fbiColors  destColor;
    FxU32 sbColor;
    
    // Set up arguments to ablend
    blendMode.ablend_en = shadowRegs->alphaMode & SST_ENALPHABLEND ? 1 : 0;
    blendMode.alpha_fact_src = (unsigned char) ((shadowRegs->alphaMode & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT);
    blendMode.alpha_fact_dst = (unsigned char) ((shadowRegs->alphaMode & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT);
    blendMode.subtract = (shadowRegs->fogMode & SST_RGB_BLEND_SUB) != 0;
    blendMode.reverse = (shadowRegs->fogMode & SST_RGB_BLEND_REVERSE) != 0;
    
    srcColor.red   = pixel->red;
    srcColor.green = pixel->green;
    srcColor.blue  = pixel->blue;
    srcColor.alpha  = pixel->alpha;
    
    // Read color out of frame buffer
    sbColor = CSIM_PIXEL_RD( currentDrawBuffer( shadowRegs ), x, y );
    if (diago.rgb == 16)
	destColor.alpha = (short) CSIM_PIXEL_RD(CSIM_BUF_3D_AUX1, x, y);
    else
	destColor.alpha = (short)(sbColor >> 24) & 0xFF;
    destColor.red   = (short)(sbColor >> 16) & 0xFF;
    destColor.green = (short)(sbColor >> 8) & 0xFF;
    destColor.blue  = (short)(sbColor >> 0) & 0xFF;
    if (diago.rgb == 15) {
	if (destColor.alpha)
	    destColor.alpha = 0xFF;
    }

    gdbg_info( 10, "  -RGBADV ABlend dst  = %.2x %.2x %.2x %.2x\n",
               destColor.red, destColor.green, destColor.blue, destColor.alpha);
    ablend( &blendMode, 0,0, &srcColor, &srcColorPreFog, &destColor, pixel );
    // GMT: we don't deal with blending the alpha channel
    destColor.alpha = srcColor.alpha;
    return FXTRUE;
}



/* Function: EmulatePixPipe
**
** Summary: Emulate Pixel Pipeline Functionality for LFB Writes
**
** Parameters: pixel      - input pixel from lfb write /
**                          storage for output pixel
**             x, y       - frame buffer coordinate
**             depth      - fixed point depth value
**             shadowRegs - pointer to shadowed register structure
**             realRegs   - pointer to hardware(simulator) registers
**                          these are necessary for alpha-blending
*/
FxBool EmulatePixPipe( fbiColors *pixel,
                     FxU32 x, FxU32 y, FxU32 *depth,
                     SstRegs *shadowRegs, SstRegs *realRegs )
{
	FxU32 fbzColorPathBackup;
	FxU32 tmpColor, stencil;
	FxI32  zmax = diago.rgb == 32 ? 0xFFFFFF : 0xFFFF;
	FxI32  zlshift = diago.rgb == 32 ? 8 : 16;
    
	// Fix Coordinate for YOrigin Flip
	if ( flipYOrigin( shadowRegs )  ) {
		y = (diago.ymaxscreen - 1) - y;
	}
    
	// Initialize State of Buffers, ValidFlag
	pixelValid = FXTRUE;
    
	stencil = CSIM_PIXEL_RD( CSIM_BUF_3D_AUX1, x, y );
	workingDepthValue = bufferDepthValue = stencil & 0x00FFFFFF;

	tmpColor = CSIM_PIXEL_RD( currentDrawBuffer( shadowRegs ), x, y );
	bufferColorValue.alpha   =(FxU16)((tmpColor >> 24) & 0xFF);
	bufferColorValue.red   =(FxU16)((tmpColor >> 16) & 0xFF);
	bufferColorValue.green =(FxU16)((tmpColor >>  8) & 0xFF);
	bufferColorValue.blue  =(FxU16) (tmpColor & 0xFF);
    
    
	gdbg_info( 10, "EmulatePixPipe\n" );
	gdbg_info( 10, "  -RGBADV In          = %.2x %.2x %.2x %.2x %.4x %.1s\n",
			   pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );

    
	// get depth in .32 format
	if ((shadowRegs->lfbMode & SST_LFB_FORMAT) != SST_LFB_Z32) {
	    *depth = (*depth<<16) | *depth;
	}
	// Truncate and MSB Replicate Input Colors Based on lfbMode
	// Set depth to register Value if Depth is not in the format
	// Set up alpha if alpha is not included
	switch( shadowRegs->lfbMode & SST_LFB_FORMAT ) {
		case SST_LFB_565:
			// if depth isn't included, get it from zaColor
			*depth = (shadowRegs->zaColor & zmax)<<zlshift;
		case SST_LFB_Z565:
			pixel->alpha = (FxU16) ( shadowRegs->zaColor >> 24 );
			pixel->red = (FxU16)packBits( 3, 0, 24, (pixel->red >> 3), 5, (pixel->red >> 5), 3 );
			pixel->green = (FxU16)packBits( 3, 0, 24, (pixel->green >> 2), 6, (pixel->green >> 6), 2 );
			pixel->blue = (FxU16)packBits( 3, 0, 24, (pixel->blue >> 3), 5, (pixel->blue >> 5), 3 );
			break;
		case SST_LFB_555:
			// if depth isn't included, get it from zaColor
			*depth = (shadowRegs->zaColor & zmax)<<zlshift;
		case SST_LFB_Z555:        
			pixel->alpha = (FxU16) ( shadowRegs->zaColor >> 24 );
			pixel->red = (FxU16)packBits( 3, 0, 24, (pixel->red >> 3), 5, (pixel->red >> 5), 3 );
			pixel->green = (FxU16)packBits( 3, 0, 24, (pixel->green >> 3), 5, (pixel->green >> 5), 3 );
			pixel->blue = (FxU16)packBits( 3, 0, 24, (pixel->blue >> 3), 5, (pixel->blue >> 5), 3 );
			break;
		case SST_LFB_1555:
			// if depth isn't included, get it from zaColor
			*depth = (shadowRegs->zaColor & zmax)<<zlshift;
		case SST_LFB_Z1555:       
			pixel->alpha = (FxU16)packBits( 2, 0, 24, (pixel->alpha?0xFF:0x0), 8 );
			pixel->red = (FxU16)packBits( 3, 0, 24, (pixel->red >> 3), 5, (pixel->red >> 5), 3 );
			pixel->green = (FxU16)packBits( 3, 0, 24, (pixel->green >> 3), 5, (pixel->green >> 5), 3 );
			pixel->blue = (FxU16)packBits( 3, 0, 24, (pixel->blue >> 3), 5, (pixel->blue >> 5), 3 );
			break;
		case SST_LFB_888:
			pixel->alpha = (FxU16) ( shadowRegs->zaColor >> 24 );
		case SST_LFB_8888:        
			// if depth isn't included, get it from zaColor
			*depth = (shadowRegs->zaColor & zmax)<<zlshift;
			break;
		case SST_LFB_ZZ:  
		case SST_LFB_Z32:
			pixel->alpha = (FxU16)( shadowRegs->zaColor >> 24 );
			pixel->red = (FxU16)((shadowRegs->c1 >> 16) & 0xFF);
			pixel->green = (FxU16)((shadowRegs->c1 >> 8) & 0xFF);
			pixel->blue = (FxU16)(shadowRegs->c1 & 0xFF);
			break;
		case SST_LFB_u1:          
		case SST_LFB_u2:  
		case SST_LFB_u3:  
		default:
			gdbg_printf( "Error: EmulatePixPipe() unrecognized/reserved lfb write format\n" );
			DIAG_INCERROR();
			break;
	}

	gdbg_info( 10, "  -RGBADV Internal    = %.2x %.2x %.2x %.2x %.08x %.1s\n",
			   pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );

	// If Pixel Pipe is Disabled, Punt
	if ( !(shadowRegs->lfbMode & SST_LFB_ENPIXPIPE ) ) {
		int zonly = 0;
		// *depth is in .32 format, now right justify it
		*depth >>= (diago.rgb == 32) ? 8 : 16;
		switch( shadowRegs->lfbMode & SST_LFB_FORMAT ) {
			case SST_LFB_565:
			case SST_LFB_555:
			case SST_LFB_888:
				*depth = stencil;	// previous contents
				if (diago.rgb!=15)
				  pixel->alpha = bufferColorValue.alpha;
				break;
			case SST_LFB_1555:
			case SST_LFB_8888:
				if ( shadowRegs->fbzMode & SST_ENALPHABUFFER ) {
					*depth = pixel->alpha;
				} else {
					*depth = stencil;// previous contents
				}
				break;
			case SST_LFB_Z565:
			case SST_LFB_Z555:
				if ( shadowRegs->fbzMode & SST_ENALPHABUFFER ) {
					*depth = stencil;// previous contents
				}
				if (diago.rgb!=15)
				  pixel->alpha = bufferColorValue.alpha;
				break;
			case SST_LFB_Z1555:
				if ( shadowRegs->fbzMode & SST_ENALPHABUFFER ) {
					*depth = pixel->alpha;
				}
				break;
			case SST_LFB_Z32:
			case SST_LFB_ZZ:
				pixel->red = bufferColorValue.red;
				pixel->green = bufferColorValue.green;
				pixel->blue = bufferColorValue.blue;
				pixel->alpha = bufferColorValue.alpha;
				if ( shadowRegs->fbzMode & SST_ENALPHABUFFER ) {
					*depth = stencil;// previous contents
				}
				zonly=1;
				break;
		}
		if (diago.rgb==15 && !zonly) {
		    if ((shadowRegs->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO)
			pixel->alpha = 0;
		    if ((shadowRegs->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE)
			pixel->alpha = 0xFF;
		}
		/* Truncate it. */
		if (diago.rgb < 32) {
		    pixel->red &= 0xf8;
		    pixel->green &= diago.rgb == 16 ? 0xfc : 0xf8;
		    pixel->blue &= 0xf8;
		}
		else {	// stencil planes are NEVER modified
		    *depth = (stencil & 0xFF000000) | (*depth & 0x00FFFFFF);
		}
		gdbg_info( 10, "  -disabled, color truncated\n" );
		gdbg_info( 10, "  -RGBADV Out         = %.2x %.2x %.2x %.2x %.08x %.1s\n",
		pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
		return pixelValid;
	}
    // now isolate just the stencil value
    stencil &= 0xFF000000;
    
    // Save Color Path and Force To LFB Inputs
    fbzColorPathBackup = shadowRegs->fbzColorPath;
    
    shadowRegs->fbzColorPath &= ~SST_RGBSELECT;
    shadowRegs->fbzColorPath &= ~SST_ASELECT;
    
    shadowRegs->fbzColorPath |= SST_RGBSEL_LFB;
    shadowRegs->fbzColorPath |= SST_ASEL_LFB;
    
    // Clip
    if ( shadowRegs->fbzMode & SST_ENRECTCLIP )
    {
	FxU32 left, right, top, bottom;
#ifdef CVG
	right  = shadowRegs->clipLeftRight & 0x000003FF;
	left   = (shadowRegs->clipLeftRight & 0x03FF0000) >> 16;
	top    = shadowRegs->clipBottomTop & 0x000003FF;
	bottom = (shadowRegs->clipBottomTop & 0x03FF0000) >> 16;
#else
	right  = (shadowRegs->clipLeftRight & SST_CLIPRIGHT) >> SST_CLIPRIGHT_SHIFT;
	left  = (shadowRegs->clipLeftRight & SST_CLIPLEFT) >> SST_CLIPLEFT_SHIFT;
	top  = (shadowRegs->clipBottomTop & SST_CLIPTOP) >> SST_CLIPTOP_SHIFT;
	bottom  = (shadowRegs->clipBottomTop & SST_CLIPBOTTOM) >> SST_CLIPBOTTOM_SHIFT;
#endif
    
        if ( (x > right) || (x < left) )
        {
            pixelValid = FXFALSE;
        }
        else if ( (y > top) || (y < bottom) )
        {
            pixelValid = FXFALSE;
        }
    }
    
    // Chroma Key
    if( EmulateChromaKey( pixel, x, y, *depth, shadowRegs, realRegs ) == FXFALSE )
    {
        // punt
        pixelValid = FXFALSE;
    }
    
    // AlphaMask Test 
    if ( shadowRegs->fbzMode & SST_ENALPHAMASK )
    {
	if ( !(pixel->alpha & 0x1 ) )
	{
	    pixelValid = FXFALSE;
	}
    }
    // Alpha test
    if ( EmulateATest( pixel, x, y, *depth, shadowRegs, realRegs ) == FXFALSE )
    {
        // punt
        pixelValid = FXFALSE;
    }
    // now check for pixel discard (no stencil stuff)
    if (!pixelValid) goto discard_pixel;

    // Color Combine
    EmulateCCU( pixel, x, y, *depth, shadowRegs, realRegs );
    gdbg_info( 10, "  -RGBADV CCU Out     = %.2x %.2x %.2x %.2x %.08x %.1s\n",
               pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
    
    // Depth Compare
    if ( EmulateZCompare( pixel, x, y, *depth, shadowRegs, realRegs ) == FXFALSE )
    {
        // punt
        pixelValid = FXFALSE;
    }
    gdbg_info( 10, "  -RGBADV ZA Cmp Out  = %.2x %.2x %.2x %.2x %.08x %.1s\n",
               pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
    
    // Fog
    EmulateFog( pixel, x, y, *depth, shadowRegs, realRegs );
    gdbg_info( 10, "  -RGBADV Fog Out     = %.2x %.2x %.2x %.2x %.08x %.1s\n",
               pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
    
    // Alpha Blend
    EmulateAlphaBlend( pixel, x, y, *depth, shadowRegs, realRegs );
    gdbg_info( 10, "  -RGBADV ABlend Out  = %.2x %.2x %.2x %.2x %.08x %.1s\n",
               pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
    
    // Stencil
    if ( shadowRegs->stencilMode & SST_STENCIL_ENABLE) {
	if ((shadowRegs->stencilMode & SST_STENCIL_FUNC)==0) {
            pixelValid = FXFALSE;
	    goto discard_pixel;	// keep old stencil
	}
    }
	
    // Dither ( Always Disabled in Diags )
    if ( shadowRegs->fbzMode & SST_ENDITHER )
    {
        GDBG_ERROR( "EmulatePixPipe",  "Warning, Dithering enabled and not supported\n" );
    }
    
    if ( pixelValid )
    {
        // RGB Write Mask
        if ( shadowRegs->fbzMode & SST_RGBWRMASK )
        {
	    if (diago.rgb==15) {
		if ((shadowRegs->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO)
		    pixel->alpha = 0;
		if ((shadowRegs->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE)
		    pixel->alpha = 0xFF;
	    }
            // Commit Pixel
            bufferColorValue.red = pixel->red;
            bufferColorValue.green = pixel->green;
            bufferColorValue.blue = pixel->blue;
            bufferColorValue.alpha = pixel->alpha;
        }
        
        // Depth Write Mask
        if (  shadowRegs->fbzMode & SST_ZAWRMASK ) 
        {
            // Commit Depth
            bufferDepthValue = workingDepthValue;
        }
    }
    // Stencil replace with reference value
    if (shadowRegs->stencilMode & SST_STENCIL_WMASK)
	stencil = (shadowRegs->stencilMode & SST_STENCIL_REF)<<(24-SST_STENCIL_REF_SHIFT);
    
discard_pixel:
    // Restore fbzColorPath
    shadowRegs->fbzColorPath = fbzColorPathBackup;
    
    pixel->red = bufferColorValue.red;
    pixel->green = bufferColorValue.green;
    pixel->blue = bufferColorValue.blue;
    pixel->alpha = bufferColorValue.alpha;
    *depth = bufferDepthValue | stencil;
    
    gdbg_info( 10, "  -RGBADV Out         = %.2x %.2x %.2x %.2x %.08x %.1s\n",
               pixel->red, pixel->green, pixel->blue, pixel->alpha, *depth, pixelValid ? "V" : "I" );
    return pixelValid;
}
