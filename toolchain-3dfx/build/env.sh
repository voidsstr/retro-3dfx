#!/usr/bin/env bash
# Canonical environment for the retro-3dfx Wine/VC6/DDK build toolchain.
#   source toolchain-3dfx/build/env.sh
#
# The heavy toolchain (Wine, VC6, DDKs, the Wine build prefix) is large and
# reconstructable — it lives OUTSIDE the repo on a roomy Linux volume and is
# pointed at by $RETRO3DFX_TC (default below). Rebuild it with
# toolchain-3dfx/build/setup-toolchain.sh. See toolchain-3dfx/README.md.
#
# Layout under $RETRO3DFX_TC:
#   wine/       Kron4ek portable Wine 11.13 (wow64)
#   devtools/   == C:\3dfxtools  (VC6 SP5, MASM 6.15, W2K DDK, DX7 DDK, Win98 DDK)
#   prefix/     Wine prefix; drive_c/3dfx = space-free build copy of the H5 source
#   downloads/  the archive.org / github fetches that produced devtools/
#   bin/        7zz (extraction helper)
export TC="${RETRO3DFX_TC:-$HOME/retro3dfx-toolchain}"
export WINEPREFIX="$TC/prefix"
export PATH="$TC/wine/bin:$TC/bin:$PATH"
export WINEDEBUG=-all
export DISPLAY=
export COPYCMD=/Y            # xcopy overwrite w/o prompt (a prompt-loop once wrote a 38GB log)

# --- 3dfx build system env (nmake reads these; Wine imports the Unix env) ---
export BUILD_ROOT='c:\3dfx'
export BUILD_ROOT_SWLIBS='c:\3dfx\swlibs'
export FX_GLIDE_HW=H5
export FX_HW_PROJECTS=glide3
export FX_TARGET=WIN32
export FX_COMPILER=MICROSOFT
export FX_DLL_BUILD=1
export DIRECTXSDK='c:\3dfxtools\msvc6_0\vc98'
export W9XDDK='c:\3dfxtools\w9xddk'
export INCLUDE='c:\3dfxtools\msvc6_0\vc98\Include'
export LIB='c:\3dfxtools\msvc6_0\vc98\Lib'
# Windows %PATH% inside Wine: VC6 bin, the IDE bin (mspdb60.dll lives here!), MASM, and Windows dirs (xcopy)
export WINEPATH='c:\3dfxtools\msvc6_0\vc98\Bin;c:\3dfxtools\msvc6_0\Common\MSDev98\Bin;c:\3dfxtools\masm614\bin;c:\windows\system32;c:\windows'
export COMSPEC='c:\windows\system32\cmd.exe'

ulimit -f 2000000 2>/dev/null || true   # 38GB-log hazard guard — never build without it

wclean() { pkill -9 -x wineserver 2>/dev/null; pkill -9 -f 'winedevice' 2>/dev/null; true; }

# wnmake <dir-under-drive_c/3dfx> [logfile] [extra-set-cmds]
# Runs nmake inside a cmd session that sets TEMP/TMP at runtime: VC6 LINK spills a
# long object list to a response file via GetTempPath and dies ("cannot open
# TEMPFILE") when TEMP is unset — and Wine does NOT import the Unix TMP into the
# Windows environment. Judge success by artifacts + an error grep, never exit code.
wnmake() {
  local d="$1"; local log="${2:-/tmp/nmake_$(basename "$d").log}"; local extra="${3:-}"
  ( cd "$TC/prefix/drive_c/3dfx/$d" && \
    timeout 560 wine cmd /c "set TEMP=c:\\windows\\temp&& set TMP=c:\\windows\\temp&& ${extra}nmake /nologo" ) > "$log" 2>&1
  local rc=$?
  wclean
  return $rc
}

build_failed() {   # build_failed <logfile> -> 0 if a real compiler/linker error is present
  grep -qiE 'fatal error|U1077|U1073|U1052|U1056|error C[0-9]|error A[0-9]|error LNK|LNK1[0-9]' "$1"
}
