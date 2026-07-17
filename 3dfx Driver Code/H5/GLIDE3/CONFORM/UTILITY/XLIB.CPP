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

#include "conform.h"

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
  void (FX_CALL *grSurfaceReleaseContext) ( GrContext_t ctx );
 
  grSurfaceReleaseContext = (void (FX_CALL *)(GrContext_t ctx))
    (void*)grGetProcAddress( "grSurfaceReleaseContextExt" );
 
  if ( !grSurfaceReleaseContext ) {
    log_message(state, "Couldn't get grSurfaceReleaseContext proc");
    return;
  }
 
  grSurfaceReleaseContext(ctx);
}

/*-----------------------------------------------------------------------------
@func create_window
@date 9/14/98
@arg conform_state *state - the conformance tests state
@return FxU32 - the handle/ID of the window created
@imp
@key
@sect
@html
Create an XLib window and return a handle to it.
@end
-----------------------------------------------------------------------------*/
FxU32 create_window(conform_state *state)
{
  return 1;
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
Destroy the XLib window specified in the conformance state
@end
--------------------------------------------------------------------------*/
void destroy_window(conform_state *state)
{
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
  return 0;
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
#ifdef notdef
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
#endif 
}
