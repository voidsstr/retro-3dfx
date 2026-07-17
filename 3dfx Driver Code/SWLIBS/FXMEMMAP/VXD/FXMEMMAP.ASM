; THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
; PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
; TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
; INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
; DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
; THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
; EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
; FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
; 
; USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
; RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
; TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
; AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
; SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
; THE UNITED STATES.  
; 
; COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
;
; $Revision: 45 $
; $Date: 6/24/99 10:46a $
;

; This VxD provides a DeviceIoControl interface to a subset of DPMI type     
; services in the Windows95 VMM because Int 31h DPMI calls do not work in 
; Win32 apps/DLLs. The VWIN32.VXD allegedly provides routines for invoking
; interrupt 31h but I could not find them documented in the DDK.

   name  capvxd
   .586p
   include vmm.inc
   include vwin32.inc

if 0    
   include vmmreg.inc
else
ERROR_SUCCESS   equ 0h    
endif

   include debug.inc
    
ifdef VGASWITCH
   include minivdd.inc
   include vkd.inc
endif


devid    equ   039d2h                ; need unique id for V86, PM entry pt.

; These changes *MUST* be kept in sync with fxmemmap.h!!!
fxmajor_ver    equ 01h               ; "dynamic" version. Started at 1.00
fxminor_ver    equ 04h               ; currently 1.04

byp  equ   <byte ptr>
wp   equ   <word ptr>
dwp  equ   <dword ptr>
fwp  equ   <fword ptr>

;------------------------------------------------------------------------------
;          D E V I C E   D E F I N I T I O N   B L O C K
;------------------------------------------------------------------------------

ifndef  VGASWITCH
;Declare_Virtual_Device FXMEMMAP, 1, 0, control, devid, Undefined_Init_Order 
Declare_Virtual_Device FXMEMMAP, fxmajor_ver, fxminor_ver, control, devid, Undefined_Init_Order 
else
;Declare_Virtual_Device FXMEMMAP, 1, 0, control, devid, Undefined_Init_Order, API_Proc, API_Proc 
Declare_Virtual_Device FXMEMMAP, fxmajor_ver, fxminor_ver, control, devid, Undefined_Init_Order, API_Proc, API_Proc 
endif
     

;------------------------------------------------------------------------------
;              D A T A   A R E A S
;------------------------------------------------------------------------------

VxD_LOCKED_DATA_SEG

P6wcfix_keyname   db "SOFTWARE\3Dfx Interactive\Shared", 0
P6wcfix_valuename db "DisableP6WCfix", 0

MutexCount db 0
gwhatstring db "@#%Fxmemmap MS Version 4.10.01.0015, Glide Version 2.5",0 

servtbl  label dword                 ; DeviceIoControl service table
   dd  offset32 closehandle          ; -1 -- DIOC_CLOSEHANDLE
   dd  offset32 getversion           ;  0 -- DIOC_GETVERSION
   dd  offset32 getappversion        ;  1 -- GETAPPVERSION
   dd  offset32 getlinearaddress     ;  2 -- GETLINEARADDR
   dd  offset32 getldtselector       ;  3 -- GETLDTSELECTOR
   dd  offset32 freeldtselector      ;  4 -- FREELDTSELECTOR
    
ifdef VGASWITCH
   dd  offset32 getlinearaddress     ;  5 -- GETLINEARADDR (DUAL_HEAD)
   dd  offset32 getmsr               ;  6 -- GETMSR
   dd  offset32 setmsr               ;  7 -- SETMSR
   dd  offset32 decrementmutex       ;  8 -- DECREMENTMUTEX
   dd  offset32 setpassthroughbase   ;  9 -- SETPASSTHROUGHBASE
   dd  offset32 setaddrperm          ; 10 -- SetAddrPermission
endif    

numservices equ ($-servtbl)/4
    
ifdef VGASWITCH
   sstBaseAddr         dd 0          ; SST Base address
   sstBaseLen          dd 0          ; SST Hw address space size
   pPrevSystemControl  dd 0          ; old VMM_System_Control proc
   pPrevSetVMType      dd 0          ; old VDD_Set_VMType proc
   pPrevPifState       dd 0          ; old VDD_PIF_State  proc
   myVMHandle          dd 0          ; VM handle of owner of mmap.
   sysVMHandle         dd 0
   hHotKey             dd 0
   vgStatus            dd 0
   temp                dd 0    
   temp1               dd 0    
   temp2               dd 0
   addrPerm16Buf       DIOCParams { 0 }
endif
   physAddr            dd 0
   physMask            dd 0    
   physSize            dd 0
   vID                 dd 0
   dID                 dd 0    
   isP6                dd 0
   dataPtr             dd 0
   inputBuffer         dd 0
   outputBuffer        dd 0    
VxD_LOCKED_DATA_ENDS

;------------------------------------------------------------------------------
;        D E V I C E   C O N T R O L   P R O C E D U R E
;------------------------------------------------------------------------------

VxD_LOCKED_CODE_SEG

BeginProc control

   Control_Dispatch Sys_Dynamic_Device_Init, dyninit
   Control_Dispatch Sys_Dynamic_Device_Exit, dynexit
   Control_Dispatch W32_DeviceIocontrol,     devctrl

   clc                               ; indicate no error
   ret

EndProc  control

;------------------------------------------------------------------------------
;     D Y N A M I C   L O A D   I N I T I A L I Z A T I O N
;------------------------------------------------------------------------------
   
BeginProc dyninit
   Trace_Out 'FXMEMMAP: Dynamic loading (ds: #ds)'
   ;Good place to detect things
   ; like CPU type or to allocate
   ; thread local storage
    
   ;; First, check for P6
   call  CheckForP6                  ; This sets isP6

   clc                               ; indicate success
   ret                               ; return to vxd loader

dyninit_fail:
   Debug_Out 'FXMEMMAP: Not loading'
   stc                               ; indicate failure
   ret                               ; return to vxd loader

EndProc  dyninit

;------------------------------------------------------------------------------
;              D Y N A M I C   U N L O A D
;------------------------------------------------------------------------------

BeginProc dynexit
   Trace_Out 'FXMEMMAP: Dynamic unload (ds: #ds)'
   ; Good place to free things
   ; like locked pages or
   ; thread local storage

ifdef VGASWITCH
   call  UnhookProcs                 ; unhook everything we hooked.
   ; jc    dynexit_fail         
endif

   clc                               ; indicate no error
   ret                               ; return to vxd loader

dynexit_fail:
   Debug_Out 'FXMEMMAP: Cannot unload'
   stc                               ; indicate failure
   ret                               ; return to vxd loader
EndProc   dynexit

;------------------------------------------------------------------------------
;       D E V I C E I O C O N T R O L   I N T E R F A C E
;------------------------------------------------------------------------------

;  ENTRY:
;  ESI -> address of DIOC structure

BeginProc devctrl
   mov   ecx, [esi+dwIoControlCode]  ; ECX = service index
   inc   ecx                         ; add one to service index
   cmp   ecx, numservices            ; is service in bounds?
   jae   devctrl_fail                ; if not, fail the call
   Trace_Out 'FXMEMMAP: Jumping to routine #ecx '
   jmp   servtbl[4*ecx]              ; yes. branch to server

;------------------------------------------------------------------------------
;
;  ECX = -1 (DIOC_CLOSEHANDLE)

closehandle:
   mov     al, MutexCount
   test    al, al
   jz      pastDecCH
   lock    dec MutexCount            ; Decrement instance count for
                                     ; Glide in a window Mutex

ifdef VGASWITCH
    ;; dpc - 9 aug 1997
    ;; If we're closing the vxd and done holding the mutex,
    ;; then its time to unhook anything. Its safe to call UnhookProcs
    ;; here because it checks to see if we hooked anything before
    ;; doing anything else.
    ;;
    ;; The problem occurs (for me, leo, and miriam) in the following way:
    ;; (We used mdk as our sample application.)
    ;; 
    ;;  1 - Run and quit Glide app
    ;;      vxd is unloaded upon call to pciClose.
    ;;  2 - Run and quit D3D app
    ;;      The mutex count is zero, but the vxd is not
    ;;      unloaded when app quits.
    ;;  3 - Run Glide app
    ;;      Mutex count properly goes to zero, but the vxd is not
    ;;      unloaded since the reference count has not hit zero.
    ;;      The vxd hooked when glide used it to map a linear address,
    ;;      and these hooks are still installed.
    ;;  4 - Run D3D app
    ;;      Driver sets vga pass through to whatever it wants it to
    ;;      be, and moments later the vxd hook does likewise. The
    ;;      machine appears to be hung since the screen is blank
    ;;      and even the three finger salute seems to do nothing
    ;;      helpful (unless you consider re-booting helpful). Nasty.

   mov     al, MutexCount
   test    al, al
   jnz     pastDecCH

   Trace_Out "CloseHandle(Unhook)"
   call    UnhookProcs

    ;; Make sure that the linear address space of the hw is
    ;; set to be writable so that if we are closing because of
    ;; a crash or something things are sane on the next init
    mov     eax, sstBaseAddr
    test    eax, eax
    jz      pastDecCH

    mov     ebx, sstBaseLen

    ;; Convert to page units
    shr     eax, 12
    shr     ebx, 12

    ;; Set the page characteristics so that we keep all the
    ;; current characteristics and OR in PC_USER
    mov     ecx, -1
    mov     edx, PC_USER

    Trace_Out "FxMemmap(CloseHandle): VMMCall _PageModifyPermissions <#eax, #ebx, #ecx, #edx>"
    VMMCall _PageModifyPermissions <eax, ebx, ecx, edx>
    Trace_Out "FxMemmap(CloseHandle):   -> #eax"
endif
    
pastDecCH: 
   jmp     devctrl_okay

;------------------------------------------------------------------------------
;
;    ECX = 0 (DIOC_GETVERSION)

getversion:
   lock    inc MutexCount            ; Increment instance count for
                                     ; Glide in a window Mutex
ifdef Debug
   xor     eax, eax
   mov     al, MutexCount            ;
   Trace_out "MutexCount = #eax"     ;
endif
   jmp     devctrl_okay              ; satisfy check by VWIN32's
                                     ;   CreateFile handler

;------------------------------------------------------------------------------
;
;    ECX = 1 (GETAPPVERSION)
;
;    Output:
;    0     (DWORD) version number of this VxD

