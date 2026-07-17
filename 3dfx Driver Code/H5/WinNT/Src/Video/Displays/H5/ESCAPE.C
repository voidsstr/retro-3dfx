/******************************Module*Header*******************************\
* Module Name: escape.c
*
* Escape handler for Banshee drivers and other escapes.
*
* Copyright (c) 1996 Microsoft Corporation
* Copyright (c) 1997 3Dfx Interactive, Inc.
*
\**************************************************************************/

#include "precomp.h"
#include "qmodes.h"

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
#include "edgeesc.h"
#include "tv.h"
#include "fxtvout.h"
#endif //def TVOUT_SUPPORTED

//#define DFB_INSTRUMENTATION
#ifdef DFB_INSTRUMENTATION
#include "dfbinfo.h"
#endif

#define CSIM_TESTFILL         0x1234
#define CSIM_LOAD_SIMULATOR	0x1235

#define CSIM_SETREG		0x1240
#define CSIM_GETREG		0x1241


#ifdef CSIM

static BOOL
LoadCSIM( PDEV *ppdev )
{
    static BOOL  first = TRUE;

	ppdev->CSIM_LFBbase = ppdev->pjScreen;
	ppdev->CSIM_LFBlength = (ULONG) ppdev->cjBank;

	DISPDBG((1, "CSIMbase: %lx  CSIMlength: %lx",
	            ppdev->CSIM_LFBbase, ppdev->CSIM_LFBlength));

    if( first == TRUE )
    {
	    H3Hal_Dyn_Init();
							
	    csimInitDriver( ppdev->CSIM_LFBlength,
		    	(ULONG *) ppdev->CSIM_LFBbase,
			    (ULONG *) 0x10000000LU );

        first = FALSE;
    }

	return TRUE;

//ReturnFalse:

	//return FALSE;

}


static void
drawrect( PDEV *ppdev )
{
    BYTE*   pjH3Base = (BYTE *) ( 0x10000000L + SST_2D_OFFSET );

	//pattern alias 0
	SET_DW( pjH3Base, pattern0alias,0xaaaaccccL);
	//pattern alias 1
	SET_DW( pjH3Base, pattern1alias,0x55553333L);

	//color fore
	SET_DW( pjH3Base, colorFore, 0x001f001fl);
	//colorBack
	SET_DW( pjH3Base, colorBack, 0xf800f800l);

	//dst size
	SET_DW( pjH3Base, dstSize, 0x00300030l);
	//dstxy
	SET_DW( pjH3Base,  dstXY, ppdev->cxScreen - 0x31L );

	// rect fill
	SET_DW( pjH3Base, command,
		5L|
		BIT(8)|
		SSTG_MONO_PATTERN |
		SSTG_X_PATOFFSET |
		SSTG_Y_PATOFFSET |
		0xf0000000);
}
#endif

void SetInvariantReg( PDEV *ppdev)
{
	DWORD   dwtemp;
	BYTE*   pjH3Base = ppdev->pjH3Base;

	// CHECK_FIFO_ROOM(PH);
	SET_DW( pjH3Base, clip0min, 0L);
    #if ENABLE_LINEAR_DFBS
	SET_DW( pjH3Base, clip0max, ppdev->cyMemory << 16 | 0x0FFF );
    #else
	SET_DW( pjH3Base, clip0max, ppdev->cyMemory << 16 | ppdev->cxMemory );
    #endif

	// because the of the 15 bpp shift, tricks don't work
	// need to decode h3 bpp dstformat with switch
	switch( ppdev->cBitsPerPel )
	{
		case 8:
			dwtemp = SSTG_PIXFMT_8BPP;
		break;
		
		case 15:
			dwtemp = SSTG_PIXFMT_15BPP;
		break;

		case 16:
			dwtemp = SSTG_PIXFMT_16BPP;
		break;
		
		case 24:
			dwtemp = SSTG_PIXFMT_24BPP;
		break;
		
		case 32:
			dwtemp = SSTG_PIXFMT_32BPP;
		break;
	}
	
	ppdev->ulScreenFormat = dwtemp | ppdev->lDelta;
	SET_DW( pjH3Base, dstFormat, ppdev->ulScreenFormat );
	SET_DW( pjH3Base, srcFormat, ppdev->ulScreenFormat );

	SET_DW( pjH3Base, dstBaseAddr, 0L);
	SET_DW( pjH3Base, srcBaseAddr, 0L);
}
	
#ifdef CSIM

#ifdef MS_BUILD
extern void gdbg_set_debuglevel(const int level, const int value);
#endif

VOID
H5CSIMInit(PDEV *ppdev)
{
#if (_WIN32_WINNT >= 0x0500) && USE_D3D_CODE
  SETDW(ghw->chipMask, SST_CHIP_MASK_ALL_CHIPS);
#endif

#ifdef MS_BUILD
  gdbg_set_debuglevel(120,0);
#endif
}

BOOL bEnableCSIM (PDEV *ppdev)
{
  if( LoadCSIM(ppdev) == FALSE )
    return FALSE;

  H5CSIMInit(ppdev);

  return TRUE;
}


