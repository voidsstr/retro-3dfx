#
# glide and ATB use these flags...
# -7 -fp5 -5s -mf -bt#dos -D__DOS32__ -DPHARLAP -DGLIDE_HARDWARE

#
# WARNING:  you must set these variables in order for this file to work!
#
3DS4_PROCESS_DIR=c:\3ds4\process

ATB_LIBPATH=c:\3dfx\atb\lib\free
GLIDE_LIBPATH=c:\3dfx\atb\lib\free

ATB_INCLUDE=c:\3dfx\atb\include
GLIDE_INCLUDE=c:\3dfx\atb\include
#
# END WARNING
#


LINKDBG=Debug All
IPAS_LIBNAME=.\lib\xpwc10.lib
3DS_FILE_TOOLKIT_LIBNAME=.\lib\3dftkw95.lib

INCLUDE=c:\watcom\h
IPAS_INCLUDE=.\include
3DS_FILE_TOOLKIT_INCLUDE=.\include

LFLAGS=Option Stack=16384 Option Maxdata=0 Libpath C:\WATCOM\lib386;C:\WATCOM\lib386\dos;$ATB_LIBPATH;$GLIDE_LIBPATH Library sst1init.lib,glide.lib,atr.lib,atm.lib,atu.lib,texus.lib Library $IPAS_LIBNAME Library $3DS_FILE_TOOLKIT_LIBNAME Library clib3s.lib Library math387s.lib Format Pharlap Option Dosseg Option Symfile Option Quiet 


#CFLAGS=/w4 /s /j /ez /mf /fpi87 /3s /oail /zq /d2 /i=$INCLUDE /i=$IPAS_INCLUDE /i=$3DS_FILE_TOOLKIT_INCLUDE /i=$ATB_INCLUDE /i=$GLIDE_INCLUDE

CFLAGS=/w4 /s /j /ez /mf /fpi87 /3s /zq /d2 /i=$INCLUDE /i=$IPAS_INCLUDE /i=$3DS_FILE_TOOLKIT_INCLUDE /i=$ATB_INCLUDE /i=$GLIDE_INCLUDE /dPHARLAP=1 /d__DOS32__ /dGLIDE_HARDWARE $DEBUG_OUTPUT

OBJS=kview.obj util.obj joy.obj resize.obj

LFILES=FILE kview.obj,util.obj,joy.obj,resize.obj

DROP_FILES=renderdb.c readdb.c kxpshell.c kview.c util.c joy.c resize.c kview.h database.h joy.h mymalloc.h kxp.obj pxp.obj init.3de makefile vw3dfx_i.pxp makefile.wat av3dfx_i.pxp vw3dfx_i.kxp av3dfx_i.kxp

pxp: av3dfx_i.pxp vw3dfx_i.pxp

kxp: av3dfx_i.kxp vw3dfx_i.kxp

all:  vw3dfx_i.pxp vw3dfx_i.kxp av3dfx_i.pxp av3dfx_i.kxp

vw3dfx_i.pxp: $OBJS pxpshell.obj renderdb.obj pxp.obj readdb.obj av3dfx_i.pxp
	@set LFLAGS=$LFLAGS
	@set LFILES=$LFILES
	@set LINKDBG=$LINKDBG
	wlink $LINKDBG @LFILES FILE renderdb.obj FILE pxpshell.obj FILE readdb.obj FILE pxp.obj @LFLAGS NAME vw3dfx_i.pxp
	copy vw3dfx_i.pxp $3DS4_PROCESS_DIR

av3dfx_i.pxp: $OBJS pxpshell.obj renderdb.obj pxp.obj areaddb.obj
	@set LFLAGS=$LFLAGS
	@set LFILES=$LFILES
	@set LINKDBG=$LINKDBG
	wlink $LINKDBG @LFILES FILE renderdb.obj FILE pxpshell.obj FILE areaddb.obj FILE pxp.obj @LFLAGS NAME av3dfx_i.pxp
	copy av3dfx_i.pxp $3DS4_PROCESS_DIR

