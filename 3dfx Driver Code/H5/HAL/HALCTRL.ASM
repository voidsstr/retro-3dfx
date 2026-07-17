;****************************************************************************
; MGACTRL.ASM
;----------------------------------------------------------------------------
; Copyright 1996, Microsoft Inc.  All rights reserved.
;
; MGA Mini-VDD Support Functions C Wrapper
;
;****************************************************************************

title		MGA Mini-VDD Support Functions
.386p

.xlist
include 	VMM.INC
.list

extern _H3Hal_API:proc
		
;		Virtual Device Declaration
        Declare_Virtual_Device  H3HAL,                                 \
			        3,					\
			        1,					\
			        H3HAL_Control,			\
			        0beefh,			\
			        Undefined_Init_Order, 			\
			        _H3Hal_API,				\
			        _H3Hal_API,				\
			        ,

VxD_LOCKED_CODE_SEG


BeginProc H3HAL_Control

         Control_Dispatch Sys_Dynamic_Device_Init, H3Hal_Dyn_Init,sCall,<ebx>
         Control_Dispatch Sys_Dynamic_Device_Exit, H3Hal_Shutdown,sCall
         clc                                   ; indicate no error
         ret

EndProc  H3HAL_Control

; Begin_Control_Dispatch	H3HAL	; 
; 	Control_Dispatch Sys_Dynamic_Device_Init, H3Hal_Dyn_Init,sCall,<ebx>
; 	Control_Dispatch Sys_Dynamic_Device_Exit, 
; End_Control_Dispatch H3HAL

BeginProc _MyPrintf
	push eax
	push ebx
	VMMCall _Debug_Printf_Service
	pop ebx
	pop eax
	ret
EndProc _MyPrintf

	
VxD_LOCKED_CODE_ENDS

 end
