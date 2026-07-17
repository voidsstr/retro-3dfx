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

/*-----------------------------------------------------------------------------
@func nrand
@date 8/14/98
@arg int num - number of descrete random values
@return int - integer random value
@imp
@key
@sect
@html
Generate an integer random value between 0 and num-1
@end
-----------------------------------------------------------------------------*/
int nrand(int num) 
{
  int val;

  val = RAND_MAX/num;

  val = rand()/val;

  if(val >= num)
    val = num-1;

  return(val);
}

/*-----------------------------------------------------------------------------
@func next_arg
@date 8/28/98
@arg conform_state *state - current conformance state
@arg int *argc - pointer to command line argument count
@arg char *** argv - pointer to command line list of argument strings
@return char * - pointer to next argument string
@imp
@key
@sect
@html
Return the next program argument.  Automatically handles argument filkes and
file recursion.
@end
-----------------------------------------------------------------------------*/
char *next_arg(conform_state *state, int *argc, char ***argv)
{
  char *curarg;
  char instr[64];

  while(1) {
    if(state->argdepth == 0) {
      // read from arg list
      if(--(*argc)) {
	curarg = *((*argv)++);
	if(!_stricmp(curarg,"-f") ||
	   !_stricmp(curarg,"/f")) {
	  if(--(*argc)) {
	    curarg = *((*argv)++);
	    if((state->argstack[0] = fopen(curarg,"rt")) == NULL) {
	      // bad file
	      log_perror(state, "Could not open argument file");
	    } else {
	      // opened file
	      state->argdepth = 1;
	    }
	  } else {
	    // no filename specified - error
	    log_message(state, "-f option specified with no file");
	    // ignore arg
	    return(NULL);
	  }
	} else {
	  // return next argument
	  return(curarg);
	}
      } else {
	// end of arguments
	return(NULL);
      }
    } else {
      // read from file
      if(fscanf(state->argstack[state->argdepth-1],"%s",instr)== EOF) {
	// end of file - close it
	fclose(state->argstack[state->argdepth-1]);
	state->argdepth--;
      } else {
	if(!_stricmp(instr,"-f") ||
	   !_stricmp(instr,"/f")) {
	  if(state->argdepth == MAX_FILE_DEPTH) {
	    // Recursion overflow
	    log_message(state, "MAX_FILE_DEPTH exceeded");
	  } else {
	    if(fscanf(state->argstack[state->argdepth-1],"%s",instr)!= EOF) {
	      if((state->argstack[state->argdepth] =
		  fopen(instr,"rt")) == NULL) {
		// bad file
		log_perror(state, "Could not open argument file");
	      } else {
		// opened file
		state->argdepth++;
	      }
	    } else {
	      // no filename specified - error
	      log_message(state, "-f option specified with no file");
	      // ignore arg
  	      return(NULL);
	    }
	  }
	} else {
	  // return next arg from file
	  // copy this to individual storage
	  if(state->numargs == MAX_FILE_ARGS) {
	    // too many arguments
	    log_message(state, "MAX_FILE_ARGS exceeded");
	    return(NULL);
	  }
	  if((state->fileargs[state->numargs] =
	      (char *)malloc(strlen(instr)+1)) == NULL) {
	    // allocation failed
	    log_perror(state, "Could not allocate argument storage");
	    return(NULL);
	  }
	  state->numargs++;
	  strcpy(state->fileargs[state->numargs-1],instr);
	  return(state->fileargs[state->numargs-1]);
	}
      }
    }
  }
}

/*-----------------------------------------------------------------------------
@func clear_fileargs
@date 8/28/98
@arg conform_state *state - current conformance state
@return - None
@imp
@key
@sect
@html
Release storage allocated by file arguments
@end
-----------------------------------------------------------------------------*/
void clear_fileargs(conform_state *state)
{
  while(state->numargs--) {
    free(state->fileargs[state->numargs]);
  }
}

