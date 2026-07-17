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
** File Name: 	fnproto.h
**
** Description: Function prototypes for dd32 sources.
**
*/

/***************************************************************************/
/*                             DEFINES                                     */
/***************************************************************************/

// we handle video to video, system memory to video blts

typedef enum tag_bltType { vid2vid, mem2vid, vid2mem, mem2mem } BLT_TYPE;


/*-------------------------------------------------------------------------
 *  ddblt32.c
 *-------------------------------------------------------------------------
 */
DWORD __stdcall Blt32_ColorFill( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD );
DWORD __stdcall Blt32_DoBltNoSP( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD );
DWORD __stdcall Blt32_DoBltS( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD );
DWORD __stdcall Blt32_FastFill( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD );
DWORD __stdcall Blt32_SystemToVideo( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD, DWORD );
#if !defined(WINNT) || (_WIN32_WINNT >= 0x0500)
DWORD __stdcall Blt32_CopyFourCC(NT9XDEVICEDATA *,LPDDRAWI_DDRAWSURFACE_LCL,RECTL*,LPDDRAWI_DDRAWSURFACE_LCL,RECTL*);
#endif
VOID  StretchTexture( NT9XDEVICEDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,RECTL*,RECTL*);

DWORD __stdcall DdBlt( LPDDHAL_BLTDATA );
DWORD __stdcall DdGetBltStatus( LPDDHAL_GETBLTSTATUSDATA );

/*-------------------------------------------------------------------------
 *  ddcsim32.c
 *-------------------------------------------------------------------------
 */
DWORD __stdcall h3Read32( volatile void * );
void  __stdcall h3Write32( volatile void *, unsigned long );
void  __stdcall h3Write32f( volatile void *, float );

/*-------------------------------------------------------------------------
 *  ddflip.c
 *-------------------------------------------------------------------------
 */
#if !defined(WINNT) && defined(AGP_CMDFIFO)

VOID Promote_CmdFifoToAGP( NT9XDEVICEDATA * ppdev );
VOID Demote_CmdFifoFromAGP( NT9XDEVICEDATA * ppdev );
int  DoConfig( NT9XDEVICEDATA * ppdev );

#endif // !defined(WINNT) && defined(AGP_CMDFIFO)

DWORD __stdcall DdGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA );
DWORD __stdcall DdGetScanLine( LPDDHAL_GETSCANLINEDATA );
DWORD __stdcall DdFlip( LPDDHAL_FLIPDATA );
DWORD __stdcall DdWaitForVerticalBlank( LPDDHAL_WAITFORVERTICALBLANKDATA );

#ifndef WINNT
DWORD __stdcall DdFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA );
#endif

BOOL  FXGETFLIPSTATUS( NT9XDEVICEDATA * );
BOOL  FXINVERTICALBLANK( NT9XDEVICEDATA * );
DWORD FXGETSCANLINE( NT9XDEVICEDATA * );
WORD  Set_Busy( WORD * );

/*-------------------------------------------------------------------------
 *  ddfx95.c
 *-------------------------------------------------------------------------
 */
DWORD _stdcall DriverInit( DWORD );
BOOL __stdcall DrvEnableDirectDraw( DHPDEV, DDHAL_DDCALLBACKS *, DDHAL_DDSURFACECALLBACKS *, DDHAL_DDPALETTECALLBACKS * );
BOOL __stdcall DrvGetDirectDrawInfo( DHPDEV, DDHALINFO *, DWORD *, VIDEOMEMORY *, DWORD *, DWORD * );

/*-------------------------------------------------------------------------
 *  ddfxs32.c
 *-------------------------------------------------------------------------
 */
double Exponential( double, double );
double Logarithm( double );
void LOADGAMMA( NT9XDEVICEDATA * );
void RESTOREGAMMA( NT9XDEVICEDATA * );
void SETPALETTE( NT9XDEVICEDATA *, DWORD FAR *, int, int );

#ifdef SLI_AA
DWORD Promote_DeviceToSLIAA(NT9XDEVICEDATA * ppdev);
DWORD Demote_DeviceFromSLIAA(NT9XDEVICEDATA * ppdev);
void  Compute_SLIAA_Config(NT9XDEVICEDATA * ppdev, FxU32 numBuffers);
void  Clear_SLIAA_Buffers(NT9XDEVICEDATA * ppdev);
void  MoveTileMark(NT9XDEVICEDATA * ppdev);
DWORD SwitchToHostCursor(NT9XDEVICEDATA * ppdev);
#ifdef RD_ABORT_ERROR
void  Modify_SLI_Read(NT9XDEVICEDATA * ppdev, DWORD dwRequest);
#endif
#endif

