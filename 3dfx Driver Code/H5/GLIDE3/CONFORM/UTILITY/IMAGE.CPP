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
@func frame_name
@date 8/21/98
@arg char *name - string tag associated with current frame
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Associate a string tag with the current frame.
@end
-----------------------------------------------------------------------------*/
void frame_name(char *name, conform_state *state)
{
  strncpy(state->frameroot,name,64);
  state->framecnt = 0;
}

/*-----------------------------------------------------------------------------
@func img_file_type
@date 8/21/98
@arg ImgType type - bitmap image type as defined in fximg.h
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Associate image file type with current frame
@end
-----------------------------------------------------------------------------*/
void img_file_type(ImgType type, conform_state *state)
{
  state->imagetype = type;
}



/*--------------------------------------------------------------------------
@func save_image_to_file
@arg conform_state *state - the framework state
@arg char *filename - the name in which to save the image
@arg FxU32 *data - the RGBA8888 format data to make an image out of
@arg int width - the width of the image to save
@arg int height - the height of the image to save
@return FXTRUE if successful, FXFALSE otherwise
@html
Given RGBA888 data, create a (ppm) image out of it and save it to a
file.
@end
---------------------------------------------------------------------------*/
FxBool
save_image_to_file(conform_state *state, char *filename, 
                   FxU32 *data, int width, int height, FxBool unpackFlag)
{          
  ImgInfo iinfo;
  switch(state->imagetype) {
    case IMG_P6:
      strcat(filename, ".ppm");
      switch(state->origin) {
      case GR_ORIGIN_UPPER_LEFT:
        iinfo.tgaInfo.yOrigin = 1; // upper left
        break;
      case GR_ORIGIN_LOWER_LEFT:
        iinfo.tgaInfo.yOrigin = 0; // lower left
        break;
      }
      break;
#ifdef RMG
    case IMG_BMP:
      strcat(filename, ".bmp");
      break;
#endif // RMG
    default:
      // unsupported file format
      log_message(state,"User requested unsupported file format");
      return FXFALSE;
      break;
  }

  int buflen = width * height;
  iinfo.any.width = width;
  iinfo.any.height = height;
  iinfo.any.sizeInBytes = buflen * sizeof(FxU32);

  FxU32 *tmp_buf = (FxU32 *)malloc(buflen * sizeof(FxU32));

  if (!tmp_buf) {
    printf("couldn't alloc temporary image buffer!\n");
    return FXFALSE;
  }

  // copy incoming data to tmp buf
  memcpy(tmp_buf, data, buflen * sizeof(FxU32));

  // if necessary, unpack it in place
  if (unpackFlag) {
    unpack_native_buffer(state, tmp_buf, buflen);
  }

  // at this point, it's RGBA. We need ARGB.
  FxU32 *color = tmp_buf;
  for (int x = 0; x < buflen; x++) {
    FxU8 r = (FxU8)((*color & 0xff000000) >> 24);
    FxU8 g = (FxU8)((*color & 0x00ff0000) >> 16);
    FxU8 b = (FxU8)((*color & 0x0000ff00) >> 8);
    FxU8 a = (FxU8)((*color & 0x000000ff));
    *color++ = (a << 24) | (r << 16) | (g << 8) | b; // RGBA to ARGB
  }

  iinfo.any.data = (FxU8*)tmp_buf;
  if (!imgWriteFile(filename, &iinfo, state->imagetype, (FxU8*)tmp_buf)) {
    return FXFALSE;
  }

  free((void*)tmp_buf);

  return FXTRUE;
}