VOID vDisableCSIM (PDEV *ppdev)
{
}


//****************************************************************************
// ULONG DrvEscape(SURFOBJ *, ULONG, ULONG, VOID *, ULONG cjOut, VOID *pvOut)
//
// Driver escape entry point.  This function should return TRUE for any
// supported escapes in response to QUERYESCSUPPORT, and FALSE for any
// others.  All supported escapes are called from this routine.
//****************************************************************************

ULONG DrvEscape(SURFOBJ *pso, ULONG iEsc,
                ULONG cjIn, VOID *pvIn,
                ULONG cjOut, VOID *pvOut)
{
    PDEV*   ppdev;
	ULONG	retVal = (ULONG) FALSE;

	ppdev = (PDEV *)pso->dhpdev;

	if ( (cjIn >= sizeof(ULONG)) )	// All our stuff uses 1 ULONG arg.
	{
		switch ( iEsc )
		{
		case	QUERYESCSUPPORT:
			switch ( *(ULONG *)pvIn )
			{
				// These are the escapes we support
				case QUERYESCSUPPORT:
				case CSIM_TESTFILL:
				case CSIM_LOAD_SIMULATOR:
				case CSIM_SETREG:
				case CSIM_GETREG:
					retVal = (ULONG) TRUE;
			}
			break;

		case	CSIM_GETREG:	// Escapes that return information
			if( cjOut >= sizeof( ULONG ) )
			{
				PDEV	*ppdev;

				// Simulator must already be loaded.
				ppdev = (PDEV *)pso->dhpdev;
				if( ppdev->CSIM_LFBbase == NULL )
						break;		// return FALSE

				// Return the value of the register passed in pvIn.
				*(ULONG *)pvOut = GET_ABSOLUTE( *(ULONG *)pvIn );
				retVal = (ULONG) TRUE;
			}
			break;

        case EXT_HWC:
        case EXT_HWC_OLD:
            return hwcExt((DWORD *) pvIn, (DWORD *) pvOut, ppdev);
            break;

       default:
			retVal = (ULONG) FALSE;

		} // switch iEsc
	}

    return retVal;
}



ULONG DrvDrawEscape(SURFOBJ *pso, ULONG iEsc,
                CLIPOBJ *pco, RECTL *prcl,
                ULONG cjIn, VOID *pvIn )
{
    PDEV*   ppdev;
    ULONG*  pulH3Base;

    GWH_DECL;

	// Load simulator if we haven't already.
	ppdev = (PDEV *)pso->dhpdev;
	if( ppdev->CSIM_LFBbase == NULL )
		if( LoadCSIM(ppdev) == TRUE )
			SetInvariantReg( ppdev );
		else
			return (ULONG) FALSE;

#ifdef CSIM
    pulH3Base = (ULONG *) (0x10000000L + SST_2D_OFFSET);
#else
    pulH3Base = (ULONG *) ppdev->pjH3Base;
#endif

	switch ( iEsc )
    {

   case  CSIM_LOAD_SIMULATOR:
        return (ULONG) TRUE;

	case	CSIM_TESTFILL:
		drawrect( ppdev );
        return (ULONG) TRUE;

	case	CSIM_SETREG:
		// Set register.  Simulator already loaded above.
		if (  cjIn >= (2*sizeof(ULONG))  )
		{
            GWH_PROLOG;

            CHECK_FIFO_ROOM(ppdev, 1);
            GWH_BEGIN_PKT1_PACKET(1 , SSTCP_PKT1_OFFSETTOREG(*(ULONG *)pvIn), SSTCP_PKT1_NOINC);
//			SET_ABSOLUTE( *(ULONG *)pvIn, ((ULONG *)pvIn)[1] );
			SET_ABSOLUTE( pulH3Base + *(ULONG *)pvIn, ((ULONG *)pvIn)[1] );
			GWH_END_PKT1_PACKET( 1 );

			GWH_EPILOG;

			return (ULONG) TRUE;
		}

	default:
        return (ULONG) FALSE;
		
    }

    return (ULONG) FALSE;
}
#else

//****************************************************************************
// ULONG DrvEscape(SURFOBJ *, ULONG, ULONG, VOID *, ULONG cjOut, VOID *pvOut)
//
// Driver escape entry point.  This function should return TRUE for any
// supported escapes in response to QUERYESCSUPPORT, and FALSE for any
// others.  All supported escapes are called from this routine.
//****************************************************************************

