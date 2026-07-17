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
** $Revision: 5$
** $Date: 10/11/00 8:08:41 PM$
*/

#include <stdlib.h>
#include <string.h>

#include <h3.h>
#include "h3sim.h"

// sanity check
#if ( SST_ST64_FRACBITS != SST_W64_FRACBITS )
#  error "csimInit:  SST_ST_64_FRACBITS != SST_W64_FRACBITS"
#endif

#ifdef WINSIM
extern unsigned int dwHostV3Base1;
#endif // WINSIM

//Ugly f'n global variables. Had to be added to add a hack on.
//The csim is so f'ed that it shouldn't be noticeable though.
SstRegs *globalSST;
SstRegs *globalChildrenSST[3];
SstRegs *currentRenderingChip;

static FxU32 hack_numBytes;
static volatile unsigned char *hack_memory;

//----------------------------------------------------------------------
// this gets called before anything else and gives us a memory buffer to
// use instead of malloc'ing one
// this is a HACK used when we are used within a video driver
//----------------------------------------------------------------------
FX_EXPORT void FX_CSTYLE
csimInitMemory( FxU32 numBytes, volatile FxU32 *memory )
{
    GDBG_INFO( 100,"csimInitMemory(%d, 0x%x)\n", numBytes, memory );
    hack_numBytes = numBytes;
    hack_memory = (volatile unsigned char *)memory;
}

//----------------------------------------------------------------------
// this gets called to replace all the fxHal* calls to init the hardware
// take a real hardware address from caller
// this is a HACK used when we are used within a video driver
// we pretty much duplicate a lot of code in fxHalMapBoard() YUCK!!!
//----------------------------------------------------------------------
FX_EXPORT void FX_CSTYLE
csimInitHwAddress( volatile FxU32 *hw )
{
    int bn=0;

    GDBG_INFO( 100,"csimInitHwAddress(0x%x)\n", hw );
    fxHalInit( 0 );
    halInfo.csim = -1;			// flag that a video driver is the client
    fxHalMapBoard( bn );
    hw += SST_3D_OFFSET>>2;
    halInfo.boardInfo[bn].virtAddr[0] =(SstRegs *)hw;
    halInfo.boardInfo[bn].sstHW = (SstRegs *)hw;
    fxHalInitRegisters( (SstRegs *)hw );

    if ( guiOpen( bn ) == FXFALSE )	// open up a CSIM main control window
        halInfo.video = 0;
}

// convenience routine
FX_EXPORT void FX_CSTYLE
csimInitDriver( FxU32 numBytes, volatile FxU32 *memory,volatile FxU32 *hw )
{
    csimInitMemory( numBytes, memory );
    csimInitHwAddress( hw );
}


