# Microsoft Developer Studio Generated NMAKE File, Based on dll.dsp
!IF "$(CFG)" == ""
CFG=dll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dll - Win32 Release" && "$(CFG)" != "dll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dll.mak" CFG="dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dll - Win32 Release"

OUTDIR=.\dll___Wi
INTDIR=.\dll___Wi

!IF "$(RECURSE)" == "0" 

ALL : "..\splashplug\3dfxsplash.dll"

!ELSE 

ALL : "..\splashplug\3dfxsplash.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\Texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\3dfxsplash.exp"
	-@erase "$(OUTDIR)\3dfxsplash.lib"
	-@erase "..\splashplug\3dfxsplash.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "c:\bin\3dfx\glide\src\sst1\include" /I\
 "..\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D "SPLASH_DLL"\
 /Fp"$(INTDIR)\dll.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\dll___Wi/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\3dfxsplash.pdb" /machine:I386\
 /out:"..\splashplug\3dfxsplash.dll" /implib:"$(OUTDIR)\3dfxsplash.lib" 
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\Texcache.obj" \
	"..\..\Bin\Msvc50\Vc\Lib\Largeint.lib" \
	"..\..\Bin\Msvc50\Vc\Lib\Winmm.lib" \
	"..\lib\3dfx\glide2x.lib"

"..\splashplug\3dfxsplash.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

OUTDIR=.\dll___W0
INTDIR=.\dll___W0

!IF "$(RECURSE)" == "0" 

ALL : "..\splashplug\3dfxsplash.dll"

!ELSE 

ALL : "..\splashplug\3dfxsplash.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\Texcache.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\3dfxsplash.exp"
	-@erase "$(OUTDIR)\3dfxsplash.lib"
	-@erase "$(OUTDIR)\3dfxsplash.pdb"
	-@erase "..\splashplug\3dfxsplash.dll"
	-@erase "..\splashplug\3dfxsplash.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I\
 "c:\bin\3dfx\glide\src\sst1\include" /I "..\common" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "__MSC__" /D "SPLASH_DLL" /Fp"$(INTDIR)\dll.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\dll___W0/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)\3dfxsplash.pdb" /debug /machine:I386\
 /out:"..\splashplug\3dfxsplash.dll" /implib:"$(OUTDIR)\3dfxsplash.lib"\
 /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\Texcache.obj" \
	"..\..\Bin\Msvc50\Vc\Lib\Largeint.lib" \
	"..\..\Bin\Msvc50\Vc\Lib\Winmm.lib" \
	"..\lib\3dfx\glide2x.lib"

"..\splashplug\3dfxsplash.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "dll - Win32 Release" || "$(CFG)" == "dll - Win32 Debug"
SOURCE=..\common\GlideGraphics.cpp

!IF  "$(CFG)" == "dll - Win32 Release"

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


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

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

!IF  "$(CFG)" == "dll - Win32 Release"

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


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_MAIN_=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\fxdll.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	
NODEP_CPP_MAIN_=\
	"..\common\revglidegraphics.h"\
	"..\common\revgraphics.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

SOURCE=..\Common\RevPoint.cpp

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_REVPO=\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_REVPO=\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\Common\Texcache.cpp

!IF  "$(CFG)" == "dll - Win32 Release"

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
	

"$(INTDIR)\Texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_TEXCA=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\revpoint.h"\
	"..\common\revtypes.h"\
	"..\common\texcache.h"\
	
NODEP_CPP_TEXCA=\
	"..\common\revglidegraphics.h"\
	"..\common\revgraphics.h"\
	

"$(INTDIR)\Texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

