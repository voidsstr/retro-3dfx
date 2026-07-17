#-------------------------------------------------------------------------
# Trace sublibrary makefile
#-------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Directories
#----------------------------------------------------------------------------

!ifndef DEBUG
TARGET_DIR      = .\release
DESTINATION_DIR = ..\release
!else
!if "$(DEBUG)" == "BC"
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

TARGET = trace

all: $(DESTINATION_DIR)\$(TARGET).lib

!include "..\include\make\ogl.mak"

#-------------------------------------------------------------------------
# FILES
#-------------------------------------------------------------------------
THISFILE = trace.mak
LIB      = $(TARGET_DIR)\$(TARGET).lib
CFILES =                \
         .\db_disp.c 

!ifndef DEBUG
OFILES = $(CFILES:.c=.obj)
OFILES = $(OFILES:.\=.\release\)
!else
CFILES = $(CFILES)\
         .\db_trace.c \
         .\db_stat.c   \
         .\db_stub.c 
OFILES = $(CFILES:.c=.obj)
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
    if exist "$(TARGET_DIR)\*" $(DELDIR) $(TARGET_DIR)

clean:
    @echo clean intermediate files
    if exist "$(TARGET_DIR)\*" attrib -r $(TARGET_DIR)\*.*
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.obj
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.pch
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.map
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.pdb
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.asm
