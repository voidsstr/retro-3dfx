/* -*-c++-*- */
/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:12:11 PM$
*/

#include <stdio.h>
#include <time.h>
#include <fxos.h>
#include <h3.h>
#include <string.h>
#include "udiag.h"
#include "allocate.h"

#ifdef assert
#undef assert
#endif
#define assert(exp) (void)( (exp) || (diagAssert(#exp, __FILE__, __LINE__), 0) )


static char expected_eos = 0;
static float startTime;
Diagopts diago;
DiagFrameBuffer diagfb;

static void logit(char *msg, char *name, char *time)
{
    char *cp, logname[200];
    FILE *fp;

    if ( diago.halInfo->hw )
      return;          // don't print logs during hardware bringup

    strcpy(logname,name);
    for (cp = logname; *cp; cp++)       // lower case the name
        *cp = tolower(*cp);
    cp = strstr(logname,".exe");        // strip off trailing .exe
    if (cp) *cp = '\0';
    strcat(logname,".log");
    fp = fopen(logname,"w");
    if (fp == NULL) {
        gdbg_printf ("ERROR: could not open log file '%s'\n",logname);
        DIAG_FAIL();
        exit(2);
    }
    fprintf(fp,msg,name,time);
    fclose(fp);
}

static void usage2d(void)
{
    gdbg_printf("usage: %s [-bBcCDehIkKLmMoPrTUv] [-dEgGijOpstuwW #] [-fR # #] "
                "[-Q file]\n\t\t[+l #]\n", diago.pgm_name);
    gdbg_printf("\t\t-b => random Byte/word swapping\n");
    gdbg_printf("\t\t-B => random Blit x,y direction\n");
    gdbg_printf("\t\t-c => random clipSelect\n");
    gdbg_printf("\t\t-C => Check results per triangle\n");
    gdbg_printf("\t\t-d => Drawbuffer\n");
    gdbg_printf("\t\t-D => Diff the HW and SW framebuffers\n");
    gdbg_printf("\t\t-e => direct cmd fifo Execution mode\n");
    gdbg_printf("\t\t-E => set the Error limit\n");
    gdbg_printf("\t\t-f => force source color Format and packing\n");
    gdbg_printf("\t\t-g => use CMD FIFO #n\n");
    gdbg_printf("\t\t-G => set the Good bitmask for pixel compares\n");
    gdbg_printf("\t\t-h => disable Hole counting\n");
    gdbg_printf("\t\t-i => device Id (1=>SST1, 3=>H3, 4=>H3+, 5=>H4)\n");
    gdbg_printf("\t\t-I => input filename\n");
    gdbg_printf("\t\t-j => enable vector file generation\n");
    gdbg_printf("\t\t-k => random enable source color key\n");
    gdbg_printf("\t\t-K => random enable dest color key\n");
    gdbg_printf("\t\t-l => Log file name\n");
    gdbg_printf("\t\t-L => random enable Linestipple\n");
#if COLORTRANSLUT
    gdbg_printf("\t\t-m => random enable colorTransLut\n");
#endif
    gdbg_printf("\t\t-M => random Monochrome patterns\n");
    gdbg_printf("\t\t-N => Number of words in CMDFIFO ring (<0 enables out of order)\n");
    gdbg_printf("\t\t-o => list available test Options\n" );
    gdbg_printf("\t\t-O => test Option\n" );
    gdbg_printf("\t\t-p => number of Passes to run\n");
    gdbg_printf("\t\t-P => random Pattern offsets\n");
    gdbg_printf("\t\t-q => dump PPM files\n");
    gdbg_printf("\t\t-Q => preload image file into frontbuffer\n");
    gdbg_printf("\t\t-r => random Rops\n");
    gdbg_printf("\t\t-R => Revision FBI TREX\n");
    gdbg_printf("\t\t-s => random Seed\n");
    gdbg_printf("\t\t-t => random primitive size\n");
    gdbg_printf("\t\t-T => random Transparent patterning\n");
    gdbg_printf("\t\t-u => destination format 1=>8, 3=>16, 4=>24, 5=>32\n");
    gdbg_printf("\t\t-U => random update destination XY\n");
    gdbg_printf("\t\t-v => random wait for Vsync\n");
    gdbg_printf("\t\t-V => enable frontbuffer Video testing\n");
    gdbg_printf("\t\t-w => Width of display screen\n");
    gdbg_printf("\t\t-y => tiled memory (-1=random,0=Lsrc/Ldst,1=T/T,2=L/T,3=T/L\n");

    // '+' options
    gdbg_printf("\n\t\t+l => set pixel limit\n");
}