/*-----------------------------------------------------------------------------
@func rgbacolor
@date 9/03/98
@arg conform_state *state - current conformance state
@arg FxU32 color - 32 RGBA color
@return - GrColor_t color in current display format
@imp
@key
@sect
@html
Convert an RGBA color to the current display format 
@end
-----------------------------------------------------------------------------*/
GrColor_t rgbacolor(conform_state *state, FxU32 color)
{
  FxU8 red, grn, blu, alp;
  FxU32 outcolor;

  red = (FxU8)((color>>24)&0xff);
  grn = (FxU8)((color>>16)&0xff);
  blu = (FxU8)((color>>8)&0xff);
  alp = (FxU8)(color&0xff);

  switch(state->colorformat) {
  case GR_COLORFORMAT_RGBA:
    outcolor = ((FxU32)red)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)blu)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ARGB:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)red)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)blu;
    break;
  case GR_COLORFORMAT_BGRA:
    outcolor = ((FxU32)blu)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)red)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ABGR:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)blu)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)red;
    break;
  default:
    break;
  }

  return((GrColor_t )outcolor);
}

/*-----------------------------------------------------------------------------
@func argbcolor
@date 9/03/98
@arg conform_state *state - current conformance state
@arg FxU32 color - 32 ARGB color
@return - GrColor_t color in current display format
@imp
@key
@sect
@html
Convert an ARGB color to the current display format 
@end
-----------------------------------------------------------------------------*/
GrColor_t argbcolor(conform_state *state, FxU32 color)
{
  FxU8 red, grn, blu, alp;
  FxU32 outcolor;

  alp = (FxU8)((color>>24)&0xff);
  red = (FxU8)((color>>16)&0xff);
  grn = (FxU8)((color>>8)&0xff);
  blu = (FxU8)(color&0xff);

  switch(state->colorformat) {
  case GR_COLORFORMAT_RGBA:
    outcolor = ((FxU32)red)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)blu)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ARGB:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)red)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)blu;
    break;
  case GR_COLORFORMAT_BGRA:
    outcolor = ((FxU32)blu)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)red)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ABGR:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)blu)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)red;
    break;
  default:
    break;
  }

  return((GrColor_t)outcolor);
}

/*-----------------------------------------------------------------------------
@func bgracolor
@date 9/03/98
@arg conform_state *state - current conformance state
@arg FxU32 color - 32 BGRA color
@return - GrColor_t color in current display format
@imp
@key
@sect
@html
Convert an BGRA color to the current display format 
@end
-----------------------------------------------------------------------------*/
GrColor_t bgracolor(conform_state *state, FxU32 color)
{
  FxU8 red, grn, blu, alp;
  FxU32 outcolor;

  blu = (FxU8)((color>>24)&0xff);
  grn = (FxU8)((color>>16)&0xff);
  red = (FxU8)((color>>8)&0xff);
  alp = (FxU8)(color&0xff);

  switch(state->colorformat) {
  case GR_COLORFORMAT_RGBA:
    outcolor = ((FxU32)red)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)blu)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ARGB:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)red)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)blu;
    break;
  case GR_COLORFORMAT_BGRA:
    outcolor = ((FxU32)blu)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)red)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ABGR:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)blu)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)red;
    break;
  default:
    break;
  }

  return((GrColor_t)outcolor);
}

/*-----------------------------------------------------------------------------
@func abgrcolor
@date 9/03/98
@arg conform_state *state - current conformance state
@arg FxU32 color - 32 ABGR color
@return - GrColor_t color in current display format
@imp
@key
@sect
@html
Convert an ABGR color to the current display format 
@end
-----------------------------------------------------------------------------*/
GrColor_t abgrcolor(conform_state *state, FxU32 color)
{
  FxU8 red, grn, blu, alp;
  FxU32 outcolor;

  alp = (FxU8)((color>>24)&0xff);
  blu = (FxU8)((color>>16)&0xff);
  grn = (FxU8)((color>>8)&0xff);
  red = (FxU8)(color&0xff);

  switch(state->colorformat) {
  case GR_COLORFORMAT_RGBA:
    outcolor = ((FxU32)red)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)blu)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ARGB:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)red)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)blu;
    break;
  case GR_COLORFORMAT_BGRA:
    outcolor = ((FxU32)blu)<<24;
    outcolor |= ((FxU32)grn)<<16;
    outcolor |= ((FxU32)red)<<8;
    outcolor |= (FxU32)alp;
    break;
  case GR_COLORFORMAT_ABGR:
    outcolor = ((FxU32)alp)<<24;
    outcolor |= ((FxU32)blu)<<16;
    outcolor |= ((FxU32)grn)<<8;
    outcolor |= (FxU32)red;
    break;
  default:
    break;
  }

  return((GrColor_t)outcolor);
}


