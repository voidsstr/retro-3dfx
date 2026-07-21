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
** File name:   d6dp2.c
**
** Description: DrawPrimitive2 implementation
**
** $Revision: 106$
** $Date: 10/31/00 2:00:24 AM$
**
** $Log: 
**  106  3dfx      1.86.1.5.1.1210/31/00 Johnny Trainor  Changes due to modifation
**       of StateBlock structure.
**  105  3dfx      1.86.1.5.1.1110/30/00 Johnny Trainor  Fixed a build error.
**  104  3dfx      1.86.1.5.1.1010/25/00 Johnny Trainor  Removed the global
**       variable vertexType. We now use the dwVertexType element of the RC
**       structure.
**  103  3dfx      1.86.1.5.1.910/13/00 Johnny Trainor  Modified the call to our
**       Clear function. We now pass a pointer to a D3DHAL_DP2CLEAR structure
**       rather than the values of the five individual elements.
**  102  3dfx      1.86.1.5.1.810/13/00 Johnny Trainor  #ifdefed some code that
**       only applies to DX7 builds
**  101  3dfx      1.86.1.5.1.710/11/00 Brent           Forced check in to enforce
**       branching.
**  100  3dfx      1.86.1.5.1.609/01/00 Leo Galway      Re-position #ifdef
**       ENABLE_ASM define so as #ELSE code path from NEWASMTRI == 1 is not used
**       when !ENABLE_ASM. 
**  99   3dfx      1.86.1.5.1.508/25/00 Russ Lind       Added debug output for
**       ignored RenderState and TextureStageState settings in state blocks.
**  98   3dfx      1.86.1.5.1.408/11/00 Russ Lind       merge of various w9x
**       changes into w2k
**       including
**         LODBIASPERCHIP
**         LODDITHER
**         MAXPENDINGBUFFERS
**         TEXTURE_REPEAT_NOT_SCALED
**         COLUMNBANDCONTROL_DEFAULT
**         SLI_ABOVE_1280
**         fix for z buffer problems in Shadow of the Empire
**         flat shaded specular fix
**         part of the Daytona/Napalm2 changes
**  97   3dfx      1.86.1.5.1.308/10/00 Russ Lind       Added a couple of defines
**       to make enabling/disabling the w2k error checking and the asm code
**       simpler.  The defaults are to enable the asm code and enable the w2k error
**       checking.
**  96   3dfx      1.86.1.5.1.207/19/00 Jonny Cochrane  Fix for PRS 14997 and 14904
**       - Support for TEXTURETRANSFORMFLAGS 2d texture coords
**  95   3dfx      1.86.1.5.1.107/10/00 Russ Lind       fix for PRS 14417, disable
**       zbuffering in bufferStateSetup when zEnable is 0 and zWriteEnable is 1
**  94   3dfx      1.86.1.5.1.006/29/00 Russ Lind       pull in guard band clipping
**       fix for DCT 350 Vertex Clipping from w9x
**       added comments about where the aW and bW equations come from in WINFO
**  93   3dfx      1.86.1.5    06/15/00 Edwin Wong      Check for null AABufferAddr
**       in BufferStateSetup(), fix Flanker 2.0 demo hang (use rev 12 of
**       ddmemmgr.c, i.e. do not force allocation of  Z to desktopsize, to
**       reproduce hang). Also, disable AA mode and rotation dither if AA buffers
**       are not allocated in HandleSLIAA(), except for modes that use backend
**       video to render AA, which do not require any AA buffers.
**  92   3dfx      1.86.1.4    06/06/00 Russ Lind       merge from w9x, mods to 4
**       chip support
**  91   3dfx      1.86.1.3    05/31/00 Russ Lind       changes from Leo Galway to
**       correctly handle fillMode == WIREFRAME in INDEXEDTRIANGLELIST &
**       TRIANGLEFAN_IMM cases in ddiDrawPrimitives2
**  90   3dfx      1.86.1.2    05/11/00 Russ Lind       merge of pRc->dwZeroJitter
**       init from w9x
**       disable use of asm code in INDEXEDTRIANGLELIST if w scaling
**       for NEWASMTRI disabled case, disable use of asm code in
**       INDEXEDTRIANGLELIST2 if w scaling (this doesn't have any effect since
**       NEWASMTRI is enabled)
**  89   3dfx      1.86.1.1    05/09/00 Russ Lind       to fix potential access
**       violation issues with NT Stress verify we have a valid PALHNDL for
**       palettized textures before dereferencing it
**  88   3dfx      1.86.1.0    05/08/00 Russ Lind       on w2k, use GET_HW_ADDR
**       instead of GET_HW_OFFSET
**  87   3dfx      1.86        04/25/00 Bob Seitsinger  In
**       ddiDrawPrimitives2/TriangleFan - set dwZeroJitter to 1 if we have a 4
**       vertex square. In HandleSLIAA, if dwZeroJitter == 1, write zero jitter
**       values, otherwise write values captured at init time.
**  86   3dfx      1.85        04/24/00 Matt McClure    Fixed problems with 3DMark
**       2000 demo test and Z_ACCESS_OPT
**  85   3dfx      1.84        04/24/00 Andrew Sobczyk  Added code to support Hot
**       Key
**  84   3dfx      1.83        04/20/00 Matt McClure    Simplified the Z Buffer
**       Optimization code.
**  83   3dfx      1.82        04/19/00 Matt McClure    Implementation of Z Buffer
**       Access Optimization.  Added code to support setup and reset of
**       optimization based on the amount of flips that have occurred without a Z
**       Buffer Clear.
**  82   3dfx      1.81        04/19/00 Andrew Sobczyk  Fixed the flag to work with
**       1,2, or 4 chips
**  81   3dfx      1.80        04/07/00 Andrew Sobczyk  New code paths for 2-chip
**       2-sample AA
**  80   3dfx      1.79        04/07/00 Russ Lind       remove a couple pRc !=
**       _D3(lastContext) comparisions, one in HandleSLIAA incorrectly enables
**       stenciling whenever the context changes and one in setDX6state is too late
**       since lastContext was already set to pRc if they weren't equal.  And
**       additional debug output.
**  79   3dfx      1.78        04/05/00 Andrew Sobczyk  Fixed a problem where the
**       chip mask was getting reset so that the first chip was missing register
**       writes
**  78   3dfx      1.77        04/04/00 Tim Little      Fixed a problem with
**       partial texture downloads not being handled correctly.  This problem was
**       exhibited in WinBench 2000.
**  77   3dfx      1.76        03/23/00 James Hunter    Fix for PRS#12663 (Roll
**       Cage 2, W-Buffering overflow) - Voodoo 3 specific.
**  76   3dfx      1.75        03/21/00 Andrew Sobczyk  Removed call to SliClear
**  75   3dfx      1.74        03/21/00 Chris W. Shaw    Init the 2nd chip when
**       entering SLI mode.  Previously the updating of some of the registers
**       depended only on the contents of the first chip.  If the 2nd chip was not
**       considered, then the 2nd chip would not always get updated when switching
**       to a SLI mode.
** 
**  74   3dfx      1.73        03/20/00 Jonny Cochrane  There is currently no
**       support for w scaling in atri.asm and afan.asm for w scaling (aW != 1.0f
**       and bW != 0.0f). This only applies to non KNI machines. 
** 
** 
**  73   3dfx      1.72        03/14/00 Russ Lind       fix for F22 Lightning 3
**       texture corruption (PRS 13133?) from Bob Holden of IGX
**  72   3dfx      1.71        03/13/00 Bob Seitsinger  Change debug level for
**       'terminating command stream' message from 0 to 5. Causes too much of a
**       slow down in softice when always displaying.
**  71   3dfx      1.70        03/10/00 Bob Seitsinger  Code to allow for the
**       validation of guardband clipping. Added to bufferStateSetup. Encased in
**       #ifdef DEBUG.
**  70   3dfx      1.69        03/10/00 Bob Seitsinger  Code to allow checking
**       seperate RGB channel wrties. Encased in #ifdef DEBUG.
**  69   3dfx      1.68        03/10/00 Christopher Wilcox Fixed problem with SLI
**       initialization of 3D registers.
** 
**  68   3dfx      1.67        03/01/00 Scott Kephart   SW T&L: Removed init of
**       guardband clip bit and moved it to tnlinit.c where it belonged in the
**       first place.
**  67   3dfx      1.66        03/01/00 Justin McCartney Moved flag
**       txtStageStateValidated from _D3 global structure into the RC structure.
**  66   3dfx      1.65        03/01/00 Justin McCartney Fix for PRS 12099 problem
**       with Unreal Tournament.  Added code to set flag
**       _D3(txtStageStateValidated) to FALSE if texture stage state is changed. 
**       Search for jmccartney 28/02/00
**  65   3dfx      1.64        02/25/00 Christopher Wilcox Fixed validation problem
**       where guardband clipping was not being updated when Direct3D rendering
**       context was lost.
**  64   3dfx      1.63        02/18/00 Andrew Sobczyk  Added a ifdef to stop
**       calling SliClear
**  63   3dfx      1.62        02/17/00 Christopher Wilcox Partial fix for PRS
**       12972, workaround for Napalm hardware problem with disabling stencil. 
**       Fixes Shadow Volumes and Shadow Volumes 2 SDK applications.
** 
**  62   3dfx      1.61        02/15/00 Andrew Sobczyk  Added call to SliClear
**       function
** 
**  61   3dfx      1.60        02/15/00 Bob Seitsinger  Minor change to 'if'
**       statement in the CLEAR: ddiDrawPrimitives2 case for when to use the 2D
**       blit clear function - changed to use it when SLI enabled only - no longer
**       checks for AA enabled.
**  60   3dfx      1.59        02/08/00 Matt McClure    Modified newasmtri check to
**       not draw triangles in the newasmtri fast path, 
**       if zEnable is true.  This is to avert theoretical specular buffer problem
**       with a triangle that is specular, but no z.  Also restored asm_update_rc
**       in setdx6state.  There was no reason why it couldn't be used instead of
**       the newasmtri modifications.
**  59   3dfx      1.58        02/04/00 Steve Houston   Added new asm triangle
**       routines by Matt McClure and Allen Hanson to W2K driver.
**       Added NEWASMTRI, STBKNI and K6_2 defines to W2K\build\stbperf.inc to
**       enable this code.
** 
**  58   3dfx      1.57        02/02/00 Christopher Wilcox Updated ZSCALE value in
**       kni_array to fix PRS 12543.
** 
**  57   3dfx      1.56        02/01/00 Chris W. Shaw   Fixed a PK1 comand to
**       target only the FBI PK1CHIP.
**  56   3dfx      1.55        01/28/00 Scott Kephart   Big T&L Merge: FVF handling
**       changes
**  55   3dfx      1.54        01/24/00 Matt McClure    Modification to fix Non
**       STBKNI or K6_2 builds
**  54   3dfx      1.53        01/24/00 Matt McClure    Added support for new KNI
**       and 3DNow triangle rendering routines.  New fast path only enabled if
**       !WINNT, K6_2, STBKNI==1 and NEWASMTRI==1
**  53   3dfx      1.52        01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  52   3dfx      1.51        01/17/00 Christopher Wilcox Fixed problem where
**       assembly code was using a 16bpp zscale value instead of 32bpp.
** 
**  51   3dfx      1.50        01/11/00 Scott Kephart   Reverted back to source
**       prior to new triangle assembly code.
**       This is the same as Revision 48 (1.47) from 1/5/00. The block Russ
**       reinserted into version 50 is in this version.
**  50   3dfx      1.49        01/11/00 Russ Lind       restore block in
**       INDEXTRIANGLELIST2 which was dropped with the NEWASMTRI changes
**       this is currently ifdef'd for WINNT since atrikni.asm probably isn't
**       compatible anymore for win9x
**       this restores the 3DWB performance drop
**  49   3dfx      1.48        01/10/00 Matt McClure    Added support for new
**       assembler triangle routines, #ifdef NEWASMTRI==1 surrounds all changes.
**  48   3dfx      1.47        01/05/00 Andrew Sobczyk  Moved AA setup to its own
**       function.  Added function call before Clear.  Fixed a problem with Jittery
**       values where they were not written to all chips.
**  47   3dfx      1.46        12/14/99 Matt McClure    Fix for PRS 11794.  Added
**       WRAPn functionality and fixed autostripping bug with shadow maps on 3D
**       Winbench 2k Test 6 - Stations.
**  46   3dfx      1.45        12/14/99 Russ Lind       fix for PRS 11174 - Alpha
**       Palette failures in dct 300
**       when chromakeying with palettized textures that contain alpha data, the hw
**       uses 6bit r,g,b values and replicates the 2 msb's into the 2 lsb's
**       the fix is, for this format, the sw needs to replicate the 2msb's into the
**       2 lsb's before writing to the chomakey register
**       plus some additional debug output in setDX6State
**  45   3dfx      1.44        12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  44   3dfx      1.43        12/10/99 Bob Seitsinger  Correct typo for non-dx7
**       builds in statement that retrieves the AABufferAddr in bufferStateSetup.
**       (pRc->DDS to pRc->lpDDS).
**  43   3dfx      1.42        12/09/99 Chris W. Shaw   Changed clipping to use
**       viewport bounds instead of rendertarget bounds for V3 and Napalm.
**  42   3dfx      1.41        12/06/99 Chris W. Shaw   Fixed a Napalm specific bug
**       for writting 12 NOPs on the transition from 2ppc mode to 1pcc mode.
**       Also, on Napalm when not texturing, we need to write texture mode because
**       Napalm sometimes uses the TCUs for getting iterated data to the CCU.  PRS
**       11235.
** 
**  41   3dfx      1.40        12/03/99 Christopher Wilcox Updated surface access
**       method for DirectX 7.
** 
**  40   3dfx      1.39        12/01/99 Bob Seitsinger  Changes to apply default
**       jitter values (as defined by Gary T.) appropriately.
**  39   3dfx      1.38        11/30/99 Bob Seitsinger  Disable use of the 2d blit
**       Clear function until problems are resolved. Use Fastfill for all Napalm
**       clears.
**  38   3dfx      1.37        11/30/99 Bob Seitsinger  In ddiDrawPrimitives2, for
**       the CLEAR case, add check for AA being disabled when determining whether
**       to use the Clear function or not.
**  37   3dfx      1.36        11/24/99 Bob Seitsinger  Add IS_NAPALM in
**       bufferStateSetup to 'if' statement for secondary buffers.
**  36   3dfx      1.35        11/23/99 Scott Kephart   Fixes for Vertex buffers
**  35   3dfx      1.34        11/17/99 Russ Lind       removed passing of
**       dwhContext to setDX6State & bufferStateSetup, it's redundant with pRc
**       removed passing dwhContext to pfnClear, it's redundant with pRc
**  34   3dfx      1.33        11/17/99 Russ Lind       partial fix for PC99
**       TextureSizes failures & partial fix for PC99 MultiTexture failures, flush
**       the texture cache in setDX6State
**       also added support for 0x184 vertexType (TEX1|SPECULAR|XYZRHW) in
**       UpdateCtxFVFChanges for 3DMark2000
**  33   3dfx      1.32        11/17/99 Bob Seitsinger  Modified DP2OP_CLEAR
**       command to reflect new parameter - DWORD dwhContext - for clear function
**       call. Pulled buffer address setup code out into function bufferStateSetup,
**       and called it from setDX6state.
**  32   3dfx      1.31        11/11/99 Russ Lind       Fix for texture colorkey
**       failures in dct263 on w2k napalm csim for 32bit textures.  Corrected
**       chroma key case for TEXFMT_ARGB_8888 in setDX6State.
**  31   3dfx      1.30        11/10/99 Scott Kephart   Added profiling changes for
**       T&L
**  30   3dfx      1.29        11/09/99 Chris W. Shaw   Enabled the legacy
**       textureblend code (0x7ffffffe) for win9x.
**  29   3dfx      1.28        11/08/99 Russ Lind       for W2K, modified TEXBLT to
**       handle how the DX7 runtime munges the surface width and height of system
**       memory DXTn surfaces
**  28   3dfx      1.27        11/04/99 Edwin Wong      Added __stdcall to the
**       typedef statement of PTEXBLTFUNC, this fixed D3D sample crashes on Win9x.
**  27   3dfx      1.26        11/04/99 Russ Lind       corrected my erroneous port
**       the of the Mip Filter ? - MaxMipLevel fix from V3_TOT
**       added some debug output in TEXBLT
**       set the SC_SOMETHING bit in pRc->hwStateChanged if TEXTURELOAD is used in
**       TEXBLT
**  26   3dfx      1.25        11/02/99 Russ Lind       fix for win9x build error,
**       add Blt32_CopyFourCC prototype for win9x
**  25   3dfx      1.24        11/02/99 Russ Lind       fix for DCT 262 Mip Filter
**       Point & Linear - MaxMipLevel failures (pulled in from V3_TOT) and modified
**       TEXBLT to call Blt32_CopyFourCC for textures with FourCC formats
**  24   3dfx      1.23        10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  23   3dfx      1.22        10/27/99 Russ Lind       cast changes, etc. so the
**       TnL_HAL code builds for w2k
**  22   3dfx      1.21        10/27/99 Russ Lind       ifdef legacy
**       D3DTBLEND_MODULATE changes for WINNT only
**       fix changes to three casts that broke the w2k build
**  21   3dfx      1.20        10/27/99 Bob Seitsinger  Additional rotated dither
**       matrix changes.
**  20   3dfx      1.19        10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  19   3dfx      1.18        10/25/99 Russ Lind       fix for dct 250 Texture
**       Blend - Decal, Modulate, ModulateAlpha & Copy failures, pull in legacy
**       D3DTBLEND_MODULATE handling from V3_TOT
**  18   3dfx      1.17        10/20/99 Bob Seitsinger  Rotated dither matrix
**       selection fogMode register values clean up.
**  17   3dfx      1.16        10/19/99 Russ Lind       fix for Texture Colorkey
**       failures in dct 250
**       added TEXFMT_ARGB_8332, TEXFMT_ALPHA_INTENSITY_44 & TEXFMT_ARGB_8888 cases
**       to chroma keying setup in setDX6State
**  16   3dfx      1.15        10/18/99 Bob Seitsinger  PRS 8373 fix -
**       conditionally modify the source and/or destination alpha blend mode.
**  15   3dfx      1.14        10/18/99 Russ Lind       in TEXBLT case, walk src
**       mipmap chain until we find the src size that matches the top dst level
**       size
**  14   3dfx      1.13        10/18/99 Tim Little      Included some of the
**       checking for invalide texture handles etc. that was previously only in use
**       for W2k
**  13   3dfx      1.12        10/07/99 Russ Lind       replaced use of
**       tempD3DHAL_DP2STATESET with D3DHAL_DP2STATESET
**  12   3dfx      1.11        10/04/99 Edwin Wong      Add TXTRHNDL_INRANGE() to
**       validate texture handle.
**  11   3dfx      1.10        09/30/99 Christopher Wilcox Fixes for DirectX 7
**       multi-buffering problem (PRS #8782)
** 
**  10   3dfx      1.9         09/28/99 Bob Seitsinger  SetDX6State - disable
**       stencilling if not in 32bpp mode.
**  9    3dfx      1.8         09/27/99 Christopher Wilcox Fix PRS 9031, caused by
**       overwriting the stack area where registers were saved.
**  8    3dfx      1.7         09/24/99 Christopher Wilcox Temporary removal of
**       DrawTriFanImmAsm call, due to problems with the assembly code.
** 
**  7    3dfx      1.6         09/21/99 Edwin Wong      Remove __sdtcall in
**       PCLRFUNC typedef statement since function is not defined with standard
**       call.
**  6    3dfx      1.5         09/20/99 Steve Rogers    Porting my fix for PRS 6247
**       from V3 OEM tree:  Reload texture pointers if context is switched.
** 
**  5    3dfx      1.4         09/16/99 Bob Seitsinger  Dither matrix rotate
**       changes - forgot to include WINNT_CONTEXT_COMPARE #define.
**  4    3dfx      1.3         09/16/99 Bob Seitsinger  Dither matrix rotate
**       changes. setDX6State -  write fogMode and renderMode registers as
**       appropriate to enable and select dither matrix rotation; also numerous
**       comments added and some code cleanup.
**  3    3dfx      1.2         09/15/99 Edwin Wong      Fix compiling error in
**       Win9x, add type cast for pfnClear assignment.
**  2    3dfx      1.1         09/15/99 Edwin Wong      Change policy in
**       determining which clear functions to use for D3DDP2OP_CLEAR.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
** 38    9/07/99 12:04p Bseitsin
** Properly (re)set sst.chipMask in setDX6state.
**
** 37    8/31/99 2:18p Edwin
** D3DDP2ROP_CLEAR will call FastFill() when running on Napalm and not in
** SLI mode.
**
** 36    8/30/99 10:02a Russ
** tweak to DX7 zbuffer processing in setDX6State (fixes Pod Racer z
** buffer problems on W2K)
**
** 35    8/27/99 1:43p Cshaw
** Check that the texture handle is valid before accessing ->surfLcl.
** Fixes PRS 8239.
**
** 34    8/27/99 1:23p Bseitsin
** Embellish a debug message in DrawPrimitives2, outside of case
** statement.
**
** 33    8/25/99 1:57p Russ
** for DX7, in setDX6State if we don't have a zbuffer for the current RC,
** disable zEnable & zWriteEnable
** for W2K & DX7, fix for potential access violation issues with NT Stress
** verify we have a valid TXTRHNDL for surfaces before dereferencing the
** TXTRHNDL_PTR
**
** 32    8/19/99 10:59a Russ
** added debug output when terminating command stream processing early
**
** 31    8/17/99 11:58a Russ
** for DX7, in TEXBLT call TEXTURELOAD repeatedly when loading mipmaps
**
** 30    8/10/99 1:24p Cshaw
** Added regisrty support for disabling guardband and 2ppc.
**
** 29    8/10/99 10:41a Russ
** for DX7, remove translations from MSHandle to TDFXHandle
**               in SETRENDERTARGET, add check for NULL Zbuffer handle
**
** 28    8/03/99 1:54p Msmith
** More fixes for Perspective Correct DCT failure
** from Bob J.
**
** 27    8/02/99 2:48p Cshaw
** Fixed a bug in the sw workaround for the 12 nop hw bug.
**
** 26    8/02/99 12:33p Russ
** for W2K, restore ASSERTDD's that were disabled last week
**
** 25    7/30/99 6:23p Msmith
** Enabled Bob J's changes again.
**
** 24    7/30/99 5:50p Msmith
** took out Bob J's changes temporarily
**
** 23    7/30/99 4:41p Msmith
** Checked in by me for Bob Johnston. Changes for multi-mon support
**
** 22    7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
**
** 21    7/28/99 9:05a Cshaw
** Added SW workaround for HW "feature": 2ppc to 1ppc transition requires
** writting 12 NOPs.
**
** 20    7/27/99 4:27p Bseitsin
** Antialiasing changes.
**
** 19    7/21/99 3:17p Cshaw
** Added 2ppc tuning via registry.
**
** 18    7/21/99 10:00a Cshaw
** Fixed blank textures in tunnel upon bootup.
**
** 17    7/14/99 3:17p Bseitsin
** Remove ddiDrawPrimitives2 change of 7/9/99. Unnecessary/invalid.
**
** 16    7/13/99 3:02p Russ
** change LPD3DHAL_DP2VIEWPORTINFO to D3DHAL_DP2VIEWPORTINFO * so w2k will
** compile again
**
** 15    7/13/99 3:38p Cshaw
** Added guardband for Napalm.
**
** 14    7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
**
** 13    7/12/99 1:13p Russ
** for DX7, in the D3DDP2OP_RENDERSTATE & D3DDP2OP_TEXTURESTAGESTATE
** handlers, if we're in recording mode then save states to state block,
** if we're not in recording mode then execute the states as usual.
** enable STATESET handler in DP2
** fix a W2K compile error added in previous rev
**
** 12    7/09/99 4:47p Bseitsin
** Bug fix in ddiDrawPrimitives2 when checking for vertex type or
** rendering context change.
**
** 11    7/09/99 4:33p Bseitsin
** Backwards compatability changes.
**
** 10    7/01/99 2:02p Russ
** fix for Texture Colorkey failures in DCT200
** add handlers for 8 bit luminance & 16 bit luminance+alpha texture
** formats in
** chromakey section of setDX6State
**
** 9     6/23/99 9:17p Russ
** mods to the new DX7 token handlers in DP2 to check return codes from
** sub funcs
** enable SETPALETTE & UPDATEPALETTE handlers in DP2
** DX7 palettized texture changes (#ifdef'd for DX >= 7) in setDX6State
**
** 8     6/22/99 10:03a Russ
** fix for broken 9x build
** make NT error checking macros nops for 9x builds
**
** 7     6/21/99 5:37p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
** added TEXBLT handler for DX >= 7
** reenable asm code for NT in TRIANGLEFAN_IMM
** update to NT error checking code in DP2
**
** 6     6/16/99 3:45p Cshaw
** Added more texop support for texturefactors on color1, and alpha
** replicate.
**
** 5     6/16/99 10:46a Russ
** added translation of ms handle to tdfx handle in
** D3DDP2OP_TEXTURESTAGESTATE
** modified D3DDP2OP_SETRENDERTARGET handler
** added D3DDP2OP_CLEAR handler
**
** 4     6/14/99 3:17p Bseitsin
** Triangle Iterator Column Band Control support.
**
** 3     6/11/99 12:50p Russ
** initial changes for DX7
**
** 2     6/04/99 7:14p Bseitsin
** 24bit Z/8bit stencil mods.
**
** 1     6/02/99 6:44a Michael
** Branch from H3
**
** 73    5/24/99 5:06p Bseitsin
** Removal of Antialiasing code.
**
** 71    5/17/99 12:20p Stb_gbullard
** Corrected mangling of a SETPD call argument in setDX6state (from an IGX
** fix check-in error on 5/14/99)
**
** 70    5/14/99 4:54p Stb_gbullard
** Updated with contractor (NVH, Intelligraphics) fixes
**
** 69    5/12/99 5:15p Stb_gbullard
** Updated with contractor (NVH, Intelligraphics) fixes
**
** 68    5/04/99 8:54a Cshaw
** Added &&defined(NAPALM_CU) so we can disable Napalm CU code until CSIM
** works on win98.
**
** 67    4/26/99 1:18p Stb_bseitsin
** 32bit mods. Use new SETSURFACEPIXELDEPTH macro.
**
** 66    4/26/99 1:49p Cshaw
** combineMode mods.
**
** 65    4/23/99 2:06p Stb_bseitsin
** 32-bit rendering mods
**
** 64    4/15/99 3:00p Cshaw
** Added some preliminary code for Napalm's combineMode.
**
** 63    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 62    4/02/99 7:58a Russ
** fix for wcmp and wfog problems in dct version 166 on NT5
**   in WINFO, set aW and bW to scale 1:1 if wMin and wMax are
**   already inside hw range
** additional error checking for NT5
** additional debug output
**
** 61    3/17/99 2:42p Andrew
** reverted to old version to remove swaphang workaround
**
** 59    2/26/99 4:35p Peterm
** Removed code related to sub-optimal fix for when sourcing alpha from
** textures without alpha.  Turns out hw doesn't need to source from
** color1 in this case.
**
** 58    2/22/99 9:03a Russ
** replaced usage of NT_D3DPRINT with D3DPRINT
**
** 57    2/19/99 3:37p Peterm
** Added code s.t. if SST_ASEL_C1 is set in fbzColorPath, color1 is loaded
** with 0xff000000.  This fixes a bug when sourcing alpha from textures
** that don't contain alpha (we redirect alpha to color1 and set color1's
** alpha to 0xff)
**
** 56    2/18/99 8:35a Russ
** add/modify a bunch of debug output for NT5
** add more error checking in D3DDP2OP_POINTS (check for bogus
**      vertex data)
** add error checking in D3DDP2OP_TRIANGLELIST
** temp fix for MS RAID bug 278998, wbuffer dct fails tests 1 and 99 using
**      asm code but pass using the C code
** hack to fix MS RAID bugs 286493 and 286496
**      in setDX6State disable alpha blending enable bit in alphaMode
** register
**      if (srcBlend == SRCALPHA) && (dstBlend==INVSRCALPHA)
**         && ((texMapBlend==COPY) || (texMapBlend==DECAL))
**
** 55    2/10/99 5:14p Adrians
** Fixes a problem with palettized textures and chromakeying (PRS-4148).
**
** 54    2/03/99 9:51p Russ
** for NT5, added error checking in some of the DP2OP2 cases (more needs
** to be added). This is needed in order to pass a new DCT test that
** throws garbage data at DP2.
**
** 53    1/26/99 5:27p Peterm
** Added unified header information
**
** 52    1/21/99 4:18p Cwilcox
** Fix for problem with vertex type changes when context switching.
**
** 51    1/20/99 1:49p Adrians
** Changes to DX6 hardware setup.
**
** 50    1/18/99 2:08p Adrians
** Removed obsolete variables and associated code.
**
** 49    1/15/99 11:24a Adrians
** Remove some duplicate code.
**
** 48    1/14/99 3:03p Adrians
** Fix min/max LOD to 1x1 for unused tmu's.
**
** 47    1/12/99 8:04a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
**
** 46    1/07/99 12:01p Cshaw
** Added #defs for doing wb99 performance analysis.
** To disable areas of the driver use the following environment vars:
** nd=1 (manditory - enables NULLDRIVER)
** OND_TEXD=1 (optional - OverridesNullDriver by executing
** TextureDownloads)
** OND_STATE=1 (optional - OverridesNullDriver by executing
** D3DStateChanges)
** OND_HWSU=1 (optional - OverridesNullDriver by executing Hardware setup)
** OND_ZB=1 (optional - OverridesNullDriver by executing ZBuffer clears)
** OND_TRI=1 (optional - OverridesNullDriver by executing Triangle
** Rendering)
**
** 45    12/23/98 1:16p Martin
** Set auxBufferAddr when zWriteEnable is on.  If an app (MFCtex), enables
** writes to the zbuffer, but doesn't enable zbuffer compares, then we
** still need to set up the zbuffer address.
**
** 44    12/16/98 6:34a Russ
** fix for corruption when resizing tunnel or twist window on NT5
**
** 43    12/11/98 2:55p Martin
** If trying to set textureHandle to -1 set to 0.  Fixes
** overdraw/underdraw with DCT 50.
**
** 42    12/09/98 6:44a Russ
** NT5 D3D changes for Banshee
**
** 41    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 40    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 39    11/28/98 5:10p Adrians
** Added support for INDEXEDLINELIST2.
** Minor code changes.
**
** 38    11/27/98 4:30p Adrians
** Fix for chromakey problem with Moto Racer 2 on Banshee and Avenger.
**
** 37    11/22/98 9:09p Andrew
** Changes to support multi-monitor
**
** 36    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
**
** 35    11/08/98 3:22p Hanson
** Improvements for Winbench 99
**
** 34    11/06/98 5:56p Adrians
** Add 2 new FVF formats for WinBench99.
**
** 33    11/02/98 1:40p Miriam
** Fix the null driver build (misspelled pointer).
**
** 32    11/03/98 12:15a Artg
** made changes to for multitextures for h4
**
** 31    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 30    10/08/98 2:01a Hanson
** K6 3D continued cleanup
**
** 29    10/07/98 10:39p Adrians
** Added K6-2 optimisations.
**
** 27    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 26    10/05/98 5:31p Adrians
** Added support for DrawPrimitive2 command INDEXEDLINELIST.
**
** 25    10/04/98 9:47p Adrians
** Add support for mirrored textures.
**
** 24    10/04/98 5:52p Hanson
** Added Prefetch of 8 case to D3DDP2OP_INDEXEDTRIANGLELIST2
**
** 23    10/01/98 5:31p Hanson
** Triangle setup bug fix.
**
** 22    10/01/98 5:05p Hanson
** Dx6 changes mostly affecting triangle flavoring and state flags.
**
** 21    9/26/98 5:32p Hanson
** Dx6 Assembly interface Optimizations
**
** 20    9/26/98 4:06p Hanson
** Dx6 fixes for Billboard and some multitexturing samples.
**
** 19    9/24/98 4:22p Adrians
** Added initial Instrumentation support.
**
** 18    9/24/98 11:51a Hanson
** Dx6 Optimizations
**
** 17    9/21/98 3:12p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
**
** 16    9/13/98 11:20p Adrians
** Add support for TextureFactor renderstate.
** Fix bug with HW_STATE checking.
**
** 15    9/12/98 12:57a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
**
** 14    9/08/98 3:24p Adrians
** Propagate stage textureAddress change to textureaddressU/V.
**
** 13    9/05/98 5:01p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 12    9/02/98 12:33p Adrians
** DX6 multitexture change.
**
** 11    8/28/98 1:33p Adrians
** Support for new multi-texture scheme.
** Added DP2 LineStrip support.
**
** 10    8/26/98 10:39a Adrians
** Add previous typo fix to execute buffers and DX6 setup.
**
** 9     8/15/98 12:15a Adrians
** Take Flat Shaded color from correct vertex for strips and fans.
** Use the same alpha component for all 3 vertices when Flat Shading.
** Add a Flat Shaded vertex component parameter to the line and point
** drawing.
**
** 8     8/14/98 8:30p Adrians
** Fix texture clamping.
**
** 7     8/14/98 8:10p Adrians
** Add automipmap support.
** Add setup changes from DX5 driver.
**
** 2     8/10/98 5:18p Adrians
** Add support for AutoMipMap.
** Add state changes from DX5 driver to DX6 driver.
**
** 6     8/05/98 4:37p Adrians
** Correct support for ADD and SUBTRACT texturing modes as well as adding
** support for BLENDDIFFUSEALPHA and BLENDTEXTUREALPHA.
**
** 4     7/30/98 4:36p Adrians
** DX6 line drawing support.
**
** 3     7/29/98 7:24p Adrians
** DX6 changes.
**
** 1     5/06/98 6:23p Adrians
** New DX6 files.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
**
**
** 1     4/29/98 6:31p Adrians
** Created
*/

#include "precomp.h"

#if( DX >= 6 )

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
extern DWORD __stdcall Blt32_CopyFourCC(NT9XDEVICEDATA *,LPDDRAWI_DDRAWSURFACE_LCL,RECTL*,LPDDRAWI_DDRAWSURFACE_LCL,RECTL*);
#endif

#if( DX >= 8 )

extern VOID    _D3D_OP_MStream_SetSrc(RC *pRc, DWORD dwStream, DWORD dwVBHandle, DWORD dwStride);
extern VOID    _D3D_OP_MStream_SetSrcUM(RC *pRc, DWORD dwStream, DWORD dwStride, LPDWORD lpVertices, DWORD dwVBSize);
extern VOID    _D3D_OP_MStream_SetIndices(RC *pRc, DWORD dwVBHandle, DWORD dwStride);
extern VOID    _D3D_OP_MStream_DrawPrim(RC *pRc, D3DPRIMITIVETYPE primType, DWORD VStart, DWORD PrimitiveCount);
extern VOID    _D3D_OP_MStream_DrawIndxP(RC *pRc, D3DPRIMITIVETYPE primType, DWORD BaseVertexIndex, DWORD MinIndex, DWORD NumVertices, DWORD StartIndex, DWORD PrimitiveCount);
extern VOID    _D3D_OP_MStream_DrawPrim2(RC *pRc, D3DPRIMITIVETYPE primType, DWORD FirstVertexOffset, DWORD PrimitiveCount);
extern VOID    _D3D_OP_MStream_DrawIndxP2(RC *pRc, D3DPRIMITIVETYPE primType, INT BaseVertexOffset, DWORD MinIndex, DWORD NumVertices, DWORD StartIndexOffset, DWORD PrimitiveCount);
extern VOID    _D3D_OP_MStream_ClipTriFan(RC *pRc, DWORD FirstVertexOffset, DWORD dwEdgeFlags, DWORD PrimitiveCount);
extern VOID    _D3D_OP_MStream_DrawRectSurface(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim);
extern VOID    _D3D_OP_MStream_DrawTriSurface(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim);
extern HRESULT _D3D_OP_VertexShader_Create(RC *pRc, DWORD dwVtxShaderHandle, DWORD dwDeclSize, DWORD dwCodeSize, BYTE *pShader);
extern VOID    _D3D_OP_VertexShader_Delete(RC *pRc, DWORD dwVtxShaderHandle);
extern VOID    _D3D_OP_VertexShader_Set(RC *pRc, DWORD dwVtxShaderHandle);
extern VOID    _D3D_OP_VertexShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwConst, DWORD *pdwValues);
extern HRESULT _D3D_OP_PixelShader_Create(RC *pRc, DWORD dwPxlShaderHandle, DWORD dwCodeSize, BYTE *pShader);
extern VOID    _D3D_OP_PixelShader_Delete(RC *pRc, DWORD dwPxlShaderHandle);
extern VOID    _D3D_OP_PixelShader_Set(RC *pRc, DWORD dwPxlShaderHandle);
extern VOID    _D3D_OP_PixelShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwCount, DWORD *pdwValues);

