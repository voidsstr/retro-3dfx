/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 4$
** $Date: 10/11/00 7:50:27 PM$
*/
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "context.h"
#include "render.h"
#include "global.h"

#define EXPMASK 0x7ff00000

#include "ras_og.h"

/* long aligned */
void FastMemClear4(__GLcontext* gc,
		   unsigned long *lptr,
		   int w,
		   int h,
		   int skip,
		   unsigned long val)
{ 
  if (__glIsMmx()) {
      static GLboolean cold = GL_TRUE;
      static unsigned char clear_code[500];
      static void* klank = clear_code;

      if (cold) {
	  cold = GL_FALSE;
	  __glGenerateMMXFill(gc, clear_code);
      }

      __asm mov     ecx,w
      __asm imul    ecx,h
      __asm test    ecx,ecx
      __asm je      done

      __asm mov     esi,skip
      __asm mov     edi,lptr
      __asm mov     eax,val

      __asm test    esi,esi
      __asm jne     itskips

      __asm call    klank
      __asm jmp     done

      itskips:
      __asm mov     edx,h

      skipper:
      __asm mov     ecx,w
      __asm call    klank
      __asm add     edi,esi
      __asm dec     edx
      __asm jnz     skipper

      done:

      /* Need to restore fp stack to something sane */

      __asm __emit  0x0f	/* EMMS */
      __asm __emit  0x77

      return;
  }
  /* Assuming that w and h are non-negative */
  if((val == 0) || (((val & EXPMASK) <= 0x7fc00000) && ((val & EXPMASK) >= 0x00100000))) {
     if(skip == 0) {
           /* One contiguous array */
           unsigned int count = w*h;
           if(count == 0) return; 

           __asm mov eax, val          ; EAX holds value to write
           __asm mov edi, lptr         ; EDI points to start of block to be written
           __asm mov ecx, count        ; ECX holds count number of DWORDS to write   
           __asm test edi, 0x4         ; Is block on an 8 byte boundary?
           __asm push eax              ; Initialize QWORD
           __asm push eax
           __asm je skip1              ;   Skip if yes

           __asm mov [edi], eax        ;   Otherwise, write out first DWORD
           __asm add edi, 4            ;       Advance block pointer
           __asm dec ecx               ;       Decrement count of words to write
   skip1:
           __asm mov edx, ecx          ; Get a copy of the (possibly adjusted) count
           __asm fld QWORD PTR [esp]   ; Load QWORD into FP Top of Stack
           __asm shr ecx, 1            ; Divide count by 2 (writing QWORDS)
           __asm je skip2              ; Skip loop if nothing more to do
   loop1:   
           __asm fst QWORD PTR [edi]   ; Write out QWORD value to block
           __asm add edi, 8            ; Increment block pointer by sizeof(QWORD)
           __asm dec ecx               ; Decrement count
           __asm jne loop1             ; Loop if more to do
   skip2:
           __asm test edx, 1           ; Was the adjusted count odd?
           __asm je skip3              ; Skip if not

           __asm mov [edi], eax        ; Otherwise, write out last value
   skip3:
           __asm fstp QWORD PTR [esp]  ; Pop value off of FP Top of Stack
           __asm pop eax               ; Clean up stack
           __asm pop eax

      } else {
           /* Non-contiguous array */
           __asm mov eax, val          ; EAX holds value to write
           __asm mov edi, lptr         ; EDI points to start of span to be written
           __asm mov esi, skip         ; Get the span pad (in bytes)
           __asm mov edx, h            ; EDX holds number of spans to write
           __asm push eax              ; Initialize QWORD
           __asm push eax
           __asm fld QWORD PTR [esp]   ; Load QWORD into FP Top of Stack
   loopout:
           __asm mov ecx, w            ; ECX holds count number of DWORDS to write
           __asm test edi, 0x4         ; Is span on an 8 byte boundary?
           __asm je skip4              ; Skip if yes

           __asm mov [edi], eax        ; Otherwise, write out first DWORD
           __asm dec ecx               ; Decrement count of words to write
           __asm add edi, 4            ; Advance span pointer
   skip4:
           __asm mov ebx, ecx          ; Get a copy of the (possibly adjusted) count
           __asm shr ecx, 1            ; Divide count by 2 (writing QWORDS)
           __asm je skip5              ; Skip loop if nothing more to do
   loopin:  
           __asm fst QWORD PTR [edi]   ; Write out QWORD value to block
           __asm add edi, 8            ; Increment span pointer by sizeof(QWORD)
           __asm dec ecx               ; Decrement count
           __asm jne loopin            ; Loop if more to do
   skip5:
           __asm test ebx, 1           ; Was the adjusted count odd?
           __asm je skip6              ; Skip if not

           __asm mov [edi], eax        ; Otherwise, write out last value
           __asm add edi, 4            ; Advance span pointer
   skip6:
           __asm add edi, esi          ; Advance span pointer 
           __asm dec edx               ; Decrement span count 
           __asm jne loopout           ; Loop if more to do

           __asm fstp QWORD PTR [esp]  ; Pop value off of FP Top of Stack
           __asm pop eax               ; Clean up stack
           __asm pop eax
      }
   } else {
      /* Exponent would create a "bad" double value, have to use slower blt method */
      if(skip == 0) {
          unsigned int count = w*h; 
          /* One contiguous array */
          __asm mov edi, lptr          ; EDI points to start of block to be written
          __asm mov eax, val           ; EAX holds value to write
          __asm mov ecx, count         ; ECX holds numer of locations to be written
//          __asm mov ebx, [edi]         ; Pre-read in cache line
          __asm and ecx, 0x7           ; Convert DWORD count to intra-line count
          __asm je skip11              ; Skip intra-line loop if count is cache multiple

   loop10:
          __asm mov [edi], eax         ; Write out value
          __asm add edi, 4             ; Advance pointer
          __asm dec ecx                ; Decrement count
          __asm jne loop10             ; Loop if more to be done
   skip11:      
          __asm mov ecx, count         ; Reload DWORD count 
          __asm shr ecx, 3             ; Convert to cache line count (count/8)
          __asm je quit1               ; If no cache lines we are finished
          
   loop11:
//          __asm mov ebx, [edi+32]      ; Pre-cache next cache line
          __asm mov [edi+ 4], eax      ; Write out values (avoid first bank conflict)
          __asm mov [edi+ 0], eax
          __asm mov [edi+ 8], eax
          __asm mov [edi+12], eax
          __asm mov [edi+16], eax
          __asm mov [edi+20], eax
          __asm mov [edi+24], eax
          __asm mov [edi+28], eax
          __asm add edi, 32            ; Advance pointer 
          __asm dec ecx                ; Decrement count
          __asm jne loop11             ; Loop if more to be done
    quit1: ;        
      } else { 
          /* Non-contiguous array */
           __asm mov eax, val          ; EAX holds value to write
           __asm mov edi, lptr         ; EDI points to start of span to be written
           __asm mov esi, skip         ; Get the span pad
           __asm mov edx, h            ; EDX holds number of spans to write
 loopout1:
           __asm mov ecx, w            ; ECX holds count number of DWORDS to write
//           __asm mov ebx, [edi]        ; Pre-read in cache line
           __asm and ecx, 0x7          ; Convert DWORD count to intra-line count
           __asm je skip21             ; Skip intra-line loop if count is cache multiple
        
   loop20:
           __asm mov [edi], eax        ; Write out value
           __asm add edi,4             ; Advance pointer
           __asm dec ecx               ; Decrement count
           __asm jne loop20            ; Loop if more to be done
   skip21:
           __asm mov ecx, w            ; Reload DWORD count
           __asm shr ecx, 3            ; Convert to cache line count (count/8)
           __asm je skip31             ; If no cache lines we are finished with this span
   loop21:
//          __asm mov ebx, [edi+32]      ; Pre-cache next cache line
          __asm mov [edi+ 4], eax      ; Write out values (avoid first bank conflict)
          __asm mov [edi+ 0], eax
          __asm mov [edi+ 8], eax
          __asm mov [edi+12], eax
          __asm mov [edi+16], eax
          __asm mov [edi+20], eax
          __asm mov [edi+24], eax
          __asm mov [edi+28], eax
          __asm add edi, 32            ; Advance pointer 
          __asm dec ecx                ; Decrement count
          __asm jne loop21             ; Loop if more to be done
        
   skip31:
          __asm add edi, esi           ; Skip to next span
          __asm dec edx                ; Decrement span count
          __asm jne loopout1           ; Loop if more to do
      }
   }
}
        