getappversion:
   cmp   [esi+cbOutBuffer], 4        ; need at least 4 output bytes
   jb    @F                          ; fail call if not
   mov   edx, [esi+lpvOutBuffer]     ; point to output buffer
   test  edx, edx                    ; be sure not null
   jz    @F                          ;   ..

   movzx ax, byp [MutexCount]        ; 
   shl   eax, 16
   mov   ax, wp FXMEMMAP_DDB + DDB_Dev_Major_Version
   xchg  ah, al                      ; (fields reversed in DDB)

   mov   dwp [edx], eax              ; store version number

   mov   edx, [esi+lpcbBytesReturned]; get ptr to caller's variable
   test  edx, edx                    ; make sure not null
   jz    @F                          ;   ..
   mov   dwp [edx], 4                ; store # bytes returned
@@:
   jmp   devctrl_okay                ; done


;------------------------------------------------------------------------------
;
;    ECX = 2 (GETLINEARADDR)
;
;    Input:
;   0      (DWORD) Address of pointer to physical address and size
;    Output:
;   0      (DWORD) Address of pointer to linear address and size return
getlinearaddress:
   Trace_Out 'FXMEMMAP: GETLINEARADDR (ds:  #ds)'
   cmp   [esi+cbInBuffer], 4         ; need at least 4 input bytes
   jb    cantmap                     ; error if not

   cmp   [esi+cbOutBuffer], 4        ; need at least 4 output bytes
   jb    cantmap                     ; error if not

   mov   edx, [esi+lpvOutBuffer]     ; point to output buffer
   test  edx, edx                    ; be sure not null
   jb    cantmap                     ; error if not

   mov   edx, [esi+lpvInBuffer]      ; point to input data
   mov   eax, dwp [edx]              ; get pointer to physical address
   test  eax, eax                    ; is it zero?
   jz    cantmap                     ; if yes, nothing more to do

   mov   ebx, dwp [eax+4]            ; get physical size
   test  ebx, ebx                    ; is it zero?
   jz    cantmap                     ; if yes, nothing more to do

   mov   eax, dwp [eax]              ; get physical address
   test  eax, eax                    ; is it zero?
   jz    cantmap                     ; if yes, nothing more to do

   ; Save current parameters
   mov  physAddr, eax
   mov  physSize, ebx      

   ;
   ; Map this physical address into linear memory
   ;
   ; eg: VMMCall  _MapPhysToLinear <0fb000000h, 01000000h, 0> 
   ; input   eax = physical address
   ;     ebx = physical size
   ; returns eax = ring 0 linear address
   ;     ecx = used
   ;     edx = used
   VMMCall  _MapPhysToLinear <eax, ebx, 0>

   mov  sstBaseAddr, eax   ;     
   je   cantmap

   mov   edx, [esi+lpvOutBuffer]     ; point to output data
   mov   edx, dwp [edx]              ; get pointer to linear address
   mov   dwp [edx], eax              ; store linear address
   mov   dwp [edx+4], ebx            ; store physical size

   mov   edx, [esi+lpcbBytesReturned]; get ptr to caller's variable
   test  edx, edx                    ; make sure not null
   jz    @F                  
   mov   dwp [edx], 4                ; store # bytes returned

@:
ifdef VGASWITCH
   mov   edx, [esi+dwIoControlCode]  ; ECX = service index
   Trace_Out 'FXMEMMAP: IoControlCode:  #edx'
   cmp   edx, 5                      ; auto VGA Switch enabled?
   jne   @F
   mov   ebx, [esi+VMHandle]         ; EAX=addr, EBX=vmhandle
     
   call  HookProcs                   ; set up hooks if necessary
   jc    devctrl_fail
endif
@@:
   jmp   devctrl_okay                ; done

cantmap:
   jmp   devctrl_fail                ; fail the call


;------------------------------------------------------------------------------
;
;    ECX = 3 (GETLDTSELECTOR)
;
;    Input:
;   0      (DWORD) Address of pointer to linear address and size
;    Output
;   0      (DWORD) Address of pointer for created selector

getldtselector:
   Trace_Out 'FXMEMMAP: GETLDTSELECTOR'

   cmp   [esi+cbInBuffer], 4         ; need at least 4 input bytes
   jb    cantgetsel                  ; error if not

   cmp   [esi+cbOutBuffer], 4        ; need at least 4 output bytes
   jb    cantgetsel                  ; error if not

   mov   edx, [esi+lpvOutBuffer]     ; point to output buffer
   test  edx, edx                    ; be sure not null
   jb    cantgetsel                  ; error if not

   mov   edx, [esi+lpvInBuffer]      ; point to input data
   mov   eax, dwp [edx]              ; get pointer to linear address
   test  eax, eax                    ; is it zero?
   jz    cantgetsel                  ; erro if yes

   mov   ebx, dwp [eax]              ; get linear address
   test  ebx, ebx                    ; is it zero?
   jz    cantgetsel                  ; error if yes

   mov   ecx, dwp [eax+4]            ; get physical size
   shr   ecx, 12                     ; size/4096 
   test  ecx, ecx                    ; is it zero?
   jz    cantgetsel                  ; error if yes

   ;
   ; Build a descriptor for the LDT selector allocation
   ;
   ; input   ebx = linear base addr
   ;     ecx = 20-bit segment limit
   ; returns eax = low-order dword of segment descriptor
   ;     edx = high-order dword of segment descriptor
   ;     ecx = used

   VMMCall  _BuildDescriptorDWORDs, <ebx, ecx, RW_DATA_TYPE, D_GRAN_PAGE, 0>
   mov  sstBaseAddr, eax
   
   ;
   ; Create an LDT selector 
   ;
   ; input   ebx = handle of the VM to receive selectors
   ;     eax = low-order dword of segment descriptor
   ;     edx = high-order dword of segment descriptor
   ; returns eax = the new selector
   ;     edx = loword is ldt selector, hiword is LDT size
   ;     ecx = used

   mov   ebx, [esi+VMHandle]         ; get the current VM

   VMMCall  _Allocate_LDT_Selector, <ebx, edx, eax, 1, 0>

   cmp   eax, -1                     ; was there an error?
   je    cantgetsel

   mov   edx, [esi+lpvOutBuffer]     ; point to output data

   mov   edx, dwp [edx]              ; point to selector storage
   mov   dwp [edx], eax              ; store selector

   mov   edx, [esi+lpcbBytesReturned]; get ptr to caller's variable
   test  edx, edx                    ; make sure not null
   jz    @F                  
   mov   dwp [edx], 4                ; store # bytes returned
@@:
   jmp   devctrl_okay                ; done

cantgetsel:
   jmp   devctrl_fail                ; fail the call

;------------------------------------------------------------------------------
;
;    ECX = 4 (FREELDTSELECTOR)
;
;    Input:
;   0   (DWORD) Address of pointer to created selector    
;    Output:
;   0   (DWORD) Address of pointer to return result

freeldtselector:
   Trace_Out 'FXMEMMAP: FREELDTSELECTOR'



   cmp   [esi+cbInBuffer], 4         ; need at least 4 input bytes
   jb    cantfreesel                 ; error if not

   cmp   [esi+cbOutBuffer], 4        ; need at least 4 output bytes
   jb    cantfreesel                 ; error if not

   mov   edx, [esi+lpvOutBuffer]     ; point to output buffer
   test  edx, edx                    ; be sure not null
   jb    cantfreesel                 ; error if not

   mov   edx, [esi+lpvInBuffer]      ; point to input data
   mov   eax, dwp [edx]              ; get pointer to selector
   test  eax, eax                    ; is it zero?
   jz    cantfreesel                 ; error if yes

   mov   ebx, dwp [eax]              ; get selector
   test  ebx, ebx                    ; is it zero?

   jz    cantfreesel                 ; error if yes

   ;
   ; Free an LDT selector 
   ;
   ; input   eax = handle of the VM owning selector
   ;     ebx = selector
   ; returns eax = non-zero if successful

   mov   eax, [esi+VMHandle]         ; get the current VM

   VMMCall  _Free_LDT_Selector, <eax, ebx, 0>

   test  eax, eax                    ; did it work?
   jz    cantfreesel

   mov   edx, [esi+lpvOutBuffer]     ; point to output data

   mov   edx, dwp [edx]              ; point to selector storage
   mov   dwp [edx], eax              ; store 'good' result

   mov   edx, [esi+lpcbBytesReturned]; get ptr to caller's variable
   test  edx, edx                    ; make sure not null
   jz    @F                  
   mov   dwp [edx], 4                ; store # bytes returned
@@:
   jmp   devctrl_okay                ; done

cantfreesel:
   jmp   devctrl_fail                ; fail the call

;;;----------------------------------------------------------------------------
;;;  ECX = 6 (GETMSR)
;;; 
;;;  Input:
;;;    DWORD 0:    MSR Number
;;;    DWORD 1:    irrelavent
;;;    DWORD 2:    irrelavent
;;;  Output:
;;;    DWORD 0:    MSR Number if success; 0FFFh if failure
;;;    DWORD 1:    Lo 32 bits (EAX after rdmsr)
;;;    DWORD 2:    Hi 32 bits (EDX after rdmsr)
getmsr:    
   Trace_out "FXMEMMAP(GETMSR):"

   ;; First check to see if we're on a P6
   mov     eax, isP6
   test    eax, eax
   jz      msrNotP6 

   ;; Check input size
   cmp     [esi + cbInBuffer], 12    ; MSRInfo Structure is 3 DWORDS
   jb      getmsrFailed              ;

   ;; Check output size
   cmp     [esi + cbOutBuffer], 12   ; MSRInfo Structure is 3 DWORDS
   jb      getmsrFailed              ; 

   ;; Check for nonzero input buffer
   mov     ebx, dwp [esi + lpvInBuffer]
   test    ebx, ebx                  ; Test for zero
   jz      getmsrFailed              ; zero is bad
   
   ;; Check for nonzero output buffer
   mov     ebx, dwp [esi + lpvOutBuffer]
   test    ebx, ebx                  ; Check for zero
   jz      getmsrFailed              ; BAD zero! BAD!
   
   ;; Get the output buffer and squirrel 
   mov     eax, [esi + lpvOutBuffer] ; buffer
   mov     outputBuffer, eax         ; save it away

   ;;
   ;; Get the MSR
   ;;

   ;; First, clear eax and edx;
   xor     eax, eax
   xor     edx, edx   

   mov     eax, [esi + lpvInBuffer]  ; Get the MSR number
   mov     ecx, dwp [eax]            ; rdmsr expects MSR in ecx

   rdmsr                             ; Read the MSR
          
   ;;
   ;; Save output data - remember we already have the output data
   ;; pointer in ebx.
   ;;
   mov     [ebx], ecx   
   mov     [ebx + 4], eax            ; Store lo 32 bits in lo DWORD
   mov     [ebx + 8], edx            ; Store hi 32 bits in hi DWORD

   jmp     getmsrSuccess
   
getmsrFailed:  
   Trace_Out "FXMEMMAP(GETMSR):  Failure"
   mov     [ebx], 0FFFh              ; Indicate failure
   jmp     devctrl_fail              ; 