#endif

#include "dxins.h"

#define ENABLE_ASM              1
#define ENABLE_ERROR_CHECKING   1

#if defined(WINNT) && ENABLE_ERROR_CHECKING
/* retro3dfx: flight-record every DP2 parse error (op + hr + offset) so a
   D3DERR_DRIVERINTERNALERROR seen by the runtime can be traced to the
   failing command without a checked build.  ppdev is in scope at every
   expansion site (SETUP_PPDEV in ddiDrawPrimitives2). */
#if ENABLE_LOG_FILE
#define RETRO_DP2_ERRLOG(pDP2Data, pIns, pStartIns, ddrvalue)               \
    retroLogForce(ppdev, "retro3dfx DP2-PARSE-ERR: hr=%08lXh op=%d off=%ld cmdLen=%ld\r\n", \
                  (DWORD)(ddrvalue),                                        \
                  (int)((LPD3DHAL_DP2COMMAND)(pIns))->bCommand,             \
                  (LONG)((LPBYTE)(pIns)-(LPBYTE)(pStartIns)),               \
                  (pDP2Data)->dwCommandLength)
#else
#define RETRO_DP2_ERRLOG(pDP2Data, pIns, pStartIns, ddrvalue)
#endif
#define PARSE_ERROR_AND_EXIT(pDP2Data, pIns, pStartIns, ddrvalue)      \
  {                                                                    \
    D3DPRINT(0, "  returning error code %08lX", ddrvalue);             \
    RETRO_DP2_ERRLOG(pDP2Data, pIns, pStartIns, ddrvalue);             \
    pDP2Data->dwErrorOffset = (DWORD)((LPBYTE)pIns-(LPBYTE)pStartIns); \
    pDP2Data->ddrval = ddrvalue;                                       \
    goto Exit_DrawPrimitives2;                                         \
  }

// Macros for verifying validity of the command and vertex buffers. This MUST
// be done by the driver even on free builds as the runtime avoids this check
// in order to not parse the command buffer too.
#define CHECK_CMDBUF_LIMITS(pDP2Data, pBuf, type, num, extrabytes)            \
        CHECK_CMDBUF_LIMITS_S(pDP2Data, pBuf, sizeof(type), num, extrabytes)

#define CHECK_CMDBUF_LIMITS_S(pDP2Data, pBuf, typesize, num, extrabytes)      \
  {                                                                           \
    LPBYTE pBase,pEnd,pBufEnd;                                                \
    pBase = (LPBYTE)(pDP2Data->lpDDCommands->lpGbl->fpVidMem +                \
                     pDP2Data->dwCommandOffset);                              \
    pEnd  = pBase + pDP2Data->dwCommandLength;                                \
    pBufEnd = ((LPBYTE)pBuf + ((num) * (typesize)) + (extrabytes) - 1);       \
    if (! ((LPBYTE)pBufEnd < pEnd) && ( pBase <= (LPBYTE)pBuf))               \
    {                                                                         \
      D3DPRINT(0,"DP2: Trying to read past Command Buffer limits "            \
               "%x %x %x %x",pBase ,(LPBYTE)pBuf, pBufEnd, pEnd );            \
      PARSE_ERROR_AND_EXIT(pDP2Data, lpCmd, pDP2Data->lpDDCommands->lpGbl->fpVidMem, \
                           D3DERR_COMMAND_UNPARSED);                          \
    }                                                                         \
  }

#define CHECK_DATABUF_LIMITS(pDP2Data, iIndex)                                \
  {                                                                           \
    if (! (((LONG)iIndex >= 0) &&                                             \
           ((LONG)iIndex <(LONG)pDP2Data->dwVertexLength)))                   \
    {                                                                         \
      D3DPRINT(0, "DP2: Trying to read past Vertex Buffer limits "            \
               "%d limit= %d ",(LONG)iIndex, (LONG)pDP2Data->dwVertexLength); \
      PARSE_ERROR_AND_EXIT(pDP2Data, lpCmd, pDP2Data->lpDDCommands->lpGbl->fpVidMem, \
                           D3DERR_COMMAND_UNPARSED);                          \
    }                                                                         \
  }

#define CULL_TRI(pCtxt,p0,p1,p2)                                         \
    ((pCtxt->cullMode != D3DCULL_NONE) &&                                \
     (((p1->sx - p0->sx)*(p2->sy - p0->sy) <=                            \
       (p2->sx - p0->sx)*(p1->sy - p0->sy)) ?                            \
      (pCtxt->cullMode == D3DCULL_CCW)     :                             \
      (pCtxt->cullMode == D3DCULL_CW) ) )

#else // win9x
#define PARSE_ERROR_AND_EXIT(pDP2Data, pIns, pStartIns, ddrvalue)
#define CHECK_CMDBUF_LIMITS(pDP2Data, pBuf, type, num, extrabytes)
#define CHECK_CMDBUF_LIMITS_S(pDP2Data, pBuf, typesize, num, extrabytes)
#define CHECK_DATABUF_LIMITS(pDP2Data, iIndex)

#define CULL_TRI(pCtxt,p0,p1,p2)                                         \
    ((pCtxt->cullMode != D3DCULL_NONE) &&                                \
     (((p1->sx - p0->sx)*(p2->sy - p0->sy) <=                            \
       (p2->sx - p0->sx)*(p1->sy - p0->sy)) ?                            \
      (pCtxt->cullMode == D3DCULL_CCW)     :                             \
      (pCtxt->cullMode == D3DCULL_CW) ) )

#endif

#define LP_FVF_VERTEX(lpBaseAddr, wIndex) \
          (LPD3DTLVERTEX)((LPBYTE)(lpBaseAddr) + (wIndex) * FVFO_SIZE * sizeof(DWORD))

#define LP_FVF_NXT_VTX(lpVtx) \
          (LPD3DTLVERTEX)((LPBYTE)(lpVtx) + FVFO_SIZE * sizeof(DWORD))

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// Macros for updating the instruction pointer to the next instruction
// in the command buffer
#define NEXTINSTRUCTION(ptr, type, num, extrabytes)                            \
        NEXTINSTRUCTION_S(ptr, sizeof(type), num, extrabytes)

#define NEXTINSTRUCTION_S(ptr, typesize, num, extrabytes)                      \
    ptr = (LPD3DHAL_DP2COMMAND)((LPBYTE)ptr + sizeof(D3DHAL_DP2COMMAND) +      \
                                ((num) * (typesize)) + (extrabytes))
#endif

// Static function prototypes
DWORD setDX6state( RC *pRc, DWORD primitiveType, DWORD count );

#ifndef WINNT
PFND3DPARSEUNKNOWNCOMMAND dp2Callback = NULL;
#endif

//FxU32 vertexType=D3DFVF_TLVERTEX;

#define ENABLE_VERTEX_DUMP    0
//#define ENABLE_VERTEX_DUMP    1

#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
static __inline VOID
DumpFVFData ( const char *start, LPD3DTLVERTEX pV)
{
  D3DPRINT(D3DDBGLVL+1, "%s - %08lXh %08lXh %08lXh %08lX %08lXh %08lXh %08lXh %08lX",
           start,
           INTP(pV)[FVFO_SX],   INTP(pV)[FVFO_SY],
           INTP(pV)[FVFO_SZ],   INTP(pV)[FVFO_RHW],
           (FVFO_COLOR    == 128) ? 0xDEADDEAD : INTP(pV)[FVFO_COLOR],
           (FVFO_SPECULAR == 128) ? 0xDEADDEAD : INTP(pV)[FVFO_SPECULAR],
           (FVFO_TU       == 128) ? 0xDEADDEAD : INTP(pV)[FVFO_TU],
           (FVFO_TV       == 128) ? 0xDEADDEAD : INTP(pV)[FVFO_TV]);
}
#else
#define DumpFVFData             1 ? (void)0 : (void)
#endif


/*------------------------------------------------------------------------------------
Function Name:  UpdateCtxFVFChanges

Parameters:     DWORD pContext 
                DWORD dwOutputVertexType

Description:    UpdateCtxFVFChanges() is a function that resets context state and 
                assembly rendering dispatch functions when it detects either a context 
                change or Output Vertex type change. 

                This logic used inline at the top of the ddDrawPrimitives2 in the old 
                DX6 interface, but since Output Vertex Type is not known until
                TnL is performed, we must now execute this only before we call down to the 
                triangle drawing code. 

Return:         VOID
-------------------------------------------------------------------------------------*/
static VOID UpdateCtxFVFChanges(DWORD pContext, DWORD dwOutputVertexType)
{                   
  SETUP_PPDEV(pContext)
  RC *pRc = CONTEXT_PTR(pContext);
  DWORD     i, size, type;
#if (NEWASMTRI == 1)
  #ifdef K6_2
    extern BOOL _3DNowAllowed;
  #endif // K6_2
#endif // NEWASMTRI

#if defined(TNL_PROFILE) && defined(TnL_HAL)
  UpdateFVFStats(dwOutputVertexType); 
#endif // TNL_PROFILE

  /* Check for vertex type or rendering context change. -CGW- */
  if ((_asm_data.vertex_type != dwOutputVertexType ) || (pContext != _D3(lastContext)))
  {
    _asm_data.vertex_type=dwOutputVertexType;
    pRc->state|=FLAGS_FVF;
    switch( dwOutputVertexType )
    {
      case D3DFVF_TLVERTEX :
        pRc->dwVertexType = FVFOT_TLVERTEX;
        pRc->state &=~(FLAGS_FVF_EXECUTE|FLAGS_FVF_IDX2);
        pRc->DrawTriIdx2=PrefetchTriIdx2_8_Asm;
        break;

      // case (D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0x44 :
        pRc->dwVertexType = FVFOT_XYZWD;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_5_Asm;
        break;

      // case (D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0x144 :
        pRc->dwVertexType = FVFOT_XYZWDT1;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_7_Asm;
        break;

      // case (D3DFVF_TEX1 | D3DFVF_SPECULAR | D3DFVF_XYZRHW) :
      case 0x184:
        pRc->dwVertexType = FVFOT_XYZWST1;
		#if (NEWASMTRI == 1)
		  #ifdef K6_2
            if (_3DNowAllowed)
              pRc->DrawTriIdx2=PrefetchTriIdx2_XX_Asm_K6;
		  #endif // K6_2
        #endif //NEWASMTRI
        break;

      // case (D3DFVF_TEX2 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW ) :
      case 0x244 :
        pRc->dwVertexType = FVFOT_XYZWDT2;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_9_Asm;
        break;

      // case (D3DFVF_TEX2 | D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0x2C4 :
        pRc->dwVertexType = FVFOT_XYZWDST2;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_10_Asm;
        break;

      // case (D3DFVF_TEX2 | D3DFVF_SPECULAR | D3DFVF_XYZRHW) :
      case 0x284 :
        pRc->dwVertexType = FVFOT_XYZWST2;
		#if (NEWASMTRI == 1)
		  #ifdef K6_2
            if (_3DNowAllowed)
              pRc->DrawTriIdx2=PrefetchTriIdx2_XX_Asm_K6;
		  #endif // K6_2
        #endif // NEWASMTRI
        break;

      // case (D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0xC4 :
        pRc->dwVertexType = FVFOT_XYZWDS;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_6G_Asm;
        break;

      // case (D3DFVF_TEX1 | D3DFVF_XYZRHW) :
      case 0x104 :
        pRc->dwVertexType = FVFOT_XYZWT1;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_6T_Asm;
        break;

      // case (D3DFVF_TEX2 | D3DFVF_XYZRHW) :
      case 0x204 :
        pRc->dwVertexType = FVFOT_XYZWT2;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_8_Asm;
        break;

      // case (D3DFVF_TEX3 | D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0x3C4 :
        pRc->dwVertexType = FVFOT_XYZWDST3;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_12_Asm;
        break;

      // case (D3DFVF_TEX3 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW) :
      case 0x344 :
        pRc->dwVertexType = FVFOT_XYZWDT3;
        pRc->state &=~FLAGS_FVF_IDX2;
        pRc->DrawTriIdx2=PrefetchTriIdx2_11_Asm;
        break;

      default :
        // We need to create a custom table entry based on the FVF flags
        D3DPRINT( 0, "  <WARNING> Generating a new FVF table entry  (VertexType=%8lXh)", dwOutputVertexType );

        pRc->dwVertexType = FVFOT_CUSTOM;
        type = dwOutputVertexType;
        size = 0;

        fvfOffsetTable[pRc->dwVertexType].sx = size++;
        fvfOffsetTable[pRc->dwVertexType].sy = size++;
        fvfOffsetTable[pRc->dwVertexType].sz = size++;

        if( type & D3DFVF_XYZRHW )
           fvfOffsetTable[pRc->dwVertexType].rhw = size++;
        else
           fvfOffsetTable[pRc->dwVertexType].rhw = 128;

        if( type & D3DFVF_DIFFUSE )
           fvfOffsetTable[pRc->dwVertexType].color = size++;
        else
           fvfOffsetTable[pRc->dwVertexType].color = 128;

        if( type & D3DFVF_SPECULAR )
           fvfOffsetTable[pRc->dwVertexType].specular = size++;
        else
           fvfOffsetTable[pRc->dwVertexType].specular = 128;

        *((LPDWORD)&fvfOffsetTable[pRc->dwVertexType].tu) = 0;
        *((LPDWORD)&fvfOffsetTable[pRc->dwVertexType].tv) = 0;

        for( i = 0; i < (type & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT; i++ )
        {
           *((LPDWORD)&fvfOffsetTable[pRc->dwVertexType].tu + (i << 1)) = size++;
           *((LPDWORD)&fvfOffsetTable[pRc->dwVertexType].tv + (i << 1)) = size++;
        }
		
		#if (NEWASMTRI == 1)
          #ifdef K6_2		  
            if (_3DNowAllowed)
              pRc->DrawTriIdx2=PrefetchTriIdx2_XX_Asm_K6;
		  #endif // K6_2
        #endif // NEWASMTRI

        fvfOffsetTable[pRc->dwVertexType].size = size;
        break;
    }

    pRc->dwVerticesStride = fvfOffsetTable[pRc->dwVertexType].size;
    _asm_data.vertex_size=(fvfOffsetTable[pRc->dwVertexType].size) << 2;
  }

} /* UpdateCtxFVFChanges */

#ifdef TnL_HAL
/*-------------------------------------------------------------------
Function Name:  UpdateFVF

Description:    Check for changes in the input FVF format for the T&L 
                HAL, or the input FVF format for transformed and lit 
                data from D3D.

                This updates:
                pRc->tl.dwFVFIn 
                pRc->state
                pRc->DrawTriIdx2                
                fvfOffsetTable
                _asm_data.vertex_type


                The FVF is confusing to deal with. If the vertex buffer
                has been transformed and lit by D3D or the app, the 
                incoming FVF format is the same format as used by the 
                rendering code. If the vertex buffer requires T&L,
                then the incoming FVF will be different from that which
                is ultimately used for rendering.


Parameters:     DWORD pContext 
                DWORD dwInputVertexType

Return:         VOID
-------------------------------------------------------------------*/

static VOID UpdateFVF(DWORD pContext, DWORD dwInputVertexType)
{
  SETUP_PPDEV(pContext)
  RC *pRc = CONTEXT_PTR(pContext);


  if (FVF_TRANSFORMED(dwInputVertexType))
  {
    // Handle transformed vertices
    pRc->tl.dwTLState &= ~(TLPV_TLNEEDED | TLPV_VALIDCLIPBUFFER);
    UpdateCtxFVFChanges(pContext, dwInputVertexType);
    return;    
  }

  // Handle untransformed vertices

  pRc->tl.dwTLState |= TLPV_TLNEEDED;
  if ((DWORD)pRc->tl.InFVF.dwFVFType != dwInputVertexType)
  {
    pRc->tl.InFVF.dwFVFType = dwInputVertexType;
    
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FVFIN;
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FVFOUT;
  }
    
}  


#endif // TnL_HAL


/*-------------------------------------------------------------------
Function Name:  ddiDrawPrimitives2

Description:    Entry point for DrawPrimitives2 function.  This is
                the DX6 workhorse.

Return:         DWORD
-------------------------------------------------------------------*/

void HandleSLIAA(RC * pRc);
DWORD __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d )
{
  SETUP_PPDEV(lpdp2d->dwhContext)
  RC                  *pRc;
  LPDWORD             lpVertices, lpVOffset, lpV0, lpV1;
#ifdef TnL_HAL 
  LPVBSURFACEDATA	  lpVBSurfData = 0;
#endif // TnL_HAL 
  LPD3DHAL_DP2COMMAND lpCmd, lpResumeCmd;
  LPBYTE              lpPrim, lpIdx;
  DWORD               cmdEnd, i, j;
  HRESULT             hr = D3D_OK;
  DWORD               dwEdgeFlags;
#if (NEWASMTRI == 1)
  #ifdef K6_2
    extern BOOL _3DNowAllowed;
  #endif // K6_2
#endif // NEWASMTRI

  D3D_ENTRY( "ddiDrawPrimitives2" );
  INS_ENTRY( INSC_D3DDRAWPRIMITIVES2 );

#if defined( NULLDRIVER )
  if ((!_D3(ondrtTri)) && (!_D3(ondrtState)) && (!_D3(ondrtHWSU)) )
  {
    lpdp2d->ddrval = DD_OK;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif /*OND_*/

#ifdef FXTRACE
  if (CONTEXT_VALIDATE(lpdp2d->dwhContext))
  {
    D3DPRINT( 255, "DrawPrimitives2 bad context =0x08lx", lpdp2d->dwhContext);
    lpdp2d->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

  pRc = CONTEXT_PTR(lpdp2d->dwhContext);

  if( lpdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES )
    lpVertices = (LPDWORD)((LPBYTE)lpdp2d->lpVertices + lpdp2d->dwVertexOffset);
  else
  {
    lpVertices = (LPDWORD)((LPBYTE)lpdp2d->lpDDVertex->lpGbl->fpVidMem + lpdp2d->dwVertexOffset);

 #if defined(TnL_HAL) && defined(VERT_BUFF)
    lpVBSurfData =(LPVBSURFACEDATA)lpdp2d->lpDDVertex->lpGbl->dwReserved1; 
 #endif // TnL_HAL 
  }

  D3DPRINT( 255, "wpVtData = 0x%08lx", lpVertices );

  lpCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpdp2d->lpDDCommands->lpGbl->fpVidMem + lpdp2d->dwCommandOffset);
  cmdEnd = (DWORD)lpCmd + lpdp2d->dwCommandLength;

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  // Take the VB address from our header info if we are 
  // processing a DX7 or earlier context. Otherwise we'll get updates 
  // through the new DX8 DP2 tokens (D3DDP2OP_SETSTREAMSOURCE & 
  // D3DDP2OP_SETVERTEXSHADER)
  if (IS_DX7_OR_EARLIER_APP(pRc))
  {
      // Update place from where vertices will be processed for this context
      pRc->lpVertices = lpVertices;
  }
#endif //(DIRECT3D_VERSION >= 0x0800) && (DX >= 8)


#ifdef TnL_HAL
  // On Primitives with vertices already transformed and lit
  // update the context and assembler data structures 
  // here before we process the FOR loop on the command stream
  UpdateFVF(lpdp2d->dwhContext, lpdp2d->dwVertexType);
#else
  UpdateCtxFVFChanges(lpdp2d->dwhContext, lpdp2d->dwVertexType);
#endif


  UPDATE_HW_STATE( SC_BUFFERS );

#if ENABLE_LOG_FILE
  /* retro3dfx: mark the first DP2 batch per context — if a context is
     destroyed WITHOUT this line, the runtime aborted before any drawing
     reached the driver (device-init stage failure). */
  {
    static DWORD g_retroLastDp2Ctx = 0;
    if (lpdp2d->dwhContext != g_retroLastDp2Ctx)
    {
      g_retroLastDp2Ctx = lpdp2d->dwhContext;
      retroLogForce(ppdev, "retro3dfx DP2-FIRST: ctx=%08lXh cmdLen=%ld\r\n",
                    lpdp2d->dwhContext, lpdp2d->dwCommandLength);
    }
  }
#endif

  // Always disable zeroing the jitter values at the start of this for loop
  // and let the special case code enable it, if appropriate.
  pRc->dwZeroJitter = 0;

  // Now we process the DrawPrimitive2 commands.
  for(;;)
  {
    D3DPRINT( 255, "  DP2 Command =%2d, Count =%d", lpCmd->bCommand, lpCmd->wStateCount );
    #if defined( FLAVOR_PROFILE )
      updateState |= UPDATE_STATE;
    #endif

    // Setup primitive data pointer
    lpPrim = (LPBYTE)(lpCmd + 1);

    // switch based on the command
    switch( lpCmd->bCommand )
    {
      case D3DDP2OP_RENDERSTATE :
      {
        D3DPRINT( 32, "  Command RENDERSTATE (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "  D3DDP2OP_RENDERSTATE (wStateCount=%d)", lpCmd->wStateCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2RENDERSTATE, lpCmd->wStateCount, 0);

        for (i = 0; i < lpCmd->wStateCount; i++, ((LPD3DHAL_DP2RENDERSTATE)lpPrim)++)
        {
          #if defined(NULLDRIVER)
          if (_D3(ondrtState))
          #endif
          {
            D3DRENDERSTATETYPE renderState = ((LPD3DHAL_DP2RENDERSTATE)lpPrim)->RenderState;
            DWORD data                     = ((LPD3DHAL_DP2RENDERSTATE)lpPrim)->dwState;

            printRenderState( renderState, data );

            if( IS_OVERRIDE( renderState ) )
            {
              DWORD override = GET_OVERRIDE( renderState );

              if ( data )
              {
                  STATESET_SET( pRc->overrides, override);
              }
              else
              {
                  STATESET_CLEAR( pRc->overrides, override);
              }
              continue;
            }

            if( STATESET_ISSET( pRc->overrides, renderState ) )
            {
              continue;
            }

            {
              // We are going to use renderState to access our array of
              // rendering functions.   Here we account for the possiblity
              // that NT is running some future version of DirectX that we
              // do not have a rendering functions for.  We will test
              // renderState before we use it so that we don't read
              // past the end of the array of rendering functions.
              // If we do not have a renderinf function, we will use
              // the dummy function at index 0.

              D3DRENDERSTATETYPE renderState_Tmp =
                  renderState <= MAX_NUM_RSTATES ? renderState : 0;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              // if we're recording a state block just go save the render state
              if (FALSE == pRc->bSBRecMode)
              {
#endif
                #ifdef WINNT
                  #if DBG
                    if (_renderFuncs[renderState_Tmp] == &dummy)
                      D3DPRINT( RSTATE_DBG_LVL, "unsupported Renderstate %ld, state = %ld", renderState, data );
                  #endif

                  lpdp2d->ddrval = _renderFuncs[renderState_Tmp](pRc, data);
                  if (DD_OK != lpdp2d->ddrval)
                  {
                    PARSE_ERROR_AND_EXIT(lpdp2d, lpCmd, lpdp2d->lpDDCommands->lpGbl->fpVidMem, lpdp2d->ddrval);
                  }
                #else
                  _renderFuncs[ renderState_Tmp ]( pRc, data );
                #endif
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              }
              else
              {
                // make sure it's a render state we care about
                if ((NULL != pRc->pCurrSB) &&
                    (renderState <= MAX_RENDERSTATES) &&
                    (_renderFuncs[renderState] != &dummy))
                {
                  D3DPRINT(RSTATE_DBG_LVL, "    storing STATEBLOCK Renderstate %ld, state = %ld", renderState, data);
                  pRc->pCurrSB->uc.RenderStates[renderState] = data;
                  // set flag to mark this render state is stored in state block
                  SET_SB_RS_FLAG(pRc->pCurrSB,renderState);
                }
#if DBG
                else
                {
                  D3DPRINT(RSTATE_DBG_LVL, "    ignoring STATEBLOCK Renderstate %ld, state = %ld", renderState, data);
                }
#endif
              }
#endif
            }
          }
        }

        UPDATE_HW_STATE( SC_SOMETHING );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)lpPrim;
        hr = D3D_OK;
        break;
      }

      // Texture Stage States
      case D3DDP2OP_TEXTURESTAGESTATE :
      {
        D3DPRINT( 32, "  Command TEXTURESTAGE (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "  D3DDP2OP_TEXTURESTAGESTATE (wStateCount=%d)", lpCmd->wStateCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TEXTURESTAGESTATE, lpCmd->wStateCount, 0);

        for(i = 0; i < lpCmd->wStateCount; i++, ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)++ )
        {
       #if defined(NULLDRIVER)
        if (_D3(ondrtState)) {
       #endif
          DWORD                     stage = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->wStage;
          D3DTEXTURESTAGESTATETYPE  state = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->TSState;
          DWORD                     data  = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->dwValue;

          D3DTEXTURESTAGESTATETYPE state_tmp =
            state <= MAX_NUM_TSTATES ? state : 0;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          // if we're recording a state block just go save the texture stage state
          if (FALSE == pRc->bSBRecMode)
          {
#endif
            printTextureStageState( stage, state, data );
#if defined(WINNT)
          D3DPRINT(D3DDBGLVL, "    stage=%ld  state=%ld  data=%ld", stage, state, data);
#endif

            switch( state )
            {
              case 0 :  // DCT 50's send down -1 for a texture handle.

                if ( !( TXTRHNDL_INRANGE(data)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
                // to fix potential access violation issues with NT Stress
                // verify we have a TXTRHNDL for this texture otherwise
                // set the texture to zero

                        && TXTRHNDL_PTR(data)
#endif
                      )
                    )
                {
                    data = 0;
                }

                ((LPDWORD)&pRc->textureStage[stage])[state_tmp] = data;
                break;

#if (DIRECT3D_VERSION < 0x0800) || (DX < 8)
              case D3DTSS_ADDRESS :
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;
#endif

              case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
              case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
              case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
              case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
                if(pRc->texMapBlend == 0x7ffffffe)
                  pRc->texMapBlend = D3DTBLEND_MODULATE;
		
                ((LPDWORD)&pRc->textureStage[stage])[state_tmp] = data;
                break;

			  case D3DTSS_TEXTURETRANSFORMFLAGS:
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;
 

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
              case D3DTSS_ADDRESSW: 
                break;
#endif
              default :
                ((LPDWORD)&pRc->textureStage[stage])[state_tmp] = data;
            }
			/*	Part of workaround for PRS 12099 - jmccartney 28/02/00
				texture stage state has changed so set texturestageStateValidated to FALSE
			*/
			if (!IS_NAPALM)
				pRc->txtStageStateValidated = FALSE;  // part of workaround for PRS 12099 
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          }
          else
          {
            if ((NULL != pRc->pCurrSB) &&
                (((IS_NAPALM) && (stage < NUMTEXTUREUNITS)) || ((IS_VOODOO3) && (stage < NUMTEXTUREUNITS+1))) &&
                (state <= MAX_TEXTURESTAGESTATES))
            {
              D3DPRINT(D3DDBGLVL, "    storing STATEBLOCK stage=%ld  state=%ld  data=%ld", stage, state, data);
              pRc->pCurrSB->uc.TssStates[stage][state] = data;
              // set flag to mark this texture stage state is stored in state block
              SET_SB_TSS_FLAG(pRc->pCurrSB,stage,state);
            }
#if DBG
            else
            {
              D3DPRINT(D3DDBGLVL, "    ignoring STATEBLOCK stage=%ld  state=%ld  data= %ld", stage, state, data);
            }
#endif
          }
#endif

            ((LPDWORD)&pRc->textureStage[stage])[TSS_CHANGED] = 1;
          #if defined (NULLDRIVER)
          } /*ondrtState*/
          #endif
        }

        UPDATE_HW_STATE( SC_SOMETHING );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)lpPrim;
        hr = D3D_OK;
        break;
      }

      case D3DDP2OP_VIEWPORTINFO :
      {
#if defined(NEW_CLIP_FOR_GB) || defined(TnL_HAL)
        D3DHAL_DP2VIEWPORTINFO *pVpt;
        // Keep only the last viewport notification
        pVpt = (D3DHAL_DP2VIEWPORTINFO *)(lpCmd + 1) + (lpCmd->wStateCount - 1);
#endif

#ifdef  TnL_HAL
        // Update T&L viewport state
        pRc->tl.Viewport.dwX = pVpt->dwX;
        pRc->tl.Viewport.dwY = pVpt->dwY;
        pRc->tl.Viewport.dwWidth = pVpt->dwWidth;
        pRc->tl.Viewport.dwHeight = pVpt->dwHeight;
        pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWRECT;
#endif  //TnL_HAL

        D3DPRINT( 32, "  Command VIEWPORTINFO (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "  D3DDP2OP_VIEWPORTINFO");

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2VIEWPORTINFO, 1, 0);

        if (IS_NAPALM) { //cws gb
            DWORD dwX,dwY,dwWidth,dwHeight;

            dwX=((D3DHAL_DP2VIEWPORTINFO *)lpPrim)->dwX;
            dwY=((D3DHAL_DP2VIEWPORTINFO *)lpPrim)->dwY;
            dwWidth=((D3DHAL_DP2VIEWPORTINFO *)lpPrim)->dwWidth;
            dwHeight=((D3DHAL_DP2VIEWPORTINFO *)lpPrim)->dwHeight;

            if ((dwX+dwWidth)%2==1) { /* Adjust right to even boundary */
              dwWidth += 1;
            }

            if ((dwX+dwWidth) > 0xffe) { /*clipRight1 must <= 0xffe*/
              dwWidth = 0xffe-dwX;
            }
            if ((dwY+dwHeight) > 0xfff) { /*clipBottom1 must <= 0xfff*/
              dwHeight = 0xfff-dwY;
            }
            /* the left must be even - Taken care of by dwX&0xffe below*/
            pRc->sst.clipLeftRight1 = ((dwX&0xffe)<<16) | (((dwX+dwWidth)&0xffe)<<0);      //Left<Right
            pRc->sst.clipBottomTop1 = (((dwY+dwHeight)&0xfff)<<0) |  (((dwY)&0xfff)<<16);//Top<Bottom
#ifndef NEW_CLIP_FOR_GB
            UPDATE_HW_STATE( SC_BUFFERS );
#endif
            D3DPRINT(255,"VIEWPORTINFO %08x %08x %08x %08x",dwX,dwY,dwWidth,dwHeight );
        }
#ifdef NEW_CLIP_FOR_GB
        pRc->sst.clipLeftRight= (((pVpt->dwX                  )&0xfff)<<16) | (((pVpt->dwX + pVpt->dwWidth /*+1*/)&0xfff)<<0 ); //Take out +1 so bottom and right are exclusive of drawn pixels (not drawn).  
        pRc->sst.clipBottomTop= (((pVpt->dwY+ pVpt->dwHeight /*+1*/)&0xfff)<<0 ) | (((pVpt->dwY                   )&0xfff)<<16);
        UPDATE_HW_STATE( SC_BUFFERS );
#endif
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2VIEWPORTINFO*)(lpPrim) + 1);
        break;
      }

      case D3DDP2OP_WINFO :
      {
#ifdef  DCT_FIX
       float scale;
#endif

        D3DPRINT( 32, "  Command WINFO (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "  D3DDP2OP_WINFO");

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2WINFO, 1, 0);

        pRc->minW = ((LPD3DHAL_DP2WINFO)lpPrim)->dvWNear;
        pRc->maxW = ((LPD3DHAL_DP2WINFO)lpPrim)->dvWFar;

		/* The following line fixes a problem with Roll Cage II on the Voodoo 3, we were
		   getting passed W-Buffering values to large for the hardware (ie. 24/32-bit)
		   test to see if the max. value is greater than 16-bit value and clamp) */
		if (!IS_NAPALM)
			if (pRc->maxW > WMAX) pRc->maxW = WMAX ; // Workaround for PRS 12663

#ifdef DCT_FIX
// RAJ 8/2/99 Fix for WHQL DCT perspective correction test
   if (pRc->minW > WMIN)
   {
     if (pRc->maxW < WMAX)
      scale = WMAX / pRc->maxW;
     else
     {
      float f1 = WMIN / pRc->minW;
      float f2 = WMAX / pRc->maxW;

      if (f1 > f2)
        scale = f1;
      else
        scale = f2;
     }
   }
   else
   {
     if (pRc->maxW < WMAX)
     {
      float f1 = WMIN / pRc->minW;
      float f2 = WMAX / pRc->maxW;

      if (f1 > f2)
        scale = f1;
      else
        scale = f2;
     }
     else
     {
      scale = 1;
      OutputDebugString("ddiDrawPrimitives2(..) - Application set bad w range.  Try minW = 1.f, maxW = 100.f");
     }
   }

   pRc->scaleW = 1.f / scale;

        D3DPRINT(D3DDBGLVL+1, "    minW=%lX, maxW=%lX, scaleW=%lX",
                 *(ULONG *)&pRc->minW, *(ULONG *)&pRc->maxW);

#else

        if ((WMIN > pRc->minW) || (WMAX < pRc->maxW))
        {
          // Here we are doing a linear scale of all incoming rhw's
          // in the range minW to maxW into a scaled rhw for the range
          // the hw accepts.
          // So the aW and bW equations can be derived from this:
          // 
          //             1                     1
          //     RHW - ----      scaledRHW - ----
          //           minW                  WMIN
          //    ------------  =  -----------------
          //      1      1            1      1
          //    ---- - ----         ---- - ----
          //    maxW   minW         WMAX   WMIN
          // 
          // so if you do a bunch of rearranging you get
          // 
          //                      1      1
          //                    ---- - ----
          //               1    WMAX   WMIN            1
          // scaledRHW - ---- = ----------- * (RHW - ----)
          //             WMIN     1      1           minW
          //                    ---- - ----
          //                    maxW   minW
          // 
          // and with more rearranging
          // 
          //               minW * maxW       1      1             1      1      minW * maxW       1      1
          // scaledRHW =  ------------- * (---- - ----) * RHW + ---- - ---- * (------------- * (---- - ----))
          //               minW - maxW     WMAX   WMIN          WMIN   minW     minW - maxW     WMAX   WMIN
          // 
          // 
          // so then we end up with
          // 
          // scaledRHW = aW * RHW + bW
          // 
          // where
          // 
          //       minW * maxW       1      1
          // aW = ------------- * (---- - ----)
          //       minW - maxW     WMAX   WMIN
          // 
          // and
          //        1      1
          // bW = ---- - ---- * aW
          //      WMIN   minW

          pRc->aW = ((pRc->minW * pRc->maxW) / (pRc->minW - pRc->maxW)) * ((1.f / WMAX) - 1.f);
          pRc->bW = (1.f / WMIN) - (pRc->aW / pRc->minW);
        }
        else
        {
          pRc->aW = 1.0f;
          pRc->bW = 0.0f;
        }
        D3DPRINT(D3DDBGLVL, "    minW=%lX, maxW=%lX, aW=%lX, bW=%lX",
                 *(ULONG *)&pRc->minW, *(ULONG *)&pRc->maxW,
                 *(ULONG *)&pRc->aW,   *(ULONG *)&pRc->bW);

#endif


    #if 0
      #if defined( DEBUG )
        {
          char minW[24], maxW[24], aW[24], bW[24];

          strcpy( minW, float2String(pRc->minW) );
          strcpy( maxW, float2String(pRc->maxW) );
          strcpy( aW, float2String(pRc->aW) );
          strcpy( bW, float2String(pRc->bW) );

          D3DPRINT( 58, "minW=%s, maxW=%s, aW=%s, bW=%s", minW, maxW, aW, bW );
        }
      #endif
    #endif
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2WINFO*)(lpPrim) + 1);
        break;
      }

      // Rendering of triangles

      // Triangle list
      case D3DDP2OP_TRIANGLELIST :
      {
        D3DPRINT( 32, "  Command TRIANGLELIST (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLELIST, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart + 3 * lpCmd->wPrimitiveCount - 1);

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TRIANGLELIST (wPrimitiveCount=%ld, wVStart=%ld)",
                 lpCmd->wPrimitiveCount, ((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart);

#ifndef TnL_HAL
        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart);

#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
        {
          LPD3DTLVERTEX   lpV0, lpV1, lpV2;


          lpV0 = (LPD3DTLVERTEX)lpVOffset;

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            lpV1 = LP_FVF_NXT_VTX(lpV0);
            lpV2 = LP_FVF_NXT_VTX(lpV1);

            DumpFVFData("lpV0", lpV0);
            DumpFVFData("lpV1", lpV1);
            DumpFVFData("lpV2", lpV2);

            lpV0 = LP_FVF_NXT_VTX(lpV2);
          }
        }
#endif
#endif // TnL_HAL

#ifdef TnL_HAL

        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumVertices = lpCmd->wPrimitiveCount*3;
          DWORD dwVStart = (DWORD)((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");
   
          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                D3DPT_TRIANGLELIST, lpVBSurfData );

          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLELIST*)(lpCmd + 1) + 1);
            hr = D3D_OK;
            break;
          }

          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
          UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Read the vertex offset value and adjust the primtive data pointer
           lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((D3DHAL_DP2TRIANGLELIST *)lpPrim)->wVStart);