/* short aligned */
void FastMemClear2(__GLcontext *gc,
		   unsigned short *sptr, int w, int h, 
                   int skip, unsigned long val)
{
    if(skip == 0) {
        /* Contiguous array */
        int count = w*h;

        /* Trim off leading short if address is not 4 byte aligned */
        if(((unsigned long) sptr)&2) {
            *sptr++ = (unsigned short) val;
            count--;
        }

        /* Trim off trailing short if count is not 4 byte aligned */
        if(count&1) {
            *(sptr + count - 1) = (unsigned short) val;
            count--;
        }

        if(count > 0) {
            FastMemClear4(gc,
			  (unsigned long *)sptr, count>>1, 1, 0, 
                          (val&0xffff) | ((val&0xffff)<<16));
        }

    } else {
        /* Non-contiguous array */

        /* Check to see if we need to trim off leading column. */
        if(((unsigned long) sptr)&2) {
            int count = h;
            unsigned short *ptr = sptr;
            unsigned int pitch = (w<<1) + skip;

            do {
                *ptr = (unsigned short) val;
                ((unsigned char *)ptr) += pitch;
            } while(--count);
            sptr++;
            skip += 2;
            w--;
        }

        /* Check to see if we need to trim off trailing column. */
        if(w&1) {
            int count = h;
            unsigned short *ptr = sptr + (w - 1);
            unsigned int pitch = (w<<1) + skip;

            do {
                *ptr = (unsigned short)val;
                ((unsigned char *)ptr) += pitch;
            } while (--count);
            skip += 2;
            w--;
        }

        /* If anything is left, we have clear 4 byte aligned rows now */
        if(w > 0) {
            FastMemClear4(gc,
			  (unsigned long *)sptr, w>>1, h, skip, 
                          (val&0xffff) | ((val&0xffff)<<16));

        }
    }

}