// Tolerance for auto compares
#define TOLERANCER (1.5f/32.0f)
#define TOLERANCEG (1.5f/32.0f)
#define TOLERANCEB (1.5f/32.0f)

/*-----------------------------------------------------------------------------
@func compare_color
@date 03/01/99
@arg conform_state *state - current conformance state
@arg FxU32 *screen_color - color in native format (expanded to 32 bits)
@arg Color4D *expected_color - pointer to expected color in Color4D format
@return - TEST_PASS if the colors are within tolerance, TEST_FAIL otherwise.
@imp
@key
@sect
@html
Using the native color format stored in the state (hardcoded to RGB565
for the moment), decide whether the screen_color and the expected_color
are within tolerance of eachother. The tolerances are defined above.
@end
-----------------------------------------------------------------------------*/
TestResult compare_color(conform_state *state, 
                         FxU32 screen_color,
                         Color4D *expected_color)
{
  Color4D screen;
  TestResult status = TEST_PASS;

  switch(/*state->something*/GR_LFBWRITEMODE_565) {
    case GR_LFBWRITEMODE_565: 
      RGB565_TO_Color4D(screen_color, &screen);
      break;
    case GR_LFBWRITEMODE_555:
    case GR_LFBWRITEMODE_1555: 
    case GR_LFBWRITEMODE_ZA16: 
    case GR_LFBWRITEMODE_888:
    case GR_LFBWRITEMODE_8888:
    case GR_LFBWRITEMODE_565_DEPTH:
    case GR_LFBWRITEMODE_555_DEPTH:
    case GR_LFBWRITEMODE_1555_DEPTH:
      log_perror(state, "Unsupported native format in compare_color4D");
      status = TEST_FAIL;
    default:
      break; 
  }

  if((TEST_FAIL == status) ||
     (screen.red > (expected_color->red + TOLERANCER)) ||
     (screen.red < (expected_color->red - TOLERANCER)) ||
     (screen.green > (expected_color->green + TOLERANCEG)) ||
     (screen.green < (expected_color->green - TOLERANCEG)) ||
     (screen.blue > (expected_color->blue + TOLERANCEB)) ||
     (screen.blue < (expected_color->blue - TOLERANCEB))) {
    status = TEST_FAIL;
  } 
  return status;
}

/*-----------------------------------------------------------------------------
@func unpack_native_buffer
@date 3/1/99
@arg conform_state *state - the framework's state
@arg GrLfbInfo_t *linfo - the lfb info returned from grLfbLock()
@arg FxU32 *buf - the data to unpack in place. 
@arg int buflen- the number of 32bit words in the buffer
@return none
@imp
@key
@sect
@html
The data in 'buf' (generally returned from an LFB read and expanded to
take up 32bits), is converted in place to RGBA8888 format. 
@end
-----------------------------------------------------------------------------*/
void unpack_native_buffer(conform_state *state, FxU32* buf, int buflen)
{
  FxU32 *color = (FxU32*)buf;

  switch(/* state->something*/ GR_LFBWRITEMODE_565) {

    case GR_LFBWRITEMODE_565: {
      for (int x = 0; x < buflen; x++) {
        FxU8 r = (FxU8)((*color & 0xf800) >> 8);
        FxU8 g = (FxU8)((*color & 0x07e0) >> 3);
        FxU8 b = (FxU8)((*color & 0x1f) << 3);
        *color++ = (r << 24) | (g << 16) | (b << 8) | 0xff; // RGBA8888
      }
      break;
    }
    case GR_LFBWRITEMODE_555:
    case GR_LFBWRITEMODE_1555:
    case GR_LFBWRITEMODE_888:
    case GR_LFBWRITEMODE_8888:
    case GR_LFBWRITEMODE_565_DEPTH:
    case GR_LFBWRITEMODE_555_DEPTH:
    case GR_LFBWRITEMODE_1555_DEPTH:
    log_perror(state, "Unsupported writemode in unpack_native_format()"); 
    default:
      break;
  }
}

