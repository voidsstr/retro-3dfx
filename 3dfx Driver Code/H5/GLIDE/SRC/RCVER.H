
#define MANVERSION              2
#define MANREVISION             60

#ifdef __WIN32__
#define BUILD_NUMBER            0658
#endif

#define VERSIONSTR "2.61.00.0658"

#if defined(__WIN32__)
#define _TRADEMARK_  "(tm)"
#define _REGISTERED_ "\256"
#define _COPYRIGHT_  "\251"
#elif defined(macintosh)
#define _TRADEMARK_  "\252"
#define _REGISTERED_ "\250"
#define _COPYRIGHT_  "\251"
#else
#define _TRADEMARK_ "(tm)"
#define _REGISTERED_ "(R)"
#define _COPYRIGHT_  "(C)"
#endif

#ifdef SST1
#   define HWSTR   "Voodoo Graphics" _REGISTERED_ ""
#   ifdef NT_BUILD
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo Graphics" _REGISTERED_ " & Windows" _REGISTERED_ " NT"
#   else
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo Graphics" _COPYRIGHT_ " & Windows" _REGISTERED_ " 95/98"
#   endif /* NT_BUILD */
#elif defined(SST96)
#   define HWSTR   " Voodoo Rush" _REGISTERED_ ""
#   ifdef NT_BUILD
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo Rush" _COPYRIGHT_ " & Windows" _REGISTERED_ " NT"
#   else
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo Rush" _COPYRIGHT_ " & Windows" _REGISTERED_ " 95/98"
#   endif /* NT_BUILD */
#elif defined(CVG) || defined(VOODOO2)
#   define HWSTR   " Voodoo^2" _REGISTERED_ ""
#   ifdef NT_BUILD
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo^2" _COPYRIGHT_ " & Windows" _REGISTERED_ " NT"
#   else
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo^2" _COPYRIGHT_ " & Windows" _REGISTERED_ " 95/98"
#   endif /* NT_BUILD */
#elif defined(H3)
#   define HWSTR   " Voodoo Banshee" _REGISTERED_ ", Voodoo3" _REGISTERED_ ", & Velocity" _TRADEMARK_ " 100/200"
#     define PRODNAME "Glide" _REGISTERED_ " for Voodoo Banshee" _REGISTERED_ ", Voodoo3" _REGISTERED_ ", & Velocity" _TRADEMARK_ " 100/200; Windows 95" _REGISTERED_ ", Windows 98" _REGISTERED_ ", & Windows NT" _REGISTERED_ ""
#else
#   define HWSTR   "Some Hoopti Chip" _TRADEMARK_ ""
#endif
