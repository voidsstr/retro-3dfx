
#define MANVERSION              4
#define MANREVISION             1

#define VERSIONSTR "4.1\0"

#if defined(CVG) || defined(VOODOO2)
#   define HWSTR   " Voodoo(tm)\0"
#   ifdef NT_BUILD
#     define PRODNAME "Glide(tm) for Voodoo\251 and Windows\256 NT\0"
#   else
#     define PRODNAME "Glide(tm) for Voodoo\251 and Windows\256 95/98\0"
#   endif /* NT_BUILD */
#elif defined(H3)
#   define HWSTR   " Banshee(tm)\0"
#   ifdef NT_BUILD
#     define PRODNAME "Glide(tm) for Banshee\251 and Windows\256 NT\0"
#   else
#     define PRODNAME "Glide(tm) for Banshee\251 and Windows\256 95/98\0"
#   endif /* NT_BUILD */
#else
#   define PRODNAME "Something really, really important\0"
#   define HWSTR    " Some Hoopti Chip(tm)\0"
#endif
