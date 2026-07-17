# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=ogtst - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to ogtst - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ogtst - Win32 Release" && "$(CFG)" != "ogtst - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ogtst.mak" CFG="ogtst - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ogtst - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ogtst - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "ogtst - Win32 Release"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "ogtst - Win32 Release"

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

ALL : "$(OUTDIR)\ogtst.exe"

CLEAN : 
	-@erase "$(INTDIR)\aafatlines.obj"
	-@erase "$(INTDIR)\abgr.obj"
	-@erase "$(INTDIR)\accbuf.obj"
	-@erase "$(INTDIR)\alloc.obj"
	-@erase "$(INTDIR)\alphafunc.obj"
	-@erase "$(INTDIR)\arbcliplin.obj"
	-@erase "$(INTDIR)\arbcliptri.obj"
	-@erase "$(INTDIR)\array.obj"
	-@erase "$(INTDIR)\bgnend.obj"
	-@erase "$(INTDIR)\bitmaps.obj"
	-@erase "$(INTDIR)\blendfunc.obj"
	-@erase "$(INTDIR)\calllist.obj"
	-@erase "$(INTDIR)\cancliplin.obj"
	-@erase "$(INTDIR)\cancliptri.obj"
	-@erase "$(INTDIR)\check.obj"
	-@erase "$(INTDIR)\clamp.obj"
	-@erase "$(INTDIR)\clear.obj"
	-@erase "$(INTDIR)\clearaccum.obj"
	-@erase "$(INTDIR)\clearall.obj"
	-@erase "$(INTDIR)\cleardepth.obj"
	-@erase "$(INTDIR)\clearstencil.obj"
	-@erase "$(INTDIR)\clip_edge_flags.obj"
	-@erase "$(INTDIR)\clipcolorlin.obj"
	-@erase "$(INTDIR)\clipcolorpnt.obj"
	-@erase "$(INTDIR)\clipcolortri.obj"
	-@erase "$(INTDIR)\cliptextlin.obj"
	-@erase "$(INTDIR)\cliptexttri.obj"
	-@erase "$(INTDIR)\cmpltcolorlin.obj"
	-@erase "$(INTDIR)\cmpltcolorpnt.obj"
	-@erase "$(INTDIR)\cmpltcolortri.obj"
	-@erase "$(INTDIR)\cmplttextlin.obj"
	-@erase "$(INTDIR)\cmplttexttri.obj"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\complement_one_plane.obj"
	-@erase "$(INTDIR)\copytex.obj"
	-@erase "$(INTDIR)\ctxsw.obj"
	-@erase "$(INTDIR)\cullface.obj"
	-@erase "$(INTDIR)\cyls.obj"
	-@erase "$(INTDIR)\defaultState.obj"
	-@erase "$(INTDIR)\depthbuf.obj"
	-@erase "$(INTDIR)\depthfunc.obj"
	-@erase "$(INTDIR)\disks.obj"
	-@erase "$(INTDIR)\doodle.obj"
	-@erase "$(INTDIR)\edgeflag.obj"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\eval.obj"
	-@erase "$(INTDIR)\evalattrib.obj"
	-@erase "$(INTDIR)\evaluator.obj"
	-@erase "$(INTDIR)\evalvertexmix.obj"
	-@erase "$(INTDIR)\extvertarray.obj"
	-@erase "$(INTDIR)\fog.obj"
	-@erase "$(INTDIR)\frustum.obj"
	-@erase "$(INTDIR)\getenv.obj"
	-@erase "$(INTDIR)\getmatrix.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getstatecheck.obj"
	-@erase "$(INTDIR)\getviewport.obj"
	-@erase "$(INTDIR)\glPixelTransfer.obj"
	-@erase "$(INTDIR)\glRasterPos.obj"
	-@erase "$(INTDIR)\gltst.obj"
	-@erase "$(INTDIR)\imbuildutil.obj"
	-@erase "$(INTDIR)\imgutil.obj"
	-@erase "$(INTDIR)\imutil.obj"
	-@erase "$(INTDIR)\index.obj"
	-@erase "$(INTDIR)\info.obj"
	-@erase "$(INTDIR)\inout.obj"
	-@erase "$(INTDIR)\internalformat.obj"
	-@erase "$(INTDIR)\light.obj"
	-@erase "$(INTDIR)\lightsimulate.obj"
	-@erase "$(INTDIR)\line.obj"
	-@erase "$(INTDIR)\linestrip.obj"
	-@erase "$(INTDIR)\listcov.obj"
	-@erase "$(INTDIR)\logicop.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrixflow.obj"
	-@erase "$(INTDIR)\nest.obj"
	-@erase "$(INTDIR)\nodraw.obj"
	-@erase "$(INTDIR)\normals.obj"
	-@erase "$(INTDIR)\ortho.obj"
	-@erase "$(INTDIR)\packedpix.obj"
	-@erase "$(INTDIR)\pdisks.obj"
	-@erase "$(INTDIR)\pgonoffset.obj"
	-@erase "$(INTDIR)\pixel.obj"
	-@erase "$(INTDIR)\pixelinv.obj"
	-@erase "$(INTDIR)\pixutil.obj"
	-@erase "$(INTDIR)\point.obj"
	-@erase "$(INTDIR)\pointsize.obj"
	-@erase "$(INTDIR)\polys.obj"
	-@erase "$(INTDIR)\polystipple.obj"
	-@erase "$(INTDIR)\popmatrix.obj"
	-@erase "$(INTDIR)\pushattrib.obj"
	-@erase "$(INTDIR)\qstrip.obj"
	-@erase "$(INTDIR)\quads.obj"
	-@erase "$(INTDIR)\rand.obj"
	-@erase "$(INTDIR)\ReadPixelsRGBA.obj"
	-@erase "$(INTDIR)\readwrite.obj"
	-@erase "$(INTDIR)\rect.obj"
	-@erase "$(INTDIR)\rgblogicop.obj"
	-@erase "$(INTDIR)\rotate.obj"
	-@erase "$(INTDIR)\scale.obj"
	-@erase "$(INTDIR)\set.obj"
	-@erase "$(INTDIR)\sgilib.obj"
	-@erase "$(INTDIR)\shade.obj"
	-@erase "$(INTDIR)\spec.obj"
	-@erase "$(INTDIR)\spheres.obj"
	-@erase "$(INTDIR)\stencil.obj"
	-@erase "$(INTDIR)\stencilmask.obj"
	-@erase "$(INTDIR)\stereobuffer.obj"
	-@erase "$(INTDIR)\texcoords.obj"
	-@erase "$(INTDIR)\texobj.obj"
	-@erase "$(INTDIR)\texobject.obj"
	-@erase "$(INTDIR)\texsim.obj"
	-@erase "$(INTDIR)\textureswap.obj"
	-@erase "$(INTDIR)\texutil.obj"
	-@erase "$(INTDIR)\tfans.obj"
	-@erase "$(INTDIR)\transattrib.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\tris.obj"
	-@erase "$(INTDIR)\tstrip.obj"
	-@erase "$(INTDIR)\txdl1.obj"
	-@erase "$(INTDIR)\txdl1_2.obj"
	-@erase "$(INTDIR)\txdl1_3.obj"
	-@erase "$(INTDIR)\txdl2.obj"
	-@erase "$(INTDIR)\txdl2_2.obj"
	-@erase "$(INTDIR)\txdl2_3.obj"
	-@erase "$(INTDIR)\txdl3.obj"
	-@erase "$(INTDIR)\txdl3_2.obj"
	-@erase "$(INTDIR)\txdl3_3.obj"
	-@erase "$(INTDIR)\txdl4.obj"
	-@erase "$(INTDIR)\txdl4_2.obj"
	-@erase "$(INTDIR)\txdl4_3.obj"
	-@erase "$(INTDIR)\txdlim1_2.obj"
	-@erase "$(INTDIR)\txdlim2.obj"
	-@erase "$(INTDIR)\txdlim2_2.obj"
	-@erase "$(INTDIR)\txdlim2_3.obj"
	-@erase "$(INTDIR)\txdlim3_2.obj"
	-@erase "$(INTDIR)\txdlim4.obj"
	-@erase "$(INTDIR)\txdlim4_2.obj"
	-@erase "$(INTDIR)\txdlim4_3.obj"
	-@erase "$(INTDIR)\txim1.obj"
	-@erase "$(INTDIR)\txim2.obj"
	-@erase "$(INTDIR)\txim3.obj"
	-@erase "$(INTDIR)\txim4.obj"
	-@erase "$(INTDIR)\txmodblend.obj"
	-@erase "$(INTDIR)\txmoddecal.obj"
	-@erase "$(INTDIR)\unsign11.obj"
	-@erase "$(INTDIR)\unsign12.obj"
	-@erase "$(INTDIR)\unsign13.obj"
	-@erase "$(INTDIR)\unsign14.obj"
	-@erase "$(INTDIR)\unsign21.obj"
	-@erase "$(INTDIR)\unsign22.obj"
	-@erase "$(INTDIR)\unsign23.obj"
	-@erase "$(INTDIR)\unsign24.obj"
	-@erase "$(INTDIR)\unsign31.obj"
	-@erase "$(INTDIR)\unsign32.obj"
	-@erase "$(INTDIR)\unsign33.obj"
	-@erase "$(INTDIR)\unsign34.obj"
	-@erase "$(INTDIR)\unsign41.obj"
	-@erase "$(INTDIR)\unsign42.obj"
	-@erase "$(INTDIR)\unsign43.obj"
	-@erase "$(INTDIR)\unsign44.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vertarray.obj"
	-@erase "$(INTDIR)\vertexes.obj"
	-@erase "$(INTDIR)\viewport.obj"
	-@erase "$(INTDIR)\wind.obj"
	-@erase "$(INTDIR)\xformsimulate.obj"
	-@erase "$(OUTDIR)\ogtst.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /G5 /Gz /W3 /GX /O2 /I "env" /I "inc" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "__STDC__" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /G5 /Gz /ML /W3 /GX /O2 /I "env" /I "inc" /D "NDEBUG" /D\
 "WIN32" /D "_CONSOLE" /D "__STDC__" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ogtst.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 glu.lib opengl.lib kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=glu.lib opengl.lib kernel32.lib user32.lib gdi32.lib advapi32.lib\
 /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)/ogtst.pdb"\
 /machine:I386 /out:"$(OUTDIR)/ogtst.exe" 