ULONG DrvEscape(SURFOBJ *pso, ULONG iEsc,
                ULONG cjIn, VOID *pvIn,
                ULONG cjOut, VOID *pvOut)
{
  PDEV*   ppdev;
  ULONG	retVal = (ULONG) FALSE;
#ifdef TVOUT_SUPPORTED
  LPVIDEOPARAMETERS lpVidParams;
  int		ii;
#endif //def TVOUT_SUPPORTED

  ppdev = (PDEV *)pso->dhpdev;

#ifdef TVOUT_SUPPORTED
	if	(EdgeEscape((int) iEsc, pvIn, pvOut, ppdev))
	{
		return TRUE;
	}
#endif //def TVOUT_SUPPORTED

  switch ( iEsc )
  {
    case QUERYESCSUPPORT:
		  switch ( *(ULONG *)pvIn )
		  {
		    // These are the escapes we support
            case QUERYESCSUPPORT:
            case QUERYESCMODE:

#ifdef  OPENGL_ICD
            case OPENGL_GETINFO:
#endif  // OPENGL_ICD

            case EXT_HWC:
            case EXT_HWC_OLD:
            case EXT_HWC_SHARE_CPUTYPE:
#ifdef DFB_INSTRUMENTATION
            case DFBINFO_COMMAND:
#endif
#ifdef TVOUT_SUPPORTED
            case VIDEO_PARAMETERS:
#endif //def TVOUT_SUPPORTED
                retVal = (ULONG) TRUE;
                break;

#ifdef  OPENGL_ICD
            case OPENGL_CMD:
                retVal = (ULONG) FALSE; // For now, we don't support this mechanism for OpenGL
                                        // to escape into the driver, but later we might.
                break;
#endif  // OPENGL_ICD

        }
        break;

#ifdef  OPENGL_ICD

      case OPENGL_GETINFO:
         if (((POPENGLGETINFO) pvIn)->ulSubEsc == OPENGL_GETINFO_DRVNAME )
         {
            PGLDRVNAMERET pOut      = (PGLDRVNAMERET) pvOut;

            pOut->ulVersion         = GL_DRV_VERSION;
            pOut->ulDriverVersion   = GL_DRV_DRIVER_VERSION;

            RtlCopyMemory( pOut->awch, GL_DRV_NAME, GL_DRV_NAME_LEN );

            return TRUE;
         }
         else
         {
            return FALSE;
         }
         break;

      case OPENGL_CMD:
         return FALSE;  // No support for OpenGL escapes yet.
         break;

#endif  // OPENGL_ICD

    case EXT_HWC:
    case EXT_HWC_OLD:
      return hwcExt((DWORD *) pvIn, (DWORD *) pvOut, ppdev);

    case EXT_HWC_SHARE_CPUTYPE:
      return hwcShareCPUType((DWORD *) pvIn, (DWORD *) pvOut, ppdev);

    case QUERYESCMODE:
      return QueryMode(ppdev, pvIn, pvOut);

#ifdef DFB_INSTRUMENTATION
    case DFBINFO_COMMAND:
      {
        DFBINFOCMD  *pDfbCmd;

        pDfbCmd = pvIn;
        if (pDfbCmd->dwCommand == DFBINFO_CLEAR)
        {
          memset(&DfbInfo, 0, sizeof(DFBINFO));
          return TRUE;
        }
        else if (pDfbCmd->dwCommand == DFBINFO_GET)
        {
          memcpy(pvOut, &DfbInfo, sizeof(DFBINFO));
          return TRUE;
        }
        return FALSE;
      }
#endif

#ifdef TVOUT_SUPPORTED
     case VIDEO_PARAMETERS:
      {
	    if (pvOut==NULL)
		  {
		  // workaround common error case where InterVideo, Inc.'s WinDVD passes us only an input buffer pointer
          pvOut = pvIn;
		  }
        ii = TVOutVideoParameters (ppdev, (LPVIDEOPARAMETERS)pvIn, (LPVIDEOPARAMETERS)pvOut);
        if (ii >= 0)
          return (TRUE);
        lpVidParams = (LPVIDEOPARAMETERS)pvOut;
        if ( lpVidParams->dwCommand == VP_COMMAND_GET )
        {
          lpVidParams->dwFlags = 0;
          lpVidParams->dwMode = 0;
          lpVidParams->dwTVStandard = 0;
          lpVidParams->dwAvailableModes = 0;
          lpVidParams->dwAvailableTVStandard = 0;
          lpVidParams->dwFlickerFilter = 0;
          lpVidParams->dwOverScanX = 0;
          lpVidParams->dwOverScanY = 0;
          lpVidParams->dwMaxUnscaledX = 0;
          lpVidParams->dwMaxUnscaledY = 0;
          lpVidParams->dwPositionX = 0;
          lpVidParams->dwPositionY = 0;
          lpVidParams->dwBrightness = 0;
          lpVidParams->dwContrast = 0;
          lpVidParams->dwCPType = 0;
          lpVidParams->dwCPCommand = 0;
          lpVidParams->dwCPStandard = 0;
          lpVidParams->dwCPKey = 0;
          lpVidParams->bCP_APSTriggerBits = 0;
          memset (lpVidParams->bOEMCopyProtection, 0,
          sizeof(lpVidParams->bOEMCopyProtection));
        }
      return TRUE;
      }
#endif //def TVOUT_SUPPORTED

    default:
      retVal = (ULONG) FALSE;

	} // switch iEsc

    return retVal;
}

#endif //CSIM