//----------------------------------------------------------------------
// allocate an instance of a SST chip for C simulation
//----------------------------------------------------------------------
FX_EXPORT SstRegs* FX_CSTYLE csimInit( FxU32 bn )
{
    SstRegs *sst;
    CsimPrivate *cpriv;
    static FxI32 callCounter=0;
    char *environmentValue;

    currentRenderingChip = NULL;

    sst = (SstRegs *)malloc( sizeof(SstRegs) );
    memset( (void *)sst,0,sizeof(SstRegs) );	// clear the memory to 0
    GDBG_INFO( 100,"csimInit: sstCSIM malloced at address 0x%x size=%d\n",
        sst,sizeof(SstRegs) );

    CSIM_PRIVATE( sst ) = cpriv = (CsimPrivate *)malloc( sizeof(CsimPrivate) );
    memset( cpriv,0,sizeof(CsimPrivate) );	// clear the memory to 0
    CSIMG_PRIVATE( &cpriv->gui ) = cpriv;
    cpriv->fifo0data.state = NEW_PACKET;	// init CMD FIFO state
    cpriv->fifo1data.state = NEW_PACKET;
    cpriv->info = &halInfo.boardInfo[bn];

    //Don't let anything write to the command fifo region unless
    //it is a raw lfb.
    cpriv->environment.allowAccessesToCommandFifoRegion = FXFALSE;

    //Initialize the PCI config space
    csimCfgInitialize( sst );

    //Look for triangle pertubation environment variables
    if ( environmentValue = getenv( "SST_TRIANGLE_OFFSET_X" ) )
    {
        if ( strncmp( environmentValue, "0x", 2 ) )
            cpriv->environment.triangleOffsetX = atoi( environmentValue );
        else
            sscanf( environmentValue, "0x%x", &cpriv->environment.triangleOffsetX );

        GDBG_INFO( 0, "X Triangle offset of 0x%x.%x\n", cpriv->environment.triangleOffsetX >> 4, 
            cpriv->environment.triangleOffsetX  & 0xF );
    }
    else
        cpriv->environment.triangleOffsetX = 0;

    //By default check for error where 12 NOPs to TMUs aren't written immediately before
    //switching from 2ppc to 1ppc
    cpriv->environment.detectNopError = FXTRUE;

    //Look for triangle pertubation environment variables
    if ( environmentValue = getenv( "SST_TRIANGLE_OFFSET_Y" ) )
    {
        if ( strncmp( environmentValue, "0x", 2 ) )
            cpriv->environment.triangleOffsetY = atoi( environmentValue );
        else
            sscanf( environmentValue, "0x%x", &cpriv->environment.triangleOffsetY );

        GDBG_INFO( 0, "Y Triangle offset of 0x%x.%x\n", cpriv->environment.triangleOffsetY >> 4, 
            cpriv->environment.triangleOffsetY & 0xF );
    }
    else
        cpriv->environment.triangleOffsetY = 0;

    if ( callCounter == 0 )
    {
        globalSST = sst;
        cpriv->environment.parentDevice = FXTRUE;
        cpriv->info->parentDevice = FXTRUE;

        strcpy( cpriv->environment.name, "Parent" );
    }
    else
    {	
        char name[32];

        globalChildrenSST[callCounter-1] = sst;
        cpriv->environment.parentDevice = FXFALSE;
        cpriv->info->parentDevice = FXFALSE;
        sprintf( name, "Child%d", callCounter-1 );
        strcpy( cpriv->environment.name, name );
    }
    cpriv->environment.aaPrimaryBuffers = FXTRUE;
    cpriv->environment.chipIndex = callCounter;
    callCounter++;

    // init all environement options
#ifndef NO_FLOAT
    /* NO_FLOAT
    * loderr is only used in the "#define COMPARE 1" case,
    * which we don't care about, so we can safely cut this line out
    */
    cpriv->environment.loderr = 999.0;
#endif /* #ifndef NO_FLOAT */

    cpriv->environment.flushOnCommands = 0;
    cpriv->environment.flushCount = 0;

    cpriv->environment.pauseAfterNextSwap = (getenv( "CSIM_PAUSE_AFTER_SWAP" ) != NULL);
    cpriv->environment.saveAfterSwap = (getenv( "CSIM_SAVE_AFTER_SWAP" ) != NULL);
    if ( cpriv->environment.saveAfterSwap ) gdbg_printf( "CSIM_SAVE_AFTER_SWAP is enabled.\n" );

    if ( getenv( "CSIM_RECIP" ) )
    {
        cpriv->environment.recipFlag = atoi( getenv( "CSIM_RECIP" ) );
        gdbg_printf( "WARNING: CSIM_RECIP = %d\n",cpriv->environment.recipFlag );
    }
    if ( getenv( "CSIM_FASTFILL" ) )
    {
        cpriv->environment.fastFill = 1;
        gdbg_info( 0,"CSIM_FASTFILL is enabled, fastfills are not dithered\n" );
    }
    if ( getenv( "CSIM_STATS" ) )
    {
        cpriv->environment.statsVerboseLevel = atoi( getenv( "CSIM_STATS" ) );
        gdbg_info( 0,"CSIM_STATS = %d\n",cpriv->environment.statsVerboseLevel );
    }

    sstRecipInit( sst );
    csimRegisterInfoInit( sst );
    sst->status = SST_FIFOLEVEL;			// make FIFO empty

    cpriv->chipMask = 0x7;			// 1 FBI, 2 TMUs
#ifndef NO_FLOAT
    /* NO_FLOAT
    * ooSTWfracbits is completely unused, so we can safely cut this line out
    */
    cpriv->ooSTWfracbits = 1.0F/(1<<4)/(1<<(SST_ST64_FRACBITS-4));
#endif /* #ifndef NO_FLOAT */

    // malloc TMU0's data for initRegisters
    TMU_PRIVATE( cpriv->trex ) = (TmuData *)malloc( sizeof(TmuData) );
    CSIM_PRIVATE( cpriv->trex ) = cpriv;

    // malloc TMU1's data for initRegisters
    TMU_PRIVATE( &cpriv->trex[1] ) = (TmuData *)malloc( sizeof(TmuData) );
    CSIM_PRIVATE( &cpriv->trex[1] ) = cpriv;

    return sst;
}

