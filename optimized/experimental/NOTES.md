# Multitexture overbright — 4 attempts, root cause narrowed

Single-pass GL_ARB_multitexture WORKS (+5-6% fps: 80.1->85.0) but renders ~half
brightness because Q3 sends world vertex color = identityLight (0.5), expecting a
2x overbright that the 2-pass path gets free from the lightmap blend
(GL_SRC_COLOR/GL_DST_COLOR). The 2x is not being reproduced in single-pass.

Attempts (all confirmed the gate ENGAGES on hardware via C:\3dfxogl.log, none
brightened the pixels):
1. grColorCombineExt output shift=1 (SST_CM_CC_OUTSHIFT_2X). Log: ext_active=1,
   grColorCombineExt resolved, texEnv0/1=0x2100. Result: still dark — the Napalm
   ext output-shift does NOT double the combined result in practice.
2. Hardened the gate (dropped the ' COMBINE ' ext-string check, probe
   grGetProcAddress only). Log confirms it engages. Still dark.
3. Iterated-color doubling in the 5 sst_pgmode.c triangle-fill procs
   (__glSSTRenderTriangle/Flat/Smooth/*OneSided). Log shows the branch flag set
   ("applying 2x via vertex-color double") but the render-proc "doubling iterated
   color (was N,N,N)" line NEVER fires -> Q3's multitextured world geometry does
   NOT go through those 5 fill procs.

ROOT CAUSE NARROWED: Q3's world path is glDrawElements -> vertex-array compile ->
a DIFFERENT triangle dispatch than the 5 immediate-fill procs. The 2x must be
applied on THAT path. Next: trace which proc gc->procs.renderTriangle / the
vertex-array JIT (SST_OG.C / sst_vararray) actually calls for the collapsed
MODULATExMODULATE world batch, and double the iterated color (or apply a working
2x) THERE. The instrumentation + gate + combine plumbing are all in place; only
the injection point is wrong. Needs live confirmation via the "doubling iterated
color (was ...)" log actually firing.

Experimental DLLs here (all render dark, DO NOT SHIP):
- 3dfxogl-0.1.4-multitexture-DARK.dll            (ext-shift 2x, md5 c10dcc38)
- 3dfxogl-0.1.4-multitexture-itercolor-DARK.dll  (fill-proc doubling, md5 1fc3e1ce)
Shipped/correct driver = 0.1.3 (ICD md5 0d8c9a5a), restored to dist/ and .143.

## Attempt 5 (S_VARRAY.C CompileElementsIndexed) — ALSO misses
Doubled the compiled vertex color right after (*compileElements)(gc,offset+i,el,1)
in CompileElementsIndexed, gated on __glSSTOverbright2xVtx. On hardware the
combine-branch log ("applying 2x") fires but the new "VARRAY: doubled compiled
vtx color" log does NOT — so Q3's world geometry does not reach that per-element
doubling site either (its JIT-batched drawVertexes path bypasses the loop, and/or
the gate is reset between combine-validation and compile by
__glSSTSetCDRSTexture / __glSSTResetCombineCache).

## Conclusion after 5 attempts
The 2x gate FLAG sets correctly on every attempt, but the doubling never lands on
Q3's actual pixel-producing path. This needs RUNTIME TRACING, not more guesses:
instrument gc->procs.renderTriangle and the __GL_CODEGEN JIT (__glSSTGenerateCompile
in SST_OG.C) to log the ACTUAL proc/address Q3 calls per world triangle, find where
the GrVertex r,g,b is finally written to the FIFO, and double there — OR verify the
gate isn't being cleared between validate and draw (check the __glSSTSetCDRSTexture
/ __glSSTResetCombineCache reset calls; the branch log fires so the gate is set at
SOME point, but maybe not when the draw's vertices are built). All plumbing is in
place; this is purely an injection-point / gate-lifetime problem.
Shipped/correct = 0.1.3 (ICD md5 0d8c9a5a). 4 dark experimental DLLs preserved here.

## Glide->GDI teardown / stuck-mode / garble — DIAGNOSED (2026-07-17 morning)
Symptom: after Quake III exits the Voodoo5 is stuck in 640x480 Glide mode; forcing
a GDI mode change (setmode/ChangeDisplaySettings) then garbles the screen (Glide
FB + GDI intertwined), self-heals as the desktop repaints.

Root cause (confirmed with grSstWinClose logging -> C:\glide3x.log):
- CLEAN Q3 exit (+quit): grSstWinClose runs fully, *lostContext==0, WILL restore
  video, hwcRestoreVideo OK -> **desktop restores to 1024x768, NO garble.** Works.
- CRASH / taskkill exit: grSstWinClose is NEVER CALLED (process died) -> board left
  in Glide mode -> stuck 640x480 -> garble when GDI reclaims. The 0.1.0 dummy
  lostContext is NOT the cause (its value is 0, so the early-return skip never fires).
So the driver teardown is CORRECT; the stuck/garble is purely the abnormal-exit case
where the app can't clean up.

Fix options:
1. Workaround (shipped): play_q3.bat = start /wait quake3 ... then setmode 1024 768
   32 85. Clean exit: redundant (grSstWinClose already restored). Crash: setmode
   restores GDI (brief self-healing garble).
2. Proper driver fix (needs display-driver rebuild + reboot; logging is BUILT):
   display driver DrvAssertMode(enable) on GDI reclaim after a dead Glide app must
   fully reinit/clear the framebuffer so there's no garble. Instrumented in
   3dfxv5d.dll (ENABLE.C DrvAssertMode/DrvEnableSurface, HWCEXT.C lifecycle escapes,
   EngDebugPrint '3DFXV5D:') + miniport writes 'LastMode' REG_BINARY (readable via
   REGREAD at HKLM\System\CurrentControlSet\Services\<3dfxv5m>\Device0\LastMode).
Logging builds in dist/: glide3x 21c7422e (grSstWinClose trace, DEPLOYED, no-reboot),
3dfxv5d.dll e4e1ed2a + 3dfxv5m.sys 2fb49957 (need reboot to deploy).
