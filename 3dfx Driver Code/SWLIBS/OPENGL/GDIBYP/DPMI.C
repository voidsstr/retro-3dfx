//: dpmi.c
//: Copyright (C) alt.drivers inc. 1997
//: Glenn Nissen

#include "altgdi.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Macros                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

WORD POPAX( VOID );
#pragma aux POPAX value =                   \
    "pop    ax";

WORD INT31_ALLOCATELDT( WORD, WORD );
#pragma aux INT31_ALLOCATELDT =             \
    "int    31h"                            \
    "push   ax"                             \
    "xor    ax, ax"                         \
    "rol    ax, 1"                          \
    parm   [ax][cx]                         \
    modify [ax bx cx dx si di];

WORD INT31_SETSEGMENTBASE( WORD, WORD, WORD, WORD );
#pragma aux INT31_SETSEGMENTBASE =          \
    "int    31h"                            \
    "xor    ax, ax"                         \
    "rol    ax, 1"                          \
    parm   [ax][bx][cx][dx]                 \
    modify [ax bx cx dx si di];

WORD INT31_SETSEGMENTLIMIT( WORD, WORD, WORD, WORD );
#pragma aux INT31_SETSEGMENTLIMIT =         \
    "int    31h"                            \
    "xor    ax, ax"                         \
    "rol    ax, 1"                          \
    parm   [ax][bx][cx][dx]                 \
    modify [ax bx cx dx si di];

VOID INT31_FREELDT( WORD, WORD );
#pragma aux INT31_FREELDT =                 \
    "int    31h"                            \
    parm   [ax][bx]                         \
    modify [ax bx cx dx si di];

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  SelectorAlloc                                                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

WORD SelectorAlloc
(
    WORD wOldSel
)
{
    WORD wNewSel;
    WORD wFailed;
    WORD wAX;
    WORD wBX;
    WORD wCX;
    WORD wDX;

    DWORD dwBase;
    DWORD dwLimit;

    dwBase  = GetSelectorBase( wOldSel );
    dwLimit = GetSelectorLimit( wOldSel );

    wAX = 0x0000;
    wCX = 0x0001;
    wFailed = INT31_ALLOCATELDT( wAX, wCX );
    wAX = POPAX();
    wNewSel = wAX;

    if( wFailed )
    {
        return 0;
    }

    wAX = 0x0007;
    wBX = wNewSel;
    wCX = HIWORD( dwBase );
    wDX = LOWORD( dwBase );
    wFailed = INT31_SETSEGMENTBASE( wAX, wBX, wCX, wDX );

    if( wFailed )
    {
        SelectorFree( wNewSel );
        return 0;
    }

    wAX = 0x0008;
    wBX = wNewSel;
    wCX = HIWORD( dwLimit );
    wDX = LOWORD( dwLimit );
    wFailed = INT31_SETSEGMENTLIMIT( wAX, wBX, wCX, wDX );

    if( wFailed )
    {
        SelectorFree( wNewSel );
        return 0;
    }

    return wNewSel;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  SelectorFree                                                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

VOID SelectorFree
(
    WORD wSel
)
{
    WORD wAX;
    WORD wBX;

    wAX = 0x0001;
    wBX = wSel;
    INT31_FREELDT( wAX, wBX );
}

