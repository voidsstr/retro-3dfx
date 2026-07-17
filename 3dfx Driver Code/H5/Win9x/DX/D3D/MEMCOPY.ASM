IFE @Version GE 614
;; Include Katmai New Instruction macro definitions
INCLUDE iaxmm.inc
ENDIF

	TITLE	KNImemcp
	.686P
	.mmx
IF @Version GE 614
        .xmm
ENDIF

.model FLAT

PUBLIC  _KNI_movq_memcpy

_DATA	SEGMENT
	
block_size		 dd 01000h
block_size_plus_1	 dd 01001h
	
_DATA	ENDS


	
_TEXT	SEGMENT
	.code

srcPtr  = 4+16
dstPtr  = 8+16
nDwords = 12+16
temp    = 16+16
	
;***************************************************************************/
;*
;*  Function: _KNI_movq_memcpy
;*     copy <nDwords> dwords from <srcPtr> to <dstPtr>
;*	assumes dstPtr is already 32-byte aligned
;*
;***************************************************************************/

;	const int block_size = 0x1000; //4KBytes, the page size
;	const int block_size_plus_1 = block_size + 1;
	

align 16	
_KNI_movq_memcpy PROC NEAR

	push ebx		; vc optimzer assumes this is callee-preserved
	push esi		; vc optimzer assumes this is callee-preserved
	push edi		; vc optimzer assumes this is callee-preserved
	push ebp		; vc optimzer assumes this is callee-preserved
	
	mov esi, srcPtr [esp]		; esi <- srcPtr
	mov edi, dstPtr [esp]		; edi <- dstPtr
	mov eax, nDwords [esp]
	shl eax, 2			; eax <- nBytes to copy
	
	;; Copy 8 bytes to prevent reading unallocated memory at the 
	;; beginning of the source in the inner loop
	;; 
	movq	mm0,	[esi]
	sub	eax,	8		;eax <- count - 8
	add	esi,	8		;esi <- src + 8
	movq	[edi],	mm0
	add	edi,	8		;edi <- dst + 8

	;; align desitnation to 32 bytes
	;;	
	movups	xmm0,	[esi]
	movups	xmm1,	[esi+16]
	mov	ebx,	edi
	mov	edx,	020h
	and	ebx,	01fh
	sub	edx,	ebx		;edx <- offset to align destination
	add	esi,	edx		;add offset to source, 
					; source may still be unaligned
					;esi <- src + 8 + dst alignment
	movups	[edi],	xmm0
	movups	[edi+16],xmm1
	add	edi,	edx		;add offset to destination,
					;edi <- dst + 8 + dst alignment
	sub	eax,	edx		;eax <- count - 8 - dst alignment

	mov	ecx,	0FFFFFFE0h
	sub	eax,	8		;prevents reading unallocated memory 
					;at the end of the source in inner loop
	and	ecx,	eax		;will handle remaining 0-39 elements
					; after the MAINLOOP
	movd	mm5,	ecx
		
	;; This implementation of block move avoids cache line splits by 
	;; replacing the load that spans a cache line with two 8-byte 
	;; aligned loads that are shifted and ORed together.  To use this 
	;; implementation three offsets are needed in the inner loop:
	;; *** movq_offset			= (0x10 + esi) & 0x18
	;; *** avoid_split_offset		= 0x18 - (esi & 0x1f)
	;; *** 16_byte_offset			= esi & 0x10

	;; Get the movq offset
	;; 
	mov	ebx, esi
	add	ebx, 010h
	and	ebx, 018h		;ebx = (0x10 + esi) & 0x18

	;; Get the Right and Left Shifters in order to avoid the cache 
	;; line split
	;; 
	mov	eax,	7
	and	eax,	esi		;misalignment		
	movd	mm3,	eax
	shl	eax,	3
	movd	mm7,	eax		;Right Shifter:misalignment*8
	mov	eax,	040h
	movd	mm6,	eax
	psubd	mm6,	mm7		;Left Shifter:	64 - misalignment*8
		
	;; Get the avoid_split_offset
	;; 
	mov	edx,	018h
	mov	eax,	esi
	and	eax,	01fh
	sub	edx,	eax		;edx = 0x18 - esi&0x1f
	movd	mm4,	edx
	paddd	mm3,	mm4		;Used to store the data that spans 
					; the cacheline
MAINLOOP:
	movd	eax,	mm5
	mov	ecx,	eax
	cmp	eax,	block_size
	cmova	eax,	block_size
	cmova	ecx,	block_size_plus_1

	;; touch the last element or the next page to update the TLB
	;; 
	movzx	ecx,	byte ptr [esi][ecx-1]		

	movd	mm4,	eax
	psubd	mm5,	mm4		;update count
	
	;; preloop to fetch block to L1
	;; 
	mov	ecx,	0
	
PREFETCHLOOP:
	prefetchnta	[esi][ecx]
	add	ecx,	32	
	cmp	ecx,	eax
	jne	PREFETCHLOOP

	;;  copy the block
	;;
	add	eax,	esi		;Used in comparison to exit the block
	mov	ecx,	esi		; copy loop
	and	ecx,	010h		;16_byte_offset, the 128-bit load/
					; store (movups/movntps)
	movd	ebp,	mm3		;offset to store the data that spanned
					; cacheline
BLOCKCOPY:
	;; Copy 32 bytes per iteration
	;; 
	movq	mm0,	[esi][edx]
	psrlq	mm0,	mm7
	movq	mm2,	[esi][ebx]
	movq	mm1,	[esi][edx+8]
	movups	xmm3,	[esi][ecx]
	psllq	mm1,	mm6
	por	mm0,	mm1	       ;same 8-byte data that spanned cacheline
	movntq	[edi][ebx], mm2
	movntq	[edi][ebp], mm0
	movntps	[edi][ecx], xmm3
	add	esi,	32
	add	edi,	32
	cmp	esi,	eax
	jne	BLOCKCOPY		; END OF BLOCK COPY
	
	movd	ecx,	mm5
	test	ecx,	ecx		;if more to copy, jmp to MainLoop
	jnz	MAINLOOP		; END OF MAIN LOOP

	;; LAST_40:
	;; Handle remaining elements by Copying the last 40 bytes
	;;
	
	;; make sure all writes finish before return
	;; 
	;;sfence		not needed for the AGP cmdfifo, we fence before
	;;		bumping anyway -KMW
	;; 
		
	;; last40src pts to the last 40 src bytes
	;; float *last40src = (float *)(sPtr + count - 40);
	;;
	;; last40dest points to the last 40 dst bytes
	;; float *last40dest = (float *)(dPtr + count - 40);
	;; 

	mov	ecx,	nDwords [esp]
	shl	ecx,	2
	sub	ecx,	40		;ecx <- (countInBytes - 40)
	mov	esi,	srcPtr [esp]
	add	esi,	ecx		;esi <- last40src
	mov	edi,	dstPtr [esp]
	add	edi,	ecx		;edi <- last40dst
	movups	xmm0,	[esi] 		;source may be unaligned
	movups	xmm1,	[esi+16]
	movq	mm2,	[esi+32]
	movups	[edi],	xmm0  		;destination may be unaligned
	movups	[edi+16], xmm1
	movq	[edi+32], mm2
DONE:
	emms

	pop ebp			; restore preserved register
	pop edi			; restore preserved register
	pop esi			; restore preserved register
	pop ebx			; restore preserved register	

	ret
_KNI_movq_memcpy ENDP
				
				
_TEXT ENDS
      END