/*-----------------------------------------------------------------------------
@func end_frame
@date 8/21/98
@arg int op - end of frame operation code
@arg conform_state *state - current conformance state
@arg void **mem - list of pointers to memory buffers for "save to memory"
@arg int buffer_mask - specifies buffer types process
@return none
@imp
@key
@sect
@html
Specify end of frame and process the buffers by copying the active region to
either an image file or a memory buffer
@end
-----------------------------------------------------------------------------*/
void end_frame(int op, conform_state *state, void **mem, int  buffer_mask)
{
  int i, j, native_size;
  FxU32 buffer;
  FxU8 *curbuffer;
  FxU8 *dataptr;
  FxU8 *bufferptr;
  GrLfbInfo_t linfo;
  char filebuf[3];
  char filename[256+128]; // path + name
  char tmp_str[512]; 
  FILE *datafile;
  listptr curhead;
  listptr nexthead;
  unsigned int checksum;

  // make sure 3D has completed
  grFinish();

  linfo.size = sizeof( GrLfbInfo_t );
 
  // CVG Glide3 grLfbLock() doesn't do anything with linfo.writeMode
  // for read only locks at the moment. This value should just pass
  // through.
  linfo.writeMode = GR_LFBWRITEMODE_565; 

  if ((op != END_OP_NOOP) || state->dumpall) {
    buffer = 0;
    for(i=0;i<END_MAX_BUFFER_TYPE;i++) {
      if (buffer_mask & (1<<i)) {
        // dump this buffer
        if ((1<<i)&END_BUFFER_FRONTBUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock frontbuffer");
            return;
          }
          strcpy(filebuf,"fb");
        }
        if ((1<<i)&END_BUFFER_BACKBUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock backbuffer");
            return;
          }
          strcpy(filebuf,"bb");
        }
        if ((1<<i)&END_BUFFER_AUXBUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock auxbuffer");
            return;
          }
          strcpy(filebuf,"xb");
        }
        if ((1<<i)&END_BUFFER_DEPTHBUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock depthbuffer");
            return;
          }
          strcpy(filebuf,"zb");
        }
        if ((1<<i)&END_BUFFER_ALPHABUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_ALPHABUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock alphabuffer");
            return;
          }
          strcpy(filebuf,"ab");
        }
        if ((1<<i)&END_BUFFER_TRIPLEBUFFER) {
          if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_TRIPLEBUFFER,
                        GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                        &linfo)) {
            // unable to lock buffer
            log_message(state,"Could not lock triplebuffer");
            return;
          }
          strcpy(filebuf,"tb");
        }
  
        if (op & END_OP_SAVE_TO_MEM) {
          // use user supplied buffer
          if (mem == NULL) {
            // error - user didn't specify buffer
            log_message(state,"User did not specify memory pointer list");
            return;
          }
          if (mem[buffer] == NULL) {
            // error - user didn't specify buffer
            log_message(state,"User did not specify memory buffer");
            return;
          }
          // copy to buffer list
          curbuffer = (FxU8 *)mem[buffer];
        } else {
          // allocate buffer
          if ((curbuffer = (FxU8 *)
               malloc(state->conform_width*state->conform_height*4)) == NULL) {
            // error - could not allocate memory
            log_perror(state,"Could not allocate temporary frame storage");
            return;
          }
        }
     
        //  figure out the size of the native FB data
        switch(linfo.writeMode) {
          case GR_LFBWRITEMODE_565:
          case GR_LFBWRITEMODE_555:
          case GR_LFBWRITEMODE_1555:
            native_size = 2; // 16 bit
            break;
          case GR_LFBWRITEMODE_888:
          case GR_LFBWRITEMODE_8888:
          case GR_LFBWRITEMODE_565_DEPTH:
          case GR_LFBWRITEMODE_555_DEPTH:
          case GR_LFBWRITEMODE_1555_DEPTH:
            native_size = 4; // 32 bit    
            break;
          default:
          log_perror(state, "Unsupported writemode came back from grLfbLock()");
        }

        // It's possible that the conform window can be offscreen now. 
        // Make sure the region we'll read is onscreen.

        int min_x, min_y, max_x, max_y;
        min_x = state->conform_x;
        min_y = state->conform_y;
        max_x = state->conform_x + state->conform_width;
        max_y = state->conform_y + state->conform_height;
       
        if (min_x < 0) min_x = 0;
        if (min_y < 0) min_y = 0;
        if (min_x > (state->screen_width-1)) min_x = state->screen_width-1;
        if (min_y > (state->screen_height-1)) min_y = state->screen_height-1;

        if (max_x < 0) max_x = 0;
        if (max_y < 0) max_y = 0;
        if (max_x > (state->screen_width-1)) max_x = state->screen_width-1;
        if (max_y > (state->screen_height-1)) max_y = state->screen_height-1; 

        int box_x, box_y, box_width, box_height;
        box_x = min_x;
        box_y = min_y;
        box_width = (max_x - min_x);
        box_height = (max_y - min_y);

        // read data from buffer
        dataptr = (FxU8 *)linfo.lfbPtr;
        switch(state->origin) {
        case GR_ORIGIN_UPPER_LEFT:
          dataptr += (box_y * linfo.strideInBytes)+
                     (box_x * native_size);
          break;
        case GR_ORIGIN_LOWER_LEFT:
          dataptr += ((state->screen_height -
                       (box_height + box_y))*
                      linfo.strideInBytes) + (box_x * native_size);
          break;
        }

        // read native data into our 32bit buffer scanline by scanline
        bufferptr = curbuffer;
        for(j = 0; j < box_height; j++) {
          lfb_read_data(state, bufferptr, dataptr,
                        box_width * native_size);
          dataptr += linfo.strideInBytes;
          bufferptr += box_width * 4;
        }

        // At this point, we've read the native format data into our
        // 32bit buffer. If the native data was 16 bit, a scanline takes
        // up half of our 32bit scanline. We need to expand each 16bit
        // pixel to take up the least significant half of 32bits.

        expand_native_buffer(state, &linfo, curbuffer,
                             box_width, box_height);

        if ((op & END_OP_SAVE_TO_FILE)|state->dumpall) {
 
          // Gompute the checksum of the image data after it's been
          // converted from raw LFB format.  
          checksum = compute_checksum(curbuffer,
                            box_height * box_width *4);

          // create file name
          sprintf(filename,"%s/%s_%s_%04d",
                          state->output_dir, state->frameroot, filebuf,
                          state->framecnt);
 
          // save the image, if we're supposed to, and log the checksum
          if (state->save_images) {
            if (!save_image_to_file(state, filename, (FxU32*)curbuffer, 
                                    box_width, box_height,
                                    FXTRUE)) {
             // image write failed
             log_perror(state,"Could not write image file");
             return;
            }
            sprintf(tmp_str, "checksum: 0x%08X, image: %s_%s_%04d", 
                    checksum, state->frameroot, filebuf, state->framecnt);
          } else {
            sprintf(tmp_str, "checksum: 0x%08X, image: %s_%s_%04d (not saved)", 
                    checksum, state->frameroot, filebuf, state->framecnt);
          }
          log_frame(state, tmp_str); 
        } 
  
        // free temporary buffer
        if (!(op & END_OP_SAVE_TO_MEM)) {
          free(curbuffer);
        } else {
          // The memory image is always forced to be upper left origin
          // independent of screen image
          if (state->origin == GR_ORIGIN_LOWER_LEFT) {
            // invert memory buffer if origin is upper left
            FxU32 *startbuffer;
            FxU32 *endbuffer;
            FxU32 tmpval;
            int x, y;
            FxU32 *curbufptr = (FxU32*)curbuffer;
            
            for(y = 0; y < (state->conform_height/2); y++) {
              startbuffer = curbufptr + (y * state->conform_width);
              endbuffer = curbufptr + 
                ((state->conform_height - y - 1) * state->conform_width);
              for(x = 0; x < (state->conform_width); x++) {
                tmpval = *startbuffer;
                *startbuffer++ = *endbuffer;
                *endbuffer++ = tmpval;
              }
            }
          }
        }
  
        // unlock this buffer
        if ((1<<i)&END_BUFFER_FRONTBUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER);
        }
        if ((1<<i)&END_BUFFER_BACKBUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER);
        }
        if ((1<<i)&END_BUFFER_AUXBUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER);
        }
        if ((1<<i)&END_BUFFER_DEPTHBUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER);
        }
        if ((1<<i)&END_BUFFER_ALPHABUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_ALPHABUFFER);
        }
        if ((1<<i)&END_BUFFER_TRIPLEBUFFER) {
          grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_TRIPLEBUFFER);
        }
  
  
        // point to next buffer
        buffer++;
      }
    }
  }

  // write out associated frame data if we're in -dump mode
  if (state->dumpall && (state->datahead != NULL)) {
    // create file name
    sprintf(filename,"%s/%s_%04d.dat",
            state->output_dir, state->frameroot, state->framecnt);
    
    // open file
    if ((datafile = fopen(filename,"wb")) == NULL) {
      // could not open file
      log_perror(state,"Could not open date log file");
      return;
    }

    // start at head of data list
    curhead = state->datahead;
    state->datahead = NULL;

    // process each block
    while(curhead != NULL) {

      // write out data
      fwrite(curhead->data,1,curhead->size,datafile);

      nexthead = curhead->next;

      // free list element
      free(curhead->data);
      free(curhead);

      curhead = nexthead;
    }

    fclose(datafile);
  }

  state->framecnt++;

  // wait for user if interactive
  pause(state);
  
  if (op & END_OP_SWAP) {
    grBufferSwap(1);
    return;
  }
}

