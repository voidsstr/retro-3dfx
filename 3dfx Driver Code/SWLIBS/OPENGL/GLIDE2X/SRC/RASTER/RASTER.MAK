#-------------------------------------------------------------------------
# RASTER sublibrary makefile
#-------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Directories
#----------------------------------------------------------------------------

!ifndef DEBUG
TARGET_DIR      = .\release
DESTINATION_DIR = ..\release
!else
!if "$(DEBUG)"=="BC"
TARGET_DIR = .\bchecker
DESTINATION_DIR = ..\bchecker
!else
TARGET_DIR = .\debug
DESTINATION_DIR = ..\debug
!endif
!endif

#----------------------------------------------------------------------------
# Target, Default Rule, Common Make Code
#----------------------------------------------------------------------------
TARGET = raster

all: $(DESTINATION_DIR)\$(TARGET).lib

!include "..\include\make\ogl.mak"

#-------------------------------------------------------------------------
# FILES
#-------------------------------------------------------------------------
THISFILE = raster.mak
LIB      = $(TARGET_DIR)\$(TARGET).lib
CFILES =             \
    .\fr_og.c        \
    .\fr_pick.c      \
    .\fr_setup.c     \
    .\fr_sqrs.c      \
    .\fr_stncl.c     \
    .\fr_store.c     \
    .\fr_tex.c       \
    .\g_ciclr.c      \
    .\g_cicmb.c      \
    .\g_cipix.c      \
    .\g_citri.c      \
    .\g_rgbcb2.c     \
    .\g_rgbcb4.c     \
    .\g_rgbcb5.c     \
    .\g_rgbcba.c     \
    .\g_rgbcl2.c     \
    .\g_rgbcl4.c     \
    .\g_rgbcl5.c     \
    .\g_rgbcla.c     \
    .\g_rgbpx2.c     \
    .\g_rgbpx4.c     \
    .\g_rgbpx5.c     \
    .\g_rgbpxa.c     \
    .\g_rgbtri.c     \
    .\g_texel.c      \
    .\g_ztest.c      \
    .\og.c           \
    .\og_cache.c     \
    .\ras_og.c       


OFILES = $(CFILES:.c=.obj)
!ifndef DEBUG
OFILES = $(OFILES:.\=.\release\)
!else
!if "$(DEBUG)" != "BC"
OFILES = $(OFILES:.\=.\debug\)
!else
OFILES = $(OFILES:.\=.\bchecker\)
!endif
!endif

#-------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------

$(DESTINATION_DIR)\$(TARGET).lib: "$(DESTINATION_DIR)" $(LIB)
    if exist $(DESTINATION_DIR)\$(TARGET).lib $(DEL) $(DESTINATION_DIR)\$(TARGET).lib
    copy $(LIB) $(DESTINATION_DIR)

$(LIB): $(THISFILE) "$(TARGET_DIR)" $(OFILES)
    $(AR) @<<
    $(AR_OPTS)
    $(OFILES)
    /out:$(LIB)
<<


$(OFILES): $(THISFILE)


{.}.c{$(TARGET_DIR)}.obj::
    $(CPP) @<<
    $(CPP_OPTS) 
    $(CPP_DEFS) 
    $(CPP_PATHS) 
    $<
<<

clobber:
    @echo clobber all files
    if exist "$(TARGET_DIR)/*" $(DELDIR) $(TARGET_DIR)

clean:
    @echo clean intermediate files
    if exist "$(TARGET_DIR)/*" attrib -r $(TARGET_DIR)\*.*
    if exist "$(TARGET_DIR)/*" $(DEL) $(TARGET_DIR)\*.obj
    if exist "$(TARGET_DIR)/*" $(DEL) $(TARGET_DIR)\*.pch
    if exist "$(TARGET_DIR)/*" $(DEL) $(TARGET_DIR)\*.map
    if exist "$(TARGET_DIR)/*" $(DEL) $(TARGET_DIR)\*.pdb
    if exist "$(TARGET_DIR)/*" $(DEL) $(TARGET_DIR)\*.asm