#ifndef WINNT
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
void __stdcall TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *);
void __stdcall TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *);
#else
void __stdcall TXTRCHANGEDPALETTE( NT9XDEVICEDATA *, LPDDRAWI_DDRAWPALETTE_GBL);
void __stdcall TXTRNEWPALETTE( NT9XDEVICEDATA *, LPDDRAWI_DDRAWPALETTE_GBL);
#endif
#endif

DWORD __stdcall DdCreatePalette( LPDDHAL_CREATEPALETTEDATA );
DWORD __stdcall DdDestroyDriver( LPDDHAL_DESTROYDRIVERDATA );
DWORD __stdcall DdDestroyPalette( LPDDHAL_DESTROYPALETTEDATA );
DWORD __stdcall DdSetEntries( LPDDHAL_SETENTRIESDATA );

#ifndef WINNT
DWORD __stdcall DdSetExclusiveMode( LPDDHAL_SETEXCLUSIVEMODEDATA );
#endif

DWORD __stdcall DdSetDrvColorKey( LPDDHAL_DRVSETCOLORKEYDATA );
DWORD __stdcall DdSetPalette( LPDDHAL_SETPALETTEDATA );

#if ENABLE_3D
DWORD Enter_3DApplication( NT9XDEVICEDATA * );
DWORD Exit_3DApplication( NT9XDEVICEDATA * );
#endif

#if ENABLE_TILED_HEAP
DWORD Promote_PrimaryToOverlay( NT9XDEVICEDATA * );
DWORD Demote_PrimaryFromOverlay( NT9XDEVICEDATA * );
#endif

/*-------------------------------------------------------------------------
 *  ddinit.c
 *-------------------------------------------------------------------------
 */
#ifdef WINNT
extern DWORD __stdcall DdMapMemory(  PDD_MAPMEMORYDATA lpMapMemory  );
#endif

#if ENABLE_3D
#if defined(TnL_HAL) && defined(VERT_BUFF)
BOOL __stdcall D3DHALCreateDriver( NT9XDEVICEDATA *, LPD3DHAL_GLOBALDRIVERDATA *, LPD3DHAL_CALLBACKS *, DDHAL_DDEXEBUFCALLBACKS *);
#else
BOOL __stdcall D3DHALCreateDriver( NT9XDEVICEDATA *, LPD3DHAL_GLOBALDRIVERDATA *, LPD3DHAL_CALLBACKS * );
#endif
#endif
BOOL __stdcall DrvEnableDirectDraw( DHPDEV, DDHAL_DDCALLBACKS *, DDHAL_DDSURFACECALLBACKS *, DDHAL_DDPALETTECALLBACKS * );
BOOL __stdcall DrvGetDirectDrawInfo( DHPDEV, DDHALINFO *, DWORD *, VIDEOMEMORY *, DWORD *, DWORD * );
BOOL __stdcall fxinit( NT9XDEVICEDATA *, BOOL );

/*-------------------------------------------------------------------------
 *  ddinit32.c
 *-------------------------------------------------------------------------
 */
#if defined(WINNT) &&( _WIN32_WINNT == 0x0400)
#else
DWORD __stdcall DdGetDriverInfo( LPDDHAL_GETDRIVERINFODATA );
#endif

char *ddgetenv( NT9XDEVICEDATA *, const char * );

/*-------------------------------------------------------------------------
 *  ddmemmgr.c
 *-------------------------------------------------------------------------
 */

FLATPTR WINAPI DDHAL32_VidMemAlloc( LPDDRAWI_DIRECTDRAW_GBL, int, DWORD, DWORD );
void    WINAPI DDHAL32_VidMemFree( LPDDRAWI_DIRECTDRAW_GBL, int, FLATPTR );

#ifdef WINNT

HRESULT memMgr_allocSurface( PDEV *, DWORD type, DWORD, DWORD, DWORD, DWORD, DWORD *, DWORD *, DWORD *, DWORD *, DWORD *, VIDEOMEMORY ** );
HRESULT memMgr_allocSecondary( PDEV *, DWORD type, DWORD, DWORD, DWORD, DWORD, DWORD *, DWORD *, DWORD *, DWORD *, DWORD *, VIDEOMEMORY ** );
void memMgr_freeSurface( PDEV *, DWORD, DWORD, DWORD, DWORD, DWORD, VIDEOMEMORY * );