#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
           {
             LPD3DTLVERTEX   lpV0, lpV1, lpV2;
   

             lpV0 = (LPD3DTLVERTEX)lpVOffset;

             for (i = lpCmd->wPrimitiveCount; i > 0; i--)
             {
               lpV1 = LP_FVF_NXT_VTX(lpV0);
               lpV2 = LP_FVF_NXT_VTX(lpV1);
   
               DumpFVFData("lpV0", lpV0);
               DumpFVFData("lpV1", lpV1);
               DumpFVFData("lpV2", lpV2);
   
               lpV0 = LP_FVF_NXT_VTX(lpV2);
             }
           }
#endif
        }
#endif //TnL_HAL

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 0, lpCmd->wPrimitiveCount);

        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2TriangleAllFill( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );
        else
          dp2TriangleAll( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLELIST*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed triangle list
      case D3DDP2OP_INDEXEDTRIANGLELIST :
      {
        D3DPRINT( 32, "  Command INDEXEDTRIANGLELIST (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2INDEXEDTRIANGLELIST, lpCmd->wPrimitiveCount, 0);

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDTRIANGLELIST (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        if (!(lpCmd->wPrimitiveCount))
        {
           lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
           hr = D3D_OK;
           break;
        }

#ifdef WINNT
        {
          LPBYTE  lpPrim2 = lpPrim;

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2INDEXEDTRIANGLELIST *)lpPrim2)->wV1);
            CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2INDEXEDTRIANGLELIST *)lpPrim2)->wV2);
            CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2INDEXEDTRIANGLELIST *)lpPrim2)->wV3);

            lpPrim2 += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST);
          }
        }
#endif

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 1, lpCmd->wPrimitiveCount );

        // Handle case where pRc->fillMode == D3DFILL_WIREFRAME
        if (pRc->fillMode == D3DFILL_WIREFRAME)
        {
          WORD          wFlags;
          LPD3DTLVERTEX lpV0, lpV1, lpV2;
          LPBYTE        lpPrim2 = lpPrim;

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {		
            //Get wFlags for edge detection
            wFlags  = ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim2)->wFlags;

            //Get Vertices
            lpV0 = LP_FVF_VERTEX(lpVertices, ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim2)->wV1);
            lpV1 = LP_FVF_VERTEX(lpVertices, ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim2)->wV2);
            lpV2 = LP_FVF_VERTEX(lpVertices, ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim2)->wV3);
            
            if (!CULL_TRI(pRc, lpV0, lpV1, lpV2))
            {
              if ( wFlags & D3DTRIFLAG_EDGEENABLE1 )
                dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV0, (LPVOID)lpV1, pRc->dwVertexType);
              
              if ( wFlags & D3DTRIFLAG_EDGEENABLE2 )
                dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV1, (LPVOID)lpV2, pRc->dwVertexType);
              
              if ( wFlags & D3DTRIFLAG_EDGEENABLE3 )
                dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV2, (LPVOID)lpV0, pRc->dwVertexType);
            }
                
            lpPrim2 += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST);				
          }
			
          // Update the command stream pointer
          lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
          hr = D3D_OK;
          break;			    		
        }

        if( pRc->specialModes )
        {
          D3DPRINT( 255, "Special Mode %ld - INDEXEDTRIANGLELIST", pRc->specialModes );

          // Draw the triangle list
          if ( pRc->fillMode != D3DFILL_SOLID )
            dp2IdxTriangleAllFill( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );
          else
            dp2IdxTriAll_SM3( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );

          // Update the command stream pointer
          lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
          hr = D3D_OK;
          break;
        }

#if ENABLE_ASM
#ifdef  TnL_HAL
        // BAJ 10/12/1999 Check both the state and the Clipping Union Flag to see if it is safe 
        // to call the ASM code.
        if (pRc->state & FLAGS_BAD_EXECUTE_CASE || (pRc->tl.clipUnion))
#else
        // if bW != 0.0, then we are w scaling
        // no support in amesh2.asm for w scaling
        if ((pRc->state & FLAGS_BAD_EXECUTE_CASE) || (0.0f != pRc->bW))
#endif
#endif
        {
           if ( pRc->fillMode != D3DFILL_SOLID )
             dp2IdxTriangleAllFill( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVertices, pRc->dwVertexType );
           else
             dp2IdxTriangleAll( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVertices, pRc->dwVertexType );

           // Update the command stream pointer
           lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
           hr = D3D_OK;
           break;
        }

#if ENABLE_ASM
        ASM_SET_RC(pRc);
        pRc->DrawMeshAsm(lpCmd->wPrimitiveCount,(FxU32 *)lpPrim,lpVertices);
        ASM_SYNC();

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
        hr = D3D_OK;
        break;
#endif
      }

      // Indexed triangle list2
      case D3DDP2OP_INDEXEDTRIANGLELIST2 :
      {
        D3DPRINT( 32, "  Command INDEXEDTRIANGLELIST2 (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDTRIANGLELIST2 (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);


        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2INDEXEDTRIANGLELIST2, lpCmd->wPrimitiveCount, sizeof(D3DHAL_DP2STARTVERTEX));

#ifdef WINNT
        {
          LPBYTE  lpPrim2;
          WORD    wIndxBase;

          wIndxBase = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;
          lpPrim2 = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            DumpFVFData("lpV0", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV1));
            DumpFVFData("lpV1", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV2));
            DumpFVFData("lpV2", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV3));

            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV1);
            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV2);
            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLELIST2 *)lpPrim2)->wV3);

            lpPrim2 += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2);
          }
        }
#endif

#ifndef  TnL_HAL
        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);
#endif  // TnL_HAL

        if (!(lpCmd->wPrimitiveCount))
        {
           lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim);
           hr=D3D_OK;
           break;
        }
#ifdef  TnL_HAL

        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumIndices = lpCmd->wPrimitiveCount*3;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");
   
          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SaveIdxPrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, (UINT) lpdp2d->dwVertexLength,
                                D3DPT_TRIANGLELIST , (LPWORD) lpPrim, (UINT) dwNumIndices, lpVBSurfData );

#ifdef NULL_TNL		// NULL point for T&L HAL for 3D Winbench 2000
          {
  
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDTRIANGLELIST2, lpCmd->wPrimitiveCount, 2);
            hr = D3D_OK;
            break;
          }
#else
          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDTRIANGLELIST2, lpCmd->wPrimitiveCount, 2);
            hr = D3D_OK;
            break;
          }
#endif
        
          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
		  UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
          lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);
        }
        else
        {
           // Read the vertex offset value and adjust the primtive data pointer
          lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
          lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);
        }
#endif  //TnL_HAL

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 2, lpCmd->wPrimitiveCount );

   #if defined(NULLDRIVER)
      if (_D3(ondrtTri)) {    
   #endif

        if( pRc->specialModes )
        {
          D3DPRINT( 255, "Special Mode %ld - INDEXEDTRIANGLELIST2", pRc->specialModes );

          // Draw the triangle list
          if ( pRc->fillMode != D3DFILL_SOLID )
            dp2IdxTriangle2AllFill( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );
          else
            dp2IdxTri2All_SM3( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );

          // Update the command stream pointer
          lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2)));
          hr = D3D_OK;
          break;
        }

        
#if ENABLE_ASM        
#if (NEWASMTRI == 1) // && !defined(WINNT)  shouston 1-29-00 Now supported under Win2K
/*******************************************************************************/
/*  New Asm Triangle Rendering routines						                   */
/*  																		   */
/*  Supports:																   */
/*    Multitexturing														   */
/*    Specular																   */
/*    Flat Shading															   */
/*    Wrap																	   */
/*    Fog                                                                      */
/*    Dynamic Hardware Setupflags                                              */
/*  																		   */
/*  The following sets up the variables needed in the shared data structure of */
/*  asmfifodata.  These variables are needed for triangle rendering with the   */
/*  new assembler triangle rendering routines in 3DNow and KNI				   */
/*                                                                             */
/*  Replaces the C-Code for the majority of the rendering cases                */
/*******************************************************************************/
//      ASM_SET_RC(pRc);

#ifdef WINNT
        _asm_data.fifo_ptr  = (FxU32 *)ppdev->fifoData.fifoPtr;         
        _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToEnd;         
        if (ppdev->fifoData.roomToReadPtr < ppdev->fifoData.roomToEnd)  
          _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToReadPtr;   
        _asm_data.fifo_room /= sizeof(DWORD);                           
        _asm_data.pRc = pRc;                                            
        _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;                
#else        
        _asm_data.fifo_ptr=(FxU32 *)(CMDFIFOPTR);
        _asm_data.fifo_room=(FxI32)(CMDFIFOSPACE);
        _asm_data.pRc = pRc;
        _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<<2;
#endif                        
		_asm_data.t1_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t1CoordIndex)<<2;
		_asm_data.Specular_Offset=(fvfOffsetTable[pRc->dwVertexType].specular)<<2;
		_asm_data.scale_z=pRc->zScale;
        _asm_data.rhw_offset = (fvfOffsetTable[pRc->dwVertexType].rhw)<<2;
		_asm_data.z_offset = (fvfOffsetTable[pRc->dwVertexType].sz)<<2;
		_asm_data.color_offset = (fvfOffsetTable[pRc->dwVertexType].color)<<2;
 		_asm_data.FVF_Size=(fvfOffsetTable[pRc->dwVertexType].size);
        _asm_data.Scale_W_AW = pRc->aW;
		_asm_data.Scale_W_BW = pRc->bW;
		_asm_data.CMD_Start = (CMDFIFO_BUILD_PK3( CMD_START, 3, pRc->sst.sSetupMode, 1 ) | (1<<22));
        _asm_data.CMD_Cont = (CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 ));
        _asm_data.CMD_Tri = (CMDFIFO_BUILD_PK3( CMD_TRI, 3, pRc->sst.sSetupMode, 1 ));
		_asm_data.CMD_ALPHA_BLEND_SPECULAR = (CMDFIFO_BUILD_PK3( CMD_START, 3, pRc->sst.sSetupMode, 1 ) | (1<<22));
		_asm_data.CMD_NO_ALPHA_BLEND_SPECULAR = (CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode & 
		                                        ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)),1 ) | 
		                                        (1<<22));
     	_asm_data.CMD_CONT_ALPHA_BLEND_SPECULAR = (CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 ));
		_asm_data.CMD_CONT_NO_ALPHA_BLEND_SPECULAR = (CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode & 
		                                        ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)),1 ));
    	_asm_data.SetupFlag = pRc->sst.sSetupMode;
        _asm_data.RenderState = pRc->state; // States needed

/*******************************************************************************/
/* Rejection Logic                                                             */
/* 																			   */
/* If ( Katmai and pRc->state is not ( FLAGS_FVF_IDX2 OR STATE_REQUIRES_AA OR  */
/*                                     STATE_NOT_SOLID_FILL )				   */
/*	 Call the Katmai Code													   */
/* Else "We are not a Katmai CPU, so we are 3DNow or FPU"                      */
/*   If ( 3DNow and pRc->state is not ( STATE_REQUIRES_AA OR 				   */
/*	                                    STATE_NOT_SOLID_FILL )            	   */
/*     Call the 3DNow Code                                                     */
/*   Else "We are not a Katmai or 3DNow CPU, so lets call the FPU code"        */
/*     If ( pRc->state is not ( FLAGS_BAD_TRI_IDX2_CASE ) &&                   */
/*          (pRc->wrapT0 || pRc->wrapT1))                                      */
/*       Call the FPU Code                                                     */
/*     Else "We have a state flag that is not supported by the old FPU code,   */
/*           lets just not call the assembler and call the C-Code"             */
/* 																			   */
/*******************************************************************************/
#ifdef  TnL_HAL
        // BAJ 10/12/1999 Check both the state and the clipping union flag to see if it is safe 
        // to call the ASM code.
        if ( (
        #if (STBKNI==1)
             ((CPUTYPE & P6_INTELCPU_WITH_KNI) && !(pRc->state & (FLAGS_FVF_IDX2 | STATE_REQUIRES_AA | STATE_NOT_SOLID_FILL)) && (pRc->zEnable)) ||
		#endif
		#ifdef K6_2
             ((_3DNowAllowed) && !(pRc->state & (STATE_REQUIRES_AA | STATE_NOT_SOLID_FILL)) && (pRc->zEnable)) ||
		#endif
             (!(pRc->state & FLAGS_BAD_TRI_IDX2_CASE) && 
              !(pRc->wrapT0 || pRc->wrapT1))) && 
             !pRc->tl.clipUnion )
#else
        if ( (
        #if (STBKNI==1)
             ((CPUTYPE & P6_INTELCPU_WITH_KNI) && !(pRc->state & (FLAGS_FVF_IDX2 | STATE_REQUIRES_AA | STATE_NOT_SOLID_FILL)) && (pRc->zEnable)) ||           
		#endif
		#ifdef K6_2
             ((_3DNowAllowed) && !(pRc->state & (STATE_REQUIRES_AA | STATE_NOT_SOLID_FILL)) && (pRc->zEnable)) ||
		#endif
             (!(pRc->state & FLAGS_BAD_TRI_IDX2_CASE) && 
              !(pRc->wrapT0 || pRc->wrapT1) && (pRc->bW==0.0f) )) ) //jcochrane - can't use asm if w scaling
#endif
        {
           lpCmd=(LPD3DHAL_DP2COMMAND)(pRc->DrawTriIdx2(lpCmd->wPrimitiveCount,(FxU32 *)lpPrim,lpVOffset));
           ASM_SYNC();

           hr = D3D_OK;
           break;
        }
#else // NOT ( NEWASMTRI )
        ASM_SET_RC(pRc);

#if 1
#ifdef  TnL_HAL
        // BAJ 10/12/1999 Check both the state and the clipping union flag to see if it is safe 
        // to call the ASM code.
        if (!(pRc->state & FLAGS_BAD_TRI_IDX2_CASE) && !pRc->tl.clipUnion 
            && !(pRc->wrapT0 || pRc->wrapT1))
#else
        // if bW != 0.0, then we are w scaling
        // no support in atri.asm for w scaling
        if (!(pRc->state & FLAGS_BAD_TRI_IDX2_CASE) 
            && !(pRc->wrapT0 || pRc->wrapT1) && (0.0f == pRc->bW))
#endif
        {
           lpCmd=(LPD3DHAL_DP2COMMAND)(pRc->DrawTriIdx2(lpCmd->wPrimitiveCount,(FxU32 *)lpPrim,lpVOffset));
           ASM_SYNC();

           hr = D3D_OK;
           break;
        }

#endif
#endif // NEWASMTRI && !WINNT
#endif

        // Draw the triangle list
        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2IdxTriangle2AllFill( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );
        else
          dp2IdxTriangle2All( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );

   #if defined(NULLDRIVER)
      }/*ondrtTri*/
   #endif

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2)));
        hr = D3D_OK;
        break;
      }

      // Triangle strip
      case D3DDP2OP_TRIANGLESTRIP :
      {
        D3DPRINT( 32, "  Command TRIANGLESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

#ifdef  TnL_HAL

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLESTRIP, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart + lpCmd->wPrimitiveCount + 1);

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TRIANGLESTRIP (wPrimitiveCount=%ld, wVStart=%ld)",
                 lpCmd->wPrimitiveCount,
                 ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);

        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumVertices = lpCmd->wPrimitiveCount + 2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                D3DPT_TRIANGLESTRIP, lpVBSurfData);

          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLESTRIP*)(lpCmd + 1) + 1);
            hr = D3D_OK;
            break;
          }

          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
		  UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Read the vertex offset value and adjust the primtive data pointer
           lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);
        }

#endif  //TnL_HAL

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 3, lpCmd->wPrimitiveCount );

#ifndef TnL_HAL
        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLESTRIP, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart + lpCmd->wPrimitiveCount + 1);

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TRIANGLESTRIP (wPrimitiveCount=%ld, wVStart=%ld)",
                 lpCmd->wPrimitiveCount,
                 ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart);
#endif  //~TnL_HAL

#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
        {
          LPD3DTLVERTEX   lpV0, lpV1, lpV2;


          lpV0 = (LPD3DTLVERTEX)lpVOffset;
          lpV1 = lpV2 = LP_FVF_NXT_VTX(lpV0);

          for (i = 0; i < lpCmd->wPrimitiveCount; i++)
          {
            lpV2 = LP_FVF_NXT_VTX(lpV2);

            DumpFVFData("lpV0", lpV0);
            DumpFVFData("lpV1", lpV1);
            DumpFVFData("lpV2", lpV2);

            if (i & 1)
              lpV1 = lpV2;
            else
              lpV0 = lpV2;
          }
        }
#endif

        // Draw the fans
        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2StripAllFill( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );
        else
          dp2StripAll( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLESTRIP*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // triangle indexed strip
      case D3DDP2OP_INDEXEDTRIANGLESTRIP :
      {
        D3DPRINT( 32, "  Command INDEXEDTRIANGLESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDTRIANGLESTRIP (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);


        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 4, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, WORD, lpCmd->wPrimitiveCount + 2, sizeof(D3DHAL_DP2STARTVERTEX));

#ifdef WINNT
        {
          LPBYTE  lpPrim2;
          WORD    wIndxBase;


          wIndxBase = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;
          lpPrim2 = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

          DumpFVFData("lpV0", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[0]));
          DumpFVFData("lpV1", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[1]));

          CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[0]);
          CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[1]);

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            DumpFVFData("lpV2", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[2]));

            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLESTRIP *)lpPrim2)->wV[2]);
            lpPrim2 += sizeof(WORD);
          }
        }
#endif

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumIndices = lpCmd->wPrimitiveCount + 2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");
   
          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SaveIdxPrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, (UINT) lpdp2d->dwVertexLength,
                                D3DPT_TRIANGLESTRIP , (LPWORD) lpPrim, (UINT) dwNumIndices, lpVBSurfData );

          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDTRIANGLESTRIP, lpCmd->wPrimitiveCount, 2);
            hr = D3D_OK;
            break;
          }
        
          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
		  UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Read the vertex offset value and adjust the primtive data pointer
           lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        }
#else   // TnL_HAL

        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);

