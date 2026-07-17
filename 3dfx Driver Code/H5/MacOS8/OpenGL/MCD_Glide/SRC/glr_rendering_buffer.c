/*________________________________________________________________________________________
** 
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**  Description: Buffers related methods for the Apple OpenGL plugin
**
** 
**
*/


#include "glr.h"

#include "glr_drawing.h"
#include <unix.h>
#include <string.h>

#if PROFILE
#include <Profiler.h>
#endif

//#include "PerfMon.h"

FxU32 nextEntry = 0;
FxU32 _c0[10000];
FxU32 _c1[10000];
FxU32 _c2[10000];
FxU32 _c3[10000];

/*
________________________________________________________________________________________

      gldClear
________________________________________________________________________________________

*/

void gldClear(
	GLDContext			inContext,
	GLbitfield			inMask)
{
	GrColor_t			theColor;
	FxU16				theDepth;
	
	DEBUG_ENTRY( gldClear );
//        grBufferSwap( 1 );

	if( !inContext ) {
		DEBUG_ERROR( gldClear, "Invalid context pointer : 0x%08x\n", inContext);
		return;
	}
	
	// Setup clear values
	theColor  = (GrColor_t) (GLint) (inContext->state->clear_color.color.r * 255.99) << 16;
	theColor |= (GrColor_t) (GLint) (inContext->state->clear_color.color.g * 255.99) << 8;
	theColor |= (GrColor_t) (GLint) (inContext->state->clear_color.color.b * 255.99) << 0;
	
	theDepth = (FxU16) ( (inContext->state->clear_color.depth) * 65535.0 );

	// Set masks
	if( !(inMask & GL_COLOR_BUFFER_BIT) ) {
		grColorMask(FXFALSE, FXFALSE);
	}

#if SHOW_OVERDRAW
    grColorMask(FXTRUE, FXFALSE);
    theColor = 0;
#endif    	
    // In this case we need to clear the buffer even if GL_DEPTH_TEST is disabled.
 	grDepthMask( (inContext->state->mask_mode.depth_mask && (inMask & GL_DEPTH_BUFFER_BIT)) ? FXTRUE : FXFALSE);
	
	// Clear buffers
	grBufferClear( theColor, 0, theDepth );
	
	// Reset masks
	if( !(inMask & GL_COLOR_BUFFER_BIT) ) {
		FxBool theRGB;
		
		theRGB = ( inContext->state->mask_mode.color_red_mask
					&& inContext->state->mask_mode.color_green_mask
					&& inContext->state->mask_mode.color_blue_mask ? FXTRUE : FXFALSE);
		
		grColorMask( theRGB, FXFALSE );
	}
	
	//if( !(inMask & GL_DEPTH_BUFFER_BIT) ) {
	grDepthMask( (inContext->state->mask_mode.depth_mask && inContext->state->depth_test.enable)  ? FXTRUE : FXFALSE );
	//}
}


/*
________________________________________________________________________________________

      gldSwapBuffers
________________________________________________________________________________________

*/