#else

DWORD memMgr_allocSurface( LPDDRAWI_DIRECTDRAW_GBL, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD *, DWORD *, DWORD *, DWORD *, DWORD * );
DWORD memMgr_allocSecondary( LPDDRAWI_DIRECTDRAW_GBL, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD *, DWORD *, DWORD *, DWORD *, DWORD * );
void memMgr_freeSurface( LPDDRAWI_DIRECTDRAW_GBL, DWORD, DWORD, DWORD, DWORD, DWORD );

DWORD HwPtrToLfbPtr(NT9XDEVICEDATA *, DWORD);
DWORD LfbPtrToHwPtr(NT9XDEVICEDATA *, DWORD, DWORD);
DWORD AdjustHwPtr(NT9XDEVICEDATA *, DWORD);
DWORD AdjustLfbPtr(NT9XDEVICEDATA *, DWORD, DWORD *);

#endif  // ifdef WINNT

/*-------------------------------------------------------------------------
 *  ddovl32.c
 *-------------------------------------------------------------------------
 */
VOID CanCreateOverlaySurface(NT9XDEVICEDATA *, LPDDHAL_CANCREATESURFACEDATA,
                                DWORD , DWORD);
DWORD CreateOverlaySurface(NT9XDEVICEDATA *,LPDDHAL_CREATESURFACEDATA,
                LPDDRAWI_DDRAWSURFACE_LCL ,FXSURFACEDATA *,DWORD,DWORD);
VOID DestroyOverlaySurface(NT9XDEVICEDATA *,LPDDRAWI_DDRAWSURFACE_LCL,
                           LPDDRAWI_DDRAWSURFACE_GBL,FXSURFACEDATA*);
VOID UnlockOverlaySurface(NT9XDEVICEDATA * ,LPDDHAL_UNLOCKDATA);
VOID   FlipOverlaySurface(NT9XDEVICEDATA *,LPDDHAL_FLIPDATA,
                          FXSURFACEDATA *, FXSURFACEDATA *, DWORD *);
DWORD Get_ScalingFactor( DWORD, DWORD, int );
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
void  SetBltParams( NT9XDEVICEDATA *, LPDDRAWI_DDRAWSURFACE_LCL );
DWORD ShrinkOverlaySurface( NT9XDEVICEDATA *, LPDDRAWI_DDRAWSURFACE_LCL );
void shrinkTexture(NT9XDEVICEDATA*,DWORD,DWORD,DWORD,DWORD, DWORD ,
                 DWORD,DWORD,DWORD,DWORD);

#endif
DWORD __stdcall SetOverlayPosition32( LPDDHAL_SETOVERLAYPOSITIONDATA );
DWORD __stdcall UpdateOverlay32( LPDDHAL_UPDATEOVERLAYDATA );


/*-------------------------------------------------------------------------
 *  ddsurf.c
 *-------------------------------------------------------------------------
 */
DWORD __stdcall DdAddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA );
DWORD __stdcall DdCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA );
DWORD __stdcall DdCreateSurface( LPDDHAL_CREATESURFACEDATA );
DWORD __stdcall DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA );
DWORD __stdcall DdLock( LPDDHAL_LOCKDATA );
DWORD __stdcall DdUnlock( LPDDHAL_UNLOCKDATA );
DWORD __stdcall DdSetSurfaceColorKey( LPDDHAL_SETCOLORKEYDATA );

HRESULT FXGETBUSYSTATUS( NT9XDEVICEDATA * );
/*-------------------------------------------------------------------------
 *  ddtrace.c
 *-------------------------------------------------------------------------
 */
void ddDebugPrint( LPSTR, ... );
#ifndef FXTRACE
    #define Msg           1 ?( void)0 :( void)
#else

void __cdecl Msg( NT9XDEVICEDATA *, int, LPSTR, ...  );

