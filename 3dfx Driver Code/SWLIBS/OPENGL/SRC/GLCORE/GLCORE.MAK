#-------------------------------------------------------------------------
# GLCORE sublibrary makefile
#-------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Directories
#----------------------------------------------------------------------------

!ifndef DEBUG
TARGET_DIR      = .\release
DESTINATION_DIR = ..\release
!else
!if "$(DEBUG)"=="BC"
TARGET_DIR = .\bchecker
DESTINATION_DIR = ..\bchecker
!else
TARGET_DIR = .\debug
DESTINATION_DIR = ..\debug
!endif
!endif

#----------------------------------------------------------------------------
# Target, Default Rule, Common Make Code
#----------------------------------------------------------------------------

TARGET = glcore

all: $(DESTINATION_DIR)\$(TARGET).lib

!include "..\include\make\ogl.mak"

#-------------------------------------------------------------------------
# FILES
#
# CFILES and OFILES are split artificially
# here to accomodate a limitation in the 
# imagination of some microsoft engineer
#-------------------------------------------------------------------------

THISFILE = glcore.mak
LIB      = $(TARGET_DIR)\$(TARGET).lib
SUBLIB   = $(TARGET_DIR)\$(TARGET)_SUB.lib
CFILES_0 =           \
    .\dl_block.c     \
    .\dl_heap.c      \
    .\dl_init.c      \
    .\dl_list.c      \
    .\dl_mopt.c      \
    .\dl_opt.c       \
    .\dl_pplst.c     \
    .\dl_splc.c      \
    .\dl_splst.c     \
    .\dl_table.c     \
    .\fastdraw.c     \
    .\fmemclr.c      \
    .\g_api.c        \
    .\g_lcmpex.c     \
    .\g_lcomp.c      \
    .\g_lexec.c      \
    .\g_ltab.c       \
    .\g_noptab.c     \
    .\g_splcmp.c     \
    .\g_varray.c     \
    .\geom_og.c      \
    .\h_histogram.c  \
    .\ns.c           \
    .\px_api.c       \
    .\px_mod.c       \
    .\px_pack.c      \
    .\px_paths.c     \
    .\px_read.c      \
    .\px_rend.c      \
    .\px_upack.c     \
    .\s_accum.c      \
    .\s_atest.c      \
    .\s_attrib.c     \
    .\s_bitmap.c     \
    .\s_blend.c      \
    .\s_btdraw.c     \
    .\s_buffer.c     \
    .\s_capi.c       \
    .\s_ci.c         \
    .\s_clear.c      \
    .\s_clip.c       \
    .\s_contxt.c     \
    .\s_cpydsp.c     \
    .\s_ctable.c     \
    .\s_depth.c      \
    .\s_disp.c       \
    .\s_dlist.c      \
    .\s_eval.c       \
    .\s_export.c     \
    .\s_feed.c       \
    .\s_finish.c     \
    .\s_fog.c        

CFILES_1 =           \
    .\s_get.c        \
    .\s_global.c     \
    .\s_image.c      \
    .\s_itest.c      \
    .\s_lcache.c     \
    .\s_lcomp.c      \
    .\s_light.c      \
    .\s_lnclip.c     \
    .\s_lndraw.c     \
    .\s_lnfast.c     \
    .\s_lnfin.c      \
    .\s_lnspan.c     \
    .\s_math.c       \
    .\s_memmgr.c     \
    .\s_names.c      \
    .\s_napi.c       \
    .\s_pgaa.c       \
    .\s_pgclip.c     \
    .\s_pgdraw.c     \
    .\s_pgfin.c      \
    .\s_pgmode.c     \
    .\s_pgspan.c     \
    .\s_pick.c       \
    .\s_prim.c       \
    .\s_ptaa.c       \
    .\s_ptdraw.c     \
    .\s_ptfin.c      \
    .\s_rapi.c       \
    .\s_rect.c       \
    .\s_rgb.c        \
    .\s_rpos.c       \
    .\s_select.c     \
    .\s_size.c       \
    .\s_span.c       \
    .\s_sten.c       \
    .\s_store.c      \
    .\s_tapi.c       \
    .\s_tex.c        \
    .\s_texel.c      \
    .\s_texmgr.c     \
    .\s_vapi.c       \
    .\s_varray.c     \
    .\s_vertex.c     \
    .\s_xform.c      \
    .\s_zfuncs.c     \
    .\vc_cache.c     \
    .\vc_line.c      \
    .\vc_point.c     \
    .\vc_poly.c

OFILES_0 = $(CFILES_0:.c=.obj)
OFILES_1 = $(CFILES_1:.c=.obj)
!ifndef DEBUG
OFILES_0 = $(OFILES_0:.\=.\release\)
OFILES_1 = $(OFILES_1:.\=.\release\)
!else
!if "$(DEBUG)" != "BC"
OFILES_0 = $(OFILES_0:.\=.\debug\)
OFILES_1 = $(OFILES_1:.\=.\debug\)
!else
OFILES_0 = $(OFILES_0:.\=.\bchecker\)
OFILES_1 = $(OFILES_1:.\=.\bchecker\)
!endif
!endif


#-------------------------------------------------------------------------
# Rules
#
# Broken into two libraries because CL can't take
# a response line > 1024 characters long.
#
#-------------------------------------------------------------------------

$(DESTINATION_DIR)\$(TARGET).lib: "$(DESTINATION_DIR)" $(LIB)
    if exist $(DESTINATION_DIR)\$(TARGET).lib $(DEL) $(DESTINATION_DIR)\$(TARGET).lib
    copy $(LIB) $(DESTINATION_DIR)

$(LIB): $(THISFILE) "$(TARGET_DIR)" $(SUBLIB) $(OFILES_1)
    $(AR) @<<"$(TARGET_DIR)\tmp.ar"
    $(AR_OPTS)
    $(SUBLIB)
    $(OFILES_1)
    /out:$(LIB)
<<KEEP

$(SUBLIB): $(THISFILE) "$(TARGET_DIR)" $(OFILES_0)
    $(AR) @<<"$(TARGET_DIR)\tmp0.ar"
    $(AR_OPTS)
    $(OFILES_0)
    /out:$(SUBLIB)
<<

$(OFILES_0): $(THISFILE)
$(OFILES_1): $(THISFILE)


{.}.c{$(TARGET_DIR)}.obj::
    $(CPP) @<<"$(TARGET_DIR)\tmp.cmp"
    $(CPP_OPTS) 
    $(CPP_DEFS) 
    $(CPP_PATHS) 
    $<
<<KEEP

clobber:
    @echo clobber all files
    if exist "$(TARGET_DIR)\*" $(DELDIR) $(TARGET_DIR)

clean:
    @echo clean intermediate files
    if exist "$(TARGET_DIR)\*" attrib -r $(TARGET_DIR)\*.*
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.obj
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.pch
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.map
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.pdb
    if exist "$(TARGET_DIR)\*" $(DEL) $(TARGET_DIR)\*.asm
    if exist "$(TARGET_DIR)\*" $(DEL) $(SUBLIB)
