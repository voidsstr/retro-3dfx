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
*/

#ifndef __WIN32__
# error "This file should only be compiled on WIN32"
#endif // NOT __WIN32__

#include "conform.h"
#include <windows.h>

/*-----------------------------------------------------------------------------
@func process_window_events
@date 9/14/98
@arg conform_state *state - the conformance tests state
@return 
@imp
@key
@sect
@html
Process window system events that are on the event Q. Return when
the Q is empty.
@end
-----------------------------------------------------------------------------*/
void process_window_events(conform_state *state)
{
  MSG msg;

  // Process whatever events are on the event Q
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

}

/*-----------------------------------------------------------------------------
@func wait_for_keypress
@date 9/14/98
@arg conform_state *state - the conformance tests state
@return 
@imp
@key
@sect
@html
Process window system events until a keypress is encountered. This
routine will NOT RETURN until a keypress is encountered.
@end
-----------------------------------------------------------------------------*/
void wait_for_keypress(conform_state *state)
{
  MSG msg;
  int status  = FALSE;
  char orig_title[128];
  char new_title[200];

  // Get the original caption.
  GetWindowText((HWND)state->window_handle, orig_title, sizeof(orig_title));

  // Set the new 'PAUSED' title so the user knows we're paused.
  sprintf(new_title, "%s - PAUSED - hit a key", orig_title);
  SetWindowText((HWND)state->window_handle, new_title);

  while (status = GetMessage(&msg, (HWND)state->window_handle, 0, 0)) {
    if (status == -1) {
      log_perror(state, "Error while getting events.");
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // If we got our keypress, return
    if (msg.message == WM_CHAR) {
      break;
    }
  }

  // Restore the original title.
  SetWindowText((HWND)state->window_handle, orig_title);
                
}

/*-----------------------------------------------------------------------------
@func event_handler
@date 9/14/98
@arg HWND hWnd - our window handle
@arg UINT message - the event ID that we've been called to handle
@arg WPARAM wparam - parameters to the event
@arg LPARAM lparam - parameters to the event
@return - 
@imp
@key
@sect
@html
Given a windows event/message, handle it. For the moment, we just
pass it on to the default event handler.
@end
-----------------------------------------------------------------------------*/
LRESULT CALLBACK
event_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CREATE:
    case WM_PAINT:
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_MOVE:
    case WM_DISPLAYCHANGE:
    case WM_SIZE:
      break;
    case WM_CHAR:
      switch (wParam) {
        case '\033': // ESC
          // NOTE: Exits without shutting down Glide!
          exit(100);
          break;
      }
      break;
    default:
      break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
  
} 

typedef FxU32 GrSurfaceContextType_t;
#define GR_SURFACECONTEXT_WINDOWED      0
#define GR_SURFACECONTEXT_FULLSCREEN    1

/*-----------------------------------------------------------------------------
@func create_window
@date 9/14/98
@arg conform_state *state - the conformance tests state
@return FxU32 - the handle/ID of the window created
@imp
@key
@sect
@html
Create a WIN32 window and return a handle to it.
@end
-----------------------------------------------------------------------------*/
FxU32 create_window(conform_state *state) 
{
  WNDCLASS    wc;
  BOOL        rc;
  HWND        hWnd;
  HANDLE      hInstance = GetModuleHandle(NULL);

  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = event_handler;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (struct HINSTANCE__ *) hInstance;
  wc.hIcon = LoadIcon( NULL, IDI_APPLICATION);    /* generic icon */
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (struct HBRUSH__ *)GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName =  NULL;
  wc.lpszClassName = "WinGlideClass";
  rc = RegisterClass( &wc );

  if( !rc ) {
    return 0;
  }
  
  hWnd=
    CreateWindowEx(
                   WS_EX_APPWINDOW,
                   "WinGlideClass",
                   state->test_name,
                   WS_OVERLAPPED |     
                   WS_CAPTION  |     
                   WS_THICKFRAME | 
                   WS_MAXIMIZEBOX | 
                   WS_MINIMIZEBOX | 
                   WS_VISIBLE | /* so we don't have to call ShowWindow */
                   WS_POPUP |   /* non-app window */
                   WS_SYSMENU,  /* so we get an icon in the tray */
                   CW_USEDEFAULT, 
                   CW_USEDEFAULT,
                   state->screen_width,
                   state->screen_height,
                   NULL,
                   NULL,
                   (HINSTANCE )hInstance,
                   NULL
                   );
  
  if(hWnd) {
    //SetCursor(NULL);
    ShowWindow(hWnd, SW_NORMAL);
    UpdateWindow(hWnd);
    SetFocus(hWnd);
  }

  return (FxU32) hWnd;
}

/*--------------------------------------------------------------------------
@func destroy_window
@date 9/16/98
@arg conform_state *state - the conformance tests state
@return none
@imp
@key
@sect
@html
Destroy a the WIN32 window specified in the conformance state
@end
--------------------------------------------------------------------------*/
void destroy_window(conform_state *state)
{
  if (state->window_handle) {
    SendMessage((HWND)state->window_handle,WM_CLOSE,0,0L);
  }
}

/*--------------------------------------------------------------------------
@func surface_create_context
@date 1/12/99
@arg conform_state *state - the conformance tests state
@return a glide context or 0 if unsuccessful.
@imp
@key
@sect
@html
Create a glide context using the surface extensions. If they're not
available, return 0.
@end
--------------------------------------------------------------------------*/
GrContext_t  surface_create_context(conform_state *state)
{
  GrContext_t grCtx = 0;

  // declare the extension function ptrs
  GrContext_t (__stdcall *grSurfaceCreateContext) (GrSurfaceContextType_t type);

  // check to see if the surface extentions are available.
  const char *extensions = grGetString( GR_EXTENSION );
  if ( !strstr(extensions, " SURFACE " ) ) {
    log_message(state, "Surface extentions not available.");
    return 0;
  }

  // load 'er up!
  grSurfaceCreateContext = 
    (GrContext_t (__stdcall *)(GrSurfaceContextType_t type))
      grGetProcAddress( "grSurfaceCreateContextExt" );
 
  // make sure we got 'er
  if ( !grSurfaceCreateContext ) {
    log_message(state, "Couldn't get grSurfaceCreateContext proc");
    return 0;
  }

  // Create a new glide state vector 
  grCtx = grSurfaceCreateContext(GR_SURFACECONTEXT_WINDOWED);
  if (!grCtx) {
    log_message(state, "grSurfaceCreateContext(_WINDOWED) failed");
  } else {
    grSelectContext(grCtx);
  }

  return grCtx; 
}


/*--------------------------------------------------------------------------
@func surface_release_context
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg GrContext_t ctx - the context to release
@imp
@key
@sect
@html
Release a previously allocated glide context.
@end
--------------------------------------------------------------------------*/
void surface_release_context(conform_state *state, GrContext_t ctx)
{
  void (__stdcall *grSurfaceReleaseContext) ( GrContext_t ctx );

  grSurfaceReleaseContext = (void (__stdcall *)(GrContext_t ctx))
    (void*)grGetProcAddress( "grSurfaceReleaseContextExt" );
  
  if ( !grSurfaceReleaseContext ) {
    log_message(state, "Couldn't get grSurfaceReleaseContext proc");
    return;
  }

  grSurfaceReleaseContext(ctx);
}