vw3dfx_i.kxp: $OBJS kxpshell.obj kxp.obj readdb.obj krendrdb.obj
	@set LFLAGS=$LFLAGS
	@set LFILES=$LFILES
	@set LINKDBG=$LINKDBG
	wlink $LINKDBG @LFILES FILE krendrdb.obj FILE kxpshell.obj FILE readdb.obj FILE kxp.obj @LFLAGS NAME vw3dfx_i.kxp
	copy vw3dfx_i.kxp $3DS4_PROCESS_DIR

av3dfx_i.kxp: $OBJS kxpshell.obj kxp.obj areaddb.obj krendrdb.obj
	@set LFLAGS=$LFLAGS
	@set LFILES=$LFILES
	@set LINKDBG=$LINKDBG
	wlink $LINKDBG @LFILES FILE krendrdb.obj FILE kxpshell.obj FILE areaddb.obj FILE kxp.obj @LFLAGS NAME av3dfx_i.kxp
	copy av3dfx_i.kxp $3DS4_PROCESS_DIR


test.exp: c:\test.obj
	@set LFLAGS=$LFLAGS
	@set LFILES=$LFILES
	@set LINKDBG=$LINKDBG
	wlink $LINKDBG FILE c:\test.obj @LFLAGS NAME test.exp

c:\test.obj:  c:\test.c
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=$*.obj $*.c

kxp.obj: c:\ipas4\examples\kxp\kxp.obj
	@copy /b /q c:\ipas4\examples\kxp\kxp.obj .

pxp.obj: c:\ipas4\examples\pxp\pxp.obj
	@copy /b /q c:\ipas4\examples\pxp\pxp.obj .

kxpshell.obj: kxpshell.c init.3de kview.h database.h joy.h	
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /dKXP /fo=kxpshell.obj kxpshell.c

pxpshell.obj: kxpshell.c init.3de kview.h database.h joy.h	
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=pxpshell.obj kxpshell.c

util.obj:  .\util.c .\database.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=$*.obj $*.c

areaddb.obj:  .\readdb.c .\database.h .\mymalloc.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=areaddb.obj readdb.c

readdb.obj:  .\readdb.c .\database.h .\mymalloc.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /DUSE_TEXUS /fo=$*.obj $*.c

renderdb.obj:  .\renderdb.c .\database.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=renderdb.obj renderdb.c

krendrdb.obj:  .\renderdb.c .\database.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /dKXP /fo=krendrdb.obj renderdb.c

resize.obj:  .\database.h .\resize.c .\mymalloc.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=$*.obj $*.c

kview.obj:  .\database.h .\kview.c .\mymalloc.h .\kview.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=$*.obj $*.c

joy.obj:  .\joy.h
	@set CFLAGS=$CFLAGS
	wcc386 @CFLAGS /fo=$*.obj $*.c

zip.lst: $DROP_FILES
	echo renderdb.c > zip.lst
	echo readdb.c >> zip.lst
	echo kview.c >> zip.lst
	echo kxpshell.c >> zip.lst
	echo util.c >> zip.lst
	echo joy.c >> zip.lst
	echo resize.c >> zip.lst
	echo kview.h >> zip.lst
	echo database.h >> zip.lst
	echo joy.h >> zip.lst
	echo mymalloc.h >> zip.lst
	echo kxp.obj >> zip.lst
	echo pxp.obj >> zip.lst
	echo init.3de >> zip.lst
	echo vw3dfx_i.pxp >> zip.lst
	echo av3dfx_i.pxp >> zip.lst
	echo vw3dfx_i.kxp >> zip.lst
	echo av3dfx_i.kxp >> zip.lst
	echo readme.txt >> zip.lst
	echo makefile.wat >> zip.lst
	echo lib\*.* >> zip.lst
	echo include\*.* >> zip.lst

drop.zip: zip.lst
	pkzip -P drop.zip @zip.lst


