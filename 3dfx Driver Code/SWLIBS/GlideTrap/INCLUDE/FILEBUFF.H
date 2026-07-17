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
**  filebuff.h
**
**  Simple file buffering for GlideTrap.
*/

#define FILEBUFF_SIZE (256*1024)

typedef struct {
  FILE  *fp;
  FxU8   buf[FILEBUFF_SIZE];
  int    write_offset;
  int    read_offset;
  int    read_buf_size;
  FxBool always_flush;
} FILEBUFF;

extern FILEBUFF *fileBuffOpen(char *filename, char *mode);
extern void fileBuffFlush(FILEBUFF *fp);
extern void fileBuffClose(FILEBUFF *fp);
extern int  fileBuffEOF  (FILEBUFF *fp);

extern void fileBuffWriteFxU8     (FILEBUFF *fp, FxU8  data);
extern void fileBuffWriteFxU16    (FILEBUFF *fp, FxU16 data);
extern void fileBuffWriteFxU32    (FILEBUFF *fp, FxU32 data);
extern void fileBuffWriteFloat    (FILEBUFF *fp, float data);
extern void fileBuffWriteFxU8Array(FILEBUFF *fp, FxU8 *data, int size);

extern FxU8  fileBuffReadFxU8     (FILEBUFF *fp);
extern FxU16 fileBuffReadFxU16    (FILEBUFF *fp);
extern FxU32 fileBuffReadFxU32    (FILEBUFF *fp);
extern float fileBuffReadFloat    (FILEBUFF *fp);
extern void  fileBuffReadFxU8Array(FILEBUFF *fp, FxU8 *data, int size);
