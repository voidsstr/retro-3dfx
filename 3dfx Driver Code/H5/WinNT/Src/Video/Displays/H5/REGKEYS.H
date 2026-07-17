/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 14$
** $Date: 10/11/00 8:57:23 PM$
** $Log: 
**  14   3dfx      1.12.3.0    10/11/00 Brent           Forced check in to enforce
**       branching.
**  13   3dfx      1.12        04/25/00 Andrew Sobczyk  Added toggle key value read
**       from registry
**  12   3dfx      1.11        04/20/00 Matt McClure    Added Registry key for
**       Z_ACCESS_OPT.
**  11   3dfx      1.10        03/20/00 Bob Seitsinger  Added #define LOD_BIAS.
**  10   3dfx      1.9         03/13/00 Bob Seitsinger  Added DITHMAT_FORCEFLAG and
**       AAJITTER_FORCEFLAG.
**  9    3dfx      1.8         03/11/00 Andrew Sobczyk  Added some defines
**  8    3dfx      1.7         03/02/00 Andrew Sobczyk  Added SLI_BAND_HEIGHT
**       override
**  7    3dfx      1.6         02/22/00 Christopher Wilcox Check in code to
**       silently fail AA enable when secondary buffers do not exist in !SLI modes.
**        Fixes PRS 12659.
**  6    3dfx      1.5         12/14/99 Christopher Wilcox Changed to using
**       SLI_AA_CONFIGURATION registry variable to control SLI and AA
**       configuration.
** 
**  5    3dfx      1.4         12/01/99 Bob Seitsinger  Changes to apply default AA
**       jitter values (as suggested by Gary T.) appropriately.
**  4    3dfx      1.3         10/27/99 Bob Seitsinger  Additional rotated dither
**       matrix changes.
**  3    3dfx      1.2         10/20/99 Bob Seitsinger  Rotated dither matrix
**       selection fogMode register values clean up.
**  2    3dfx      1.1         09/16/99 Bob Seitsinger  Dither matrix rotate
**       changes. Add DITHER_ROTATION registry key, and rename existing dither
**       matrix rotation and antialiasing jitter registry key names to shorten
**       them.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 7     8/20/99 10:57a Andrew
** Changed DIGITAL_SLI_AA to drop _NAME from registry string
** 
** 6     8/09/99 10:29a Cwilcox
** Added ENABLE_AA_NAME and DISABLE_SLI_NAME.
** 
** 5     7/16/99 6:23p Bseitsin
** Constants for new registry keys.
** 
** 4     7/09/99 8:22a Andrew
** Added three new SLI AA Keys
** 
** 3     7/08/99 10:16a Tlittle
** Minor change for run-time checking of H5
** 
** 2     6/14/99 3:15p Bseitsin
** Triangle Iterator Column Band Control support.
** 
** 1     6/02/99 6:52a Michael
** Branch from h3
** 
** 12    5/10/99 2:05p Stb_srogers
** Adding Video Filter and Alpha Dither Subtraction quality updates. Fixes
** PRS 5442
** 
** 11    4/09/99 12:38p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 10    12/03/98 6:02p Martin
** Add 3D Studio Max registry variable, and setup texture formats if
** turned on.
** 
** 9     11/24/98 5:56p Martin
** Disable trilinear mipmapping, and
** Disable unsupported blend modes
** 
** 8     11/07/98 7:44a Russ
** added DDRAW_ENABLED define
**
** 7     10/16/98 4:29p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 6     10/06/98 2:03p Miriam
** Automipmap support
**
** 5     9/16/98 5:08p Miriam
** Comment registry key and clarify which key is used by Voodoo2 & which
** key is used by Banshee.
**
*/

#ifndef __REGKEYS_H__
#define __REGKEYS_H__

// Value prefixed to all registry key names.
#define RK_PREFIX                          "SSTH3_"

//////////////////////////////////////////////////////////////////////////////////////////////
// R E G I S T R Y    K E Y     N A M E S 
//

// 0=disable AA
// 1=enable edge AA
// 2=enable super sampling AA
#define ANTIALIAS                          RK_PREFIX"ANTIALIAS"

// This is for internal use only.
// Size of the antialias buffer allocated for edge antialiasing
#define ANTIALIAS_BUFFER                   RK_PREFIX"ANTIALIAS_BUFFER"

// 1=pixel center at 0.5,0.5
// 0=pixel center at 0.0,0.0
#define PIXELCENTER                        RK_PREFIX"PIXELCENTER"

// 0=Swap Buffer without waiting for vertical retrace
// 1-256= swap buffer after this number of vertical retraces
#define SWAPINTERVAL                       RK_PREFIX"SWAPINTERVAL"