FX_EXPORT void FX_CSTYLE csimMakeMultiFunctionDevice(SstRegs *sst)
{
    CsimPrivate *cp=CSIM_PRIVATE( sst );
    //This says, "Hey! look at me mo' freaka. I'm a multi-function device."
    //See PCI Spec 2.1 page 188 for details
    //headerType;	      //14	7:0	PCI Header Type
    cp->pciConfigRegs.BIST_headerType_latencyTimer_cacheLineSize = ((1<<7) << 16);  
}


void csimShutdown( SstRegs *sst )
{
    int i;
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    GDBG_INFO( 100,"csimShutdown(0x%x) csim=0x%x\n", sst, sst );
    for ( i=0; i<MAX_NUM_TMUS; i++ )
    {
        TmuData *td = TMU_PRIVATE( cpriv->trex+i );
        if ( td != NULL )
        {
            free( td );
        }
    }
    free( cpriv );
    free( (void *)sst );
}

// get a variable from the environment, compare to actual hardware
static void
csimGetEnv( const char *name, const char *from, FxU32 *pval, FxU32 defaultVal)
{
    FxU32 temp;
    if ( GETENV( name ) )
    {			// get value from environment
        temp = atoi( GETENV( name ) );
        GDBG_PRINTF( "INFO: %s is set to %d\n",name,temp );
        if ( halInfo.hw || halInfo.hsim )
        {	// sanity compare to hardware
            if ( *pval != DEAD && *pval != temp )
            {
                // Don't allow mismatch between hw and env for SST_TMU_NUM.
                // Warn user about mismatches for other settings.
                if ( strcmp( name,"SST_TMU_NUM" ) )
                    GDBG_ERROR( from, "%s mismatch, CSIM=%d HW=%d\n", name, temp, *pval );
                else
                    GDBG_PRINTF( "WARNING: (%s) %s mismatch, CSIM=%d HW=%d\n", from,name, temp, *pval );
            }
        }
        *pval = temp;			// set the value now
    }
    else if ( *pval == DEAD ) // if no env setting and no hardware setting
        *pval = defaultVal;	  // then use default
    else if ( halInfo.hw || halInfo.hsim )// else let hardware value ride
        GDBG_INFO( 3,"        configuring CSIM:%s=%d(0x%x) from HW\n",name,*pval,*pval );
}

// set the FBI device ID and revision, defaults are passed in
void csimFbiSetRevision( SstRegs *sst )
{
    FxDeviceInfo *info = CSIM_PRIVATE( sst )->info;
    csimGetEnv( "SST_DEVICE_ID", "csimFbiSetRevision", &info->deviceID, CSIM_DEFAULT_DEVICE_ID );
    csimGetEnv( "SST_FBI_REV", "csimFbiSetRevision", &info->fbiRevision, CSIM_DEFAULT_FBI_REV );
    csimRegisterInfoInit( sst );	// reset any register info
    GDBG_INFO( 100,"    Device ID:    %d\n",info->deviceID );
    GDBG_INFO( 100,"    FBI Revision: %d\n",info->fbiRevision );
}