getmsrSuccess: 
   Trace_Out "FXMEMMAP(GETMSR):  EAX = #eax EDX = #edx"
   jmp devctrl_okay


;;;----------------------------------------------------------------------------
;;;  ECX = 7 (SETMSR)
;;;
;;;  Input:
;;;    DWORD 0:    MSR Number 
;;;    DWORD 1:    Lo 32 bits (EAX after rdmsr)
;;;    DWORD 2:    Hi 32 bits (EDX after rdmsr)
;;;  Output:
;;;    DWORD 0:    MSR Number if success; FFFh if failure
;;;    DWORD 1:    irrelavent
;;;    DWORD 2:    irrelavent
setmsr:
   Trace_out "FXMEMMAP(SETMSR):"
   
  ;; First check to see if we're on a P6
   mov     eax, isP6
   test    eax, eax
   jz      msrNotP6 
    
   ;; Check input size
   cmp     [esi + cbInBuffer], 12    ; MSRInfo Structure is 3 DWORDS
   jb      getmsrFailed              ;

   ;; Check output size
   cmp     [esi + cbOutBuffer], 12   ; MSRInfo Structure is 3 DWORDS
   jb      setmsrFailed              ; 

   ;; Check for nonzero input buffer
   mov     edi, dwp [esi + lpvInBuffer]
   test    edi, edi                  ; Test for zero
   jz      setmsrFailed              ; zero is bad
   
   ;; Check for nonzero output buffer
   mov     ebx, dwp [esi + lpvOutBuffer]
   test    ebx, ebx                  ; Check for zero
   jz      setmsrFailed              ; BAD zero! BAD!
   
   ;; Get the output buffer and squirrel 
   mov     eax, [esi + lpvOutBuffer] ; buffer
   mov     outputBuffer, eax         ; save it away

   ;;
   ;; Set the MSR
   ;;
   ;; At this point, ebx contains the output buffer, and edi contains
   ;; the input buffer
   
   mov     ecx, dwp [edi]            ; rdmsr expects MSR in ecx
   mov     eax, dwp [edi + 4]        ; Load the lo DWORD
   mov     edx, dwp [edi + 8]        ; load the hi DWORD

   wrmsr

setmsrSuccess:  
   mov     [ebx], ecx                ; Save the msr number to indicate sucess
   jmp     devctrl_okay              ; Jump to success point
       
setmsrFailed:  
   Trace_Out "FXMEMMAP(SETMSR):  Failure"
   mov     [ebx], 0FFFh              ; Indicate failure
   jmp     devctrl_fail              ; Jump to failure point
    
msrNotP6:       
   Trace_Out "FXMEMMAP(MSR):  Not P6"
   jmp       devctrl_fail   

;;;----------------------------------------------------------------------------
;;;  ECX = 8 (DECREMENTMUTEX)
;;;
;;;  Input:    NONE
;;;  Output:   NONE
decrementmutex:
   mov     al, MutexCount
   test    al, al
   Trace_Out "FXMEMMAP(DecrementMutex): #al"
    
   jz      pastDecDM
   lock    dec MutexCount            ; Decrement instance count for
                                     ; Glide in a window Mutex
pastDecDM:  
   jmp     devctrl_okay              ; Jump to success point    

;------------------------------------------------------------------------------
;    ECX = 9 (SETPASSTHROUGHBASE)
;
;    Input:
;   0 (DWORD) Base of hw address space
;   1 (DWORD) Length of hw address space 
;    Output:
;   None
setpassthroughbase:
   Trace_Out "FXMEMMAP(SETPASSTHROUGHBASE)"

   ;; Check input size
   cmp     [esi + cbInBuffer], 4     ; We only expect the 32-bit address
   jb      setpassthroughexit        ;
    
   ;; Check for nonzero input buffer
   mov     edi, dwp [esi + lpvInBuffer]
   test    edi, edi                  ; Test for zero
   jz      setpassthroughexit        ; zero is bad

   ;; Set the base address
   mov     eax, dwp [edi]
   mov     sstBaseAddr, eax
   Trace_Out "FXMEMMAP(SETPASSTHROUGHBASE): HW Base Addr: #eax"

   ;; Set the hw length for checking in the
   ;; page table diddling code later
   mov     eax, dwp [edi + 4]
   mov     sstBaseLen, eax
   Trace_Out "FXMEMMAP(SETPASSTHROUGHBASE): HW Len: #eax"

setpassthroughexit:
   jmp     devctrl_okay              ; This always succeeds.    

;;------------------------------------------------------------------------------
;;    ECX = 10 (SetAddrPerm)
;;
;    Input:
;   0 (DWORD) Linear address
;   1 (DWORD) Size of range to change permissions on
;   2 (DWORD) Permission bits to set (see vmm.h)
;    Output:
;   None
SetAddrPerm:
   Trace_Out "FXMEMMAP(SetAddrPerm)"

   ;; Check input size
   cmp     [esi + cbInBuffer], 12    ; 3 dwords thank you.
   jb      __addrPermExit
    
   ;; Check for nonzero input buffer
   mov     edi, dwp [esi + lpvInBuffer]
   test    edi, edi                  ; Test for zero
   jz      __addrPermExit

    ;; Grab the parameters
    mov     eax, [edi]               ; Base Linear Address
    mov     ecx, [edi + 4]           ; Length of range in bytes
    mov     edx, [edi + 8]           ; New permission.

   Trace_Out "FXMEMMAP(SetAddrPerm): Addr: (#eax : #ecx) : Perm: #edx"

    ;; Check to see that these are in the bounds of the current
    ;; hw base. This is a 'policy' decision, but so is CryBabyGlide
    cmp     eax, sstBaseAddr
    jl      __addrPermExit

    cmp     ecx, sstBaseLen
    jg      __addrPermExit

    ;; Convert the address to 0x1000 byte pages
    shr     eax, 12
    shr     ecx, 12

    ;; Mask off the user writeable bit
    mov     esi, PC_USER
    not     esi

    ;; Change the permissions by clearing all of the current
    ;; permission bits and or-ing in our new bits.
    Trace_Out "FXMEMMAP(SetAddrPerm): _PageModifyPermissions(#eax, #ecx, #esi, #edx)"        
    VMMCall _PageModifyPermissions <eax, ecx, esi, edx>
    Trace_Out "FxMemmap(SetAddrPerm):    -> #eax"

__addrPermExit: 
   jmp     devctrl_okay              ; This always succeeds.    

;------------------------------------------------------------------------------
devctrl_okay:
   xor   eax, eax                    ; indicate success
   clc
   ret                               ; return to VWIN32

devctrl_fail:
   Trace_Out "FxMemMap(devCtrl): ERROR_NOT_SUPPORTED: #ecx"
   mov   eax, 50                     ; ERROR_NOT_SUPPORTED
   stc                               ; indicate error
   ret                               ; return to VWIN32

EndProc  devctrl


;------------------------------------------------------------------------------
ifdef VGASWITCH
;
;
; The (almost) undocumented Hooks into various VxDs and VMM
;
;

VDD_PIF_STATE      equ 0A0001h
VDD_SET_VMTYPE     equ 0A0004h       ; should be in vdd.inc
VMM_SYSTEM_CONTROL equ 010093h       ; should be in vmm.inc
SCANCODE_F9    equ 000043h
CTRL_KEY       equ 040004h

;------------------------------------------------------------------------------
;
; We Hook a few VM and VDD messages.
;
;
; EAX = the SST-1 Linear Address.
; EBX = the current VM handle
;
; All registers are fair game.
; 
; Save SST base address in local store, and hook a few VM/VDD messages.

HookProcs:
   Trace_Out "FXMEMMAP: Enter Hook Process"
   mov   edx, [myVMHandle]
   test  edx, edx
   jnz   hook_done                   ; already hooked!

   mov   myVMHandle, ebx             ; my VM handle
   VMMCall Get_Sys_VM_Handle
   mov   sysVMHandle, ebx            ; the system VM handle
   mov   ebx, [myVMHandle]

   Trace_Out "FXMEMMAP: Hooking VDD_Set_VMType vm = #ebx"
   mov   eax, VDD_Set_VMType         ; VDD_Set_VMType hook
   mov   esi, OFFSET32 mySetVMType
   VMMCall Hook_Device_Service
   jc    hook_failed

   Trace_Out "FXMEMMAP: Hooking VDD_PIF_State vm = #ebx"
   mov   eax, VDD_PIF_State          ; VDD_PIF_State hook
   mov   esi, OFFSET32 myPifState
   VMMCall Hook_Device_Service
   jc    hook_failed

   Trace_Out "FXMEMMAP: Hooking VMM_SYS_CONTROL vm = #ebx"
   mov   eax, VMM_System_Control     ; VMM_System_Control hook
   mov   esi, OFFSET32 mySystemControl
   VMMCall Hook_Device_Service
   jc    hook_failed

   Trace_Out "FXMEMMAP: Hooking Keyscans CTRL_F9"
   mov   eax, SCANCODE_F9            ; scan code for F9
   mov   ebx, CTRL_KEY               ; shift state = CTRL
   mov   ecx, CallOnPress            ; when key pressed.
   mov   edx, 1                      ; reference data
   mov   esi, OFFSET32 HotKeyProc    ; whom to call 
   xor   edi, edi                    ; elapsed time
   VxDCall VKD_Define_Hot_Key
   jc    hook_failed
   mov   [hHotKey], eax
    
    
   mov   eax, [myVMHandle]
   Trace_Out "FXMEMMAP: Hooked all procs: VM = #eax"
hook_done:
   clc
hook_failed:
   ret

;------------------------------------------------------------------------------
;
; We unhook every message that was hooked above.
;
; All registers are fair game.
UnhookProcs:
   Trace_Out "FXMEMMAP:  UnhookProcs"
   mov   eax, [myVMHandle]
   test  eax, eax
   jz    unhook_done                 ; wasn't hooked

   Trace_Out "FXMEMMAP:  UnhookProcs:  VDD_Set_VMType"
   mov   eax, VDD_Set_VMType
   mov   esi, OFFSET32 mySetVMType
   VMMCall Unhook_Device_Service
   jc    @1
   Trace_Out "Unhooked VDD_Set_VMType"
   jmp   @2
@1: 
   Trace_Out "Unhook VDD_Set_VMType failed"

@2: 
   Trace_Out "FXMEMMAP:  UnhookProcs:  VDD_PIF_State"  
   mov   eax, VDD_PIF_State
   mov   esi, OFFSET32 myPifState
   VMMCall Unhook_Device_Service
   jc    @3
   Trace_Out "Unhooked VDD_PIF_State"
   jmp   @4
@3: 
   Trace_Out "Unhook VDD_PIF_State failed"
