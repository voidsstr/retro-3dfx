# Microsoft Developer Studio Generated NMAKE File, Based on playback.dsp
!IF "$(CFG)" == ""
CFG=playback - Win32 Debug
!MESSAGE No configuration specified. Defaulting to playback - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "playback - Win32 Release" && "$(CFG)" !=\
 "playback - Win32 Debug"
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

!IF "$(RECURSE)" == "0" 

ALL : "..\splashplug\playback.exe"

!ELSE 

ALL : "..\splashplug\playback.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "..\splashplug\playback.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\common" /I\
 "c:\bin\3dfx\glide\src\sst1\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "_MBCS" /D "__MSC__" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\playback.pdb" /machine:I386 /out:"..\splashplug\playback.exe" 
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"..\..\bin\msvc50\VC\lib\LARGEINT.LIB" \
	"..\..\bin\msvc50\VC\lib\WINMM.LIB" \
	"..\lib\3dfx\glide2x.lib"

"..\splashplug\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : "..\splashplug\playback.exe"

!ELSE 

ALL : "..\splashplug\playback.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\playback.pdb"
	-@erase "..\splashplug\playback.exe"
	-@erase "..\splashplug\playback.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "c:\bin\3dfx\glide\src\sst1\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "_MBCS" /D "__MSC__" /Fp"$(INTDIR)\playback.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\playback.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\playback.pdb" /debug /machine:I386\
 /out:"..\splashplug\playback.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\texcache.obj" \
	"..\..\bin\msvc50\VC\lib\LARGEINT.LIB" \
	"..\..\bin\msvc50\VC\lib\WINMM.LIB" \
	"..\lib\3dfx\glide2x.lib"

"..\splashplug\playback.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(CFG)" == "playback - Win32 Release" || "$(CFG)" ==\
 "playback - Win32 Debug"
SOURCE=..\common\GlideGraphics.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_GLIDE=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_GLIDE=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Main.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_MAIN_=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\fxdll.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_MAIN_=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\fxdll.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\RevPoint.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_REVPO=\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_REVPO=\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	
NODEP_CPP_REVPO=\
	"..\common\util.h"\
	"..\common\utilapi.h"\
	

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\texcache.cpp

!IF  "$(CFG)" == "playback - Win32 Release"

DEP_CPP_TEXCA=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

DEP_CPP_TEXCA=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\glidegraphics.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

