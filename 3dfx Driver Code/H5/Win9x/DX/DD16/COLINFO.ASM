;/* $Header: /devel/h3/Win95/dx/dd16/colinfo.asm 3     12/28/98 11:22a Michael $ */
;/*
;** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
;** All Rights Reserved.
;**
;** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
;** the contents of this file may not be disclosed to third parties, copied or
;** duplicated in any form, in whole or in part, without the prior written
;** permission of 3Dfx Interactive, Inc.
;**
;** RESTRICTED RIGHTS LEGEND:
;** Use, duplication or disclosure by the Government is subject to restrictions
;** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
;** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
;** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
;** rights reserved under the Copyright Laws of the United States.
;**
;** File name:   colinfo.asm
;**
;** Description: Handles the ColorInfo display driver export function.
;**
;** $Revision: 3 $
;** $Date: 12/28/98 11:22a $
;**
;** $History: colinfo.asm $
;; 
;; *****************  Version 3  *****************
;; User: Michael      Date: 12/28/98   Time: 11:22a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;**
;*/

        option OLDSTRUCTS
        .xlist
        include dibeng.inc
        .list

.586p

_TEXT segment WORD PUBLIC 'CODE' use16

DIB_ColorInfo PROTO FAR PASCAL

; This table contains 5 bit values for use in returning the nearest physical
; match to the 24 bit logical value, we use this to quickly replicate what the
; DIB_ColorInfo function would have returned.

ByteTo5BitLookup label byte
    db  000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
    db  001h, 001h, 001h, 001h, 001h, 001h, 001h, 001h  ;0c thru 13
    db  002h, 002h, 002h, 002h, 002h, 002h, 002h, 002h  ;14 thru 1b
    db  003h, 003h, 003h, 003h, 003h, 003h, 003h, 003h  ;1c thru 23
    db  004h, 004h, 004h, 004h, 004h, 004h, 004h, 004h  ;24 thru 2b
    db  005h, 005h, 005h, 005h, 005h, 005h, 005h, 005h  ;2c thru 33
    db  006h, 006h, 006h, 006h, 006h, 006h, 006h, 006h  ;34 thru 3b
    db  007h, 007h, 007h, 007h, 007h, 007h, 007h, 007h  ;3c thru 43
    db  008h, 008h, 008h, 008h, 008h, 008h, 008h, 008h  ;44 thru 4b
    db  009h, 009h, 009h, 009h, 009h, 009h, 009h, 009h  ;4c thru 53
    db  00ah, 00ah, 00ah, 00ah, 00ah, 00ah, 00ah, 00ah  ;54 thru 5b
    db  00bh, 00bh, 00bh, 00bh, 00bh, 00bh, 00bh, 00bh  ;5c thru 63
    db  00ch, 00ch, 00ch, 00ch, 00ch, 00ch, 00ch, 00ch  ;64 thru 6b
    db  00dh, 00dh, 00dh, 00dh, 00dh, 00dh, 00dh, 00dh  ;6c thru 73
    db  00eh, 00eh, 00eh, 00eh, 00eh, 00eh, 00eh, 00eh  ;74 thru 7b
    db  00fh, 00fh, 00fh, 00fh, 00fh, 00fh, 00fh, 00fh  ;7c thru 83
    db  010h, 010h, 010h, 010h, 010h, 010h, 010h, 010h  ;84 thru 8b
    db  011h, 011h, 011h, 011h, 011h, 011h, 011h, 011h  ;8c thru 93
    db  012h, 012h, 012h, 012h, 012h, 012h, 012h, 012h  ;94 thru 9b
    db  013h, 013h, 013h, 013h, 013h, 013h, 013h, 013h  ;9c thru a3
    db  014h, 014h, 014h, 014h, 014h, 014h, 014h, 014h  ;a4 thru ab
    db  015h, 015h, 015h, 015h, 015h, 015h, 015h, 015h  ;ac thru b3
    db  016h, 016h, 016h, 016h, 016h, 016h, 016h, 016h  ;b4 thru bb
    db  017h, 017h, 017h, 017h, 017h, 017h, 017h, 017h  ;bc thru c3
    db  018h, 018h, 018h, 018h, 018h, 018h, 018h, 018h  ;c4 thru cb
    db  019h, 019h, 019h, 019h, 019h, 019h, 019h, 019h  ;cc thru d3
    db  01ah, 01ah, 01ah, 01ah, 01ah, 01ah, 01ah, 01ah  ;d4 thru db
    db  01bh, 01bh, 01bh, 01bh, 01bh, 01bh, 01bh, 01bh  ;dc thru e3
    db  01ch, 01ch, 01ch, 01ch, 01ch, 01ch, 01ch, 01ch  ;e4 thru eb
    db  01dh, 01dh, 01dh, 01dh, 01dh, 01dh, 01dh, 01dh  ;ec thru f3
    db  01eh, 01eh, 01eh, 01eh, 01eh, 01eh, 01eh, 01eh  ;f4 thru fb
    db  01fh, 01fh, 01fh, 01fh                          ;fc thru ff

