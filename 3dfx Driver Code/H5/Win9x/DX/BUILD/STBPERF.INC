;/*
;-----------------------------Module-Header-----------------------------;
; Module Name:  STBPERF.INC
;
; This file contains optimization definitions used by the STB build of the
; Voodoo3 source.
;
; Created: 12/15/97
; Author:  Steve Rogers
;
; History:  Who	What?
; 12/15/98  SR  Created
; 12/16/98  sr  Added Handle1to1Str
; 12/16/98  sr  Added Handle1X1
; 12/16/98  sr  Added HandleSTR1X1
; 01/11/99  pz  Added STB_MMGR_TWK
; 01/13/99  sr  Added STB 16BIT Textout function from bj
; 03/08/99  mgm Comment out SKIP_STAGE2 to fix PRS 4898 (mgm=michael@3dfx)
;-----------------------------------------------------------------------;
;

; Define Assembly performance optimizations here
; Name                     Description
; Example
; STBPERF_FASTRECIP    EQU 1  ; SR Uses table lookup for a faster, less acurate reciprocal
STBPERF_HANDLE1X1      EQU 1  ; SR Pokes pixel through framebuffer for 1 pixel bitmap copies
STBPERF_HANDLESTR1X1   EQU 1  ; SR Pokes pixel through framebuffer for 1 pixel bitmap 1 to 1 stretches
STB_FIFO16_ENABLE	   EQU 1  ; BJ Enable/Allow 16 bit FIFO code to be used
STBPERF_CMBITBLT       EQU 1  ; PZ Special case for color to mono blit
STBPERF_DEVBITFIX      EQU 1  ; PZ A replacement scheme to work around a GDI bug in device bmp management
ifdef STBPERF_DEVBITFIX
;  SKIP_STAGE2            EQU 1  ; PZ Do SKIP_STAGE2 only if STBPERF_DEVBITFIX is enabled
endif
ifdef STB_FIFO16_ENABLE
	STB_16BIT_TEXTOUT  EQU 1  ; BJ Use 16 bit FIFO CODE in ExtTextOut
	STBPERF_POLYLINE16 EQU 1  ; RVB 16-bit code for Output::OS_POLYLINE
    STB_16BIT_BITBLT   EQU 1  ; BJ Use 16 bit FIFO CODE in BitBlt (PatCopy, Whiteness, Blackness)
    STB_FASTER_TR      EQU 1  ; BJ No need to have 2 segment loads to check TR
    PERF_NEW16BITBLT   EQU 1  ; SteveH - optimized 16-bit BitBlt 
    PERF_NEWMM         EQU 1  ; SteveH - optimized 2D memory manager
endif
ifdef PERF_NEWMM
    SKIP_STAGE2        EQU 1  ; SteveH - if we're using the new MM, no need to do the hostify code
endif
STBPERF_K6_ASM_AUTOSTRIP EQU 1    ; turn on auto-stripping in atrik.asm

comment `

;*/

// Define C performance optimizations here
//#pragma message("Compiling with Performance optimizations in ../build/STBPERF.INC.")
// #define Name                  Description
// Example
// #define STBPERF_FASTRECIP 1      ; SR Uses table lookup for a faster, less acurate reciprocal
#define STBPERF_HANDLE1TO1STR 1     ; SR Some StretchBlts are called with 1 to 1 stretches, so copy them.
#define STB_FIFO16_ENABLE 1         ; BJ Enable/Allow 16 bit FIFO code to be used
#define STB_16BIT_BITBLT 1          ; BJ Enable/Allow 16 bit PatCopy cases to be done in assembler
#define STBPERF_CMBITBLT            ; PZ Special case for color to mono blit
#define STBPERF_SOLIDCLRPATCPY      ; PZ Special case for patcopy with solid color brush 
//#define STBPERF_RBO_SKIPDIBENGINE 1 ; PZ Skip calling to DIBENGIE when realizing bitmaps
#define STBPERF_DEVBITFIX 1         ; PZ A replacement scheme to work around a GDI bug in device bmp management
#ifdef STBPERF_DEVBITFIX
// #define SKIP_STAGE2 1            ; PZ Do SKIP_STAGE2 only if STBPERF_DEVBITFIX is enabled
#endif
#define PERF_NEWDIB 1               ; SteveH - Handle all ROPs and 4bpp sources in DIB functions
#define PERF_NEW16BITBLT 1
#define PERF_NEWMM 1                
#ifdef PERF_NEWMM
    #define SKIP_STAGE2 1
#endif    

//#define STBPERF_2DAGPCMDFIFO        ; SR Allows 2D Cmd Fifo to reside in AGP Memory.
//                                    ; To get AGP to work, you must set two registry entries
//									  ; "aFifo" must be set to 1, and "force1xrate" must be set to 1
//									  ; both are DWORDs under 
//									  ; HKLM/System/CurrentControlSet/Services/Class/Display/XXXX/DEFAULT
#define STBPERF_USE_FLIPNOVSYNC       ; enable FLIPNOVSYNC for WB 2000 mls 7/20/99
#define STBPERF_FAST_TEXTURE_DOWNLOAD ; forces explicit rep movsd loop for non-kni texture download: mls 7/30/99
#define C_AUTOSTRIP                   ; turn on the auto stripping code in the C files
#define PERF_USE_255_LOOKUP_TABLE     ; turn on int to float translation by lookup table instead of fpu conversion
//#define PERF_TXTR_LIST_STATIC_CNTR  ; use a static list index in the texture list instead of starting from 1 each time

// Changes made by Bob S. just prior to departure - 6/29/2000
#define TEXTURE_REPEAT_NOT_SCALED     ; Sets dwMaxTextureRepeat to 256 and sets the D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE
//                                    ; bit for lines and triangles. Per Gary Tarolli.
//                                    ;
#define COLUMNBANDCONTROL_DEFAULT     ; Change the default for CBC to 8 (register value of zero) to be in synch with Glide/Ogl.
//                                    ; Prior default was 32 (register value of 2).
//                                    ;
#define MAXPENDINGBUFFERS             ; Allow pending buffer max to be controlled via the registry.
//                                    ;
#define LODDITHER                     ; Allow Lod Dithering to be controlled via the registry.
//                                    ;
#define LODBIASPERCHIP                ; Allow per-chip Lod Bias values, if multitexturing,
//                                    ; trilinear filtering and > 1 chip. This also requires
//                                    ; LODDITHER code.
//									  ;
#define SLI_ABOVE_1280				  ; Allow SLI operation via the registry at modes 1280 and greater.
//									  ;	Previously we reverted to single chip mode at these
//									  ;	high resolutions due to visual artifacts

;/*
;end comment `
;*/