@4:
    
   Trace_Out "FXMEMMAP:  UnhookProcs:  VMM_System_Control" 
   mov   eax, VMM_System_Control
   mov   esi, OFFSET32 mySystemControl
   VMMCall Unhook_Device_Service
   jc    @5
   Trace_Out "Unhooked VMM_System_Control"
   jmp   @6
@5: 
   Trace_Out "Unhook VMM_System_Control failed"
@6:

   Trace_Out "FXMEMMAP:  UnhookProcs:  VKD_Remove_Hot_Key" 
   mov   eax, [hHotKey]
   VxDCall VKD_Remove_Hot_Key
   jc    @7
   Trace_Out "Unhooked VKD_Remove_HotKey"
   jmp   @8
@7: 
   Trace_Out "Unhook VKD_Remove_HotKey failed"
@8:    
       
   Trace_Out "FXMEMMAP: UnHooked all procs"
unhook_done:
   Trace_Out "FXMEMMAP:  UnhookProcs unhook_done"
   clc

unhook_failed:
   mov   [myVMHandle], 0    
   ret

;------------------------------------------------------------------------------
;
; Process the HotKey (F9).
;
;
;
;------------------------------------------------------------------------------
HotKeyProc:
   Trace_Out "FXMEMMAP: HotKeyProc: eax=#eax"
   cmp    ah, 0
   jne    @F                         ; 0 -> key pressed.
   mov    eax, [sstBaseAddr]
   test   eax, eax
   jz     @F                         ; sstbase not available.
   jmp    Toggle
@@: ret

;------------------------------------------------------------------------------
;
; These process the 3 hooks I've installed above:
;   VMM_System_Control
;   VDD_Set_VMType
;   VDD_PIF_State
;
;------------------------------------------------------------------------------
;
; eax = subfunction, we're interested in SET_DEVICE_FOCUS
; ebx = VM handle
; esi = VID
;
BeginProc mySystemControl, HOOK_PROC, pPrevSystemControl, LOCKED
   cmp eax, SET_DEVICE_FOCUS
   jne @F

   test    edx, edx
   jz  checkVM                       ; broadcast message.

   cmp edx, 00ah                     ; VDD 
   je  checkVM

   jmp @F                            ; not interested.

checkVM:
   pushad
   pushfd
   Trace_Out "FXMEMMAP: SetDevFocus: VMH = #ebx, VID= #edx, Flags= #esi"
   popfd
   popad

   call    [pPrevSystemControl]
   pushfd                            ; save flags 

   call    Switch    
   popfd
   ret
@@: jmp [pPrevSystemControl]         ; not interested.
EndProc mySystemControl
;------------------------------------------------------------------------------
BeginProc mySetVMType, HOOK_PROC, pPrevSetVMType, LOCKED
   call    [pPrevSetVMType]
   pushfd
   Trace_Out "FXMEMMAP: setVMType"
   call    Switch
   popfd
   ret
EndProc mySetVMType
;------------------------------------------------------------------------------
BeginProc myPifState, HOOK_PROC, pPrevPifState, LOCKED
   call    [pPrevPifState]           ; chain to legitimate hook.
   pushfd
   Trace_Out "FXMEMMAP: setPIFstate"
   call    Switch
   popfd
   ret
EndProc myPifState

;------------------------------------------------------------------------------
; 
; This routine decides who controls the monitor - the VGA or the SST.
;
FBIINIT0 equ        210h
FBIINIT1 equ        214h
PASSTHRU equ        01h
BLANKEN  equ        01000h  

Switch:
   pushad

   VxdCall VDD_Get_VM_Info           ; edi = CRTC, esi = MEMC

   mov ecx, [myVMHandle]
   mov ebx, [sysVMHandle]
   cmp ecx, ebx
   je  Win32App                      ; If sysVM == myVM, I must be Win32
   jmp PMApp                         ; else, I must be DOS/Protected Mode 
                                     ; 
PMApp:
   cmp edi, ecx                      ; myH == CRTC ? 
   jne lVGA                          ; I don't own CRTC, VGA owns monitor
   jmp SST


Win32App:
   cmp edi, ecx                      ; myH == CRTC ? 
   je  lVGA                          ; I don't own CRTC, VGA owns monitor
   jmp SST

SST:    ; SST controls monitor
   ; FbiInit0   Bit( 0) == 0 -> VGA controls monitor else SST controls it
   ; FbiInit1   Bit(12) == 0 -> normal blanking, else always blank.
   Trace_Out "FXMEMMAP: To SST: sys=#ebx myH = #ecx, CRTC = #edi, MEMC = #esi"
   mov edx, [sstBaseAddr]
   mov eax, [edx + FBIINIT0]         ; turn on passthru
   or  eax, PASSTHRU
   mov [edx + FBIINIT0], eax 
    
   mov eax, [edx + FBIINIT1]         ; turn on blanking.
   and eax, NOT(BLANKEN)
   mov [edx + FBIINIT1], eax
   popad
   ret

lVGA:    ; VGA controls monitor.
    Trace_Out "FXMEMMAP: To lVGA: sys=#ebx myH = #ecx, CRTC = #edi, MEMC = #esi"
    Trace_out "FXMEMMAP:         EAX: #eax  EDX: #edx"      
    mov edx, [sstBaseAddr]
    mov eax, [edx + FBIINIT0]        ; turn off passthru
    and eax, NOT(PASSTHRU)  
    mov [edx + FBIINIT0], eax 
    mov eax, [edx + FBIINIT1]        ; turn off blanking
    or  eax, BLANKEN
    mov [edx + FBIINIT1], eax
    popad
    Trace_out "FXMEMMAP:  Returnig from To VGA"
    ret

;******************************************************************************
;
; The following is used by the HotKey for toggling passthru.
;
Toggle:
    pushad
    mov edx, [sstBaseAddr]
    mov eax, [edx + FBIINIT0]
    Trace_Out "FXMEMMAP: Toggling state of Passthru from #eax"
    test    eax, PASSTHRU
    jz  SST                          ; prev VGA, now SST.
    jmp lVGA                         ; prev SST, now VGA.

;;;***************************************************************************
;;; 
;;;   PM & V86 API_Procs.
;;; 
;;;   DESCRIPTION:
;;; 
;;;   This is the exported API procedure that is callable from VM's. 
;;;   An application needs only to use INT 2Fh, AX=1684h, BX=device ID
;;;   (3df0) and a call back address is returned. Then, when the
;;;   address is called, eventually it ends up here.
;;; 
;;;
;;;   CLIENT:
;;;   bx:ax -> ptr to input data 
;;;   dx:cx -> ptr to output data
;;;   si -> Function number
;;;    
;;;===========================================================================

BeginProc API_Proc
   pushad
   push    ebx
   
ifdef DEBUG 
   int 1       
endif

   Trace_OUT "API_Proc: Entry"
    
   ;; Check to see if EAX is of the form 0x??000000

   mov     ax, word ptr [ebp.Client_AX]
   test    ax, ax
   jnz     CheckForCmdType

   mov     bx, word ptr [ebp.Client_BX]
   test    bl, bl
   jnz     CheckForCmdType

   ;; We got here, so that means we have something that looks like a PCI 
   ;; address in Client BX:AX
        
   ;; Load client's dx:cx into ebx
   mov     bx, [ebp.Client_DX]
   shl     ebx, 16
   mov     bx, [ebp.Client_CX]
   
   cmp     ebx, 01000000h   ; 16 Mb?
   je      ProcMapPhys2
   
CheckForCmdType:        
   xor     eax, eax  
   mov     ax, word ptr [ebp.Client_AX]
   Trace_Out 'Client ax = #eax'
       
   xor     eax, eax  
   mov     ax, word ptr [ebp.Client_BX]
   Trace_Out 'Client bx = #eax'
       
   xor     eax, eax  
   mov     ax, word ptr [ebp.Client_CX]
   Trace_Out 'Client cx = #eax'
       
   xor     eax, eax  
   mov     ax, word ptr [ebp.Client_DX]
   Trace_Out 'Client dx = #eax'
       
   xor     eax, eax  
   mov     ax, word ptr [ebp.Client_SI]
   Trace_Out 'Client si = #eax'
    
   ;; Get the function
   ;;
   ;; FIXME!! Make this a table!
   xor     eax, eax 
   mov     ax, word ptr [ebp.Client_SI] 
   cmp     eax, 1
   je      ProcMapPhys               ; It's ProcMapPhys

   cmp     eax, 2
   je      ProcGetMSR                ; It's ProcGetMSR

   cmp     eax, 3
   je      ProcSetMSR                ; It's ProcSetMSR

   cmp     eax, 4
   je      ProcGetVersion            ; It's ProcGetVersion

   cmp     eax, 5                    ; It's ProcUnmapPhys
   je      ProcUnmapPhys

   cmp     eax, 6
   je      ProcMapPhys16             ; It's ProcMapPhys16

   cmp     eax, 7
   je      ProcGetMSR16                ; It's ProcGetMSR16

   cmp     eax, 8
   je      ProcSetMSR16                ; It's ProcSetMSR16

   cmp     eax, 9
   je      ProcGetVersion16          ; It's ProcGetVersion16

   cmp     eax, 10                   
   je      ProcUnmapPhys             ; It's ProcUnmapPhys16

   cmp     eax, 11
   je      ProcIncrementMutex16      ; It's ProcIncrementMutex16

   cmp     eax, 12                   
   je      ProcDecrementMutex16      ; It's ProcDecrementMutex16

   cmp     eax, 13
   je      ProcSetPassThroughBase16

   cmp     eax, 14
   je      ProcOutputDebugString16

   cmp     eax, 15
   je      ProcSetAddrPerm16

   Trace_OUT "API_Proc: Unknown call (#eax)"    
   pop     ebx  
   jmp     ProcSuccess               ;

