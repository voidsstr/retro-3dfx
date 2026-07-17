#ifndef __ATDEMOP_H__
#define __ATDEMOP_H__

#ifdef _WIN32_
#include <windows.h>
#include <windowsx.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include "atdemo.h"

#ifdef __DOS32__
#define HWND unsigned long
#define LRESULT int
#define CALLBACK 
#define UINT unsigned int
#define LONG long
#endif

/* initial size of the display surface */

#define SCREEN_X  640
#define SCREEN_Y  480

#define WIN_X     320
#define WIN_Y     240

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  INTERNAL FUNCTION PROTOTYPES
 */

void _atdInitGlobals(void);

#if macintosh
int main(void);
#else
void _atdParseCmdLine( int argc, char **argv );
#endif

#if __WIN32__
#define     HINST(x)    (HINSTANCE)GetWindowLong(x, GWL_HINSTANCE)
#define     PAPA( x)    GetParent( x)
#define     MENU( x)    GetMenu( x)
#define     CTRL( x, y) GetDlgItem( x, y)
#define     SYS( x)     GetSystemMetrics( x)
LRESULT CALLBACK atWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
BOOL WINAPI AboutProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void atdWinHelp(void);
#endif

void MessageFunc(void);

void togglePlayback(void);
void toggleRecord(void);
void togglePerformance(FxBool full);

FxBool atdWinMain(void);

#ifdef __cplusplus
};
#endif

#endif