; This table contains 6 bit values for use in returning the nearest physical
; match to the 24 bit logical value, we use this to quickly replicate what the
; DIB_ColorInfo function would have returned.

ByteTo6BitLookup label byte
    db  000h, 000h, 000h, 000h, 000h, 000h
    db  001h, 001h, 001h, 001h   ;06 thru 09
    db  002h, 002h, 002h, 002h   ;0a thru 0d
    db  003h, 003h, 003h, 003h   ;0e thru 11
    db  004h, 004h, 004h, 004h   ;12 thru 15
    db  005h, 005h, 005h, 005h   ;16 thru 19
    db  006h, 006h, 006h, 006h   ;1a thru 1d
    db  007h, 007h, 007h, 007h   ;1e thru 21
    db  008h, 008h, 008h, 008h   ;22 thru 25
    db  009h, 009h, 009h, 009h   ;26 thru 29
    db  00ah, 00ah, 00ah, 00ah   ;2a thru 2d
    db  00bh, 00bh, 00bh, 00bh   ;2e thru 31
    db  00ch, 00ch, 00ch, 00ch   ;32 thru 35
    db  00dh, 00dh, 00dh, 00dh   ;36 thru 39
    db  00eh, 00eh, 00eh, 00eh   ;3a thru 3d
    db  00fh, 00fh, 00fh, 00fh   ;3e thru 41
    db  010h, 010h, 010h, 010h   ;42 thru 45
    db  011h, 011h, 011h, 011h   ;46 thru 49
    db  012h, 012h, 012h, 012h   ;4a thru 4d
    db  013h, 013h, 013h, 013h   ;4e thru 51
    db  014h, 014h, 014h, 014h   ;52 thru 55
    db  015h, 015h, 015h, 015h   ;56 thru 59
    db  016h, 016h, 016h, 016h   ;5a thru 5d
    db  017h, 017h, 017h, 017h   ;5e thru 61
    db  018h, 018h, 018h, 018h   ;62 thru 65
    db  019h, 019h, 019h, 019h   ;66 thru 69
    db  01ah, 01ah, 01ah, 01ah   ;6a thru 6d
    db  01bh, 01bh, 01bh, 01bh   ;6e thru 71
    db  01ch, 01ch, 01ch, 01ch   ;72 thru 75
    db  01dh, 01dh, 01dh, 01dh   ;76 thru 79
    db  01eh, 01eh, 01eh, 01eh   ;7a thru 7d
    db  01fh, 01fh, 01fh, 01fh   ;7e thru 81
    db  020h, 020h, 020h, 020h   ;82 thru 85
    db  021h, 021h, 021h, 021h   ;86 thru 89
    db  022h, 022h, 022h, 022h   ;8a thru 8d
    db  023h, 023h, 023h, 023h   ;8e thru 91
    db  024h, 024h, 024h, 024h   ;92 thru 95
    db  025h, 025h, 025h, 025h   ;96 thru 99
    db  026h, 026h, 026h, 026h   ;9a thru 9d
    db  027h, 027h, 027h, 027h   ;9e thru a1
    db  028h, 028h, 028h, 028h   ;a2 thru a5
    db  029h, 029h, 029h, 029h   ;a6 thru a9
    db  02ah, 02ah, 02ah, 02ah   ;aa thru ad
    db  02bh, 02bh, 02bh, 02bh   ;ae thru b1
    db  02ch, 02ch, 02ch, 02ch   ;b2 thru b5
    db  02dh, 02dh, 02dh, 02dh   ;b6 thru b9
    db  02eh, 02eh, 02eh, 02eh   ;ba thru bd
    db  02fh, 02fh, 02fh, 02fh   ;be thru c1
    db  030h, 030h, 030h, 030h   ;c2 thru c5
    db  031h, 031h, 031h, 031h   ;c6 thru c9
    db  032h, 032h, 032h, 032h   ;ca thru cd
    db  033h, 033h, 033h, 033h   ;ce thru d1
    db  034h, 034h, 034h, 034h   ;d2 thru d5
    db  035h, 035h, 035h, 035h   ;d6 thru d9
    db  036h, 036h, 036h, 036h   ;da thru dd
    db  037h, 037h, 037h, 037h   ;de thru e1
    db  038h, 038h, 038h, 038h   ;e2 thru e5
    db  039h, 039h, 039h, 039h   ;e6 thru e9
    db  03ah, 03ah, 03ah, 03ah   ;ea thru ed
    db  03bh, 03bh, 03bh, 03bh   ;ee thru f1
    db  03ch, 03ch, 03ch, 03ch   ;f2 thru f5
    db  03dh, 03dh, 03dh, 03dh   ;f6 thru f9
    db  03eh, 03eh, 03eh, 03eh   ;fa thru fd
    db  03fh, 03fh               ;fe thru ff