;;;===========================================================================;;;   ProcMapPhys:
;;; 
;;;   ebp -> Client data area
;;;   ebx -> Current VMCB
;;; 
;;;   CLIENT: bx:ax = phys addr, dx:cx = phys size, si = 1
;;; 
ProcMapPhys:    
   ;
   ; Map this physical address into linear memory

   ;
   ; eg: VMMCall  _MapPhysToLinear <0fb000000h, 01000000h, 0> 
   ; input   eax = physical address
   ;     ebx = physical size
   ; returns eax = ring 0 linear address
   ;     ecx = used
   ;     edx = used

   ;; Increment the Mutex Count
   Trace_Out "FXMEMMAP: ProcMapPhys: Incrementing Mutex"
   lock   inc MutexCount             ; Increment instance count for
    
   mov     ax, word ptr [ebp.Client_BX]
   shl     eax, 16
   mov     ax, word ptr [ebp.Client_AX] ; eax now has phys addr.
    
   and     ax, 0feh                     ; Clear the low bit set by PCILib
    
   mov     bx, word ptr [ebp.Client_DX]
   shl     ebx, 16
   mov     bx, word ptr [ebp.Client_CX]

   Trace_Out "FXMEMMAP: API_Proc eax = #eax, ebx = #ebx"
   ;
   ; eax = phys addr, ebx = phys size, which is the way 
   ; _MapPhysToLinear works.
   ;
   mov     physAddr, eax
   mov     physSize, ebx  
    
   VMMCall _MapPhysToLinear <eax, ebx, 0> 
   cmp     eax, -1             ; is memory addressable?
   je      failedProc0
   Trace_Out "FXMEMMAP: API_Proc eax = #eax, ebx = #ebx, done"
    
   ; 
   ; Check to see if we're on a P6, and enable MTRRs for physical address
   ; This code could fail, but we won't change any registers.
   ; assumes eax = physical address.
       
   ; put eax and ebx back the way they P6Stuff wants 'em
   mov     eax, physAddr      
   mov     ebx, physSize
    
   pop     ebx                       ; The original VM.
   call    HookProcs                ; set up hooks if necessary
   jc      failedProc1

   jmp     succeedProc
       
failedProc0:
   pop     ebx
failedProc1:
   Trace_Out "FXMEMMAP: ApiProc() failed!"
succeedProc:   
   popad
   or     [ebp.Client_Flags], CF_Mask; set VM's carry flag
   ret
;;; End of ProcMapPhys
;;;===========================================================================    

;;;===========================================================================
;;;   ProcMapPhys16:
;;; 
;;;   ebp -> Ptr to client data area
;;;   ebx -> Current VMCB
;;; 
;;;   CLIENT: bx:ax = phys addr, dx:cx = phys size, si = 1
;;; 
ProcMapPhys16:    
   ;
   ; Map this physical address into linear memory

   ;
   ; eg: VMMCall  _MapPhysToLinear <0fb000000h, 01000000h, 0> 
   ; input   eax = physical address
   ;     ebx = physical size
   ; returns eax = ring 0 linear address
   ;     ecx = used
   ;     edx = used
   ;; Increment the Mutex Count
   Trace_Out "FXMEMMAP: ProcMapPhys16: Incrementing Mutex"
   lock   inc MutexCount             ; Increment instance count for
        
   mov     ax, word ptr [ebp.Client_BX]
   shl     eax, 16
   mov     ax, word ptr [ebp.Client_AX] ; eax now has ptr to client buffer
    
   mov     ebx, eax                     ; Stash the buffer pointer
   mov     eax, [eax]                   ; Set the vmm physical address parameter

   ;; Save the real address to check to see if we should
   ;; hook or not.
   mov     temp1, eax        
   and     ax, 0feh                     ; Clear the low bit set by PCILib

   mov     temp, ebx
   mov     [ebx], 0                     ; Clear the return value    

    ;; Extract the length parameter
   mov     bx, word ptr [ebp.Client_DX]
   shl     ebx, 16
   mov     bx, word ptr [ebp.Client_CX]

   Trace_Out "FXMEMMAP: _MapPhysToLinear eax = #eax, ebx = #ebx"
   ;
   ; eax = phys addr, ebx = phys size, which is the way 
   ; _MapPhysToLinear works.
   ;
   mov     physAddr, eax
   mov     physSize, ebx  
    
   VMMCall _MapPhysToLinear <eax, ebx, 0> 
   cmp     eax, -1             ; is memory addressable?
   je      failedMap16_0
   Trace_Out "FXMEMMAP: _MapPhysToLinear: (Addr: #eax) (Length: #ebx)"

   ;; Save the linear addresss for the caller.
   mov     ebx, temp
   mov     [ebx], eax

   ;; Test to see if we should be hooking or not
   ;; If the low order bit is set then SST_DUALHEAD
   ;; is set and we should not hook for passthrough.
   mov     eax, temp1
   and     eax, 01h
   pop     ebx                      ; The original VM.
   jz      succeedMap16
    
   call    HookProcs                ; set up hooks if necessary
   jc      failedMap16_1
   jmp     succeedMap16
       
failedMap16_0:
   pop     ebx
    
failedMap16_1:
   Trace_Out "FXMEMMAP: ProcMapPhys16() failed!"
    
succeedMap16:
   popad        
   or     [ebp.Client_Flags], CF_Mask; set VM's carry flag
   ret
;;; End of ProcMapPhys
;;;===========================================================================
    
;;;===========================================================================
;;; ProcGetMSR:    
;;; 
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS, si -> 2
;;;
;;; CLIENT OUTPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS
;;;
;;; inS and outS look like:
;;;    DWORD 0:    MSR Num (ecx before & after rdmsr)
;;;    DWORD 1:    MSRLo   (eax after rdmsr)
;;;    DWORD 2:    MSRHi   (edx after rdmsr)
ProcGetMSR:
   ;; 
   ;; Get the input data
   ;;
   
   ;; Check for P6
   mov     eax, isP6
   test    eax, eax
   jz      ProcSuccess               ; no need if not P6
   
   ;; Put hi 16 bits of input buffer address in bx, then shift left
   ;; into upper 16 bits of eax
   mov     bx, [ebp.Client_BX]
   shl     ebx, 16

   ;; Put lo 16 bits of input buffer address in bx
   mov     bx, [ebp.Client_AX]       ;; eax now contains input buffer address

   ;; test for NULL input
   test    ebx, ebx                  ; test for zero
   jz      ProcGetMSRFailed          ; zero is bad

   ;;
   ;; Get the output data
   ;;
    
   ;; Put hi 16 bits of input buffer address in si, then shift left
   ;; into upper 16 bits of esi
   mov     si, [ebp.Client_DX]
   shl     esi, 16

   ;; Put lo 16 bits of input buffer address in si
   mov     si, [ebp.Client_CX]       ; esi now contains output buffer address
   
   ;; test for NULL output data
   test    esi, esi                  ; test for zero
   jz      ProcGetMSRFailed          ; zero is bad
       
   ;; Load ecx with MSR Number
   mov     ecx, dwp [ebx]

   rdmsr                             ; Read the MSR
    
   ;; 
   ;; Store the data
   ;; 

   ;; Store MSRLo
   mov     [esi + 0], ecx            ; 
   mov     [esi + 4], eax
   mov     [esi + 8], edx
    
   jmp     ProcSuccess

ProcGetMSRFailed: 
   Trace_Out "ProcGetMSR Failed!"
   pop     ebx  
   popad
   ret
   
;;; End of ProcGetMSR
;;;===========================================================================
    

;;;===========================================================================
;;; ProcGetMSR16:    
;;; 
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS, si -> 2
;;;
;;; CLIENT OUTPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS
;;;
;;; inS and outS look like:
;;;    DWORD 0:    MSR Num (ecx before & after rdmsr)
;;;    DWORD 1:    MSRLo   (eax after rdmsr)
;;;    DWORD 2:    MSRHi   (edx after rdmsr)
ProcGetMSR16:         
    ;; Check for P6
    mov     eax, isP6
    test    eax, eax
    jz      ProcSuccess               ; no need if not P6

    ;; Get the input data
    mov     ax, [ebp.Client_BX] ; selector
    ;; test for NULL input
    test    ax, ax                  ; test for zero
    jz      ProcGetMSR16Failed          ; zero is bad
    push es     
    mov     es,ax
    xor     ebx,ebx
    mov     bx, [ebp.Client_AX]  ; es:bx now has ptr to input data
           
    ;; Load ecx with MSR Number
    mov     ecx, dwp es:[ebx]

    rdmsr                             ; Read the MSR

    ;; Get pointer to the output data
    mov     bx, [ebp.Client_DX]       ; set es for the output data
    mov     es, bx
    xor     ebx,ebx
    mov     bx, [ebp.Client_CX]       ; es:bx now has ptr to output data
       
    ;; Store the data    ; Store to 16:16 output buffer

    mov     dwp es:[ebx], ecx     ; Address
    mov     dwp es:[ebx+4], eax ; MSRLo
    mov     dwp es:[ebx+8], edx ; MSRHi
    
    pop es
    jmp     ProcSuccess

ProcGetMSR16Failed: 
   Trace_Out "ProcGetMSR16 Failed!"
   pop     ebx  
   popad
   ret
   
;;; End of ProcGetMSR16
;;;===========================================================================

    ;; ProcSetPassThroughBase16
    ;;
    ;; ClientInput:
    ;;  bx:ax -> Base address to set as pass through
    ;;  dx:cx -> Length of the hw address space
    ;;
    ;; ClientOutput:
    ;;  None

    ;; Get base address from 16 bit land.
ProcSetPassThroughBase16:
    Trace_Out "ProcSetPassThroughBase16: Entry"
    
    mov     bx, [ebp.Client_BX]
    shl     ebx, 16
    mov     bx, [ebp.Client_AX]

    mov     ax, [ebp.Client_DX]
    shl     eax, 16
    mov     ax, [ebp.Client_CX]

    Trace_Out "ProcSetPassThroughBase16: Base: #ebx Len: #eax"

    ;; Set the main base address for pass through
    mov     sstBaseAddr, ebx
    mov     sstBaseLen, eax

    ;; This call always succeeds so just go back to Dos
    jmp     ProcSuccess

    ;; ProcSetAddrPerm16
    ;; 
    ;; ClientInput:
    ;;  bx:ax -> Pointer to array describing memory to
    ;;           change the permissions on.
    ;;          [0]:    Base Address
    ;;          [1]:    Length of range
    ;;          [2]:    New permissions bits
    ;; 
    ;; ClientOutput:
    ;;  Boolean success or failure.

ProcSetAddrPerm16:
    Trace_Out "ProcSetAddrPerm16: Entry"

    ;; Get Pointer to client info.
    mov     bx, [ebp.Client_BX]
    shl     ebx, 16
    mov     bx, [ebp.Client_AX]

    Trace_Out "ProcSetAddrPerm16: ClientInfo #ebx"

    lea     esi, addrPerm16Buf

    ;; Setup the params for the normal vxd call
    mov     [esi + cbInBuffer], 12
    mov     [esi + lpvInBuffer], ebx

    call    SetAddrPerm

    ;; This call always succeeds so just go back to Dos
    jmp     ProcSuccess

    ;; ProcOutputDebugString16
    ;;
    ;; ClientInput:
    ;;  bx:ax -> NUL terminated string
    ;;
    ;; ClientOutput:
    ;;  None

    ;; Get base address from 16 bit land.
ProcOutputDebugString16:
    mov     bx, [ebp.Client_BX]
    shl     ebx, 16
    mov     bx, [ebp.Client_AX]

    Trace_Out ebx    

    ;; This call always succeeds so just go back to Dos
    popad
    pop     ebx  
    ret    
    