#endif  //TnL_HAL

        // Read the vertex offset value and adjust the primtive data pointer
        lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

        // Draw the fans
        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2IdxStripAllFill( pRc, lpCmd->wPrimitiveCount, (LPWORD)lpPrim, lpVOffset, pRc->dwVertexType );
        else
          dp2IdxStripAll( pRc, lpCmd->wPrimitiveCount, (LPWORD)lpPrim, lpVOffset, pRc->dwVertexType );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPWORD)lpPrim + (lpCmd->wPrimitiveCount + 2));
        hr = D3D_OK;
        break;
      }

      // Rendering fans

      // Immediate data fans with data embedded in the command stream
      case D3DDP2OP_TRIANGLEFAN_IMM :
      {
        D3DPRINT( 32, "  Command TRIANGLEAN_IMM (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TRIANGLEFAN_IMM (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          LPBYTE lpPrimFake = (LPBYTE)(((DWORD)(lpCmd + 1) + 3 ) & ~3);  

          lpPrimFake += sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

          // BobJ Temporary
          // We aren't rendering yet, just update the comand pointer,
          // set the return value and break out out this primitive
          // to go to the next one in the command stream
          lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrimFake + ((lpCmd->wPrimitiveCount + 2) * FVFO_SIZE) ); 
          hr = D3D_OK;
          break;
        }

#endif  //TnL_HAL

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 5, lpCmd->wPrimitiveCount );

        // Check primtive data pointer in command stream is DWORD aligned
        lpPrim = (LPBYTE)(((DWORD)(lpCmd + 1) + 3 ) & ~3);

        // check validity of first struct
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLEFAN_IMM, 1, 0);

        D3DPRINT(D3DDBGLVL, "Edge Flag 0x%08lx", ((LPD3DHAL_DP2TRIANGLEFAN_IMM)lpPrim)->dwEdgeFlags);

        // Get Edge flags (we still have to process them)
        dwEdgeFlags = ((LPD3DHAL_DP2TRIANGLEFAN_IMM)lpPrim)->dwEdgeFlags;

        // Move primitive data pointer past edge flags
        lpPrim += sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);

        // check validity of rest of command buffer
        CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim, FVFO_SIZE * sizeof(DWORD), lpCmd->wPrimitiveCount + 2, 0);

#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
        {
          LPD3DTLVERTEX   lpV0, lpV1, lpV2;


          lpV0 = (LPD3DTLVERTEX)lpPrim;
          lpV1 = LP_FVF_NXT_VTX(lpV0);
          lpV2 = LP_FVF_NXT_VTX(lpV1);

          for (i = 0; i < lpCmd->wPrimitiveCount; i++)
          {
            DumpFVFData("lpV0", lpV0);
            DumpFVFData("lpV1", lpV1);
            DumpFVFData("lpV2", lpV2);

            lpV1 = lpV2;
            lpV2 = LP_FVF_NXT_VTX(lpV2);
          }
        }
#endif
   
   #if defined(NULLDRIVER)
      if (_D3(ondrtTri)) {
   #endif

        if (D3DFILL_WIREFRAME == pRc->fillMode)
        {
          LPD3DTLVERTEX   lpV0, lpV1, lpV2;

          lpV0 = (LPD3DTLVERTEX)lpPrim;
          lpV1 = LP_FVF_NXT_VTX(lpV0);
          lpV2 = LP_FVF_NXT_VTX(lpV1);

          // dwEdgeFlags is a bit sequence representing the edge
          // flag for each one of the outer edges of the 
          // triangle fan

          for (i = 0; i < lpCmd->wPrimitiveCount; i++)
          {
            if (! CULL_TRI(pRc, lpV0, lpV1, lpV2))
            {
              if (0 == i)
              {
                // first triangle fan edge
                if (0x0001 & dwEdgeFlags)
                  dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV0, (LPVOID)lpV1, pRc->dwVertexType);
			
                dwEdgeFlags >>= 1;
              }

              if (0x0001 & dwEdgeFlags)
                dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV1, (LPVOID)lpV2, pRc->dwVertexType);

              dwEdgeFlags >>= 1;

              if ((DWORD)(lpCmd->wPrimitiveCount - 1) == i)
              {
                // last triangle fan edge
                if (0x0001 & dwEdgeFlags)
                  dp2Line(pRc, (LPVOID)lpV0, (LPVOID)lpV2, (LPVOID)lpV0, pRc->dwVertexType);
              }
            }

            lpV1 = lpV2;
            lpV2 = LP_FVF_NXT_VTX(lpV2);
          }

          // Update the command stream pointer
          lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 2) * FVFO_SIZE) );
          hr = D3D_OK;
          break;
        }

        if( pRc->specialModes )
        {
          D3DPRINT( 255, "Special Mode %ld - TRIANGLEFAN_IMM", pRc->specialModes );

          // Draw the triangle list
          if ( pRc->fillMode != D3DFILL_SOLID )
            dp2IdxTriangle2AllFill( pRc, lpCmd->wPrimitiveCount, lpPrim, lpVOffset, pRc->dwVertexType );
          else
            dp2FanAll_SM3( pRc, lpCmd->wPrimitiveCount, NULL, (LPDWORD)lpPrim, pRc->dwVertexType );

          // Update the command stream pointer
          lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 2) * FVFO_SIZE) );
          hr = D3D_OK;
          break;
        }

        ASM_SET_RC(pRc);

#if ENABLE_ASM
#ifdef  TnL_HAL
        // BAJ 10/12/1999 Check both the state and clipping union flags to see if it is safe 
        // to call the ASM code.
        if (!(pRc->state & FLAGS_BAD_FAN_IMM_CASE) && !pRc->tl.clipUnion)
#else
		//jcochrane - if bW!=0.0, then we are w scaling
		//no support in afan.asm for w scaling yet
        if (!(pRc->state & FLAGS_BAD_FAN_IMM_CASE) && (pRc->bW==0.0f) )
#endif
        {
           lpCmd=(LPD3DHAL_DP2COMMAND)DrawTriFanImmAsm(lpCmd->wPrimitiveCount,(FxU32 *)lpPrim,_asm_data.vertex_size);
           ASM_SYNC();

           hr = D3D_OK;
           break;
        }
#endif

        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2FanAllFill( pRc, lpCmd->wPrimitiveCount, NULL, (LPDWORD)lpPrim, pRc->dwVertexType );
        else
          dp2FanAll( pRc, lpCmd->wPrimitiveCount, NULL, (LPDWORD)lpPrim, pRc->dwVertexType );

    #if defined(NULLDRIVER)
      }/*ondrtTri*/
   #endif

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 2) * FVFO_SIZE) );
        hr = D3D_OK;
        break;
      }

      // Fans
      case D3DDP2OP_TRIANGLEFAN :
      {
        D3DPRINT( 32, "  Command TRIANGLEFAN (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TRIANGLEFAN (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

#ifdef  TnL_HAL

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLEFAN, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLEFAN *)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLEFAN *)lpPrim)->wVStart + lpCmd->wPrimitiveCount + 1);


        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumVertices = lpCmd->wPrimitiveCount + 2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2TRIANGLEFAN)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                D3DPT_TRIANGLEFAN, lpVBSurfData);

          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLEFAN*)(lpCmd + 1) + 1);
            hr = D3D_OK;
            break;
          }

          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
		  UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;

        }
        else
        {
           // Read the first vertex offset value
           lpVOffset = lpVertices + (((LPD3DHAL_DP2TRIANGLEFAN)lpPrim)->wVStart * FVFO_SIZE);
        }

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 6, lpCmd->wPrimitiveCount );

#else  //TnL_HAL
        // Setup the hardware state
//        if( HW_STATE_CHANGED )
//          setDX6state( pRc, 6, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLEFAN, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLEFAN *)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2TRIANGLEFAN *)lpPrim)->wVStart + lpCmd->wPrimitiveCount + 1);

        // Read the first vertex offset value
        lpVOffset = lpVertices + (((LPD3DHAL_DP2TRIANGLEFAN)lpPrim)->wVStart * FVFO_SIZE);

		// If we have a 4 vertex triangle fan and it's a square, then disable
		// AA for this primitive only to avoid one-pixel wide line artifacts.
		if ( (2 == lpCmd->wPrimitiveCount) && (_DD(ddAAModeEnabled) || (_DD(ddAANumberSamples) > 0)) )
		{
			LPDWORD	v1, v2, v3, v4;
			LPDWORD	verts = lpVOffset;

			v1 = verts;
			verts += FVFO_SIZE;
			v2 = verts;
			verts += FVFO_SIZE;
			v3 = verts;
			verts += FVFO_SIZE;
			v4 = verts;

			// Determine if we have a square. If so, disable AA.
			// dwZeroJitter is checked in handleSLIAA when setting aaCtrl.
			if (
				( (FLTP(v1)[FVFO_SX] == FLTP(v2)[FVFO_SX]) &&
				  (FLTP(v2)[FVFO_SY] == FLTP(v3)[FVFO_SY]) &&
				  (FLTP(v3)[FVFO_SX] == FLTP(v4)[FVFO_SX]) &&
				  (FLTP(v4)[FVFO_SY] == FLTP(v1)[FVFO_SY]) )   ||
				( (FLTP(v1)[FVFO_SY] == FLTP(v2)[FVFO_SY]) &&
				  (FLTP(v2)[FVFO_SX] == FLTP(v3)[FVFO_SX]) &&
				  (FLTP(v3)[FVFO_SY] == FLTP(v4)[FVFO_SY]) &&
				  (FLTP(v4)[FVFO_SX] == FLTP(v1)[FVFO_SX]) )
			   )
			{
				// If we already zero'd the jitter values,
				// then no need to redo.
				if (pRc->dwZeroJitter == 0)
				{
			        D3DPRINT(32, "  Setting dwZeroJitter to 1");
					pRc->dwZeroJitter = 1;
				    UPDATE_HW_STATE(SC_SOMETHING);
				}
			}
			else
			{
				// If we already wrote non-zero jitter values,
				// then no need to redo.
				if (pRc->dwZeroJitter == 1)
				{
			        D3DPRINT(32, "  Setting dwZeroJitter to 0");
					pRc->dwZeroJitter = 0;
				    UPDATE_HW_STATE(SC_SOMETHING);
				}
			}
		}

		// setup the hardware state.
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 6, lpCmd->wPrimitiveCount );
#endif // TnL_HAL

        // Draw the fans
        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2FanAllFill( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );
        else
          dp2FanAll( pRc, lpCmd->wPrimitiveCount, NULL, lpVOffset, pRc->dwVertexType );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLEFAN*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed Fans
      case D3DDP2OP_INDEXEDTRIANGLEFAN :
      {
        D3DPRINT( 32, "  Command INDEXEDTRIANGLEFAN (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDTRIANGLEFAN (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 7, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, WORD, lpCmd->wPrimitiveCount + 2, sizeof(D3DHAL_DP2STARTVERTEX));

#ifdef WINNT
        {
          LPBYTE  lpPrim2;
          WORD    wIndxBase;

          wIndxBase = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;
          lpPrim2 = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

          DumpFVFData("lpV0", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[0]));
          DumpFVFData("lpV1", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[1]));

          CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[0]);
          CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[1]);

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            DumpFVFData("lpV2", LP_FVF_VERTEX(lpVertices, wIndxBase+((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[2]));

            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDTRIANGLEFAN *)lpPrim2)->wV[2]);
            lpPrim2 += sizeof(WORD);
          }
        }
#endif

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumIndices = lpCmd->wPrimitiveCount + 2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");
   
          // Call on Save Primitive Data to initialize the T&L HAL with the proper info
          // for this particular primitive.
          SaveIdxPrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, (UINT) lpdp2d->dwVertexLength,
                                D3DPT_TRIANGLEFAN , (LPWORD) lpPrim, (UINT) dwNumIndices, lpVBSurfData );

          // Handle all TnL processing then call pass down to rendering code.
          if (ProcessPrimitive( pRc ) != D3D_OK)
          {
            // The return value from ProcessPrimitive is telling us that ALL of 
            // the primitive's vertices are clipped and we should be able
            // to trivially reject this primitive
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDTRIANGLEFAN, lpCmd->wPrimitiveCount, 2);
            hr = D3D_OK;
            break;
          }
        
          // We're going for it!  Let's Render this puppy!
          // Update the local Vertex Type with the new Output Vertex Type
		  UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
          lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Read the vertex offset value and adjust the primtive data pointer
           lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        }

#else   // TnL_HAL

        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);

#endif  //TnL_HAL

        lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

        // Draw the fans
        if ( pRc->fillMode != D3DFILL_SOLID )
          dp2IdxFanAllFill( pRc, lpCmd->wPrimitiveCount, (LPWORD)lpPrim, lpVOffset, pRc->dwVertexType );
        else
          dp2IdxFanAll( pRc, lpCmd->wPrimitiveCount, (LPWORD)lpPrim, lpVOffset, pRc->dwVertexType );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPWORD)lpPrim + (lpCmd->wPrimitiveCount + 2));
        hr = D3D_OK;
        break;
      }

      // Rendering of points

      case D3DDP2OP_POINTS :
      {
        D3DPRINT( 32, "  Command POINTS (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 8, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2POINTS, lpCmd->wPrimitiveCount, 0);

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_POINTS (wPrimitiveCount=%ld)", lpCmd->wPrimitiveCount);

        // Start our wPrimiveCount loop
        for(i = 0; i < lpCmd->wPrimitiveCount; i++)
        {
          D3DPRINT(D3DDBGLVL, "D3DDP2OP_POINTS (wCount=%ld, wVStart=%ld)",
                   ((LPD3DHAL_DP2POINTS)lpPrim)->wCount,
                   ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart);

#ifndef  TnL_HAL
          // Read the first vertex offset value
          lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart);
#endif   // TnL_HAL

          // Check first & last vertex
          CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2POINTS*)lpPrim)->wVStart);
          CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2POINTS*)lpPrim)->wVStart + ((D3DHAL_DP2POINTS*)lpPrim)->wCount - 1);

#ifdef  TnL_HAL
          // New for DX7, check to see if the passed Vertex is untransformed
          //  If so, call ProcessPrimitive and allow the TnL to be operated on
          //  the Raw Vertex data.  
          if (TRANSFORM_NEEDED)
          {
           DWORD dwNumVertices = (DWORD)((LPD3DHAL_DP2POINTS)lpPrim)->wCount;
           DWORD dwVStart = (DWORD)((LPD3DHAL_DP2POINTS)lpPrim)->wVStart;
 
             D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");
 
             // Call on Save Primitive Data to initialize the T&L HAL with the proper info
             // for this particular primitive.
             SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                      D3DPT_POINTLIST, lpVBSurfData );
 
             // Handle all TnL processing then call pass down to rendering code.
             if (ProcessPrimitive( pRc ) != D3D_OK)
             {
               // The return value from ProcessPrimitive is telling us that ALL of 
               // the primitive's vertices are clipped and we should be able
               // to trivially reject this primitive
               
               // Update the primitive data pointer to the next point primitive
               lpPrim += sizeof(D3DHAL_DP2POINTS);
               continue;
             }
 
             // We're going for it!  Let's Render this puppy!
             // Update the local Vertex Type with the new Output Vertex Type
		     UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
             lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
          }
          else
          {
             // Read the first vertex offset value
             lpVOffset = (ULONG *)LP_FVF_VERTEX(lpVertices, ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart);
          }

          // If there are any clipped vertices
          // better run it through the clipper
          if ( CLIPPING_NEEDED )
          {
           LPDWORD clipcodes = (LPDWORD) pRc->tl.pClipBuf;
           DWORD c0;

             // Start our point loop
             for( j = 0; j < ((LPD3DHAL_DP2POINTS)lpPrim)->wCount; j++ )
             {
#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
               D3DPRINT(D3DDBGLVL+1, "lpVOffset[%ld] - %08lXh %08lXh %08lXh %08lX %08lXh %08lXh %08lXh %08lX",
                        j + ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart,
                        INTP(lpVOffset)[FVFO_SX], INTP(lpVOffset)[FVFO_SY],
                        INTP(lpVOffset)[FVFO_SZ], INTP(lpVOffset)[FVFO_RHW],
                        (FVFO_COLOR    == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_COLOR],
                        (FVFO_SPECULAR == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_SPECULAR],
                        (FVFO_TU       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TU],
                        (FVFO_TV       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TV]);
#endif

                // Get the ClipCode for this point
                c0 = *clipcodes++;

                // Draw a point and update vertex pointer to the next vertex point
                if (c0 == 0)
                   dp2Point( pRc, lpVOffset, lpVOffset, pRc->dwVertexType );
                lpVOffset += FVFO_SIZE;
             }
          }
          else
          {
             // Start our point loop
             for( j = 0; j < ((LPD3DHAL_DP2POINTS)lpPrim)->wCount; j++ )
             {
#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
               D3DPRINT(D3DDBGLVL+1, "lpVOffset[%ld] - %08lXh %08lXh %08lXh %08lX %08lXh %08lXh %08lXh %08lX",
                        j + ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart,
                        INTP(lpVOffset)[FVFO_SX], INTP(lpVOffset)[FVFO_SY],
                        INTP(lpVOffset)[FVFO_SZ], INTP(lpVOffset)[FVFO_RHW],
                        (FVFO_COLOR    == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_COLOR],
                        (FVFO_SPECULAR == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_SPECULAR],
                        (FVFO_TU       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TU],
                        (FVFO_TV       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TV]);
#endif

                // Draw a point and update vertex pointer to the next vertex point
                dp2Point( pRc, lpVOffset, lpVOffset, pRc->dwVertexType );
                lpVOffset += FVFO_SIZE;
             }
          }

#else  //TnL_HAL

          // Start our point loop
          for( j = 0; j < ((LPD3DHAL_DP2POINTS)lpPrim)->wCount; j++ )
          {
#if defined(WINNT) && DBG && ENABLE_VERTEX_DUMP
            D3DPRINT(D3DDBGLVL+1, "lpVOffset[%ld] - %08lXh %08lXh %08lXh %08lX %08lXh %08lXh %08lXh %08lX",
                     j + ((LPD3DHAL_DP2POINTS)lpPrim)->wVStart,
                     INTP(lpVOffset)[FVFO_SX], INTP(lpVOffset)[FVFO_SY],
                     INTP(lpVOffset)[FVFO_SZ], INTP(lpVOffset)[FVFO_RHW],
                     (FVFO_COLOR    == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_COLOR],
                     (FVFO_SPECULAR == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_SPECULAR],
                     (FVFO_TU       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TU],
                     (FVFO_TV       == 128) ? 0xDEADDEAD : INTP(lpVOffset)[FVFO_TV]);
#endif

            // Draw a point and update vertex pointer to the next vertex point
            dp2Point( pRc, lpVOffset, lpVOffset, pRc->dwVertexType );
            lpVOffset += FVFO_SIZE;
          }

#endif  //TnL_HAL
          // Update the primitive data pointer to the next point primitive
          lpPrim += sizeof(D3DHAL_DP2POINTS);
        }

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)lpPrim;
        hr = D3D_OK;
        break;
      }

      // Rendering of lines

      // Line list
      case D3DDP2OP_LINELIST :
      {
        D3DPRINT( 32, "  Command LINELIST (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_LINELIST (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 9, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2LINELIST, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2LINELIST *)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2LINELIST *)lpPrim)->wVStart + 2 * lpCmd->wPrimitiveCount - 1);

#ifndef  TnL_HAL
        // Get the start vertex offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2LINELIST)lpPrim)->wVStart * FVFO_SIZE);
#endif   // TnL_HAL

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
         DWORD dwNumVertices = lpCmd->wPrimitiveCount*2;
         DWORD dwVStart = (DWORD)((LPD3DHAL_DP2LINELIST)lpPrim)->wVStart;

           D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

           // Call on Save Primitive Data to initialize the T&L HAL with the proper info
           // for this particular primitive.
           SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                 D3DPT_LINELIST, lpVBSurfData );

           // Handle all TnL processing then call pass down to rendering code.
           if (ProcessPrimitive( pRc ) != D3D_OK)
           {
             // The return value from ProcessPrimitive is telling us that ALL of 
             // the primitive's vertices are clipped and we should be able
             // to trivially reject this primitive
             NEXTINSTRUCTION(lpCmd, D3DHAL_DP2LINELIST, 1, 0);
             hr = D3D_OK;
             break;
           }

           // We're going for it!  Let's Render this puppy!
           // Update the local Vertex Type with the new Output Vertex Type
		   UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
           lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Get the start vertex offset and update the primitive data pointer
           lpVOffset = lpVertices + (((LPD3DHAL_DP2LINELIST)lpPrim)->wVStart * FVFO_SIZE);
        }
#endif  //TnL_HAL

        lpPrim += sizeof(D3DHAL_DP2LINELIST);

#ifdef TnL_HAL
        
        // If there are any clipped vertices
        // better run it through the clipper
        if ( CLIPPING_NEEDED )
        {
         LPDWORD clipcodes = (LPDWORD) pRc->tl.pClipBuf;
         DWORD c0, c1;
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Get pointer to first vertex
             lpV0 = lpVOffset;
             lpVOffset += FVFO_SIZE;
             c0 = *clipcodes++;
   
             // Get pointer to second vertex
             lpV1 = lpVOffset;
             lpVOffset += FVFO_SIZE;
             c1 = *clipcodes++;
   
             // Check if line is clipped on 
             // both vertices with a common clip plane
             if(!(c0 & c1))
                dp2LineClipped( pRc, lpV0, lpV0, lpV1, c0, c1, pRc->dwVertexType );
           }
        }
        else
        // No clipcode buffer present, run it through as regular
        {
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Get pointer to first vertex
             lpV0 = lpVOffset;
             lpVOffset += FVFO_SIZE;
   
             // Get pointer to second vertex
             lpV1 = lpVOffset;
             lpVOffset += FVFO_SIZE;
   
             // Draw a line
             dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
           }
        }

#else  // !TnL_HAL

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Get pointer to first vertex
          lpV0 = lpVOffset;
          lpVOffset += FVFO_SIZE;

          // Get pointer to second vertex
          lpV1 = lpVOffset;
          lpVOffset += FVFO_SIZE;

          // Draw a line
          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
        }
#endif // TnL_HAL

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2LINELIST*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed line list
      case D3DDP2OP_INDEXEDLINELIST :
      {
        D3DPRINT( 32, "  Command INDEXEDLINELIST (%d, %d)", lpCmd->bCommand, lpCmd->wPrimitiveCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDLINELIST (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 10, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2INDEXEDLINELIST, lpCmd->wPrimitiveCount, 0);

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2INDEXEDLINELIST *)lpPrim)->wV1);
          CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2INDEXEDLINELIST *)lpPrim)->wV2);

          // Get the vertex start index offset and update the primitive data pointer
          lpV0 = lpVertices + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1 * FVFO_SIZE);
          lpV1 = lpVertices + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2 * FVFO_SIZE);

          // Draw a line
          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );

          // Next primitive
          lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
        }

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)lpPrim;
        hr = D3D_OK;
        break;
      }

      // Indexed line list2
      case D3DDP2OP_INDEXEDLINELIST2 :
      {
        D3DPRINT( 32, "  Command INDEXEDLINELIST2 (%d, %d)", lpCmd->bCommand, lpCmd->wPrimitiveCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDLINELIST2 (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 11, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2INDEXEDLINELIST, lpCmd->wPrimitiveCount, sizeof(D3DHAL_DP2STARTVERTEX));

#ifndef  TnL_HAL
        // Get the vertex start index offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
#endif  // TnL_HAL

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumIndices = lpCmd->wPrimitiveCount*2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

           D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

           // Call on Save Primitive Data to initialize the T&L HAL with the proper info
           // for this particular primitive.
           SaveIdxPrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, (UINT) lpdp2d->dwVertexLength,
                                 D3DPT_LINELIST, (LPWORD) lpPrim, (UINT) dwNumIndices, lpVBSurfData );

           // Handle all TnL processing then call pass down to rendering code.
           if (ProcessPrimitive( pRc ) != D3D_OK)
           {
             // The return value from ProcessPrimitive is telling us that ALL of 
             // the primitive's vertices are clipped and we should be able
             // to trivially reject this primitive
             NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDLINELIST, lpCmd->wPrimitiveCount, 2);
             hr = D3D_OK;
             break;
           }

           // We're going for it!  Let's Render this puppy!
           // Update the local Vertex Type with the new Output Vertex Type
		   UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
           lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Get the vertex start index offset and update the primitive data pointer
           lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        }
#endif  //TnL_HAL

        lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

#ifdef TnL_HAL

        // If there are any clipped vertices
        // better run it through the clipper
        if ( CLIPPING_NEEDED )
        {
         LPDWORD clipcodes = (LPDWORD) pRc->tl.pClipBuf;
         DWORD c0, c1;
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1);
             CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2);
   
             // Get the vertex start index offset and update the primitive data pointer
             lpV0 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1 * FVFO_SIZE);
             c0 = clipcodes[((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1];
             lpV1 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2 * FVFO_SIZE);
             c1 = clipcodes[((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2];
   
             // Check if line is clipped on 
             // both vertices with a common clip plane
             if(!(c0 & c1))
                dp2LineClipped( pRc, lpV0, lpV0, lpV1, c0, c1, pRc->dwVertexType );
   
             // Next primitive
             lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
           }
        }
        else
        {
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1);
             CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2);
   
             // Get the vertex start index offset and update the primitive data pointer
             lpV0 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1 * FVFO_SIZE);
             lpV1 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2 * FVFO_SIZE);
   
             // Draw a line
             dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
   
             // Next primitive
             lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
           }
        }


#else // TnL_HAL
        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1);
          CHECK_DATABUF_LIMITS(lpdp2d, ((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2);

          // Get the vertex start index offset and update the primitive data pointer
          lpV0 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV1 * FVFO_SIZE);
          lpV1 = lpVOffset + (((LPD3DHAL_DP2INDEXEDLINELIST)lpPrim)->wV2 * FVFO_SIZE);

          // Draw a line
          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );

          // Next primitive
          lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
        }
#endif // TnL_HAL

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)lpPrim;

        hr = D3D_OK;
        break;
      }

      // Line strip
      case D3DDP2OP_LINESTRIP :
      {
        D3DPRINT( 32, "  <WARNING> Unsupported DP2 command - LINESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wPrimitiveCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_LINESTRIP (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 12, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2LINESTRIP, 1, 0);

        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2LINESTRIP *)lpPrim)->wVStart);
        CHECK_DATABUF_LIMITS(lpdp2d, ((D3DHAL_DP2LINESTRIP *)lpPrim)->wVStart + lpCmd->wPrimitiveCount);

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
         DWORD dwNumVertices = lpCmd->wPrimitiveCount + 1;
         DWORD dwVStart = (DWORD)((D3DHAL_DP2LINESTRIP *)lpPrim)->wVStart;

           D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

           // Call on Save Primitive Data to initialize the T&L HAL with the proper info
           // for this particular primitive.
           SavePrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, dwNumVertices,
                                 D3DPT_LINELIST, lpVBSurfData );

           // Handle all TnL processing then call pass down to rendering code.
           if (ProcessPrimitive( pRc ) != D3D_OK)
           {
             // The return value from ProcessPrimitive is telling us that ALL of 
             // the primitive's vertices are clipped and we should be able
             // to trivially reject this primitive
             NEXTINSTRUCTION(lpCmd, D3DHAL_DP2LINESTRIP, 1, 0);
             hr = D3D_OK;
             break;
           }

           // We're going for it!  Let's Render this puppy!
           // Update the local Vertex Type with the new Output Vertex Type
		   UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
           lpV1 = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Get the vertex start index offset and update the primitive data pointer
           lpV1 = lpVertices + (((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart * FVFO_SIZE);
        }
        // If there are any clipped vertices

        // better run it through the clipper
        if ( CLIPPING_NEEDED )
        {
         LPDWORD clipcodes = (LPDWORD) pRc->tl.pClipBuf;
         DWORD c0, c1;

           c1 = *clipcodes++;
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Move the last vertex pointer into the first
             lpV0 = lpV1;
             c0 = c1;
   
             // Now get the next vertex pointer, adjusting the index offset pointer
             // if we have read the last vertex pointer in the group of two
             lpV1 = lpV0 + FVFO_SIZE;
             c1 = *clipcodes++;

             // Check if line is clipped on 
             // both vertices with a common clip plane
             if(!(c0 & c1))
                dp2LineClipped( pRc, lpV0, lpV0, lpV1, c0, c1, pRc->dwVertexType );
           }
        }
        else
        {
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Move the last vertex pointer into the first
             lpV0 = lpV1;
   
             // Now get the next vertex pointer, adjusting the index offset pointer
             // if we have read the last vertex pointer in the group of two
             lpV1 = lpV0 + FVFO_SIZE;

             // Draw a line
             dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
           }
        }

#else  //TnL_HAL

        // Get the vertex start index offset and update the primitive data pointer
        lpV1 = lpVertices + (((D3DHAL_DP2LINESTRIP *)lpPrim)->wVStart * FVFO_SIZE);

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Move the last vertex pointer into the first
          lpV0 = lpV1;

          // Now get the next vertex pointer, adjusting the index offset pointer
          // if we have read the last vertex pointer in the group of two
          lpV1 = lpV0 + FVFO_SIZE;

          // Draw a line
          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
        }
#endif // TnL_HAL

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2LINESTRIP*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed line strip
      case D3DDP2OP_INDEXEDLINESTRIP :
      {
        D3DPRINT( 32, "  Command INDEXEDLINESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_INDEXEDLINESTRIP (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 13, lpCmd->wPrimitiveCount );

        // Update pointer to primitive data
//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, WORD, lpCmd->wPrimitiveCount + 1, sizeof(D3DHAL_DP2STARTVERTEX));

#ifdef WINNT
        {
          LPBYTE  lpPrim2;
          WORD    wIndxBase;


          wIndxBase = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;
          lpPrim2 = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

          CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDLINESTRIP *)lpPrim2)->wV[0]);

          for (i = lpCmd->wPrimitiveCount; i > 0; i--)
          {
            CHECK_DATABUF_LIMITS(lpdp2d, wIndxBase + ((D3DHAL_DP2INDEXEDLINESTRIP *)lpPrim2)->wV[1]);
            lpPrim2 += sizeof(WORD);
          }
        }
#endif

#ifndef  TnL_HAL
        // Get the vertex start index offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
