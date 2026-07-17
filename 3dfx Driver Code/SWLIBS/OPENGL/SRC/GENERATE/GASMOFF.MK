!include <win32.mak>

INC_DIR = ../include

TARGET = $(INC_DIR)/g_asmoff.h

LCINCS = -I../include -I../raster -I../include/GL

cflags = $(cflags) -D_WINDOWS -D_LANGUAGE_C -D__GL_CODEGEN \
	-D__GL_SUPPORT_MGL

$(INC_DIR)/g_asmoff.h: gasmoff.exe
	gasmoff.exe > $@

clean:
	-del *.obj

clobber: clean
	-del $(TARGET)

gasmoff.exe: $*.c
	$(cc) $(cdebug) $(LCINCS) $(cflags) $(cvars) $(scall) $*.c
	$(link) $(linkdebug) -out:$*.exe $*.obj
