# Microsoft Developer Studio Generated NMAKE File, Based on playback.dsp
!IF "$(CFG)" == ""
CFG=playback - Win32 Debug 2x
!MESSAGE No configuration specified. Defaulting to playback - Win32 Debug 2x.
!ENDIF 

!IF "$(CFG)" != "playback - Win32 Release" && "$(CFG)" != "playback - Win32 3DfxSplashDLL 2x" && "$(CFG)" != "playback - Win32 3DfxSplashDLL 3x" && "$(CFG)" != "playback - Win32 Debug 3x" && "$(CFG)" != "playback - Win32 Debug 2x"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "playback.mak" CFG="playback - Win32 Debug 2x"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "playback - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 3DfxSplashDLL 2x" (based on "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 3DfxSplashDLL 3x" (based on "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 Debug 3x" (based on "Win32 (x86) Console Application")
!MESSAGE "playback - Win32 Debug 2x" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "playback - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\playback.exe"


CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\playback.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "NDEBUG" /D "PLAYBACK" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\playback.pdb" /machine:I386 /out:"$(OUTDIR)\playback.exe" 
LINK32_OBJS= \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj"

"$(OUTDIR)\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

OUTDIR=.\3DfxSplash2x
INTDIR=.\3DfxSplash2x
# Begin Custom Macros
OutDir=.\3DfxSplash2x
# End Custom Macros

ALL : "$(OUTDIR)\3DfxSpl2x.dll"


CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\3DfxSpl2x.dll"
	-@erase "$(OUTDIR)\3DfxSpl2x.exp"
	-@erase "$(OUTDIR)\3DfxSpl2x.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

HEADERS=$(BUILD_ROOT_CVG)\glide\src
LIBINCLUDE=$(BUILD_ROOT_SWLIBS)\include
CVGINCLUDE=$(BUILD_ROOT_CVG)\include
ALLINCS=/I $(HEADERS) /I $(LIBINCLUDE) /I $(CVGINCLUDE) /I "..\common"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 $(ALLINCS) /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\3DfxSpl2x.pdb" /machine:I386 /out:"$(OUTDIR)\3DfxSpl2x.dll" /implib:"$(OUTDIR)\3DfxSpl2x.lib" 
LINK32_OBJS= \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj"

"$(OUTDIR)\3DfxSpl2x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

OUTDIR=.\3DfxSplash3x
INTDIR=.\3DfxSplash3x
# Begin Custom Macros
OutDir=.\3DfxSplash3x
# End Custom Macros

ALL : "$(OUTDIR)\3DfxSpl3x.dll"


CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\convexhull.obj"
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\3DfxSpl3x.dll"
	-@erase "$(OUTDIR)\3DfxSpl3x.exp"
	-@erase "$(OUTDIR)\3DfxSpl3x.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

HEADERS=$(BUILD_ROOT_CVG)\glide\src
LIBINCLUDE=$(BUILD_ROOT_SWLIBS)\include
CVGINCLUDE=$(BUILD_ROOT_CVG)\include
ALLINCS=/I $(HEADERS) /I $(LIBINCLUDE) /I $(CVGINCLUDE) /I "..\common"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 $(ALLINCS) /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\3DfxSpl3x.pdb" /machine:I386 /out:"$(OUTDIR)\3DfxSpl3x.dll" /implib:"$(OUTDIR)\3DfxSpl3x.lib" /libpath:"c:\3dfx\sdk\glide3x\src\lib\win32" 
LINK32_OBJS= \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj"

"$(OUTDIR)\3DfxSpl3x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"

OUTDIR=.\Debug3x
INTDIR=.\Debug3x
# Begin Custom Macros
OutDir=.\Debug3x
# End Custom Macros

ALL : "$(OUTDIR)\playback.exe" "$(OUTDIR)\playback.bsc"


CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\clip.sbr"
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
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\playback.bsc"
	-@erase "$(OUTDIR)\playback.exe"
	-@erase "$(OUTDIR)\playback.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\clip.sbr" \
	"$(INTDIR)\GlideGraphics.sbr" \
	"$(INTDIR)\Main.sbr" \
	"$(INTDIR)\RevPoint.sbr" \
	"$(INTDIR)\texcache.sbr" \
	"$(INTDIR)\convexhull.sbr"

"$(OUTDIR)\playback.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\playback.pdb" /debug /machine:I386 /out:"$(OUTDIR)\playback.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj"

"$(OUTDIR)\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"

OUTDIR=.\Debug2x
INTDIR=.\Debug2x
# Begin Custom Macros
OutDir=.\Debug2x
# End Custom Macros

ALL : "$(OUTDIR)\playback.exe" "$(OUTDIR)\playback.bsc"


CLEAN :
	-@erase "$(INTDIR)\3DfxSplash.res"
	-@erase "$(INTDIR)\clip.obj"
	-@erase "$(INTDIR)\clip.sbr"
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
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\playback.bsc"
	-@erase "$(OUTDIR)\playback.exe"
	-@erase "$(OUTDIR)\playback.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide2x\src\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\3DfxSplash.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\clip.sbr" \
	"$(INTDIR)\GlideGraphics.sbr" \
	"$(INTDIR)\Main.sbr" \
	"$(INTDIR)\RevPoint.sbr" \
	"$(INTDIR)\texcache.sbr" \
	"$(INTDIR)\convexhull.sbr"

"$(OUTDIR)\playback.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\playback.pdb" /debug /machine:I386 /out:"$(OUTDIR)\playback.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clip.obj" \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\3DfxSplash.res" \
	"$(INTDIR)\convexhull.obj"

"$(OUTDIR)\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("playback.dep")
!INCLUDE "playback.dep"
!ELSE 
!MESSAGE Warning: cannot find "playback.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "playback - Win32 Release" || "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x" || "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x" || "$(CFG)" == "playback - Win32 Debug 3x" || "$(CFG)" == "playback - Win32 Debug 2x"
SOURCE=.\3DfxSplash.rc

"$(INTDIR)\3DfxSplash.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\common\clip.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\clip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\clip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\clip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\clip.obj"	"$(INTDIR)\clip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\clip.obj"	"$(INTDIR)\clip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\convexhull.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\convexhull.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\convexhull.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\convexhull.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\convexhull.obj"	"$(INTDIR)\convexhull.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\convexhull.obj"	"$(INTDIR)\convexhull.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\GlideGraphics.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\GlideGraphics.obj"	"$(INTDIR)\GlideGraphics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\GlideGraphics.obj"	"$(INTDIR)\GlideGraphics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Main.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\Main.obj"	"$(INTDIR)\Main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\Main.obj"	"$(INTDIR)\Main.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\RevPoint.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\RevPoint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\RevPoint.obj"	"$(INTDIR)\RevPoint.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\RevPoint.obj"	"$(INTDIR)\RevPoint.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\texcache.cpp

!IF  "$(CFG)" == "playback - Win32 Release"


"$(INTDIR)\texcache.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"


"$(INTDIR)\texcache.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"


"$(INTDIR)\texcache.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"


"$(INTDIR)\texcache.obj"	"$(INTDIR)\texcache.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"


"$(INTDIR)\texcache.obj"	"$(INTDIR)\texcache.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