void DUMP_BLTDATA( NT9XDEVICEDATA *, int, LPDDHAL_BLTDATA );
void DUMP_BLTFLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_BLTFX( NT9XDEVICEDATA *, int, LPDDBLTFX );
void DUMP_BLTROPFLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_CREATEPALETTEDATA( NT9XDEVICEDATA *, int, LPDDHAL_CREATEPALETTEDATA );
void DUMP_DDCAPS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDCOLORKEY( NT9XDEVICEDATA *, int, DDCOLORKEY );
void DUMP_DDFXALPHACAPS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDFXCAPS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDHAL( NT9XDEVICEDATA *, int, LPDDHALINFO );
void DUMP_DDHAL_DESTROYSURFACEDATA( NT9XDEVICEDATA *, int, LPDDHAL_DESTROYSURFACEDATA );
void DUMP_DDHAL_FLIPDATA( NT9XDEVICEDATA *, int, LPDDHAL_FLIPDATA );
void DUMP_DDHAL_GETBLTSTATUSDATA( NT9XDEVICEDATA *, int, LPDDHAL_GETBLTSTATUSDATA );
void DUMP_DDHAL_SETMODEDATA( NT9XDEVICEDATA *, int, LPDDHAL_SETMODEDATA );
void DUMP_DDRAWIGBL_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDRAWIPAL_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDRAWISURFGBL_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDRAWISURFLCL_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDRAWPALETTE_GBL( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWPALETTE_GBL );
void DUMP_DDRAWSURFACE_GBL( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWSURFACE_GBL );
void DUMP_DDRAWSURFACE_LCL( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWSURFACE_LCL );
void DUMP_DDSCAPS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DDSD_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_DIRECTDRAW_GBL( NT9XDEVICEDATA *, int, LPDDRAWI_DIRECTDRAW_GBL );
void DUMP_DRAWCLIPPER_INT( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWCLIPPER_INT );
void DUMP_DRAWPALETTE_INT( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWPALETTE_INT );
void DUMP_DRAWSURFACE_INT( NT9XDEVICEDATA *, int, LPDDRAWI_DDRAWSURFACE_INT );
void DUMP_GETFLIPSTATUSDATA( NT9XDEVICEDATA *, int, LPDDHAL_GETFLIPSTATUSDATA );
void DUMP_GETSCANLINE( NT9XDEVICEDATA *, int, LPDDHAL_GETSCANLINEDATA );
void DUMP_LOCK_FLAGS( NT9XDEVICEDATA *, int, DWORD );
void DUMP_LOCKDATA( NT9XDEVICEDATA *, int, LPDDHAL_LOCKDATA );
void DUMP_PIXELFORMAT( NT9XDEVICEDATA *, int, DDPIXELFORMAT );
void DUMP_RECTL( NT9XDEVICEDATA *, int, LPRECTL );
void DUMP_SETSURFACECOLORKEY32( NT9XDEVICEDATA *, int, LPDDHAL_SETCOLORKEYDATA );
void DUMP_SURFACEDESC( NT9XDEVICEDATA *, int, LPDDSURFACEDESC );
void DUMP_VIDMEM( NT9XDEVICEDATA *, int, LPVIDMEM );
void DUMP_VIDMEMINFO( NT9XDEVICEDATA *, int, LPVIDMEMINFO );
void DUMP_WAITFORVERTICALBLANKDATA( NT9XDEVICEDATA *, int, LPDDHAL_WAITFORVERTICALBLANKDATA );

void SSTSTAT( NT9XDEVICEDATA *, DWORD * );

#endif

/*-------------------------------------------------------------------------
 *  ddvpe32.c
 *-------------------------------------------------------------------------
 */
#if ENABLE_VIDEOPORT
void H3_VMI_SetDecimation( NT9XDEVICEDATA *, RECT *,RECT * );
#endif