; This table contains 8 bit values for use in returning the RGB logical
; value from the 15 bit physical value, we use this to quickly replicate what the
; DIB_ColorInfo function would have returned.

PhysLog15BitLookup Label byte
    db  000h, 010h, 018h, 020h, 028h, 030h, 038h, 040h
    db  048h, 050h, 058h, 060h, 068h, 070h, 078h, 080h
    db  088h, 090h, 098h, 0a0h, 0a8h, 0b0h, 0b8h, 0c0h
    db  0c8h, 0d0h, 0d8h, 0e0h, 0e8h, 0f0h, 0f8h, 0ffh

; This table contains 8 bit values for use in returning the RGB logical
; value from the 16 bit physical value, we use this to quickly replicate what the
; DIB_ColorInfo function would have returned.

PhysLog16BitLookup label byte
    db  000h, 008h, 00ch, 010h, 014h, 018h, 01ch, 020h
    db  024h, 028h, 02ch, 030h, 034h, 038h, 03ch, 040h
    db  044h, 048h, 04ch, 050h, 054h, 058h, 05ch, 060h
    db  064h, 068h, 06ch, 070h, 074h, 078h, 07ch, 080h
    db  084h, 088h, 08ch, 090h, 094h, 098h, 09ch, 0a0h
    db  0a4h, 0a8h, 0ach, 0b0h, 0b4h, 0b8h, 0bch, 0c0h
    db  0c4h, 0c8h, 0cch, 0d0h, 0d4h, 0d8h, 0dch, 0e0h
    db  0e4h, 0e8h, 0ech, 0f0h, 0f4h, 0f8h, 0fch, 0ffh
    
;*----------------------------------------------------------------------
;Function name:  ColorInfo
;
;Description:    Converts a logical color (an RGB value) to a physical
;                color (a physical color value) or converts a physical
;                to a logical color, depending on the value of the
;                lpPColor parameter.  Every graphics device driver must
;                export a ColorInfo function.
;
;Information:
; COLORREF ColorInfo( LPPDEVICE lpDestDev, 
;                     DWORD     dwColorin, 
;                     LPPCOLOR  lpPColor);
; 
; lpDestDev
;     Address of a PDEVICE or PBITMAP structure that specifies the
;     destination device or bitmap.
; 
; dwColorin
;     A logical or physical color, depending on the value of lpPColor
;     parameter.
; 
; lpPColor
;     Address of a PCOLOR structure or is NULL.  If lpPColor points to a
;     PCOLOR structure, the function assumes that dwColorin specifies a
;     logical color (an RGB value) and copies the physical color value
;     that most closely matches the logical color to this structure.  If
;     lpPColor is NULL, the function assumes that dwColorin specifies a
;     physical color and returns the logical color that matches the
;     physical color.
; 
; The export ordinal for this function is 2.  When converting a
; logical color, ColorInfo chooses the best possible physical color to
; match the logical color and returns the RGB color value that
; corresponds to this physical color.  GDI uses the physical colors
; returned by ColorInfo to set text colors, background colors, and
; pixel colors using the Pixel function.  If dwColorin is a logical
; color, it may specify either an RGB value or a color index.  If
; dwColorin specifies a color index (high-order byte is 0xFF),
; ColorInfo should return the index without carrying out a conversion.
; If dwColorin is a physical color, ColorInfo should return an RGB
; value only for static colors and return color indices for all other
; colors.  If GDI receives a color index from ColorInfo, it uses other
; functions to determine the corresponding logical color.
; 
;
;Return:         Returns the logical color (RGB value) that most
;                closely matches the physical color either specified
;                by dwColorin or copied to the PCOLOR structure pointed
;                to by lpPColor.
;----------------------------------------------------------------------*