LINK32_OBJS= \
	"$(INTDIR)\aafatlines.obj" \
	"$(INTDIR)\abgr.obj" \
	"$(INTDIR)\accbuf.obj" \
	"$(INTDIR)\alloc.obj" \
	"$(INTDIR)\alphafunc.obj" \
	"$(INTDIR)\arbcliplin.obj" \
	"$(INTDIR)\arbcliptri.obj" \
	"$(INTDIR)\array.obj" \
	"$(INTDIR)\bgnend.obj" \
	"$(INTDIR)\bitmaps.obj" \
	"$(INTDIR)\blendfunc.obj" \
	"$(INTDIR)\calllist.obj" \
	"$(INTDIR)\cancliplin.obj" \
	"$(INTDIR)\cancliptri.obj" \
	"$(INTDIR)\check.obj" \
	"$(INTDIR)\clamp.obj" \
	"$(INTDIR)\clear.obj" \
	"$(INTDIR)\clearaccum.obj" \
	"$(INTDIR)\clearall.obj" \
	"$(INTDIR)\cleardepth.obj" \
	"$(INTDIR)\clearstencil.obj" \
	"$(INTDIR)\clip_edge_flags.obj" \
	"$(INTDIR)\clipcolorlin.obj" \
	"$(INTDIR)\clipcolorpnt.obj" \
	"$(INTDIR)\clipcolortri.obj" \
	"$(INTDIR)\cliptextlin.obj" \
	"$(INTDIR)\cliptexttri.obj" \
	"$(INTDIR)\cmpltcolorlin.obj" \
	"$(INTDIR)\cmpltcolorpnt.obj" \
	"$(INTDIR)\cmpltcolortri.obj" \
	"$(INTDIR)\cmplttextlin.obj" \
	"$(INTDIR)\cmplttexttri.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\complement_one_plane.obj" \
	"$(INTDIR)\copytex.obj" \
	"$(INTDIR)\ctxsw.obj" \
	"$(INTDIR)\cullface.obj" \
	"$(INTDIR)\cyls.obj" \
	"$(INTDIR)\defaultState.obj" \
	"$(INTDIR)\depthbuf.obj" \
	"$(INTDIR)\depthfunc.obj" \
	"$(INTDIR)\disks.obj" \
	"$(INTDIR)\doodle.obj" \
	"$(INTDIR)\edgeflag.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\eval.obj" \
	"$(INTDIR)\evalattrib.obj" \
	"$(INTDIR)\evaluator.obj" \
	"$(INTDIR)\evalvertexmix.obj" \
	"$(INTDIR)\extvertarray.obj" \
	"$(INTDIR)\fog.obj" \
	"$(INTDIR)\frustum.obj" \
	"$(INTDIR)\getenv.obj" \
	"$(INTDIR)\getmatrix.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\getstatecheck.obj" \
	"$(INTDIR)\getviewport.obj" \
	"$(INTDIR)\glPixelTransfer.obj" \
	"$(INTDIR)\glRasterPos.obj" \
	"$(INTDIR)\gltst.obj" \
	"$(INTDIR)\imbuildutil.obj" \
	"$(INTDIR)\imgutil.obj" \
	"$(INTDIR)\imutil.obj" \
	"$(INTDIR)\index.obj" \
	"$(INTDIR)\info.obj" \
	"$(INTDIR)\inout.obj" \
	"$(INTDIR)\internalformat.obj" \
	"$(INTDIR)\light.obj" \
	"$(INTDIR)\lightsimulate.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\linestrip.obj" \
	"$(INTDIR)\listcov.obj" \
	"$(INTDIR)\logicop.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\matrixflow.obj" \
	"$(INTDIR)\nest.obj" \
	"$(INTDIR)\nodraw.obj" \
	"$(INTDIR)\normals.obj" \
	"$(INTDIR)\ortho.obj" \
	"$(INTDIR)\packedpix.obj" \
	"$(INTDIR)\pdisks.obj" \
	"$(INTDIR)\pgonoffset.obj" \
	"$(INTDIR)\pixel.obj" \
	"$(INTDIR)\pixelinv.obj" \
	"$(INTDIR)\pixutil.obj" \
	"$(INTDIR)\point.obj" \
	"$(INTDIR)\pointsize.obj" \
	"$(INTDIR)\polys.obj" \
	"$(INTDIR)\polystipple.obj" \
	"$(INTDIR)\popmatrix.obj" \
	"$(INTDIR)\pushattrib.obj" \
	"$(INTDIR)\qstrip.obj" \
	"$(INTDIR)\quads.obj" \
	"$(INTDIR)\rand.obj" \
	"$(INTDIR)\ReadPixelsRGBA.obj" \
	"$(INTDIR)\readwrite.obj" \
	"$(INTDIR)\rect.obj" \
	"$(INTDIR)\rgblogicop.obj" \
	"$(INTDIR)\rotate.obj" \
	"$(INTDIR)\scale.obj" \
	"$(INTDIR)\set.obj" \
	"$(INTDIR)\sgilib.obj" \
	"$(INTDIR)\shade.obj" \
	"$(INTDIR)\spec.obj" \
	"$(INTDIR)\spheres.obj" \
	"$(INTDIR)\stencil.obj" \
	"$(INTDIR)\stencilmask.obj" \
	"$(INTDIR)\stereobuffer.obj" \
	"$(INTDIR)\texcoords.obj" \
	"$(INTDIR)\texobj.obj" \
	"$(INTDIR)\texobject.obj" \
	"$(INTDIR)\texsim.obj" \
	"$(INTDIR)\textureswap.obj" \
	"$(INTDIR)\texutil.obj" \
	"$(INTDIR)\tfans.obj" \
	"$(INTDIR)\transattrib.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\tris.obj" \
	"$(INTDIR)\tstrip.obj" \
	"$(INTDIR)\txdl1.obj" \
	"$(INTDIR)\txdl1_2.obj" \
	"$(INTDIR)\txdl1_3.obj" \
	"$(INTDIR)\txdl2.obj" \
	"$(INTDIR)\txdl2_2.obj" \
	"$(INTDIR)\txdl2_3.obj" \
	"$(INTDIR)\txdl3.obj" \
	"$(INTDIR)\txdl3_2.obj" \
	"$(INTDIR)\txdl3_3.obj" \
	"$(INTDIR)\txdl4.obj" \
	"$(INTDIR)\txdl4_2.obj" \
	"$(INTDIR)\txdl4_3.obj" \
	"$(INTDIR)\txdlim1_2.obj" \
	"$(INTDIR)\txdlim2.obj" \
	"$(INTDIR)\txdlim2_2.obj" \
	"$(INTDIR)\txdlim2_3.obj" \
	"$(INTDIR)\txdlim3_2.obj" \
	"$(INTDIR)\txdlim4.obj" \
	"$(INTDIR)\txdlim4_2.obj" \
	"$(INTDIR)\txdlim4_3.obj" \
	"$(INTDIR)\txim1.obj" \
	"$(INTDIR)\txim2.obj" \
	"$(INTDIR)\txim3.obj" \
	"$(INTDIR)\txim4.obj" \
	"$(INTDIR)\txmodblend.obj" \
	"$(INTDIR)\txmoddecal.obj" \
	"$(INTDIR)\unsign11.obj" \
	"$(INTDIR)\unsign12.obj" \
	"$(INTDIR)\unsign13.obj" \
	"$(INTDIR)\unsign14.obj" \
	"$(INTDIR)\unsign21.obj" \
	"$(INTDIR)\unsign22.obj" \
	"$(INTDIR)\unsign23.obj" \
	"$(INTDIR)\unsign24.obj" \
	"$(INTDIR)\unsign31.obj" \
	"$(INTDIR)\unsign32.obj" \
	"$(INTDIR)\unsign33.obj" \
	"$(INTDIR)\unsign34.obj" \
	"$(INTDIR)\unsign41.obj" \
	"$(INTDIR)\unsign42.obj" \
	"$(INTDIR)\unsign43.obj" \
	"$(INTDIR)\unsign44.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\vertarray.obj" \
	"$(INTDIR)\vertexes.obj" \
	"$(INTDIR)\viewport.obj" \
	"$(INTDIR)\wind.obj" \
	"$(INTDIR)\xformsimulate.obj"

