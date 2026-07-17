/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:33:23 PM$ 
**
*/

#include <atutil.h>
#include <atrender.h>

#include <glide.h>

#include <atinput.h>
#include <atscene.h>
#include <atdemop.h>
#include <fxos.h>

/*-------------------------------------------------------------------
  Function: atdDumpFrameBuffer();
  Date: 11/06/96
  Implementor(s): mlwp
  Library: AT Input
  Description:
    Dump the framebuffer to a file
  Arguments:
    fileName name of file to dump screen to
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static AtrImg i;
static AtrCanvas cSave, cFullScreen;

FxBool
atdDumpFrameBuffer(char *fileName) {
    FILE *fp;
    FxU16 *fb;
    FxU16 col;
    int x, y;
    FxU8 r, g, b;

    if ( i.data == NULL ) {
        AtrDriverCaps caps;

        atrQueryDriverCaps( &caps );
        i.format = ATR_IMGFMT_RGB_565;
        i.width = caps.width;
        i.height = caps.height;
        i.nLevels = 1;
        i.name = NULL;
        if ( ( i.data = malloc(i.width*i.height*2)) == NULL ) {
            atuError(FXTRUE, 
                     "atdDumpFrameBuffer: could not allocate buffer\n");
        }
        i.table = NULL;
        i.devPrivate = NULL;
    
        cFullScreen.xMin = 0;
        cFullScreen.yMin = 0;
        cFullScreen.xMax = i.width-1;
        cFullScreen.yMax = i.height-1;
    }

    atrQueryCanvas( &cSave );
    atrSelectCanvas( &cFullScreen);
    atrGrabImg( &i, 0, 0, ATR_BUFFER_BACKBUFFER );
    atrSelectCanvas( &cSave );

    if ( ( fp = fopen(fileName, "wb")) == NULL ) 
        return FXFALSE;

    fprintf( fp, "P6\n%d %d\n255\n", i.width, i.height );
    fflush( fp );

    fb = i.data;
    for ( y = 0; y < i.height; y++ ) {
      for( x = 0; x < i.width; x++ ) {
          col = fb[x + y * i.width];
          r = ( col & 0xf800 ) >> 8;
          g = ( col & 0x07e0 ) >> 3;
          b = ( col & 0x001f ) << 3;

          fwrite( &r, 1, 1, fp );
          fwrite( &g, 1, 1, fp );
          fwrite( &b, 1, 1, fp );
        }
    }
    fclose(fp);
    return FXTRUE;
}
