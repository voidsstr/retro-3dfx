# Microsoft Developer Studio Generated NMAKE File, Based on UTIL.DSP
!IF "$(CFG)" == ""
CFG=util - Win32 Release
!MESSAGE No configuration specified. Defaulting to util - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "util - Win32 Release" && "$(CFG)" != "util - Win32 Hybrid"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UTIL.MAK" CFG="util - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "util - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "util - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "util - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "d:\bin\3dsmax\plugins\utility.dlu"

!ELSE 

ALL : "d:\bin\3dsmax\plugins\utility.dlu"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\UTIL.pch"
	-@erase "$(INTDIR)\util.res"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\utility.exp"
	-@erase "$(OUTDIR)\utility.lib"
	-@erase "d:\bin\3dsmax\plugins\utility.dlu"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G5 /MD /W3 /GX /O2 /I "..\common" /I "..\include\3dfx" /I\
 "..\include\max" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__MSC__"\
 /Fp"$(INTDIR)\UTIL.pch" /Yu"util.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\util.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UTIL.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\utility.pdb"\
 /machine:I386 /def:".\util.def" /out:"d:\bin\3dsmax\plugins\utility.dlu"\
 /implib:"$(OUTDIR)\utility.lib" 
DEF_FILE= \
	".\util.def"
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\splash.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\util.res" \
	"..\lib\3dfx\glide2x.lib" \
	"..\lib\3dfx\texus.lib" \
	"..\lib\max\BMM.LIB" \
	"..\lib\max\CORE.LIB" \
	"..\lib\max\GEOM.LIB" \
	"..\lib\max\MESH.LIB" \
	"..\lib\max\UTIL.LIB"

"d:\bin\3dsmax\plugins\utility.dlu" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

OUTDIR=.\Hybrid
INTDIR=.\Hybrid

!IF "$(RECURSE)" == "0" 

ALL : "d:\bin\3dsmax\plugins\utility.dlu"

!ELSE 

ALL : "d:\bin\3dsmax\plugins\utility.dlu"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\GlideGraphics.obj"
	-@erase "$(INTDIR)\RevPoint.obj"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\texcache.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\UTIL.pch"
	-@erase "$(INTDIR)\util.res"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\utility.exp"
	-@erase "$(OUTDIR)\utility.lib"
	-@erase "$(OUTDIR)\utility.pdb"
	-@erase "d:\bin\3dsmax\plugins\utility.dlu"
	-@erase "d:\bin\3dsmax\plugins\utility.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "..\include\3dfx" /I "..\include\max" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "__MSC__" /Fp"$(INTDIR)\UTIL.pch" /Yu"util.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /c 
CPP_OBJS=.\Hybrid/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\util.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UTIL.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\utility.pdb" /debug\
 /machine:I386 /def:".\util.def" /out:"d:\bin\3dsmax\plugins\utility.dlu"\
 /implib:"$(OUTDIR)\utility.lib" 
DEF_FILE= \
	".\util.def"
LINK32_OBJS= \
	"$(INTDIR)\GlideGraphics.obj" \
	"$(INTDIR)\RevPoint.obj" \
	"$(INTDIR)\splash.obj" \
	"$(INTDIR)\texcache.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\util.res" \
	"..\lib\3dfx\glide2x.lib" \
	"..\lib\3dfx\texus.lib" \
	"..\lib\max\BMM.LIB" \
	"..\lib\max\CORE.LIB" \
	"..\lib\max\GEOM.LIB" \
	"..\lib\max\MESH.LIB" \
	"..\lib\max\UTIL.LIB"

"d:\bin\3dsmax\plugins\utility.dlu" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "util - Win32 Release" || "$(CFG)" == "util - Win32 Hybrid"
SOURCE=..\common\GlideGraphics.cpp

!IF  "$(CFG)" == "util - Win32 Release"

DEP_CPP_GLIDE=\
	"..\common\glidegraphics.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\include\3dfx\3dfx.h"\
	"..\include\3dfx\glide.h"\
	"..\include\3dfx\glidesys.h"\
	"..\include\3dfx\glideutl.h"\
	"..\include\3dfx\sst1vid.h"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /GX /O2 /I "..\common" /I "..\include\3dfx" /I\
 "..\include\max" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__MSC__"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

DEP_CPP_GLIDE=\
	"..\common\glidegraphics.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\include\3dfx\3dfx.h"\
	"..\include\3dfx\glide.h"\
	"..\include\3dfx\glidesys.h"\
	"..\include\3dfx\glideutl.h"\
	"..\include\3dfx\sst1vid.h"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "..\include\3dfx" /I "..\include\max" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "__MSC__" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\GlideGraphics.obj" : $(SOURCE) $(DEP_CPP_GLIDE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\common\RevPoint.cpp