// set the FBI memory size
void csimFbiSetMemory( SstRegs *sst )
{
    CsimPrivate *cpriv;
    static unsigned char *compositeMemory=NULL;
#ifndef WINSIM
    FxI32 i;
#endif

    if ( !halInfo.csim ) return;		// if not using CSIM return
    cpriv = CSIM_PRIVATE( sst );
    if ( cpriv->memory ) free( (char *)cpriv->memory );
    if ( hack_memory )
    {
        cpriv->info->fbiMemSize = hack_numBytes >> 20; 
        cpriv->memorySizeInBytes = hack_numBytes;
        cpriv->memory = hack_memory;
        GDBG_INFO( 101,"    FBI Memory: %d (supplied)\n",cpriv->info->fbiMemSize );
    }
    else
    {
        if ( hack_numBytes )
            cpriv->info->fbiMemSize = hack_numBytes >> 20;
        else
            csimGetEnv( "SST_FBI_MEM", "csimFbiSetMemory", &cpriv->info->fbiMemSize, CSIM_DEFAULT_FBI_MEM );
        GDBG_INFO( 101,"    FBI Memory: %d\n",cpriv->info->fbiMemSize );
        cpriv->memorySizeInBytes = MBYTE( cpriv->info->fbiMemSize );
        cpriv->memory = malloc( cpriv->memorySizeInBytes );
    }
    if ( cpriv->memory == NULL )
        GDBG_ERROR( "csimFbiSetMemory", "out of memory\n" );

    //If running multichip, make sure the composite memory is setup
#ifdef WINSIM
    compositeMemory = (unsigned char *) dwHostV3Base1;
    cpriv->compositeMemory = compositeMemory; 
#else    
    //Set all memory to 0
    for ( i=0; i<cpriv->memorySizeInBytes; i+=4 )
        *((FxU32*)(&cpriv->memory[i])) = 0; 

    if ( compositeMemory == NULL )
    {
        GDBG_INFO( 101, "Allocating %d bytes for multi-chip composite memory\n",
            cpriv->memorySizeInBytes );
        compositeMemory = (unsigned char *)malloc( cpriv->memorySizeInBytes );

        if ( compositeMemory == NULL )
        {
            GDBG_ERROR( "csimFbiSetMemory", "can't allocate memory for composite frame buffer!\n" );
        }
    }	
    cpriv->compositeMemory = compositeMemory; 
#endif // WINSIM    
}

void csimFbiSetBuffers( SstRegs *sst )
{
    int bufsize;
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );

    // compute number of pages in a video buffer, then multiply by page size
#ifdef CVG
    bufsize = (sst->fbiInit2 & SST_VIDEO_BUFFER_OFFSET) >> SST_VIDEO_BUFFER_OFFSET_SHIFT;
    bufsize *= 4096;
#else // H3
    bufsize = 150;
    bufsize *= 4096;    
    GDBG_INFO( 102,"csimFbiSetBuffers PS WARNING: bufsize HARDCODED to %d (640x480 res)\n",bufsize );
#endif
    cpriv->numColorBuffers = 2;
    cpriv->numBuffers = 3;
    if ( bufsize*2 > cpriv->memorySizeInBytes )
    {
        GDBG_ERROR( "csimFbiSetBuffers","no room for buffer 1\n" );
        cpriv->numColorBuffers--;
        cpriv->numBuffers--;
    }
    if ( bufsize*3 > cpriv->memorySizeInBytes )
    {
        GDBG_PRINTF( "WARNING: csimFbiSetBuffers - no room for buffer 2\n" );
        cpriv->numBuffers--;
    }
    GDBG_INFO( 102,"csimFbiSetBuffers: buffers = %d    bufsize = %d\n",
        cpriv->numBuffers,bufsize );
}

// set the TMU revision and memory, defaults are passed in
void csimTmuSetMemory( SstRegs *sst )
{
    FxU32 i;
    CsimPrivate *cpriv = CSIM_PRIVATE( sst );
    FxDeviceInfo *info;

    if ( !halInfo.csim ) return;		// if not using CSIM return
    info = cpriv->info;
    csimGetEnv( "SST_TMU_NUM", "csimTmuSetMemory", &info->numberTmus, CSIM_DEFAULT_TMU_NUM );
    csimGetEnv( "SST_TMU_REV", "csimTmuSetMemory", &info->tmuRevision, CSIM_DEFAULT_TMU_REV );
    csimRegisterInfoInit( sst );	// reset any register info

    if ( info->numberTmus > MAX_NUM_TMUS )
        GDBG_ERROR( "csimTmuSetMemory", "too many TMUs: %d\n",info->numberTmus );

    GDBG_INFO( 100,"    number TMUs: %d\n",info->numberTmus );
    if ( info->numberTmus > MAX_NUM_TMUS )
    {
        GDBG_ERROR( "csimTmuSetMemory","CSIM compiled for %d TMUs, but %d requested\n",
            MAX_NUM_TMUS,info->numberTmus );
        exit( 3 );
    }
    // setup the simulator to work with 'n' TREX chips
    for ( i=0; i < info->numberTmus; i++ )
    {
        TmuData *td;
        SstRegs *tmu = cpriv->trex+i;

        // NOTE: we already allocated TMU0 during init
        if ( i>0 )
        {
            td = (TmuData *)malloc( sizeof(TmuData) );
            TMU_PRIVATE( tmu ) = td;
        }
        else td = TMU_PRIVATE( tmu );
        CSIM_PRIVATE( tmu ) = cpriv;
        cpriv->tmuPrivate[i] = td;		// helps with debugging
        td->myNumber = i;
        // sanity check
        if ( info->tmuMemSize[i] != 0xDEAD && info->tmuMemSize[i] != 0 )
        {
            GDBG_ERROR( "csimTmuSetMemory", "tmuMemSize for TMU %d is %d not 0\n",
                i,info->tmuMemSize[i] );
        }
        info->tmuMemSize[i] = 0;
    }
    CSIM_PRIVATE( sst )->chipMask = SST_MASK( info->numberTmus+1 );
}

