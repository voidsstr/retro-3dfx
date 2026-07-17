!if "$(WCS)" == "1"
WINCS = -DWIN_CSIM
!else
WINCS = 
!endif #!if "$(WCS)" != "1"
#############################################################################
#
#   Microsoft Confidential
#   Copyright (C) Microsoft Corporation 1991
#   All Rights Reserved.
#
#   MINI project makefile include
#
#   USAGE:
#
#   to use this makefile to build the 16bit side of a Windows95 mini driver
#   define the needed macros and include this file.
#
#   NOTE your driver does not need a .RC file
#
#   REQUIRED MACROS
#
#       DRVNAME         the basename of your driver (ie vga)
#       SOURCES         the list of files needed to build your driver
#       VERDIR          location of the intermedite files, and final driver
#
#   OPTIONAL MACROS
#
#       DESCRIPTION     this becomes the descripton in the version resource.
#       DRVFILE         final name for the .DRV file, defaults to $(DRVNAME).drv
#       DRVDEF          the .def file to use, if not specifed a default will be used
#       DRVLIBS         any extra libs you need
#       DRVRESFILE      Optional driver resource file
#		DEFRULESUPPLIED Defined=1 if rule for builing .DEF file is in target
#						directory makefile.
#
#############################################################################

!ifndef VERBOSE
.SILENT :
!ifndef LOGO
LOGO = /nologo
!endif
!else
LOGO =
!endif

!ifndef VERDIR
!error
!endif

!ifndef DRVNAME
!error
!endif

!IF [set INCLUDE=;]
!ENDIF

!IF [set LIB=;]
!ENDIF

!if [cd $(VERDIR)]     ## change the current directory
!endif

SRCDIR = ..
OBJDIR = .

SOURCES = $(SOURCES:  = )
SOURCES = $(SOURCES:  = )
SOURCES = $(SOURCES:  = )
SOURCES = $(SOURCES:  = )
SOURCES = $(SOURCES:  = )
DRVOBJS = $(SOURCES:.cpp=.obj)
DRVOBJS = $(DRVOBJS:.c=.obj)
DRVOBJS = $(DRVOBJS:.asm=.obj)
DRVOBJS = $(DRVOBJS:.rc=.res)
DRVOBJS = $(DRVOBJS:.thk=.obj)

IS_16       = TRUE
IS_OEM      = TRUE
MASM6       = TRUE
!include $(SRCDIR)\..\check.mk
ASM         = $(MASM)\bin\ml
MYMAPSYM    = $(SDK32)\bin\mapsym

MYRC = $(DDKDX)\bin16\rc


ASMENV		= ML
MYCC        = $(COMP16)\bin\cl
LINK16      = $(COMP16)\bin\link

!ifndef DRVFILE
DRVFILE=$(DRVNAME).drv
!endif

!ifndef DRVDEF
DRVDEF  = $(DRVNAME).def
!endif

!if EXIST($(SRCDIR)\$(DRVDEF))
DRVDEF  = $(SRCDIR)\$(DRVDEF)
!endif

DRVMAP  = $(DRVNAME).map
DRVSYM  = $(DRVFILE:.drv=.sym)
DRVSYM  = $(DRVSYM:.DLL=.sym)
DRVSYM  = $(DRVSYM:.dll=.sym)
!ifndef DRVSYM2
DRVSYM2 = $(DRVFILE:.drv=)d.sym
!endif
DRVRES  = $(DRVNAME).res


TARGETS = $(DRVFILE)
.SUFFIXES: .thk

#
#  Make sure environment variables are properly set up
#

INCLUDE = $(SRCDIR)
!ifdef	SRCDIR2
INCLUDE = $(INCLUDE);$(SRCDIR2)
!endif

!ifdef DRVINCLUDE
INCLUDE = $(INCLUDE);$(DRVINCLUDE)
!endif

INCLUDE = $(INCLUDE);$(SRCDIR)\..\inc

INCLUDE = $(INCLUDE);$(DDKDX)\inc16
INCLUDE = $(INCLUDE);$(DDKDX)\inc
INCLUDE = $(INCLUDE);$(COMP16)\Include
INCLUDE = $(INCLUDE);$(DDK9X)\Inc16
INCLUDE = $(INCLUDE);$(DDK9X)\Inc32
INCLUDE = $(INCLUDE);$(SDK32)\Inc16
INCLUDE = $(INCLUDE);$(SDK32)\Include


DRVLIBS = $(DRVLIBS) \
          kernel.lib \
          dibeng.lib \
          ldllcew.lib