//static unsigned long arrayMask[4] = {0, 0x00ffffff, 0x0000ffff, 0x000000ff};
static unsigned long arrayMask[4] = {0, 0x000000ff, 0x0000ffff, 0x00ffffff};

/* char  aligned */
void FastMemClear1(__GLcontext *gc,
		   unsigned char  *cptr, int w, int h, 
                   int skip, unsigned long val)
{
    unsigned long val4 = (val&0xff) | ((val&0xff)<<8) |
                         ((val&0xff) << 16) | ((val&0xff) << 24);

    if(skip == 0) {
        /* Contiguous array */
        int count = w*h;
        int tmpcnt;

        if(count > 3) {
            /* Leading, trailing masks cannot intersect */

            /* Check to see if we need to trim leading bytes. */
            if(tmpcnt = (((unsigned long) cptr)&3)) {
                unsigned long arrayM = arrayMask[tmpcnt];
                unsigned long *ptr = (unsigned long *)(((unsigned int) cptr) & (~3));

                *ptr = ((*ptr)&arrayM) | ((~arrayM) & val4);
                tmpcnt = 4 - tmpcnt;
                cptr += tmpcnt;
                count -= tmpcnt;
            }

            /* Check to see if we need to trim trailing bytes. */
            if(tmpcnt = (count&3)) {
                unsigned long arrayM = ~arrayMask[tmpcnt];
                unsigned long *ptr = (unsigned long *)(cptr + count - tmpcnt);

                *ptr = ((*ptr)&arrayM) | ((~arrayM) & val4);
                count -= tmpcnt;
            }

            if(count > 0) {
                FastMemClear4(gc, (unsigned long *)cptr, count>>2, 1, 0, val4);
            }
        } else {
            /* Leading, trailing masks might intersect */

            /* Just do simple loop */
            do {
                *cptr++ = (unsigned char)val;
            } while(--count);
        }

    } else {
        /* Non-contiguous array */
        unsigned int tmpcnt;
    
        if(w > 3) {
            /* Leading, trailing masks cannot intersect */

            /* Check to see if we need to trim off leading columns */
            if(tmpcnt = (((unsigned int) cptr)&3)) {
                unsigned long arrayM = arrayMask[tmpcnt];
                unsigned long mval = ((~arrayM) & val4);
                unsigned long *ptr = (unsigned long *)(((unsigned int) cptr) & (~3));
                int count = h;
                unsigned int pitch = w + skip;
            
                do {
                    *ptr = ((*ptr)&arrayM) | mval;
                    ((unsigned char *)ptr) += pitch;
                } while(--count);

                tmpcnt = 4 - tmpcnt;
                cptr += tmpcnt;
                w -= tmpcnt;
                skip += tmpcnt;
            }

            /* Check to see if we need to trim off trailing columns */
            if(tmpcnt = (w & 3)) {
                unsigned long arrayM = ~arrayMask[tmpcnt];
                unsigned long mval = ((~arrayM) & val4);
                unsigned long *ptr = (unsigned long *)(cptr + w - tmpcnt);
                int count = h;
                unsigned int pitch = w + skip;

                w -= tmpcnt;
                skip += tmpcnt;
                do {
                    *ptr = ((*ptr)&arrayM) | mval;
                    ((unsigned char *)ptr) += pitch;
                } while(--count);
            }

            if(w > 0) {
                FastMemClear4(gc, (unsigned long *)cptr, w>>2, h, skip, val4);
            }
        } else {
            /* Leading, trailing masks might intersect */
            int count;

            /* Just do loop */
            do {
                count = w;
                do {
                    *cptr++ = (unsigned char)val;
                } while(--count);
                cptr += skip;
            } while(--h);
        }
    }
}
