# Microsoft Developer Studio Project File - Name="playback" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=playback - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "playback.mak".
!MESSAGE 
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

# Begin Project
# PROP Scc_ProjName ""$/devel/swlibs/3DfxSplash/src/playback", RWSFAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "playback - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "NDEBUG" /D "PLAYBACK" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "playback - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "playback"
# PROP BASE Intermediate_Dir "playback"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DfxSplash 2x"
# PROP Intermediate_Dir "3DfxSplash 2x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /I "C:\3Dfx\Sdk\glide2x\src\include" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplashDLL\3DfxSpl3.dll"
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /dll /machine:I386 /out:"3DfxSplash 2x\3DfxSpl2.dll"
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "playbac0"
# PROP BASE Intermediate_Dir "playbac0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DfxSplash 3x"
# PROP Intermediate_Dir "3DfxSplash 3x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /I "C:\3Dfx\Sdk\glide3x\src\include" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplashDLL\3DfxSpl3.dll"
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplash 3x\3DfxSpl3.dll" /libpath:"c:\3dfx\sdk\glide3x\src\lib\win32"
# SUBTRACT LINK32 /pdb:none /debug

!ENDIF 

# Begin Target

# Name "playback - Win32 Release"
# Name "playback - Win32 Debug"
# Name "playback - Win32 3DfxSplashDLL 2x"
# Name "playback - Win32 3DfxSplashDLL 3x"
# Begin Source File

SOURCE=.\3DfxSplash.rc
# End Source File
# Begin Source File

SOURCE=.\convexhull.cpp
# End Source File
# Begin Source File

SOURCE=..\common\GlideGraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=..\common\RevPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\common\texcache.cpp
# End Source File
# End Target
# End Project