DEP_CPP_REVPO=\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	

!IF  "$(CFG)" == "util - Win32 Release"

CPP_SWITCHES=/nologo /G5 /MD /W3 /GX /O2 /I "..\common" /I "..\include\3dfx" /I\
 "..\include\max" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__MSC__"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

CPP_SWITCHES=/nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "..\include\3dfx" /I "..\include\max" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "__MSC__" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\RevPoint.obj" : $(SOURCE) $(DEP_CPP_REVPO) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\splash.cpp

!IF  "$(CFG)" == "util - Win32 Release"

DEP_CPP_SPLAS=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\..\bin\3dfx\glide\src\swlibs\texus\lib\texus.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\common\TEXCACHE.H"\
	"d:\bin\3dsmax\maxsdk\include\bitmap.h"\
	"d:\bin\3dsmax\maxsdk\include\bmmlib.h"\
	"d:\bin\3dsmax\maxsdk\include\linklist.h"\
	"d:\bin\3dsmax\maxsdk\include\utilapi.h"\
	
NODEP_CPP_SPLAS=\
	"..\common\RevGlideGraphics.h"\
	"..\common\RevGraphics.h"\
	

"$(INTDIR)\splash.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"\
 "$(INTDIR)\UTIL.pch"


!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

DEP_CPP_SPLAS=\
	"..\common\glidegraphics.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\common\TEXCACHE.H"\
	"..\include\3dfx\3dfx.h"\
	"..\include\3dfx\glide.h"\
	"..\include\3dfx\glidesys.h"\
	"..\include\3dfx\glideutl.h"\
	"..\include\3dfx\sst1vid.h"\
	"..\include\3dfx\texus.h"\
	"..\include\max\bitmap.h"\
	"..\include\max\bmmlib.h"\
	"..\include\max\linklist.h"\
	"..\include\max\utilapi.h"\
	

"$(INTDIR)\splash.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"\
 "$(INTDIR)\UTIL.pch"


!ENDIF 

SOURCE=..\common\texcache.cpp

!IF  "$(CFG)" == "util - Win32 Release"

DEP_CPP_TEXCA=\
	"..\..\bin\3dfx\glide\src\sst1\include\3dfx.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glide.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glidesys.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\glideutl.h"\
	"..\..\bin\3dfx\glide\src\sst1\include\sst1vid.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\common\TEXCACHE.H"\
	
NODEP_CPP_TEXCA=\
	"..\common\RevGlideGraphics.h"\
	"..\common\RevGraphics.h"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /GX /O2 /I "..\common" /I "..\include\3dfx" /I\
 "..\include\max" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__MSC__"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

DEP_CPP_TEXCA=\
	"..\common\glidegraphics.h"\
	"..\common\RevPoint.h"\
	"..\common\RevTypes.h"\
	"..\common\TEXCACHE.H"\
	"..\include\3dfx\3dfx.h"\
	"..\include\3dfx\glide.h"\
	"..\include\3dfx\glidesys.h"\
	"..\include\3dfx\glideutl.h"\
	"..\include\3dfx\sst1vid.h"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "..\include\3dfx" /I "..\include\max" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "__MSC__" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\texcache.obj" : $(SOURCE) $(DEP_CPP_TEXCA) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\util.cpp

!IF  "$(CFG)" == "util - Win32 Release"

