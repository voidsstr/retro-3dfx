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