static void usage3d(void)
{
    gdbg_printf("usage: %s [-8aAbBcCDfFHIJmMnoPrSTVZ] [-dEgGijOpstwXYz #] "
                        "[-R # #] [-Q file]\n", diago.pgm_name);
    gdbg_printf("\t\t--32bpt => 32-bit textures\n");
    gdbg_printf("\t\t--aaSampleCount <N>      => Number of AA samples (default 2)\n");
    gdbg_printf("\t\t--bigAssTextures         => Use up to 2048x2048 textures\n");
    gdbg_printf("\t\t--chipCount <N>          => Use N-Way scanline interleave\n");
    gdbg_printf("\t\t--compressedTextures     => Use random compressed textures\n");
    gdbg_printf("\t\t--dontCheckEveryTriangle => Forces off checkEveryTriangle\n");
    gdbg_printf("\t\t--enableAA               => Enables antialiasing\n");
    gdbg_printf("\t\t--enableSLI              => Enables SLI\n");
    gdbg_printf("\t\t--log2BandHeight <N>     => When rendering 2 pixels per clock,\n");
    gdbg_printf("\t\t                            this selects the height of each of\n");
    gdbg_printf("\t\t                            the TMU's rendering bands\n");
    gdbg_printf("\t\t                            default: -1  => Random band height\n");
    gdbg_printf("\t\t--pixelsPerClock <N>     => Enables 2 pixel per clock rendering\n");
    gdbg_printf("\t\t                  N=1    => 1 pixel per clock\n");
    gdbg_printf("\t\t                  N=2    => 2 pixels per clock\n");
    gdbg_printf("\t\t                  N<=0   => randomized # of pixels per clock\n");
    gdbg_printf("\t\t--randomCmdFifoPlacement => Randomly Place cmd FIFOs at <16M\n");
    gdbg_printf("\t\t--randomPlacement        => Place buffers and textures randomly\n");
    gdbg_printf("\t\t--sliBandHeight <N>  => Band height that each chips renders\n");
    gdbg_printf("\t\t--triColumnBand <N>  => sets fbzColorPath[31:30]\n");
    gdbg_printf("\t\t                        N >= 0, value set once to N\n");
    gdbg_printf("\t\t                        N < 0,  Randomly selects from [0,2]\n"); 
    gdbg_printf("\t\t                        default: 0\n");
    gdbg_printf("\t\t-3 => enable triple buffering\n");
    gdbg_printf("\t\t-4 => Randomly select between 15bpp, 16bpp, and 32bpp\n");
    gdbg_printf("\t\t-5 => enable 1555 ARGB mode\n");
    gdbg_printf("\t\t-6 => enable 565 RGB mode\n");
    gdbg_printf("\t\t-7 => enable 8888 ARGB mode\n");
    gdbg_printf("\t\t-8 => 8-bit textures\n");
    gdbg_printf("\t\t-a => subpixel Adjust parameters\n");
    gdbg_printf("\t\t-A => random zA\n");
    gdbg_printf("\t\t-b => Bilinear filter texels\n");
    gdbg_printf("\t\t-B => random Bilinear filter texels\n");
    gdbg_printf("\t\t-c => Clamp texture coordinates\n");
    gdbg_printf("\t\t-C => Check results per triangle\n");
    gdbg_printf("\t\t-d => Drawbuffer\n");
    gdbg_printf("\t\t-D => Diff the HW and SW framebuffers\n");
    gdbg_printf("\t\t-E => set the Error limit\n");
    gdbg_printf("\t\t-g => use CMD FIFO #n\n");
    gdbg_printf("\t\t-G => set the Good bitmask for pixel compares\n");
    gdbg_printf("\t\t-f => mirror or Flip textures\n");
    gdbg_printf("\t\t-F => Floating point stw calculations\n");
    gdbg_printf("\t\t-h => disable Hole counting\n");
    gdbg_printf("\t\t-H => run with Hardware simulator\n");
    gdbg_printf("\t\t-i => device Id (1=>SST1, 3=>H3, 4=>H3+, 5=>H4)\n");
    gdbg_printf("\t\t-I => input filename\n");
    gdbg_printf("\t\t-j => enable vector file generation\n");
    gdbg_printf("\t\t-J => use FRONT door texture writes\n");
    gdbg_printf("\t\t-l => Log file name\n");
    gdbg_printf("\t\t-m => enable Mipmap lod bias\n");
    gdbg_printf("\t\t-M => enable Mipmap lod dither\n");
    gdbg_printf("\t\t-n => enable NCC texture compression\n");
    gdbg_printf("\t\t-o => list available test Options\n" );
    gdbg_printf("\t\t-O => test Option\n" );
    gdbg_printf("\t\t-p => number of Passes to run\n");
    gdbg_printf("\t\t-P => Perspective correct\n");
    gdbg_printf("\t\t-q => dump PPM files\n");
    gdbg_printf("\t\t-Q => preload image file into frontbuffer\n");
    gdbg_printf("\t\t-r => Rectangular textures\n");
    gdbg_printf("\t\t-R => Revision FBI TREX\n");
    gdbg_printf("\t\t-s => random Seed\n");
    gdbg_printf("\t\t-S => enable texture Split\n");
    gdbg_printf("\t\t-t => random Triangle size\n");
    gdbg_printf("\t\t-T => enable multiple Texture base addresses\n");
    gdbg_printf("\t\t-V => enable frontbuffer Video testing\n");
    gdbg_printf("\t\t-w => Width of display screen\n");
    gdbg_printf("\t\t-X => set trexInit0 initial value\n");
    gdbg_printf("\t\t-y => tiled memory (-1=random,0=linear,1=tiled,2=perf\n");
    gdbg_printf("\t\t-Y => set trexInit1 initial value\n");
    gdbg_printf("\t\t-z => TREX chip to test\n");
    gdbg_printf("\t\t-Z => enable Zero lod fraction\n");

    // '+' options
    gdbg_printf("\n\t\t+c => align tri vertices to column-of-8 boundaries\n");
    gdbg_printf("\t\t+e => save back buffer/memory before swap depending on -q/+m\n");
    gdbg_printf("\t\t+g => generate CRC value\n");
    gdbg_printf("\t\t+k => test hardware's CRC value against the given value\n");
    gdbg_printf("\t\t+m => save memory contents in a file\n");
    gdbg_printf("\t\t+o => place front/back buffer in opposite memory banks\n");
    gdbg_printf("\t\t+p => diff memory and test CRC value (paranoia mode)\n");
    gdbg_printf("\t\t+r => video refresh rate in Hz\n");
    gdbg_printf("\t\t+s => byte/word swizzling (-1=random,1=word,10=byte,11=both)\n");
    gdbg_printf("\t\t+v => set # frames in video\n");
}

static int usage(void)
{
    diago.gui ? usage2d() : usage3d();
    exit(1);
    return 0;
}

// GETARG returns either the remainder of the current argument or
//    the next argument if there is one.
#define GETARG token[1] ? ctmp=token+1, token=" ", ctmp : --argc > 0 ? *++argv : (char *)usage()