#if ! defined(KERNEL)
//----------------------------------------------------------------------
// Picture/Movie utilities
//----------------------------------------------------------------------
#include <fximg.h>

static char *picsave_prefix = NULL;

static FxBool _picsave(CsimPrivate *cp, const char *filename,
			FxU32 buffer, FxU32 width, FxU32 height, FxU32 type)
{
    char filenameEx[1000];
    FxU32 x,y,*buf;
    ImgInfo info;

    info.any.width = width;
    info.any.height = height;
    info.any.sizeInBytes = width * height * 4;
    info.any.data = (ImgData *)malloc( info.any.sizeInBytes );
    strcpy( filenameEx,filename );
#if !defined( __unix__ )
    strlwr( filenameEx );
#endif
    // IMG_UNKNOWN means the filename dictates the format
    if ( type == IMG_UNKNOWN )
    {
        int len = strlen( filenameEx );
        if ( strcmp( &filenameEx[len-4],".ppm" )==0 )
            type = IMG_P6;
        else if ( strcmp( &filenameEx[len-4],".sbi" )==0 )
            type = IMG_SBI;
        else if ( strcmp( &filenameEx[len-4],".tga" )==0 )
            type = IMG_TGA32;
        if ( type != IMG_UNKNOWN )
            filenameEx[len-4] = '\0';
    }
    switch ( type )
    {
    case IMG_P6:
        strcat( filenameEx, ".ppm" );
        break;
    case IMG_SBI:
        info.sbiInfo.redBits = 5;
        info.sbiInfo.greenBits = 6;
        info.sbiInfo.blueBits = 5;
        info.sbiInfo.yOrigin = 1;
        strcat( filenameEx, ".sbi" );
        break;
    case IMG_TGA32:
        info.tgaInfo.yOrigin = 0;
        strcat( filenameEx, ".tga" );
        break;
    default:
        GDBG_ERROR( "csimPicSave", "invalid image type %d\n",type );
        return FXFALSE;
    }

    // note we first convert the framebuffer which is in 16-bit RGB format
    // to standard 32-bit ARGB format (in memory) for the image library
    buf = (FxU32 *)info.any.data;
    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            FxU32 cfb = csimReadPixel( cp->info->sstCSIM,buffer,x,y );
#ifdef __unix__
            {
                FxU8 r, g, b;
                // since Suns are big endian, store as BGRA, so when read
                r = ((cfb & 0xF800) << 8) >> 16;
                g = ((cfb & 0x07E0) << 5) >> 8;
                b = ((cfb & 0x001F) << 3);
                *buf++ = (b<<24) | (g<<16) | (r<<8);
            }
#else
            *buf++ = ((cfb & 0xF800) << 8) | ((cfb & 0x07E0) << 5) |((cfb & 0x001F) << 3);
#endif
        }
    }
    if ( !imgWriteFile( filenameEx,&info, type, info.any.data ) )
        GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", filename, imgGetErrorString( ) );
    free( info.any.data );
    return FXTRUE;
}

// here is code to create movie file names....
#if 0
    char cbuf[256];
    static int filenum = 0;

    if ( picsave_prefix == NULL )
{			// if not yet defined
    picsave_prefix = getenv( "CSIM_PICSAVE" );	// then try environment var
    if ( picsave_prefix == NULL )			// if still no luck
        picsave_prefix = "mov";			// then default it
}
    sprintf(cbuf,"%s%03d",picsave_prefix,filenum++);
#endif

