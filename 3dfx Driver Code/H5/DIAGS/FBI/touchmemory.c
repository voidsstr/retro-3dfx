#include <stdlib.h>
#include <string.h>

#include "udiag.h"
#include "sstdiag.h"

typedef struct 
{
  int start;
  int stop;
  int step;
  int nTouches;
  int flipBits;
  int useFile;
  char filename[100];
} Options;

void parseArgs(Options *options, int *argc, char ***argv);
void usage(void);

int main(int argc, char **argv)
{
  SstRegs *sst;  
  CsimPrivate *cpriv;
  unsigned int i, start, stop, step, nTouches;
  unsigned int mask, nMaskBits;
  unsigned int address;
  Options options;

  options.start=-1;
  options.stop=-1;
  options.step=-1;
  options.flipBits=0;
  parseArgs(&options, &argc, &argv);

  sst=SST_BEGIN(argc, argv);
  cpriv=CSIM_PRIVATE(diago.sstCSIM);
  
  if(options.start < 0)
    start = 0;
  else
    start = options.start;
  
  if(options.stop < 0)
    stop = cpriv->info->fbiMemSize * 1024 * 1024;
  else
    stop = options.stop;

  if(options.nTouches < 0)    
    nTouches=128;
  else
    nTouches=options.nTouches;

  if(options.step < 0)    
    step = (stop-start)/(nTouches-1);
  else
    step = options.step;

  if(step<=0)
    step=1;

  if(start > stop)
    {
      GDBG_ERROR("main", "start must be less than stop\n");
      DIAG_FAIL();
    }

  GDBG_INFO(0, "**************************************************\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*     Use -J with touchMemory when using hsim!!  *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "*                                                *\n");
  GDBG_INFO(0, "**************************************************\n");  

  GDBG_INFO(1, "start: %d\n", start);
  GDBG_INFO(1, "stop : %d\n", stop);
  GDBG_INFO(1, "step : %d\n", step);
  GDBG_INFO(1, "flipBits: %d\n", options.flipBits);
  GDBG_INFO(1, "filename: \"%s\"\n", options.filename);

  while(DIAG_STARTPASS())
    {      
      if(options.useFile)
	{
	  FILE *fin;
	  
	  if((fin = fopen(options.filename, "rt"))==NULL)
	    {
	      GDBG_ERROR("touchMemory::main", "Couldn't open file \"%s\"\n",
			 options.filename);
	      DIAG_FAIL();	      
	    }

	  while(fscanf(fin, "0x%x\n", &address) != EOF)
	    {
	      GDBG_INFO(2, "touchMemory: address=0x%x\n", address);
	      address = address & 0xFFFFFFF0;
	      SET(sst->texBaseAddr, address);
	      
	      address = ((int) sst) + SST_TEX0_OFFSET - SST_3D_OFFSET;
	      halStore32((void *)address, rand() | (rand()<<16));
	    }
	  
	}      
      else if(options.flipBits)
	{
	  mask = (cpriv->info->fbiMemSize * 1024 * 1024) - 1;

	  nMaskBits=0;
	  i=mask;
	  while(i)
	    {
	      i>>=1;
	      nMaskBits++;
	    }

	  GDBG_INFO(2, "touchMemory: mask=0x%x nMaskBits=%d\n", 
		    mask, nMaskBits);
	  
	  for(i=0; i< nMaskBits; i++)
	    {
	      address = mask & (1<<i);
	      address = address & 0xFFFFFFF0;

	      /*rearrange address to be like f'ed up texBaseAddr*/
	      if(address & (1<<25))
		{
		  address |= 2;
		}
	      address &= SST_TEXTURE_ADDRESS;

	      GDBG_INFO(2, "touchMemory: bit=%d address=0x%x\n", i, address);
	      SET(sst->texBaseAddr, address);
	      
	      address = ((int) sst) + SST_TEX0_OFFSET - SST_3D_OFFSET;
	      halStore32((void *)address, i | (rand()<<16));
	    }
	}
      else
	{
	  for(i=0; i<=stop; i+=step)
	    {
	      GDBG_INFO(2, "touchMemory address: %d (0x%x)\n", 
			i&0xFFFFFFF0, i&0xFFFFFFF0);
	      SET(sst->texBaseAddr, i&0xFFFFFFF0);
	      address = ((int) sst) + SST_TEX0_OFFSET - SST_3D_OFFSET;
	      halStore32((void *)address, rand() | (rand()<<16));
	    }
	}
    }

  fxHalIdle(sst);

  DIAG_PASS(0);

  return(1);
}

void parseArgs(Options *options, int *argc, char ***argv)
{
  int i, j, push;
  options->useFile=0;
  
  for(i=0; i<*argc; i++)
    {
      push = 0;
      if(!strcmp((*argv)[i], "-start"))
	{
	  if(i == (*argc)-1) /*Not enough arguments*/
	    usage();
	  sscanf((*argv)[i+1], "%d", &options->start);
	  GDBG_INFO(1, "start: %d\n", options->start);
	  push = 2;
	}      
      else if(!strcmp((*argv)[i], "-stop")) 
	{
	  if(i == (*argc)-1) /*Not enough arguments*/
	    usage();
	  sscanf((*argv)[i+1], "%d", &options->stop);
	  GDBG_INFO(1, "stop: %d\n", options->stop);
	  push = 2;
	}      
      else if(!strcmp((*argv)[i], "-step")) 
	{
	  if(i == (*argc)-1) /*Not enough arguments*/
	    usage();
	  sscanf((*argv)[i+1], "%d", &options->step);
	  GDBG_INFO(1, "step: %d\n", options->step);
	  push = 2;
	}      
      else if(!strcmp((*argv)[i], "-nTouches")) 
	{
	  if(i == (*argc)-1) /*Not enough arguments*/
	    usage();
	  sscanf((*argv)[i+1], "%d", &options->nTouches);
	  GDBG_INFO(1, "nTouches: %d\n", options->nTouches);
	  push = 2;
	}      
      else if(!strcmp((*argv)[i], "-filename")) 
	{
	  if(i == (*argc)-1) /*Not enough arguments*/
	    usage();
	  strncpy(options->filename, (*argv)[i+1], 100);
	  options->useFile=1;
	  GDBG_INFO(1, "file: %s\n", options->filename);
	  push = 2;
	}      
      else if(!strcmp((*argv)[i], "-flipBits")) 
	{
	  options->flipBits=1;
	  GDBG_INFO(1, "flipBits: %d\n", options->flipBits);
	  push = 1;
	}      

      if(push>0)
	{
	  for(j=i; j< *argc; j++)
	    (*argv)[j] = (*argv)[j+push];

	  *argc-=push;
	  i--; /*Don't want to increment if args were removed*/
	}
    }  
}


void usage(void)
{
  GDBG_INFO(1, "Options:\n");
  GDBG_INFO(1, "\t[-start    <start value>]\n");
  GDBG_INFO(1, "\t[-stop     <stop value>]\n");
  GDBG_INFO(1, "\t[-step     <step value>]\n");
  GDBG_INFO(1, "\t[-nTouches <# touches>  (step overides nTouches)]\n");
  GDBG_INFO(1, "\t[-filename <filename>   (file has list of address)\n");
  GDBG_INFO(1, "\t[-flipBits]   (toggles each address bit separately)\n");
  exit(-1);
}