static void parse_opts(int argc, char **argv)
{
    int backdoor=1;
    char firstchar;
    char *cp;
    
    // init default values that are non-zero
    diago.pgm_name = argv[0];   // save name for usage()
    if ( cp = strrchr(diago.pgm_name,'/') )  // strip off path (dos)
      diago.pgm_name = cp+1;
    if ( cp = strrchr(diago.pgm_name,'\\') ) // strip off path (unix)
      diago.pgm_name = cp+1;
    cp = strstr(diago.pgm_name,".exe");  // strip off trailing .exe
    if (cp) *cp = '\0';
    cp = strstr(diago.pgm_name,".EXE");
    if (cp) *cp = '\0';
    diago.passes = 1;
    diago.rgb = 16;             // default to RGB 565 mode
    diago.goodMask = 0xFFFFFFFF;// pixel compare bitmask
    diago.ringSize = 75;        // default CMD FIFO ring size
    diago.seed = 1;
    diago.tsize = 20;           // default triangle size
    diago.width = 640;          // default window size
    diago.xmaxscreen = 640;     // default framebuffer size
    diago.ymaxscreen = 480;
    diago.hasAuxBuffer = 1;
    diago.deviceID = SST_DEVICE_ID_AP_OEM;
    diago.fbiRevision = 1;
    diago.trexRevision = CSIM_DEFAULT_TMU_REV;
    diago.usePacket6 = 0;
    diago.videoMaxFrame = 1;
    diago.refreshRate = GR_REFRESH_NONE;
    diago.tex32=0;
    diago.chipCount=1;
    diago.sliBandHeight=1;
    diago.dontCheckEveryTriangle=FXFALSE;
    diago.bigAssTextures=FXFALSE;
    diago.randomPlacement=FXFALSE;
    diago.randomCmdFifoPlacement=FXFALSE;
    diago.pixelsPerClock=1;
    diago.log2BandHeight=-1;   //Select a random band height by default
    diago.triColumnBand=0;  
    diago.aaSampleCount = 2;
    diago.aaEnabled=FXFALSE;
    diago.sliEnabled=FXFALSE;
    diago.sst=NULL;
    diago.sstChildren[0]=NULL;
    diago.sstChildren[1]=NULL;
    diago.sstChildren[2]=NULL;
    // parse command line arguments
    while (--argc > 0 && (((firstchar = **++argv) == '-') ||
                          (firstchar == '+')))
    {
        char *token, *ctmp;

        for (token = argv[0] + 1; *token; token++)
        {
        if (firstchar == '-')
        {
	  //Check for expanded names first
	  if(!strcmp("-32bpt", token))
	    {
	      diago.tex32=1;
	      break;
	    }
	  else if(!strcmp("-chipCount", token))
	    {
	      argv++;
	      argc--;

	      //Check for the sli chip count arg
	      if(*argv == NULL || (*argv[0] < '0' || *argv[0] > '9'))
		{
		  GDBG_ERROR("parse_opts", "Need the number of chips (e.g. --chipCount 2)\n");
		  exit(-1);
		}
	      
	      sscanf(*argv, "%d", &diago.chipCount);

	      if(diago.chipCount <= 0 || diago.chipCount > 8)
		{
		  GDBG_INFO(0, "WARNING! Illegal f'n chip count. Must be in [1,8]\n");
		  diago.chipCount = 0;
		}

	      break;
	    }
	  else if(!strcmp("-aaSampleCount", token))
	    {
	      argv++;
	      argc--;

	      //Check for the sli chip count arg
	      if(*argv == NULL || (*argv[0] < '0' || *argv[0] > '9'))
		{
		  GDBG_ERROR("parse_opts", "Need the number of samples (e.g. --aaSampleCount 2)\n");
		  exit(-1);
		}
	      
	      sscanf(*argv, "%d", &diago.aaSampleCount);

	      if(diago.aaSampleCount != 2 && diago.aaSampleCount != 4)
		{
		  GDBG_ERROR(0, "WARNING! Illegal f'n aaSampleCount. Must be 2 or 4!\n");
		  exit(-1);
		}

	      break;
	    }
	  else if(!strcmp("-sliBandHeight", token))
	    {
	      argv++;
	      argc--;

	      //Check for the sli chip count arg
	      if(*argv == NULL || (*argv[0] < '0' || *argv[0] > '9'))		 
		{
		  GDBG_ERROR("parse_opts", "Include the SLI band height (e.g. --sliBandHeight 4)\n");
		  exit(-1);
		}
	      
	      sscanf(*argv, "%d", &diago.sliBandHeight);

	      if(diago.sliBandHeight <= 0 || diago.sliBandHeight > 128)
		{
		  GDBG_INFO(0, "WARNING! Illegal f'n SLI band height. Must be in [1,128]\n");
		  GDBG_INFO(0, "         A band height of 1 will be used\n");
		  diago.sliBandHeight = 0;
		}

	      break;
	    }
	  else if(!strcmp("-pixelsPerClock", token))
	    {
	      argv++;
	      argc--;

	      //Check for the sli chip count arg
	      if(*argv == NULL ||
		 (!((*argv[0] >= '0' && *argv[0] <= '9') || *argv[0] == '-')))
		{
		  GDBG_ERROR("parse_opts", "Include the pixelsPerClock arg\n");
		  exit(-1);
		}
	      
	      sscanf(*argv, "%d", &diago.pixelsPerClock);

	      break;
	    }
	  else if(!strcmp("-log2BandHeight", token))
	    {
	      argv++;
	      argc--;

	      //Check for the band height argument
	      if(*argv == NULL ||
		 (!((*argv[0] >= '0' && *argv[0] <= '9') || *argv[0] == '-')))
		{
		  GDBG_ERROR("parse_opts", "Include the log2BandHeight arg\n");
		  exit(-1);
		}

	      sscanf(*argv, "%d", &diago.log2BandHeight);

	      break;
	    }
	  else if(!strcmp("-triColumnBand", token))
	    {
	      argv++;
	      argc--;

	      //Check for the band height argument
	      if(*argv == NULL ||
		 (!((*argv[0] >= '0' && *argv[0] <= '9') || *argv[0] == '-')))
		{
		  GDBG_ERROR("parse_opts", "Include the triColumnBand arg\n");
		  exit(-1);
		}

	      sscanf(*argv, "%d", &diago.triColumnBand);

	      break;
	    }
	  else if(!strcmp("-enableAA", token))
	    {
	      diago.aaEnabled = FXTRUE;
	      break;
	    }
	  else if(!strcmp("-enableSLI", token))
	    {
	      diago.sliEnabled = FXTRUE;
	      break;
	    }
	  else if(!strcmp("-dontCheckEveryTriangle", token))
	    {
	      diago.dontCheckEveryTriangle=FXTRUE;
	      break;
	    }
	  else if(!strcmp("-bigAssTextures", token))
	    {
	      diago.bigAssTextures=FXTRUE;
	      break;
	    }
	  else if(!strcmp("-randomPlacement", token))
	    {
	      diago.randomPlacement=FXTRUE;
	      break;
	    }
	  else if(!strcmp("-randomCmdFifoPlacement", token))
	    {
	      diago.randomCmdFifoPlacement=FXTRUE;
	      break;
	    }
	  else if(!strcmp("-compressedTextures", token))
	    {
	      diago.compressedTextures=FXTRUE;
	      break;
	    }
        switch (*token) {
            case '3':
                diago.triple = 1;
                break;
            case '4':
	      //We'll select the depth later if a random depth is desired
	      diago.rgb = 0;	     
	      break;
            case '5':
                diago.rgb = 15;
                break;
            case '6':
                diago.rgb = 16;
                break;
            case '7':
                diago.rgb = 32;
                break;
            case '8':
                diago.tex8 = 1;
                break;
            case 'a':
                diago.adjust = 1;
                break;
            case 'A':
                diago.randomZA = 1;
                break;
            case 'b':
                diago.bilinear = 1;
                break;
            case 'B':
                diago.bilinear = -1;
                break;
            case 'c':
                diago.clamp = 1;
                break;
            case 'C':
                diago.checkEveryTriangle = 1;
                break;
            case 'd':
                diago.drawbuffer = atoi(GETARG);
                break;
            case 'D':
                diago.diff = 1;
                break;
            case 'e':
                diago.directExec = 1;
                break;
            case 'E':
                diago.errorLimit = atoi(GETARG);
                break;
            case 'f':
                if (diago.gui) {
#ifdef CVG
                    diago.flip |= atoi(GETARG)<<SSTG_SRC_LANES_SHIFT;
#else
                    diago.flip = atoi(GETARG)<<SSTG_SRC_FORMAT_SHIFT;
                    diago.flip |= atoi(GETARG)<<SSTG_SRC_PACK_SHIFT;
#endif
                    diago.flip |= 0x80000000;   // make sure its non-zero
                }
                else diago.flip = 1;
                break;
            case 'F':
                diago.floatSTW = 1;
                break;
            case 'g':
                sscanf(GETARG,"%i",&diago.whichFifo); 
                if (diago.whichFifo >= 10) {
                  diago.usePacket6 = diago.whichFifo/10;
                  diago.whichFifo -= 10;
                }
                break;
            case 'G':
                sscanf(GETARG,"%i",&diago.goodMask); 
                break;
            case 'h':
                diago.disableHoles = 1;
                break;
            case 'H':           // talk to hardware
                diago.halInfo->hsim |= HSIM_HW_SIMULATION;
                break;
            case 'i':
                diago.deviceID = atoi(GETARG);
                break;
            case 'I':
                diago.infile_name = GETARG;
                break;
            case 'j':           // Enable generation of vector files
                sscanf(GETARG,"%x",&diago.vectorGenMask); 
                break;
            case 'J':           // turn off backdoor tex writes
                backdoor = 0;
                break;
            case 'k':
                diago.srcKey = 1;
                break;
            case 'K':
                diago.dstKey = 1;
                break;
            case 'l':
                diago.pgm_name = GETARG;
                break;
            case 'L':
                diago.lstipple = 1;
                break;
            case 'm':
                diago.lodbias = 1;
                break;
            case 'M':
                diago.loddither = 1;
                break;
            case 'n':
                diago.ncc = 1;
                break;
            case 'N':
                sscanf(GETARG,"%i",&diago.ringSize);
                break;
            case 'o':
                diago.printOpts = 1;
                break;
            case 'O':
                sscanf(GETARG,"%i",&diago.option);
                break;
            case 'q':
                diago.dumpPpm = 1;
                break;
            case 'Q':
                diago.imgFilename = GETARG;
                break;
            case 'p':
                diago.passes = atoi(GETARG);
                break;
            case 'P':
                diago.perspective = 1;
                break;
            case 'r':
                diago.rectangular = 1;
                break;
            case 'R':
                diago.fbiRevision = atoi(GETARG);
                diago.trexRevision = atoi(GETARG);
                break;
            case 's':           // initial random seed
                diago.seed = atoi(GETARG);
                break;
            case 'S':
                diago.tsplit = 1;
                break;
            case 't':           // triangle size
                diago.tsize = atoi(GETARG);
                break;
            case 'T':
                diago.multiTexBaseAddr = 1;
                break;
            case 'u':
                diago.dstFormat = atoi(GETARG);
                break;
            case 'U':
                diago.updatexy = 1;
                break;
            case 'v':
                diago.vsync = 1;
                break;
            case 'V':
                diago.videoTest = 1;
                break;
            case 'w':           // window width
                diago.width = atoi(GETARG);
                break;
            case 'W':           // write packets to CMD FIFO
                diago.writeFifo = atoi(GETARG);
                diago.agpEnable = 0;
                if (diago.writeFifo > 10) {
                  diago.agpEnable = diago.writeFifo/10;
                  diago.writeFifo -= 10;
                }

                break;
            case 'x':
                GETARG;         // diag specific option eXtension, ignore
                                // next argument
                break;          // must be used like "-xX 4" if you want a 
            case 'X':
                sscanf(GETARG,"%i",&diago.trexInit0); 
                break;
            case 'y':
                sscanf(GETARG,"%i",&diago.ytiled); 
                break;
            case 'Y':
                sscanf(GETARG,"%i",&diago.trexInit1); 
                break;
            case 'z':           // TREX chip
                diago.trex = atoi(GETARG);
                break;
            case 'Z':
                diago.zeroLodFrac = 1;
                break;
            default:
                gdbg_printf ("illegal '-' option - '%c'\n", *token);
                usage ();
                break;
        }
        } // if (firstchar == '-')
        else
        {
            // '+' options
            switch (*token)
            {
              case 'c':
                  diago.columnOf8Align = 1;
                  break;

              case 'e':
                  diago.saveBeforeSwap = 1;
                  break;

              case 'f':
		GDBG_INFO(0, "Warning! +f doesn't do shit anymore!\n");
		//diago.bumpFloor = strtol(GETARG,NULL,0);  // in dec, hex, or oct
                  break;

              case 'l':
                  diago.pixelLimit = atoi(GETARG);
                  gdbg_printf("INFO: pixelLimit set to %d\n",diago.pixelLimit);
                  break;

	      case 'g':
		  diago.generateCrc = 1;
		  break;

	      case 'k':
		  diago.checkCrc = 1;
		  diago.crc = strtoul(GETARG,NULL,0);  // in dec, hex, or oct
		  break;

              case 'm':
                  diago.dumpMemory = 1;
                  break;

	      case 'n':
		  diago.halInfo->csimio = 0;
		  break;

              case 'o':
		   GDBG_INFO(0, "WARNING!: The +o (opposite banks) is no longer supported\n");
		   //diago.oppositeBanks = 1;
                  break;

	      case 'p':
		  diago.paranoid = 1;
		  break;

              case 'r':
		  diago.refreshRate = atoi(GETARG);
		  switch ( diago.refreshRate ) {
		  case 60:  diago.refreshRate = GR_REFRESH_60Hz; break;
		  case 70:  diago.refreshRate = GR_REFRESH_70Hz; break;
		  case 72:  diago.refreshRate = GR_REFRESH_72Hz; break;
		  case 75:  diago.refreshRate = GR_REFRESH_75Hz; break;
		  case 80:  diago.refreshRate = GR_REFRESH_80Hz; break;
		  case 85:  diago.refreshRate = GR_REFRESH_85Hz; break;
		  case 90:  diago.refreshRate = GR_REFRESH_90Hz; break;
		  case 100: diago.refreshRate = GR_REFRESH_100Hz; break;
		  case 120: diago.refreshRate = GR_REFRESH_120Hz; break;
		  default: 
		    gdbg_printf("Illegal refresh rate = %d Hz\n",diago.refreshRate); 
		    usage ();
		    break;
		  }
                  break;

              case 's':
                  diago.swizzle = atoi(GETARG);
                  break;

              case 'v':
                  diago.videoMaxFrame = atoi(GETARG);
                  gdbg_printf("INFO: video frame to %d\n",diago.videoMaxFrame);
                  break;

              case 'y':
                  diago.yorigin = 1;
                  gdbg_printf("INFO: old yorigin bits enabled\n");
                  break;

              default:
                  gdbg_printf ("illegal '+' option - '%c'\n", *token);
                  usage ();
                  break;
            } // switch (*token)
        }
        } // for (token = argv[0] + 1; *token; token++)
        
        
        
    }
    if (argc > 0) usage();
    if (diago.halInfo->hsim && backdoor)
        diago.halInfo->hsim |= HSIM_TREX_BACKDOOR_TEXWRITES;
    if (diago.halInfo->hsim && diago.swizzle)
        diago.halInfo->hsim |= HSIM_SWIZZLE;
    if (diago.halInfo->hw && diago.swizzle)
        diago.halInfo->hw |= HSIM_SWIZZLE;

    //Don't allow split compressed textures
    if(diago.compressedTextures && diago.tsplit)
      {
	diago.tsplit = 0;
	GDBG_INFO(0, "\n");
	GDBG_INFO(0, "Warning! Hardware does not support split, compressed textures.\n");
	GDBG_INFO(0, "         Disabling split textures (enabled with -S)\n");	
	GDBG_INFO(0, "\n");
      }
}