void
gldSwapBuffers(
	GLDContext			inContext)
{
    //static unsigned long *lastFifoPtr;
    //long diff;
    
	DEBUG_ENTRY( gldSwapBuffers );
	/* FIX ME : We don't associate the context and the hardware here!*/
	//DEBUG_VERBOSE( gldSwapBuffers, "glrSwapBuffers(): We don't associate the context and the hardware here!\n" );
#if 0
    if(inContext->cmdTransportInfo) {
      diff = (FxU32)inContext->cmdTransportInfo->fifoPtr - (FxU32)lastFifoPtr;
      if(diff < 0) diff += inContext->cmdTransportInfo->fifoSize;
         glr_debug_printf("diff: %d\n",diff);
      lastFifoPtr = inContext->cmdTransportInfo->fifoPtr;
    }
#endif    
#if 0
    if(PerfMonAvailable()) 
    {
      FxU32 clocks, insns, l2misses, l2castouts;
      clocks = GetUPMC1();
      insns  = GetUPMC2();
      l2misses = GetUPMC3();
      l2castouts = GetUPMC4();
      
      if(nextEntry < 10000) {
      _c0[nextEntry] = clocks;
      _c1[nextEntry] = insns;
      _c2[nextEntry] = l2misses;
      _c3[nextEntry] = l2castouts;
      nextEntry++;
      }
      //glr_debug_printf("c: %d 2: %d 1: %d i: %d\n",clocks,insns,l2misses,l2castouts);
      
      SetPMC1(0);
      SetPMC2(0);
      SetPMC3(0);
      SetPMC4(0);
    }
#endif    
	if( !inContext ) {
		DEBUG_ERROR( gldSwapBuffers, "Invalid context pointer : 0x%08x\n", inContext);
		return;
	}
	
#if PROFILE
	{
		glr_get_keys();
		if(glr_check_for_key_press(73)){	// keypad equals
			ProfilerSetStatus(true);
		}

		if(glr_check_for_key_press(83)){	// keypad slash
			ProfilerSetStatus(false);
			ProfilerDump("\p3dfx OpenGL Profiler Data");
			ProfilerClear();
		}

	}
#endif

#if HOTKEYS
//#if 0
#define GR_AA_MULTI_SAMPLE      0x80000002
	{	
		static int frame_counter;
	
		frame_counter++;
		
		glr_get_keys();
		
#if 0
		if(glr_check_for_key(95)){		// clear/numlock
		
			if(glr_check_for_key_press(83)){	// keypad slash
				static int toggle;
				
				
				if(toggle){
					grEnable(GR_AA_MULTI_SAMPLE);
				} else {		
					grDisable(GR_AA_MULTI_SAMPLE);
				}		

				toggle = !toggle;
			}
			
			if(glr_check_for_key_press(73)){	// keypad equals
				char * pixels;
				GLint h, w;
				GLint buffer_size;
				
				h = inContext->viewPort.size.h;
				w = inContext->viewPort.size.w;
				
				buffer_size = w * h * 3;
				
				pixels = glmMalloc(buffer_size);
				glrReadPixels(
					inContext,
					0,
					0,
					w,
					h,
					GL_RGB,
					GL_UNSIGNED_BYTE,
					pixels,
					GL_TRUE);
					
				{
					// change from rgb to bgr
					int i;
					
					for(i = 0; i < buffer_size; i += 3){
						char c;
						
						c = pixels[i];
						pixels[i] = pixels[i + 2];
						pixels[i + 2] = c;
					}
				}
				{
					int f;
					static int counter;
					char filename[100];
					
					counter++;
					sprintf(filename, "screenshot%03d.tga", counter);
					f = creat(filename, 0);
					if(f != -1){
						unsigned char header[18];
						unsigned char footer[26] = "\0\0\0\0\0\0\0\0TRUEVISION-XFILE.\0";
						
						memset(header, 0, 18);
						
						header[0] = 0;	// id length
						header[1] = 0;	// color map type, none
						header[2] = 2;	// image type, uncompressed true color
						
						header[12] = w & 0xff;	// low byte of width
						header[13] = (w >> 8) & 0xff;	// high byte of width
					
						header[14] = h & 0xff;	// low byte of height
						header[15] = (h >> 8) & 0xff;	// high byte of height
						
						header[16] = 24;	// pixel depth
										
						write(f, (char *)header, 18);
						write(f, pixels, buffer_size);
						write(f, (char *)footer, 26);
						close(f);
					}
				}
				
				glmFree(pixels);
			}
		}
#else 
		
		if(glr_check_for_key_press(95)){	// clear / numlock
			static int toggle;
			    FxU32 pciInit0;		
			
			
			if(toggle){
				grEnable(GR_AA_MULTI_SAMPLE);
				//pciInit0 = __lwbrx((void *)0xa0000004,0);
				//pciInit0 |= (1UL << 18);
				//__stwbrx(pciInit0,(void *)0xa0000004,0);
			} else {
				grDisable(GR_AA_MULTI_SAMPLE);
				//pciInit0 = __lwbrx((void *)0xa0000004,0);
				//pciInit0 &= ~(1UL << 18);
				//__stwbrx(pciInit0,(void *)0xa0000004,0);
				//grBufferClear( 0, 0, 65535 );
			}		

			toggle = !toggle;
		}

#endif
	}
#endif
	
	if ( !inContext->renderSurface ) {
/*
		FxU32				theStatus;
		
		theStatus = grSstStatus();

		DEBUG_ENTRY_INPUT( gldSwapBuffers, "PCI FIFO free space (63=FIFO empty)          : %d\n", (theStatus & 0x0000003F));
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Vertical retrace (0=vertical retrace active) : %d\n", (theStatus & 0x00000040) >> 6);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Pixelfx graphics engine busy (0=engine idle) : %d\n", (theStatus & 0x00000080) >> 7);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "TMU busy (0=engine idle)                     : %d\n", (theStatus & 0x00000100) >> 8);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Voodoo Graphics busy (0=idle)                : %d\n", (theStatus & 0x00000200) >> 9);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Display buffer (0=buffer 0...)               : %d\n", (theStatus & 0x00000C00) >> 10);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Memory FIFO free space (65535=FIFO empty)    : %d\n", (theStatus & 0x0FFFF000) >> 12);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "Number of swap buffer commands pending       : %d\n", (theStatus & 0x70000000) >> 28);
		DEBUG_ENTRY_INPUT( gldSwapBuffers, "PCI interrupt generated (not implemented)    : %X\n", (theStatus & 0x80000000) >> 31);

#if 0
		if((status & 0x70000000) >> 28) {
			DEBUG_ERROR( glrSwapBuffers, "Swaps still pending, skipping this swap");
			return;
		}
#endif	
*/
	
		grBufferSwap(inContext->swap_interval);
	// Clear buffers
#if SHOW_OVERDRAW	
	grBufferClear( 0, 0, 65535 );
#endif	
		
	} else {
		GrVertex verts[4];
		if((*(*inContext->device)->gdPMap)->pixelSize == 16 && !inContext->glideExtensions.gls_pix_ext){
			grDisableAllEffects();
			grDitherMode(GR_DITHER_DISABLE);
			inContext->combineUnitState = 0;
			grColorCombine(
				GR_COMBINE_FUNCTION_LOCAL, 
				GR_COMBINE_FACTOR_NONE,
				GR_COMBINE_LOCAL_ITERATED, 
				GR_COMBINE_OTHER_NONE, 
				FXFALSE
				);
				
			grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
			grAlphaTestFunction(GR_CMP_ALWAYS);
				
			verts[0].x = 0;
			verts[0].y = 0;
			verts[0].pargb = 0xff7f7fff;
			
			verts[1].x = inContext->renderPort->portRect.right;
			verts[1].y = 0;
			verts[1].pargb = 0xff7f7fff;
			
			verts[2].x = inContext->renderPort->portRect.right;
			verts[2].y = inContext->renderPort->portRect.bottom;
			verts[2].pargb = 0xff7f7fff;
			
			verts[3].x = 0;
			verts[3].y = inContext->renderPort->portRect.bottom;
			verts[3].pargb = 0xff7f7fff;
			
			grDrawTriangle(verts, verts + 1, verts + 2);
			grDrawTriangle(verts, verts + 2, verts + 3);			
		}
		
		
		/* we are rendering in a windowed or offscreen PixMap, we need to blit */
		grFinish();
		{
		  GrafPtr currentPort;
		  Rect srcSwapRect, dstSwapRect;
		  
		  GetPort(&currentPort);
		  SetPort((GrafPtr)inContext->targetPort);
		  
		  srcSwapRect = inContext->renderPort->portRect;
		  dstSwapRect = inContext->targetPort->portRect;
          
          /* If buffer rect is enabled, then override target rect */
          if(inContext->buffer_rect_enabled) {
            dstSwapRect.left += inContext->window_rect.x;
            dstSwapRect.right = dstSwapRect.left + inContext->window_rect.w;
            dstSwapRect.top += inContext->window_rect.y;
            dstSwapRect.bottom = dstSwapRect.top + inContext->window_rect.h;
            
          }		  

          /* If swap rect is enabled, then we have to fix up both the src
             and destination rects. */
          /* Note: Swap rect coordinates have their origin at the lower left
             corner (OpenGL coordinates) */
          if(inContext->swap_rect_enabled) {
            srcSwapRect.left = inContext->swap_rect.x;
            srcSwapRect.top  = srcSwapRect.bottom - (inContext->swap_rect.y + inContext->swap_rect.h); 
            
            srcSwapRect.right = srcSwapRect.left + inContext->swap_rect.w;
            srcSwapRect.bottom = srcSwapRect.top + inContext->swap_rect.w;
            
            /* Okay, clamp it to the rendering surface */
            if(srcSwapRect.left < 0) srcSwapRect.left = 0;
            if(srcSwapRect.top < 0) srcSwapRect.top = 0;
            if(srcSwapRect.right > inContext->renderPort->portRect.right)
              srcSwapRect.right = inContext->renderPort->portRect.right;
            if(srcSwapRect.bottom > inContext->renderPort->portRect.bottom)
              srcSwapRect.bottom = inContext->renderPort->portRect.bottom;
            
            /* Whew.  Okay, now we have to take the swap rect info and update the 
               destination rect. */
            dstSwapRect.left += srcSwapRect.left;
            dstSwapRect.right = dstSwapRect.left + (srcSwapRect.right - srcSwapRect.left);
            dstSwapRect.top  += srcSwapRect.top;
            dstSwapRect.bottom = dstSwapRect.top + (srcSwapRect.bottom - srcSwapRect.top);

            /* At this point QuickDraw should deal with clipping the destination swap
               rect to the window's port rect (I hope) */      
          }                
		  CopyBits(	&((GrafPtr)inContext->renderPort)->portBits,
					&((GrafPtr)inContext->targetPort)->portBits,
					&srcSwapRect,
					&dstSwapRect,
					srcCopy | ditherCopy,0);
		  SetPort(currentPort);
		}			

		if((*(*inContext->device)->gdPMap)->pixelSize == 16 && !inContext->glideExtensions.gls_pix_ext){
			grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_SRC_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO);
				
			verts[0].x = 0;
			verts[0].y = 0;
			verts[0].pargb = 0xffffff7f;
			
			verts[1].x = inContext->renderPort->portRect.right;
			verts[1].y = 0;
			verts[1].pargb = 0xffffff7f;
			
			verts[2].x = inContext->renderPort->portRect.right;
			verts[2].y = inContext->renderPort->portRect.bottom;
			verts[2].pargb = 0xffffff7f;
			
			verts[3].x = 0;
			verts[3].y = inContext->renderPort->portRect.bottom;
			verts[3].pargb = 0xffffff7f;
			
			grDrawTriangle(verts, verts + 1, verts + 2);
			grDrawTriangle(verts, verts + 2, verts + 3);
			
			grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
			glrUpdateHardwareState(inContext, GLD_STATE_ALL);
			
		}

		DEBUG_VERBOSE( gldSwapBuffers, "Blitting from the rendering surface to the target GrafPort...\n");
		DEBUG_VERBOSE( gldSwapBuffers, "renderPort rect : %d, %d, %d, %d\n", inContext->renderPort->portRect.top, inContext->renderPort->portRect.left, inContext->renderPort->portRect.bottom, inContext->renderPort->portRect.right );
		DEBUG_VERBOSE( gldSwapBuffers, "targetPort rect : %d, %d, %d, %d\n", inContext->targetPort->portRect.top, inContext->targetPort->portRect.left, inContext->targetPort->portRect.bottom, inContext->targetPort->portRect.right );
	}
}



/*
________________________________________________________________________________________

      gldFinish
________________________________________________________________________________________

*/

void gldFinish(
	GLDContext			inContext)
{
	DEBUG_ENTRY( gldFinish );
	
	grFinish();

	if ( inContext->renderSurface  ) {
		if(inContext->gliPixelFormat.buffer_mode & GLI_DOUBLEBUFFER_BIT){
			switch(inContext->state->color_buffer.draw){
			case GL_FRONT:
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
				gldSwapBuffers( inContext );			
			}
		
		} else {
			/* we are rendering in a windowed or offscreen PixMap, we need to blit */
			gldSwapBuffers( inContext );
		}
	}
}


/*
________________________________________________________________________________________

      gldFlush
________________________________________________________________________________________

*/

void gldFlush(
	GLDContext			inContext)
{
	DEBUG_ENTRY( gldFlush );
	
	grFlush();
}


/*
________________________________________________________________________________________

      gldAccum
________________________________________________________________________________________

*/

void gldAccum(
	GLDContext			inContext,
	GLenum				inOp,
	GLfloat				inValue)
{
	DEBUG_NOT_YET_IMPLEMENTED( gldAccum );
}