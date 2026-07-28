#!/usr/bin/env python3
"""codegen_8e_guards.py -- built-binary regression guard for the two
KMODE_EXCEPTION_NOT_HANDLED (0x1000008E) crashes in the H5 display driver
(3dfxvs.dll -> 3dfxv3d.dll / 3dfxv5d.dll).

Root-caused from .124 (Voodoo3) XP minidumps, 2026-07-28:
  * EIP 0xBF012346  = DrvBitBlt+0x48   -- `mov (%edi),%eax` derefs psoSrc
                      (arg2) with NO NULL check; psoSrc is NULL for solid/
                      pattern fills.  Fix: BITBLT.C null-guards psoSrc (and
                      psoDst) in the ENABLE_LOG_FILE block (commit a03a9fe).
  * EIP 0xBF04E4AC  = DdBlt+0x5EC      -- `mov 0x510(%edx),%eax` where edx =
                      _D3(lastContext) (pRc) == NULL in the context-less
                      texture-upload window; pRc->pHndlList == [NULL+0x510].
                      Fix: DDBLT32.C resolves the TXTRHNDL by walking the
                      GLOBAL g_pHndlList chain instead of pRc->pHndlList
                      (commits 8de09a3, cf3ab3e).

Unlike the source-grep invariants in test_source_invariants.sh, this checks the
actual CODEGEN of the linked DLL, so it also catches a stale-obj link OR a
preprocessor/config regression (e.g. DX<7 or LF=0) that would drop the fix from
the binary while the source still reads "fixed" -- which is exactly why the
.124 box crashed on a source-fixed-but-stale binary.

Asserts BOTH the fixed value (guard present) and, by construction, that the
old-buggy pattern (unguarded deref) is gone.  Exit 0 = pass, 1 = fail,
2 = could not run (missing objdump / DLL).

Usage: codegen_8e_guards.py [path-to-3dfxvs.dll]
"""
import os, re, shutil, subprocess, sys

DEFAULT_DLL = ("toolchain-3dfx/prefix/drive_c/3dfx/H5/W2K/Src/Video/"
               "Displays/H5/objfre/i386/3dfxvs.dll")
IMAGE_BASE = 0x10000


def find_objdump():
    for c in ("i686-w64-mingw32-objdump", "objdump"):
        if shutil.which(c):
            return c
    return None


def syms_text(objdump, dll):
    out = subprocess.check_output([objdump, "-t", dll],
                                  stderr=subprocess.DEVNULL).decode("latin1", "replace")
    funcs = []           # (rva, name) in .text (sec 1)
    g_pHndlList = None
    for line in out.splitlines():
        if "(sec  1)" in line:
            m = re.search(r"(0x[0-9a-f]+) (\S+)$", line)
            if m:
                funcs.append((int(m.group(1), 16), m.group(2)))
        m2 = re.search(r"0x([0-9a-f]+) _g_pHndlList$", line)
        if m2:
            g_pHndlList = int(m2.group(1), 16)
    funcs.sort()
    return funcs, g_pHndlList


def func_range(funcs, name):
    for i, (rva, nm) in enumerate(funcs):
        if nm == name:
            nxt = funcs[i + 1][0] if i + 1 < len(funcs) else rva + 0x2000
            return rva, nxt
    return None


def disasm(objdump, dll, a, b):
    return subprocess.check_output(
        [objdump, "-d", "--start-address=%d" % (IMAGE_BASE + a),
         "--stop-address=%d" % (IMAGE_BASE + b), "-m", "i386", dll],
        stderr=subprocess.DEVNULL).decode("latin1", "replace")


def main():
    dll = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_DLL
    here = os.path.dirname(os.path.abspath(__file__))
    os.chdir(os.path.dirname(here))  # repo root
    if not os.path.isfile(dll):
        print("SKIP  codegen 0x8E guards: DLL not found (%s)" % dll)
        return 2
    objdump = find_objdump()
    if not objdump:
        print("SKIP  codegen 0x8E guards: no objdump on PATH")
        return 2

    funcs, g_pHndlList = syms_text(objdump, dll)
    fail = 0

    # --- Guard 1: DdBlt resolves texture handles via the global g_pHndlList
    #     chain (NULL-context-safe), not pRc->pHndlList (==[NULL+0x510]).
    ddb = func_range(funcs, "_DdBlt@4")
    if ddb is None or g_pHndlList is None:
        print("FAIL  DdBlt/g_pHndlList symbol missing (DdBlt=%r g_pHndlList=%r)"
              % (ddb, g_pHndlList))
        fail = 1
    else:
        vma = IMAGE_BASE + g_pHndlList
        body = disasm(objdump, dll, *ddb)
        if ("%x" % vma) in body:
            print("PASS  DdBlt references g_pHndlList (0x%x) -- NULL-context "
                  "texture-download fix compiled in" % vma)
        else:
            print("FAIL  DdBlt does NOT reference g_pHndlList -- reverted to "
                  "pRc->pHndlList [NULL+0x510] crash (bugcheck 0x8E @ DdBlt+0x5EC)")
            fail = 1

    # --- Guard 2: DrvBitBlt null-checks psoSrc (edi) before dereferencing it.
    #     In the fixed build a `test %edi,%edi` (85 ff) precedes the first
    #     `mov (%edi),%eax` (8b 07); the buggy build derefs with no test.
    dbb = func_range(funcs, "_DrvBitBlt@44")
    if dbb is None:
        print("FAIL  DrvBitBlt@44 symbol missing")
        fail = 1
    else:
        body = disasm(objdump, dll, dbb[0], min(dbb[1], dbb[0] + 0x120))
        test_edi_seen = False
        guarded = None
        for line in body.splitlines():
            if not re.match(r"\s+[0-9a-f]+:", line):
                continue
            if re.search(r":\s*85 ff\s", line):            # test %edi,%edi
                test_edi_seen = True
            if re.search(r":\s*8b 07\s", line):            # mov (%edi),%eax
                guarded = test_edi_seen
                break
        if guarded:
            print("PASS  DrvBitBlt null-checks psoSrc before deref (test edi "
                  "precedes mov (edi)) -- fill-blt crash guarded")
        elif guarded is False:
            print("FAIL  DrvBitBlt derefs psoSrc with NO NULL check -- "
                  "bugcheck 0x8E @ DrvBitBlt+0x346 on a source-less fill")
            fail = 1
        else:
            print("FAIL  DrvBitBlt: could not locate the psoSrc deref to verify guard")
            fail = 1

    return fail


if __name__ == "__main__":
    sys.exit(main())
