#
# Copyright (c) 1996, 3Dfx Interactive, Inc.
# All Rights Reserved.

LUA_TOP		= ..\lua
MDIAG_TOP 	= ..
ELIBDIR		= vgatst\lib

THISDIR = core
SUBDIRS = vgatst gplay

MAKE = wmake /f wmake.mak

# local defines, options, includes
CC      = wcc386
LCDEFS  = -DBUILD_DIAGS -D__DOS32__ -DDIAG_BUILD
LCOPTS  = -w4 -e25 -7 -fp5 -zq -5s -ei -mf -bt#dos 
LCINCS  = -i=..\3dfx\include -I..\lua\include

# sources
TARGET   = mdc.exe
CFILES   = main.c errrpt.c pcibrd.c banshee.c init.c mdclua.c misc.c memerr.c memtst.c pcitst.c builtin.c version.c crc.c lm_bios.c lm_ban.c lm_glde.c vgacore.c
OFILES   = main.obj errrpt.obj pcibrd.obj banshee.obj init.obj mdclua.obj misc.obj memerr.obj memtst.obj pcitst.obj builtin.obj version.obj crc.obj lm_bios.obj lm_ban.obj lm_glde.obj vgacore.obj
		
 
LLDLIBS = fxpci.lib llua.lib luaaux.lib h3cinit.lib minihwc.lib glide2x.lib fxmisc.lib vgacore.lib ediag.lib ediag1.lib vgadata.lib elib.lib repdev.lib gplay.lib

DEBUG = 1

##### Build Option Macros #####
LDIRS = .\vgatst\lib ..\3dfx\lib ..\lua\lib .\gplay
!ifdef DEBUG
DDEF		= -DDEBUG
COPT		= -e25 -zq -d2 -mf -5r -hw -od -wx
LOPT		= SYS dos4g op m debug all op st=8192 op maxe=25 op q
!else
DDEF		=
COPT		= -w4 -e25 -zq -otexan -d1 -mf -5r
LOPT		= SYS dos4g op m op st=8192 op maxe=25 op q op symf
!endif

LINK = wlink d all OP m OP ST=500k OP MAXE=25 OP Q SYS dos4g 

.c.obj:
	set wcc386=-d2 $(LCINCS) $(LCDEFS) $(LCOPTS)
	$(CC) $[*
	
all: subdirs $(TARGET) install
		
##### Dependents For Goal and Command Line #####
$(TARGET):  $(OFILES) foo.lnk
	$(LINK) @foo


subdirs: .SYMBOLIC
	cd vgatst
	$(MAKE)
	cd ..
	cd gplay
	$(MAKE)
	cd ..

install: .SYMBOLIC
	mkdir $(MDIAG_TOP)\bin
	-if exist $(MDIAG_TOP)\bin\*.* attrib -r $(MDIAG_TOP)\bin\*.*
	xcopy /s mdc.exe $(MDIAG_TOP)\bin

# compile a lua file into a c symbol file
# we need to fix using a compiled lua string.

clean: .SYMBOLIC
	-erase *.obj
	-if exist lib\*.lib erase lib\*.lib
	-erase mdc.exe
	cd gplay
	$(MAKE) clean
	cd ..
	cd vgatst
	$(MAKE) clean
       cd vgacore
       $(MAKE) clean
       cd ..
	cd ..
builtin.luc: builtin.lua
	$(MDIAG_TOP)\btools\luac -o builtin.luc builtin.lua 	

builtin.c: builtin.lua
	$(MDIAG_TOP)\btools\lua bytecomp.lua < builtin.lua > builtin.c

.SILENT
foo.lnk: .SYMBOLIC
 	%create $^@
	%append $^@ NAME mdc
	%append $^@ DEBUG all
	for %i in ($(LDIRS)) do %append $^@ LIBPath %i
	for %i in ($(OFILES))  do %append $^@ FILE %i
	for %i in ($(LLDLIBS)) do %append $^@ library %i

