/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
*/

/*
**  filebuff.c
**
**  Simple file buffering for GlideTrap.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glide.h>
#include <filebuff.h>

FILEBUFF *fileBuffOpen(char *filename, char *mode)
{
  FILEBUFF *new_fb;
  FILE *fp;

  if (!(fp=fopen(filename, mode)))
    return NULL;

  new_fb = (FILEBUFF*)malloc(sizeof(FILEBUFF));
  new_fb->fp            = fp;
  new_fb->write_offset  = 0;
  new_fb->read_offset   = FILEBUFF_SIZE;
  new_fb->read_buf_size = 0;
  new_fb->always_flush =  getenv("TRAP_ALWAYS_FLUSH") ? FXTRUE : FXFALSE;

  if (strstr(mode, "r")) {
    // fill the initial read buffer
    new_fb->read_buf_size = fread(new_fb->buf, 1, FILEBUFF_SIZE, new_fb->fp);
    new_fb->read_offset = 0;
  }

  return new_fb;
}

/*
 * write out any accumulated data
 */
void fileBuffFlush(FILEBUFF *fb)
{
  if (!fb || !fb->fp)
    return;
  if (fb->write_offset) {
    fwrite(fb->buf, fb->write_offset, 1, fb->fp);
    fflush(fb->fp);
    fb->write_offset = 0;
  }
}

/*
 * read data from the file as needed, and hand back data from it
 */
void fileBuffReadFxU8Array(FILEBUFF *fb, FxU8 *data, int size)
{
  if (!fb || !fb->fp) {
    memset(data, 0, size);
    return;
  }
  if (fb->read_offset + size < fb->read_buf_size) {

    // then we can just blindly go ahead and read all the data

    while (size-- > 0)
      *data++ = fb->buf[fb->read_offset++];

  } else {

    // then we will need to write out the buffer in the middle of the data

    while (size-- > 0) {

      if (fb->read_offset >= fb->read_buf_size) {
        // read in a new block of data
        fb->read_buf_size = fread(fb->buf, 1, FILEBUFF_SIZE, fb->fp);
        fb->read_offset = 0;
      }

      *data++ = (fb->read_buf_size > 0) ? fb->buf[fb->read_offset++] : 0;
    }
  }
}

FxU32 fileBuffReadFxU32(FILEBUFF *fb)
{
  FxU32 ret_val;
  if (!fb || !fb->fp)
    return 0;
  fileBuffReadFxU8Array(fb, (char*)&ret_val, 4);
#ifndef __WIN32__
  {  // byte swap
    char *p=(char*)&ret_val;
    char b0 = p[0],
         b1 = p[1],
         b2 = p[2],
         b3 = p[3];
    p[0] = b3;
    p[1] = b2;
    p[2] = b1;
    p[3] = b0;
  }
#endif
  return ret_val;
}

float fileBuffReadFloat(FILEBUFF *fb)
{
  FxU32 data = fileBuffReadFxU32(fb);
  return *(float*)&data;
}

FxU16 fileBuffReadFxU16(FILEBUFF *fb)
{
  FxU16 ret_val;
  if (!fb || !fb->fp)
    return 0;
  fileBuffReadFxU8Array(fb, (char*)&ret_val, 2);
#ifndef __WIN32__
  {  // byte swap
    char *p=(char*)&ret_val;
    char b0 = p[0],
         b1 = p[1];
    p[0] = b1;
    p[1] = b0;
  }
#endif
  return ret_val;
}

FxU8 fileBuffReadFxU8(FILEBUFF *fb)
{
  if (!fb || !fb->fp)
    return 0;
  if (fb->read_offset >= fb->read_buf_size) {
    // read in a new block of data
    fb->read_buf_size = fread(fb->buf, 1, FILEBUFF_SIZE, fb->fp);
    fb->read_offset = 0;
  }

  return (fb->read_buf_size > 0) ? fb->buf[fb->read_offset++] : 0;
}


/*
 * buffer up as much data as we can, and flush if we're out of room
 */
void fileBuffWriteFxU8Array(FILEBUFF *fb, FxU8 *data, int size)
{
  if (!fb || !fb->fp)
    return;
  if (fb->write_offset + size >= FILEBUFF_SIZE)
    fileBuffFlush(fb);

  while (size-- > 0)
    fb->buf[fb->write_offset++] = *data++;

  if (fb->always_flush)
    fileBuffFlush(fb);
}

void fileBuffWriteFxU32(FILEBUFF *fb, FxU32 data)
{
  FxU8 *p = (FxU8*)&data;

  if (!fb || !fb->fp)
    return;
  if (fb->write_offset + 4 >= FILEBUFF_SIZE)
    fileBuffFlush(fb);

#ifdef __WIN32__
  fb->buf[fb->write_offset++] = p[0];
  fb->buf[fb->write_offset++] = p[1];
  fb->buf[fb->write_offset++] = p[2];
  fb->buf[fb->write_offset++] = p[3];
#else
  fb->buf[fb->write_offset++] = p[3];
  fb->buf[fb->write_offset++] = p[2];
  fb->buf[fb->write_offset++] = p[1];
  fb->buf[fb->write_offset++] = p[0];
#endif

  if (fb->always_flush)
    fileBuffFlush(fb);
}

void fileBuffWriteFloat(FILEBUFF *fb, float data)
{
  fileBuffWriteFxU32(fb, *(FxU32*)&data);

  if (fb->always_flush)
    fileBuffFlush(fb);
}

void fileBuffWriteFxU16(FILEBUFF *fb, FxU16 data)
{
  FxU8 *p = (FxU8*)&data;

  if (!fb || !fb->fp)
    return;
  if (fb->write_offset + 2 >= FILEBUFF_SIZE)
    fileBuffFlush(fb);

#ifdef __WIN32__
  fb->buf[fb->write_offset++] = p[0];
  fb->buf[fb->write_offset++] = p[1];
#else
  fb->buf[fb->write_offset++] = p[1];
  fb->buf[fb->write_offset++] = p[0];
#endif

  if (fb->always_flush)
    fileBuffFlush(fb);
}

void fileBuffWriteFxU8(FILEBUFF *fb, FxU8 data)
{
  if (!fb || !fb->fp)
    return;
  if (fb->write_offset + 1 >= FILEBUFF_SIZE)
    fileBuffFlush(fb);

  fb->buf[fb->write_offset++] = data;

  if (fb->always_flush)
    fileBuffFlush(fb);
}

void fileBuffClose(FILEBUFF *fb)
{
  if (!fb || !fb->fp)
    return;
  fileBuffFlush(fb);
  fclose(fb->fp);
  memset(fb, 0, sizeof(FILEBUFF));
  free(fb);
}

int fileBuffEOF(FILEBUFF *fb)
{
  if (!fb || !fb->fp)
    return 1;
  return ((fb->read_offset >= fb->read_buf_size) && feof(fb->fp)) ? 1 : 0;
}