/*-----------------------------------------------------------------------------
@func expand_native_buffer
@date 3/1/99
@arg conform_state *state - the framework's state
@arg GrLfbInfo_t *linfo - the lfb info returned from grLfbLock()
@arg void *buf - the data to unpack in place. 
@arg int width - the width of the buffer (in pixels)
@arg int height - the height of the buffer (in pixels)
@return none
@imp
@key
@sect
@html
The data in 'buf' (generally returned from an LFB read), is converted
in place to take up 32bits. It goes in the least significant half.
@end
-----------------------------------------------------------------------------*/
void expand_native_buffer(conform_state *state, 
                          GrLfbInfo_t *linfo, void* buf,
                          int width, int height)
{
  int x, y;
  FxU16 *p16;
  FxU32 *p32;
  FxU8 *bufPtr = (FxU8*)buf;

  switch(linfo->writeMode) {

    case GR_LFBWRITEMODE_565: 
    case GR_LFBWRITEMODE_555:
    case GR_LFBWRITEMODE_1555: 
    case GR_LFBWRITEMODE_ZA16: {
      for (y = 0; y < height; y++) {

        // point 'em at the beginning of each scanline
        p16 = (FxU16*)(bufPtr + (y * width * 4));
        p32 = (FxU32*)(bufPtr + (y * width * 4));

        // offset them to the end of their data in the scanline
        p16 += width-1; // last 16bit entry in scanline
        p32 += width-1; // last 32bit space in scanline

        // work backwards, filling in the 32bit data 
        for (x = 0; x < width; x++) {  
          *p32-- = *p16--;
        }
      }
      break;
    }
    case GR_LFBWRITEMODE_888:
    case GR_LFBWRITEMODE_8888:
    case GR_LFBWRITEMODE_565_DEPTH:
    case GR_LFBWRITEMODE_555_DEPTH:
    case GR_LFBWRITEMODE_1555_DEPTH:
      // Already in 32bit format.
    default:
      break;
  }
}

/*-----------------------------------------------------------------------------
@func texture_base
@date 3/1/99
@arg conform_state *state - the framework's state
@arg GrChipID_t - tmu - the tmu we are downloading to
@arg FxU32      - evenOdd - which levels to download (even, odd, both)
@arg GrTexInfo * - info - texture info
@return none
@imp
@key
@sect
@html
Select a random valid location in memory to download the texture to. Ensures
correct alignment and that the texture does not cross the 2MB boundary on hardware
where this is an issue.
@end
-----------------------------------------------------------------------------*/
FxU32 texture_base(conform_state *state, GrChipID_t tmu, FxU32 evenOdd, GrTexInfo *info)
{
  FxU32 base, end;
  int rangehi;
  FxU32 seed;

  seed = grTexMaxAddress(tmu) - grTexMinAddress(tmu);

  rangehi = (int)((seed>>14)+3);

  // Generate a random valid base address
  do {
    base = (((FxU32)nrand((seed>>14)+1))<<14)|(FxU32)nrand(0x4000);
    base += grTexMinAddress(tmu);
    // Obey base alignment rules
    base &= ~((FxU32)state->tex_align-1);
    end = base + grTexTextureMemRequired(evenOdd, info);
           // cannot be out of range
  } while ((base < grTexMinAddress(tmu)) ||
	       (end > grTexMaxAddress(tmu))  ||
	      // cannot pass over 2Meg boundary
	      ((base & 0x200000) != ((end-1L) & 0x200000)));

  return(base);
}
