# Microsoft Developer Studio Project File - Name="playback" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=playback - Win32 Debug 2x
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "playback.mak".
!MESSAGE 
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
!MESSAGE "playback - Win32 3DfxSplashDLL_DBG3x" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
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

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 2x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "playback"
# PROP BASE Intermediate_Dir "playback"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DfxSplash2x"
# PROP Intermediate_Dir "3DfxSplash2x"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /dll /machine:I386 /out:"3DfxSplash2x\3dfxSpl2.dll"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Special Build Tool
TargetPath=.\3DfxSplash2x\3dfxSpl2.dll
SOURCE="$(InputPath)"
PostBuild_Desc=copy to c:\windows\system
PostBuild_Cmds=copy $(TargetPath) c:\windows\system
# End Special Build Tool

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL 3x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "playbac0"
# PROP BASE Intermediate_Dir "playbac0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DfxSplash3x"
# PROP Intermediate_Dir "3DfxSplash3x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /I "C:\3Dfx\Sdk\glide3x\src\include" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT CPP /WX /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplashDLL\3DfxSpl3.dll"
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplash3x\3dfxSpl3.dll" /libpath:"c:\3dfx\sdk\glide3x\src\lib\win32"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Special Build Tool
TargetPath=.\3DfxSplash3x\3dfxSpl3.dll
SOURCE="$(InputPath)"
PostBuild_Desc=copy to c:\windows\system
PostBuild_Cmds=copy $(TargetPath) c:\windows\system
# End Special Build Tool

!ELSEIF  "$(CFG)" == "playback - Win32 Debug 3x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "playback___Win32_Debug_3x"
# PROP BASE Intermediate_Dir "playback___Win32_Debug_3x"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug3x"
# PROP Intermediate_Dir "Debug3x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "playback - Win32 Debug 2x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "playback___Win32_Debug_2x"
# PROP BASE Intermediate_Dir "playback___Win32_Debug_2x"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug2x"
# PROP Intermediate_Dir "Debug2x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide3x\src\include" /D "USE_GLIDE3" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR /YX /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /I "c:\3dfx\sdk\glide2x\src\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSC__" /D "PLAYBACK" /FR /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide2x.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "playback - Win32 3DfxSplashDLL_DBG3x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "playback___Win32_3DfxSplashDLL_DBG3x"
# PROP BASE Intermediate_Dir "playback___Win32_3DfxSplashDLL_DBG3x"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "3DfxSplashDLL_DBG3x"
# PROP Intermediate_Dir "3DfxSplashDLL_DBG3x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /Ob2 /I "..\common" /I "C:\3Dfx\Sdk\glide3x\src\include" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /YX /FD /c
# SUBTRACT BASE CPP /WX /Fr
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /Ob2 /I "..\common" /I "C:\3Dfx\Sdk\glide3x\src\include" /D "USE_GLIDE3" /D "PLAYBACK" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__MSC__" /D "SPLASH_DLL" /D "_AFXDLL" /Fr /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /machine:I386 /out:"3DfxSplash3x\3dfxSpl3.dll" /libpath:"c:\3dfx\sdk\glide3x\src\lib\win32"
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib largeint.lib winmm.lib glide3x.lib /nologo /dll /debug /machine:I386 /out:"3DfxSplashDLL_DBG3x\3dfxSpl3.dll" /libpath:"c:\3dfx\sdk\glide3x\src\lib\win32"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetPath=.\3DfxSplashDLL_DBG3x\3dfxSpl3.dll
SOURCE="$(InputPath)"
PostBuild_Desc=copy to c:\windows\system
PostBuild_Cmds=copy $(TargetPath) c:\windows\system
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "playback - Win32 Release"
# Name "playback - Win32 3DfxSplashDLL 2x"
# Name "playback - Win32 3DfxSplashDLL 3x"
# Name "playback - Win32 Debug 3x"
# Name "playback - Win32 Debug 2x"
# Name "playback - Win32 3DfxSplashDLL_DBG3x"
# Begin Source File

SOURCE=.\3dfxplug.tga
# End Source File
# Begin Source File

SOURCE=.\3dfxsplash.dat
# End Source File
# Begin Source File

SOURCE=.\3DfxSplash.rc
# End Source File
# Begin Source File

SOURCE=..\common\clip.cpp
# End Source File
# Begin Source File

SOURCE=..\common\convexhull.cpp
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

SOURCE=..\common\scrncptr.cpp
# End Source File
# Begin Source File

SOURCE=..\common\texcache.cpp
# End Source File
# End Target
# End Project
