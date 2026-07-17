/*
 *  verinfo.h - header file to define the build version
 *
 */

#define OFFICIAL                1
#undef FINAL
#define FINAL                   0

#define MANVERSION              4
#define MANREVISION             3

#ifdef RC_INVOKED
#define VERSIONPRODUCTNAME      "Voodoo Graphics\256 Memory mapper for Windows\256  NT\0"
#define VERSIONCOPYRIGHT        "Copyright \251 3Dfx Interactive, Inc. 1996-1998\0"
#endif

#define BUILD_NUMBER            1098
#define VERSIONSTR              "2.1.2\0"

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
