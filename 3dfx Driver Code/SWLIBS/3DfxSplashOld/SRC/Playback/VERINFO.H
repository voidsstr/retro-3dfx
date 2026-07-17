/*
 *  verinfo.h - header file to define the build version
 *
 */

#define OFFICIAL                1
#undef FINAL
#define FINAL                   0

#define MANVERSION              1
#define MANREVISION             00

#ifdef RC_INVOKED
#define VERSIONPRODUCTNAME      "3Dfx Splash Screen\0"
#define VERSIONCOPYRIGHT        "Copyright \251 3Dfx Interactive, Inc. 1998\0"
#endif

#define RELEASE_NUMBER          0000
#define BUILD_NUMBER            0000

#define VERSIONSTR              "1.00.00.0000"

/***********************************************************
 * PLEASE DO NOT CHANGE THIS!!!
 *   The following comment is for SAPPHIRE (the build tool).
 *----------------------------------------------------------
 *  SAPPHIRE_TIME_STAMP Thu-19980820-22:07:26
 ***********************************************************/

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
