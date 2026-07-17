//: gdientry.c
//: Copyright (C) alt.drivers inc. 1997
//: Glenn Nissen

#include "altgdi.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NewGdiBitBlt                                                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define OPENGL_CMD      4352        /* for OpenGL ExtEscape */
#define OPENGL_GETINFO  4353        /* for OpenGL ExtEscape */
#define MAX_PATH        260
typedef struct {
  unsigned long ulVersion;
  unsigned long ulDriverVersion;
  char  awch[MAX_PATH+1];
} OglOutInfo;

extern BOOL done;

int FAR PASCAL __loadds NewEscape( HDC    dc, 
                                   int    nEscape, 
                                   int    cbInput, 
                                   LPCSTR lpvInData, 
                                   LPVOID lpvOutData ) {
    int rv;
    FARENTRY_GDIESCAPE lpEscape;
    static int bypass = 0;

    switch ( nEscape ) {
    case OPENGL_GETINFO:
        {
            unsigned long FAR *indata = (void FAR*)lpvInData;
            switch( *indata ) {
            case 0: // OPENGL_GETINFO_DRVNAME
                if ( bypass ) {
                    OglOutInfo FAR *info = lpvOutData;
                    info->ulVersion       = 2;
                    info->ulDriverVersion = 1;
                    info->awch[0]         = '3';
                    info->awch[1]         = 'D';
                    info->awch[2]         = 'f';
                    info->awch[3]         = 'x';
                    info->awch[4]         = 0;
                    rv = 1;
                } else {
                    lpEscape = (FARENTRY_GDIESCAPE) appInfo.piEscape.lpTargetFunc;
                    UNPATCHFUNC( appInfo.piEscape );
                    rv = lpEscape(dc,nEscape,cbInput,lpvInData,lpvOutData);
                    PATCHFUNC( appInfo.piEscape );
                }
                break;
            case 0xbeef: // Enable Bypass
                bypass = 1;
                break;
            case 0xfade: // Disable Bypass
                bypass = 0;
                break;
            }

        }
        break;
    case QUERYESCSUPPORT:
        if ( bypass ) {
            switch (*((UINT FAR *)lpvInData)){
            case QUERYESCSUPPORT:
            case OPENGL_GETINFO:
                return TRUE;
            default:
                lpEscape = (FARENTRY_GDIESCAPE) appInfo.piEscape.lpTargetFunc;
                UNPATCHFUNC( appInfo.piEscape );
                rv = lpEscape(dc,nEscape,cbInput,lpvInData,lpvOutData);
                PATCHFUNC( appInfo.piEscape );
                break;
            }
        } else {
            lpEscape = (FARENTRY_GDIESCAPE) appInfo.piEscape.lpTargetFunc;
            UNPATCHFUNC( appInfo.piEscape );
            rv = lpEscape(dc,nEscape,cbInput,lpvInData,lpvOutData);
            PATCHFUNC( appInfo.piEscape );
        }
        break;
    default:
        lpEscape = (FARENTRY_GDIESCAPE) appInfo.piEscape.lpTargetFunc;
        UNPATCHFUNC( appInfo.piEscape );
        rv = lpEscape(dc,nEscape,cbInput,lpvInData,lpvOutData);
        PATCHFUNC( appInfo.piEscape );
        break;
    }
    return rv;
}


BOOL FAR PASCAL __loadds NewGdiBitBlt
(
    HDC   hdcDest,
    int   nxDest,
    int   nyDest,
    int   nWidth,
    int   nHeight,
    HDC   hdcSrc,
    int   nXSrc,
    int   nYSrc,
    DWORD dwRop
)
{
    FARENTRY_GDIBITBLT lpBitBlt;
    BOOL bResult;
    
    lpBitBlt = (FARENTRY_GDIBITBLT) appInfo.piGdiBitBlt.lpTargetFunc;
    UNPATCHFUNC( appInfo.piGdiBitBlt );
    bResult = lpBitBlt( hdcDest, nxDest, nyDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop );
    PATCHFUNC( appInfo.piGdiBitBlt );

    return bResult;
}