MYLIB = $(MYLIBDIR)
MYLIB = $(MYLIB);$(SRCDIR)
MYLIB = $(MYLIB);$(DDKDX)\lib
MYLIB = $(MYLIB);$(COMP16)\lib
MYLIB = $(MYLIB);$(DDKDX)\lib16
MYLIB = $(MYLIB);$(SDK32)\lib
MYLIB = $(MYLIB);$(DDK9X)\lib
LIB =$(MYLIB)

#
#  Cflags, Aflags, and Lflags
#
#  /Al = large model
#  /Af = far data pointers
#  /Aw = ss != ds, ds not loaded on function call
#  /Au = load DS segment register on each FAR call (use this if /Aw causes problems)
#  /c  = exclude linker
#  /Cp = preserve case of user identifiers
#  /Fc = generate .COD files (combined source and assembly)
#  /G3 = gendrate optimized code for 386
#  /GD = protected mode win entry/exit code
#  /Gs = control stack probes
#  /nologo = exclude header text
#  /Oa = assume no aliasing (volatile pointers) in functions or across functions
#  /Ot = favor code spped
#  /Ox = maximum optimziations
#     /Ob = expand inline functions
#     /O2 = maximize speed
#     /Oc = local common expressions
#     /Oe = register allocation
#     /Og = global common expressions
#     /Oi = generate intrinsic functions as inline code
#     /Ol = loop optimizations
#     /On = disable unsafe optimizations
#     /Oo = peep hole optimizations in assembler generation
#     /Ot = favor code speed
#  -NT_TEXT = nmake code segment
#  /u  = remove all defined macros
#  /W3 = warning level 3
#  /Zd = generate only line number and public symbol debugger info
#  /Zi = Generate complete debugging info
#  /Zm = Enable masm 5.10 compatibility
#  /Zp = pack structure members on 1 byte boundary
#

!if "$(CF)" == "1"
CMDFIFO = -DCMDFIFO
CMDFIFOECHO = ON
!else #!if "$(CF)" == "1"
CMDFIFO =
CMDFIFOECHO = OFF
!endif #!if "$(CF)" == "1"

!if "$(DF)" == "1"
CMDFIFO = $(CMDFIFO) -DDEBUGFIFO
DBGFIFOECHO = ON
!else
CMDFIFO = $(CMDFIFO)
DBGFIFOECHO = OFF
!endif

# if DDR is defined then give Direct Draw refresh rates
!if "$(DDR)" == "1"
DXR =
!else
DXR = -DNO_REFRESH
!endif !if "$(DDR)" == "1"

# if stereo support is required
!if "$(STEREO)" == "1"
STEREO = -DSTEREO
!else
STEREO = 
!endif !if "$(STEREO)" == "1"

# define for Mode Types
!if "$(MT)" == "1"
MODE_TYPE = -DGLOP
!else
MODE_TYPE =
!endif !if "$(MT)" == "1"

# if MMI environment var is set to 1, enable memory manager integrity check
!if "$(MMI)" == "1"
MEM_MGRDEF = $(MEM_MGRDEF) -DMM_INTEGRITY
!endif !if "$(MMI)" == "1"

!if "$(SSB)" == "1"
SSBDEF = -DSSB
!endif !if "$(SSB)" == "1"

!if "$(IFB)" == "1"
IFBDEF = -DIFB
!endif !if "$(IFB)" == "1"


!if "$(ND)" == "1"
NULLDRIVER = -DNULLDRIVER
!else #!if "$(ND)" == "1"
NULLDRIVER =
!endif #!if "$(ND)" == "1"

!if "$(ACF)" == "1"
AGPCMDFIFO = -DAGP_CMDFIFO
AGPCMDFIFOECHO = (acf=1) ENABLED
!else #!if "$(ACF)" == "1"
AGPCMDFIFO = 
AGPCMDFIFOECHO = (acf=0) DISABLED
!endif #!if "$(ACF)" == "1"

!if "$(PN)" == "1"
PERFNOP = -DPERFNOP
!else #!if "$(PN)" == "1"
PERFNOP =
!endif #!if "$(PN)" == "1"

!if "$(LINEAR)" == "1"
LINEAROPT = -DLINEAR_ONLY
LINEARECHO = ON
!else
LINEAROPT =
LINEARECHO = OFF
!endif

!if "$(WCS)" == "1"
WINCS = -DWIN_CSIM
!else
WINCS = 
!endif #!if "$(WCS)" != "1"

!if "$(SA)" == "1"
SLI = -DSLI_AA
!else
SLI = 
!endif #!if "$(SA)" != "1"