// 0=Disable automipmapping
// 1=Automatically generate mipmaps
#define AUTOMIPMAP                         RK_PREFIX"AUTOMIPMAP"

// 0: Means it's not in registry, defaults to 1
// 1: Optimal- Use 2X2 video filter for modes less than 1024x768
// 2: Normal- Use 4x1 video filter always
// 3: High- Uses 2X2 video filter always  (except when using 2:1 DAC mode)
#define OVERLAYMODE                        RK_PREFIX"OVERLAYMODE"

// 0: Means it's not in registry, defaults to 1
// 1: Optimal (default)- No dither subtraction
// 2: Sharper- Right now same as Optimal
// 3: Smoother- Enables Alpha Dither Subtraction (BIT 19 in fbzMode)
#define ALPHADITHERMODE                    RK_PREFIX"ALPHADITHERMODE"

// no longer supported - should we delete this from this file?
#define LDELAY                             RK_PREFIX"LDELAY"

// Gamma settings
#define RED_NAME                           RK_PREFIX"RGAMMA"
#define BLUE_NAME                          RK_PREFIX"BGAMMA"
#define GREEN_NAME                         RK_PREFIX"GGAMMA"

// 0 = default = No 3D Studio Max support
#define STUDIOMAX                          RK_PREFIX"3DSMAX2.5"

// Triangle Iterator Column Band Control
// 0=column-of-8
// 1=column-of-16
// 2=column-of-32
// 3=column-of-4
#define CBC                                RK_PREFIX"CBC"

// 0=Disable 2 pixels-per-clock rendering
// 1=Enable 2 pixels-per-clock rendering
#define TWOPPC                             RK_PREFIX"2PPC"

// The log2 of the number of scanlines rendered by each texture unit when
// 2 pixels-per-clock rendering is enabled.
#define TWOPPC_LOG2_BAND_HEIGHT            RK_PREFIX"2PPC_LOG2_BAND_HEIGHT"

// 0=Disable Guardband Clipping
// 1=Enable Guardband Clipping#define GUARDBAND_CLIPPING RK_PREFIX"GUARDBAND_CLIPPING"
#define GUARDBAND_CLIPPING                 RK_PREFIX"GUARDBAND_CLIPPING"

// 0=Disable Dither Rotation
// 1=Enable Dither Rotation
#define DITHER_ROTATION                     RK_PREFIX"DITHER_ROTATION"

// Dither Rotation matrix selection values
// Specific to each chip. For 16bit AA only.

// Force Dither Rotation Matrix values to be:
// 0 - Force capability disabled. Follow standard behavior - Use registry entries, if present, else defaults.
// 1 - Force usage of the defaults, regardless of entries in Registry.
// 2 - Force all values to zero, regardless of entries in Registry.
#define DITHMAT_FORCEFLAG                  RK_PREFIX"DITHMAT_FORCEFLAG"

// 1 Chip 2 Sample
#define DITHMATSEL_2SMPL                   RK_PREFIX"DITHMATSEL_2SMPL"
#define DITHMATSELABLND_2SMPL              RK_PREFIX"DITHMATSELABLND_2SMPL"
#define DITHMATSELAA_2SMPL                 RK_PREFIX"DITHMATSELAA_2SMPL"
#define DITHMATSELABLNDAA_2SMPL            RK_PREFIX"DITHMATSELABLNDAA_2SMPL"

// 2 Chip 4 Sample
// Chip 0
#define DITHMATSEL_4SMPL_CHP0              RK_PREFIX"DITHMATSEL_4SMPL_CHP0"
#define DITHMATSELABLND_4SMPL_CHP0         RK_PREFIX"DITHMATSELABLND_4SMPL_CHP0"
#define DITHMATSELAA_4SMPL_CHP0            RK_PREFIX"DITHMATSELAA_4SMPL_CHP0"
#define DITHMATSELABLNDAA_4SMPL_CHP0       RK_PREFIX"DITHMATSELABLNDAA_4SMPL_CHP0"
// Chip 1
#define DITHMATSEL_4SMPL_CHP1              RK_PREFIX"DITHMATSEL_4SMPL_CHP1"
#define DITHMATSELABLND_4SMPL_CHP1         RK_PREFIX"DITHMATSELABLND_4SMPL_CHP1"
#define DITHMATSELAA_4SMPL_CHP1            RK_PREFIX"DITHMATSELAA_4SMPL_CHP1"
#define DITHMATSELABLNDAA_4SMPL_CHP1       RK_PREFIX"DITHMATSELABLNDAA_4SMPL_CHP1"