ColorInfo PROC FAR PASCAL PUBLIC,
         lpDestDev:DWORD, 
         dwColorin:DWORD, 
         lpPColor:DWORD

         mov         EAX, dwColorin    ; quick check for black or white
         and         EAX, 0FFFFFFh
         jne short   Color_003
         ; Black falls through
         cmp         word ptr lpPColor+2, 0 ; check for NULL lpPColor
         je short    Color_001
         les         BX, lpPColor
         mov         word ptr ES:[BX+2], 4000h ; returning 040000000 like DIB ENGINE
         mov         ES:[BX], AX     ; AX = 0, so we can use it here
Color_001:
         xor         DX, DX            ; for physical to logical return 
         ret
         
         ; now check for white
Color_003:
         cmp         EAX, 0FFFFFFh     ; white check
         jne short   Color_005
         cmp         word ptr lpPColor+2, 0 ; check for NULL lpPColor
         je short    Color_004
         les         BX, lpPColor
         mov         word ptr ES:[BX+2], 40FFh ; returning 40FFFFFF like DIB ENGINE
         mov         word ptr ES:[BX], 0FFFFh ; AX = 0, so we can use it here
Color_004:
         mov         DX, 0FFh          ; for physical to logical return
         ret

Color_005:
         les         BX, lpDestDev
         mov         CL, ES:[BX].deBitsPixel

         cmp         WORD PTR lpPColor+2,0
         je short    PhysicalToLogical

         cmp         cl, 16
         je          LogicalToPhysical16
         cmp         cl, 1
         je short    LogicalToPhysicalMono
         cmp         cl, 32
         je          LogicalToPhysical32

         push        lpDestDev
         push        dwColorin
         push        lpPColor
         call        DIB_ColorInfo
         ret

PhysicalToLogical:

         cmp         cl, 16
         je          PhysicalToLogical16
         cmp         cl, 32
         je          PhysicalToLogical32

         push        lpDestDev
         push        dwColorin
         push        lpPColor
         call        DIB_ColorInfo
         ret

LogicalToPhysicalMono:

         cmp         eax, 0C0C0C0h
         jz          monowhite
         cmp         eax, 0E1E1E1h
         jz          monowhite
         cmp         eax, 0919191h
         jz          monowhite
         cmp         eax, 0FF00FFh
         jz          monowhite
         cmp         eax, 0FF0000h
         jz          monoblack

         push        lpDestDev
         push        dwColorin
         push        lpPColor
         call        DIB_ColorInfo
         ret

monoblack:

         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 80000000h
         xor         ax, ax
         xor         dx, dx
         ret

monowhite:

         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 81000001h
         mov         ax, 0FFFFh
         mov         dx, 0FFh
         ret

LogicalToPhysical16:

         test        es:[bx].deFlags, FIVE6FIVE
         jne         LogicalToPhysical16_565

LogicalToPhysical16_555:

         cmp         eax, 0C0C0C0h
         je          LtoP16_C0_555

         cmp         eax, 0808080h
         je          LtoP16_80_555

         ; NOTE: Logical color is passed in as XBGR format... Blue Green Red

         mov         ebx, eax
         shr         ebx, 16
         xor         ecx, ecx
         mov         dx, bx
         mov         cl, BYTE PTR CS:[OFFSET ByteTo5BitLookup + bx]
         push        di
         movzx       di, cl
         mov         dl, BYTE PTR CS:[OFFSET PhysLog15BitLookup + di]
         mov         bl, al
         mov         ch, BYTE PTR CS:[OFFSET ByteTo5BitLookup + bx]
         movzx       di, ch
         mov         al, BYTE PTR CS:[OFFSET PhysLog15BitLookup + di]
         shl         ch, 2    ; prep for 5:5:5
         mov         bl, ah   ; get green
         mov         bl, BYTE PTR CS:[OFFSET ByteTo5BitLookup + bx]
         movzx       di, bl
         mov         ah, BYTE PTR CS:[OFFSET PhysLog15BitLookup + di]
         pop         di
         shl         bx, 5
         or          cx, bx

         les         bx, lpPColor
         mov         DWORD PTR es:[bx], ecx
         ret

