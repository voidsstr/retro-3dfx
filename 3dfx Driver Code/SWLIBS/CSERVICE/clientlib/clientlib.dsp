# Microsoft Developer Studio Project File - Name="clientlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CLIENTLIB - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "clientlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "clientlib.mak" CFG="CLIENTLIB - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clientlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "clientlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "clientlib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".." /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\csclient.lib"

!ELSEIF  "$(CFG)" == "clientlib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I ".." /I "..\..\include" /I "..\server" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\csclient.lib"

!ENDIF 

# Begin Target

# Name "clientlib - Win32 Release"
# Name "clientlib - Win32 Debug"
# Begin Source File

SOURCE=..\..\include\3dfx.h
# End Source File
# Begin Source File

SOURCE=.\buffer.c
# End Source File
# Begin Source File

SOURCE=.\csapi.h
# End Source File
# Begin Source File

SOURCE=.\csclient.c
# End Source File
# Begin Source File

SOURCE=.\csclient.h
# End Source File
# Begin Source File

SOURCE=..\server\csserver.h
# End Source File
# Begin Source File

SOURCE=..\cstypes.h
# End Source File
# Begin Source File

SOURCE=..\csvalues.h
# End Source File
# Begin Source File

SOURCE=.\data.c
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\debug.c
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\devcomm.c
# End Source File
# Begin Source File

SOURCE=.\execute.c
# End Source File
# Begin Source File

SOURCE=.\graphctx.c
# End Source File
# Begin Source File

SOURCE=.\overlay.c
# End Source File
# Begin Source File

SOURCE=.\protocol.c
# End Source File
# Begin Source File

SOURCE=.\sllist.c
# End Source File
# Begin Source File

SOURCE=.\snoop.c
# End Source File
# Begin Source File

SOURCE=.\swapbuf.c
# End Source File
# Begin Source File

SOURCE=.\vidmode.c
# End Source File
# End Target
# End Project