//----------------------------------------------------------------------
// BEGIN a diag program, perform chip independent init code
//      1) parse command line
//      2) set command line option settings
//      3) print banner
//----------------------------------------------------------------------

static char *BANNER = 
"/*----------------------------------------------------------------------*/";

void DIAG_BEGIN(int argc, char **argv)
{
    diago.halInfo = fxHalInit(0);       // init HAL and gdebug utilities
    parse_opts(argc,argv);              // parse command line options

    setSeed(diago.seed);                // set options
    gdbg_printf("%s\n",BANNER);         // print banner

    gdbg_printf("/* Diag:");            // print command line
    while (argc--) gdbg_printf(" %s",*argv++);
    gdbg_printf("\n");
    {
        char host[80];
        time_t tloc;
        gethostname(host,sizeof(host));
        time(&tloc);
        gdbg_printf("/* Host: %-14s\tDate: %s",host,ctime(&tloc));
        gdbg_printf("/*\n");    /* */
        logit("%-10s running on %s",diago.pgm_name,ctime(&tloc));
    }
    //  if its set, then there's no FBI in the hardware simulation
    //  so we skip all pixel checks (they are checked via another method)
    diago.trexStandAlone = (int)getenv("TREX_STANDALONE");
    if (diago.trexStandAlone) {
        gdbg_info(1,"TREX_STANDALONE mode enabled\n");
        diago.halInfo->hsim |= HSIM_TREX_STANDALONE;
    }
    startTime = fxTime();
}