"$(OUTDIR)\ogtst.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ogtst___"
# PROP BASE Intermediate_Dir "ogtst___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ogtst___"
# PROP Intermediate_Dir "ogtst___"
# PROP Target_Dir ""
OUTDIR=.\ogtst___
INTDIR=.\ogtst___

ALL : ".\Debug\ogtst.exe"

CLEAN : 
	-@erase ".\Debug\aafatlines.obj"
	-@erase ".\Debug\abgr.obj"
	-@erase ".\Debug\accbuf.obj"
	-@erase ".\Debug\alloc.obj"
	-@erase ".\Debug\alphafunc.obj"
	-@erase ".\Debug\arbcliplin.obj"
	-@erase ".\Debug\arbcliptri.obj"
	-@erase ".\Debug\array.obj"
	-@erase ".\Debug\bgnend.obj"
	-@erase ".\Debug\bitmaps.obj"
	-@erase ".\Debug\blendfunc.obj"
	-@erase ".\Debug\calllist.obj"
	-@erase ".\Debug\cancliplin.obj"
	-@erase ".\Debug\cancliptri.obj"
	-@erase ".\Debug\check.obj"
	-@erase ".\Debug\clamp.obj"
	-@erase ".\Debug\clear.obj"
	-@erase ".\Debug\clearaccum.obj"
	-@erase ".\Debug\clearall.obj"
	-@erase ".\Debug\cleardepth.obj"
	-@erase ".\Debug\clearstencil.obj"
	-@erase ".\Debug\clip_edge_flags.obj"
	-@erase ".\Debug\clipcolorlin.obj"
	-@erase ".\Debug\clipcolorpnt.obj"
	-@erase ".\Debug\clipcolortri.obj"
	-@erase ".\Debug\cliptextlin.obj"
	-@erase ".\Debug\cliptexttri.obj"
	-@erase ".\Debug\cmpltcolorlin.obj"
	-@erase ".\Debug\cmpltcolorpnt.obj"
	-@erase ".\Debug\cmpltcolortri.obj"
	-@erase ".\Debug\cmplttextlin.obj"
	-@erase ".\Debug\cmplttexttri.obj"
	-@erase ".\Debug\colors.obj"
	-@erase ".\Debug\complement_one_plane.obj"
	-@erase ".\Debug\copytex.obj"
	-@erase ".\Debug\ctxsw.obj"
	-@erase ".\Debug\cullface.obj"
	-@erase ".\Debug\cyls.obj"
	-@erase ".\Debug\defaultState.obj"
	-@erase ".\Debug\depthbuf.obj"
	-@erase ".\Debug\depthfunc.obj"
	-@erase ".\Debug\disks.obj"
	-@erase ".\Debug\doodle.obj"
	-@erase ".\Debug\edgeflag.obj"
	-@erase ".\Debug\errors.obj"
	-@erase ".\Debug\eval.obj"
	-@erase ".\Debug\evalattrib.obj"
	-@erase ".\Debug\evaluator.obj"
	-@erase ".\Debug\evalvertexmix.obj"
	-@erase ".\Debug\extvertarray.obj"
	-@erase ".\Debug\fog.obj"
	-@erase ".\Debug\frustum.obj"
	-@erase ".\Debug\getenv.obj"
	-@erase ".\Debug\getmatrix.obj"
	-@erase ".\Debug\getopt.obj"
	-@erase ".\Debug\getstatecheck.obj"
	-@erase ".\Debug\getviewport.obj"
	-@erase ".\Debug\glPixelTransfer.obj"
	-@erase ".\Debug\glRasterPos.obj"
	-@erase ".\Debug\gltst.obj"
	-@erase ".\Debug\imbuildutil.obj"
	-@erase ".\Debug\imgutil.obj"
	-@erase ".\Debug\imutil.obj"
	-@erase ".\Debug\index.obj"
	-@erase ".\Debug\info.obj"
	-@erase ".\Debug\inout.obj"
	-@erase ".\Debug\internalformat.obj"
	-@erase ".\Debug\light.obj"
	-@erase ".\Debug\lightsimulate.obj"
	-@erase ".\Debug\line.obj"
	-@erase ".\Debug\linestrip.obj"
	-@erase ".\Debug\listcov.obj"
	-@erase ".\Debug\logicop.obj"
	-@erase ".\Debug\main.obj"
	-@erase ".\Debug\matrix.obj"
	-@erase ".\Debug\matrixflow.obj"
	-@erase ".\Debug\nest.obj"
	-@erase ".\Debug\nodraw.obj"
	-@erase ".\Debug\normals.obj"
	-@erase ".\Debug\ogtst.exe"
	-@erase ".\Debug\ogtst.pdb"
	-@erase ".\Debug\ortho.obj"
	-@erase ".\Debug\packedpix.obj"
	-@erase ".\Debug\pdisks.obj"
	-@erase ".\Debug\pgonoffset.obj"
	-@erase ".\Debug\pixel.obj"
	-@erase ".\Debug\pixelinv.obj"
	-@erase ".\Debug\pixutil.obj"
	-@erase ".\Debug\point.obj"
	-@erase ".\Debug\pointsize.obj"
	-@erase ".\Debug\polys.obj"
	-@erase ".\Debug\polystipple.obj"
	-@erase ".\Debug\popmatrix.obj"
	-@erase ".\Debug\pushattrib.obj"
	-@erase ".\Debug\qstrip.obj"
	-@erase ".\Debug\quads.obj"
	-@erase ".\Debug\rand.obj"
	-@erase ".\Debug\ReadPixelsRGBA.obj"
	-@erase ".\Debug\readwrite.obj"
	-@erase ".\Debug\rect.obj"
	-@erase ".\Debug\rgblogicop.obj"
	-@erase ".\Debug\rotate.obj"
	-@erase ".\Debug\scale.obj"
	-@erase ".\Debug\set.obj"
	-@erase ".\Debug\sgilib.obj"
	-@erase ".\Debug\shade.obj"
	-@erase ".\Debug\spec.obj"
	-@erase ".\Debug\spheres.obj"
	-@erase ".\Debug\stencil.obj"
	-@erase ".\Debug\stencilmask.obj"
	-@erase ".\Debug\stereobuffer.obj"
	-@erase ".\Debug\texcoords.obj"
	-@erase ".\Debug\texobj.obj"
	-@erase ".\Debug\texobject.obj"
	-@erase ".\Debug\texsim.obj"
	-@erase ".\Debug\textureswap.obj"
	-@erase ".\Debug\texutil.obj"
	-@erase ".\Debug\tfans.obj"
	-@erase ".\Debug\transattrib.obj"
	-@erase ".\Debug\translate.obj"
	-@erase ".\Debug\tris.obj"
	-@erase ".\Debug\tstrip.obj"
	-@erase ".\Debug\txdl1.obj"
	-@erase ".\Debug\txdl1_2.obj"
	-@erase ".\Debug\txdl1_3.obj"
	-@erase ".\Debug\txdl2.obj"
	-@erase ".\Debug\txdl2_2.obj"
	-@erase ".\Debug\txdl2_3.obj"
	-@erase ".\Debug\txdl3.obj"
	-@erase ".\Debug\txdl3_2.obj"
	-@erase ".\Debug\txdl3_3.obj"
	-@erase ".\Debug\txdl4.obj"
	-@erase ".\Debug\txdl4_2.obj"
	-@erase ".\Debug\txdl4_3.obj"
	-@erase ".\Debug\txdlim1_2.obj"
	-@erase ".\Debug\txdlim2.obj"
	-@erase ".\Debug\txdlim2_2.obj"
	-@erase ".\Debug\txdlim2_3.obj"
	-@erase ".\Debug\txdlim3_2.obj"
	-@erase ".\Debug\txdlim4.obj"
	-@erase ".\Debug\txdlim4_2.obj"
	-@erase ".\Debug\txdlim4_3.obj"
	-@erase ".\Debug\txim1.obj"
	-@erase ".\Debug\txim2.obj"
	-@erase ".\Debug\txim3.obj"
	-@erase ".\Debug\txim4.obj"
	-@erase ".\Debug\txmodblend.obj"
	-@erase ".\Debug\txmoddecal.obj"
	-@erase ".\Debug\unsign11.obj"
	-@erase ".\Debug\unsign12.obj"
	-@erase ".\Debug\unsign13.obj"
	-@erase ".\Debug\unsign14.obj"
	-@erase ".\Debug\unsign21.obj"
	-@erase ".\Debug\unsign22.obj"
	-@erase ".\Debug\unsign23.obj"
	-@erase ".\Debug\unsign24.obj"
	-@erase ".\Debug\unsign31.obj"
	-@erase ".\Debug\unsign32.obj"
	-@erase ".\Debug\unsign33.obj"
	-@erase ".\Debug\unsign34.obj"
	-@erase ".\Debug\unsign41.obj"
	-@erase ".\Debug\unsign42.obj"
	-@erase ".\Debug\unsign43.obj"
	-@erase ".\Debug\unsign44.obj"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vertarray.obj"
	-@erase ".\Debug\vertexes.obj"
	-@erase ".\Debug\viewport.obj"
	-@erase ".\Debug\wind.obj"
	-@erase ".\Debug\xformsimulate.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /G5 /Gz /W3 /Gm /GX /Zi /Od /I "env" /I "inc" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "__STDC__" /Fo"Debug/" /Fd"Debug/" /c
