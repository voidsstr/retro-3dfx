/*
 * Diag to test swapbuffer functionality.
 *
 * $Id$
 */


#include "udiag.h"
#include "sstdiag.h"
#include "h3asm.h"
#include <fximg.h>

#define TEST_NAME "swap2"

extern int diagSwaps;		// number of swaps commands issued (without DONT_SWAP)
extern void DIAG_SWAPBUFFER_EX();

int waitOnVsync = 1;
int dontSwap = 0;

SstRegs *sst;
void XParseOpts(int argc, char **argv);
void Xusage(void);

void draw_a_tri(Triangle * pt)
{
  randomTriangle(pt,diago.tsize,1);	    // pick random triangle
  randomRgbaTriangle(pt);		    // with random colors
		
  areaTriangle(pt);			    // compute the area (before setup)
  setupTriangle(pt,1,0,0);		    // setup RGB slopes
  sortTriangle(pt);			    // sort it
  printTriangle(2,pt,1,0,0);
  GDBG_INFO(3,"    triangle area = %d\n", pt->area);
		
  drawTriangle(sst,pt,1,0,0);
}

void main (int argc, char **argv)
{
  int n, i, j;
  static Triangle t;
  int swapInterval;
  int iters;
  int tsize;

  sst = SST_BEGIN( argc, argv );
  tsize = diago.tsize;

  XParseOpts(argc, argv);
  if (diago.printOpts) Xusage();

  // setup some reasonable starting modes
  SET(sst->fbzColorPath, SST_RGBSEL_RGBA | (diago.adjust?SST_PARMADJUST:0));
  SET(sst->fbzMode, SST_RGBWRMASK);

  // set the screen to grey, zbuffer to vertical stripes
  //   for ( k = 0; k < diago.ymaxscreen; k++ )
  //     for ( j = 0; j < diago.xmaxscreen; j++ )
  // 		{
  // 		  if (k & 1) DIAG_FORCE_PIXEL( CSIM_BUF_3D_FRONT, j, k, 0x808080);
  // 		  DIAG_FORCE_PIXEL( CSIM_BUF_3D_BACK, j, k, 0x808080);
  // 		  DIAG_FORCE_PIXEL( CSIM_BUF_3D_TRIPLE, j, k, 0xa5a5a5);
  // 		  if (diago.hasAuxBuffer)
  // 			 if (j & 1)
  // 				DIAG_FORCE_PIXEL( CSIM_BUF_3D_AUX1, j, k, 0xFFFF );
  // 		}

  iters = 10;
  while (DIAG_STARTPASS())					 // for each pass
    {
	for (n=0; n<iters; n++)
	  {
	    int pending;
	    int num_swaps;

	    GDBG_INFO(1,"iter = %d\n", n);

	    // generate a triangle of variable size
	    diago.tsize = (1+iRandom(4)) * tsize;
	    GDBG_INFO(3,"drawing a tri with side length = %d\n",diago.tsize);
	    draw_a_tri(&t);

	    // do 0, 1 or 2 swaps
	    num_swaps = iRandom(diago.triple?2:1);
	    GDBG_INFO(1,"doing %d swaps\n", num_swaps);
	    for (i=0; i<num_swaps; i++) {
		swapInterval = iRandom(2);

		// don't allow swapbuffer pending count to get larger than 6
	    again:
		pending = (GET(sst->status) & SST_SWAPBUFPENDING) >>
		  SST_SWAPBUFPENDING_SHIFT;
		if (!(pending < 7)) {
#ifdef HAL_HSIM
		  PCI_STALL(500);
		  GDBG_INFO(1,"stalling for swapbufpending to go down (%d)\n",pending);
		  fflush(stdout);
#endif
		  goto again;
		}

		DIAG_SWAPBUFFER_EX(swapInterval, waitOnVsync, dontSwap);
	    }
	  }
    }


  sst_idle_really(sst);

  // Check that the number of pending swaps is 0x0...
  j = (GET(sst->status) & SST_SWAPBUFPENDING) >> SST_SWAPBUFPENDING_SHIFT;
  if (j) {
    GDBG_ERROR(TEST_NAME,"Found non-zero swaps pending...pending_swaps=%d\n",j);
    DIAG_FAIL();
  }

  GDBG_INFO(1,"idle at time %d ns ... fbiSwapHistory = %8.8x\n",
		   DIAG_TIME(), GET(sst->fbiSwapHistory));

  DIAG_PASS(0);
}


void
Xusage(void)
{
  GDBG_PRINTF( "swap2 option description:\n"  );
  GDBG_PRINTF( "-xnov  -> disable waiting for vsync\n" );
  GDBG_PRINTF( "-xnos  -> disable swapping\n" );
  exit( 0 );
  return;
}

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
	    if (!strcmp(opts, "nov"))
		waitOnVsync = 0;
	    else if (!strcmp(opts, "nos"))
		dontSwap = 1;
	    else
		Xusage();

	    while (*opts!='\n' && *opts!='\t' && *opts!=' ' && *opts) opts++;
	  }
    }

  GDBG_INFO(1,"CONFIG: sync_to_vsync=%d,  swap_enable=%d\n", waitOnVsync, dontSwap);
}