;;;===========================================================================
;;; ProcSetMSR:
;;;
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS, si -> 2
;;;
;;; CLIENT OUTPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS
;;;
;;; inS and outS look like:
;;;    DWORD 0:    MSR Num (ecx before & after rdmsr)
;;;    DWORD 1:    MSRLo   (eax after rdmsr)
;;;    DWORD 2:    MSRHi   (edx after rdmsr)
;;;
ProcSetMSR:
   ;; Check for P6
   mov     eax, isP6
   test    eax, eax
   jz      ProcSuccess               ; no need if not P6
   
   ;;
   ;; Load regs needed by wrmsr
   ;;
    
   ;; Put hi 16 bits of input buffer address in bx, then shift left
   ;; into upper 16 bits of eax
   mov     bx, [ebp.Client_BX]
   shl     ebx, 16

   ;; Put lo 16 bits of input buffer address in bx
   mov     bx, [ebp.Client_AX]       ;; eax now contains input buffer address

   ;; test for NULL input
   test    ebx, ebx                  ; test for zero
   jz      ProcSetMSRFailed          ; zero is bad

   ;;
   ;; Get the output data
   ;;
    
   ;; Put hi 16 bits of input buffer address in si, then shift left
   ;; into upper 16 bits of esi
   mov     si, [ebp.Client_DX]
   shl     esi, 16

   ;; Put lo 16 bits of input buffer address in si
   mov     si, [ebp.Client_CX]       ; esi now contains output buffer address
   
   ;; test for NULL output data
   test    esi, esi                  ; test for zero
   jz      ProcSetMSRFailed          ; zero is bad

   ;; Load ecx with MSR Number
   mov     ecx, dwp [ebx]
   ;; Load eax with MSRLo
   mov     eax, dwp [ebx + 4]
   ;; Load edx with MSRHi
   mov     edx, dwp [ebx + 8]
    
   wrmsr                             ; Write the MSR

   jmp     ProcSuccess               ; Done!

ProcSetMSRFailed: 
   Trace_Out "ProcSetMSR Failed!"
   popad
   pop     ebx     
   ret
;;; End of ProcSetMSR
;;;===========================================================================
    
;;;===========================================================================
;;; ProcSetMSR16:
;;;
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr to outS, si -> 2
;;;
;;; inS looks like:
;;;    DWORD 0:    MSR Num (ecx before wrmsr)
;;;    DWORD 1:    MSRLo   (eax before wrmsr)
;;;    DWORD 2:    MSRHi   (edx before wrmsr)
;;;
ProcSetMSR16:
   ;; Check for P6
   mov     eax, isP6
   test    eax, eax
   jz      ProcSuccess               ; no need if not P6
   
   ;; Get pointer to input data
    mov     ax, [ebp.Client_BX] ; selector
    ;; test for NULL input
    test    ax, ax                  ; test for zero
    jz      ProcSetMSR16Failed          ; zero is bad
    push es     
    mov     es,ax
    xor ebx,ebx
    mov     bx, [ebp.Client_AX]  ; es:bx now has ptr to input data

          
    ;; Load ecx with MSR Number
    mov     ecx, dwp es:[ebx]
    ;; Load eax with MSRLo
    mov     eax, dwp es:[ebx + 4]
    ;; Load edx with MSRHi
    mov     edx, dwp es:[ebx + 8]
    pop es
    
    wrmsr                             ; Write the MSR

    jmp     ProcSuccess               ; Done!
   
ProcSetMSR16Failed: 
   Trace_Out "ProcSetMSR16 Failed!"
   popad
   pop     ebx     
   ret
;;; End of ProcSetMSR16
;;;===========================================================================
    
;;;===========================================================================
;;; ProcGetVersion:
;;;
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS, si -> 2
;;;
;;; CLIENT OUTPUT:
;;; bx:ax -> 0, dx:cx -> ptr to version
;;;
;;;
ProcGetVersion:
   ;; Move pointer to output data into eax  
   mov     ax, [ebp.Client_DX]
   shl     eax, 16
   mov     ax, [ebp.Client_CX]  ; eax now has ptr to output data

   ;; Check for Null output pointer
   test    eax, eax                  ; test for zero
   jz      ProcGetVersionFailed      ; Bad zero! Bad!

   ;; Load the version into ebx
   xor     ebx, ebx                  ; clear ebx
   mov     bl, MutexCount            ;
   shl     ebx, 8                    ; 15:8  - Mutex, 7:0 - clear
   mov     bl, fxmajor_ver           ; 15:8  - Mutex, 7:0 - major
   shl     ebx, 8                    ; 23:16 - Mutex, 15:8 - major, 7:0 - clear
   mov     bl, fxminor_ver           ; 23:16 - Mutex, 15:8 - major, 7:0 - minor

   mov     [eax], ebx                ; Store version to output buffer
   Trace_Out "ProcGetVersion Success: #ebx"
   jmp     ProcSuccess               ; We're done!

ProcGetVersionFailed: 
   Trace_Out "ProcGetVersion Failed!"
   popad
   pop     ebx     
   ret
;;; End of ProcGetVersion
;;;===========================================================================
    
;;;===========================================================================
;;; ProcUnmapPhys
;;;
;;; This doesn't really unmap the board, but it does decrement the
;;; reference count
;;;
;;; CLIENT INPUT:  None
;;; CLIENT OUTPUT: None

ProcUnmapPhys:
   mov     al, MutexCount
   test    al, al
   jz      pastDecUP
   lock    dec MutexCount            ; Decrement instance count for
                                     ; Glide in a window Mutex

   ;; If we're done mapping the boards then reset the first board
   ;; address for the next mapping.
   sub     al, 1
   cmp     al, 0
   jne     pastDecUP

   Trace_Out "FXMEMMAP(ProcUnmapPhys): Clearing Hooks"
   call    UnhookProcs
                
pastDecUP:  
   jmp     ProcSuccess
;;; End of ProcUnmapPhys
;;;===========================================================================


;;;===========================================================================
;;; ProcGetVersion16: 
;;;
;;; CLIENT INPUT:
;;; bx:ax -> ptr to inS, dx:cx -> ptr ot outS, si -> 2
;;;
;;; CLIENT OUTPUT:
;;; bx:ax -> 0, dx:cx -> ptr to version
;;;
;;; Input/Output pointers are assumed to be 16:16
;;;
ProcGetVersion16:
   ;; Move pointer to output data into eax  

   mov     ax, [ebp.Client_DX]
   shl     eax, 16
   mov     ax, [ebp.Client_CX]  ; eax now has ptr to output data

   ;; Check for Null output pointer
   test    eax, eax                  ; test for zero
   jz      ProcGetVersionFailed16    ; Bad zero! Bad!

   ;; Load the version into ebx
   xor     ebx, ebx                  ; clear ebx
   mov     bl, MutexCount            ;
   shl     ebx, 8                    ; 15:8  - Mutex, 7:0 - clear
   mov     bl, fxmajor_ver           ; 15:8  - Mutex, 7:0 - major
   shl     ebx, 8                    ; 23:16 - Mutex, 15:8 - major, 7:0 - clear
   mov     bl, fxminor_ver           ; 23:16 - Mutex, 15:8 - major, 7:0 - minor

   mov     dx, es                    ; save es
   mov     ax, [ebp.Client_DX]       ; set es for the output data
   mov     es, ax
   xor     eax, eax
   mov     ax, [ebp.Client_CX]       ; eax now has ptr to output data
   mov     dwp es:[eax], ebx         ; Store to 16:16 output buffer
   Trace_Out "ProcGetVersion16: #ebx"
    
   mov     es, dx                    ; restore es

   jmp     ProcSuccess               ; We're done!

ProcGetVersionFailed16: 
   Trace_Out "ProcGetVersion Failed!"
   popad
   pop     ebx     
   ret
;;; End of ProcGetVersion16
;;;===========================================================================


;;;===========================================================================
;;; ProcIncrementMutex16
;;;
;;; This increments the reference count
;;;
;;; CLIENT INPUT:  None
;;; CLIENT OUTPUT: None

ProcIncrementMutex16:

   mov     al, MutexCount
   cmp     al, 0ffh                 ; clamp at 255
   je      pastProcIncMutex16       

   lock   inc MutexCount             ; Increment instance count for
                                     ; Glide in a window Mutex

pastProcIncMutex16:
   Trace_Out "ProcIncrementMutex16 "
   popad
   pop     ebx     
   ret
;;; End of ProcIncremntMutex16
;;;===========================================================================

;;;===========================================================================
;;; ProcDecrementMutex16
;;;
;;; This decrements the reference count
;;;
;;; CLIENT INPUT:  None
;;; CLIENT OUTPUT: None

ProcDecrementMutex16:

   mov     al, MutexCount
   test    al, al
   jz      pastProcDecMutex16        ; clamp at 0

   lock   dec MutexCount             ; Decrement instance count for
                                     ; Glide in a window Mutex
pastProcDecMutex16:
   Trace_Out "ProcDecrementMutex16 "
   popad
   pop     ebx     
   ret
;;; End of ProcDecrementMutex16
;;;===========================================================================

ProcSuccess:        
   Trace_Out "FXMEMMAP: ApiProc() OK!"
   popad
   pop     ebx  
   ret
    
;=========================== Old V1.1 Memmap Code
ProcMapPhys2:
         ;
         ; Map this physical address into linear memory
         ;
         ; eg: VMMCall  _MapPhysToLinear <0fb000000h, 01000000h, 0> 
         ; input   eax = physical address
         ;         ebx = physical size
         ; returns eax = ring 0 linear address
         ;         ecx = used
         ;         edx = used

        mov     ax, word ptr [ebp.Client_BX]
        shl     eax, 16
        mov     ax, word ptr [ebp.Client_AX]    ; eax now has phys addr.

        mov     bx, word ptr [ebp.Client_DX]
        shl     ebx, 16
        mov     bx, word ptr [ebp.Client_CX]

        Trace_Out "FXMEMMAP 1.1: API_Proc eax = #eax, ebx = #ebx"
        ;
        ; eax = phys addr, ebx = phys size, which is the way 
        ; _MapPhysToLinear works.
        ;
        mov  physAddr, eax
        mov  physSize, ebx      
        
        VMMCall _MapPhysToLinear <eax, ebx, 0> 
        cmp     eax, -1                         ; is memory addressable?
        je      failedProc20
        Trace_Out "FXMEMMAP 1.1: API_Proc eax = #eax, ebx = #ebx, done"
        

        ; 
        ; FILTHY HACK!!!
        ;
        ; To find out if it's SST1, write the first DWORD of memory,
        ; and then read it back.  If it's SST1, then we won't read
        ; back what we read.
        
        ;  First, squirrel away the first DWORD of memory, as someone
        ;  who would notice two botched pixels is probably not
        ;  someone we want to irritate (according to PGJ)
