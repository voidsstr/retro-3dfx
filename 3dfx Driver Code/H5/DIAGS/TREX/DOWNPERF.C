/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Id$
**
** Texture download performance test.
**
** By default, we test 16-bit texture downloads for all lods (0-8).
** With options -8 and -xm#, you can override these defaults.
**
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

int single_mipmap = 0;
int mipmap;

Xusage(void)
{
  gdbg_printf( "downperf option description:\n"  );
  gdbg_printf( "-x""m#"" -> download only mipmap n\n");
  gdbg_printf( "-8   -> use 8-bit textures (16-bit are the default)\n");
  exit( 0 );
  return 0;
}

/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

void
XParseOpts(int argc, char **argv)
{
  char *opts = 0;
  FxBool done;
    
  while ((--argc > 0) && (**++argv))
    {
	if (argv[0][0] != '-')
	  continue;
	if (argv[0][1] != 'x')
	  continue;

	/* now parse all extended parameters */
	done = 0;
	opts = &argv[0][2];
	if (*opts == '\0')
	  Xusage();
	
	while (!done && *opts)
	  {
	    switch (*opts)
		{
	      case 'm':
		  sscanf(XGETARG(), "%i", &mipmap);
		  single_mipmap = 1;
		  break;
	      default:
		  Xusage();
		}
	    
	    opts += 1;
	  }
    }
}

void
main (int argc, char **argv)
{
  int n,i,format;
  SstRegs *sst;
  int iters, startTime, endTime, texels;
  int start_mip, end_mip;
  register busy;
  Triangle *t;
  
  // Parse the Command Line and Initialize the Simulator
  sst = SST_BEGIN( argc, argv );
  XParseOpts( argc, argv );
  FXUNUSED(startTime);
  FXUNUSED(endTime);
  
  // Handle Options
  if ( diago.printOpts )
    Xusage();

  if(diago.bigAssTextures)
    t = buildTriangle(2048, 2048);
  else
    t = buildTriangle(256, 256);

  t->next = NULL;
  diago.adjust = 1;			// need this on
 
  while (DIAG_STARTPASS()) {		// for each pass

    if (single_mipmap) {
	start_mip = mipmap;
	end_mip = mipmap;
    } else {
	start_mip = 0;
	end_mip = 8;
    }
    
    sst_idle_really(sst);
    texels = 0;
    startTime = DIAG_TIME();
    gdbg_info(1,"downperf: txl depth = %s, mipmap range: %d - %d\n",
		  diago.tex8 ? "8bpt" : "16bpt",
		  start_mip, end_mip);
    gdbg_info(1,"start of downloading at time = %d ns\n",startTime);

    for (n=start_mip; n<=end_mip; n++) {
	format = diago.tex8 ? SST_RGB332 : SST_RGB565;
	t->tex->tMode = format | SST_TC_REPLACE | SST_TCA_REPLACE;

	// if a single mipmap, then do more downloads for the smaller mipmaps.
	if (single_mipmap) { 
	  iters = n ? (1<<(n-1)) : 1;
	  gdbg_info(1, " lod=%d, iters=%d\n", n, iters);
	} else {
	  iters = 1;
	}

	// download the mipmap iters times
	for (i=0;i<iters;i++) {
	  texRandomTextureMap(sst,diago.trex, 0,8-n,8-n,t->tex);
	  texels += 1<<(2*(8-n));
	}
    }
    
    // do my own idle loop, so that i don't kill command fifo performance
    // with status reads:

    busy = 1;
    hb_histFlushAll();		// flush command packet history buffer
    while (busy) {
#ifdef HAL_HSIM
      PCI_STALL(2000/15);	// wait for 2000 ns
#endif
      busy = GET(diago.sst->status);
      busy |= GET(diago.sst->status);
      busy |= GET(diago.sst->status);
      busy &= SST_BUSY;
    }

    endTime = DIAG_TIME();

    gdbg_info(1, "finished downloading at time = %d ns\n", endTime);
    gdbg_info(1, "%d texels downloaded in %d ns, %.2fM texels/sec\n",
		  texels, endTime-startTime, texels*1e3/(endTime-startTime));
  }

  DIAG_PASS(1);				// check for black screen
}