#endif

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {
          DWORD dwNumIndices = lpCmd->wPrimitiveCount*2;
          DWORD dwVStart = (DWORD)((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

           D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

           // Call on Save Primitive Data to initialize the T&L HAL with the proper info
           // for this particular primitive.
           SaveIdxPrimitiveData( pRc, lpdp2d->dwVertexType, (LPVOID) lpVertices, dwVStart, (UINT) lpdp2d->dwVertexLength,
                                 D3DPT_LINESTRIP, (LPWORD) lpPrim, (UINT) dwNumIndices, lpVBSurfData );

           // Handle all TnL processing then call pass down to rendering code.
           if (ProcessPrimitive( pRc ) != D3D_OK)
           {
             // The return value from ProcessPrimitive is telling us that ALL of 
             // the primitive's vertices are clipped and we should be able
             // to trivially reject this primitive
             NEXTINSTRUCTION(lpCmd, D3DHAL_DP2INDEXEDLINESTRIP, lpCmd->wPrimitiveCount, 2);
             hr = D3D_OK;
             break;
           }

           // We're going for it!  Let's Render this puppy!
           // Update the local Vertex Type with the new Output Vertex Type
		   UpdateCtxFVFChanges(lpdp2d->dwhContext, (DWORD)pRc->tl.TLFVF.dwFVFType);
           lpVOffset = (LPDWORD) pRc->tl.TLVBuf.alignedBuf;
        }
        else
        {
           // Get the vertex start index offset and update the primitive data pointer
           lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        }
#endif  //TnL_HAL


        lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

        // Keep a copy of the index offset pointer and read the first index
        lpIdx = lpPrim;
        lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];

#ifdef TnL_HAL
        if ( CLIPPING_NEEDED )
        {
         LPDWORD clipcodes = (LPDWORD) pRc->tl.pClipBuf;
         DWORD c0, c1;
           c1 = clipcodes[((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]];

           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Move the last vertex pointer into the first
             lpV0 = lpV1;
             c0 = c1;

             // Now get the next vertex pointer, adjusting the index offset pointer
             // if we have read the last vertex pointer in the group of two
             if( i & 1 )
             {
               lpIdx += sizeof(D3DHAL_DP2INDEXEDLINESTRIP);
               lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];
               c1 = clipcodes[((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]];
             }
             else
             {
               lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[1]) * FVFO_SIZE];
               c1 = clipcodes[((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[1]];
             }

             // Check if line is clipped on 
             // both vertices with a common clip plane
             if(!(c0 & c1))
                dp2LineClipped( pRc, lpV0, lpV0, lpV1, c0, c1, pRc->dwVertexType );
           }
        }
        else
        {
           // Now loop through each line
           for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
           {
             // Move the last vertex pointer into the first
             lpV0 = lpV1;

             // Now get the next vertex pointer, adjusting the index offset pointer
             // if we have read the last vertex pointer in the group of two
             if( i & 1 )
             {
               lpIdx += sizeof(D3DHAL_DP2INDEXEDLINESTRIP);
               lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];
             }
             else
               lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[1]) * FVFO_SIZE];

             // Draw a line
             dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
           }
        }
#else // TnL_HAL

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Move the last vertex pointer into the first
          lpV0 = lpV1;

          // Now get the next vertex pointer, adjusting the index offset pointer
          // if we have read the last vertex pointer in the group of two
          if( i & 1 )
          {
            lpIdx += sizeof(D3DHAL_DP2INDEXEDLINESTRIP);
            lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];
          }
          else
            lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[1]) * FVFO_SIZE];

          // Draw a line
          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
        }
#endif // TnL_HAL

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (sizeof(WORD) * (lpCmd->wPrimitiveCount + 1)));
        hr = D3D_OK;
        break;
      }

      // Line list using data embedded in command stream
      case D3DDP2OP_LINELIST_IMM :
      {
        D3DPRINT( 32, "  Command LINELIST_IMM (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_LINELIST_IMM (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

#ifdef  TnL_HAL
        // New for DX7, check to see if the passed Vertex is untransformed
        //  If so, call ProcessPrimitive and allow the TnL to be operated on
        //  the Raw Vertex data.  
        if (TRANSFORM_NEEDED)
        {

          D3DPRINT( 32, "  VERTEXDATA is Untrasformed, preparing for software HAL TnL");

          // BobJ Temporary
          // We aren't rendering yet, just update the comand pointer,
          // set the return value and break out out this primitive
          // to go to the next one in the command stream
#ifdef WINNT
          if(D3D_OK == (hr = ppdev->pD3DParseUnknownCommand((LPVOID)lpCmd,(LPVOID*)&lpResumeCmd)))
#else
          if(D3D_OK == (hr = dp2Callback( (LPVOID)lpCmd, (LPVOID*)&lpResumeCmd ) ) )
#endif
          {
            lpCmd = lpResumeCmd;
            break;
          }

        }

#endif  //TnL_HAL

        // Setup the hardware state
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 14, lpCmd->wPrimitiveCount );

        // Check primtive data pinter in command stream is DWORD aligned
        lpPrim = (LPBYTE)(((DWORD)(lpCmd + 1) + 3) & ~3);

        CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim, FVFO_SIZE * sizeof(DWORD), lpCmd->wPrimitiveCount + 1, 0);

        // Get the first vertex pointer
        lpV1 = (LPDWORD)lpPrim;

        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Copy the last vertex pointer into the first and then get
          // the next vertex pointer
          lpV0 = lpV1;
          lpV1 = lpV0 + FVFO_SIZE;

          dp2Line( pRc, lpV0, lpV0, lpV1, pRc->dwVertexType );
        }

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 1) * FVFO_SIZE));
        hr = D3D_OK;
        break;
      }

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      case D3DDP2OP_TEXBLT:
      {
        // Inform the driver to perform a BitBlt operation from a source
        // texture to a destination texture. A texture can also be cubic
        // environment map. The driver should copy a rectangle specified
        // by rSrc in the source texture to the location specified by pDest
        // in the destination texture. The destination and source textures
        // are identified by handles that the driver was notified with
        // during texture creation time. If the driver is capable of
        // managing textures, then it is possible that the destination
        // handle is 0. This indicates to the driver that it should preload
        // the texture into video memory (or wherever the hardware
        // efficiently textures from). In this case, it can ignore rSrc and
        // pDest. Note that for mipmapped textures, only one D3DDP2OP_TEXBLT
        // instruction is inserted into the D3dDrawPrimitives2 command stream.
        // In this case, the driver is expected to BitBlt all the mipmap
        // levels present in the texture.

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TEXBLT (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TEXBLT, lpCmd->wStateCount, 0);

        for (i = 0; i < lpCmd->wStateCount; i++)
        {
          typedef DWORD (__stdcall * PTEXBLTFUNC)(NT9XDEVICEDATA *, TXTRHNDL *, RECTL *, int, TXTRHNDL *, RECTL*, int);
  
          PTEXBLTFUNC               pfnTexBlt;
          RECTL                     dstRect;
          DWORD                     hSrcSurf, hDstSurf;
          TXTRHNDL                  *pSrcSurf, *pDstSurf;
          D3DHAL_DP2TEXBLT          *pTexBlt = (D3DHAL_DP2TEXBLT *)lpPrim;
          int                       nSrcLOD, nDstLOD;


          hSrcSurf = pTexBlt->dwDDSrcSurface;
          hDstSurf = pTexBlt->dwDDDestSurface;

#if 1 //defined(WINNT)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for these surfaces before dereferencing them
          if (! ((TXTRHNDL_INRANGE(hSrcSurf) && TXTRHNDL_PTR(hSrcSurf)) &&
                 (TXTRHNDL_INRANGE(hDstSurf) && TXTRHNDL_PTR(hDstSurf)))
             )
          {
            // what should we do here?
            // just skip this blt or end processing of the command stream?
            //
            // Due to the past experience with a variety of problems arising
            // when terminating processing of the command stream early
            // I'm choosing to ignore this blt and move on to the next one
            // or move on the the next command in the command stream
            // if there are no more texblt's in this command
            D3DPRINT(0, "  TXTBLT: src or dst TXTRHNDL_PTR is NULL!  hSrc=%ld, hDst=%ld", hSrcSurf, hDstSurf);
            hr = D3D_OK;
            lpPrim += sizeof(D3DHAL_DP2TEXBLT);
            continue;
          }
#endif

          pSrcSurf = TXTRHNDL_PTR(hSrcSurf);
          pDstSurf = TXTRHNDL_PTR(hDstSurf);

#if defined(WINNT) && DBG
   D3DPRINT(1, "  TEXBLT - pSrcSurfLcl=%8lXh (hSrc = %ld) pDstSurfLcl=%8lXh (hDst=%ld)",
            pSrcSurf, hSrcSurf, pDstSurf, hDstSurf);
#endif

          if ((DDPF_FOURCC & pDstSurf->dwFlags) &&
              (DDPF_FOURCC & pSrcSurf->dwFlags))
          {
            pfnTexBlt = (PTEXBLTFUNC)Blt32_CopyFourCC;
          }
          else
          {
            pfnTexBlt = (PTEXBLTFUNC)textureLoad;   // can't use TEXTURELOAD here
            // TEXTURELOAD modifies some of the texture registers
            // so we need to force setDX6State to be called before rendering again
            UPDATE_HW_STATE( SC_SOMETHING );
          }

          // D3DHAL_DP2TEXBLT only has upper left corner of dst
          // so create a rect that can be passed to TEXTURELOAD
          dstRect.left   = pTexBlt->pDest.x;
          dstRect.right  = dstRect.left + (pTexBlt->rSrc.right - pTexBlt->rSrc.left);
          dstRect.top    = pTexBlt->pDest.y;
          dstRect.bottom = dstRect.top + (pTexBlt->rSrc.bottom - pTexBlt->rSrc.top);

          // call TEXTURELOAD repeatedly for mipmaps
          if ((DDSCAPS_MIPMAP & pSrcSurf->dwCaps) &&
              (DDSCAPS_MIPMAP & pDstSurf->dwCaps))
          {
            RECTL   srcRect;
            DWORD   dstWidthToMatch, dstHeightToMatch;


#ifdef WINNT
            // the DX7 runtime appears to munge the width and height of system memory DXTn
            // surfaces
            if ((DDSCAPS_SYSTEMMEMORY & pSrcSurf->dwCaps) &&
                (pfnTexBlt == (PTEXBLTFUNC)Blt32_CopyFourCC) &&
                ((FOURCC_DXT1 == pDstSurf->dwFourCC) ||
                 (FOURCC_DXT2 == pDstSurf->dwFourCC) ||
                 (FOURCC_DXT3 == pDstSurf->dwFourCC) ||
                 (FOURCC_DXT4 == pDstSurf->dwFourCC) ||
                 (FOURCC_DXT5 == pDstSurf->dwFourCC)))
            {
              DWORD surfSize;
            
              dstHeightToMatch = (pDstSurf->wHeight + 3) / 4;
            
              surfSize = ((pDstSurf->wWidth  + 3) / 4) * dstHeightToMatch;
              if (FOURCC_DXT1 == pDstSurf->dwFourCC)
                surfSize *= 8;    // The size of a DXT1 4x4 pixel block
              else
                surfSize *= 16;   // The size of a DXT2,DXT3,DXT4 or DXT5 4x4 pixel block
            
              dstWidthToMatch = surfSize / dstHeightToMatch;
            }
            else
#endif
            {
              dstHeightToMatch = pDstSurf->wHeight;
              dstWidthToMatch  = pDstSurf->wWidth;
            }

            memcpy(&srcRect, &pTexBlt->rSrc, sizeof(srcRect));

            nSrcLOD = 0;
            nDstLOD = 0;

            // skip src mipmaps that have a larger size than the top dst mipmap
            while (nSrcLOD < pSrcSurf->nLevels)
            {
              if ((dstWidthToMatch  == pSrcSurf->mmData[nSrcLOD].wWidth ) &&
                  (dstHeightToMatch == pSrcSurf->mmData[nSrcLOD].wHeight))
                break;

              if (nSrcLOD < pSrcSurf->nLevels)
                nSrcLOD++;
              else
              {
                // we didn't find a matching mip level at all
                pSrcSurf = 0;
                D3DPRINT(0, "  TEXBLT - no matching mip level found, no blt will be performed");
                break;
              }

              // recompute srcRect & dstRect by
              // wacking the width and height in half
              // I believe we need to change the .left and .top as well....tl
              // Patch up srcRect.
              srcRect.left = srcRect.left >> 1;
              srcRect.right = (srcRect.right+1) >> 1;
              srcRect.top = srcRect.top >> 1;
              srcRect.bottom = (srcRect.bottom+1) >> 1;
              // Make sure the width and height are always at least 1
              if ((srcRect.bottom - srcRect.top) <= 0)
                 srcRect.bottom = srcRect.top + 1;
              if ((srcRect.right - srcRect.left) <= 0)
                 srcRect.right = srcRect.left + 1;

              // Patch up dstRect.
              dstRect.left = dstRect.left >> 1;
              dstRect.right = (dstRect.right+1) >> 1;
              dstRect.top = dstRect.top / 2;
              dstRect.bottom = (dstRect.bottom+1) >> 1;
              // Make sure the width and height are always at least 1
              if ((dstRect.bottom - dstRect.top) <= 0)
                 dstRect.bottom = dstRect.top + 1;
              if ((dstRect.right - dstRect.left) <= 0)
                 dstRect.right = dstRect.left + 1;
            }

            // loop until no more src or dst mipmaps
            while ((nSrcLOD < pSrcSurf->nLevels) && (nDstLOD < pDstSurf->nLevels))
            {
              // call texture download code in d3txtr.c
              hr = pfnTexBlt(ppdev,
                             pSrcSurf, &srcRect, nSrcLOD,
                             pDstSurf, &dstRect, nDstLOD);

              if (D3D_OK != hr)
                break;

              // step to next src mip level
              nSrcLOD++;

              // step to next dst mip level
              nDstLOD++;

              // recompute srcRect & dstRect by
              // wacking the width and height in half
              // I believe we need to change the .left and .top as well....tl
              // Patch up srcRect.
              srcRect.left = srcRect.left >> 1;
              srcRect.right = (srcRect.right+1) >> 1;
              srcRect.top = srcRect.top >> 1;
              srcRect.bottom = (srcRect.bottom+1) >> 1;
              // Make sure the width and height are always at least 1
              if ((srcRect.bottom - srcRect.top) <= 0)
                 srcRect.bottom = srcRect.top + 1;
              if ((srcRect.right - srcRect.left) <= 0)
                 srcRect.right = srcRect.left + 1;

              // Patch up dstRect.
              dstRect.left = dstRect.left >> 1;
              dstRect.right = (dstRect.right+1) >> 1;
              dstRect.top = dstRect.top >> 1;
              dstRect.bottom = (dstRect.bottom+1) >> 1;
              // Make sure the width and height are always at least 1
              if ((dstRect.bottom - dstRect.top) <= 0)
                 dstRect.bottom = dstRect.top + 1;
              if ((dstRect.right - dstRect.left) <= 0)
                 dstRect.right = dstRect.left + 1;
            }
          }
          else
          {
            // call texture download code in d3txtr.c
            hr = pfnTexBlt(ppdev,
                           pSrcSurf, &pTexBlt->rSrc, 0,
                           pDstSurf, &dstRect, 0);
          }
          if (D3D_OK != hr)
            break;

          lpPrim += sizeof(D3DHAL_DP2TEXBLT);
        }

        if (D3D_OK != hr)
          break;

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2TEXBLT, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_SETPALETTE:
      {
        // Attach a palette to a texture, that is, map an association
        // between a palette handle and a surface handle, and specify
        // the characteristics of the palette. The number of
        // D3DNTHAL_DP2SETPALETTE structures to follow is specified by
        // the wStateCount member of the D3DNTHAL_DP2COMMAND structure

        D3DHAL_DP2SETPALETTE  *pSetPal;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETPALETTE (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETPALETTE, lpCmd->wStateCount, 0);

        pSetPal = (D3DHAL_DP2SETPALETTE *)lpPrim;

        for (i = 0; i < lpCmd->wStateCount; i++, pSetPal++)
        {
          hr = PaletteSet(pRc,
                          pSetPal->dwSurfaceHandle,
                          pSetPal->dwPaletteHandle,
                          pSetPal->dwPaletteFlags);
          if (D3D_OK != hr)
            break;
        }

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETPALETTE, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_UPDATEPALETTE:
      {
        // Perform modifications to the palette that is used for palettized
        // textures. The palette handle attached to a surface is updated
        // with wNumEntries PALETTEENTRYs starting at a specific wStartIndex
        // member of the palette. (A PALETTENTRY (defined in wingdi.h and
        // wtypes.h) is actually a DWORD with an ARGB color for each byte.)
        // After the D3DNTHAL_DP2UPDATEPALETTE structure in the command
        // stream the actual palette data will follow (without any padding),
        // comprising one DWORD per palette entry. There will only be one
        // D3DNTHAL_DP2UPDATEPALETTE structure (plus palette data) following
        // the D3DNTHAL_DP2COMMAND structure regardless of the value of
        // wStateCount.

        D3DHAL_DP2UPDATEPALETTE *pUpdatePal;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_UPDATEPALETTE");

//      lpPrim = (LPBYTE)(lpCmd + 1);
        pUpdatePal = (D3DHAL_DP2UPDATEPALETTE *)lpPrim;

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2UPDATEPALETTE, 1, pUpdatePal->wNumEntries * sizeof(PALETTEENTRY));

        // We will ALWAYS have only 1 palette update structure + palette
        // following the D3DDP2OP_UPDATEPALETTE token
#ifdef WINNT
        ASSERTDD(1 == lpCmd->wStateCount, "1 != wStateCount in D3DDP2OP_UPDATEPALETTE");
#endif

        hr = PaletteUpdate(pRc,
                           pUpdatePal->dwPaletteHandle,
                           pUpdatePal->wStartIndex,
                           pUpdatePal->wNumEntries,
                           (BYTE*)(pUpdatePal+1));

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2UPDATEPALETTE, 1,
                        ((D3DHAL_DP2UPDATEPALETTE *)lpPrim)->wNumEntries*sizeof(PALETTEENTRY));
        break;
      }

      case D3DDP2OP_SETRENDERTARGET:
      {
        // Map a new rendering target surface and depth buffer in
        // the current context.  This replaces the old mySetRenderTarget32
        // callback.

        D3DHAL_DP2SETRENDERTARGET *pSRTData;
        DWORD hRenderTarg, hZBuff;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETRENDERTARGET");

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETRENDERTARGET, lpCmd->wStateCount, 0);

        // get new data by ignoring all but the last structure
        pSRTData = (D3DHAL_DP2SETRENDERTARGET *)lpPrim + (lpCmd->wStateCount - 1);

        hRenderTarg = pSRTData->hRenderTarget;
        hZBuff = pSRTData->hZBuffer;
        if (hZBuff)
        {
#if 1 // defined(WINNT)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for these surfaces before dereferencing them
          if (! ((TXTRHNDL_INRANGE(hRenderTarg) && TXTRHNDL_PTR(hRenderTarg)) &&
                 (TXTRHNDL_INRANGE(hZBuff) && TXTRHNDL_PTR(hZBuff)))
             )
          {
            // what should we do here?
            // just skip this setrendertaret or end processing of the command stream?
            //
            // Due to the past experience with a variety of problems arising
            // when terminating processing of the command stream early
            // I'm choosing to ignore this setrendertarget
            // and move on the the next command in the command stream
            D3DPRINT(0, "  SETRENDERTARGET: hRenderTarg or hZBuff TXTRHNDL_PTR is NULL!  hRenderTarget=%ld, hZBuff=%ld", hRenderTarg, hZBuff);
            hr = D3D_OK;
          }
          else
#endif
          {
            hr = SetRenderTarget(pRc, hRenderTarg, hZBuff);
          }
        }
        else
        {
#if 1 //defined(WINNT)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for this surface before dereferencing it
          if (! (TXTRHNDL_INRANGE(hRenderTarg) && TXTRHNDL_PTR(hRenderTarg)))
          {
            // what should we do here?
            // just skip this setrendertaret or end processing of the command stream?
            //
            // Due to the past experience with a variety of problems arising
            // when terminating processing of the command stream early
            // I'm choosing to ignore this setrendertarget
            // and move on the the next command in the command stream
            D3DPRINT(0, "  SETRENDERTARGET: hRenderTarg TXTRHNDL_PTR is NULL!  hRenderTarget=%ld", hRenderTarg);
            hr = D3D_OK;
          }
          else
#endif
          {
            hr = SetRenderTarget(pRc, hRenderTarg, (DWORD) NULL);
          }
        }

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETRENDERTARGET, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_CLEAR:
      {
        // Perform hardware-assisted clearing on the rendering target,
        // depth buffer or stencil buffer. This replaces the old ddiClear
        // and ddiClear2 callbacks.
        typedef HRESULT (* PCLRFUNC) (RC *, D3DHAL_DP2CLEAR *, DWORD);
        PCLRFUNC pfnClear;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CLEAR");

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, RECT, lpCmd->wStateCount, (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)));

        // clear render target & z separately with two fastfill commands,
        // the two passes is a workaround for V3
        if (IS_NAPALM)
        {
          // use fastfill for Napalm, non-SLI mode
          pfnClear = (PCLRFUNC) FastFill;
        }
        else
        {
          // use 2 pass fastfill for Voodoo3
          pfnClear = (PCLRFUNC) FastFill_2pass;
        }

		// Disable the per-primitive forcing of jitter values to zero.
        D3DPRINT(32, "  Setting dwZeroJitter to 0");
		pRc->dwZeroJitter = 0;

        HandleSLIAA(pRc);
        pfnClear(pRc, (D3DHAL_DP2CLEAR *) lpPrim, (DWORD) lpCmd->wStateCount);

        NEXTINSTRUCTION(lpCmd, RECT, lpCmd->wStateCount, (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)));
        break;
      }

      case D3DDP2OP_STATESET:
      {
        D3DHAL_DP2STATESET *pStateSetOp;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_STATESET (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2STATESET, lpCmd->wStateCount, 0);

        pStateSetOp = (D3DHAL_DP2STATESET *)lpPrim;

        for (i = 0; i < lpCmd->wStateCount; i++, pStateSetOp++)
        {
          switch (pStateSetOp->dwOperation)
          {
            case D3DHAL_STATESETBEGIN  :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETBEGIN  dwHandle=%ld", pStateSetOp->dwParam);
              hr = BeginStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETEND    :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETEND");
              hr = EndStateBlock(pRc);
              break;
            case D3DHAL_STATESETDELETE :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETDELETE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = DeleteStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETEXECUTE:
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETEXECUTE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = ExecuteStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETCAPTURE:
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETCAPTURE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = CaptureStateBlock(pRc,pStateSetOp->dwParam);
              break;
            default :
              D3DPRINT(0, "D3DDP2OP_STATESET has invalid dwOperation %8lXh",
                       pStateSetOp->dwOperation);
              break;
          }
          if (D3D_OK != hr)
            break;
        }

        if (D3D_OK == hr)
        {
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2STATESET, lpCmd->wStateCount, 0);
        }
        break;
      }
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)

      case D3DDP2OP_SETSTREAMSOURCE:
      {
        LPD3DHAL_DP2SETSTREAMSOURCE pSetStreamSource;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETSTREAMSOURCE (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETSTREAMSOURCE, lpCmd->wStateCount, 0);

        // Loop for each state
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pSetStreamSource = (D3DHAL_DP2SETSTREAMSOURCE *) lpPrim;
            
            _D3D_OP_MStream_SetSrc(pRc,
                                   pSetStreamSource->dwStream,
                                   pSetStreamSource->dwVBHandle,
                                   pSetStreamSource->dwStride);
                         
            lpPrim += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETSTREAMSOURCE, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_SETSTREAMSOURCEUM:
      {
        D3DHAL_DP2SETSTREAMSOURCEUM *pSetStreamSourceUm;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETSTREAMSOURCEUM (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETSTREAMSOURCEUM, lpCmd->wStateCount, 0);

        // Loop for each state
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pSetStreamSourceUm = (D3DHAL_DP2SETSTREAMSOURCEUM *) lpPrim;
            
            _D3D_OP_MStream_SetSrcUM(pRc,
                                     pSetStreamSourceUm->dwStream,
                                     pSetStreamSourceUm->dwStride,
                                     lpVertices,
                                     lpdp2d->dwVertexLength);
                         
            lpPrim += sizeof(D3DHAL_DP2SETSTREAMSOURCEUM);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETSTREAMSOURCEUM, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_SETINDICES:
      {
        D3DHAL_DP2SETINDICES *pSetIndices;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETSETINDICES (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

//      lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETINDICES, lpCmd->wStateCount, 0);

        // Loop for each state
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pSetIndices = (D3DHAL_DP2SETINDICES *) lpPrim;
            
            _D3D_OP_MStream_SetIndices(pRc,
                                       pSetIndices->dwVBHandle,
                                       pSetIndices->dwStride);

            lpPrim += sizeof(D3DHAL_DP2SETSTREAMSOURCEUM);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETINDICES, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWPRIMITIVE:
      {
        D3DHAL_DP2DRAWPRIMITIVE *pDrawPrimitive;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWPRIMITIVE (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        // Setup the hardware state
        if( HW_STATE_CHANGED )
            setDX6state(pRc, 0, lpCmd->wPrimitiveCount);

        // iterate through each 
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pDrawPrimitive = (D3DHAL_DP2DRAWPRIMITIVE *) lpPrim;
            
            _D3D_OP_MStream_DrawPrim(pRc,
                                     pDrawPrimitive->primType,
                                     pDrawPrimitive->VStart,
                                     pDrawPrimitive->PrimitiveCount);
                         
            lpPrim += sizeof(D3DHAL_DP2DRAWPRIMITIVE);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWPRIMITIVE, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWINDEXEDPRIMITIVE:
      {
        D3DHAL_DP2DRAWINDEXEDPRIMITIVE *pDrawIndexedPrimitive;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWINDEXEDPRIMITIVE (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Setup the hardware state
        if( HW_STATE_CHANGED )
            setDX6state(pRc, 0, lpCmd->wPrimitiveCount);

        // iterate through each 
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pDrawIndexedPrimitive = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE *) lpPrim;
            
            _D3D_OP_MStream_DrawIndxP(pRc,
                                      pDrawIndexedPrimitive->primType,
                                      pDrawIndexedPrimitive->BaseVertexIndex,
                                      pDrawIndexedPrimitive->MinIndex,
                                      pDrawIndexedPrimitive->NumVertices,
                                      pDrawIndexedPrimitive->StartIndex,
                                      pDrawIndexedPrimitive->PrimitiveCount);
                         
            lpPrim += sizeof(D3DHAL_DP2DRAWINDEXEDPRIMITIVE);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWINDEXEDPRIMITIVE, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWPRIMITIVE2:
      {
        D3DHAL_DP2DRAWPRIMITIVE2 *pDrawPrimitive2;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWPRIMITIVE2 (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Setup the hardware state
        if( HW_STATE_CHANGED )
            setDX6state(pRc, 0, lpCmd->wPrimitiveCount);

        // iterate through each 
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pDrawPrimitive2 = (D3DHAL_DP2DRAWPRIMITIVE2 *) lpPrim;
            
            _D3D_OP_MStream_DrawPrim2(pRc,
                                      pDrawPrimitive2->primType,
                                      pDrawPrimitive2->FirstVertexOffset,
                                      pDrawPrimitive2->PrimitiveCount);
                         
            lpPrim += sizeof(D3DHAL_DP2DRAWPRIMITIVE2);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWPRIMITIVE2, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWINDEXEDPRIMITIVE2:
      {
        D3DHAL_DP2DRAWINDEXEDPRIMITIVE2 *pDrawIndexedPrimitive2;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWINDEXEDPRIMITIVE2 (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Setup the hardware state
        if( HW_STATE_CHANGED )
            setDX6state(pRc, 0, lpCmd->wPrimitiveCount);

        // iterate through each 
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pDrawIndexedPrimitive2 = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE2 *) lpPrim;
            
            _D3D_OP_MStream_DrawIndxP2(pRc,
                                       pDrawIndexedPrimitive2->primType,
                                       pDrawIndexedPrimitive2->BaseVertexOffset,
                                       pDrawIndexedPrimitive2->MinIndex,
                                       pDrawIndexedPrimitive2->NumVertices,
                                       pDrawIndexedPrimitive2->StartIndexOffset,
                                       pDrawIndexedPrimitive2->PrimitiveCount);
                         
            lpPrim += sizeof(D3DHAL_DP2DRAWINDEXEDPRIMITIVE2);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWINDEXEDPRIMITIVE2, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_CLIPPEDTRIANGLEFAN:
      {
        D3DHAL_CLIPPEDTRIANGLEFAN *pClippedTriangleFan;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CLIPPEDTRIANGLEFAN (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Setup the hardware state
        if( HW_STATE_CHANGED )
            setDX6state(pRc, 0, lpCmd->wPrimitiveCount);

        // iterate through each 
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            pClippedTriangleFan = (D3DHAL_CLIPPEDTRIANGLEFAN *) lpPrim;
            
            _D3D_OP_MStream_ClipTriFan(pRc,
                                       pClippedTriangleFan->FirstVertexOffset,
                                       pClippedTriangleFan->dwEdgeFlags,
                                       pClippedTriangleFan->PrimitiveCount);
                         
            lpPrim += sizeof(D3DHAL_CLIPPEDTRIANGLEFAN);
        } 
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_CLIPPEDTRIANGLEFAN, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWRECTPATCH:
      {
//      D3DHAL_DP2DRAWRECTPATCH *pDrawRectPatch;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWRECTPATCH (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWRECTPATCH, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_DRAWTRIPATCH:
      {
//      D3DHAL_DP2DRAWTRIPATCH *pDrawTriPatch;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DRAWTRIPATCH (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);





        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWTRIPATCH, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_CREATEVERTEXSHADER:
      {
        D3DHAL_DP2CREATEVERTEXSHADER *pCreateVtxShader;
        DWORD dwExtraBytes = 0;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CREATEVERTEXSHADER (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // iterate through each passed vertex shader creation block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // verify that the next vertex shader is readable
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                                D3DHAL_DP2CREATEVERTEXSHADER, 1, 0);

            // Get the passed in vertex shader
            pCreateVtxShader = (D3DHAL_DP2CREATEVERTEXSHADER *) lpPrim;

            // Check if the size of the declaration and body of the 
            // vertex shader don't exceed the command buffer limits
            CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim,
                                  0, 0, 
                                  pCreateVtxShader->dwDeclSize + 
                                  pCreateVtxShader->dwCodeSize);  

            // Advance lpPrim so that it points to the vertex shader's
            // declaration and body
            lpPrim += sizeof(D3DHAL_DP2CREATEVERTEXSHADER);

            // Create this particular shader
            hr = _D3D_OP_VertexShader_Create(pRc,
                                             pCreateVtxShader->dwHandle,
                                             pCreateVtxShader->dwDeclSize,
                                             pCreateVtxShader->dwCodeSize,
                                             lpPrim);

            if (hr != DD_OK)
            {
            }
                                  
            // Update lpPrim in order to get to the next vertex
            // shader creation command block. 
            dwExtraBytes +=   pCreateVtxShader->dwDeclSize
                            + pCreateVtxShader->dwCodeSize;

            lpPrim +=         pCreateVtxShader->dwDeclSize
                            + pCreateVtxShader->dwCodeSize;      
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2CREATEVERTEXSHADER, lpCmd->wStateCount, dwExtraBytes);
        break;
      }

      case D3DDP2OP_SETVERTEXSHADER:
      {
        D3DHAL_DP2VERTEXSHADER* pSetVtxShader;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETVERTEXSHADER");

        // Following the DP2 token there is one and only one
        // set vertex shader block
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2VERTEXSHADER, 
                            1, 0);    

        // Get the passed in vertex shader
        pSetVtxShader = (D3DHAL_DP2VERTEXSHADER *) lpPrim;

        // Setup the given vertex shader.
        _D3D_OP_VertexShader_Set(pRc,
                                 pSetVtxShader->dwHandle);                

        // Now skip into the next DP2 token in the command buffer
        lpPrim += sizeof(D3DHAL_DP2VERTEXSHADER);               

        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2VERTEXSHADER, 1, 0);
        break;
      }
        
      case D3DDP2OP_DELETEVERTEXSHADER:
      {
        D3DHAL_DP2VERTEXSHADER *pDelVtxShader;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DELETEVERTEXSHADER (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // verify that all the following vertex shader 
        // delete blocks are readable
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2VERTEXSHADER, 
                            lpCmd->wStateCount, 0);

        // iterate through each passed vertex shader delete block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // Get the passed in vertex shader
            pDelVtxShader = (D3DHAL_DP2VERTEXSHADER*)lpPrim;

            // Destroy the given vertex shader.
            _D3D_OP_VertexShader_Delete(pRc,
                                        pDelVtxShader->dwHandle);

            // Update lpPrim in order to get to the next vertex
            // shader delete command block. 
            lpPrim += sizeof(D3DHAL_DP2VERTEXSHADER);               
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2VERTEXSHADER, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_SETVERTEXSHADERCONST:
      {
        D3DHAL_DP2SETVERTEXSHADERCONST *pVtxShaderConst;
        DWORD dwExtraBytes = 0;                

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETVERTEXSHADERCONST (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // verify that all the following vertex shader 
        // constant blocks are readable
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2SETVERTEXSHADERCONST,
                            lpCmd->wStateCount, 0);

        // iterate through each passed vertex shader constant block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // Get the passed in vertex shader constant
            pVtxShaderConst = (D3DHAL_DP2SETVERTEXSHADERCONST *) lpPrim;

            // Advance lpPrim so that it points to the constant
            // values to be loaded
            lpPrim += sizeof(D3DHAL_DP2SETVERTEXSHADERCONST);

            // constant block in order to Set up the constant entries
            _D3D_OP_VertexShader_SetConst(pRc,
                                          pVtxShaderConst->dwRegister,
                                          pVtxShaderConst->dwCount,
                                          (DWORD *)lpPrim);

            // Update lpPrim in order to get to the next vertex
            // shader constants command block. Each register has 4 floats.
            lpPrim += pVtxShaderConst->dwCount * 4 * sizeof(FLOAT);

            dwExtraBytes += pVtxShaderConst->dwCount * 4 * sizeof(FLOAT);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETVERTEXSHADERCONST, lpCmd->wStateCount, dwExtraBytes);
        break;
      }
                    
      case D3DDP2OP_CREATEPIXELSHADER:
      {
        D3DHAL_DP2CREATEPIXELSHADER *pCreatePxlShader;
        DWORD dwExtraBytes = 0;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CREATEPIXELSHADER (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // iterate through each passed pixel shader creation block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // verify that the next pixel shader is readable
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                                D3DHAL_DP2CREATEPIXELSHADER, 1, 0);    

            // Get the passed in pixel shader
            pCreatePxlShader = (D3DHAL_DP2CREATEPIXELSHADER *) lpPrim;

            // Check if the size of the declaration and body of the 
            // pixel shader don't exceed the command buffer limits
            CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim,
                                  0, 0, 
                                  pCreatePxlShader->dwCodeSize);

            // Update lpPrim to point to the actual pixel shader code
            lpPrim += sizeof(D3DHAL_DP2CREATEPIXELSHADER);

            // Create the given pixel shader
            hr = _D3D_OP_PixelShader_Create(pRc,
                                            pCreatePxlShader->dwHandle,
                                            pCreatePxlShader->dwCodeSize,
                                            lpPrim);

            if (hr != DD_OK)
            {
                D3DPRINT(D3DDBGLVL, "ERROR: Pixel Shader couln't be created!");
                PARSE_ERROR_AND_EXIT(lpdp2d, lpCmd, lpCmdStart,
                                     D3DERR_DRIVERINVALIDCALL);                                           
            }                                                  

            // Update lpPrim in order to get to the next vertex
            // shader creation command block. 
            lpPrim += pCreatePxlShader->dwCodeSize;               
            
            dwExtraBytes += pCreatePxlShader->dwCodeSize;
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2CREATEPIXELSHADER, lpCmd->wStateCount, dwExtraBytes);
        break;
      }
        
      case D3DDP2OP_SETPIXELSHADER:
      {
        D3DHAL_DP2PIXELSHADER *pSetPxlShader;

        D3DPRINT(D3DDBGLVL, "D3DHAL_DP2SETPIXELSHADER");

        // Following the DP2 token there is one and only one
        // set pixel shader block
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2PIXELSHADER, 
                            1, 0);    

        // Get the passed in pixel shader
        pSetPxlShader = (D3DHAL_DP2PIXELSHADER *) lpPrim;

        // Setup the given pixel shader.
        _D3D_OP_PixelShader_Set(pRc,
                                pSetPxlShader->dwHandle);

        // Now skip into the next DP2 token in the command buffer
        lpPrim += sizeof(D3DHAL_DP2PIXELSHADER);

        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2PIXELSHADER, 1, 0);
        break;
      }
        
      case D3DDP2OP_DELETEPIXELSHADER:
      {
        D3DHAL_DP2PIXELSHADER *pDelPxlShader;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_DELETEPIXELSHADER (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // verify that all the following pixel shader 
        // delete blocks are readable
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2PIXELSHADER, 
                            lpCmd->wStateCount, 0);

        // iterate through each passed vertex shader delete block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // Get the passed in vertex shader
            pDelPxlShader = (D3DHAL_DP2PIXELSHADER *) lpPrim;

            // Destroy the given pixel shader
            _D3D_OP_PixelShader_Delete(pRc,
                                       pDelPxlShader->dwHandle);

            // Update lpPrim in order to get to the next vertex
            // shader delete command block. 
            lpPrim += sizeof(D3DHAL_DP2PIXELSHADER);               
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2PIXELSHADER, lpCmd->wStateCount, 0);
        break;
      }
        
      case D3DDP2OP_SETPIXELSHADERCONST:
      {
        D3DHAL_DP2SETPIXELSHADERCONST *pPxlShaderConst;
        DWORD dwExtraBytes = 0;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETPIXELSHADERCONST (wStateCount=%ld)",
                 lpCmd->wStateCount);

        // verify that all the following vertex shader 
        // constant blocks are readable
        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                            D3DHAL_DP2SETPIXELSHADERCONST, 
                            lpCmd->wStateCount, 0);    

        // iterate through each passed vertex shader constant block
        for (i = 0; i < lpCmd->wStateCount; i++)
        {
            // Get the passed in vertex shader constant
            pPxlShaderConst = (D3DHAL_DP2SETPIXELSHADERCONST *) lpPrim;

            // Update lpPrim to point to the const data to setup
            lpPrim += sizeof(D3DHAL_DP2SETPIXELSHADERCONST);     

            // Set up the constant entries
            _D3D_OP_PixelShader_SetConst(pRc,
                                         pPxlShaderConst->dwRegister,
                                         pPxlShaderConst->dwCount,
                                         (DWORD *)lpPrim);

            // Update lpPrim in order to get to the next vertex
            // shader delete command block. Each register has 4 floats.
            lpPrim += pPxlShaderConst->dwCount * 4 * sizeof(FLOAT);

            dwExtraBytes += pPxlShaderConst->dwCount * 4 * sizeof(FLOAT);
        }
        // Update the command buffer pointer
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETPIXELSHADERCONST, lpCmd->wStateCount, dwExtraBytes);
        break;
      }

#endif // DX8

     /*-------------------------------------------------------------**
     ** Software Transformation and Lighting HAL stuff starts here! **
     **-------------------------------------------------------------*/

#ifdef  TnL_HAL
      case D3DDP2OP_ZRANGE:
          {
            D3DHAL_DP2ZRANGE *pZRange;
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_ZRANGE");

            // Keep only the last viewport notification
            pZRange = (D3DHAL_DP2ZRANGE *)(lpCmd + 1) + (lpCmd->wStateCount - 1);

            // Update T&L viewport state
            pRc->tl.Viewport.dvMinZ = pZRange->dvMinZ;
            pRc->tl.Viewport.dvMaxZ = pZRange->dvMaxZ;
            pRc->tl.dwDirtyFlags |= TLPV_DIRTY_ZRANGE;

            // Update the command buffer pointer
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2ZRANGE, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND)(pZRange + 1);
            break;
          }
      case D3DDP2OP_SETMATERIAL:
          {
            D3DHAL_DP2SETMATERIAL *pSetMat;
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETMATERIAL");

            // Keep only the last material notification
            pSetMat = (D3DHAL_DP2SETMATERIAL *)(lpCmd + 1) + (lpCmd->wStateCount - 1);

            pRc->tl.Material = *(D3DMATERIAL7 *)pSetMat;
            pRc->tl.dwDirtyFlags |= TLPV_DIRTY_MATERIAL;

            // Update the command buffer pointer
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETMATERIAL, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND)(pSetMat + 1);
            break;
          }
      case D3DDP2OP_SETLIGHT:
          {
            /* ScottK -- Light Changes */
            DWORD extra = 0;
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETLIGHT");
            hr = DP2TL_SetLight(pRc, lpCmd, &extra);
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETLIGHT, lpCmd->wStateCount, extra);
            /* ScottK -- Light Changes */
            break;
          }
      case D3DDP2OP_CREATELIGHT:
          {
            /* ScottK -- Light Changes */
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_CREATELIGHT");
            DP2TL_CreateLight(pRc, lpCmd);
            /* ScottK -- Light Changes */
            // Update the command buffer pointer
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2CREATELIGHT, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND)
            //      ((D3DHAL_DP2CREATELIGHT *)(lpCmd + 1) + lpCmd->wStateCount);
            break;
          }
      case D3DDP2OP_SETTRANSFORM:
          {
            WORD wNumXfrms = lpCmd->wStateCount;
            D3DHAL_DP2SETTRANSFORM *pSetXfrm = (D3DHAL_DP2SETTRANSFORM *)(lpCmd + 1);
            int i;
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETTRANSFORM num=%d" ,wNumXfrms );

            for (i = 0; i < (int) wNumXfrms; i++, pSetXfrm++)
            {
              D3DPRINT(D3DDBGLVL, "    D3DDP2OP_SETTRANSFORM type=%d" ,pSetXfrm->xfrmType );
              SetXfrm(pRc, pSetXfrm->xfrmType, &pSetXfrm->matrix);
            }

            // Update the command buffer pointer
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETTRANSFORM, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND) ((D3DHAL_DP2SETTRANSFORM *)(lpCmd + 1) + lpCmd->wStateCount);
            break;
          }
      case D3DDP2OP_EXT:
          {
            // Update the command buffer pointer
            D3DPRINT(D3DDBGLVL, "D3DDP2OP_EXT");
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2EXT, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND)
            //       ((D3DHAL_DP2EXT *)(lpCmd + 1) + lpCmd->wStateCount);
            break;
          }
      case D3DDP2OP_SETCLIPPLANE:
          {
            WORD wNumClipPlanes = lpCmd->wStateCount;
            D3DHAL_DP2SETCLIPPLANE *pSetClipPlane = (D3DHAL_DP2SETCLIPPLANE *)(lpCmd + 1);
            int i;

            D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETCLIPPLANE");
            for (i = 0; i < (int) wNumClipPlanes; i++, pSetClipPlane++)
            {
              memcpy( &pRc->tl.UserClipPlanes[pSetClipPlane->dwIndex],
                      pSetClipPlane->plane, sizeof(TLVECTOR4) );
            }
      
            pRc->tl.dwDirtyFlags |= TLPV_DIRTY_CLIPPLANES;

            // Update the command buffer pointer
            NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETCLIPPLANE, lpCmd->wStateCount, 0);
            //lpCmd = (LPD3DHAL_DP2COMMAND)(pSetClipPlane + 1);
            break;
          }
