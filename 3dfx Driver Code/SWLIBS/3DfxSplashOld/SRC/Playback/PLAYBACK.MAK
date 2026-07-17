# Microsoft Developer Studio Generated NMAKE File, Based on playback.dsp
!IF "$(CFG)" == ""
CFG=playback - Win32 Debug
!MESSAGE No configuration specified. Defaulting to playback - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "playback - Win32 Release" && "$(CFG)" !=\
 "playback - Win32 Debug" && "$(CFG)" != "playback - Win32 3DfxSplashDLL 2x" &&\
 "$(CFG)" != "playback - Win32 3DfxSplashDLL 3x"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "playback.mak" CFG="playback - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "playback - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 3DfxSplashDLL 2x" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 3DfxSplashDLL 3x" (based on\
 "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

HEADERS=$(BUILD_ROOT_CVG)\glide3\src
LIBINCLUDE=$(BUILD_ROOT_SWLIBS)\include
CVGINCLUDE=$(BUILD_ROOT_CVG)\include
NEWINCLUDE=/I $(HEADERS) /I $(LIBINCLUDE) /I $(CVGINCLUDE)

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "playback - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\playback.exe"

!ELSE 

ALL : "$(OUTDIR)\playback.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\playback.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 /I "..\common" /D "USE_GLIDE3" /D\
 "NDEBUG" /D "PLAYBACK" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__"\
 /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)\playback.pdb" /machine:I386\
 /out:"$(OUTDIR)\playback.exe" 
LINK32_OBJS= \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj"

"$(OUTDIR)\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\playback.exe" "$(OUTDIR)\playback.bsc"

!ELSE 

ALL : "$(OUTDIR)\playback.exe" "$(OUTDIR)\playback.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\convexhull.sbr"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\GlideGraphics.sbr"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Main.sbr"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\RevPoint.sbr"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\texcache.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\playback.bsc"
	-@erase "$(OUTDIR)\playback.exe"
	-@erase "$(OUTDIR)\playback.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\common" $(NEWINCLUDE) /I\
 "c:\3dfx\sdk\glide2x\src\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\playback.pch"\
 /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\convexhull.sbr" \
	"$(INTDIR)\GlideGraphics.sbr" \
	"$(INTDIR)\Main.sbr" \
	"$(INTDIR)\RevPoint.sbr" \
	"$(INTDIR)\texcache.sbr"

"$(OUTDIR)\playback.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)\playback.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\playback.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj"

"$(OUTDIR)\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

OUTDIR=.\3DfxSplash 2x
INTDIR=.\3DfxSplash 2x
# Begin Custom Macros
OutDir=.\3DfxSplash 2x
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\3DfxSpl2.dll"

!ELSE 

ALL : "$(OUTDIR)\3DfxSpl2.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\3DfxSpl2.dll"
	-@erase "$(OUTDIR)\3DfxSpl2.exp"
	-@erase "$(OUTDIR)\3DfxSpl2.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 /I "..\common" $(NEWINCLUDE) /I\
 "C:\3Dfx\Sdk\glide2x\src\include" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /Fp"$(INTDIR)\playback.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=".\3DfxSplash 2x/"
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /dll /incremental:no\
 /pdb:"$(OUTDIR)\3DfxSpl2.pdb" /machine:I386 /out:"$(OUTDIR)\3DfxSpl2.dll"\
 /implib:"$(OUTDIR)\3DfxSpl2.lib" 
LINK32_OBJS= \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj"

"$(OUTDIR)\3DfxSpl2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

OUTDIR=.\3DfxSplash 3x
INTDIR=.\3DfxSplash 3x
# Begin Custom Macros
OutDir=.\3DfxSplash 3x
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\3DfxSpl3.dll"

!ELSE 

ALL : "$(OUTDIR)\3DfxSpl3.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\3DfxSpl3.dll"
	-@erase "$(OUTDIR)\3DfxSpl3.exp"
	-@erase "$(OUTDIR)\3DfxSpl3.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 /I "..\common" $(NEWINCLUDE) /I\
 "C:\3Dfx\Sdk\glide3x\src\include" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D\
 "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /Fp"$(INTDIR)\playback.pch"\
 /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=".\3DfxSplash 3x/"
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /incremental:no\
 /pdb:"$(OUTDIR)\3DfxSpl3.pdb" /machine:I386 /out:"$(OUTDIR)\3DfxSpl3.dll"\
 /implib:"$(OUTDIR)\3DfxSpl3.lib" 
LINK32_OBJS= \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj"

"$(OUTDIR)\3DfxSpl3.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "playback - Win32 Release" || "$(CFG)" ==\
 "playback - Win32 Debug" || "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x" ||\
 "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"
SOURCE=.\3DfxSplash.rc
DEP_RSC_3DFXS=\
	".\3dfxsplash.dat"\
	".\3dfxsplash.wav"\
	

"$(INTDIR)\3DfxSplash.res" : $(SOURCE) $(DEP_RSC_3DFXS) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\convexhull.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_CONVE=\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\convexhull.obj" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_CONVE=\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\convexhull.obj"	"$(INTDIR)\convexhull.sbr" : $(SOURCE)\
 $(DEP_CPP_CONVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

DEP_CPP_CONVE=\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\convexhull.obj" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

DEP_CPP_CONVE=\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\convexhull.obj" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\GlideGraphics.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_GLIDE=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_GLIDE=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\GlideGraphics.obj"	"$(INTDIR)\GlideGraphics.sbr" : $(SOURCE)\
 $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

DEP_CPP_GLIDE=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

DEP_CPP_GLIDE=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	".\convexhull.h"\
	

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Main.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_MAIN_=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	".\convexhull.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_MAIN_=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\fxdll.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	".\convexhull.h"\
	

"$(INTDIR)\Main.obj"	"$(INTDIR)\Main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

DEP_CPP_MAIN_=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\fxdll.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	".\convexhull.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

DEP_CPP_MAIN_=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	".\convexhull.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\RevPoint.cpp
DEP_CPP_REVPO=\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"


"$(INTDIR)\RevPoint.obj"	"$(INTDIR)\RevPoint.sbr" : $(SOURCE) $(DEP_CPP_REVPO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\texcache.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_TEXCA=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_TEXCA=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj"	"$(INTDIR)\texcache.sbr" : $(SOURCE) $(DEP_CPP_TEXCA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

DEP_CPP_TEXCA=\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\3dfx.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glide.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glidesys.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\glideutl.h"\
	"..\..\..\..\..\3dfx\sdk\glide2x\src\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

DEP_CPP_TEXCA=\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

