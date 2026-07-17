#----------------------------------------------------------------------------
# Global Make Includes
#----------------------------------------------------------------------------

#-------------------------------------------------------------------------
# ENVIRONMENT VALIDATION
#-------------------------------------------------------------------------
!ifndef INCLUDE
!error "Environment Variable INCLUDE must be defined to continue"
!endif

!ifndef LIB
!error "Environment Variable LIB must be defined to continue"
!endif

!ifndef GLIDE_ROOT
!error "Environment Variable GLIDE_ROOT must be defined to continue"
!endif

!ifndef BUILD_ROOT_SWLIBS
!error "Environment Variable BUILD_ROOT_SWLIBS must be defined to continue"
!endif

!ifndef _NMAKE_VER
!error "You must upgrade to NMAKE version 1.62 or greater
!endif

#----------------------------------------------------------------------------
# Tools
#----------------------------------------------------------------------------

!if "$(DEBUG)"=="BC"
LNK = nmlink.exe
CPP = nmcl.exe
!else
LNK = link.exe
CPP = cl.exe
!endif
AR  = lib.exe
BSC = bscmake.exe

!if "$(OS)"=="Windows_NT"
DEL    = del /q 
DELDIR = rd /s /q 
!else
DEL    = del 
DELDIR = deltree /Y
!endif



#----------------------------------------------------------------------------
# Tool Options
#
# LINK.EXE
#
# /nologo       - suppress command line banner
# /base:nnnnnn  - set the preferred virtual base address
# /subsystem:x  - set the subsystem in which to run ( windows )
# /dll          - build a .dll
# /incremental  - allows faster incremental linking, remove
#                 for release since it may pad and thunk code
# /pdb:x        - set name for .pdb file ( debug only )
# /map:x        - set the name for the mapfile
# /debug        - direct linker to include debugging info
# /debugtype:x  - direct linker as to type of debugging info ( both )
# /machine:x    - target a particular machine ( i386 )
# /def:x        - specify a .def file
# /out:x        - specify the output filename
# /implib:x     - specify the name for the import library generated
# /verbose      - specify verbose output
#
# CL.EXE - options
#
# /nologo - disable runtime banner
# /G5     - optimize for a P5
# /MT     - create a multi-thread-safe object
# /W3     - Warning level 3 ( out of 4 )
# /GX     - catch/Throw exception handling ( ****remove**** )
# /Zi     - create debugging information including .pdb file
# /Z7     - create debugging information and imbed in o-files
# /O2     - Maximize Speed at the Expense of Size ( Ox, Gfy )
# /FAcs   - Create a listing file with Machine, Source and Asm ( ***make optional*** )
# /Fa     - Specify directory for listing file
# /FR     - Specify a filename for .sbr browser file
# /Fp     - Specify precompiled header filename
# /YX     - Enable use of precompiled headers
# /Fo     - Set output directory/name for obj files
# /Fd     - Set output directory/name for .pdb file
# /c      - suppress linking
#
# LIB.EXE - options
#
# /nologo      - disable runtime banner
# /machine:x   - hint to archiver as to .obj type
# /out:x       - specify the output filename
# /subsystem:x - hint to archiver as to target platform
#-------------------------------------------------------------------------
AR_OPTS   = /NOLOGO       \
            /MACHINE:I386 \
            /SUBSYSTEM:windows
CPP_OPTS  = /nologo /MT /W3 /YX /c
CPP_DEFS  = /D "__GL_SST"\
            /D "__MSC__"\
            /D "WIN32"\
            /D "_WINDOWS"\
            /D "_LANGUAGE_C"\
            /D "__GL_FUTURE"\
            /D "_GDI32_"\
            /D "__GL_OLD_THREAD"\
            /D "__GL_COSMO_BUILD"\
            /D "__GL_SUPPORT_MGL"\
!if "$(GL_NOASM)"==""
            /D "__GL_USE_INTEL_ASM"\
!endif
            /D "__WGL_USE_GLIDE"  
CPP_PATHS = /I "$(BUILD_ROOT_SWLIBS)\opengl\glide3x\include"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\glide3x\include\GL"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\glide3x\wgl"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\glide3x\sst"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\glide3x\trace"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\src\raster"\
            /I "$(BUILD_ROOT_SWLIBS)\opengl\src\include"\
            /I "$(GLIDE_ROOT)\include"\
            /I "$(BUILD_ROOT_SWLIBS)\include"
LNK_OPTS  = /nologo\
            /base:0x69000000\
            /subsystem:windows\
            /dll\
            /map:"$(TARGET_DIR)/opengl.map"\
            /machine:I386\
            /def:".\$(DEFFILE)"\
            /out:"$(TARGET_DLL)"

!ifdef DEBUG
!if "$(DEBUG)" != "BC"
#----------------------------------------------------------------------------
# Debugging-Specific Options
#----------------------------------------------------------------------------
CPP_OPTS  = $(CPP_OPTS) /Zi /FAc \
            /Fa".\debug/"\
            /FR".\debug/"\
            /Fp".\debug\$(TARGET).pch"\
            /Fo".\debug/"\
            /Fd".\debug/"
CPP_DEFS  = $(CPP_DEFS)\
            /D "__GL_BUILD_TRACE"\
            /D "_DEBUG" 
LNK_OPTS  = $(LNK_OPTS)\
            /incremental:no\
            /debug\
            /debugtype:both\
            /pdb:"$(TARGET_DIR)/opengl.pdb" 
!else
#----------------------------------------------------------------------------
# BoundsChecker-Specific Options
#----------------------------------------------------------------------------
CPP_OPTS  = $(CPP_OPTS) /Zi /FAc \
            /Fa".\bchecker/"\
            /FR".\bchecker/"\
            /Fp".\bchecker\$(TARGET).pch"\
            /Fo".\bchecker/"\
            /Fd".\bchecker/"
CPP_DEFS  = $(CPP_DEFS)\
            /D "__GL_BUILD_TRACE"\
            /D "_DEBUG" 
LNK_OPTS  = $(LNK_OPTS)\
            /incremental:no\
            /debug\
            /debugtype:both\
            /pdb:"$(TARGET_DIR)/opengl.pdb" 
!endif
!else
#----------------------------------------------------------------------------
# Release-Specific Options
#----------------------------------------------------------------------------
CPP_OPTS  = $(CPP_OPTS) /O2 /G6 /FAc \
             /Fa".\release/"\
             /Fp".\release\$(TARGET).pch"\
             /Fo".\release/"
CPP_DEFS  = $(CPP_DEFS)\
             /D "NDEBUG"
LNK_OPTS  = $(LNK_OPTS)\
            /incremental:no

!endif

#----------------------------------------------------------------------------
# Rules
#----------------------------------------------------------------------------

"$(TARGET_DIR)":
    if not exist "$(TARGET_DIR)/$(NULL)" mkdir "$(TARGET_DIR)"

"$(DESTINATION_DIR)" :
    if not exist "$(DESTINATION_DIR)/$(NULL)" mkdir "$(DESTINATION_DIR)"


