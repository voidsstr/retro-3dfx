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
#include <stdio.h>
#include <stdarg.h>

#include "conform.h"
#include "vertex.h"

/*-----------------------------------------------------------------------------
@func add_data
@date 8/21/98
@arg conform_state *state - current conformance state
@arg void *buffer - pointer to buffer containing frame data
@arg FxU32 bufferlen - length of data in bytes
@return none
@imp
@key
@sect
@html
Used to save information associated with each frame such as 
vertex information, initial state, etc.
@end
-----------------------------------------------------------------------------*/
void add_data(conform_state *state, void *buffer, FxU32 bufferlen)
{
  void *data;
  listptr block;

  if((block = (listptr)malloc(sizeof(listentry))) == NULL) {
    // error - could not allocate memory
    log_perror(state,"Could not allocate data header");
    return;
  }

  if((data = malloc(bufferlen)) == NULL) {
    // error - could not allocate memory
    log_perror(state,"Could not allocate data block");
    return;
  }

  // copy data
  memcpy(data,buffer,bufferlen);

  // append new data block at tail of list
  block->next = NULL;
  block->data = data;
  block->size = bufferlen;

  if(state->datahead == NULL) {
    // first block
    state->datahead = block;
    state->datatail = block;
  } else {
    // append the block to the tail end of the list
    state->datatail->next = block;
    // and update the tail ptr
    state->datatail = block;
  }
}


/*-----------------------------------------------------------------------------
@func add_vertex_data
@date 8/25/98
@arg conform_state *state - current conformance state
@arg GrVertex *verts - the vertices that you want to put in the data file
@arg int num_verts - the number of vertices 
@return none
@imp
@key
@sect
@html
Used to save information associated with each frame such as 
vertex information, initial state, etc.
@end
-----------------------------------------------------------------------------*/

#define CHARS_PER_FLOAT   22 // 20 chars + comma, space
#define CHARS_PER_PADDING 40 // '{', '}', newlines, whatever
void add_vertex_data(conform_state *state, void *vertex_data, int num_verts)
{
  GrVertex *verts = (GrVertex *)vertex_data;
  char *buf;
  char tmu_buf[128];  // enough to hold an ASCII GrTmuVertex
  int num_bytes;

  // Figure out roughly how many bytes we'll need to hold the
  // ASCII version of a GrVertex. Allow a max of 20 characters
  // per float, plus a space and comma in between, and add some
  // padding for safety. Remember that the GrVertex might have
  // up to 8 GrTmuVertexs.

  num_bytes = sizeof(GrVertex) / sizeof(float) * CHARS_PER_FLOAT + 
              CHARS_PER_PADDING;

  if((buf = (char*) malloc(num_bytes)) == NULL) {
    // error - could not allocate memory
    log_perror(state,"Could not allocate vertex buffer");
    return;
  }

  /*
   * We should probably come up with the format for the data file.
   * This is a first pass that just dumps the raw vertex data with
   * a simple header stating that it's in GrVertex form and how many
   * of them follow.
   */

  sprintf(buf, "\nVertexData: GrVertex: %d:\n", num_verts);
  add_data(state, buf, strlen(buf));

  for (int i = 0; i < num_verts; i++) {
    sprintf(buf, "{%f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
            verts->x,
            verts->y,
            verts->ooz,
            verts->oow,
            verts->r,
            verts->g,
            verts->b,
            verts->a,
            verts->w);

    // now add the embedded tmu data
    for (int tmu = 0; tmu < GRVERTEX_NUM_TMU; tmu++) {
      sprintf(tmu_buf, " {%f, %f, %f}\n",
              verts->tmuvtx[tmu].sow,
              verts->tmuvtx[tmu].tow,
              verts->tmuvtx[tmu].oow);
      strcat(buf, tmu_buf);

      if ((int)strlen(buf) > num_bytes) {
        // ERROR! Overwriting Memory!  
	log_message(state,"Overwriting vertex memory");
	return;
      }
    }
    strcat(buf, "}\n");

    // send it to the log file.
    add_data(state, buf, strlen(buf));

    ++verts; // next GrVertex
  }

  free((void*)buf);
}


/*-----------------------------------------------------------------------------
@func add_command_data
@date 8/25/98
@arg conform_state *state - current conformance state 
@arg char *command_name - the name of the command to record
@arg int argc - the count of arguments following
@arg a variable number of arguments, treated as integers
@return none
@imp
@key
@sect
@html
Used to save a command associated with each frame.
Use as so: add_command_data(state, "grDrawPoint", 0);
@end
-----------------------------------------------------------------------------*/
#define CHARS_PER_FxU32 22 // 20, plus comma and and space
void add_command_data(conform_state *state,
                      char *command_name, int argc, ... /* args */)
{
  va_list argv;
  char *buf;           // temporary space to hold the output string.
  int num_bytes;       // size of temporary space required
  char int_buf[CHARS_PER_FxU32];

  num_bytes = (CHARS_PER_FxU32 * argc) + 
              strlen(command_name) + CHARS_PER_PADDING;

  if ((buf = (char*)malloc(num_bytes)) == NULL) {
    // error - could not allocate memory
    log_perror(state,"Could not allocate command buffer");
    return;
  }

  sprintf(buf, "%s", command_name);

  // Now get the variable list of args.
  va_start(argv, argc);

  // the rest of the args are options 
  for ( ; argc; argc-- ) {
    sprintf(int_buf, ", %d", va_arg(argv, FxU32));
    strcat(buf, int_buf);
  }
  strcat(buf, "\n");

  va_end(argv);

  // send it to the log file.
  add_data(state, buf, strlen(buf));

  free((void*)buf);
}