!if "$(RA)" == "1"
RAE = -DRD_ABORT_ERROR
!else
RAE = 
!endif #!if "$(RAE)" != "1"

!if "$(RN)" == "1"
RLN = -DREAL_NET
!else
RLN = 
!endif #!if "$(RLN)" != "1"

!if "$(GBLDATA_IN_PDEV)" == "1"
GBLDATADEF = -DGBLDATA_IN_PDEV
!endif

!ifdef OVERRIDE_L16FLAGS
L16FLAGS = $(OVERRIDE_L16FLAGS)
!else
# KMW ORIG: L16FLAGS = /NOE/NOD/NOPACKCODE/NOPACKFUNCTIONS/LI/MAP/AL:16 $(L16DEF)
L16FLAGS = /CO/NOE/NOD/NOPACKCODE/NOPACKFUNCTIONS/LI/MAP/AL:16 $(L16DEF)
!endif

!ifdef OVERRIDE_C16FLAGS
CFLAGS = $(OVERRIDE_C16FLAGS)
!else #!ifdef OVERRIDE_C16FLAGS

#
# standard DDK gdi16 options flags
#

!if "$(VERDIR)" == "debug"
CDEBUGOPT = -Od -DDEBUG -DTRACE
!else
CDEBUGOPT = -Oxa 
!endif

CFLAGS = -GD -Alfw $(CDEBUGOPT) -Z7i -DWIN95 -I$(DDKDX)\inc -I$(DDK9X)\inc16 -I$(DDK9X)\inc32 -I..\inc \
	-W3 -c -Zp -Gf3s -nologo -DDRVNAME=\"$(DRVNAME)\" -Fc


#
# add 3Dfx specific flags for gdi16
#

CFLAGS = $(CFLAGS) $(BRDCFLAGS) -DIS_16 -D__MSC__ -DMSVC16 $(MEM_MGRDEF) \
         $(SSBDEF) $(IFBDEF) $(GBLDATADEF) $(CMDFIFO) -DWRITECOMBINE \
	 $(DXR) $(NULLDRIVER) $(PERFNOP) $(WINCS) $(SLI) $(RAE) $(RLN) \
	 $(AGPCMDFIFO) $(MODE_TYPE) $(LINEAROPT) $(STEREO)

!if "$(CT)" == "1"
CFLAGS = $(CFLAGS) -DCRASHTEST
CRASHECHO = ON
!else #!if "$(CT)" == "1"
CRASHECHO = OFF
!endif #!if "$(CT)" == "1"

CFLAGS = $(CFLAGS) $(HWREV) $(HWPRODUCT)


!endif #!ifdef OVERRIDE_C16FLAGS


!ifdef OVERRIDE_A16FLAGS
AFLAGS = $(OVERRIDE_A16FLAGS)
!else
#
# standard DDK gdi16 asm flags
#

AFLAGS = -D?QUIET -D?LARGE -DDEBUG -Zd -W3 -WX -c -Cx -nologo -DMASM6 -Fl -Sn 

#
# 3Dfx specific gdi16 asm flags
#

AFLAGS = $(AFLAGS) $(BRDAFLAGS) -DIS_16 $(CMDFIFO) $(GBLDATADEF) $(NULLDRIVER) \
	$(PERFNOP) -DWRITECOMBINE $(SLI)

!endif #!ifdef OVERRIDE_A16FLAGS

#############################################################################
#
# build rules
#
#############################################################################


{$(SRCDIR)\..\common}.asm.obj:
        $(ASM) @<<
$(AFLAGS)
-Fo$*.obj $<
<<

{$(SRCDIR)\..\common}.c.obj:
        @$(MYCC) @<<
$(CFLAGS)
-Fo$*.obj $<
<<

!ifdef SRCDIR2
{$(SRCDIR)\$(SRCDIR2)}.asm.obj:
        $(ASM) @<<
$(AFLAGS)
-Fo$*.obj $<
<<

{$(SRCDIR)\$(SRCDIR2)}.c.obj:
        @$(MYCC) @<<
$(CFLAGS)
-Fo$*.obj $<
<<

{$(SRCDIR)\$(SRCDIR2)}.cpp.obj:
        @$(MYCC) @<<
$(CFLAGS)
-Fo$*.obj $<
<<
!endif

{$(SRCDIR)}.asm.obj:
        $(ASM) $(AFLAGS) -Fo$*.obj $<

{$(SRCDIR)}.c.obj:
        @$(MYCC) $(LOGO) @<<
$(CFLAGS)
-Fo$*.obj $<
<<

