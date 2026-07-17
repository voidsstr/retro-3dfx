#ifndef __HEADER_H__
#define __HEADER_H__

   //h3 csim stuff
#include "3dfx.h"
#include "h3.h"

#ifndef _WIN32
   #define NOUSER
   #define NOGDI
   //#undef NOGDI
   #define NOGDIOBJ 
   //#undef NOGDIOBJ 
   #define NOGDICAPMASKS
   #include <windows.h>

   #define NOPTRC

   #define PTTYPE POINT
   #include <gdidefs.inc>
   #include <dibeng.inc>
   typedef struct tagPALETTEENTRY FAR* LPPALETTEENTRY;
   typedef struct tagRGNDATA      FAR* LPRGNDATA;
   #include <ddrawi.h>
#endif

#include "shared.h"
#include "h3g.h"
#ifndef IS_32
#include "fifomgr.h"
#endif // #ifndef IS_32

// no need for render code in 16 bit yet.
// there is now....
#ifdef IS_16
#include "cmddefs.h"
#endif

#endif       // __HEADER_H__
