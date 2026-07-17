TOP=..\..

include ../../make.config

MSFMT = msfmt.exe

GLDOTH = $(TOP)\include\GL\gl.h
GLCLTPT= glcltpt.c

all: $(TOP)\include\g_disp_dt.h

$(TOP)\include\g_disp_dt.h: $(MSFMT) $(GLDOTH) $(GLCLTPT)
	$(MSFMT) $(GLDOTH) $(GLCLTPT) >$(TOP)\include\g_disp_dt.h

%.exe : %.c
	$(CC) $*.c -o $*.exe
