# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Runner - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Runner - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Runner - Win32 Release" && "$(CFG)" != "Runner - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Runner.mak" CFG="Runner - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Runner - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Runner - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Runner - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Runner - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Runner.exe"

CLEAN : 
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\Runner.obj"
	-@erase "$(INTDIR)\Runner.pch"
	-@erase "$(INTDIR)\Runner.res"
	-@erase "$(INTDIR)\RunnerDoc.obj"
	-@erase "$(INTDIR)\RunnerView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\Runner.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=fl32.exe
# ADD BASE F90 /I "Release/"
# ADD F90 /I "Release/"
F90_OBJS=.\Release/
F90_PROJ=/I "Release/" /Fo"Release/" 

.for{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

.f{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

.f90{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Runner.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c\
 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Runner.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Runner.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Runner.pdb" /machine:I386 /out:"$(OUTDIR)/Runner.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Runner.obj" \
	"$(INTDIR)\Runner.res" \
	"$(INTDIR)\RunnerDoc.obj" \
	"$(INTDIR)\RunnerView.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Runner.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Runner.exe" "$(OUTDIR)\Runner.bsc"

CLEAN : 
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\Runner.obj"
	-@erase "$(INTDIR)\Runner.pch"
	-@erase "$(INTDIR)\Runner.res"
	-@erase "$(INTDIR)\Runner.sbr"
	-@erase "$(INTDIR)\RunnerDoc.obj"
	-@erase "$(INTDIR)\RunnerDoc.sbr"
	-@erase "$(INTDIR)\RunnerView.obj"
	-@erase "$(INTDIR)\RunnerView.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Runner.bsc"
	-@erase "$(OUTDIR)\Runner.exe"
	-@erase "$(OUTDIR)\Runner.ilk"
	-@erase "$(OUTDIR)\Runner.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=fl32.exe
# ADD BASE F90 /I "Debug/"
# ADD F90 /I "Debug/"
F90_OBJS=.\Debug/
F90_PROJ=/I "Release/" /Fo"Release/" 

.for{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

.f{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

.f90{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<  

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Runner.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Runner.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Runner.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\Runner.sbr" \
	"$(INTDIR)\RunnerDoc.sbr" \
	"$(INTDIR)\RunnerView.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Runner.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Runner.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Runner.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Runner.obj" \
	"$(INTDIR)\Runner.res" \
	"$(INTDIR)\RunnerDoc.obj" \
	"$(INTDIR)\RunnerView.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Runner.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "Runner - Win32 Release"
# Name "Runner - Win32 Debug"

!IF  "$(CFG)" == "Runner - Win32 Release"

!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "Runner - Win32 Release"

!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Runner.cpp
DEP_CPP_RUNNE=\
	".\MainFrm.h"\
	".\Runner.h"\
	".\RunnerDoc.h"\
	".\RunnerView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Runner - Win32 Release"


"$(INTDIR)\Runner.obj" : $(SOURCE) $(DEP_CPP_RUNNE) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"


"$(INTDIR)\Runner.obj" : $(SOURCE) $(DEP_CPP_RUNNE) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"

"$(INTDIR)\Runner.sbr" : $(SOURCE) $(DEP_CPP_RUNNE) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Runner - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Runner.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Runner.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Runner.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Runner.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\Runner.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Runner - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RunnerDoc.cpp
DEP_CPP_RUNNER=\
	".\Runner.h"\
	".\RunnerDoc.h"\
	".\RunnerView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Runner - Win32 Release"


"$(INTDIR)\RunnerDoc.obj" : $(SOURCE) $(DEP_CPP_RUNNER) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"


"$(INTDIR)\RunnerDoc.obj" : $(SOURCE) $(DEP_CPP_RUNNER) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"

"$(INTDIR)\RunnerDoc.sbr" : $(SOURCE) $(DEP_CPP_RUNNER) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RunnerView.cpp
DEP_CPP_RUNNERV=\
	".\Runner.h"\
	".\RunnerDoc.h"\
	".\RunnerView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Runner - Win32 Release"


"$(INTDIR)\RunnerView.obj" : $(SOURCE) $(DEP_CPP_RUNNERV) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ELSEIF  "$(CFG)" == "Runner - Win32 Debug"


"$(INTDIR)\RunnerView.obj" : $(SOURCE) $(DEP_CPP_RUNNERV) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"

"$(INTDIR)\RunnerView.sbr" : $(SOURCE) $(DEP_CPP_RUNNERV) "$(INTDIR)"\
 "$(INTDIR)\Runner.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Runner.rc
DEP_RSC_RUNNER_=\
	".\res\bitmap1.bmp"\
	".\res\Runner.ico"\
	".\res\Runner.rc2"\
	".\res\RunnerDoc.ico"\
	".\res\toolbar1.bmp"\
	

"$(INTDIR)\Runner.res" : $(SOURCE) $(DEP_RSC_RUNNER_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