//----------------------------------------------------------------------
// start a new pass
//----------------------------------------------------------------------
static FxU32 pixLimit;  // stats.fbiPixelsOut limit for this pass

int DIAG_STARTPASS(void)
{
    static passnum = 0;

    if (passnum < diago.passes) {
        passnum++;
        gdbg_printf("Pass %d: seed=%u\n",passnum,getSeed());

	//Be sure to unallocate all non-essential chunks of memory in the
	//frame buffer. This includes textures and random allocations
	//The color buffer, aux buffer, and command FIFOs aren't 
	//affected because they're locked	
	unallocateAll();

        pixLimit = diago.sstCSIM->stats.fbiPixelsOut + diago.pixelLimit;
        pixLimit += CSIM_PRIVATE(diago.sstCSIM)->pixelsOut2d;

        return 1;
    }
    else return 0;
}

//----------------------------------------------------------------------
// check to see if the pixel limit has been exceeded, return non-zero if yes
//----------------------------------------------------------------------
int DIAG_EXCEEDED_PIXEL_LIMIT(void)
{
#ifdef CVG
    if (diago.sstCSIM->stats.fbiPixelsOut > pixLimit) {
#else
    if (diago.sstCSIM->stats.fbiPixelsOut +
        CSIM_PRIVATE(diago.sstCSIM)->pixelsOut2d > pixLimit) {
#endif
        gdbg_info(0,"cutting diag short, more than %d pixels rendered this pass\n",
                        diago.pixelLimit);
        return 1;
    }
    return 0;
}

// GMT: this is getting very complicated
// if check_screen is non-zero we test the screen to make sure it's all 0's
// if checkEveryTriangle==0 then we are not checking and erasing triangles as
// the diags draw them, so the only thing we can do is to diff the screen at
// the end (which we do even it if the diag didn't ask for it)
void DIAG_PASS(int check_screen)
{
    time_t tloc;

    DIAG_END(0);

    if (check_screen >= 0) {    // Some diags shouldn't check memory
      if ( diago.halInfo->hw ) {
	if ( diago.checkCrc ) {
	  if ( diago.paranoid ) 
	    DIAG_DIFFMEMORY();
	  DIAG_TEST_CRC(diago.crc);
	} else if ( diago.generateCrc ) {
	  if ( diago.paranoid )
	    DIAG_DIFFMEMORY();
	  GDBG_INFO(0,"Hardware's composite CRC is 0x%x\n",DIAG_READ_CRC_COMPOSITE());
	  // if/when it's possible to compute the CRC on-the-fly, compare calculated
	  // value with hardware's value.  Be sure they either both pass or both fail.

	  //if ( DIAG_TEST_CRC( DIAG_CALC_CRC_COMPOSITE() ) ) {  // crc failed
	  //  if ( diago.paranoid == 1 && diago.errorCount == 0 && GDBG_GET_ERRORS() == 0 )
	  //    GDBG_ERROR("CRC","Paranoia: CRC passed but there are errors!\n");
	  //} 
	} else {
	  DIAG_DIFFMEMORY();
	  // for real hardware, only diff screens if there's an error after diff'ing memory
	  if ( diago.errorCount || GDBG_GET_ERRORS() ) {
	    if (check_screen && diago.checkEveryTriangle)
	      DIAG_TESTSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1,0);
	    else if (diago.diff || !diago.checkEveryTriangle) 
	      DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
	  }
	}
      } else {
	// if no hardware, diff screens first then diff memory
	// thus, if we hit the diago.errorLimit limit we know which pixels failed
	if (check_screen && diago.checkEveryTriangle)
	  DIAG_TESTSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1,0);
	else if (diago.diff || !diago.checkEveryTriangle) 
	  DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
	DIAG_DIFFMEMORY();
      } 
    }

    if (diago.errorCount)               // double check error count
        DIAG_FAIL();                    // since it may not have reached limit
