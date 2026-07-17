;/*
;// stbperf.inc
;// STB optimizations for Napalm\Voodoo3\BlackMagic 6X00

K6_2        EQU 1   ; Enable the 3DNow optimizations - IMPORTANT comment this line out
                    ; to disable 3DNow code, equating to 0 will not work
STBKNI      EQU 1   ; Enable the STB Katmai KNI optimizations
NEWASMTRI   EQU 1   ; New 3DNow and KNI Assembler Triangle Rendering routines

IFNDEF NEWASMTRI
NEWASMTRI   EQU 0
ENDIF

IFNDEF STBKNI
STBKNI      EQU 0
ENDIF

NEW_CCU     EQU 1

comment `

;*/

//#define PERF_COPY_BITS_OPT	1	// handle trivial 8bpp to 32bpp, 8bpp to 16bpp, and 8bpp to 24bpp copybits instead of punting
#define PERF_ASM_XLATE		1	// speed up the 8 to 16 and 8 to 32 translations

#if (_WIN32_WINNT < 0x0500)
#define PERF_DISABLE_CANBLTSYSMEM 1	// D3D HEL hangs when system to video memory blts are enabled
#endif

#define STBPERF_USE_FLIPNOVSYNC         // enable FLIPNOVSYNC for WB 2000 mls 7/20/99
//#define STBPERF_FAST_TEXTURE_DOWNLOAD // forces explicit rep movsd loop for non-kni texture download: mls 7/30/99
//#define C_AUTOSTRIP                   // turn on the auto stripping code in the C files
//#define PERF_USE_255_LOOKUP_TABLE     // turn on int to float translation by lookup table instead of fpu conversion

#define K6_2        1   // Enable the 3DNow optimizations- IMPORTANT comment this line out
                        // to disable 3DNow code, equating to 0 will not work
#define STBKNI      1   // Enable the STB Katmai KNI optimizations
#define NEWASMTRI   1   // New 3DNow and KNI Assembler Triangle Rendering routines

#ifndef NEWASMTRI
#define NEWASMTRI   0
#endif

#ifndef STBKNI
#define STBKNI      0
#endif

#define NEW_CCU     1

// end perf.inc

;/*
;end comment `
;*/
