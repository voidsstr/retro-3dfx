;/* $Header: /devel/h3/Win95/dx/dd16/dispinfo.asm 2     12/28/98 11:24a Michael $ */
;/*
;** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
;** Permission to copy this software and to use it for any
;** purpose is hereby granted, subject to the following use
;** restrictions:
;**
;** 1. ANY COPY MADE OF THIS SOURCE CODE MUST INCLUDE THE ABOVE
;**    COPYRIGHT NOTICE AND THIS LIST OF USE RESTRICTIONS IN FULL.
;**
;** 2. MODIFIED VERSIONS OF THIS SOURCE CODE MAY NOT BE REDIS-
;**    TRIBUTED WITHOUT THE EXPRESSED WRITTEN CONSENT OF
;**    CIRRUS LOGIC, INC.
;**
;** 3. THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT
;**    WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
;**    INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
;**    MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
;**
;** File name:   dispinfo.asm
;**
;** Description: provides the Plug-n-Play interface for PNP16VPM.
;**
;** $Revision: 2 $
;** $Date: 12/28/98 11:24a $
;**
;** $History: dispinfo.asm $
;; 
;; *****************  Version 2  *****************
;; User: Michael      Date: 12/28/98   Time: 11:24a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;** 
;*/

   .MODEL  LARGE,C
   .DATA
   apiaddr	dd		?
   .CODE
;  .386
   .486


DISPLAYINFO STRUCT
   diHdrSize               WORD  ?
   diInfoFlags             WORD  ?
   diDevNodeHandle         DWORD ?
   diDriverName            BYTE 16 DUP (?)
   diXRes                  WORD  ?
   diYRes                  WORD  ?
   diDPI                   WORD  ?
   diPlanes                BYTE  ?
   diBpp                   BYTE  ?
   diRefreshRateMax        WORD  ?
   diRefreshRateMin        WORD  ?
   diLowHorz               WORD  ?
   diHighHorz              WORD  ?
   diLowVert               WORD  ?
   diHighVert              WORD  ?
   diMonitorDevNodeHandle  DWORD ?
   diHorzSyncPolarity      BYTE  ?
   diVertSyncPolarity      BYTE  ?
DISPLAYINFO ENDS


;*----------------------------------------------------------------------
;Function name:  GetDisplayInfo
;
;Description:    Fills in the DISPLAYTINFO structure
;
;Information:
;
;Return:         AX     1 = success or,
;                       0 = failure
;----------------------------------------------------------------------*
GetDisplayInfo  PROC FAR C PUBLIC USES ds es di si ebx ecx edx,
                                 lpDisplayInfo      :DWORD

                                 LOCAL EntryPointVDD :DWORD


   ;; CLEAR OUT THE DISPLAY INFO STRUCTURE.
      les   di, lpDisplayInfo
      mov   ax, es
      mov   ds, ax
      mov   cx,      (size DISPLAYINFO)/2
      xor   ax,      ax
      rep   stosw

   ;; GET THE VDD RING3 ENTRY POINT.
      mov   ax,      1684h                     ;Int 2FH: GET DEVICE API ENTRY POINT
      mov   bx,      000Ah                     ;this is VxD ID for VDD
      int   2fh

   ;; ES:DI CONTAINS THE ENTRY-POINT ADDRESS.
      mov   word ptr EntryPointVDD,di
      mov   word ptr EntryPointVDD+2,es


   ;; PUT THE "MAGIC NUMBER" IN DX. (VMHandle for VDD)
      mov   ax,      1683h
      int   2fh
      movzx edx,     bx

      xor   edi, edi			 ;Set edi to zero
      lds   di, lpDisplayInfo            ;es:edi-->DispInfo structure
      mov   ax, ds
      mov   es, ax
      mov   eax, 85h                 ;put service number in EAX
      mov   ebx, edx                 ;put VDD VMHandle in EBX
      mov   ecx, size DISPLAYINFO    ;put structure size in ECX
      call  dword ptr EntryPointVDD
      cmp   eax,     85h                 ;Did the call work?
      je    gdn_bogus                    ;no
      inc   eax                          ;yes. Could vdd read the registry?
      jz    gdn_bogus                    ;no.
      mov   ax, 1
      ret	
gdn_bogus:
      xor   ax, ax
      ret
GetDisplayInfo ENDP


END