{$(SRCDIR)}.cpp.obj:
        @$(MYCC) @<<
$(CFLAGS)
-Fo$*.obj $<
<<

{$(SRCDIR)}.thk.obj:
        @$(SDK32)\bin\thunk.exe $(THUNK16_FLAGS) $(SRCDIR)\$*.thk -o $*.asm
        @$(ASM) -D?LARGE -D?QUIET -Zp4 -DIS_16 -c -Cx -DMASM6 $*.asm -Fo$*.obj



############################################################################
#
#   build the driver
#
#############################################################################
$(DRVFILE):  HFILES $(DRVOBJS) $(DRVRES) $(DRVDEF)
	$(LINK16) $(L16FLAGS) @<<$(DRVNAME).lnk
$(DRVOBJS: = +^
)
$(DRVFILE)
$(DRVMAP)
$(DRVLIBS:  = +^
)
$(DRVDEF);
<<KEEP
#KMW ORIG
#        $(MYRC) -b $(DRVRES) $(DRVFILE)
#	copy $(DRVMAP) display.map
#        -$(MYMAPSYM) /n  display
#	del display.map
#	del $(DRVSYM)
#	rename display.sym $(DRVSYM)
#	copy $(DRVSYM) $(DRVSYM2)
#	copy $(DRVFILE) ..\..\..\$(VERDIR)\$(DRVFILE)
#	copy $(DRVSYM) ..\..\..\$(VERDIR)\$(DRVSYM2)
#!if EXIST($(SRCDIR)\inf)
#	copy $(SRCDIR)\inf\*.* ..\..\..\$(VERDIR)
#!endif

        $(MYRC) -b $(BRDCFLAGS) $(DRVRES) $(DRVFILE)	
!ifdef SIW95
	$(SIW95)\nmsym /trans:always,package $(DRVFILE)
!endif

!ifndef DEFRULESUPPLIED


#############################################################################
#
#   build a default .def file
#
#############################################################################
$(DRVDEF):
	copy << $(DRVDEF)
LIBRARY DISPLAY

DESCRIPTION 'DISPLAY : 100, 96, 96 : $(DESCRIPTION)'

EXETYPE WINDOWS

;;DATA PRELOAD MOVEABLE SINGLE
;;CODE PRELOAD MOVEABLE DISCARDABLE

DATA PRELOAD FIXED SINGLE
CODE PRELOAD FIXED

HEAPSIZE 64

SEGMENTS
    _TEXT       PRELOAD                       SHARED
    _INIT       PRELOAD  MOVEABLE DISCARDABLE SHARED

EXPORTS
;;;;WEP
    ValidateMode
;   ValidateDesk
    BitBlt              @1
    ColorInfo           @2
    Control             @3
    Disable             @4
    Enable              @5
    EnumDFonts          @6
    EnumObj             @7
    Output              @8
    Pixel               @9
    RealizeObject       @10
    StrBlt              @11
    ScanLR              @12
;   DeviceMode          @13
    ExtTextOut          @14
    GetCharWidth        @15
    DeviceBitmap        @16
;   FastBorder          @17
    SetAttribute        @18
    DibBlt              @19
    CreateDIBitmap      @20
    DibToDevice         @21
    SetPalette          @22
    GetPalette          @23
    SetPaletteTranslate @24
    GetPaletteTranslate @25
    UpdateColors        @26
    StretchBlt          @27
    StretchDIBits       @28
    SelectBitmap        @29
    BitmapBits          @30
    ReEnable            @31
    SaveScreenBitmap    @92
    Inquire             @101
    SetCursor           @102
    MoveCursor          @103
    CheckCursor         @104

IMPORTS
    AllocCSToDSAlias            = KERNEL.170
    AllocDSToCSAlias            = KERNEL.171
    KernelsScreenSel            = KERNEL.174
    AllocSelector               = KERNEL.175
    FreeSelector                = KERNEL.176
    PrestoChangoSelector        = KERNEL.177
    __WinFlags                  = KERNEL.178
    GetSelectorBase             = KERNEL.186
    SetSelectorBase             = KERNEL.187
    SetSelectorLimit            = KERNEL.189
    __A000h                     = KERNEL.174
    __B800h                     = KERNEL.182
    __C000h                     = KERNEL.195
    GlobalSmartPageLock         = KERNEL.230

    kernel.LoadLibraryEx32W     ;= KERNEL.513
    kernel.FreeLibrary32W       ;= KERNEL.514
    kernel.GetProcAddress32W    ;= KERNEL.515
    kernel._CallProcEx32W       ;= KERNEL.518
<<

!endif
