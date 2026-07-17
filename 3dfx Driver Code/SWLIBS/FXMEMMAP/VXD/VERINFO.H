/* 
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
*/

/*
 *  verinfo.h - header file to define the build version
 *
 */

#define OFFICIAL                1
#undef FINAL
#define FINAL                   1

#define MANVERSION              4
#define MANREVISION             11

#ifdef RC_INVOKED
#define VERSIONPRODUCTNAME      "Voodoo Graphics\256 DirectX for Windows\256  95\0"
#define VERSIONCOPYRIGHT        "Copyright \251 3Dfx Interactive, Inc. 1997\0"
#endif

#define BUILD_NUMBER            15
#define VERSIONSTR              "4.11.01.0015\0"


/***************************************************************************
 *  DO NOT TOUCH BELOW THIS LINE                                           *
 ***************************************************************************/

#ifdef RC_INVOKED
#define VERSIONCOMPANYNAME      "3Dfx Interactive, Inc.\0"

/*
 *  Version flags 
 */

#undef VER_PRIVATEBUILD
#ifndef OFFICIAL
#define VER_PRIVATEBUILD        VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD        0
#endif

#undef VER_PRERELEASE
#ifndef FINAL
#define VER_PRERELEASE          VS_FF_PRERELEASE
#else
#define VER_PRERELEASE          0
#endif

#undef VER_DEBUG
#ifdef DEBUG
#define VER_DEBUG               VS_FF_DEBUG    
#elif RDEBUG
#define VER_DEBUG               VS_FF_DEBUG    
#else
#define VER_DEBUG               0
#endif

#define VERSIONFLAGS            (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)
#define VERSIONFILEFLAGSMASK    0x0030003FL
#endif