;        mov ebx, [eax]
;        mov temp, ebx
        
;        mov [eax], 0deadcafeh
;        mov ebx, [eax]
;        xor ebx, 0deadcafeh
;        jz pastP6StuffVR2 ;  jump past P6Stuff if it's VG96

        ; 
        ; Check to see if we're on a P6, and enable MTRRs for physical address
        ; This code could fail, but we won't change any registers.
        ; assumes eax = physical address.
       
        ; put eax and ebx back the way they P6Stuff wants 'em
        mov eax, physAddr      
        mov ebx, physSize
        call  P6Stuff_11
;        jmp   VGVRCheckDone2    
;   xor     ebx, ebx
;pastP6StuffVR2:    
;        ; put the first DWORD back the way we found it.
;        mov ebx, temp
;        mov [eax], ebx
;VGVRCheckDone2:  
        
        pop     ebx                             ; The original VM.
        call    HookProcs                       ; set up hooks if necessary
        jc      failedProc21

        Trace_Out "FXMEMMAP 1.1: ApiProc() OK!"
        popad
        and     [ebp.Client_Flags], NOT CF_Mask ; clear VM's carry flag
        ret

failedProc20:
        pop     ebx
failedProc21:
        Trace_Out "FXMEMMAP 1.1: ApiProc() failed!"
        popad
        or     [ebp.Client_Flags], CF_Mask ; set VM's carry flag
        ret
        
;******************************************************************************
;
; Detect whether we're running on the P6. If so, write privileged registers
; to map the 16MB address space as USWC, and the first 4K as UC.
;
;
; We can detect a P6 using the following mechanism:
;       First, determine whether CPUID instruction is available
;               [look to see if bit 21 of EFLAGS can be toggled]
;       If not, punt.
;       If CPUID is available,
;               Check to see if intel processor.
;               Check to see if version == 6. 
;
;       Do P6 specific stuff.
;
; Usage: P6Stuff        eax = physical address of Voodoo.
; 
; no registers are changed!
;
P6Stuff_11:
        pushad                          ; save all regs.
        Trace_Out "FXMEMMAP 1.1: Setting MTRR for P6: address #ebp"
        mov     ebp, eax                ; move to ebp, will be untouched there
        and     ebp, 0fffff000h         ; page align.
        jz      not_P6                  ; Bad physical address (0!)

        ;
        ; First, determine whether CPUID instruction is available.
        ;
        Trace_Out "FXMEMMAP 1.1: Is CPUID available?"
        pushfd                          ; push original EFLAGS.
        pop     eax                     ; pop into eax
        mov     ecx, eax                ; save original EFLAGS in ecx
        xor     eax, 0200000h           ; flip ID bit in EFLAGS
        push    eax                     ; put it back on stack
        popfd                           ; pop into EFLAGS
        pushfd                          ; get EFLAGS back
        pop     eax                     ; into eax
        xor     eax, ecx                ; check to see if we could toggle ID
        jz      not_P6_11                  ; Sorry, not P5 or P6.

        ;
        ; Now determine whether it's an intel P6 CPU.
        ;
        Trace_Out "FXMEMMAP 1.1: Is it an Intel CPU?"
        xor     eax, eax                ; eax = 0.
        cpuid                           ; get cpuid
        xor     ebx, 0756e6547h         ; "Genu"
        jnz     not_P6_11                  
        xor     edx, 049656e69h         ; "ineI"
        jnz     not_P6_11
        xor     ecx, 06c65746eh         ; "ntel"
        jnz     not_P6_11

        ;
        ; Intel processor, can we ask for model number?
        ;
        Trace_Out "FXMEMMAP 1.1: May we ask for version number?"
        cmp     eax, 1
        jl      not_P6_11                  ; can't ask for version! must not be P6

        ;
        ; Yes, you may ask for version number, verify family==6.
        ;
        Trace_Out "FXMEMMAP 1.1: Verifying architecture family"
        mov     eax, 1
        cpuid                           ; get family/model/stepping
        shr     eax, 8                  ; rid of model & stepping number
        and     eax, 0fh                ; use only family
        cmp     eax, 6
        jne     not_P6_11                  ; sorry, some other model

        ;
        ; Intel P6 processor. 
        ; Make sure it supports Memory Type Range Request registers
        ;
        Trace_Out "FXMEMMAP 1.1: Does this P6 support MTRRs?"
        test    edx, 01000h             ; bit 12 in edx = MTRR support
        jz      not_P6_11                  ; not supported!
        test    edx, 00020h             ; bit 5, supports MSR?
        jz      not_P6_11                  ; rdmsr, wrmsr not supported!

        ; Intel P6 processor identified.
        ;
        ; Work around P6 write combining problem with certain intel chipsets.
        ; Check registry entry to see if we should bypass the fix code.
        ;
        call    P6_CheckWcRegEntry              ; Check registry to find out
        jc      DontCheckForBrokenChipset_11       ; whether to bypass check and
        call    P6_CheckForBrokenIntelChipset   ; fix for broken Intel chipset
DontCheckForBrokenChipset_11:                      ; on a P6.

        ;
        ; Intel P6 processor identified. 
        ; We know this processor always supports 8 MTRR's and USWC modes. 
        ; So look through the 8 registers to see if any of them belong to us. 
        ; Also, look for 2 free registers in case we don't find any of ours.
        ;
        ;
        ; Walk backwards from the highest numbered MTRR (0x20B). 
        ; Stop when you fall below the lowest numbered MTRR    (0x200).
        ;
        ; esi will have 2 MTRRs that belong to our physical address.
        ; edi will have 2 MTRRs that are free.
        ;
        mov     ecx, 020Bh              ; MTRRphysMask5. (6 & 7 belong to OS)
        xor     esi, esi
        xor     edi, edi
        
mtrrLoop:
        rdmsr                           ; ecx odd, returns edx:eax = physMask
        test    eax, 0800h              ; valid bit set?
        jz      mtrrFree                ; not valid, process this as free MTRR

        ;
        ; This MTRR is busy. Does it have our physical address?
        ;
        dec     ecx                     ; ecx even, MTRRphysBase.
        rdmsr                           ; edx:eax = phys addr
        and     eax, 0fffff000h         ; mask off lower 12 bits
        cmp     eax, ebp                ; match our phys base address?
        jne     mtrrNext                ; not ours, go on to next mtrr.
        and     edx, 00000000fh         ; mask off lower 4 bits
        jnz     mtrrNext                ; some bits set, not ours, move on.

        ;
        ; We've located an MTRR that belongs to us (possibly set in a 
        ; previous invocation of the VxD).. Keep track in esi
        ;
        shl     esi, 16                 ; move old port# to high bits.
        or      esi, ecx                ; save new port# in low bits.
        test    esi, 0ffff0000h         ; do we have 2 MTRRs?
        jnz     mtrrFound2              ; yes
        jmp     mtrrNext                ; no, find another one.

mtrrFree:
        ;
        ; This mtrr is free. keep track, we may need it later.
        ;
        dec     ecx                     ; ecx even, MTRRphysBase
        shl     edi, 16                 ; move old free port# to high bits.
        or      edi, ecx                ; save new free port# in low bits.

mtrrNext:
        ;
        ; If we're not done, go back and look for more.
        ;
        dec     ecx                     ; ecx odd, MTRRphysMask
        cmp     ecx, 01ffh              ; dropped below MTRR0?
        jne     mtrrLoop

        ;
        ; We're done with the loop. 
        ; We either found 0, 1 or 2 MTRRs that belonged to us.
        ; If we had found 2 MTRRs we would have branched to mtrrFound2.
        ; For the other 2 situations, we use 1 or 2 mtrrs free from edi
        ;
        ; Also, put one MTRR in esi, the other in edi when we're done.

        test    esi, esi                ; test for 0
        jnz     mtrrFound1              ; no, found 1 MTRR that was ours.

mtrrFound0:
        ; 
        ; Found NO mtrrs that used to be ours. Make sure we have 2 free mtrrs
        ; in edi, copy into esi and continue
        ;
        mov     esi, edi                ; copy both to esi
        and     esi, 0ffffh             ; mask off first mtrr
        jz      mtrrFailed              ; not a valid mtrr
        shr     edi, 16                 ; move other free mtrr to low bits.
        test    edi, edi                ; is it valid?
        jz      mtrrFailed              ; not a valid mtrr
        jmp     mtrrFix                 ; found 2 free ones, go fix 'em.


mtrrFound1:
        and     edi, 0ffffh             ; mask off lower bits of free MTRR
        jz      mtrrFailed              ; not valid
        jmp     mtrrFix                 ; found 2 free ones, go fix 'em.

mtrrFound2:
        mov     edi, esi
        and     esi, 0ffffh             ; mask off first mtrr
        jz      mtrrFailed              ; not a valid mtrr
        shr     edi, 16                 ; move other mtrr to low bits
        test    edi, edi                ; is it valid?
        jz      mtrrFailed              ; not a valid mtrr
        jmp     mtrrFix                 ; found 2 free ones, go fix 'em.

mtrrFailed:
        Trace_Out "FXMEMMAP 1.1: Sorry, couldn't find 2 free MTRRs!"
        popad
        ret
        
        ; 
        ; When we get here: 
        ;       esi contains 1 MTRR port# we can use
        ;       edi contains another port# .
        ; Remember that both MTRRs above are even port addresses (physBase).
mtrrFix:
        ;
        ; first, disable both MTRRs.
        ;
        xor     eax, eax                ; eax = 0, edx = 0
        xor     edx, edx                ; this disables both ports below.

        mov     ecx, esi                ; port#0
        inc     ecx                     ; write to odd address, physMask
        wrmsr                           ; disable port #0.

        mov     ecx, edi                ; port#1
        inc     ecx                     ; write to odd address, physMask
        wrmsr                           ; disable port #1.

        ;
        ; Set port0 to be UC, port1 to be USWC
        ;
        xor     edx, edx                ; edx = top 4 bits of 36bit phys addr

        mov     ecx, esi                ; port#0
        mov     eax, ebp                ; physBase = ebp
        wrmsr                           ; port#0 is UC

        mov     ecx, edi                ; port#1
        mov     eax, ebp                ; physBase = ebp
        inc     eax                     ; mark USWC
        wrmsr                           ; port#1 is USWC

        ;
        ; Set port0 to be 4KB UC, port1 to be 16M USWC
        ; remember to write to odd addresses.
        ;
        mov     edx, 0fh                ; upper bits of 36 bit size field.
        
        mov     ecx, esi                ; port#0
        inc     ecx                     ; physMask for port
        mov     eax, 0fffff800h         ; 4KB size, valid port
        wrmsr

        mov     ecx, edi                ; port#1
        inc     ecx                     ; physMask for port
        mov     eax, 0ff000800h         ; 16MB size, valid port
        wrmsr
        
        ;
        ; done
        ;
        Trace_Out "FXMEMMAP 1.1: Done MTRRs for this P6!"
        popad
        ret

