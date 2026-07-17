#******************************************************************************
#
#       Napalm CSim Proper Makefile
#
#******************************************************************************
#
#   Title:      Makefile
#
#   Revision:   1.00
#
#   Date:       October 1, 1998
#
#   Author:     Randy Spurlock
#
#******************************************************************************
#
#   Change log:
#
#     DATE    REVISION                  DESCRIPTION
#   --------  --------  -------------------------------------------------------
#   08/01/98    1.0     Original version                      Randy Spurlock
#   10/01/98    1.1     Modified for CDev                     Dale D. Kenaston
#   02/05/99    1.2     Modified for SST2                     Dale D. Kenaston
#   02/08/99    1.3     Modified for CSim                     Dale D. Kenaston
#   04/03/99    1.4     Modified for Napalm                   Goran Devic
#   05/27/99    1.5     Cleaned up                            Goran Devic
#
#******************************************************************************
#       Define the Component Operating System and Direct X Versions
#******************************************************************************
OS              =       9x              # Set OS version to Windows 9x
DX              =       DX              # Set DX version to Direct X
VC32            =       VC              # Set VC version to Visual C/C++
MD              =       32              # Set 16-bit Mode

#******************************************************************************
#       Define the required tools for this component
#******************************************************************************
TOOLS           =       DDK VC32 SDK    # DDK, MSVC and SDK

#******************************************************************************
#       Define the Macro Assembler Flags
#******************************************************************************
AFLAGS  = /nologo /c /W3 /Cx /coff /DBLD_COFF /DIS_32 /DMASM6                 \
          /DDate=$(SLASHDATE) /DVer=$(VERSION) /Zi /DDEBUG

#******************************************************************************
#       Define the Compiler Flags
#******************************************************************************
CFLAGS  = /nologo /c /W3 /Zp /Gs /Zl /DBLD_COFF /DIS_32 /D_X86_ /D__MSC__     \
          /DWIN32 /D__WIN32__ /DFX_DLL_ENABLE /DGDBG_INFO_ON /DKERNEL         \
          /DNO_FLOAT /DHAL_CSIM /DH4 /DWINSIM /DBUILD_HAL /Zi /Od /FAcs
          
CH3ASM  = /nologo    /W3 /Zp                    /DIS_32 /D_X86_ /D__MSC__     \
          /DWIN32 /D__WIN32__ /DFX_DLL_ENABLE /DGDBG_INFO_ON /DKERNEL         \
          /DNO_FLOAT /DHAL_CSIM /DH4 /DWINSIM /DBUILD_HAL

#******************************************************************************
#       Define the Lib Flags
#******************************************************************************
LFLAGS  = -nologo

#******************************************************************************
#       Include the Makefile Macros
#******************************************************************************
!Include        ..\..\WinSIM\Macro.Mak     # Include Makefile Macro Definitions

#******************************************************************************
#       Define any Special Inference Rules
#******************************************************************************
.c.obj:
        @$(CC) $(CFLAGS) /Fo$(*F).Obj $(*F).C >>Build.Log

#******************************************************************************
#       Component target dependencies
#
#  The process of compiling have to start with the compilation of the
#  temporary utility program "h3asm.exe" that is able, when run, of producing
#  a single header file.  That header file is used when compiling the
#  CSim Proper library.
#
#******************************************************************************
All:                            MakeH3Asm                                     \
                                All_Pseudo                                    \
                                NapSim.Lib
        @Echo   Napalm CSim Proper Component Build Complete
        @If Not Exist Build.Log If Exist Build.Tmp Ren Build.Tmp Build.Log >Nul
        @If Exist Build.Tmp Del Build.Tmp >Nul

All_Pseudo:                     Env                                           \
                                $(TOOLS)                                      \
                                Verbose
        @Echo   Building Napalm CSim Proper Component
        @If Exist Build.Tmp Del Build.Tmp >Nul
        @If Exist Build.Log Ren Build.Log Build.Tmp >Nul
        @Set Include=..\..\WinSIM\VcrtD\include;..\incsrc;..\..\swlibs\fxmisc;..\..\swlibs\newpci\pcilib;%include%%

MakeH3Asm:                      Env                                           \
                                $(TOOLS)
        @Set Include=..\incsrc;..\..\swlibs\fxmisc;..\..\swlibs\newpci\pcilib;%include%%
        @$(CC) $(CH3ASM) h3asm.c >>Build.Log
        @If Exist h3asm.h Del h3asm.h
        @h3asm.exe > h3asm.h
        
#******************************************************************************
#       Napalm module dependencies
#******************************************************************************
NapSim.Lib:                     alpha.obj                                     \
                                ccu.obj                                       \
                                cmdfifo.obj                                   \
                                compress.obj                                  \
                                csim.obj                                      \
                                csimio.obj                                    \
                                fbi.obj                                       \
                                go.obj                                        \
                                go2.obj                                       \
                                h3asm.obj                                     \
                                lfb.obj                                       \
                                lod.obj                                       \
                                miptable.obj                                  \
                                recip.obj                                     \
                                regs.obj                                      \
                                rgbfmt.obj                                    \
                                setup.obj                                     \
                                sstgprnt.obj                                  \
                                sstprint.obj                                  \
                                trex.obj                                      \
                                trexfunc.obj                                  \
                                yuv.obj
        @$(LB) $(LFLAGS) /out:$(*F).Lib $(**F)

#******************************************************************************
#       Napalm source module dependencies
#******************************************************************************
$(OBJ_DIR)\Xlate.Obj:           $(SRC_DIR)\Xlate.c                            \
                                $(BLD_DIR)\Makefile

#******************************************************************************
#       Component clean dependencies
#******************************************************************************
Clean:                  Clean_Pseudo
        @If Exist *.Exp         Del *.Exp               >Nul
        @If Exist *.Lib         Del *.Lib               >Nul
        @If Exist *.Log         Del *.Log               >Nul
        @If Exist *.Map         Del *.Map               >Nul
        @If Exist *.Sym         Del *.Sym               >Nul
        @If Exist *.VxD         Del *.VxD               >Nul
        @If Exist *.Pdb         Del *.Pdb               >Nul
        @If Exist *.Nms         Del *.Nms               >Nul
        @If Exist *.I           Del *.I                 >Nul
        @If Exist *.Lst         Del *.Lst               >Nul
        @If Exist *.Obj         Del *.Obj               >Nul
        @If Exist *.Res         Del *.Res               >Nul
        @If Exist *.Lib         Del *.Lib               >Nul
        @If Exist *.Exe         Del *.Exe               >Nul
        @If Exist *.Cod         Del *.Cod               >Nul
        @Echo Napalm CSIM Component Clean Complete

Clean_Pseudo:
        @Echo Cleaning Napalm CSIM Component

