# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=texture - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to texture - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "texture - Win32 Release" && "$(CFG)" !=\
 "texture - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "texture.mak" CFG="texture - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "texture - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "texture - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "texture - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "texture - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\texture.exe"

CLEAN : 
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(OUTDIR)\texture.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "\s\devel\sst1\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "\s\devel\sst1\include" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "__MSC__" /Fp"$(INTDIR)/texture.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/texture.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glide2x.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=opengl32.lib glu32.lib glide2x.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/texture.pdb" /machine:I386\
 /out:"$(OUTDIR)/texture.exe" 
LINK32_OBJS= \
	"$(INTDIR)\texture.obj"

"$(OUTDIR)\texture.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "texture - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\texture.exe"

CLEAN : 
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\texture.exe"
	-@erase "$(OUTDIR)\texture.ilk"
	-@erase "$(OUTDIR)\texture.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "\s\devel\sst1\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "\s\devel\sst1\include" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /Fp"$(INTDIR)/texture.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/texture.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glide2x.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=opengl32.lib glu32.lib glide2x.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/texture.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/texture.exe" 
LINK32_OBJS= \
	"$(INTDIR)\texture.obj"

"$(OUTDIR)\texture.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "texture - Win32 Release"
# Name "texture - Win32 Debug"

!IF  "$(CFG)" == "texture - Win32 Release"

!ELSEIF  "$(CFG)" == "texture - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\texture.c

!IF  "$(CFG)" == "texture - Win32 Release"

DEP_CPP_TEXTU=\
	"\s\devel\sst1\include\glide.h"\
	"\s\devel\sst1\include\glidesys.h"\
	"\s\devel\sst1\include\glideutl.h"\
	"\s\devel\sst1\include\sst1vid.h"\
	{$(INCLUDE)}"\3dfx.h"\
	{$(INCLUDE)}"\gl\Gl.h"\
	{$(INCLUDE)}"\gl\Glu.h"\
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "texture - Win32 Debug"

DEP_CPP_TEXTU=\
	"\s\devel\sst1\include\glide.h"\
	"\s\devel\sst1\include\glidesys.h"\
	"\s\devel\sst1\include\glideutl.h"\
	"\s\devel\sst1\include\sst1vid.h"\
	{$(INCLUDE)}"\3dfx.h"\
	{$(INCLUDE)}"\gl\Gl.h"\
	{$(INCLUDE)}"\gl\Glu.h"\
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
