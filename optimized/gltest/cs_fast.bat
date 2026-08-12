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
rem Renderer: OpenGL (battle-tested path). NOTE: as of driver cf3ab3e the
rem Direct3D renderer ALSO works on the Voodoo5 (crash + white-world fixed) and
rem timedemos FASTER than GL (33.5 vs 30.6 fps @1024x768) -- picking "Direct3D"
rem in Video options is safe now. This launcher stays on GL because the GL path
rem has the per-process vsync-off optimization above (biggest fps win) and
rem months of stability history; switch to -d3d after more D3D playtime.
reg add "HKCU\Software\Valve\Half-Life\Settings" /v EngineD3D /t REG_DWORD /d 0 /f >nul 2>&1
cd /d "C:\Program Files\Bcs16 Romania\Counter-Strike 1.6"
start "" hl.exe -game cstrike -gl -w 1024 -h 768 -full -noipx -nojoy +exec userconfig.cfg +r_dynamic 0 +mp_decals 30 +cl_himodels 0 +fps_max 100 %*