LtoP16_C0_555:
         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 5EF7h
         mov         dx, 0C0h
         ret

LtoP16_80_555:
         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 3DEFh
         mov         dx, 080h
         ret


LogicalToPhysical16_565:

         cmp         eax, 0C0C0C0h
         je          LtoP16_C0_565

         cmp         eax, 0808080h
         je          LtoP16_80_565

    ; NOTE: Logical color is passed in as XBGR format... Blue Green Red

         mov         ebx, eax
         shr         ebx, 16
         xor         ecx, ecx
         mov         dx, bx
         mov         cl, BYTE PTR CS:[OFFSET ByteTo5BitLookup + bx]
         push        di
         movzx       di, cl
         mov         dl, BYTE PTR CS:[OFFSET PhysLog15BitLookup + di]
         mov         bl, al
         mov         ch, BYTE PTR CS:[OFFSET ByteTo5BitLookup + bx]
         movzx       di, ch
         mov         al, BYTE PTR CS:[OFFSET PhysLog15BitLookup + di]
         shl         ch, 3    ; prep for 5:6:5
         mov         bl, ah   ; get green
         mov         bl, BYTE PTR CS:[OFFSET ByteTo6BitLookup + bx]
         movzx       di, bl
         mov         ah, BYTE PTR CS:[OFFSET PhysLog16BitLookup + di]
         pop         di
         shl         bx, 5
         or          cx, bx

         les         bx, lpPColor
         mov         DWORD PTR es:[bx], ecx
         ret

LtoP16_C0_565:
         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 0BDF7h
         mov         dx, 0C0h
         ret

LtoP16_80_565:
         les         bx, lpPColor
         mov         DWORD PTR es:[bx], 7BEFh
         mov         dx, 080h
         ret

PhysicalToLogical16:

         test        es:[bx].deFlags, FIVE6FIVE
         jne         PhysicalToLogical16_565

PhysicalToLogical16_555:

         mov         bx, ax
         and         bx, 0001fh
         movzx       dx, BYTE PTR CS:[OFFSET PhysLog15BitLookup + bx]
         mov         bx, ax
         and         bx, 07c00h
         shr         bx, 10
         mov         cl, BYTE PTR CS:[OFFSET PhysLog15BitLookup + bx]
         mov         bx, ax
         and         bx, 003e0h
         shr         bx, 5
         mov         ch, BYTE PTR CS:[OFFSET PhysLog15BitLookup + bx]
         mov         ax, cx
         ret

PhysicalToLogical16_565:

         mov         bx, ax
         and         bx, 0001fh
         movzx       dx, BYTE PTR CS:[OFFSET PhysLog15BitLookup + bx]
         mov         bx, ax
         and         bx, 0f800h
         shr         bx, 11
         mov         cl, BYTE PTR CS:[OFFSET PhysLog15BitLookup + bx]
         mov         bx, ax
         and         bx, 007e0h
         shr         bx, 5
         mov         ch, BYTE PTR CS:[OFFSET PhysLog16BitLookup + bx]
         mov         ax, cx
         ret

LogicalToPhysical32:          ; Simply perform BGR/RGB swap and return in DX:AX
         mov         dx, WORD PTR dwColorin + 2
         mov         cl, dl
         mov         dl, al
         mov         al, cl
         les         bx, lpPColor      ; Store to lpPColor as well
         mov         WORD PTR es:[bx+2], dx
         mov         WORD PTR es:[bx], ax

         mov         dx, WORD PTR dwColorin + 2
         mov         ax, WORD PTR dwColorin
         ret

PhysicalToLogical32:          ; Simply perform BGR/RGB swap and return in DX:AX
         mov         dx, WORD PTR dwColorin + 2
         mov         bl, al
         xor         dh, dh
         mov         al, dl
         mov         dl, bl
         ret

ColorInfo ENDP

_TEXT   ends

END