#ifdef HAL_HSIM
    if (diago.halInfo->hsim)
        if (TESTBENCH_CHECK_DONE())     // double check hardware all done
            DIAG_FAIL();
#endif
    if (GDBG_GET_ERRORS() > 0) {
        GDBG_ERROR("DIAG_PASS","%d GDEBUG errors detected\n",GDBG_GET_ERRORS());
        DIAG_INCERROR();
        DIAG_FAIL();                    // since it may not have reached limit
    }
    expected_eos = 1;
    time(&tloc);
    gdbg_printf("/*\n");
    gdbg_printf("/* No Errors\t\tDate: %s",ctime(&tloc)); /* */
    gdbg_printf("%s\n",BANNER);
    logit("%-10s passed on %s",diago.pgm_name,ctime(&tloc));
    fxHalShutdown(diago.sst);
#ifdef HAL_HSIM
    if (diago.halInfo->hsim)
        TESTBENCH_PASS();
#endif
    exit(0);
}

void DIAG_FAIL(void)
{
    time_t tloc;
    static flag = 0;

    expected_eos = 1;
    DIAG_END(1);
    if (flag) exit(3);
    flag = 1;                   // prevent infinite loop

    time(&tloc);
    gdbg_printf("/*\n");
    gdbg_printf("/* %s failed\t\tDate: %s",diago.pgm_name,ctime(&tloc)); /* */
    gdbg_printf("%s\n",BANNER);
    logit("%-10s *** FAILED *** on %s",diago.pgm_name,ctime(&tloc));
    fxHalShutdown(diago.sst);
#ifdef HAL_HSIM
    if (diago.halInfo->hsim)
        TESTBENCH_FAIL();       // This will automatically stop further execution..
#endif
    exit(6);
}

// Routine that is called when the hardware simulator ends
void DIAG_EOS(void)
{
    time_t tloc;

    if(!expected_eos) {
        time(&tloc);
        logit("%-10s incomplete on %s",diago.pgm_name,ctime(&tloc));
    }
}

// increment the error count, and if it exceeds the limit then fail the diag
int DIAG_INCERROR(void)
{
    diago.errorCount++;
    if (!expected_eos)          // if already within DIAG_FAIL don't recurse
    if (diago.errorCount >= diago.errorLimit) {
        gdbg_printf("WARNING: error count of %d exceeded, aborting...\n",diago.errorLimit);
        DIAG_FAIL();
        return 1;
    }
    return 0;
}

// compare 2 colors, report an error, return results
int DIAG_COMPARE_PIXEL(char *which, FxI32 buffer, int x, int y, FxU32 cRead, FxU32 cGood)
{
    FxI32 mask = diago.goodMask;
    char bufferName[32];

    switch(buffer)
      {
      case CSIM_BUF_2D_STRETCH_SRC:
	strcpy(bufferName, "2D Stretch Source");
	break;
      case CSIM_BUF_2D_SRC:
	strcpy(bufferName, "2D Source");
	break;
      case CSIM_BUF_2D_DST:
	strcpy(bufferName, "2D Destination");
	break;
      case CSIM_BUF_3D_FRONT:
	strcpy(bufferName, "3D Front");
	break;
      case CSIM_BUF_3D_BACK:
	strcpy(bufferName, "3D Back");
	break;
      case CSIM_BUF_3D_AUX1:
	strcpy(bufferName, "3D Aux1");
	break;
      case CSIM_BUF_3D_AUX2:
	strcpy(bufferName, "3D Aux2");
	break;
      case CSIM_BUF_3D_COLOR:
	strcpy(bufferName, "3D Color");
	break;
      case CSIM_BUF_DESKTOP:
	strcpy(bufferName, "Desktop");
	break;
      case CSIM_BUF_OVERLAY:
	strcpy(bufferName, "Overlay");
	break;
      case CSIM_BUF_CURSOR:
	strcpy(bufferName, "Cursor");
	break;
      case CSIM_BUF_YUV:
	strcpy(bufferName, "YUV");
	break;
      case CSIM_BUF_3D_TRIPLE:
	strcpy(bufferName, "3D Triple");
	break;
	
      default:
	strcpy(bufferName, "Unknown");
	break;
      }



    if (buffer == CSIM_BUF_3D_AUX1)     // if aux buffer
        mask >>= 24;                    // hack: sign extend 8-bit mask
    cRead &= mask;
    cGood &= mask;
    if (cRead != cGood) {
        gdbg_printf("ERROR(%s): expecting \"%s\" pixel[%d,%d] == %d(0x%x) but read %d(0x%x)\n",
                        which, bufferName, x,y, cGood,cGood, cRead,cRead);
        return DIAG_INCERROR();
    }
    return 0;
}

// compare 2 memory values, report an error, return results
int DIAG_COMPARE_MEM(char *which, FxU32 addr, FxU32 cRead, FxU32 cGood)
{
  int err = cRead == cGood ? 0 : 1;
  if ( cRead != cGood ) {
    gdbg_printf("ERROR(%s): expected data at 0x%x to be %d(0x%x) but read %d(0x%x)\n",
                which,addr,cGood,cGood,cRead,cRead);
    return DIAG_INCERROR();
  }
  return 0;
}


void DIAG_NOPS(void)
{
#ifdef HAL_HSIM
    PCI_RNDM(iRandom(0xFFFFFF));
#endif
}

// return the current simulation time in nanoseconds
int DIAG_TIME(void)
{
    int tim;
#ifdef HAL_HSIM
    if (diago.halInfo->hsim)
        tim = get_hdl_time();
    else
#endif
        // this is a totally bogus value....
        tim = (int)((fxTime()-startTime) * 1e06);
    gdbg_info(2,"DIAG_TIME = %d nsec\n",tim);
    return tim;
}

int DIAG_TESTREG16(char *regname, FxU16 good, FxU16 regval)
{
    if (regval != good) {
        gdbg_printf(
            "ERROR: expecting register '%s' to be %d(0x%x) but is %d(0x%x)\n",
            regname,good,good,regval,regval);
        return DIAG_INCERROR();
    }
    return 0;
}

