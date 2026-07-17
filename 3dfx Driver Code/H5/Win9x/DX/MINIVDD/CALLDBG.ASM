;/* $Header: /devel/h3/Win95/dx/minivdd/calldbg.asm 3     1/04/99 1:21p Michael $ */
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
;** File name:   calldbg.asm
;**
;** Description: The CallOutputDebugString function
;**
;** $Revision: 3 $
;** $Date: 1/04/99 1:21p $
;**
;** $History: calldbg.asm $
;; 
;; *****************  Version 3  *****************
;; User: Michael      Date: 1/04/99    Time: 1:21p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Implement the 3Dfx/STB unified header.
;; 
;; *****************  Version 2  *****************
;; User: Ken          Date: 4/15/98    Time: 6:41p
;; Updated in $/devel/h3/win95/dx/minivdd
;; added unified header to all files, with revision, etc. info in it
;**
;*/

        .486
        .MODEL  FLAT, C

;=======================================================================
;       type definitions
;=======================================================================

PPROC16 TYPEDEF FAR16 PTR


;=======================================================================
        .DATA
;=======================================================================

dbgfcn  PPROC16 ?
        .CODE
;=======================================================================


;*----------------------------------------------------------------------
;Function name:  CallOutputDebugString
;
;Description:    Send a string to the debug output device.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
CallOutputDebugString PROC PUBLIC, 
					buff:dword,
					msgfcn:dword
;-----------------------------------------------------------------------
        movzx   ebp, bp
        push    cs
        push    OFFSET done
		mov eax, dword ptr ss:[msgfcn]
		mov dbgfcn, eax
		jmp [dbgfcn]

done:
	ret

CallOutputDebugString  endp


end