DEP_CPP_UTIL_=\
	".\UTIL.H"\
	"d:\bin\3dsmax\maxsdk\include\acolor.h"\
	"d:\bin\3dsmax\maxsdk\include\animtbl.h"\
	"d:\bin\3dsmax\maxsdk\include\assert1.h"\
	"d:\bin\3dsmax\maxsdk\include\bitarray.h"\
	"d:\bin\3dsmax\maxsdk\include\box2.h"\
	"d:\bin\3dsmax\maxsdk\include\box3.h"\
	"d:\bin\3dsmax\maxsdk\include\channels.h"\
	"d:\bin\3dsmax\maxsdk\include\cmdmode.h"\
	"d:\bin\3dsmax\maxsdk\include\color.h"\
	"d:\bin\3dsmax\maxsdk\include\control.h"\
	"d:\bin\3dsmax\maxsdk\include\coreexp.h"\
	"d:\bin\3dsmax\maxsdk\include\custcont.h"\
	"d:\bin\3dsmax\maxsdk\include\dbgprint.h"\
	"d:\bin\3dsmax\maxsdk\include\dpoint3.h"\
	"d:\bin\3dsmax\maxsdk\include\evuser.h"\
	"d:\bin\3dsmax\maxsdk\include\export.h"\
	"d:\bin\3dsmax\maxsdk\include\gencam.h"\
	"d:\bin\3dsmax\maxsdk\include\genhier.h"\
	"d:\bin\3dsmax\maxsdk\include\genlight.h"\
	"d:\bin\3dsmax\maxsdk\include\genshape.h"\
	"d:\bin\3dsmax\maxsdk\include\geom.h"\
	"d:\bin\3dsmax\maxsdk\include\geomlib.h"\
	"d:\bin\3dsmax\maxsdk\include\gfx.h"\
	"d:\bin\3dsmax\maxsdk\include\gfxlib.h"\
	"d:\bin\3dsmax\maxsdk\include\hitdata.h"\
	"d:\bin\3dsmax\maxsdk\include\hold.h"\
	"d:\bin\3dsmax\maxsdk\include\impapi.h"\
	"d:\bin\3dsmax\maxsdk\include\impexp.h"\
	"d:\bin\3dsmax\maxsdk\include\imtl.h"\
	"d:\bin\3dsmax\maxsdk\include\inode.h"\
	"d:\bin\3dsmax\maxsdk\include\interval.h"\
	"d:\bin\3dsmax\maxsdk\include\ioapi.h"\
	"d:\bin\3dsmax\maxsdk\include\iparamb.h"\
	"d:\bin\3dsmax\maxsdk\include\ipoint2.h"\
	"d:\bin\3dsmax\maxsdk\include\ipoint3.h"\
	"d:\bin\3dsmax\maxsdk\include\lockid.h"\
	"d:\bin\3dsmax\maxsdk\include\matrix2.h"\
	"d:\bin\3dsmax\maxsdk\include\matrix3.h"\
	"d:\bin\3dsmax\maxsdk\include\max.h"\
	"d:\bin\3dsmax\maxsdk\include\maxapi.h"\
	"d:\bin\3dsmax\maxsdk\include\maxcom.h"\
	"d:\bin\3dsmax\maxsdk\include\maxtypes.h"\
	"d:\bin\3dsmax\maxsdk\include\mesh.h"\
	"d:\bin\3dsmax\maxsdk\include\meshlib.h"\
	"d:\bin\3dsmax\maxsdk\include\mouseman.h"\
	"d:\bin\3dsmax\maxsdk\include\mtl.h"\
	"d:\bin\3dsmax\maxsdk\include\nametab.h"\
	"d:\bin\3dsmax\maxsdk\include\nurbs.h"\
	"d:\bin\3dsmax\maxsdk\include\nurbslib.h"\
	"d:\bin\3dsmax\maxsdk\include\nurbsobj.h"\
	"d:\bin\3dsmax\maxsdk\include\object.h"\
	"d:\bin\3dsmax\maxsdk\include\objmode.h"\
	"d:\bin\3dsmax\maxsdk\include\patch.h"\
	"d:\bin\3dsmax\maxsdk\include\patchlib.h"\
	"d:\bin\3dsmax\maxsdk\include\patchobj.h"\
	"d:\bin\3dsmax\maxsdk\include\plugapi.h"\
	"d:\bin\3dsmax\maxsdk\include\plugin.h"\
	"d:\bin\3dsmax\maxsdk\include\point2.h"\
	"d:\bin\3dsmax\maxsdk\include\point3.h"\
	"d:\bin\3dsmax\maxsdk\include\ptrvec.h"\
	"d:\bin\3dsmax\maxsdk\include\quat.h"\
	"d:\bin\3dsmax\maxsdk\include\ref.h"\
	"d:\bin\3dsmax\maxsdk\include\render.h"\
	"d:\bin\3dsmax\maxsdk\include\rtclick.h"\
	"d:\bin\3dsmax\maxsdk\include\sceneapi.h"\
	"d:\bin\3dsmax\maxsdk\include\snap.h"\
	"d:\bin\3dsmax\maxsdk\include\soundobj.h"\
	"d:\bin\3dsmax\maxsdk\include\stack.h"\
	"d:\bin\3dsmax\maxsdk\include\stack3.h"\
	"d:\bin\3dsmax\maxsdk\include\strbasic.h"\
	"d:\bin\3dsmax\maxsdk\include\strclass.h"\
	"d:\bin\3dsmax\maxsdk\include\tab.h"\
	"d:\bin\3dsmax\maxsdk\include\trig.h"\
	"d:\bin\3dsmax\maxsdk\include\triobj.h"\
	"d:\bin\3dsmax\maxsdk\include\units.h"\
	"d:\bin\3dsmax\maxsdk\include\utilexp.h"\
	"d:\bin\3dsmax\maxsdk\include\utillib.h"\
	"d:\bin\3dsmax\maxsdk\include\vedge.h"\
	"d:\bin\3dsmax\maxsdk\include\winutil.h"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /GX /O2 /I "..\common" /I "..\include\3dfx" /I\
 "..\include\max" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__MSC__"\
 /Fp"$(INTDIR)\UTIL.pch" /Yc"util.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\util.obj"	"$(INTDIR)\UTIL.pch" : $(SOURCE) $(DEP_CPP_UTIL_)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "util - Win32 Hybrid"