int DIAG_TESTREG32(char *regname, FxU32 good, FxU32 regval)
{
    if (regval != good) {
        gdbg_printf(
            "ERROR: expecting register '%s' to be %d(0x%x) but is %d(0x%x)\n",
            regname,good,good,regval,regval);
        return DIAG_INCERROR();
    }
    return 0;
}

int DIAG_TESTPORT32(char *portname, FxU32 good, FxU32 portval)
{
    if (portval != good) {
        gdbg_printf(
            "ERROR: expecting port '%s' to be %d(0x%x) but is %d(0x%x)\n",
            portname,good,good,portval,portval);
        return DIAG_INCERROR();
    }
    return 0;
}

// convert a fixed number to a float, add a random delta to it
// and then convert back to an identical fixed point number
float fix2float(int *x, int fracbits)
{
    float f = (float)*x;                // convert to float

    f += rfRandom(-1,1);                // generate some random crap
    *x = float2fix(f/(1<<fracbits),fracbits);
    return ((float)*x)/(1<<fracbits);
}

int float2fix(float fdata, int fracbits)
{
    int exp,data,retval;

    data = *(int *)&fdata;              // load into integer
    exp = (data>>23) & 0xFF;            // peel off exponent
    fracbits = 150 - exp - fracbits;    // compute shift amount
    data |= 0x800000;                   // add in the hidden bit
    if (fracbits > 0) {
        if (fracbits > 31) fracbits = 31;
        retval = (data&0xFFFFFF) >> fracbits;
    }
    else {
        fracbits = -fracbits;
        if (fracbits > 31) fracbits = 31;
        retval = (data&0xFFFFFF) << fracbits;
    }

// since most of the int register (RGBAZ) have guard bits, don't worry
// about overflows, plus most of them occur in triangles that don't fill
// any pixels
//    if (retval < 0) gdbg_printf("ERROR: retval < 0\n");
    // negate after shift, this rounds down instead of to zero
    if (data & 0x80000000) retval = -retval;
    return retval;
}

FxI64 float2fix64(float fdata, int fracbits)
{
    int exp,data;
    FxI64 retval;

    data = *(int *)&fdata;              // load into integer
//GDBG_INFO(19,"float2fix64(%08x,%d)\n",data,fracbits);
    exp = (data>>23) & 0xFF;            // peel off exponent
    fracbits = 150 - exp - fracbits;    // compute shift amount
    data |= 0x800000;                   // add in the hidden bit
    if (fracbits > 0) {
        if (fracbits > 31) fracbits = 31;
        retval = (data&0xFFFFFF) >> fracbits;
    }
    else {
        fracbits = -fracbits;
        if (fracbits > 31) fracbits = 31;
        retval = (data&0xFFFFFF);
        retval <<= fracbits;            // use 64-bit shift
    }

    // negate after shift, this rounds down instead of to zero
    if (data & 0x80000000) retval = -retval;
//GDBG_INFO(19,"float2fix64 ==> %08x_%08x\n",FX_LO64(retval>>32),FX_LO64(retval));
    return retval;
}

// print out a fixed point number with a specified number of fraction bits
// the hard part is to handle negative numbers, especially -0.fraction
// also returns a static char buffer (one of 16 in a circle)
char *printFix(char *fmt, int val, int fracbits)
{
    char *pbuf;
    static char buf[16][32];
    static int nbuf;

    nbuf++; if (nbuf==16) nbuf=0;       // circulate the buffers
    pbuf = buf[nbuf];                   // get current buffer
    if (val >= 0)                       // if positive, then easy
        sprintf(pbuf, fmt, val >> fracbits, val & (0xFFFFFFFF >> (32-(fracbits))));
    else {                              // else negative, then tricky
        int i = (-val)>>fracbits;
        if (fmt[2] == 'x')
            i &= 0xFFFFFFFF >> (32-(fmt[1] - '0')*4);
        else
            i = -i;
        sprintf(pbuf, fmt, i, (-val) & (0xFFFFFFFF >> (32-(fracbits))));
        if (i == 0 || fmt[2] == 'x') {          // special case =0.fraction
            for (i=0; i<sizeof(buf[0]); i++) {  // search for 1st char
                if (pbuf[i] != ' ') {
                    if (i==0) gdbg_printf(
"WARNING: printFix - integer field is too small to fit negative sign\n");
                    else pbuf[i-1] = '-';       // add a '-' in front of it
                    break;
                }
            }
        }
    }
    return pbuf;
}

// NOTE: the hardcoded 48 is the size of STW iterators
char *printFix64(char *fmt, FxI64 val, int fracbits)
{
    char *pbuf;
    static char buf[16][32];
    static int nbuf;

    nbuf++; if (nbuf==16) nbuf=0;       // circulate the buffers
    pbuf = buf[nbuf];                   // get current buffer
    if (!(FX_HI64(val)&0x8000)) {       // if positive, then easy
        sprintf(pbuf, fmt,              // NOTE: fracbits must be <= 32
                FX_LO64(FX_SHR64(val,fracbits)) & (0xFFFFFFFF >> (32-(48-fracbits))),
                FX_LO64(val) & (0xFFFFFFFF >> (32-(fracbits))));
    }
    else {                                      // else negative, then tricky
        int i;
        FxI64 negval;
        negval = FX_NEG64(val);
        i = FX_LO64(FX_SHR64(negval,fracbits)); // integer portion
        i &= (0xFFFFFFFF >> (32-(48-fracbits)));
        if (fmt[2] == 'x')
            i &= 0xFFFFFFFF >> (32-(fmt[1] - '0')*4);
        else
            i = -i;
        sprintf(pbuf, fmt, i, FX_LO64(negval) & (0xFFFFFFFF >> (32-(fracbits))));
        if (i == 0 || fmt[2] == 'x') {          // special case =0.fraction
            for (i=0; i<sizeof(buf[0]); i++) {  // search for 1st char
                if (pbuf[i] != ' ') {
                    if (i==0) gdbg_printf(
"WARNING: printFix64 - integer field is too small to fit negative sign\n");
                    else pbuf[i-1] = '-';               // add a '-' in front of it
                    break;
                }
            }
        }
    }
    return pbuf;
}