not_P6_11:
        Trace_Out "FXMEMMAP 1.1: Sorry, not P6: #eax #ebx #ecx #edx"
        popad
        ret

EndProc API_Proc


;;;***************************************************************************
;;;
;;; Detect whether we're running on the P6. If so, write privileged registers
;;; to map the 16MB address space as USWC, and the first 4K as UC.
;;;
;;;
;;; We can detect a P6 using the following mechanism:
;;;   First, determine whether CPUID instruction is available
;;;       [look to see if bit 21 of EFLAGS can be toggled]
;;;   If not, punt.
;;;   If CPUID is available,
;;;       Check to see if intel processor.
;;;       Check to see if version == 6. 
;;;
;;;   Do P6 specific stuff.
;;;
;;; Usage: P6Stuff    eax = physical address of Voodoo.
;;; 
;;; no registers are changed!
;;; 
CheckForP6:    
   pushad                            ; save all regs.
   Trace_Out "FXMEMMAP: Identifying presence of P6"
    
   ;
   ; First, determine whether CPUID instruction is available.
   ;
   Trace_Out "FXMEMMAP: Is CPUID available?"
   pushfd                            ; push original EFLAGS.
   pop     eax                       ; pop into eax
   mov     ecx, eax                  ; save original EFLAGS in ecx
   xor     eax, 0200000h             ; flip ID bit in EFLAGS
   push    eax                       ; put it back on stack
   popfd                             ; pop into EFLAGS
   pushfd                            ; get EFLAGS back
   pop     eax                       ; into eax
   xor     eax, ecx                  ; check to see if we could toggle ID
   jz      not_P6                    ; Sorry, not P5 or P6.

   ;
   ; Now determine whether it's an intel P6 CPU.
   ;
   Trace_Out "FXMEMMAP: Is it an Intel CPU?"
   xor     eax, eax                  ; eax = 0.
   cpuid                             ; get cpuid
   xor     ebx, 0756e6547h           ; "Genu"
   jnz     notIntel                  ; 
   xor     edx, 049656e69h           ; "ineI"
   jnz     notIntel                  ; 
   xor     ecx, 06c65746eh           ; "ntel"
   jnz     notIntel
   jmp     intelP6

NotIntel:
    ;; This is a non-Intel processor. Figure out whether it supports
    ;; msr's which is all the vxd cares about


	;;jcochrane@3dfx.com
	;;no mtrr setup for cyrix yet
	;;check for cyrix processor
   xor     eax, eax                  ; eax = 0.
   cpuid                             ; get cpuid
   xor     ebx, 069727943h           ; "Cyri"
   jz      isCyrix                  ; 
   xor     edx, 049656e69h           ; "Xins"
   jz      isCyrix                  ; 
   xor     ecx, 064616574h           ; "tead"
   jz      isCyrix

    xor esi, esi     ; default feature flags
    xor edi, edi     ; default extended feature flags

    ;; execute standard feature function
    mov eax, 1
    cpuid
    mov esi, edx

    test esi, 20h               ; rdmsr/wrmsr support (bit 5)
    jz   not_P6
    jmp  DontCheckForBrokenChipset

intelP6:        
   ;
   ; Intel processor, can we ask for model number?
   ;
   Trace_Out "FXMEMMAP: May we ask for version number?"
   cmp     eax, 1
   jl      not_P6                    ; can't ask for version! must not be P6

   ;
   ; Yes, you may ask for version number, verify family==6.
   ;
   Trace_Out "FXMEMMAP: Verifying architecture family"
   mov     eax, 1
   cpuid                             ; get family/model/stepping
   shr     eax, 8                    ; rid of model & stepping number
   and     eax, 0fh                  ; use only family
   cmp     eax, 6                    ; 
   jne     not_P6                    ; sorry, some other model

   ;
   ; Intel P6 processor. 
   ; Make sure it supports Memory Type Range Request registers
   ;
   Trace_Out "FXMEMMAP: Does this P6 support MTRRs?"; 
   test    edx, 01000h               ; bit 12 in edx = MTRR support
   jz      not_P6                    ; not supported!
   test    edx, 00020h               ; bit 5, supports MSR?
   jz      not_P6                    ; rdmsr, wrmsr not supported!
    
   ; Intel P6 processor identified.
   ;
   ; Work around P6 write combining problem with certain intel chipsets.
   ; Check registry entry to see if we should bypass the fix code.
   ;
   call    P6_CheckWcRegEntry        ; Check registry to find out
   jc      DontCheckForBrokenChipset ; whether to bypass check and
   call    P6_CheckForBrokenIntelChipset; fix for broken Intel chipset
    
DontCheckForBrokenChipset:           ; on a P6.

   mov     isP6, 1h                  ; Tell the code we have a P6
   popad
   ret

isCyrix:
not_P6:
   Trace_Out "FXMEMMAP: Sorry, not P6: #eax #ebx #ecx #edx"
   mov     isP6, 0h                  ; Tell the code we have a P6
   popad
   ret

;******************************************************************************
;
;  P6_CheckWcRegEntry - Check registry to determine whether to do
;           P6_CheckForBrokenIntelChipset
;
;   Check the registry entry defined by the variables P6wcfix_keyname and
;   P6wcfix_valuename (currently \HK_LOCAL_MACHINE\SOFTWARE\
;   3Dfx Interactive\Shared\DisableP6WCfix).  If there is no
;   such registry entry, or the value of the entry is 0 return CLC,
;   otherwise return STC.
;
; no registers are changed.
;
P6_CheckWcRegEntry PROC
   LOCAL   P6_cbData:dword, P6_Data:dword, P6_Datatype:dword, VxDHKey:dword

   pushad
   lea     ebx, VxDHKey
   lea     ecx, P6wcfix_keyname
   VMMCall _RegOpenKey, < HKEY_LOCAL_MACHINE, ecx, ebx >
   cmp     eax, ERROR_SUCCESS
   jne     P6_DoFix

   mov     P6_cbData, 4              ; Use 4 byte buffer
   lea     ebx, P6_cbData
   lea     ecx, P6_Data
   lea     edx, P6wcfix_valuename
   lea     eax, P6_Datatype
   VMMCall _RegQueryValueEx, < VxDHKey, edx, 0, eax, ecx, ebx>
   cmp     eax, ERROR_SUCCESS
   pushf
   VMMCall _RegCloseKey, VxDHKey
   popf
   jne     P6_DoFix

   cmp     byte ptr P6_Data, 00      ; If the registry value was
   je      P6_DoFix                  ; 00, clear the carry flag,


P6_DontFix:
   stc                               ; else set the carry flag.
   jmp     P6_CheckWcRegEntryExit    ; 
P6_DoFix:
   clc
P6_CheckWcRegEntryExit:
   popad
   ret
P6_CheckWcRegEntry ENDP


;******************************************************************************
;  P6_CheckForBrokenIntelChipset
;
;   On an Intel PCI chipset combination of Host bridge ID 0x1237 and
;   ISA bridge IDE 0x7000:
;   
;   There are two bits, one in the PCI bridge, and one in the ISA bridge,
;   that when set a certain way, cause P6 systems to lock up when access
;   to WC memory (write combine) on PCI is performed simultaneously with
;   an ISA access.  Here is the table of bits:
;
;   PCI ISA works?
;   --- --- ------
;   0   0   Yes
;   0   1   Yes
;   1   0   No
;   1   1   Yes
;
;   This routine detects the failing case and sets the ISA bit to a 1
;   to avoid the failure.
;

; PCI BIOS equates
PCI_FUNCTION_ID     equ 0B1h

FIND_PCI_DEVICE     equ 002h
READ_CONFIG_BYTE    equ 008h
WRITE_CONFIG_BYTE   equ 00Bh

P6_CheckForBrokenIntelChipset:
   pushad                            ; Save all regs.
   ; Our board is only PCI -- assume PCI BIOS present.

   ; Check for the problem Host bridge chip.
   mov     ax, (PCI_FUNCTION_ID SHL 8) OR FIND_PCI_DEVICE
   mov     cx, 1237h                 ; Broken Host bridge PCI ID
   mov     dx, 8086h                 ; Intel's PCI vendor ID
   xor     si, si                    ; Check for 1st instance
   push    dword ptr 1ah             ; PCI BIOS interrupt
   VMMcall Exec_VxD_Int              ; 
   cmp     AH, 00h                   ; Find it?
   jne     ChipsetOK                 ; 

   ; Get DBC register from Host bridge
   mov     ax, (PCI_FUNCTION_ID SHL 8) OR READ_CONFIG_BYTE; 
   mov     di, 53h                   ; DBC - DBX buffer control register
   push    dword ptr 1ah             ; PCI BIOS interrupt
   VMMcall Exec_VxD_Int
   test    cl, 20h                   ; Check bit 5
   je      ChipsetOK                 ; If it's zero we're OK, else continue

   ; Check for the problem ISA bridge.
   mov     ax, (PCI_FUNCTION_ID SHL 8) OR FIND_PCI_DEVICE
   mov     cx, 7000h                 ; Broken ISA bridge PCI ID
   mov     dx, 8086h                 ; Intel's PCI vendor ID
   xor     si, si                    ; Check for 1st instance
   push    dword ptr 1ah             ; PCI BIOS interrupt
   VMMcall Exec_VxD_Int
   cmp     AH, 00h                   ; Find it?
   jne     ChipsetOK

   ; Get register 82h from ISA bridge
   push    bx                        ; Save Bus/Device info.
   mov     ax, (PCI_FUNCTION_ID SHL 8) OR READ_CONFIG_BYTE; 
   mov     di, 82h
   push    dword ptr 1ah             ; PCI BIOS interrupt
   VMMcall Exec_VxD_Int
   pop     bx
   test    cl, 02h                   ; Check bit 1
   jne     ChipsetOK                 ; If it's zero we're OK, else continue

; Problem chipset and CPU identified, fix problem bit.
   or      cl, 02h                   ; Fix bit 1 of ISA bridge.
   mov     ax, (PCI_FUNCTION_ID SHL 8) OR WRITE_CONFIG_BYTE
   mov     di, 82h
   push    dword ptr 1ah             ; PCI BIOS interrupt
   VMMcall Exec_VxD_Int

ChipsetOK:
    popad
    ret

;******************************************************************************


endif
VxD_LOCKED_CODE_ENDS
end