BOOL H3_Calculate_Bandwidth( NT9XDEVICEDATA *, RECT *, RECT * ,BOOL );
void H3_InitBandwidth( NT9XDEVICEDATA *  );
void H3_InitializeAll( NT9XDEVICEDATA *  );
void H3_VMI_Disable( NT9XDEVICEDATA * );
void H3_VMI_Enable_Func( NT9XDEVICEDATA * );
void H3_VMI_Reset( NT9XDEVICEDATA * );
void H3_VMI_SetDecimation( NT9XDEVICEDATA *, RECT *, RECT * );
DWORD  IOControl(NT9XDEVICEDATA * , DWORD , DWORD , DWORD );
DWORD __stdcall kSyncSurfaceData32( LPDDHAL_SYNCSURFACEDATA );
DWORD __stdcall kSyncVideoportData32( LPDDHAL_SYNCVIDEOPORTDATA );
DWORD __stdcall ovlColorControl32( LPDDHAL_COLORCONTROLDATA );
DWORD __stdcall vpCanCreateVideoPort32( LPDDHAL_CANCREATEVPORTDATA );
DWORD __stdcall vpColorControl32( LPDDHAL_VPORTCOLORDATA );
DWORD __stdcall vpCreateVideoPort32( LPDDHAL_CREATEVPORTDATA );
DWORD __stdcall vpDestroyVideoPort32( LPDDHAL_DESTROYVPORTDATA );
DWORD __stdcall vpFlipVideoPort32( LPDDHAL_FLIPVPORTDATA );
DWORD __stdcall vpGetVideoPortBandwidth32( LPDDHAL_GETVPORTBANDWIDTHDATA );
DWORD __stdcall vpGetVideoPortConnectInfo32( LPDDHAL_GETVPORTCONNECTDATA );
DWORD __stdcall vpGetVideoPortField32( LPDDHAL_GETVPORTFIELDDATA );
DWORD __stdcall vpGetVideoPortFlipstatus32( LPDDHAL_GETVPORTFLIPSTATUSDATA );
DWORD __stdcall vpGetVideoPortInputFormats32( LPDDHAL_GETVPORTINPUTFORMATDATA );
DWORD __stdcall vpGetVideoPortLine32( LPDDHAL_GETVPORTLINEDATA );
DWORD __stdcall vpGetVideoPortOutputFormats32( LPDDHAL_GETVPORTOUTPUTFORMATDATA );
DWORD __stdcall vpGetVideoSignalStatus32( LPDDHAL_GETVPORTSIGNALDATA );
DWORD __stdcall vpUpdateVideoPort32( LPDDHAL_UPDATEVPORTDATA );
DWORD __stdcall vpWaitForVideoPortSync32( LPDDHAL_WAITFORVPORTSYNCDATA );

/*-------------------------------------------------------------------------
 *  i2cfx.c
 *-------------------------------------------------------------------------
 */
#ifndef WINNT

PSTR GETREGPTR( NT9XDEVICEDATA *, BYTE, BYTE );
int  I2C_ACK( NT9XDEVICEDATA * );
BYTE I2C_IN( NT9XDEVICEDATA *, BYTE );
void I2C_INIT( NT9XDEVICEDATA * );
int  I2C_OUT( NT9XDEVICEDATA *, BYTE, BYTE );
void I2C_REL( NT9XDEVICEDATA *, BYTE );
void I2C_SENDACK( NT9XDEVICEDATA * );
void I2C_SENDNACK( NT9XDEVICEDATA * );
void I2C_START( NT9XDEVICEDATA * );
void I2C_STOP( NT9XDEVICEDATA * );
int  I2C_WBYTE( NT9XDEVICEDATA *, BYTE );
void I2CDEV_CLOSE( NT9XDEVICEDATA *, BYTE );
int  I2CDEV_INIT( NT9XDEVICEDATA *, I2C_DEV * );
BYTE READI2CDATA( NT9XDEVICEDATA *, BYTE, BYTE, BYTE, BYTE * );
BYTE READI2CREGCACHE( NT9XDEVICEDATA *, BYTE, BYTE );
BYTE READI2CSTATUS( NT9XDEVICEDATA *, BYTE );
void WRITEI2CDATA( NT9XDEVICEDATA *, BYTE, BYTE, BYTE * );
void WRITEI2CREG( NT9XDEVICEDATA *, BYTE, BYTE, BYTE );

#endif

/*-------------------------------------------------------------------------
 *  memmgr32.c
 *-------------------------------------------------------------------------
 */
#ifndef WINNT

void __cdecl QT_Thunk( void );

DWORD mmEvict32( NT9XDEVICEDATA *, DWORD, DWORD, DWORD );
DWORD mmReclaim32( NT9XDEVICEDATA *, DWORD );

#endif

/*-------------------------------------------------------------------------
 *  runtime.c
 *-------------------------------------------------------------------------
 */
#ifndef WINNT

float   dd_itof_inv( int );
double  ddatof( const char * );
int     ddatoi( const char * );
unsigned long ddftol( float );
int     dditoa( int, char * );
float   ddpow( double, double );
int     ddsscanf( const char *, const char *, FxU32 * );
int     ddxtoa( DWORD, char * );
int     ddxtoi( DWORD );
float   int_exp( float, int );
float   MY_LN( float );
float   pow_4_R( float, float );
BOOL    regGetDouble( HKEY, LPSTR, double * );
BOOL    regGetDword( HKEY, LPSTR, LPDWORD );

#endif