Texture* buildTexture(FxU32 width, FxU32 height)
{
  Texture *texture;
  FxU32 nMipmaps;
  FxU32 maxDimension, nData;
  Mipmap *mipmap;
  FxU32 lod;

  if(width > height)
    maxDimension = width;
  else
    maxDimension = height;
  nData = maxDimension * maxDimension;

  texture = (Texture *)malloc(sizeof(Texture));
  assert(texture != NULL);

  //Zero out the texture structure
  memset(texture, 0, sizeof(Texture));

  //Set up mipmaps
  nMipmaps=0;
  lod=0;
  while(width > 0 && height > 0)
    {
      assert(nMipmaps <= 11);

      mipmap = (Mipmap *)malloc(sizeof(Mipmap));
      assert(mipmap != NULL);
      
      //Allocate space for mipmap data with a little bit of slop
      assert(nData > 0);
      mipmap->data = (FxU32 *)malloc((sizeof(FxU32))*nData);
      assert(mipmap->data != NULL);

      //Set up mipmap info
      mipmap->textureBaseAddress = 0;
      mipmap->mipmapBaseAddress = 0;
      mipmap->lod = lod++;
      mipmap->width = width;
      mipmap->height = height;
      mipmap->nData=nData;

      //Set pointer in texture to this mipmap
      texture->mip[nMipmaps] = mipmap;
      
      nMipmaps++;
      width/=2;
      height/=2;
      nData/=4;

      //Always allocate enough room for an 8x4 texture. This is
      //necessary to accomodate compressed textures which can
      //never really be smaller than 8x4
      if(nData < 32)
	nData = 32;

      if(width == 0 && height != 0)
	width = 1;
      else if(height == 0 && width != 0)
	height = 1;	
    }  

  texture->nMipmaps = nMipmaps;

  return(texture);
}

Triangle* buildTriangle(FxU32 textureWidth, FxU32 textureHeight)
{
  Triangle *triangle;

  triangle = (Triangle *)malloc(sizeof(Triangle));
  assert(triangle != NULL);

  //Allocate the texture for this triangle
  triangle->tex = buildTexture(textureWidth, textureHeight);
  
  return(triangle);
}

void copyTexture(Texture *destination, Texture *source)
{
  FxU32 i;
  Mipmap *mip[16];
  FxU32 *data;

  assert(destination != NULL);
  assert(source != NULL);
  assert(destination->nMipmaps == source->nMipmaps);
  assert(source->nMipmaps == destination->nMipmaps);

  //Backup mip pointers in destination
  memcpy(mip, destination->mip, sizeof(mip));
  
  //Copy main texture information
  memcpy(destination, source, sizeof(Texture));
  memcpy(destination->mip, mip, sizeof(mip));

  //copy the mipmaps
  for(i=0; i<source->nMipmaps; i++)
    {
      assert(source->mip[i]->nData = destination->mip[i]->nData);      

      //Backup pointer to destination's data
      data = destination->mip[i]->data;

      //Copy mipmap info
      memcpy(destination->mip[i], source->mip[i], sizeof(Mipmap));
      destination->mip[i]->data = data;
      
      //Copy the data
      memcpy(destination->mip[i]->data, source->mip[i]->data, sizeof(FxU32) * source->mip[i]->nData);
    }
}

void copyTriangle(Triangle *destination, Triangle *source)
{
  Texture *tex;
  
  assert(destination != NULL);
  assert(source != NULL);
  
  //Backup up destination's tex pointer
  tex = destination->tex;

  //Copy shit
  memcpy(destination, source, sizeof(Triangle));
  
  destination->tex = tex;
  
  //Copy the texture
  copyTexture(destination->tex, source->tex);
}

void deleteTexture(Texture *texture)
{
  FxU32 i;

  assert(texture != NULL);
  
  //Go through and delete all mipmaps
  for(i=0; i<texture->nMipmaps; i++)
    {
      assert(texture->mip != NULL);
      assert(texture->mip[i]->data != NULL);
      free(texture->mip[i]->data);
      free(texture->mip);
    }
  
  free(texture);
}

void deleteTriangle(Triangle *triangle)
{
  assert(triangle != NULL);
  assert(triangle->tex != NULL);
  
  deleteTexture(triangle->tex);
  free(triangle);
}

FxU32* calculateMipmapDataAddress(Texture *texture, FxU32 lod, FxU32 u, FxU32 v)
{
  Mipmap *mipmap;
  FxU32 width, height;

  assert(lod < texture->nMipmaps);
  mipmap = texture->mip[lod];

  width = mipmap->width;
  height = mipmap->height;

  //For compressed textures, we need to do some goofy stuff  
  if(SST_T4BIT_COMPRESSED(texture->tMode))
    {
      if(width < 8)
	width = 8;
      
      if(height < 4)
	height = 4;
    }
  else if(SST_T8BIT_COMPRESSED(texture->tMode))
    {
      if(width < 4)
	width = 4;
     
      if(height < 4)
	height = 4;
    }

  assert(u < width);
  assert(v < height);
  
  return(&mipmap->data[v*width + u]);
}

FxU32 getMipmapData(Texture *texture, FxU32 lod, FxU32 u, FxU32 v)
{
  return(*calculateMipmapDataAddress(texture, lod, u, v));
}

void setMipmapData(Texture *texture, FxU32 lod, FxU32 u, FxU32 v, FxU32 data)
{
  *calculateMipmapDataAddress(texture, lod, u, v) = data;
}

void convertFromMicroTile(FxU32 textureMode, FxI32 *u, FxI32 *v)
{
  if(SST_T4BIT_COMPRESSED(textureMode))
    {
      *u *= 8;
      *v *= 4;
    }
  else if(SST_T8BIT_COMPRESSED(textureMode))
    {      
      *u *= 4;
      *v *= 4;
    }
  else
    assert(0);
}

void convertToMicroTile(FxU32 textureMode, FxI32 *u, FxI32 *v)
{
  if(SST_T4BIT_COMPRESSED(textureMode))
    {
      *u = roundDownDivide(*u, 8);
      *v = roundDownDivide(*v, 4);
    }
  else if(SST_T8BIT_COMPRESSED(textureMode))
    {      
      *u = roundDownDivide(*u, 4);
      *v = roundDownDivide(*v, 4);
    }
  else
    assert(0);
}

FxI32 roundDownDivide(FxI32 n, FxI32 divisor)
{
  assert(divisor > 0);

  if(n > 0)
    return(n / divisor);
  else
    return((n - divisor + 1) / divisor);
}

FxBool isMultiTexturing(void)
{
  return(csimIsMultiTexturing2(shadowRegisters3D[0][0].fbzColorPath,
			       shadowRegisters3D[0][0].combineMode,
			       shadowRegisters3D[0][1].textureMode,
			       shadowRegisters3D[0][1].combineMode));
}

void diagAssert(char *expression, char *filename, int line)
{
  GDBG_ERROR("Assertion failed", "\"%s\" %s(%d)\n", expression, filename, line);
  DIAG_FAIL();
}