#endif  //TnL_HAL

      default:
        D3DPRINT(D3DDBGLVL, "default (lpCmd->bCommand=%ld)", lpCmd->bCommand);
        // If we don't recognise the command we need to refer it back to the
        // DrawPrimitive2 callback.
#ifdef WINNT
        if (D3D_OK == ( hr = ppdev->pD3DParseUnknownCommand((LPVOID)lpCmd,(LPVOID*)&lpResumeCmd)) )
#else
        if( D3D_OK == ( hr = dp2Callback( (LPVOID)lpCmd, (LPVOID*)&lpResumeCmd ) ) )
#endif
        {
          lpCmd = lpResumeCmd;
          break;
        }

        // We got an error back from the callback, we will now fall out of
        // the command stream parsing because hr != D3D_OK
        D3DPRINT( 32, "  Unhandled DrawPrimitive2 Command - %d", lpCmd->bCommand );
        break;
    }

    // Check our current return code, if it is not D3D_OK then we need
    // to calculate the address of the command that caused the error and return
    // this back when we exit.
    if( D3D_OK != hr )
    {
      D3DPRINT(5, "  hr != D3D_OK -- terminating command stream processing, hr=%8lXh", hr);
      if( D3DERR_COMMAND_UNPARSED == hr )
        lpdp2d->dwErrorOffset = (LPBYTE)lpCmd - (LPBYTE)(lpdp2d->lpDDCommands->lpGbl->fpVidMem);

      break;
    }

    // Check and see if we have reached the end of the command stream.
    if( (DWORD)lpCmd >= cmdEnd )
      break;
  }

  lpdp2d->ddrval = hr;

#if ENABLE_LOG_FILE
  /* retro3dfx: record non-parse DP2 failures (unknown-command callback etc.) */
  if (D3D_OK != hr)
    retroLogForce(ppdev, "retro3dfx DP2-EXIT-ERR: hr=%08lXh lastOp=%d errOff=%ld\r\n",
                  (DWORD)hr, (int)lpCmd->bCommand, lpdp2d->dwErrorOffset);
#endif

#if defined(WINNT) && ENABLE_ERROR_CHECKING
Exit_DrawPrimitives2:
#endif

  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

/*-------------------------------------------------------------------
Function Name:  bufferStateSetup

Description:    Setup the buffers for rendering.

Return:         DWORD
-------------------------------------------------------------------*/
DWORD bufferStateSetup( RC *pRc, DWORD unconditionalSet )
{
  SETUP_PPDEV(pRc)

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
  ULONG ColBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);
#else
  ULONG ColBufferAddr = GET_HW_ADDR(pRc->lpDDS);
#endif

#ifdef DEBUG
  ULONG lastColBufferAddr = _D3(last).colBufferAddr;  // to help with debug
#endif

  CMDFIFO_PROLOG(cmdFifo);

  CLEAR_HW_STATE( SC_BUFFERS );

  // P R I M A R Y   C O L O R   B U F F E R
  if (((DWORD)pRc != _D3(lastContext)) ||
      unconditionalSet ||
      (_D3(last).colBufferAddr != ColBufferAddr))
  {
#ifdef NEW_CLIP_FOR_GB
      CMDFIFO_CHECKROOM( cmdFifo, 1 * (PH1_SIZE + 2) );
#else
      CMDFIFO_CHECKROOM( cmdFifo, 2 * (PH1_SIZE + 2) );
#endif

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );

  #if ENABLE_TILED_HEAP
    if(IS_TILED(ColBufferAddr))
    {
      DWORD colBufferStride;

      colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

      SETPD( cmdFifo, ghw0->colBufferAddr,   (ColBufferAddr & 0x7FFFFFFFL));
      SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
      D3DPRINT(D3DDBGLVL, "  colBufferAddr=%08lXh, colBufferStride=%08lXh",
               (ColBufferAddr & 0x7FFFFFFFL), colBufferStride);
    }
    else
  #endif
    {
      SETPD( cmdFifo, ghw0->colBufferAddr,   ColBufferAddr);
      SETPD( cmdFifo, ghw0->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
      D3DPRINT(D3DDBGLVL, "  colBufferAddr=%08lXh, colBufferStride=%08lXh",
               ColBufferAddr, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
    } // Tiled?

#ifndef NEW_CLIP_FOR_GB
    // Reset the clipping registers
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
    SETPD( cmdFifo, ghw0->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
    SETPD( cmdFifo, ghw0->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);
#endif
    _D3(last).colBufferAddr = ColBufferAddr;
  } // Primary color buffer last != current?

#ifdef NEW_CLIP_FOR_GB
  if (((DWORD)pRc != _D3(lastContext)) ||
       (_D3(last).clipLeftRight != pRc->sst.clipLeftRight) ||
       (_D3(last).clipBottomTop != pRc->sst.clipBottomTop)) 
  {
        CMDFIFO_CHECKROOM( cmdFifo, 1 * (PH1_SIZE + 2) );
        // Reset the clipping registers
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
        SETPD( cmdFifo, ghw0->clipLeftRight, pRc->sst.clipLeftRight);
        SETPD( cmdFifo, ghw0->clipBottomTop, pRc->sst.clipBottomTop);

        D3DPRINT(D3DDBGLVL, "  clipLeftRight=%08lXh, clipBottomTop=%08lXh",
                 pRc->sst.clipLeftRight, pRc->sst.clipBottomTop);

        _D3(last).clipLeftRight = pRc->sst.clipLeftRight;
        _D3(last).clipBottomTop = pRc->sst.clipBottomTop;
  }
#endif

#ifdef SLI_AA
  // S E C O N D A R Y   C O L O R   B U F F E R
  // If antialiasing is enabled, setup the secondary color buffer
  if (IS_NAPALM && _DD(ddAAModeEnabled))
  {

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
  ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSHndl);
#else
  ULONG AABufferAddr = GET_AAHW_ADDR(pRc->lpDDS);
#endif

#ifdef DEBUG
  ULONG lastAABufferAddr = _D3(last).colAABufferAddr; // to help with debug
#endif
	if (AABufferAddr) // Make sure that the AA buffers are allocated. 
	{
      if (((DWORD)pRc != _D3(lastContext)) ||
          unconditionalSet ||
          (_D3(last).colAABufferAddr != AABufferAddr))
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2) );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );

    #if ENABLE_TILED_HEAP
        if(IS_TILED(AABufferAddr))
        {
          DWORD colBufferStride;

          colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

          SETPD( cmdFifo, ghw0->colBufferAddr,   ((AABufferAddr & 0x7FFFFFFFL) | SST_BUFFER_BASE_SELECT));
          SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
          D3DPRINT(D3DDBGLVL, "  AAcolBufferAddr=%08lXh, AAcolBufferStride=%08lXh",
                   ((AABufferAddr & 0x7FFFFFFFL) | SST_BUFFER_BASE_SELECT), colBufferStride);
        }
        else
    #endif
        {
          SETPD( cmdFifo, ghw0->colBufferAddr,   (AABufferAddr | SST_BUFFER_BASE_SELECT));
          SETPD( cmdFifo, ghw0->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
          D3DPRINT(D3DDBGLVL, "  AAcolBufferAddr=%08lXh, AAcolBufferStride=%08lXh",
                   (AABufferAddr | SST_BUFFER_BASE_SELECT), TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
        } // Tiled?

        _D3(last).colAABufferAddr = AABufferAddr;
      } // Secondary color buffer last != current?
	} // AABufferAddr
  } // AA Enabled?
#endif // SLI_AA

  // G U A R D B A N D   C L I P P I N G
  if (IS_NAPALM && _D3(GuardbandClipping) )    // cws gb
  {
      // Reset the GuardBand clipping registers
      if (((DWORD)pRc != _D3(lastContext)) ||
          unconditionalSet ||
          (_D3(last).clipLeftRight1 != pRc->sst.clipLeftRight1) ||
          (_D3(last).clipBottomTop1 != pRc->sst.clipBottomTop1)    )
      {
          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2) );

          if (pRc->sst.clipLeftRight1 & 0x0010001)
          {
              D3DPRINT(255,"ODD ClipLeftRight1 %08x",pRc->sst.clipLeftRight1);
          }

          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight1 , 0xf ) );
          SETPD( cmdFifo, ghw0->clipLeftRight1, pRc->sst.clipLeftRight1);
          SETPD( cmdFifo, ghw0->clipBottomTop1, pRc->sst.clipBottomTop1);

          D3DPRINT(D3DDBGLVL, "  clipLeftRight1=%08lXh, clipBottomTop1=%08lXh",
                   pRc->sst.clipLeftRight1, pRc->sst.clipBottomTop1);

          _D3(last).clipLeftRight1 = pRc->sst.clipLeftRight1;
          _D3(last).clipBottomTop1 = pRc->sst.clipBottomTop1;
      }
  }

#ifdef DEBUG
  // Code to allow for the validation of guardband clipping.
  // SSTH3_GUARDBAND_CLIPPING must be set to 1 to enable GB clipping.
  {
	  static unsigned long ulGB = 0;

	  if (ulGB)
	  {
		  // Reduce clipping extents by 20 pixels, by default.
		  static int ulGBAdjust = 20;

		  unsigned long ulLeft;
		  unsigned long ulRight;
		  unsigned long ulBottom;
		  unsigned long ulTop;
		  unsigned long ulLeftRight;
		  unsigned long ulBottomTop;

		  ulLeft   = (pRc->sst.clipLeftRight1 & SST_CLIPLEFT)   >> SST_CLIPLEFT_SHIFT;
		  ulRight  = (pRc->sst.clipLeftRight1 & SST_CLIPRIGHT)  >> SST_CLIPRIGHT_SHIFT;
		  ulBottom = (pRc->sst.clipBottomTop1 & SST_CLIPBOTTOM) >> SST_CLIPBOTTOM_SHIFT;
		  ulTop    = (pRc->sst.clipBottomTop1 & SST_CLIPTOP)    >> SST_CLIPTOP_SHIFT;

		  ulLeft   += ulGBAdjust;
		  ulRight  -= ulGBAdjust;
		  ulBottom += ulGBAdjust;
		  ulTop    -= ulGBAdjust;

		  ulLeft   = (ulLeft   << SST_CLIPLEFT_SHIFT)   & SST_CLIPLEFT;
		  ulRight  = (ulRight  << SST_CLIPRIGHT_SHIFT)  & SST_CLIPRIGHT;
		  ulBottom = (ulBottom << SST_CLIPBOTTOM_SHIFT) & SST_CLIPBOTTOM;
		  ulTop    = (ulTop    << SST_CLIPTOP_SHIFT)    & SST_CLIPTOP;

		  ulLeftRight = pRc->sst.clipLeftRight1 & SST_ENRECTCLIP1;
		  ulBottomTop = pRc->sst.clipBottomTop1 & SST_RECTCLIP1_EX;

		  ulLeftRight |= ulLeft   | ulRight;
		  ulBottomTop |= ulBottom | ulTop;

		  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight1 , 0xf ) );
		  SETPD( cmdFifo, ghw0->clipLeftRight1, ulLeftRight);
		  SETPD( cmdFifo, ghw0->clipBottomTop1, ulBottomTop);
	  }
  }
#endif
  // E N D   G U A R D B A N D   C L I P P I N G

  // Z   B U F F E R
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  if (pRc->DDSZHndl && (pRc->zEnable || pRc->zWriteEnable))
#else
  if (pRc->zEnable || pRc->zWriteEnable)
#endif
  {

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    ULONG ZBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
#else
    ULONG ZBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
#endif

#ifdef DEBUG
    ULONG lastZBufferAddr = _D3(last).auxBufferAddr; // to help with debug
#endif

    // P R I M A R Y   Z   B U F F E R
    if (((DWORD)pRc != _D3(lastContext)) ||
        unconditionalSet ||
        (_D3(last).auxBufferAddr != ZBufferAddr))
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );

  #if ENABLE_TILED_HEAP
      if(IS_TILED(ZBufferAddr))
      {
        DWORD auxBufferStride;

        auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

        SETPD( cmdFifo, ghw->auxBufferAddr,   (ZBufferAddr & 0x7fffffffL));
        SETPD( cmdFifo, ghw->auxBufferStride, auxBufferStride);
        D3DPRINT(D3DDBGLVL, "  auxBufferAddr=%08lXh, auxBufferStride=%08lXh",
                 (ZBufferAddr & 0x7FFFFFFFL), auxBufferStride);
      }
      else
  #endif
      {
        SETPD( cmdFifo, ghw->auxBufferAddr,   ZBufferAddr);
        SETPD( cmdFifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
        D3DPRINT(D3DDBGLVL, "  auxBufferAddr=%08lXh, auxBufferStride=%08lXh",
                 ZBufferAddr, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
      } // Tiled?

      _D3(last).auxBufferAddr = ZBufferAddr;
    } // primary Z buffer last != current?

    // fix for PRS 14417, Homeworld has a valid zbuffer, and has pRc->zEnable clear with pRc->zWriteEnable set
    // this is supposed to disable z buffering, but we're leaving the ZAWRMASK bit set in fbzMode
    // which is causing incorrect data to be put in the zbuffer for the rest of the frame
    if ((D3DZB_FALSE == pRc->zEnable) && pRc->zWriteEnable)
    {
      pRc->sst.fbzMode &= ~(SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL | SST_WBUFFER | SST_ZAWRMASK);
    }

#ifdef SLI_AA
    // S E C O N D A R Y   Z   B U F F E R
    // If antialiasing is enabled, setup the secondary Z buffer
    if (IS_NAPALM && _DD(ddAAModeEnabled))
    {

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
#else
    ULONG AABufferAddr = GET_AAHW_ADDR(pRc->lpDDSZ);
#endif

#ifdef DEBUG
    ULONG lastAABufferAddr = _D3(last).auxAABufferAddr; // to help with debug
#endif

      if (((DWORD)pRc != _D3(lastContext)) ||
          unconditionalSet ||
          (_D3(last).auxAABufferAddr != AABufferAddr))
      {
          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2) );

          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );

        #if ENABLE_TILED_HEAP
            if(IS_TILED(AABufferAddr))
            {
              DWORD auxBufferStride;

              auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

              SETPD( cmdFifo, ghw->auxBufferAddr,   ((AABufferAddr & 0x7fffffffL) | SST_BUFFER_BASE_SELECT));
              SETPD( cmdFifo, ghw->auxBufferStride, auxBufferStride);
              D3DPRINT(D3DDBGLVL, "  AAauxBufferAddr=%08lXh, AAauxBufferStride=%08lXh",
                       ((AABufferAddr & 0x7FFFFFFFL) | SST_BUFFER_BASE_SELECT), auxBufferStride);
            }
            else
        #endif
            {
              SETPD( cmdFifo, ghw->auxBufferAddr,   (AABufferAddr | SST_BUFFER_BASE_SELECT));
              SETPD( cmdFifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
              D3DPRINT(D3DDBGLVL, "  AAauxBufferAddr=%08lXh, AAauxBufferStride=%08lXh",
                       (AABufferAddr| SST_BUFFER_BASE_SELECT), TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
            } // Tiled?

            _D3(last).auxAABufferAddr = AABufferAddr;
        } // secondary Z buffer last != current?
      } // AA enabled?