// Antialias jitter values.

// Force Jitter values to be:
// 0 - Force capability disabled. Follow standard behavior - Use registry entries, if present, else defaults.
// 1 - Force usage of the defaults, regardless of entries in Registry.
// 2 - Force all values to zero, regardless of entries in Registry.
#define AAJITTER_FORCEFLAG                      RK_PREFIX"AAJITTER_FORCEFLAG"

// AA Jittery Toggle Key
// 0 - 255 ASCII Value
#define AAJITTER_TOGGLE_KEY                      RK_PREFIX"AAJITTER_TOGGLE_KEY"

/// Primary and secondary AA buffers. Specific to each chip.
// 1 Chip 2 Sample
#define PRI_BUF_VERTEX_OFFSET_X_2SMPL           RK_PREFIX"PRIBUFVTXOFFX_2SMPL"
#define PRI_BUF_VERTEX_OFFSET_Y_2SMPL           RK_PREFIX"PRIBUFVTXOFFY_2SMPL"
#define SEC_BUF_VERTEX_OFFSET_X_2SMPL           RK_PREFIX"SECBUFVTXOFFX_2SMPL"
#define SEC_BUF_VERTEX_OFFSET_Y_2SMPL           RK_PREFIX"SECBUFVTXOFFY_2SMPL"

// 2 Chip 4 Sample
// Chip 0
#define PRI_BUF_VERTEX_OFFSET_X_4SMPL_CHIP0     RK_PREFIX"PRIBUFVTXOFFX_4SMPL_CHP0"
#define PRI_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP0     RK_PREFIX"PRIBUFVTXOFFY_4SMPL_CHP0"
#define SEC_BUF_VERTEX_OFFSET_X_4SMPL_CHIP0     RK_PREFIX"SECBUFVTXOFFX_4SMPL_CHP0"
#define SEC_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP0     RK_PREFIX"SECBUFVTXOFFY_4SMPL_CHP0"
// Chip 1
#define PRI_BUF_VERTEX_OFFSET_X_4SMPL_CHIP1     RK_PREFIX"PRIBUFVTXOFFX_4SMPL_CHP1"
#define PRI_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP1     RK_PREFIX"PRIBUFVTXOFFY_4SMPL_CHP1"
#define SEC_BUF_VERTEX_OFFSET_X_4SMPL_CHIP1     RK_PREFIX"SECBUFVTXOFFX_4SMPL_CHP1"
#define SEC_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP1     RK_PREFIX"SECBUFVTXOFFY_4SMPL_CHP1"

// For NT only!
// 0=Disable DirectDraw
// 1=Enable DirectDraw
#define DDRAW_ENABLED                      RK_PREFIX"DDRAW_ENABLED"


#ifdef SLI_AA
 #define SLI_AA_CONFIG_NAME                RK_PREFIX"SLI_AA_CONFIGURATION"
 #define AA_ENABLE_OUTOFMEMORY_NAME        RK_PREFIX"AA_ENABLE_OUTOFMEMORY"
 #define DIGITAL_SLI_AA_NAME               RK_PREFIX"DIGITAL_SLI_AA"
 #define SLI_AA_VIDEO_FORMAT_DIGITAL       (0)
 #define SLI_AA_VIDEO_FORMAT_ANALOG        (1)
 #define SLI_BAND_HEIGHT_NAME              RK_PREFIX"SLI_BAND_HEIGHT"
 #define SLI_BAND_HEIGHT_OVERRIDE_NAME     RK_PREFIX"SLI_BAND_HEIGHT_OVERRIDE"
 #define SWAPBUFFER_ALGO_NAME              RK_PREFIX"SWAPBUFFER_ALGO"
#endif

// Texture LOD bias
#define LOD_BIAS                           RK_PREFIX"LOD_BIAS"

/////////////////////////////////////////////////////////////////////////////////////////
// D E B U G G I N G    K E Y S
// Keys specifically for debugging. Not intended to be used in retail drivers.

// Used to set debug output in debugging drivers.
#define DDDEBUGLEVEL                       "DD_DebugLevel"
#define D3DDEBUGLEVEL                      "D3D_DebugLevel"

// 0=Disable 32bpp rendering
// 1=Enable 32bpp rendering
#define THIRTYTWOBPP_RENDERING             RK_PREFIX"32BPP_RENDERING"

#ifdef Z_ACCESS_OPT
#define Z_CLEAR_OPTIMIZATION               RK_PREFIX"Z_CLEAR_OPTIMIZATION"
#endif

#endif /* __REGKEYS_H__ */