CPP_PROJ=/nologo /G5 /Gz /MLd /W3 /Gm /GX /Zi /Od /I "env" /I "inc" /D "_DEBUG"\
 /D "WIN32" /D "_CONSOLE" /D "__STDC__" /Fo"Debug/" /Fd"Debug/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/ogtst.bsc"
BSC32_FLAGS=/nologo /o"Debug/ogtst.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 glu.lib opengl.lib kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"Debug/ogtst.pdb" /debug /machine:I386 /out:"Debug/ogtst.exe"
LINK32_FLAGS=glu.lib opengl.lib kernel32.lib user32.lib gdi32.lib advapi32.lib\
 /nologo /subsystem:console /incremental:no /pdb:"Debug/ogtst.pdb" /debug\
 /machine:I386 /out:"Debug/ogtst.exe" 
LINK32_OBJS= \
	".\Debug\aafatlines.obj" \
	".\Debug\abgr.obj" \
	".\Debug\accbuf.obj" \
	".\Debug\alloc.obj" \
	".\Debug\alphafunc.obj" \
	".\Debug\arbcliplin.obj" \
	".\Debug\arbcliptri.obj" \
	".\Debug\array.obj" \
	".\Debug\bgnend.obj" \
	".\Debug\bitmaps.obj" \
	".\Debug\blendfunc.obj" \
	".\Debug\calllist.obj" \
	".\Debug\cancliplin.obj" \
	".\Debug\cancliptri.obj" \
	".\Debug\check.obj" \
	".\Debug\clamp.obj" \
	".\Debug\clear.obj" \
	".\Debug\clearaccum.obj" \
	".\Debug\clearall.obj" \
	".\Debug\cleardepth.obj" \
	".\Debug\clearstencil.obj" \
	".\Debug\clip_edge_flags.obj" \
	".\Debug\clipcolorlin.obj" \
	".\Debug\clipcolorpnt.obj" \
	".\Debug\clipcolortri.obj" \
	".\Debug\cliptextlin.obj" \
	".\Debug\cliptexttri.obj" \
	".\Debug\cmpltcolorlin.obj" \
	".\Debug\cmpltcolorpnt.obj" \
	".\Debug\cmpltcolortri.obj" \
	".\Debug\cmplttextlin.obj" \
	".\Debug\cmplttexttri.obj" \
	".\Debug\colors.obj" \
	".\Debug\complement_one_plane.obj" \
	".\Debug\copytex.obj" \
	".\Debug\ctxsw.obj" \
	".\Debug\cullface.obj" \
	".\Debug\cyls.obj" \
	".\Debug\defaultState.obj" \
	".\Debug\depthbuf.obj" \
	".\Debug\depthfunc.obj" \
	".\Debug\disks.obj" \
	".\Debug\doodle.obj" \
	".\Debug\edgeflag.obj" \
	".\Debug\errors.obj" \
	".\Debug\eval.obj" \
	".\Debug\evalattrib.obj" \
	".\Debug\evaluator.obj" \
	".\Debug\evalvertexmix.obj" \
	".\Debug\extvertarray.obj" \
	".\Debug\fog.obj" \
	".\Debug\frustum.obj" \
	".\Debug\getenv.obj" \
	".\Debug\getmatrix.obj" \
	".\Debug\getopt.obj" \
	".\Debug\getstatecheck.obj" \
	".\Debug\getviewport.obj" \
	".\Debug\glPixelTransfer.obj" \
	".\Debug\glRasterPos.obj" \
	".\Debug\gltst.obj" \
	".\Debug\imbuildutil.obj" \
	".\Debug\imgutil.obj" \
	".\Debug\imutil.obj" \
	".\Debug\index.obj" \
	".\Debug\info.obj" \
	".\Debug\inout.obj" \
	".\Debug\internalformat.obj" \
	".\Debug\light.obj" \
	".\Debug\lightsimulate.obj" \
	".\Debug\line.obj" \
	".\Debug\linestrip.obj" \
	".\Debug\listcov.obj" \
	".\Debug\logicop.obj" \
	".\Debug\main.obj" \
	".\Debug\matrix.obj" \
	".\Debug\matrixflow.obj" \
	".\Debug\nest.obj" \
	".\Debug\nodraw.obj" \
	".\Debug\normals.obj" \
	".\Debug\ortho.obj" \
	".\Debug\packedpix.obj" \
	".\Debug\pdisks.obj" \
	".\Debug\pgonoffset.obj" \
	".\Debug\pixel.obj" \
	".\Debug\pixelinv.obj" \
	".\Debug\pixutil.obj" \
	".\Debug\point.obj" \
	".\Debug\pointsize.obj" \
	".\Debug\polys.obj" \
	".\Debug\polystipple.obj" \
	".\Debug\popmatrix.obj" \
	".\Debug\pushattrib.obj" \
	".\Debug\qstrip.obj" \
	".\Debug\quads.obj" \
	".\Debug\rand.obj" \
	".\Debug\ReadPixelsRGBA.obj" \
	".\Debug\readwrite.obj" \
	".\Debug\rect.obj" \
	".\Debug\rgblogicop.obj" \
	".\Debug\rotate.obj" \
	".\Debug\scale.obj" \
	".\Debug\set.obj" \
	".\Debug\sgilib.obj" \
	".\Debug\shade.obj" \
	".\Debug\spec.obj" \
	".\Debug\spheres.obj" \
	".\Debug\stencil.obj" \
	".\Debug\stencilmask.obj" \
	".\Debug\stereobuffer.obj" \
	".\Debug\texcoords.obj" \
	".\Debug\texobj.obj" \
	".\Debug\texobject.obj" \
	".\Debug\texsim.obj" \
	".\Debug\textureswap.obj" \
	".\Debug\texutil.obj" \
	".\Debug\tfans.obj" \
	".\Debug\transattrib.obj" \
	".\Debug\translate.obj" \
	".\Debug\tris.obj" \
	".\Debug\tstrip.obj" \
	".\Debug\txdl1.obj" \
	".\Debug\txdl1_2.obj" \
	".\Debug\txdl1_3.obj" \
	".\Debug\txdl2.obj" \
	".\Debug\txdl2_2.obj" \
	".\Debug\txdl2_3.obj" \
	".\Debug\txdl3.obj" \
	".\Debug\txdl3_2.obj" \
	".\Debug\txdl3_3.obj" \
	".\Debug\txdl4.obj" \
	".\Debug\txdl4_2.obj" \
	".\Debug\txdl4_3.obj" \
	".\Debug\txdlim1_2.obj" \
	".\Debug\txdlim2.obj" \
	".\Debug\txdlim2_2.obj" \
	".\Debug\txdlim2_3.obj" \
	".\Debug\txdlim3_2.obj" \
	".\Debug\txdlim4.obj" \
	".\Debug\txdlim4_2.obj" \
	".\Debug\txdlim4_3.obj" \
	".\Debug\txim1.obj" \
	".\Debug\txim2.obj" \
	".\Debug\txim3.obj" \
	".\Debug\txim4.obj" \
	".\Debug\txmodblend.obj" \
	".\Debug\txmoddecal.obj" \
	".\Debug\unsign11.obj" \
	".\Debug\unsign12.obj" \
	".\Debug\unsign13.obj" \
	".\Debug\unsign14.obj" \
	".\Debug\unsign21.obj" \
	".\Debug\unsign22.obj" \
	".\Debug\unsign23.obj" \
	".\Debug\unsign24.obj" \
	".\Debug\unsign31.obj" \
	".\Debug\unsign32.obj" \
	".\Debug\unsign33.obj" \
	".\Debug\unsign34.obj" \
	".\Debug\unsign41.obj" \
	".\Debug\unsign42.obj" \
	".\Debug\unsign43.obj" \
	".\Debug\unsign44.obj" \
	".\Debug\util.obj" \
	".\Debug\vertarray.obj" \
	".\Debug\vertexes.obj" \
	".\Debug\viewport.obj" \
	".\Debug\wind.obj" \
	".\Debug\xformsimulate.obj"