// if buffer >=0 then save a buffer
// else enable/disable the saving of pictures during swapbuffers
FX_EXPORT FxBool FX_CSTYLE
csimPicSave(CsimPrivate *cp, const char *filename, FxU32 buffer, FxU32 width, FxU32 height, FxU32 imgType)
{
    // now decode the request
    switch ( buffer )
    {
    case 0:
    case 1:
    case 2:		// aux buffer
#ifndef CVG // H3
    case 4:         // color buffer
#endif
        return _picsave( cp,filename,buffer,width,height,imgType );
        break;
    default:
        GDBG_ERROR( "csimPicSave", "buffer %d NYI\n",buffer );
    }
    return FXFALSE;
}
#endif

//This should only be passed the fbi sst; do not pass it the sst
//for the tmus.
FX_EXPORT FxBool FX_CSTYLE csimIsMultiTexturing(SstRegs *sst)
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    return( csimIsMultiTexturing2( sst->fbzColorPath, sst->combineMode,
        cp->trex[0].textureMode, cp->trex[0].combineMode ) );			       
}


//This function determines if we're actually using both TMU's to do useful multitexturing
//work. This does not include the case where tmu1 is doing work and tmu0 is passing 
//through the result.
FX_EXPORT FxBool FX_CSTYLE csimIsMultiTexturing2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
						 FxU32 tmu0TextureMode, FxU32 tmu0CombineMode)
{
    FxBool tmu0Used, rgbTmu0Used, alphaTmu0Used;
    FxBool tmu1Used;

    tmu0Used = FXFALSE;
    tmu1Used = FXFALSE;

    //If we're not texture mapping, we're definitely not
    //multi-texturing
    if ( !(fbiFbzColorPath & SST_ENTEXTUREMAP) )
        return( FXFALSE );

    //Check for trilinear (This is overly paranoid)
    if ( tmu0TextureMode & SST_TRILINEAR )
        return( FXTRUE );

    //Check to see if TMU 0 is using TMU 1's output
    if ( fbiCombineMode & SST_CM_USE_COMBINE_MODE )
    {      
        //Using combineMode to select other and local input

        //RGB junk
        if ( (((tmu0CombineMode & SST_CM_TC_OTHERSELECT) == SST_CM_TC_OTHERSELECT_OTHER_TRGB) ||
            ((tmu0CombineMode & SST_CM_TC_OTHERSELECT) == SST_CM_TC_OTHERSELECT_OTHER_TA)) &&
            (tmu0TextureMode & SST_TC_ZERO_OTHER) == 0 )
            tmu1Used=FXTRUE;

        if ( ((tmu0CombineMode & SST_CM_TC_LOCALSELECT) == SST_CM_TC_LOCALSELECT_OTHER_TRGB) ||
            ((tmu0CombineMode & SST_CM_TC_LOCALSELECT) == SST_CM_TC_LOCALSELECT_OTHER_TA) )      
            tmu1Used=FXTRUE;

        if ( (tmu0CombineMode & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB )
            tmu1Used=FXTRUE;

        //Alpha junk
        if ( ((tmu0CombineMode & SST_CM_TCA_OTHERSELECT) == SST_CM_TCA_OTHERSELECT_OTHER_TA) &&
            (tmu0TextureMode & SST_TCA_ZERO_OTHER) == 0 )
            tmu1Used=FXTRUE;

        if ( (tmu0CombineMode & SST_CM_TCA_LOCALSELECT) == SST_CM_TCA_LOCALSELECT_OTHER_TA )
            tmu1Used=FXTRUE;
    }
    else
    {
        //Using textureMode to select other input
        if ( (tmu0TextureMode & SST_TC_ZERO_OTHER) == 0 )
            tmu1Used=FXTRUE;

        if ( (tmu0TextureMode & SST_TCA_ZERO_OTHER) == 0 )
            tmu1Used=FXTRUE;
    }


    if ( ((tmu0TextureMode & SST_TC_MSELECT) == SST_TC_MAOTHER) ||
        ((tmu0TextureMode & SST_TCA_MSELECT) == SST_TCA_MAOTHER) )
        tmu1Used=FXTRUE;


    //Check if tmu0 is in pass-through
    rgbTmu0Used=FXTRUE;
    alphaTmu0Used=FXTRUE;

    //RGB check
    if ( (tmu0CombineMode & 
        (SST_CM_CC_INVERT_OTHER | SST_CM_CC_INVERT_LOCAL |					    
        SST_CM_CC_INVERT_ADD_LOCAL | SST_CM_CC_OUTSHIFT)) == 0 )
    {
        if ( (tmu0TextureMode &
            (SST_TC_ZERO_OTHER | SST_TC_SUB_CLOCAL | SST_TC_MONE | 
            SST_TC_REVERSE_BLEND | SST_TC_ADD_CLOCAL | SST_TC_ADD_ALOCAL)) == 0 )
            rgbTmu0Used = FXFALSE;
    }

    //Alpha check
    if ( (tmu0CombineMode & 
        (SST_CM_CCA_INVERT_OTHER | SST_CM_CCA_INVERT_LOCAL |					    
        SST_CM_CCA_INVERT_ADD_LOCAL | SST_CM_CCA_OUTSHIFT)) == 0 )
    {
        if ( (tmu0TextureMode &
            (SST_TCA_ZERO_OTHER | SST_TCA_SUB_CLOCAL | SST_TCA_MONE | 
            SST_TCA_REVERSE_BLEND | SST_TCA_ADD_CLOCAL | SST_TCA_ADD_ALOCAL)) == 0 )
            alphaTmu0Used = FXFALSE;
    }


    tmu0Used = rgbTmu0Used || alphaTmu0Used;

    /*
    GDBG_INFO(0, "tmu 0: combineMode=0x%08x textureMode=0x%08x\n",
        tmu0CombineMode, tmu0TextureMode);
    GDBG_INFO(0, "tmu0Used=%d rgbTmu0Used=%d alphaTmu0Used=%d tmu1Used=%d\n",
        tmu0Used, rgbTmu0Used, alphaTmu0Used, tmu1Used);
    */

    return( tmu0Used && tmu1Used );  
}

FX_EXPORT FxI32 FX_CSTYLE csimActiveTMUs(SstRegs *sst)
{
    CsimPrivate *cp = CSIM_PRIVATE( sst );

    return( csimActiveTMUs2( sst->fbzColorPath, sst->combineMode,
        cp->trex[0].textureMode, cp->trex[0].combineMode ) );
}

//This function returns the number of active TMUs
//  texturing disabled => 0
//
//  TMU 0 not using TMU 1's output => 1
//
//  TMU 0 using TMU 1's output => 2.    
//     This includes the case when TMU 0 is just passing
//     through the results of TMU 1.
FX_EXPORT FxI32 FX_CSTYLE csimActiveTMUs2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
					  FxU32 tmu0TextureMode, FxU32 tmu0CombineMode)
{  
    FxI32 activeTMUCount=1;

    if ( !(fbiFbzColorPath & SST_ENTEXTUREMAP) )
        return( 0 );

    //Check for trilinear (This is overly paranoid)
    if ( tmu0TextureMode & SST_TRILINEAR )
        return( 2 );

    //Check to see if TMU 0 is using TMU 1's output
    //RGB junk
    if ( (((tmu0CombineMode & SST_CM_TC_OTHERSELECT) == SST_CM_TC_OTHERSELECT_OTHER_TRGB) ||
        ((tmu0CombineMode & SST_CM_TC_OTHERSELECT) == SST_CM_TC_OTHERSELECT_OTHER_TA)) &&
        (tmu0TextureMode & SST_TC_ZERO_OTHER) == 0 )
        activeTMUCount=2;

    if ( ((tmu0CombineMode & SST_CM_TC_LOCALSELECT) == SST_CM_TC_LOCALSELECT_OTHER_TRGB) ||
        ((tmu0CombineMode & SST_CM_TC_LOCALSELECT) == SST_CM_TC_LOCALSELECT_OTHER_TA) )      
        activeTMUCount=2;

    if ( ((tmu0CombineMode & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB) &&
        (tmu0TextureMode & SST_TC_MSELECT == SST_TC_MCMSELECT7) )
        activeTMUCount=2;

    //Alpha junk
    if ( ((tmu0CombineMode & SST_CM_TCA_OTHERSELECT) == SST_CM_TCA_OTHERSELECT_OTHER_TA) &&
        (tmu0TextureMode & SST_TCA_ZERO_OTHER) == 0 )
        activeTMUCount=2;

    if ( (tmu0CombineMode & SST_CM_TCA_LOCALSELECT) == SST_CM_TCA_LOCALSELECT_OTHER_TA )
        activeTMUCount=2;

    if ( (tmu0TextureMode & SST_TC_MSELECT == SST_TC_MAOTHER) )
        activeTMUCount=2;

    if ( (tmu0TextureMode & SST_TCA_MSELECT == SST_TCA_MAOTHER) )
        activeTMUCount=2;

    return( activeTMUCount );
}

