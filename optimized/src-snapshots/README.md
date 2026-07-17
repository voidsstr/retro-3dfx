# ICD source snapshots

The OpenGL ICD (`3dfxogl.dll`) is built in the untracked Wine prefix at
`toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/`. The build tree is
**not** in git — only the pristine leaked source (`3dfx Driver Code/`) and the
output DLLs (`optimized/*.dll`) are. That means our accumulated source edits had
no diffable, revertible record. These patch snapshots fix that: each is a
line-ending-normalized `diff -u` of the build tree against the pristine
`3dfx Driver Code/SWLIBS/OPENGL` reference, so any build's exact source delta is
recoverable and reviewable.

## Files

- `icd-0.1.4-experimental-vs-pristine.patch` — the state that produced the
  0.1.4-EXPERIMENTAL multitexture builds (dark, **not shipped**). Contains the
  **shipped** 0.1.1–0.1.3 optimizations (P6 codegen, glide3x dedup, S_VARRAY
  post-transform vertex cache) **plus** the unshipped 0.1.4 GL_ARB_multitexture
  single-pass plumbing (SST_TEX.C combine, sst_pgmode.c fill-proc 2x, S_VARRAY.C
  per-element 2x — all of which render at half brightness; see
  `../experimental/NOTES.md`). SST_OG.C (the JIT codegen) is **pristine** — the
  multitexture 2x injection point was never found on Q3's actual world path.

## Apply / inspect

```bash
REF="3dfx Driver Code/SWLIBS/OPENGL"          # pristine, in repo
BT=toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL   # Wine build tree
# review a single file's delta:
sed -n '/+++ b\/GLIDE3X\/SST\/SST_TEX.C/,/^diff /p' icd-0.1.4-experimental-vs-pristine.patch
# reconstruct the build-tree file from pristine (paths are a/<rel> b/<rel>):
patch -p1 -d "$BT" < icd-0.1.4-experimental-vs-pristine.patch   # (from a pristine copy)
```

The shipped/correct driver is **0.1.3** (ICD md5 `0d8c9a5a`). To get clean 0.1.3
source from this snapshot, drop the multitexture hunks (SST_TEX.C, the
sst_pgmode.c/S_VARRAY.C 2x sites, the ARB entry points in WGLCMDS.C/GLICD.C,
sst_export.c, and the `__glSSTOverbright2x*` globals) and keep the vertex-cache
+ codegen hunks.