".\Debug\ogtst.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "ogtst - Win32 Release"
# Name "ogtst - Win32 Debug"

!IF  "$(CFG)" == "ogtst - Win32 Release"

!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\env\wind.c
DEP_CPP_WIND_=\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\wind.obj" : $(SOURCE) $(DEP_CPP_WIND_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\wind.obj" : $(SOURCE) $(DEP_CPP_WIND_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\getopt.c
DEP_CPP_GETOP=\
	".\env\getopt.h"\
	
NODEP_CPP_GETOP=\
	".\env\config.h"\
	".\env\gstddef.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\getopt.obj" : $(SOURCE) $(DEP_CPP_GETOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\getopt.obj" : $(SOURCE) $(DEP_CPP_GETOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\getstatecheck.c
DEP_CPP_GETST=\
	".\env\getstatecheck.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\getstatecheck.obj" : $(SOURCE) $(DEP_CPP_GETST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\getstatecheck.obj" : $(SOURCE) $(DEP_CPP_GETST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\imutil.c
DEP_CPP_IMUTI=\
	".\env\env.h"\
	".\env\sgi.h"\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\imutil.obj" : $(SOURCE) $(DEP_CPP_IMUTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\imutil.obj" : $(SOURCE) $(DEP_CPP_IMUTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\info.c
DEP_CPP_INFO_=\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\info.obj" : $(SOURCE) $(DEP_CPP_INFO_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\info.obj" : $(SOURCE) $(DEP_CPP_INFO_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\inout.c
DEP_CPP_INOUT=\
	".\env\env.h"\
	".\env\getopt.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\inout.obj" : $(SOURCE) $(DEP_CPP_INOUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\inout.obj" : $(SOURCE) $(DEP_CPP_INOUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\main.c
DEP_CPP_MAIN_=\
	".\env\architectures.h"\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	".\inc\strings.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\sgilib.c
DEP_CPP_SGILI=\
	".\env\sgi.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\sgilib.obj" : $(SOURCE) $(DEP_CPP_SGILI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\sgilib.obj" : $(SOURCE) $(DEP_CPP_SGILI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\spec.c
DEP_CPP_SPEC_=\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\spec.obj" : $(SOURCE) $(DEP_CPP_SPEC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\spec.obj" : $(SOURCE) $(DEP_CPP_SPEC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\env\ctxsw.c
DEP_CPP_CTXSW=\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\ctxsw.obj" : $(SOURCE) $(DEP_CPP_CTXSW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\ctxsw.obj" : $(SOURCE) $(DEP_CPP_CTXSW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\complement_one_plane.c"
DEP_CPP_COMPL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\complement_one_plane.obj" : $(SOURCE) $(DEP_CPP_COMPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\complement_one_plane.obj" : $(SOURCE) $(DEP_CPP_COMPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\arbcliptri.c"
DEP_CPP_ARBCL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\arbcliptri.obj" : $(SOURCE) $(DEP_CPP_ARBCL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\arbcliptri.obj" : $(SOURCE) $(DEP_CPP_ARBCL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cancliplin.c"
DEP_CPP_CANCL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cancliplin.obj" : $(SOURCE) $(DEP_CPP_CANCL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cancliplin.obj" : $(SOURCE) $(DEP_CPP_CANCL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cancliptri.c"
DEP_CPP_CANCLI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cancliptri.obj" : $(SOURCE) $(DEP_CPP_CANCLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cancliptri.obj" : $(SOURCE) $(DEP_CPP_CANCLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\clip_edge_flags.c"
DEP_CPP_CLIP_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clip_edge_flags.obj" : $(SOURCE) $(DEP_CPP_CLIP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clip_edge_flags.obj" : $(SOURCE) $(DEP_CPP_CLIP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\clipcolorlin.c"
DEP_CPP_CLIPC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clipcolorlin.obj" : $(SOURCE) $(DEP_CPP_CLIPC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clipcolorlin.obj" : $(SOURCE) $(DEP_CPP_CLIPC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\clipcolorpnt.c"
DEP_CPP_CLIPCO=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clipcolorpnt.obj" : $(SOURCE) $(DEP_CPP_CLIPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clipcolorpnt.obj" : $(SOURCE) $(DEP_CPP_CLIPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\clipcolortri.c"
DEP_CPP_CLIPCOL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clipcolortri.obj" : $(SOURCE) $(DEP_CPP_CLIPCOL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clipcolortri.obj" : $(SOURCE) $(DEP_CPP_CLIPCOL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cliptextlin.c"
DEP_CPP_CLIPT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cliptextlin.obj" : $(SOURCE) $(DEP_CPP_CLIPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cliptextlin.obj" : $(SOURCE) $(DEP_CPP_CLIPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cliptexttri.c"
DEP_CPP_CLIPTE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cliptexttri.obj" : $(SOURCE) $(DEP_CPP_CLIPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cliptexttri.obj" : $(SOURCE) $(DEP_CPP_CLIPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cmpltcolorlin.c"
DEP_CPP_CMPLT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cmpltcolorlin.obj" : $(SOURCE) $(DEP_CPP_CMPLT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cmpltcolorlin.obj" : $(SOURCE) $(DEP_CPP_CMPLT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cmpltcolorpnt.c"
DEP_CPP_CMPLTC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cmpltcolorpnt.obj" : $(SOURCE) $(DEP_CPP_CMPLTC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cmpltcolorpnt.obj" : $(SOURCE) $(DEP_CPP_CMPLTC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cmpltcolortri.c"
DEP_CPP_CMPLTCO=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cmpltcolortri.obj" : $(SOURCE) $(DEP_CPP_CMPLTCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cmpltcolortri.obj" : $(SOURCE) $(DEP_CPP_CMPLTCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cmplttextlin.c"
DEP_CPP_CMPLTT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cmplttextlin.obj" : $(SOURCE) $(DEP_CPP_CMPLTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cmplttextlin.obj" : $(SOURCE) $(DEP_CPP_CMPLTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\cmplttexttri.c"
DEP_CPP_CMPLTTE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cmplttexttri.obj" : $(SOURCE) $(DEP_CPP_CMPLTTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cmplttexttri.obj" : $(SOURCE) $(DEP_CPP_CMPLTTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\clip\arbcliplin.c"
DEP_CPP_ARBCLI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\arbcliplin.obj" : $(SOURCE) $(DEP_CPP_ARBCLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\arbcliplin.obj" : $(SOURCE) $(DEP_CPP_ARBCLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=".\func\clip-cull\facecheck\cullface.c"
DEP_CPP_CULLF=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cullface.obj" : $(SOURCE) $(DEP_CPP_CULLF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cullface.obj" : $(SOURCE) $(DEP_CPP_CULLF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\coords\color\colors.c
DEP_CPP_COLOR=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\colors.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\colors.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\coords\index\index.c
DEP_CPP_INDEX=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\index.obj" : $(SOURCE) $(DEP_CPP_INDEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\index.obj" : $(SOURCE) $(DEP_CPP_INDEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\coords\normals\normals.c
DEP_CPP_NORMA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\normals.obj" : $(SOURCE) $(DEP_CPP_NORMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\normals.obj" : $(SOURCE) $(DEP_CPP_NORMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\coords\texcoord\texcoords.c
DEP_CPP_TEXCO=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\texcoords.obj" : $(SOURCE) $(DEP_CPP_TEXCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\texcoords.obj" : $(SOURCE) $(DEP_CPP_TEXCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\coords\vertex\vertexes.c
DEP_CPP_VERTE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\vertexes.obj" : $(SOURCE) $(DEP_CPP_VERTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\vertexes.obj" : $(SOURCE) $(DEP_CPP_VERTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\corner\errors.c
DEP_CPP_ERROR=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\errors.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\corner\clamp.c
DEP_CPP_CLAMP=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clamp.obj" : $(SOURCE) $(DEP_CPP_CLAMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clamp.obj" : $(SOURCE) $(DEP_CPP_CLAMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\corner\bgnend.c
DEP_CPP_BGNEN=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\bgnend.obj" : $(SOURCE) $(DEP_CPP_BGNEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\bgnend.obj" : $(SOURCE) $(DEP_CPP_BGNEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\enable\edgeflag.c
DEP_CPP_EDGEF=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\edgeflag.obj" : $(SOURCE) $(DEP_CPP_EDGEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\edgeflag.obj" : $(SOURCE) $(DEP_CPP_EDGEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\eval\evalvertexmix.c
DEP_CPP_EVALV=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\evalvertexmix.obj" : $(SOURCE) $(DEP_CPP_EVALV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\evalvertexmix.obj" : $(SOURCE) $(DEP_CPP_EVALV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\eval\evalattrib.c
DEP_CPP_EVALA=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\evalattrib.obj" : $(SOURCE) $(DEP_CPP_EVALA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\evalattrib.obj" : $(SOURCE) $(DEP_CPP_EVALA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\eval\eval.c
DEP_CPP_EVAL_=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\eval.obj" : $(SOURCE) $(DEP_CPP_EVAL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\eval.obj" : $(SOURCE) $(DEP_CPP_EVAL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\feedback\nodraw.c
DEP_CPP_NODRA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\nodraw.obj" : $(SOURCE) $(DEP_CPP_NODRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\nodraw.obj" : $(SOURCE) $(DEP_CPP_NODRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fog\fog.c
DEP_CPP_FOG_C=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\fog.obj" : $(SOURCE) $(DEP_CPP_FOG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\fog.obj" : $(SOURCE) $(DEP_CPP_FOG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\stencilmask.c
DEP_CPP_STENC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\stencilmask.obj" : $(SOURCE) $(DEP_CPP_STENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\stencilmask.obj" : $(SOURCE) $(DEP_CPP_STENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\blendfunc.c
DEP_CPP_BLEND=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\blendfunc.obj" : $(SOURCE) $(DEP_CPP_BLEND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\blendfunc.obj" : $(SOURCE) $(DEP_CPP_BLEND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\depthbuf.c
DEP_CPP_DEPTH=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\depthbuf.obj" : $(SOURCE) $(DEP_CPP_DEPTH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\depthbuf.obj" : $(SOURCE) $(DEP_CPP_DEPTH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\depthfunc.c
DEP_CPP_DEPTHF=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\depthfunc.obj" : $(SOURCE) $(DEP_CPP_DEPTHF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\depthfunc.obj" : $(SOURCE) $(DEP_CPP_DEPTHF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\logicop.c
DEP_CPP_LOGIC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\logicop.obj" : $(SOURCE) $(DEP_CPP_LOGIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\logicop.obj" : $(SOURCE) $(DEP_CPP_LOGIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\rgblogicop.c
DEP_CPP_RGBLO=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\rgblogicop.obj" : $(SOURCE) $(DEP_CPP_RGBLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\rgblogicop.obj" : $(SOURCE) $(DEP_CPP_RGBLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\stencil.c
DEP_CPP_STENCI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\stencil.obj" : $(SOURCE) $(DEP_CPP_STENCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\stencil.obj" : $(SOURCE) $(DEP_CPP_STENCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\fragop\alphafunc.c
DEP_CPP_ALPHA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\alphafunc.obj" : $(SOURCE) $(DEP_CPP_ALPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\alphafunc.obj" : $(SOURCE) $(DEP_CPP_ALPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\acbuf\accbuf.c
DEP_CPP_ACCBU=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\accbuf.obj" : $(SOURCE) $(DEP_CPP_ACCBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\accbuf.obj" : $(SOURCE) $(DEP_CPP_ACCBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\clear\clearstencil.c
DEP_CPP_CLEAR=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clearstencil.obj" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clearstencil.obj" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\clear\clearaccum.c
DEP_CPP_CLEARA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clearaccum.obj" : $(SOURCE) $(DEP_CPP_CLEARA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clearaccum.obj" : $(SOURCE) $(DEP_CPP_CLEARA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\clear\clearall.c
DEP_CPP_CLEARAL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clearall.obj" : $(SOURCE) $(DEP_CPP_CLEARAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clearall.obj" : $(SOURCE) $(DEP_CPP_CLEARAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\clear\cleardepth.c
DEP_CPP_CLEARD=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cleardepth.obj" : $(SOURCE) $(DEP_CPP_CLEARD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cleardepth.obj" : $(SOURCE) $(DEP_CPP_CLEARD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\clear\clear.c
DEP_CPP_CLEAR_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\clear.obj" : $(SOURCE) $(DEP_CPP_CLEAR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\clear.obj" : $(SOURCE) $(DEP_CPP_CLEAR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\framebuffer\drawbuffer\stereobuffer.c
DEP_CPP_STERE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\stereobuffer.obj" : $(SOURCE) $(DEP_CPP_STERE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\stereobuffer.obj" : $(SOURCE) $(DEP_CPP_STERE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lighting\lightsimulate.c
DEP_CPP_LIGHT=\
	".\func\lighting\light.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\lightsimulate.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\lightsimulate.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lighting\light.c
DEP_CPP_LIGHT_=\
	".\func\lighting\light.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\light.obj" : $(SOURCE) $(DEP_CPP_LIGHT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\light.obj" : $(SOURCE) $(DEP_CPP_LIGHT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lists\nest.c
DEP_CPP_NEST_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\nest.obj" : $(SOURCE) $(DEP_CPP_NEST_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\nest.obj" : $(SOURCE) $(DEP_CPP_NEST_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lists\calllist.c
DEP_CPP_CALLL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\calllist.obj" : $(SOURCE) $(DEP_CPP_CALLL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\calllist.obj" : $(SOURCE) $(DEP_CPP_CALLL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lists\listcov.c
DEP_CPP_LISTC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\listcov.obj" : $(SOURCE) $(DEP_CPP_LISTC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\listcov.obj" : $(SOURCE) $(DEP_CPP_LISTC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\lists\array.c
DEP_CPP_ARRAY=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\extensions\packedpix.c
DEP_CPP_PACKE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\imgutil.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\packedpix.obj" : $(SOURCE) $(DEP_CPP_PACKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\packedpix.obj" : $(SOURCE) $(DEP_CPP_PACKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\ReadPixelsRGBA.c
DEP_CPP_READP=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\ReadPixelsRGBA.obj" : $(SOURCE) $(DEP_CPP_READP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\ReadPixelsRGBA.obj" : $(SOURCE) $(DEP_CPP_READP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\glPixelTransfer.c
DEP_CPP_GLPIX=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\glPixelTransfer.obj" : $(SOURCE) $(DEP_CPP_GLPIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\glPixelTransfer.obj" : $(SOURCE) $(DEP_CPP_GLPIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\pixel.c
DEP_CPP_PIXEL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pixel.obj" : $(SOURCE) $(DEP_CPP_PIXEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pixel.obj" : $(SOURCE) $(DEP_CPP_PIXEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\pixelinv.c
DEP_CPP_PIXELI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pixelinv.obj" : $(SOURCE) $(DEP_CPP_PIXELI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pixelinv.obj" : $(SOURCE) $(DEP_CPP_PIXELI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\pixels\doodle.c
DEP_CPP_DOODL=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\doodle.obj" : $(SOURCE) $(DEP_CPP_DOODL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\doodle.obj" : $(SOURCE) $(DEP_CPP_DOODL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\bitmaps\bitmaps.c
DEP_CPP_BITMA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\bitmaps.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\bitmaps.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\extensions\extvertarray.c
DEP_CPP_EXTVE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\extvertarray.obj" : $(SOURCE) $(DEP_CPP_EXTVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\extvertarray.obj" : $(SOURCE) $(DEP_CPP_EXTVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\lines\linestrip.c
DEP_CPP_LINES=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\linestrip.obj" : $(SOURCE) $(DEP_CPP_LINES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\linestrip.obj" : $(SOURCE) $(DEP_CPP_LINES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\lines\line.c
DEP_CPP_LINE_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\meshes\tstrip.c
DEP_CPP_TSTRI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\tstrip.obj" : $(SOURCE) $(DEP_CPP_TSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\tstrip.obj" : $(SOURCE) $(DEP_CPP_TSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\meshes\tfans.c
DEP_CPP_TFANS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\tfans.obj" : $(SOURCE) $(DEP_CPP_TFANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\tfans.obj" : $(SOURCE) $(DEP_CPP_TFANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\meshes\qstrip.c
DEP_CPP_QSTRI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\qstrip.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\qstrip.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\objects\spheres.c
DEP_CPP_SPHER=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\spheres.obj" : $(SOURCE) $(DEP_CPP_SPHER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\spheres.obj" : $(SOURCE) $(DEP_CPP_SPHER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\objects\disks.c
DEP_CPP_DISKS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\disks.obj" : $(SOURCE) $(DEP_CPP_DISKS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\disks.obj" : $(SOURCE) $(DEP_CPP_DISKS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\objects\pdisks.c
DEP_CPP_PDISK=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pdisks.obj" : $(SOURCE) $(DEP_CPP_PDISK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pdisks.obj" : $(SOURCE) $(DEP_CPP_PDISK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\objects\cyls.c
DEP_CPP_CYLS_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\cyls.obj" : $(SOURCE) $(DEP_CPP_CYLS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\cyls.obj" : $(SOURCE) $(DEP_CPP_CYLS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\points\point.c
DEP_CPP_POINT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\point.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\point.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\polygons\polystipple.c
DEP_CPP_POLYS=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\GL/glx.h"\
	".\inc\imbuildutil.h"\
	".\inc\ogtst.h"\
	".\inc\strings.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\polystipple.obj" : $(SOURCE) $(DEP_CPP_POLYS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\polystipple.obj" : $(SOURCE) $(DEP_CPP_POLYS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\polygons\polys.c
DEP_CPP_POLYS_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\polys.obj" : $(SOURCE) $(DEP_CPP_POLYS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\polys.obj" : $(SOURCE) $(DEP_CPP_POLYS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\quads\rect.c
DEP_CPP_RECT_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\rect.obj" : $(SOURCE) $(DEP_CPP_RECT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\rect.obj" : $(SOURCE) $(DEP_CPP_RECT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\quads\quads.c
DEP_CPP_QUADS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\quads.obj" : $(SOURCE) $(DEP_CPP_QUADS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\quads.obj" : $(SOURCE) $(DEP_CPP_QUADS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\rasterpos\glRasterPos.c
DEP_CPP_GLRAS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\glRasterPos.obj" : $(SOURCE) $(DEP_CPP_GLRAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\glRasterPos.obj" : $(SOURCE) $(DEP_CPP_GLRAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\tris\tris.c
DEP_CPP_TRIS_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\tris.obj" : $(SOURCE) $(DEP_CPP_TRIS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\tris.obj" : $(SOURCE) $(DEP_CPP_TRIS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\prims\vertarray\vertarray.c
DEP_CPP_VERTA=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\vertarray.obj" : $(SOURCE) $(DEP_CPP_VERTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\vertarray.obj" : $(SOURCE) $(DEP_CPP_VERTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\rasterize\shade.c
DEP_CPP_SHADE=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\shade.obj" : $(SOURCE) $(DEP_CPP_SHADE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\shade.obj" : $(SOURCE) $(DEP_CPP_SHADE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\rasterize\pgonoffset.c
DEP_CPP_PGONO=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pgonoffset.obj" : $(SOURCE) $(DEP_CPP_PGONO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pgonoffset.obj" : $(SOURCE) $(DEP_CPP_PGONO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\rasterize\pointsize.c
DEP_CPP_POINTS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pointsize.obj" : $(SOURCE) $(DEP_CPP_POINTS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pointsize.obj" : $(SOURCE) $(DEP_CPP_POINTS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\rasterize\aafatlines.c
DEP_CPP_AAFAT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\aafatlines.obj" : $(SOURCE) $(DEP_CPP_AAFAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\aafatlines.obj" : $(SOURCE) $(DEP_CPP_AAFAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\state\attrib\transattrib.c
DEP_CPP_TRANS=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\transattrib.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\transattrib.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\state\attrib\pushattrib.c
DEP_CPP_PUSHA=\
	".\func\state\attrib\pushattrib.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	".\inc\strings.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pushattrib.obj" : $(SOURCE) $(DEP_CPP_PUSHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pushattrib.obj" : $(SOURCE) $(DEP_CPP_PUSHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\copy\copytex.c
DEP_CPP_COPYT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\copytex.obj" : $(SOURCE) $(DEP_CPP_COPYT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\copytex.obj" : $(SOURCE) $(DEP_CPP_COPYT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\env\txmoddecal.c
DEP_CPP_TXMOD=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txmoddecal.obj" : $(SOURCE) $(DEP_CPP_TXMOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txmoddecal.obj" : $(SOURCE) $(DEP_CPP_TXMOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\env\txmodblend.c
DEP_CPP_TXMODB=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txmodblend.obj" : $(SOURCE) $(DEP_CPP_TXMODB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txmodblend.obj" : $(SOURCE) $(DEP_CPP_TXMODB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\extensions\abgr.c
DEP_CPP_ABGR_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\abgr.obj" : $(SOURCE) $(DEP_CPP_ABGR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\abgr.obj" : $(SOURCE) $(DEP_CPP_ABGR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign44.c
DEP_CPP_UNSIG=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign44.obj" : $(SOURCE) $(DEP_CPP_UNSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign44.obj" : $(SOURCE) $(DEP_CPP_UNSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign11.c
DEP_CPP_UNSIGN=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign11.obj" : $(SOURCE) $(DEP_CPP_UNSIGN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign11.obj" : $(SOURCE) $(DEP_CPP_UNSIGN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign12.c
DEP_CPP_UNSIGN1=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign12.obj" : $(SOURCE) $(DEP_CPP_UNSIGN1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign12.obj" : $(SOURCE) $(DEP_CPP_UNSIGN1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign13.c
DEP_CPP_UNSIGN13=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign13.obj" : $(SOURCE) $(DEP_CPP_UNSIGN13) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign13.obj" : $(SOURCE) $(DEP_CPP_UNSIGN13) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign14.c
DEP_CPP_UNSIGN14=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign14.obj" : $(SOURCE) $(DEP_CPP_UNSIGN14) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign14.obj" : $(SOURCE) $(DEP_CPP_UNSIGN14) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign21.c
DEP_CPP_UNSIGN2=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign21.obj" : $(SOURCE) $(DEP_CPP_UNSIGN2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign21.obj" : $(SOURCE) $(DEP_CPP_UNSIGN2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign22.c
DEP_CPP_UNSIGN22=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign22.obj" : $(SOURCE) $(DEP_CPP_UNSIGN22) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign22.obj" : $(SOURCE) $(DEP_CPP_UNSIGN22) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign23.c
DEP_CPP_UNSIGN23=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign23.obj" : $(SOURCE) $(DEP_CPP_UNSIGN23) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign23.obj" : $(SOURCE) $(DEP_CPP_UNSIGN23) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign24.c
DEP_CPP_UNSIGN24=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign24.obj" : $(SOURCE) $(DEP_CPP_UNSIGN24) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign24.obj" : $(SOURCE) $(DEP_CPP_UNSIGN24) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign31.c
DEP_CPP_UNSIGN3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign31.obj" : $(SOURCE) $(DEP_CPP_UNSIGN3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign31.obj" : $(SOURCE) $(DEP_CPP_UNSIGN3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign32.c
DEP_CPP_UNSIGN32=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign32.obj" : $(SOURCE) $(DEP_CPP_UNSIGN32) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign32.obj" : $(SOURCE) $(DEP_CPP_UNSIGN32) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign33.c
DEP_CPP_UNSIGN33=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign33.obj" : $(SOURCE) $(DEP_CPP_UNSIGN33) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign33.obj" : $(SOURCE) $(DEP_CPP_UNSIGN33) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign34.c
DEP_CPP_UNSIGN34=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign34.obj" : $(SOURCE) $(DEP_CPP_UNSIGN34) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign34.obj" : $(SOURCE) $(DEP_CPP_UNSIGN34) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign41.c
DEP_CPP_UNSIGN4=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign41.obj" : $(SOURCE) $(DEP_CPP_UNSIGN4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign41.obj" : $(SOURCE) $(DEP_CPP_UNSIGN4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign42.c
DEP_CPP_UNSIGN42=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign42.obj" : $(SOURCE) $(DEP_CPP_UNSIGN42) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign42.obj" : $(SOURCE) $(DEP_CPP_UNSIGN42) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\unsign43.c
DEP_CPP_UNSIGN43=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\unsign43.obj" : $(SOURCE) $(DEP_CPP_UNSIGN43) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\unsign43.obj" : $(SOURCE) $(DEP_CPP_UNSIGN43) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\image2d\internalformat.c
DEP_CPP_INTER=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\internalformat.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\internalformat.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txim4.c
DEP_CPP_TXIM4=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txim4.obj" : $(SOURCE) $(DEP_CPP_TXIM4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txim4.obj" : $(SOURCE) $(DEP_CPP_TXIM4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl1_2.c
DEP_CPP_TXDL1=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl1_2.obj" : $(SOURCE) $(DEP_CPP_TXDL1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl1_2.obj" : $(SOURCE) $(DEP_CPP_TXDL1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl1_3.c
DEP_CPP_TXDL1_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl1_3.obj" : $(SOURCE) $(DEP_CPP_TXDL1_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl1_3.obj" : $(SOURCE) $(DEP_CPP_TXDL1_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl2.c
DEP_CPP_TXDL2=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl2.obj" : $(SOURCE) $(DEP_CPP_TXDL2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl2.obj" : $(SOURCE) $(DEP_CPP_TXDL2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl2_2.c
DEP_CPP_TXDL2_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl2_2.obj" : $(SOURCE) $(DEP_CPP_TXDL2_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl2_2.obj" : $(SOURCE) $(DEP_CPP_TXDL2_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl2_3.c
DEP_CPP_TXDL2_3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl2_3.obj" : $(SOURCE) $(DEP_CPP_TXDL2_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl2_3.obj" : $(SOURCE) $(DEP_CPP_TXDL2_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl3.c
DEP_CPP_TXDL3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl3.obj" : $(SOURCE) $(DEP_CPP_TXDL3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl3.obj" : $(SOURCE) $(DEP_CPP_TXDL3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl3_2.c
DEP_CPP_TXDL3_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl3_2.obj" : $(SOURCE) $(DEP_CPP_TXDL3_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl3_2.obj" : $(SOURCE) $(DEP_CPP_TXDL3_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl3_3.c
DEP_CPP_TXDL3_3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl3_3.obj" : $(SOURCE) $(DEP_CPP_TXDL3_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl3_3.obj" : $(SOURCE) $(DEP_CPP_TXDL3_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl4.c
DEP_CPP_TXDL4=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl4.obj" : $(SOURCE) $(DEP_CPP_TXDL4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl4.obj" : $(SOURCE) $(DEP_CPP_TXDL4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl4_2.c
DEP_CPP_TXDL4_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl4_2.obj" : $(SOURCE) $(DEP_CPP_TXDL4_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl4_2.obj" : $(SOURCE) $(DEP_CPP_TXDL4_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl4_3.c
DEP_CPP_TXDL4_3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl4_3.obj" : $(SOURCE) $(DEP_CPP_TXDL4_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl4_3.obj" : $(SOURCE) $(DEP_CPP_TXDL4_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim1_2.c
DEP_CPP_TXDLI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim1_2.obj" : $(SOURCE) $(DEP_CPP_TXDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim1_2.obj" : $(SOURCE) $(DEP_CPP_TXDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim2.c
DEP_CPP_TXDLIM=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim2_2.c
DEP_CPP_TXDLIM2=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim2_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim2_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim2_3.c
DEP_CPP_TXDLIM2_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim2_3.obj" : $(SOURCE) $(DEP_CPP_TXDLIM2_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim2_3.obj" : $(SOURCE) $(DEP_CPP_TXDLIM2_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim3_2.c
DEP_CPP_TXDLIM3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim3_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim3_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim4.c
DEP_CPP_TXDLIM4=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim4.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim4.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim4_2.c
DEP_CPP_TXDLIM4_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim4_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim4_2.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdlim4_3.c
DEP_CPP_TXDLIM4_3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdlim4_3.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdlim4_3.obj" : $(SOURCE) $(DEP_CPP_TXDLIM4_3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txim1.c
DEP_CPP_TXIM1=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txim1.obj" : $(SOURCE) $(DEP_CPP_TXIM1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txim1.obj" : $(SOURCE) $(DEP_CPP_TXIM1) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txim2.c
DEP_CPP_TXIM2=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txim2.obj" : $(SOURCE) $(DEP_CPP_TXIM2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txim2.obj" : $(SOURCE) $(DEP_CPP_TXIM2) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txim3.c
DEP_CPP_TXIM3=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txim3.obj" : $(SOURCE) $(DEP_CPP_TXIM3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txim3.obj" : $(SOURCE) $(DEP_CPP_TXIM3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\mipmaps\txdl1.c
DEP_CPP_TXDL1_C=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\txdl1.obj" : $(SOURCE) $(DEP_CPP_TXDL1_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\txdl1.obj" : $(SOURCE) $(DEP_CPP_TXDL1_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\objects\textureswap.c
DEP_CPP_TEXTU=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\textureswap.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\textureswap.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\objects\texobject.c
DEP_CPP_TEXOB=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\texobject.obj" : $(SOURCE) $(DEP_CPP_TEXOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\texobject.obj" : $(SOURCE) $(DEP_CPP_TEXOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\texture\objects\texobj.c
DEP_CPP_TEXOBJ=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\texobj.obj" : $(SOURCE) $(DEP_CPP_TEXOBJ) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\texobj.obj" : $(SOURCE) $(DEP_CPP_TEXOBJ) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\xformsimulate.c
DEP_CPP_XFORM=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\xformsimulate.obj" : $(SOURCE) $(DEP_CPP_XFORM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\xformsimulate.obj" : $(SOURCE) $(DEP_CPP_XFORM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\getmatrix.c
DEP_CPP_GETMA=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\getmatrix.obj" : $(SOURCE) $(DEP_CPP_GETMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\getmatrix.obj" : $(SOURCE) $(DEP_CPP_GETMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\getviewport.c
DEP_CPP_GETVI=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\getviewport.obj" : $(SOURCE) $(DEP_CPP_GETVI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\getviewport.obj" : $(SOURCE) $(DEP_CPP_GETVI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\matrixflow.c
DEP_CPP_MATRI=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\matrixflow.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\matrixflow.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\ortho.c
DEP_CPP_ORTHO=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\ortho.obj" : $(SOURCE) $(DEP_CPP_ORTHO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\ortho.obj" : $(SOURCE) $(DEP_CPP_ORTHO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\popmatrix.c
DEP_CPP_POPMA=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\popmatrix.obj" : $(SOURCE) $(DEP_CPP_POPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\popmatrix.obj" : $(SOURCE) $(DEP_CPP_POPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\rotate.c
DEP_CPP_ROTAT=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\rotate.obj" : $(SOURCE) $(DEP_CPP_ROTAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\rotate.obj" : $(SOURCE) $(DEP_CPP_ROTAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\scale.c
DEP_CPP_SCALE=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\scale.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\scale.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\translate.c
DEP_CPP_TRANSL=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\translate.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\translate.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\viewport.c
DEP_CPP_VIEWP=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\viewport.obj" : $(SOURCE) $(DEP_CPP_VIEWP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\viewport.obj" : $(SOURCE) $(DEP_CPP_VIEWP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\func\xform\frustum.c
DEP_CPP_FRUST=\
	".\func\xform\xform.h"\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\frustum.obj" : $(SOURCE) $(DEP_CPP_FRUST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\frustum.obj" : $(SOURCE) $(DEP_CPP_FRUST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\util.c
DEP_CPP_UTIL_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\check.c
DEP_CPP_CHECK=\
	".\env\env.h"\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\check.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\check.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\defaultState.c
DEP_CPP_DEFAU=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\defaultState.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\defaultState.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\evaluator.c
DEP_CPP_EVALU=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\evaluator.obj" : $(SOURCE) $(DEP_CPP_EVALU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\evaluator.obj" : $(SOURCE) $(DEP_CPP_EVALU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\getenv.c
DEP_CPP_GETEN=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\getenv.obj" : $(SOURCE) $(DEP_CPP_GETEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\getenv.obj" : $(SOURCE) $(DEP_CPP_GETEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\gltst.c
DEP_CPP_GLTST=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\gltst.obj" : $(SOURCE) $(DEP_CPP_GLTST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\gltst.obj" : $(SOURCE) $(DEP_CPP_GLTST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\imbuildutil.c
DEP_CPP_IMBUI=\
	".\inc\..\env\pc.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\imbuildutil.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\imbuildutil.obj" : $(SOURCE) $(DEP_CPP_IMBUI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\imbuildutil.obj" : $(SOURCE) $(DEP_CPP_IMBUI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\imgutil.c
DEP_CPP_IMGUT=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\imgutil.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\imgutil.obj" : $(SOURCE) $(DEP_CPP_IMGUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\imgutil.obj" : $(SOURCE) $(DEP_CPP_IMGUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\matrix.c
DEP_CPP_MATRIX=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\pixutil.c
DEP_CPP_PIXUT=\
	".\inc\..\env\pc.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\pixutil.obj" : $(SOURCE) $(DEP_CPP_PIXUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\pixutil.obj" : $(SOURCE) $(DEP_CPP_PIXUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\rand.c
DEP_CPP_RAND_=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\rand.obj" : $(SOURCE) $(DEP_CPP_RAND_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\rand.obj" : $(SOURCE) $(DEP_CPP_RAND_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\readwrite.c
DEP_CPP_READW=\
	".\inc\..\env\pc.h"\
	".\inc\alloca.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\readwrite.obj" : $(SOURCE) $(DEP_CPP_READW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\readwrite.obj" : $(SOURCE) $(DEP_CPP_READW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\set.c
DEP_CPP_SET_C=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\set.obj" : $(SOURCE) $(DEP_CPP_SET_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\set.obj" : $(SOURCE) $(DEP_CPP_SET_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\texsim.c
DEP_CPP_TEXSI=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	".\inc\strings.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\texsim.obj" : $(SOURCE) $(DEP_CPP_TEXSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\texsim.obj" : $(SOURCE) $(DEP_CPP_TEXSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\texutil.c
DEP_CPP_TEXUT=\
	".\inc\..\env\pc.h"\
	".\inc\bstring.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	".\lib\utilfont.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\texutil.obj" : $(SOURCE) $(DEP_CPP_TEXUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\texutil.obj" : $(SOURCE) $(DEP_CPP_TEXUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\alloc.c
DEP_CPP_ALLOC=\
	".\inc\..\env\pc.h"\
	".\inc\GL/glx.h"\
	".\inc\ogtst.h"\
	{$(INCLUDE)}"\GL\gl.h"\
	{$(INCLUDE)}"\GL\glu.h"\
	

!IF  "$(CFG)" == "ogtst - Win32 Release"


"$(INTDIR)\alloc.obj" : $(SOURCE) $(DEP_CPP_ALLOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ogtst - Win32 Debug"


".\Debug\alloc.obj" : $(SOURCE) $(DEP_CPP_ALLOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