#endif // SLI_AA
    } // Z enabled?
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    else if ((0 != pRc->DDSZHndl))
    {
	  // A z-buffer has been introduced so set up the hardware state accordingly.
	  extern RENDERFXN_RETVAL __stdcall zEnable(RC *pRc, ULONG state) ;
	  extern RENDERFXN_RETVAL __stdcall zWriteEnable(RC *pRc, ULONG state) ;
	  (void)zWriteEnable(pRc,pRc->zWriteEnable);
	  (void)zEnable(pRc,pRc->zEnable);
    }
    else if ((0 == pRc->DDSZHndl))
    {																	  																	  
      // No z buffer. Update the hardware state but leave the z render states as they are.
      pRc->sst.fbzMode &= ~(SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL | SST_WBUFFER);

#ifdef Z_ACCESS_OPT
      // Since we are disabling the ZBuffer, lets clear the CachedFBZMode flags
      if ( _DD(ddEnableZClearOpt) )
      {
        if ( _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
        {
          pRc->dwCachedFBZMode = 0;
    
          if (pRc->dwZClearOptEnabled)
          {
            _DD(ddFlipsWithoutZClear) = 0x80000000;
          }
          else
          {
            // Don't overwrite a reset status.
            if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
              _DD(ddFlipsWithoutZClear) = 0;
          }
        }
      }
#endif

      {
        DWORD offset = getFnZOffset( pRc );
        pRc->state = (pRc->state & ~FNZ_STATE_MASK) | FnZTable[ offset ][ FNZ_STATE ];
        pRc->sst.sSetupMode = (pRc->sst.sSetupMode & ~FNZ_SETUP_MASK) | FnZTable[ offset ][ FNZ_SETUP ];
        pRc->sst.fogMode = (pRc->sst.fogMode & ~FNZ_FOGMODE_MASK) | FnZTable[ offset ][ FNZ_FOGMODE ];
      }

      // no z buffer, so undo the zWriteEnable render state settings
      pRc->sst.fbzMode &= ~SST_ZAWRMASK;
    }
#endif

  CMDFIFO_EPILOG( cmdFifo );

  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  HandleSLIAA

Description:    Lightweight SLI/AA SetRenderTarget Change

Return:         DWORD
-------------------------------------------------------------------*/
void HandleSLIAA(RC * pRc)
{
   SETUP_PPDEV(pRc)
   ULONG ulBpp = GETSURFBPP(pRc->DDSHndl);
   ULONG ulStencilMode, ulStencilModeLast;
   DWORD dwChipMask;
   DWORD dwChipMask1, dwChipMask2;
   DWORD dwChipMask3, dwChipMask4;
#ifndef WINNT
   extern DWORD * pHotKeyData;
#else
   DWORD * pHotKeyData = NULL;
#endif

   CMDFIFO_PROLOG(cmdFifo);

#ifdef Z_ACCESS_OPT
// Main code for handling Z_ACCESS_OPT
//
// If Z_CLEAR_OPTIMIZATION
// {
//   If ( We have not flipped more than 6 times, and we are not in a reset state ( 0x80000000 ) )
//        and we are in full screen, and we have 3 - 4 3D Buffers allocated
//   {
//     We cache the Z Buffer State
//	   Then we disable the Z Buffer
//   }
//   else If we are in a reset state
//   { 
//     If we are in full screen, and we have 3-4 3D Buffers allocated
//     Then we reset the previous Z Buffer state and reset the cached variable
//     Reset the Flip Counter
//   }
    if ( _DD(ddEnableZClearOpt) )
	{
	  if ( pRc->zEnable && _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && 
	       (_FF(dd3DSurfaceCount) < 5))
	  {
        if ( ((_DD(ddFlipsWithoutZClear) & 0x7fffffff) > 6) &&
             (pRc->dwZClearOptEnabled == 0) )
        {
          pRc->dwCachedFBZMode = ((pRc->sst.fbzMode) & (SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL));

          pRc->dwZClearOptEnabled = 1;

          // no z buffer, so undo the zEnable render state settings
          pRc->sst.fbzMode &= ~(SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL);
        }
        else if ( (_DD(ddFlipsWithoutZClear) == 0x80000000) && 
                  (pRc->dwZClearOptEnabled == 1) )
        {
          pRc->sst.fbzMode |= pRc->dwCachedFBZMode;
          pRc->dwCachedFBZMode = 0;
          pRc->dwZClearOptEnabled = 0;
          _DD(ddFlipsWithoutZClear) = 0;
        }
      }
	  else // We are not in fullscreen and Z Is not enabled and we have more than
	  {    // 3 buffers
        // If the optimization was enabled, disable it now!
        if ( (pRc->dwZClearOptEnabled == 1) )
        {
          pRc->sst.fbzMode |= pRc->dwCachedFBZMode;
          pRc->dwCachedFBZMode = 0;
          pRc->dwZClearOptEnabled = 0;
        }

	    _DD(ddFlipsWithoutZClear) = 0;
      pRc->dwCachedFBZMode = 0;
	  }
	}
#endif

    if (ulBpp != 32)
    {
      // Disable stenciling.
      ulStencilMode     = pRc->sst.stencilMode & ~(SST_STENCIL_ENABLE);
      ulStencilModeLast = _D3(last).stencilMode & ~(SST_STENCIL_ENABLE);
    }
    else
    {
      ulStencilMode     = pRc->sst.stencilMode;
      ulStencilModeLast = _D3(last).stencilMode;
    }

    dwChipMask = pRc->sst.chipMask;
    if (2 == _FF(dwNumUnits))
      {
      dwChipMask1 = SST_CHIP_MASK_CHIP_0_ENABLE;
      dwChipMask2 = SST_CHIP_MASK_CHIP_1_ENABLE;
      }
    else
      {
      if (_DD(ddAANumberSamples) >= 4)
         {
         dwChipMask1 = SST_CHIP_MASK_CHIP_0_ENABLE;
         dwChipMask2 = SST_CHIP_MASK_CHIP_1_ENABLE;
         dwChipMask3 = SST_CHIP_MASK_CHIP_2_ENABLE;
         dwChipMask4 = SST_CHIP_MASK_CHIP_3_ENABLE;
         }
      else
         {
         dwChipMask1 = SST_CHIP_MASK_CHIP_2_ENABLE | SST_CHIP_MASK_CHIP_0_ENABLE;
         dwChipMask2 = SST_CHIP_MASK_CHIP_3_ENABLE | SST_CHIP_MASK_CHIP_1_ENABLE;
         }
      }

#ifdef SLI_AA
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
    if ((_DD(ddAAModeEnabled) || (_DD(ddAANumberSamples) > 0)) && (IS_TILED(GET_HW_OFFSET(pRc->DDSHndl))))
#else
    if ((_DD(ddAAModeEnabled) || (_DD(ddAANumberSamples) > 0)) && (IS_TILED(GET_HW_ADDR(pRc->lpDDS))))
#endif
    {
     // A N T I A L I A S I N G
     // Implies napalm.
     // Check Target.. If Target in Linear Space then turn off AA
      if (_DD(ddAANumberSamples) > 0)
      {
        // Disable AA if AA mode is enabled but the AA buffers are not allocated.
        //
        // !(_DD(ddAAModeEnabled)) handles the 2sample case, because we don't allocate
        // AA buffers on each chip - the full primary buffer is on one chip and the
        // full AA buffer is on the other.
        // (GET_AAHW_ADDR(pRc->lpDDS)) handles the 4sample case.
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
        if ( !(_DD(ddAAModeEnabled)) || (GET_AAHW_OFFSET(pRc->DDSHndl)) )
#else
        if ( !(_DD(ddAAModeEnabled)) || (GET_AAHW_ADDR(pRc->lpDDS)) )
#endif
        {
          switch ( _DD(ddAANumberSamples) )
          {
            case 2: // 2 chip or 4 chip
            {
              if (_FF(dwNumUnits) > 1)
              {
                CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 4 );
                
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0
                
                // Primary Offset goes to chip 1
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_2Smpl_Chp0));
                }
			   
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1
                
                // Secondary Offset goes to chip 2
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_2Smpl_Chp1));
                }
              }
              else  // single chip
              {
                CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 2 );
                
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask);  // Focus write to chip 0,1
                
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_2Smpl) | SST_AA_CONTROL_AA_ENABLE);
                }
              }
              break;
            }

            case 4: // 2 or 4 chip
            {
              if (4 == _FF(dwNumUnits))
              {
                CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 8 );
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0,2
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp0));
                }
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1,3
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp1));
                }
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask3);  // Focus write to chip 1,3
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp2));
                }
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask4);  // Focus write to chip 1,3
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, 0x0);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp3));
                }
                D3DPRINT(D3DDBGLVL, "  4 sample aa: chipMask=%08lXh, aaCtrl=%08lXh, chipMask=%08lXh, aaCtrl=%08lXh chipMask=%08lXh, aaCtrl=%08lXh chipMask=%08lXh, aaCtrl=%08lXh",
                         dwChipMask1, _D3(aaJitterValues_4Smpl_Chp0),
                         dwChipMask2, _D3(aaJitterValues_4Smpl_Chp1),
                         dwChipMask3, _D3(aaJitterValues_4Smpl_Chp2),
                         dwChipMask4, _D3(aaJitterValues_4Smpl_Chp3));
              }
              else
              {
                CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 4 );
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0,2
                
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp0) | SST_AA_CONTROL_AA_ENABLE);
                }
              
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
                SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1,3
                
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
                if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
                {
                  SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
                }
                else
                {
                  SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_4Smpl_Chp1) | SST_AA_CONTROL_AA_ENABLE);
                }
                
                D3DPRINT(D3DDBGLVL, "  4 sample aa: chipMask=%08lXh, aaCtrl=%08lXh, chipMask=%08lXh, aaCtrl=%08lXh",
                         dwChipMask1, _D3(aaJitterValues_4Smpl_Chp0) | SST_AA_CONTROL_AA_ENABLE,
                         dwChipMask2, _D3(aaJitterValues_4Smpl_Chp1) | SST_AA_CONTROL_AA_ENABLE);
              }

              break;
            }

            case 8: // 4 chip only
            {
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 2 * 4 );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0,2
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
              if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
              {
                SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
              }
              else
              {
                SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_8Smpl_Chp0) | SST_AA_CONTROL_AA_ENABLE);
              }
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1,3
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
              if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
              {
                SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
              }
              else
              {
                SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_8Smpl_Chp1) | SST_AA_CONTROL_AA_ENABLE);
              }
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask3);  // Focus write to chip 1,3
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
              if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
              {
                SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
              }
              else
              {
                SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_8Smpl_Chp2) | SST_AA_CONTROL_AA_ENABLE);
              }
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask4);  // Focus write to chip 1,3
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
              if ((pRc->dwZeroJitter) || ((NULL != pHotKeyData) && (1 == *pHotKeyData)))
              {
                SETPD( cmdFifo, ghw->aaCtrl, SST_AA_CONTROL_AA_ENABLE);
              }
              else
              {
                SETPD( cmdFifo, ghw->aaCtrl, _D3(aaJitterValues_8Smpl_Chp3) | SST_AA_CONTROL_AA_ENABLE);
              }
              D3DPRINT(D3DDBGLVL, "  4 sample aa: chipMask=%08lXh, aaCtrl=%08lXh, chipMask=%08lXh, aaCtrl=%08lXh chipMask=%08lXh, aaCtrl=%08lXh chipMask=%08lXh, aaCtrl=%08lXh",
                       dwChipMask1, _D3(aaJitterValues_8Smpl_Chp0) | SST_AA_CONTROL_AA_ENABLE,
                       dwChipMask2, _D3(aaJitterValues_8Smpl_Chp1) | SST_AA_CONTROL_AA_ENABLE,
                       dwChipMask3, _D3(aaJitterValues_8Smpl_Chp2) | SST_AA_CONTROL_AA_ENABLE,
                       dwChipMask4, _D3(aaJitterValues_8Smpl_Chp3) | SST_AA_CONTROL_AA_ENABLE);
              break;
            }

            default:
            {
              D3DPRINT(0, "setDX6state: AA setup. Number of AA samples not 2 or 4 [%d]", _DD(ddAANumberSamples));
            
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 1 );
            
              // Invalid number of samples. Zero out the aaCtrl register.
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
              SETPD( cmdFifo, ghw->aaCtrl, 0x0);

              break;
            }
          }
        }
      }
      else
      {
        // AA enabled, but no samples (this shouldn't happen). Zero out the aaCtrl register.
        D3DPRINT(0, "setDX6state: Number of AA samples not > 0 [%d]", _DD(ddAANumberSamples));

        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 1 );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
        SETPD( cmdFifo, ghw->aaCtrl, 0x0);
      }

      // D I T H E R   M A T R I X   R O T A T I O N   S E L E C T

      if ( _D3(ditherRotation) && (_DD(ddAANumberSamples) > 0) && (ulBpp == 16) &&
      // Disable dither matrix rotation if AA mode is enabled but the AA buffers are not allocated.
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
          ( !(_DD(ddAAModeEnabled)) || (GET_AAHW_OFFSET(pRc->DDSHndl)) ) )
#else
          ( !(_DD(ddAAModeEnabled)) || (GET_AAHW_ADDR(pRc->lpDDS)) ) )
#endif
      {
        switch ( _DD(ddAANumberSamples) )
        {
          case 2: // 2 chip or 4 chip
          {
            if (_FF(dwNumUnits) > 1)
            {
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 6 );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode |  _D3(DithMatSel_2Smpl_Chp0) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode |  _D3(DithMatSel_2Smpl_Chp1) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask);  // Focus write to chip 0/chip 1
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
              SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
            }
            else // single chip
            {
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 3 );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask);  // Focus write to chip 0
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode |  _D3(DithMatSel_2Smpl) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
              SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
            }

            break;
          }

          case 4: //
          {
            if (4 == _FF(dwNumUnits))
            {
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 10 );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp0) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp1) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask3);  // Focus write to chip 2
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp2) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask4);  // Focus write to chip 3
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp3) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask);  // Focus write to chip 0/chip 1
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
              SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
              
              D3DPRINT(D3DDBGLVL, "  4 sample aa first 2: chipMask=%08lXh, fogMode=%08lXh, renderMode",
                       dwChipMask1, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp0),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION,
                       dwChipMask2, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp1),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
              D3DPRINT(D3DDBGLVL, "  4 sample aa last 2: chipMask=%08lXh, fogMode=%08lXh, renderMode",
                       dwChipMask3, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp2),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION,
                       dwChipMask4, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp3),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            }
            else
            {
              CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 6 );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0/2
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp0) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
              SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
              SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1/3
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
              SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp1) );
              
              SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
              SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
              
              D3DPRINT(D3DDBGLVL, "  2 sample aa: chipMask=%08lXh, fogMode=%08lXh, renderMode",
                       dwChipMask1, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp0),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION,
                       dwChipMask2, pRc->sst.fogMode | _D3(DithMatSel_4Smpl_Chp1),
                       pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            }

            break;
          }

          case 8: // 4 chip only
          {
            CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 10 );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, dwChipMask1);  // Focus write to chip 0
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
            SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp0) );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, dwChipMask2);  // Focus write to chip 1
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
            SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp1) );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, dwChipMask3);  // Focus write to chip 2
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
            SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp2) );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, dwChipMask4);  // Focus write to chip 3
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
            SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp3) );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, dwChipMask);  // Focus write to chip 0/chip 1
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
            SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode | SST_RM_DITHER_ROTATION );
            
            D3DPRINT(D3DDBGLVL, "8 sample aa: chipMask=%08lXh, fogMode=%08lXh, renderMode=%08lXh\n",
                     dwChipMask1, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp0),
                     pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            D3DPRINT(D3DDBGLVL, "8 sample aa: chipMask=%08lXh, fogMode=%08lXh, renderMode=%08lXh\n",
                     dwChipMask2, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp1),
                     pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            D3DPRINT(D3DDBGLVL, "8 sample aa: chipMask=%08lXh, fogMode=%08lXh, renderMode=%08lXh\n",
                     dwChipMask3, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp2),
                     pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            D3DPRINT(D3DDBGLVL, "8 sample aa: chipMask=%08lXh, fogMode=%08lXh, renderMode=%08lXh\n",
                     dwChipMask4, pRc->sst.fogMode | _D3(DithMatSel_8Smpl_Chp3),
                     pRc->sst.renderMode | SST_RM_DITHER_ROTATION);
            
            break;
          }
          
          default:
          {
            D3DPRINT(0, "setDX6state: DithMat setup. Number of AA samples not 2 or 4 [%d]", _DD(ddAANumberSamples));
            
            // Reset for subsequent writes.
            CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 3 );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, pRc->sst.chipMask);
            
            // Need to zero out dither matrix rotate bits and the enable dither rotation bit.
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
            SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode & SST_ROTDITHMATSELRESET );
            
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
            SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION );

            break;
          }
        }
      }
      else
      {
        // Reset for subsequent writes.
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 3 );
        
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
        SETPD( cmdFifo, ghw->chipMask, pRc->sst.chipMask);
        
        // AA enabled, but no samples (this shouldn't happen) or 32bpp. Zero out
        // dither matrix rotate bits and the enable dither rotation bit.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
        SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode & SST_ROTDITHMATSELRESET );
        
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION );
        
        D3DPRINT(D3DDBGLVL, "  fogMode=%08lXh, renderMode=%08lXh",
                 pRc->sst.fogMode & SST_ROTDITHMATSELRESET,
                 pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION);
      }
 
      // Reset for subsequent writes.
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
      SETPD( cmdFifo, ghw->chipMask, pRc->sst.chipMask);

      D3DPRINT(D3DDBGLVL, "  chipMask=%08lXh", pRc->sst.chipMask);
    }
    else // ddAAModeEnabled
    {
      if (IS_NAPALM)
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 3 );

        // When AA is disabled make sure to zero out the dither matrix rotate bits,
        // the enable dither rotation bit and the aaCtrl register.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
        SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode & SST_ROTDITHMATSELRESET );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, aaCtrl, 0 ) );
        SETPD( cmdFifo, ghw->aaCtrl, 0 );

        D3DPRINT(D3DDBGLVL, "  fogMode=%08lXh, renderMode=%08lXh, aaCtrl=%08lXh",
                 pRc->sst.fogMode & SST_ROTDITHMATSELRESET,
                 pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION,
                 0);
      }
      else
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0 ) );
        SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
        D3DPRINT(D3DDBGLVL, "  fogMode=%08lXh", pRc->sst.fogMode);
      }
    }

    //Fixup SLI Enable
    if (_DD(ddSLIModeEnabled))
      {
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) /* && !defined(WINNT) */
      if (IS_TILED(GET_HW_OFFSET(pRc->DDSHndl)))
#else
      if (IS_TILED(GET_HW_ADDR(pRc->lpDDS)))
#endif
         {
         DWORD AAShift;
         DWORD rMask;
         DWORD sMask;
         DWORD cMask;   
         DWORD sliCtrl;
         DWORD sliCtrlBase;
         DWORD i;
         DWORD log2NChips;

         // Enable 3d SLI
         CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 2 * _FF(dwNumUnits));

         // Special Case when AA is enabled and number of chips != Sli Ness
         log2NChips = _DD(dwlog2NumChips); 
         if ((_DD(ddSLINumberWays) != _FF(dwNumUnits)))
            AAShift = 1;
         else
            AAShift = 0;

         rMask = ((_FF(dwNumUnits) - 1) >> AAShift) << _DD(dwlog2BandHeight);
         sMask = _DD(ddSLINumberScanlines) - 1;
         sliCtrlBase = (rMask << SST_SLI_CONTROL_RENDER_MASK_SHIFT) | (log2NChips << SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT) | (sMask << SST_SLI_CONTROL_SCAN_MASK_SHIFT) | SST_SLI_CONTROL_SLI_ENABLE;
         for (i=0; i<_FF(dwNumUnits); i++)
            {
            cMask = ((i >> AAShift) << _DD(dwlog2BandHeight)) << SST_SLI_CONTROL_COMPARE_MASK_SHIFT;
            sliCtrl = sliCtrlBase | cMask; 
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdFifo, ghw->chipMask, BIT(i));
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, sliCtrl, 0 ) );
            SETPD( cmdFifo, ghw->sliCtrl, sliCtrl);
            D3DPRINT(D3DDBGLVL, "  chipMask=%08lXh, sliCtrl=%08lXh", BIT(i), sliCtrl);
            }
         CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 1);
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
         SETPD( cmdFifo, ghw->chipMask, pRc->sst.chipMask);
         D3DPRINT(D3DDBGLVL, "  chipMask=%08lXh", pRc->sst.chipMask);
         }
      else
         {
         // Disable 3d SLI
         CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) * 1 );

         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, sliCtrl, 0 ) );
         SETPD( cmdFifo, ghw->sliCtrl, 0x0);
         D3DPRINT(D3DDBGLVL, "  sliCtrl=%08lXh", 0);
         }
      }
   
    // M I S C   S T A T E
    if( HW_STATE_CHANGED & SC_NEED_NOP )
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 4) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R2|R3|R7, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      SETPD( cmdFifo, ghw->nopCMD, 0 );
    }
    else
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 3) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R2|R3, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
    }
    D3DPRINT(D3DDBGLVL, "  fbzColorPath=%08lXh, alphaMode=%08lXh, fbzMode=%08lXh",
             pRc->sst.fbzColorPath, pRc->sst.alphaMode, pRc->sst.fbzMode);

    if (IS_NAPALM)
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 2) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1, stencilMode, 0 ) );
#ifdef WINNT
      if (ulStencilMode & SST_STENCIL_ENABLE)
#else
      if ( ((DWORD)pRc != _D3( lastContext )) || (ulStencilMode & SST_STENCIL_ENABLE) )
#endif
      {
        SETPD( cmdFifo, ghw->stencilMode, ulStencilMode );
        SETPD( cmdFifo, ghw->stencilOp, pRc->sst.stencilOp );
        D3DPRINT(D3DDBGLVL, "  stencilMode=%08lXh, stencilOp=%08lXh",
                 ulStencilMode, pRc->sst.stencilOp);
      }
      else
      {
        SETPD( cmdFifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
        SETPD( cmdFifo, ghw->stencilOp, 0); // D3DSTENCILOP_KEEP
        D3DPRINT(D3DDBGLVL, "  stencilMode=%08lXh, stencilOp=%08lXh",
                 SST_STENCIL_MODE_DISABLE, 0);
      }
    }
#else // SLI_AA
    // M I S C   S T A T E
    if( HW_STATE_CHANGED & SC_NEED_NOP )
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 5) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2|R3|R7, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      SETPD( cmdFifo, ghw->nopCMD, 0 );
    }
    else
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 4) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2|R3, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
    }

    if (IS_NAPALM)
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 3) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2, renderMode, 0 ) );
      SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode );

      if (ulStencilMode & SST_STENCIL_ENABLE)
      {
        SETPD( cmdFifo, ghw->stencilMode, ulStencilMode );
        SETPD( cmdFifo, ghw->stencilOp, pRc->sst.stencilOp );
      }
      else
      {
        SETPD( cmdFifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
        SETPD( cmdFifo, ghw->stencilOp, 0); // D3DSTENCILOP_KEEP
      }
    }
#endif // SLI_AA

#ifdef DEBUG
	{
		// Variable to allow enabling code that selects seperate RGB channel writes.
		static unsigned long ulRGBWrites = 0;

		if (ulRGBWrites)
		{
			// Variable to allow toggling of channel to write.
			// Values are: 0 = Red, 1 = Green, 2 = Blue
			static unsigned long ulRGB = 0;
			unsigned long ulRenderMode = pRc->sst.renderMode & ~(SST_RM_RGBA_WMASK);

			if (ulRGB == 0)
			{
				SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
				SETPD( cmdFifo, ghw->renderMode, ulRenderMode | SST_RM_RED_WMASK);
			}

			if (ulRGB == 1)
			{
				SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
				SETPD( cmdFifo, ghw->renderMode, ulRenderMode | SST_RM_GREEN_WMASK);
			}

			if (ulRGB == 2)
			{
				SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0 ) );
				SETPD( cmdFifo, ghw->renderMode, ulRenderMode | SST_RM_BLUE_WMASK);
			}
		}

	}
#endif // ifdef DEBUG
	
	CMDFIFO_EPILOG( cmdFifo );
}

/*-------------------------------------------------------------------
Function Name:  setDX6state

Description:    Setup the accelerator for a given state

Return:         DWORD
-------------------------------------------------------------------*/
DWORD setDX6state( RC *pRc, DWORD primitiveType, DWORD count )
{
   SETUP_PPDEV(pRc)

#if defined(NULLDRIVER)
  if (_D3(ondrtHWSU))
  {
#endif

  ULONG ulBpp = GETSURFBPP(pRc->DDSHndl);
  ULONG ulStencilMode, ulStencilModeLast;

  CMDFIFO_PROLOG(cmdFifo);

  ///////////////////////////////////////////////////////////////////////////////////////
  // A D J U S T   A L P H A   B L E N D   M O D E S
  // For Napalm:
  // 1. If necessary, adjust RGB component alpha blend modes.
  // 2. If not in 32bpp mode:
  //    a. Disable stenciling.
  //    b. If necessary, adjust alpha component alpha blend modes.
  if (IS_NAPALM)
  {
    if (CHECK_HW_STATE(SC_ALPHABLEND))
    {
      switch ((pRc->sst.alphaMode & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT)
      {
        case SST_A_DSTALPHA:
          pRc->sst.alphaMode &= ~(SST_RGBSRCFACT); // Clear RGB source
          pRc->sst.alphaMode |= (SST_A_ONE << SST_RGBSRCFACT_SHIFT);
          break;
        case SST_AOM_DSTALPHA:
        case SST_A_SATURATE:
          pRc->sst.alphaMode &= ~(SST_RGBSRCFACT); // Clear RGB source
          pRc->sst.alphaMode |= (SST_A_ZERO << SST_RGBSRCFACT_SHIFT);
          break;
      }

      switch ((pRc->sst.alphaMode & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT)
      {
        case SST_A_DSTALPHA:
          pRc->sst.alphaMode &= ~(SST_RGBDSTFACT); // Clear RGB destination
          pRc->sst.alphaMode |= (SST_A_ONE << SST_RGBDSTFACT_SHIFT);
          break;
        case SST_AOM_DSTALPHA:
        case SST_A_COLORBEFOREFOG :
          pRc->sst.alphaMode &= ~(SST_RGBDSTFACT); // Clear RGB destination
          pRc->sst.alphaMode |= (SST_A_ZERO << SST_RGBDSTFACT_SHIFT);
          break;
      }
    }

    if (ulBpp != 32)
    {
      // Disable stenciling.
      ulStencilMode     = pRc->sst.stencilMode & ~(SST_STENCIL_ENABLE);
      ulStencilModeLast = _D3(last).stencilMode & ~(SST_STENCIL_ENABLE);

      // Check for blend modes that require destination alpha, but none exists.
      //
      // Even though srcAlpha might exist, you still need destination alpha
      // (which doesn't exist in non-32bpp modes) to perform the actual blend
      // operation. So, if the alpha component is anything other than zero or
      // one, adjust.

      if (CHECK_HW_STATE(SC_ALPHABLEND))
      {
        switch ((pRc->sst.alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT)
        {
          case SST_A_DSTALPHA:
          case SST_A_SRCALPHA:
          case SST_AOM_SRCALPHA:
            pRc->sst.alphaMode &= ~(SST_ASRCFACT);   // Clear Alpha source
            pRc->sst.alphaMode |= (SST_A_ONE << SST_ASRCFACT_SHIFT);
            break;
          case SST_AOM_DSTALPHA:
            pRc->sst.alphaMode &= ~(SST_ASRCFACT);   // Clear Alpha source
            pRc->sst.alphaMode |= (SST_A_ZERO << SST_ASRCFACT_SHIFT);
            break;
        }

        switch ((pRc->sst.alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT)
        {
          case SST_A_DSTALPHA:
          case SST_A_SRCALPHA:
          case SST_AOM_SRCALPHA:
            pRc->sst.alphaMode &= ~(SST_ADSTFACT);   // Clear Alpha destination
            pRc->sst.alphaMode |= (SST_A_ONE << SST_ADSTFACT_SHIFT);
            break;
          case SST_AOM_DSTALPHA:
            pRc->sst.alphaMode &= ~(SST_ADSTFACT);   // Clear Alpha destination
            pRc->sst.alphaMode |= (SST_A_ZERO << SST_ADSTFACT_SHIFT);
            break;
        }
      }
    }
    else
    {
      ulStencilMode     = pRc->sst.stencilMode;
      ulStencilModeLast = _D3(last).stencilMode;
    }

    // Reset the bit that indicates src/dst alpha blend has changed.
    CLEAR_HW_STATE(SC_ALPHABLEND);
  }
  // E N D   A D J U S T   A L P H A   B L E N D   M O D E S
  ///////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////
  //  B U F F E R   C H A N G E D
  if( HW_STATE_CHANGED & SC_BUFFERS )
  {
    CMDFIFO_SAVE( cmdFifo );

    bufferStateSetup(pRc, FALSE);

    CMDFIFO_RELOAD( cmdFifo );
  }

  // S U R F A C E   P I X E L   D E P T H
  // C O L U M N   B A N D   C O N T R O L
  // 2 P P C   B A N D   H E I G H T
  if (IS_NAPALM)
  {
      // set target surface pixel depth.
      // this macro modifies the renderMode and fbzMode registers.
      SETSURFACEPIXELDEPTH(pRc, pRc->DDSHndl);

      // Set the triangle iterator column band control.
      // This value is taken from a registry key, or is zero if no key exists.
      pRc->sst.fbzColorPath |= (( _D3(columnBandControl) << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT ) 
                                & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL );

      // Clear then set the 2ppc band height.
      pRc->sst.renderMode &= ~ SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION;
      pRc->sst.renderMode |=  ( _D3(TwoPpcLog2BandHeight) << SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT);
  }

  //  E N D    B U F F E R    C H A N G E D
  /////////////////////////////////////////////////////////////////////////////////////

  // This is used when triangle flavour profiling under DEBUG mode. -Ade
  triangleFlavour( pRc, pRc->state, primitiveType, count );

#if defined(TNL_PROFILE) && defined(TnL_HAL)
  TnLFlavour(pRc, primitiveType, count);
#endif
  //------------------------------------------------------------
  // Update Context
  // Either we just changed contexts or something in the context
  // changed and we need to update the hardware state.
  //------------------------------------------------------------

  // Check if our context has changed. -Ade
  if( (DWORD)pRc != _D3( lastContext ) )
  {
    UPDATE_HW_STATE( SC_FOG | SC_TEXTUREFACTOR | SC_ZBIAS | SC_NEED_NOP );
    UPDATE_FOG_STATE( SC_FOGALL );

    ASM_UPDATE_RC( pRc );

    _D3(lastContext) = (DWORD)pRc;

   // srogers 8/17/99 - Reload textures if context is switched
   // Fixes PRS 6247
    pRc->textureStage[0].changed = 1;
    pRc->textureStage[1].changed = 1;
  }

  /////////////////////////////////////////////////////////////////////////////////////
  // H A R D W A R E   S T A T E   C H A N G E D

  // Check if any hardware state has changed. -Ade
  if( HW_STATE_CHANGED )
  {
    // T E X T U R I N G
    setupTexturing( pRc );
#ifdef TnL_HAL 
    // Texture stage was just reset
	// Tell SetupFVFData that we need to recalculate 
	// the number of textures and index to the fast
	// path renderer
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FVFOUT;
#endif

    // 2   P I X E L S   P E R   C L O C K
    if (IS_NAPALM)
    {
        /* If going from 2ppc to 1ppc, we need to send 12 NOPs. */
#ifdef WINNT
      // _D3(lastContext) was already set to pRc above if they weren't the same
      // so it's rather pointless the compare them here
      if (_D3(NapalmIsIn2ppc) &&
          ((!(pRc->sst.combineModeT0 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)) ||
           (!(pRc->sst.combineModeT1 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))))
#else
      if ( (DWORD)pRc != _D3( lastContext ) || 
            ( _D3(NapalmIsIn2ppc) &&
                ( (!(pRc->sst.combineModeT0 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)) ||
                (!(pRc->sst.combineModeT1 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))   ) )  )
#endif
      {
         CMDFIFO_CHECKROOM( cmdFifo,  (PH1_SIZE + 12) );

		 // Send to TMU0 and TMU1 only.
         SETPH(cmdFifo, CMDFIFO_BUILD_PK1CHIP( 12, 0, nopCMD, 0x6 ) );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
         SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
      }

      // track 2ppc mode written to chips
      if (pRc->sst.combineModeT0 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
      {
         _D3(NapalmIsIn2ppc) = 1;
      }
      else
      {
         _D3(NapalmIsIn2ppc) = 0;
      }

      // we need to write combineMode before all other 3D regs to
      // clear/set 2ppc mode in case it changed!
      CMDFIFO_CHECKROOM( cmdFifo,  3 * (PH1_SIZE + 1) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );
      SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
      SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, pRc->sst.combineModeT0 );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
      SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, pRc->sst.combineModeT1 );

      D3DPRINT(D3DDBGLVL, "  combineModeFBI=%08lXh, combineModeT0=%08lXh, combineModeT1=%08lXh",
               pRc->sst.combineModeFBI, pRc->sst.combineModeT0, pRc->sst.combineModeT1);

    } // E N D   2   P I X E L S   P E R   C L O C K

    if( HW_STATE_CHANGED & SC_TRIANGLE_FLAVOUR )
      setDrawTriangle6(pRc);

    // T E X T U R I N G   A N D   C H R O M A K E Y I N G

    if ( (pRc->state & (STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1))

#if /* defined(WINNT) && */ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    // to fix potential access violation issues with NT Stress
    // verify we have a TXTRHNDL for this texture before dereferencing it
    //
    // this check protects all use of TXTRHNDL_PTR (both explict and use inside
    // other macros) withing this if block

          && (TXTRHNDL_INRANGE(pRc->texture) && TXTRHNDL_PTR(pRc->texture))
#endif
       )
    {
      TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);

#ifdef LODBIASPERCHIP
      // If we are multitexturing and trilinear filtering, and number
      // chips > 1, set the Lod Bias per chip.
      if (pRc->dwMultitextureAndTrilinear && (_FF(dwNumUnits) > 1))
	  {
		DWORD dwLod;
		int   i;

		// This is not the most efficient way to do this, but this is the first pass to make sure it
		// works. It would make sense to not rewrite the texturemode and texture address registers
		// for each iteration.
		for (i = 0; i < (int) _FF(dwNumUnits); i++)
		{
          // partial fix for PC99 TextureSizes & PC99 MultiTexturing failures
          // flush the texture cache
          CMDFIFO_CHECKROOM(cmdFifo, 2 * ((PH1_SIZE + 1) + (PH4_SIZE + 2) + (PH4_SIZE + 1) + (PH4_SIZE + 1)));

		  // Set which chip will receive the following writes.
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
          SETPD( cmdFifo, ghw->chipMask, 1L << i);

          SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, textureMode, TMU2CHIP(TREX0)));
          SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT0);

          D3DPRINT(D3DDBGLVL, "  TREX0: textureMode=%08lXh", pRc->sst.textureMode | pRc->sst.textureModeT0);

          pRc->sst.tLOD &= ~SST_LODMIN;

          // The following code adjusts the higher level-of-detail for mipmaps by the API's maxMipLevel
          // and clamps the result to the lower level-of-detail if necessary. Note some subtle definitions
          // before any of this code is modified. SST_LODMIN corresponds to the minimum data value
          // which actually reveals the highest level-of-detail (thus the addition). SST_LODMAX actually reveals the lowest
          // level of detail. Also note that we always use maxMipLevel from stage 0. The only reason
          // to date for this is that all of the WHQL DCT MAXMIPLEVEL tests pass. The stage 1 variable
          // has not been used to date which may be a future bug. Perhaps the tests are flawed but this
          // is the only way to pass the tests. The latest change actually came about with a bug in F-22
          // Lightning 3 (PRS 12046) where there was only one LOD per texture so these two apps are necessary for
          // testing against any future changes. I'm also unsure how automipmapping will affect results
          // whenever it is enabled. It has not been enabled. Do not define IGX_TURNOFFMIPCLAMP unless
          // you want to go back one version to code without a clamp.
#ifdef IGX_TURNOFFMIPCLAMP
		  // didn't do anything here for LODBIASPERCHIP
          SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLOD | (pRc->sst.tLODT0 + (pRc->textureStage[0].maxMipLevel << 2)));

          D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT0 + (pRc->textureStage[0].maxMipLevel << 2)));
#else
          if (! pRc->textureStage[0].maxMipLevel) //see if we even need to make an adjustment
		  {
			// start with current lod register value.
			dwLod = pRc->sst.tLOD | pRc->sst.tLODT0;
			// strip out the lod bias portion.
			dwLod &= ~(SST_LODBIAS);
			// plug in the new lod bias value for this chip.
			dwLod |= pRc->dwTLodBiasT0[i];

            SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, dwLod);

            D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", dwLod);
		  }
          else
		  {
            DWORD minlod; // minimum value corresponding to the highest level-of-detail (i.e. 0==256x256)
            DWORD maxlod; // maximum value corresponding to the lowest level-of-detail  (i.e. 8==1x1)

            // adjust the highest level of detail. maxMipLevel is actually a relative index where
            // zero causes no reduction in level-of-detail. The index is simply converted to a delta
            // by shifting over two bits to a fixed pt 4.2
            minlod = ((pRc->sst.tLODT0&SST_LODMIN)>>SST_LODMIN_SHIFT) +
                     (pRc->textureStage[0].maxMipLevel<<2);
            // get the lowest allowable level-of-detail
            maxlod = (pRc->sst.tLODT0&SST_LODMAX)>>SST_LODMAX_SHIFT;

            if (minlod > maxlod)
			{
              // clamp to maxlod
              minlod = maxlod;
			}

			// start with current lod register value.
			dwLod = pRc->sst.tLOD | (pRc->sst.tLODT0 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT);
			// strip out the lod bias portion.
			dwLod &= ~(SST_LODBIAS);
			// plug in the new lod bias value for this chip.
			dwLod |= pRc->dwTLodBiasT0[i];

            //replace minlod
            SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, dwLod);

            D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", dwLod);
		  }
