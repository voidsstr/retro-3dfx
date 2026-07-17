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
#include "vmglide.h" 

/*-----------------------------------------------------------------------------
@func init_platform_fucntion_ptrs
@date 04/02/99
@arg conform_state *state - the framework's state
@return none
@html
Load platform-specific function pointers. At the momemt, we look for
3 CSIM functions. If these aren't available (ie. we're running on CVG),
they'll be set to NULL. The routines that try to use them should
notice this and act accordingly.
@end
-----------------------------------------------------------------------------*/
void init_platform_function_ptrs(conform_state *state)
{
  // These might fail, in which case they'll be set to NULL
  state->pfp.pause      = vmgrGetProcAddress("_gdPause@0");
  state->pfp.lfb_read   = vmgrGetProcAddress("_gdLfbRead@12");
  state->pfp.lfb_write  = vmgrGetProcAddress("_gdLfbWrite@12");
  state->pfp.get_pixels = vmgrGetProcAddress("_gdGetPixels@24");
}

/*-----------------------------------------------------------------------------
@func pause
@date 9/04/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Wait for user input so frame can be seen
@end
-----------------------------------------------------------------------------*/
void pause(conform_state *state)
{
  void (FX_CALL *gdPause)();

  if(state->interactive) {
    if (state->pfp.pause) {
      gdPause = (void (FX_CALL *)()) state->pfp.pause;
      gdPause();
    } else {
      wait_for_keypress(state);
    }
  }
}

/*-----------------------------------------------------------------------------
@func lfb_read_data
@date 9/04/98
@arg conform_state *state - current conformance state
@arg FxU8 *memptr - pointer to system memory
@arg FxU8 *lfbptr - pointer to LFB memory
@arg int num_bytes - number of bytes to copy
@return none
@imp
@key
@sect
@html
Copy data from LFB memory to system memory
@end
-----------------------------------------------------------------------------*/
void lfb_read_data(conform_state *state, FxU8 *memptr, FxU8 *lfbptr, int num_bytes)
{
  void (FX_CALL *gdLfbRead)(FxU8 *memptr, FxU8 *lfbptr, int num_bytes);

  if (state->pfp.lfb_read) {
    gdLfbRead = (void (FX_CALL *)(FxU8*, FxU8*, int)) state->pfp.lfb_read;
    gdLfbRead(memptr, lfbptr, num_bytes);
  } else {
    memcpy(memptr, lfbptr, num_bytes);
  }
}

/*-----------------------------------------------------------------------------
@func lfb_write_data
@date 9/04/98
@arg conform_state *state - current conformance state
@arg FxU8 *lfbptr - pointer to LFB memory
@arg FxU8 *memptr - pointer to system memory
@arg int num_bytes - number of bytes to copy
@return none
@imp
@key
@sect
@html
Copy data from system memory to LFB memory
@end
-----------------------------------------------------------------------------*/
void lfb_write_data(conform_state *state, FxU8 *lfbptr, FxU8 *memptr, int num_bytes)
{
  void (FX_CALL *gdLfbWrite)(FxU8 *memptr, FxU8 *lfbptr, int num_bytes);

  if (state->pfp.lfb_read) {
    gdLfbWrite = (void (FX_CALL *)(FxU8*, FxU8*, int)) state->pfp.lfb_write;
    gdLfbWrite(lfbptr, memptr, num_bytes);
  } else {
    memcpy(lfbptr, memptr, num_bytes);
  }
}
