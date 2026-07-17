

WRITE_FIFO_3	MACRO __addr, __offset, __data
IFDEF DEBUG_ASM
   PUSH_ALL
   sub      esp,12
   mov      dword ptr [esp],__data
   mov      dword ptr [esp+4],__offset
   mov      dword ptr [esp+8],__addr
   mov      eax,dword ptr[esp]
   mov      ebx,dword ptr[esp+4]
   add      ebx,dword ptr[esp+8]
   add      esp,12
   push     eax
   push     ebx
   call     _cvg_fifo_write_asm
   add      esp,8h
   POP_ALL
ELSE
   mov      dword ptr [__addr + __offset],__data
ENDIF
ENDM ; WRITE_FIFO_3

WRITE_FIFO_FSTP_3    MACRO __addr, __offset
IFDEF DEBUG_ASM
   PUSH_ALL
   sub      esp,12
   fstp     dword ptr[esp]
   mov      dword ptr [esp+4],__offset
   mov      dword ptr [esp+8],__addr
   mov      eax,dword ptr[esp]
   mov      ebx,dword ptr[esp+4]
   add      ebx,dword ptr[esp+8]
   add      esp,12
   push     eax
   push     ebx
   call     _cvg_fifo_write_asm
   add      esp,8h
   POP_ALL
ELSE
   fstp     dword ptr[__addr + __offset]
ENDIF
ENDM ; WRITE_FIFO_3