#endif // IGX_TURNOFFMIPCLAMP

          // Flush the texture cache by writing any other base addr (so invert the base addr)
          SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
          SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, (0xffffffff ^ pRc->sst.baseAddr));

          SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
          SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, pRc->sst.baseAddr);

          D3DPRINT(D3DDBGLVL, "         texBaseAddr=%08lXh", pRc->sst.baseAddr);

          SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, textureMode, TMU2CHIP(TREX1)));
          SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT1);

          D3DPRINT(D3DDBGLVL, "  TREX1: textureMode=%08lXh", pRc->sst.textureMode | pRc->sst.textureModeT1);

#ifdef IGX_TURNOFFMIPCLAMP
		  // didn't do anything here for LODBIASPERCHIP
          pRc->sst.tLOD &= ~SST_LODMIN; // no need to repeat this redundant step
          SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, pRc->sst.tLOD | (pRc->sst.tLODT1 + (pRc->textureStage[0].maxMipLevel << 2)));

          D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT1 + (pRc->textureStage[0].maxMipLevel << 2)));
#else
          if (! pRc->textureStage[0].maxMipLevel) //see if we even need to make an adjustment
		  {
			// start with current lod register value.
			dwLod = pRc->sst.tLOD | pRc->sst.tLODT1;
			// strip out the lod bias portion.
			dwLod &= ~(SST_LODBIAS);
			// plug in the new lod bias value for this chip.
			dwLod |= pRc->dwTLodBiasT1[i];

            SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, dwLod);

            D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", dwLod);
		  }
          else
		  {
            DWORD minlod; // minimum value corresponding to the highest level-of-detail (i.e. 0==256x256)
            DWORD maxlod; // maximum value corresponding to the lowest level-of-detail  (i.e. 8==1x1)
        
            // adjust the highest level of detail. maxMipLevel is actually a relative index where
            // zero causes no reduction in level-of-detail. The index is simply converted to a delta
            // by shifting over two bits to a fixed pt 4.2
            minlod = ((pRc->sst.tLODT1&SST_LODMIN)>>SST_LODMIN_SHIFT) +
                     (pRc->textureStage[0].maxMipLevel<<2);
            // get the lowest allowable level-of-detail
            maxlod = (pRc->sst.tLODT1&SST_LODMAX)>>SST_LODMAX_SHIFT;

            if (minlod > maxlod)
			{
              // clamp to max lod
              minlod = maxlod;
			}

			// start with current lod register value.
			dwLod = pRc->sst.tLOD | (pRc->sst.tLODT1 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT);
			// strip out the lod bias portion.
			dwLod &= ~(SST_LODBIAS);
			// plug in the new lod bias value for this chip.
			dwLod |= pRc->dwTLodBiasT1[i];

            //replace minlod
            SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, dwLod);

            D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", dwLod);
		  }
#endif // IGX_TURNOFFMIPCLAMP
		} // For each chip

		// Reset to write to all chips.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
        SETPD( cmdFifo, ghw->chipMask, pRc->sst.chipMask);

	  } // if multitexturing, trilinear filtering and number chips > 1
	  else
	  {
#endif // LODBIASPERCHIP

      // partial fix for PC99 TextureSizes & PC99 MultiTexturing failures
      // flush the texture cache
      CMDFIFO_CHECKROOM(cmdFifo, 2 * ((PH4_SIZE + 2) + (PH4_SIZE + 1) + (PH4_SIZE + 1)));

      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, textureMode, TMU2CHIP(TREX0)));
      SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT0);

      D3DPRINT(D3DDBGLVL, "  TREX0: textureMode=%08lXh", pRc->sst.textureMode | pRc->sst.textureModeT0);

      pRc->sst.tLOD &= ~SST_LODMIN;

      // The following code adjusts the higher level-of-detail for mipmaps by the API's maxMipLevel
      // and clamps the result to the lower level-of-detail if necessary. Note some subtle definitions
      // before any of this code is modified. SST_LODMIN corresponds to the minimum data value
      // which actually reveals the highest level-of-detail (thus the addition). SST_LODMAX actually reveals the lowest
      // level of detail. Also note that we always use maxMipLevel from stage 0. The only reason
      // to date for this is that all of the WHQL DCT MAXMIPLEVEL tests pass. The stage 1 variable
      // has not been used to date which may be a future bug. Perhaps the tests are flawed but this
      // is the only way to pass the tests. The latest change actually came about with a bug in F-22
      // Lightning 3 (PRS 12046) where there was only one LOD per texture so these two apps are necessary for
      // testing against any future changes. I'm also unsure how automipmapping will affect results
      // whenever it is enabled. It has not been enabled. Do not define IGX_TURNOFFMIPCLAMP unless
      // you want to go back one version to code without a clamp.
#ifdef IGX_TURNOFFMIPCLAMP
      SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLOD | (pRc->sst.tLODT0 + (pRc->textureStage[0].maxMipLevel << 2)));

      D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT0 + (pRc->textureStage[0].maxMipLevel << 2)));
#else
      if (! pRc->textureStage[0].maxMipLevel) //see if we even need to make an adjustment
      {
        SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLOD | pRc->sst.tLODT0);

        D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | pRc->sst.tLODT0);
      }
      else
      {
        DWORD minlod; // minimum value corresponding to the highest level-of-detail (i.e. 0==256x256)
        DWORD maxlod; // maximum value corresponding to the lowest level-of-detail  (i.e. 8==1x1)

        // adjust the highest level of detail. maxMipLevel is actually a relative index where
        // zero causes no reduction in level-of-detail. The index is simply converted to a delta
        // by shifting over two bits to a fixed pt 4.2
        minlod = ((pRc->sst.tLODT0&SST_LODMIN)>>SST_LODMIN_SHIFT) +
                 (pRc->textureStage[0].maxMipLevel<<2);
        // get the lowest allowable level-of-detail
        maxlod = (pRc->sst.tLODT0&SST_LODMAX)>>SST_LODMAX_SHIFT;

        if (minlod > maxlod)
        {
          // clamp to maxlod
          minlod = maxlod;
        }

        //replace minlod
        SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLOD |
              (pRc->sst.tLODT0 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT));

        D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT0 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT));
      }
#endif

      // Flush the texture cache by writing any other base addr (so invert the base addr)
      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
      SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, (0xffffffff ^ pRc->sst.baseAddr));

      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
      SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, pRc->sst.baseAddr);

      D3DPRINT(D3DDBGLVL, "         texBaseAddr=%08lXh", pRc->sst.baseAddr);

      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, textureMode, TMU2CHIP(TREX1)));
      SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT1);

      D3DPRINT(D3DDBGLVL, "  TREX1: textureMode=%08lXh", pRc->sst.textureMode | pRc->sst.textureModeT1);

#ifdef IGX_TURNOFFMIPCLAMP
      pRc->sst.tLOD &= ~SST_LODMIN; // no need to repeat this redundant step
      SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, pRc->sst.tLOD | (pRc->sst.tLODT1 + (pRc->textureStage[0].maxMipLevel << 2)));

      D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT1 + (pRc->textureStage[0].maxMipLevel << 2)));
#else
      if (! pRc->textureStage[0].maxMipLevel) //see if we even need to make an adjustment
      {
        SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, pRc->sst.tLOD | pRc->sst.tLODT1);

        D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | pRc->sst.tLODT1);
      }
      else
      {
        DWORD minlod; // minimum value corresponding to the highest level-of-detail (i.e. 0==256x256)
        DWORD maxlod; // maximum value corresponding to the lowest level-of-detail  (i.e. 8==1x1)
        
        // adjust the highest level of detail. maxMipLevel is actually a relative index where
        // zero causes no reduction in level-of-detail. The index is simply converted to a delta
        // by shifting over two bits to a fixed pt 4.2
        minlod = ((pRc->sst.tLODT1&SST_LODMIN)>>SST_LODMIN_SHIFT) +
                 (pRc->textureStage[0].maxMipLevel<<2);
        // get the lowest allowable level-of-detail
        maxlod = (pRc->sst.tLODT1&SST_LODMAX)>>SST_LODMAX_SHIFT;

        if (minlod > maxlod)
        {
          // clamp to max lod
          minlod = maxlod;
        }
        
        //replace minlod
        SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, pRc->sst.tLOD |
              (pRc->sst.tLODT1 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT));

        D3DPRINT(D3DDBGLVL, "         tLOD=%08lXh", pRc->sst.tLOD | (pRc->sst.tLODT1 & ~(SST_LODMIN)) | (minlod<<SST_LODMIN_SHIFT));
      }
#endif

      // Flush the texture cache by writing any other base addr (so invert the base addr)
      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX1)));
      SETPD(cmdFifo, SST_TREX(ghw0,TREX1)->texBaseAddr, (0xffffffff ^ pRc->sst.baseAddr1));

      SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX1)));
      SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->texBaseAddr, pRc->sst.baseAddr1);

      D3DPRINT(D3DDBGLVL, "         texBaseAddr=%08lXh", pRc->sst.baseAddr1);
#ifdef LODBIASPERCHIP
	  } // !(if multitexturing, trilinear filtering and number chips > 1)
#endif

      // if palettized texture and it changed then redownload it.
      // why download now instead of when texture id changes - this is the conservative
      // approach and the palette can change for numerous reasons so lets make sure
      // that it is download before each mesh (can optimize later)

#if defined(WINNT) && (DX <= 6)
#pragma message(__FILELINE__ "How do we handle palettized textures?")
#else
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)) &&
          PALHNDL_INRANGE(pRc->texture) && PALETTEGBL(pRc->texture))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 | CHIP_TMU1 ),
                            PALETTEGBL(pRc->texture),
                            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);

        CMDFIFO_RELOAD( cmdFifo );
      }
#else
      if( (TXTRHNDL_INUSE(pRc->texture)) && (PALETTIZEDHANDLE(pRc->texture)) &&
         ((PALETTECHANGED & _D3(flags) ) || (_D3(prevContentStamp) != ((LPDDRAWI_DDRAWSURFACE_LCL) (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->dwContentsStamp)))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 | CHIP_TMU1 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);

        CMDFIFO_RELOAD( cmdFifo );
      }
#endif // DX >= 7
#endif // DX <= 6

      //----------------
      //
      // Chroma Keying
      //
      //----------------
      // The chroma key value is stored in the texture surface. This is the only attribute
      // that is stored in the surface and not the rendering context. If the source chroma
      // key value is set for the texture surface then use the color. SST-1 chroma key
      // tests after the bi-linear filter and this is not correct. D3D performs the chroma
      // test before the filter. Scott/Gary said they would fix it one day...
      if (   (pRc->colorKeyEnable) &&
             (((TXTRHNDL *) TXTRHNDL_PTR(pRc->texture))->dwFlags & DDRAWISURF_HASCKEYSRCBLT))
      {
        int r, g, b, xrgb;

        D3DPRINT(D3DDBGLVL, "setDX6State ColorKeyLow = %08lXh ColorKeyHigh = %08lX txtrFormat = %08lXh",
                 ((TXTRHNDL *) TXTRHNDL_PTR(pRc->texture))->dwColorSpaceLowValue,
                 ((TXTRHNDL *) TXTRHNDL_PTR(pRc->texture))->dwColorSpaceHighValue,
                 txtr->format);

        r=g=b=((TXTRHNDL *) TXTRHNDL_PTR(pRc->texture))->dwColorSpaceLowValue;

        // format of the chroma key value is the format of the texture
        // need to convert from texture format to xRGB-x888
        switch (txtr->format)
        {
          case (TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT):
            r = ( r >> 8  ) & 0x0F;
            r = _imgMSBReplicate( r, 4, 0 );
            g = ( g >> 4  ) & 0x0F;
            g = _imgMSBReplicate( g, 4, 0 );
            b = ( b >> 0  ) & 0x0F;
            b = _imgMSBReplicate( b, 4, 0 );
            break;
          case (TEXFMT_ARGB_1555 << SST_TFORMAT_SHIFT):
            r = ( r >> 10 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5  ) & 0x1F;
            g = _imgMSBReplicate( g, 3, 2);
            b =   b         & 0x1F;
            b = _imgMSBReplicate( b, 3, 2);
            break;
          default:
          case (TEXFMT_RGB_565 << SST_TFORMAT_SHIFT):
            r = ( r >> 11 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5 ) &  0x3F;
            g = _imgMSBReplicate( g, 2, 4);
            b = ( b >> 0 ) &  0x1F;
            b = _imgMSBReplicate( b, 3, 2 );
            break;
          case (TEXFMT_RGB_332 << SST_TFORMAT_SHIFT):
          case (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT):
            r = ( r >> 5 ) & 0x07;
            r = (r  << 5 ) | (r << 2) | (r >> 1) ;
            g = ( g >> 2 ) & 0x07;
            g = (g  << 5 ) | (g << 2) | (g >> 1) ;
            b =   b        & 0x03;
            b = (b << 6) | (b << 4) | (b << 2) | b ;
            break;
          case (TEXFMT_P8_RGB << SST_TFORMAT_SHIFT):
          case (TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT):
            {
#if defined(WINNT) && (DX <= 6)
#pragma message(__FILELINE__ "How do we handle palettized textures?")
#else
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              PALHNDL *pPalHndl = NULL;
              if (PALHNDL_INRANGE(pRc->texture) && PALETTEGBL(pRc->texture))
                pPalHndl = PALETTEGBL(pRc->texture);
              if (pPalHndl != NULL)
              {
                g = pPalHndl->ColorTable[r].peGreen;
                b = pPalHndl->ColorTable[r].peBlue;
                r = pPalHndl->ColorTable[r].peRed;

                // fix for PRS 11174 - Alpha Palette failures in dct 300
                if (DDRAWIPAL_ALPHA & pPalHndl->dwFlags)
                {
                  // palettized ARGB are 6bit values
                  // the hw replicates the 2 msb's into the 2 lsbs'
                  g = (g & ~0x03) | ((g >> 6) & 0x03);
                  b = (b & ~0x03) | ((b >> 6) & 0x03);
                  r = (r & ~0x03) | ((r >> 6) & 0x03);
                }
              }
#else
              LPDDRAWI_DDRAWSURFACE_LCL lcl = (LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl);
              if (lcl->lpDDPalette != NULL)
              {
                g = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peGreen;
                b = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peBlue;
                r = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peRed;
              }
#endif // DX >= 7
#endif // DX <= 6
            }
            break;
          case (TEXFMT_ALPHA_8 << SST_TFORMAT_SHIFT):
          case (TEXFMT_INTENSITY_8 << SST_TFORMAT_SHIFT):
            // just keep the luminance portion of the colorkey value in each of r, g & b
            r &= L8_LMASK;
            g &= L8_LMASK;
            b &= L8_LMASK;
            break;
          case (TEXFMT_ALPHA_INTENSITY_88 << SST_TFORMAT_SHIFT):
            // just keep the luminance portion of the colorkey value in each of r, g & b
            r &= LA16_LMASK;
            g &= LA16_LMASK;
            b &= LA16_LMASK;
            break;
          case (TEXFMT_ALPHA_INTENSITY_44 << SST_TFORMAT_SHIFT):
            r &= LA8_LMASK;
            r = (r << 4) | r;
            g &= LA8_LMASK;
            g = (g << 4) | g;
            b &= LA8_LMASK;
            b = (b << 4) | b;
            break;
          case (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT):
            r = (r & RGB8888_RMASK) >> 16;
            g = (g & RGB8888_GMASK) >>  8;
            b = (b & RGB8888_BMASK);
            break;
        }

        xrgb =  (r << 16) | (g << 8) | b;

        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

        // Only set the FBI. Don't write the TMU chromakey (its used for texturefactor).
        if (IS_NAPALM)
        {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, chromaKey, 0x1 ) );
        }
        else
        {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
        }

        SETPD( cmdFifo, ghw->chromaKey, xrgb );
        D3DPRINT(D3DDBGLVL, "  chromaKey=%08lXh", xrgb);

        _D3(last).chromaKey = xrgb;

        pRc->sst.fbzMode |= SST_ENCHROMAKEY;
      }
      else
      {
        pRc->sst.fbzMode &= ~SST_ENCHROMAKEY;
      }
    }
    else // If we are not texturing, disable chromakeying
    {
      pRc->sst.fbzMode &= ~SST_ENCHROMAKEY;

      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, 0, tLOD, 0xF) );
      SETPD( cmdFifo, ghw->tLOD, 0x820 );
      D3DPRINT(D3DDBGLVL, "  tLOD=%08lXh", 0x820);
      if (IS_NAPALM) {
          // we need to write texture mode for Napalm because Napalm sometimes uses the TCUs for getting iterated data to the CCU. -cws
          CMDFIFO_CHECKROOM( cmdFifo,  2 * (PH1_SIZE + 1) );

          SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, textureMode, 2 ) );
          SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->textureMode, pRc->sst.textureModeT0 );
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, textureMode, 4 ) );
          SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->textureMode, pRc->sst.textureModeT1 );
      }
    }
    // E N D   T E X T U R I N G   A N D   C H R O M A K E Y I N G

    // F O G
    if( pRc->useFog & FOG_STATE_CHANGED )
    {
      if( FOG_STATE_CHANGED & SC_FOGCOLOR )
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogColor, 0xF ) );
        SETPD( cmdFifo, ghw->fogColor, pRc->fogColor );
        D3DPRINT(D3DDBGLVL, "  fogColor=%08lXh", pRc->fogColor);
      }

      // Hardware table fog
      if( pRc->useFogTable & FOG_STATE_CHANGED )
      {
        CMDFIFO_SAVE( cmdFifo );
        createTableAndLoad(pRc);
        CMDFIFO_RELOAD( cmdFifo );
      }

      RESET_FOG_STATE;
    }

    CMDFIFO_SAVE( cmdFifo );
    HandleSLIAA(pRc);
    CMDFIFO_RELOAD( cmdFifo );

    // T E X T U R E   F A C T O R
    if( HW_STATE_CHANGED & SC_TEXTUREFACTOR )
    {
      if (IS_NAPALM)
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH4_SIZE + 2) * 2);

        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1, chromaKey, 6 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,6)->chromaKey, pRc->textureFactor );
        SETPD( cmdFifo, SST_CHIP(ghw,6)->chromaRange, pRc->textureFactor );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1, c0, 0 ) );
        SETPD( cmdFifo, ghw->c0, pRc->textureFactor );
        SETPD( cmdFifo, ghw->c1, pRc->textureFactor );
        D3DPRINT(D3DDBGLVL, "  textureFactor (c0 & c1)=%08lXh", pRc->textureFactor);
      }
      else
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, c0, 0 ) );
        SETPD( cmdFifo, ghw->c0, pRc->textureFactor );
        D3DPRINT(D3DDBGLVL, "  textureFactor (c0)=%08lXh", pRc->textureFactor);
      }
    }

    // Z   B I A S
    // If Z buffer is enabled then we need to send the current ZBIAS value. - AS
    if( HW_STATE_CHANGED & SC_ZBIAS )
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, zaColor, 0 ) );
      SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
      D3DPRINT(D3DDBGLVL, "  zaColor=%08lXh", pRc->sst.zaColor);
    }

    // Save current values.
    _D3(last).fbzColorPath  = pRc->sst.fbzColorPath;
    _D3(last).fogMode       = pRc->sst.fogMode;
    _D3(last).alphaMode     = pRc->sst.alphaMode;
    _D3(last).fbzMode       = pRc->sst.fbzMode;
    _D3(last).zaColor       = pRc->sst.zaColor;
#ifdef NEW_CLIP_FOR_GB
    _D3(last).clipLeftRight       = pRc->sst.clipLeftRight;
    _D3(last).clipBottomTop       = pRc->sst.clipBottomTop;
#endif

    if (IS_NAPALM)
    {
      _D3(last).combineModeFBI   = pRc->sst.combineModeFBI;
      _D3(last).combineModeT0    = pRc->sst.combineModeT0 ;
      _D3(last).combineModeT1    = pRc->sst.combineModeT1 ;
      _D3(last).renderMode       = pRc->sst.renderMode;
      _D3(last).stencilMode      = pRc->sst.stencilMode;
      _D3(last).stencilOp        = pRc->sst.stencilOp;
    }
  }
  else // HW_STATE_CHANGED
  /////////////////////////////////////////////////////////////////////////////////////
  // ! H W _ S T A T E _ C H A N G E D
  {
    if( TXTRHNDL_INRANGE(pRc->texture)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    // to fix potential access violation issues with NT Stress
    // verify we have a TXTRHNDL for this texture before dereferencing it
    //
    // this check protects all use of TXTRHNDL_PTR (both explict and use inside
    // other macros) withing this if block

        && TXTRHNDL_PTR(pRc->texture)
#endif
      )
    {
#if defined(WINNT) && (DX <= 6)
#pragma message(__FILELINE__ "How do we handle palettized textures?")
#else
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
     if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)) &&
         PALHNDL_INRANGE(pRc->texture) && PALETTEGBL(pRc->texture))
     {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 | CHIP_TMU1 ),
                            PALETTEGBL(pRc->texture),
                            TXTRDESC_FROM_HNDL(pRc->texture)->formatFlags);

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);

        CMDFIFO_RELOAD( cmdFifo );

        // Check to see if we are ChromaKeying.  If we are we need to re-calculate
        // the Chromakey color. -Ade
        if( ( (DWORD)pRc != _D3(lastContext) ) || (pRc->sst.fbzMode & SST_ENCHROMAKEY) )
        {
          PALHNDL *pPalHndl = PALETTEGBL(pRc->texture);
          DWORD r, g, b;

          r = ((TXTRHNDL *) TXTRHNDL_PTR(pRc->texture))->dwColorSpaceLowValue;

          g = pPalHndl->ColorTable[r].peGreen;
          b = pPalHndl->ColorTable[r].peBlue;
          r = pPalHndl->ColorTable[r].peRed;

          if (DDRAWIPAL_ALPHA & pPalHndl->dwFlags)
          {
            // palettized ARGB are 6bit values
            // the hw replicates the 2 msb's into the 2 lsbs'
            g = (g & ~0x03) | ((g >> 6) & 0x03);
            b = (b & ~0x03) | ((b >> 6) & 0x03);
            r = (r & ~0x03) | ((r >> 6) & 0x03);
          }

          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

          if (IS_NAPALM)
          {
            // Only set the FBI.  Don't write the TMU chromakey (its used for texturefactor).
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, chromaKey, 0x1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->chromaKey, ((r << 16) | (g << 8) | b) );
          }
          else
          {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
            SETPD( cmdFifo, ghw->chromaKey, ((r << 16) | (g << 8) | b) );
          }
        }
     }
#else // DX >= 7
     if( (TXTRHNDL_INUSE(pRc->texture)) && (PALETTIZEDHANDLE(pRc->texture)) &&
         ((PALETTECHANGED & _D3(flags) ) || (_D3(prevContentStamp) != ((LPDDRAWI_DDRAWSURFACE_LCL) (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->dwContentsStamp)))
     {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 | CHIP_TMU1 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
             TXTRDESC_FROM_HNDL(pRc->texture)->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);

        CMDFIFO_RELOAD( cmdFifo );

        // Check to see if we are ChromaKeying.
        // If we are we need to re-calculate the Chromakey color. -Ade
        if( pRc->sst.fbzMode & SST_ENCHROMAKEY)
        {
          LPDDRAWI_DDRAWSURFACE_LCL lcl = (LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl);
          DWORD r, g, b;

          r = ((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl))->ddckCKSrcBlt.dwColorSpaceLowValue;

          g = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peGreen;
          b = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peBlue;
          r = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peRed;

          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1) );

          if (IS_NAPALM)
          {
            // Only set the FBI.  Don't write the TMU chromakey (its used for texturefactor).
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, chromaKey, 0x1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->chromaKey, ((r << 16) | (g << 8) | b) );
          }
          else
          {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
            SETPD( cmdFifo, ghw->chromaKey, ((r << 16) | (g << 8) | b) );
          }
        }
     }
#endif // DX >= 7
#endif // DX <= 6
    }  // pRc->texture != 0

    // ( L A S T ) . C H A N G E D
    if( _D3(last).changed )
    {
      if (IS_NAPALM)
      {
          /* If going from 2ppc to 1ppc, we need to send 12 NOPs. */
        if (  (DWORD)pRc != _D3( lastContext ) || //Need this in case we are switching to SLI mode, so 2nd chip will get updated.
            (  _D3(NapalmIsIn2ppc) &&
              ( (!(pRc->sst.combineModeT0 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)) ||
                (!(pRc->sst.combineModeT1 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))   ) ) )
        {
           CMDFIFO_CHECKROOM( cmdFifo,  (PH1_SIZE + 12) );

		   // Send to TMU0 and TMU1 only.
           SETPH(cmdFifo, CMDFIFO_BUILD_PK1CHIP( 12, 0, nopCMD, 0x6 ) );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
           SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 ); SETPD(cmdFifo, SST_CHIP(ghw,0x6)->nopCMD, 0 );
        }

        // track 2ppc mode written to chips
        if (pRc->sst.combineModeT0 & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
        {
           _D3(NapalmIsIn2ppc) = 1;
        }
        else
        {
           _D3(NapalmIsIn2ppc) = 0;
        }

        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 6) + 9 + (PH4_SIZE + 3) );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, _D3(last).combineModeFBI);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, _D3(last).combineModeT0);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, _D3(last).combineModeT1);

        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2, renderMode, 0 ) );
        SETPD( cmdFifo, ghw->renderMode, _D3(last).renderMode );
        if (ulStencilModeLast & SST_STENCIL_ENABLE)
        {
          SETPD( cmdFifo, ghw->stencilMode, ulStencilModeLast );
          SETPD( cmdFifo, ghw->stencilOp, _D3(last).stencilOp );
        }
        else
        {
          SETPD( cmdFifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
          SETPD( cmdFifo, ghw->stencilOp, 0); // D3DSTENCILOP_KEEP
        }

        // Only set the FBI.  Don't write the TMU chromakey (its used for texturefactor).
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, chromaKey, 0x1 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->chromaKey, _D3(last).chromaKey );
      }
      else
      {
        CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 3) + 6 );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
        SETPD( cmdFifo, ghw->chromaKey, _D3(last).chromaKey );
      }

      D3DPRINT(D3DDBGLVL, "  chromaKey=%08lXh", _D3(last).chromaKey);

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xF ) );
      SETPD( cmdFifo, ghw->fbzColorPath, _D3(last).fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, _D3(last).fogMode );
      SETPD( cmdFifo, ghw->alphaMode, _D3(last).alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, _D3(last).fbzMode );
      D3DPRINT(D3DDBGLVL, "  fbzColorPath=%08lXh, fogMode=%08lXh, alphaMode=%08lXh, fbzMode=%08lXh",
               _D3(last).fbzColorPath, _D3(last).fogMode, _D3(last).alphaMode, _D3(last).fbzMode);

      // If Z buffer is enabled then we need to send the current
      // ZBIAS value. - AS
      if( pRc->zEnable )
      {
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, zaColor, 0xF ) );
        SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
        D3DPRINT(D3DDBGLVL, "  zaColor=8lXh", _D3(last).zaColor);
      }

      _D3(last).changed = FALSE;
    } // (last).changed
  } // !HW_STATE_CHANGED

  RESET_HW_STATE;

  CMDFIFO_EPILOG( cmdFifo );

#if defined(NULLDRIVER)
  } // ondrtHWSU
#endif

  return D3D_OK;
}

//-------------------------------------------------------------------

#endif
