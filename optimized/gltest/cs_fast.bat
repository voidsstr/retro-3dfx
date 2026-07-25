@echo off
rem retro3dfx: launch Counter-Strike optimized for the CPU-bound 1GHz Voodoo5 box.
rem  * FX_GLIDE_SWAPINTERVAL=0 : vsync OFF for CS ONLY (GoldSrc's glide reads the
rem    process env before the registry), so the driver's global vsync stays ON for
rem    Q3/other games. Restores ~+65% fps (vsync was quantizing fps to the refresh).
rem  * +cvars on the command line run AFTER config.cfg/userconfig.cfg, so they win:
rem    r_dynamic 0    - no per-frame dynamic-lightmap recompute (big combat CPU win)
rem    mp_decals 30   - fewer bullet/blood decals (CPU + overdraw)
rem    cl_himodels 0  - low-detail player models (fewer verts to transform on CPU)
rem    fps_max 100    - stable frame pacing (uncapped just burns CPU re-rendering)
set FX_GLIDE_SWAPINTERVAL=0
cd /d "C:\Program Files\Bcs16 Romania\Counter-Strike 1.6"
start "" hl.exe -game cstrike -gl -w 1024 -h 768 -full -noipx -nojoy +exec userconfig.cfg +r_dynamic 0 +mp_decals 30 +cl_himodels 0 +fps_max 100 %*