DEP_CPP_UTIL_=\
	"..\include\max\acolor.h"\
	"..\include\max\animtbl.h"\
	"..\include\max\assert1.h"\
	"..\include\max\bitarray.h"\
	"..\include\max\box2.h"\
	"..\include\max\box3.h"\
	"..\include\max\channels.h"\
	"..\include\max\cmdmode.h"\
	"..\include\max\color.h"\
	"..\include\max\control.h"\
	"..\include\max\coreexp.h"\
	"..\include\max\custcont.h"\
	"..\include\max\dbgprint.h"\
	"..\include\max\dpoint3.h"\
	"..\include\max\evuser.h"\
	"..\include\max\export.h"\
	"..\include\max\gencam.h"\
	"..\include\max\genhier.h"\
	"..\include\max\genlight.h"\
	"..\include\max\genshape.h"\
	"..\include\max\geom.h"\
	"..\include\max\geomlib.h"\
	"..\include\max\gfx.h"\
	"..\include\max\gfxlib.h"\
	"..\include\max\hitdata.h"\
	"..\include\max\hold.h"\
	"..\include\max\impapi.h"\
	"..\include\max\impexp.h"\
	"..\include\max\imtl.h"\
	"..\include\max\inode.h"\
	"..\include\max\interval.h"\
	"..\include\max\ioapi.h"\
	"..\include\max\iparamb.h"\
	"..\include\max\ipoint2.h"\
	"..\include\max\ipoint3.h"\
	"..\include\max\lockid.h"\
	"..\include\max\matrix2.h"\
	"..\include\max\matrix3.h"\
	"..\include\max\max.h"\
	"..\include\max\maxapi.h"\
	"..\include\max\maxcom.h"\
	"..\include\max\maxtypes.h"\
	"..\include\max\mesh.h"\
	"..\include\max\meshlib.h"\
	"..\include\max\mouseman.h"\
	"..\include\max\mtl.h"\
	"..\include\max\nametab.h"\
	"..\include\max\nurbs.h"\
	"..\include\max\nurbslib.h"\
	"..\include\max\nurbsobj.h"\
	"..\include\max\object.h"\
	"..\include\max\objmode.h"\
	"..\include\max\patch.h"\
	"..\include\max\patchlib.h"\
	"..\include\max\patchobj.h"\
	"..\include\max\plugapi.h"\
	"..\include\max\plugin.h"\
	"..\include\max\point2.h"\
	"..\include\max\point3.h"\
	"..\include\max\ptrvec.h"\
	"..\include\max\quat.h"\
	"..\include\max\ref.h"\
	"..\include\max\render.h"\
	"..\include\max\rtclick.h"\
	"..\include\max\sceneapi.h"\
	"..\include\max\snap.h"\
	"..\include\max\soundobj.h"\
	"..\include\max\stack.h"\
	"..\include\max\stack3.h"\
	"..\include\max\strbasic.h"\
	"..\include\max\strclass.h"\
	"..\include\max\tab.h"\
	"..\include\max\trig.h"\
	"..\include\max\triobj.h"\
	"..\include\max\units.h"\
	"..\include\max\utilexp.h"\
	"..\include\max\utillib.h"\
	"..\include\max\vedge.h"\
	"..\include\max\winutil.h"\
	".\UTIL.H"\
	
CPP_SWITCHES=/nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\common" /I\
 "..\include\3dfx" /I "..\include\max" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "__MSC__" /Fp"$(INTDIR)\UTIL.pch" /Yc"util.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /c 

"$(INTDIR)\util.obj"	"$(INTDIR)\UTIL.pch" : $(SOURCE) $(DEP_CPP_UTIL_)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\util.rc

"$(INTDIR)\util.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

